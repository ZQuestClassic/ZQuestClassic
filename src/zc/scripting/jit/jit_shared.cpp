// Architecture-independent JIT driver: compilation pipeline, script/instance
// lifecycle, hot-function profiling, and the run loop. The per-architecture
// backends (jit_x64.cpp, jit_a64.cpp) provide code generation via
// jit_backend_compile_function; everything here is shared between them.

#include "zc/scripting/jit/jit_shared.h"
#include "base/general.h"
#include "base/util.h"
#include "core/zdefs.h"
#include "components/zasm/defines.h"
#include "components/zasm/pc.h"
#include "components/zasm/table.h"
#include "zc/scripting/jit/jit.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zasm_pipeline.h"
#include "zc/zasm_utils.h"
#include "components/zasm/serialize.h"
#include <memory>
#include <functional>

static int hot_function_loop_count_threshold;
static int hot_function_call_count_threshold;

// Very useful tool for identifying a single bad function compilation.
// Use with a tool like `find-first-fail`: https://gitlab.com/ole.tange/tangetools/-/blob/master/find-first-fail/find-first-fail
// 1. Enable ENABLE_BISECT_TOOL below.
// 2. Make a new script `tmp.sh` calling a failing replay (change to use the failing replay, but don't change --extra_args):
//        python tests/run_replay_tests.py --filter stellar --frame 40000 --not_interactive --extra_args="-replay-fail-assert-instant -jit-precompile -jit-threads 0 -test-jit-bisect $1"
// 3. Run the bisect script (may need to increase the end range if script is large, as it is based on number of functions):
//        bash ~/tools/find-first-fail.sh -s 0 -e 10000 -v -q bash tmp.sh
// 4. For the number given, set `-test-jit-bisect` to that, and set a breakpoint
//    where specified in bisect_tool_should_skip. Whatever function being processed is the one to focus on.
// #define ENABLE_BISECT_TOOL
static bool bisect_tool_should_skip()
{
#ifdef ENABLE_BISECT_TOOL
	static int64_t c = 0;
	static int64_t x = get_flag_int("-test-jit-bisect").value();
	// Skip the first x calls.
	bool skip = 0 <= c && c < x;
	c++;
	if (!skip)
		// Set a breakpoint here.
		x = x;
	return skip;
#else
	return false;
#endif
}

static bool compile_and_queue_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn)
{
	j_script->functions_requested_to_be_compiled[fn.id] = true;

	if (bisect_tool_should_skip())
		return false;

	auto j_fn = jit_backend_compile_function(script, j_script, fn);
	if (!j_fn)
		return false;

	al_lock_mutex(j_script->mutex);
	j_script->pending_compiled_jit_functions.push_back(std::move(*j_fn));
	al_unlock_mutex(j_script->mutex);

	return true;
}

// Note: even if the function is compiled instantly, the current execution will continue to use the
// interpreter until the next script entry (jit_run_script). This is because the data structures are
// protected by a mutex, and commiting the new functions only once per execution (rather than every
// call to exec_script) reduces a lot of lock overhead.
static void create_compile_function_task(JittedScript* j_script, zasm_script* script, const ZasmFunction& fn)
{
	if (auto worker_pool = zasm_pipeline_worker_pool())
	{
		worker_pool->add_task([script, j_script, fn](){
			compile_and_queue_function(script, j_script, fn);
		});
	}
	else
	{
		compile_and_queue_function(script, j_script, fn);
	}
}

static pc_t find_function_id_for_pc(JittedScriptInstance* j_instance, pc_t pc)
{
	if (auto id = util::find(j_instance->j_script->pc_to_containing_function_id_cache, pc))
		return *id;

	for (auto& fn : j_instance->j_script->structured_zasm.functions)
	{
		if (fn.start_pc <= pc && fn.final_pc >= pc)
		{
			j_instance->j_script->pc_to_containing_function_id_cache[pc] = fn.id;
			return fn.id;
		}
	}

	jit_error("[jit ERROR] could not find function containing pc %d in script %s\n", pc, j_instance->script->name().c_str());
	return -1;
}

// When a function loops enough times, create a compile task.
void jit_profiler_increment_function_back_edge(JittedScriptInstance* j_instance, pc_t pc)
{
	int threshold = hot_function_loop_count_threshold;
	pc_t fn_id = find_function_id_for_pc(j_instance, pc);
	if (fn_id == (pc_t)-1)
		return;

	int n = ++j_instance->j_script->profiler_function_back_edge_count[fn_id];
	if (n == threshold && !j_instance->j_script->functions_requested_to_be_compiled[fn_id])
	{
		auto& fn = j_instance->j_script->structured_zasm.functions[fn_id];
		jit_printf("[jit] created compilation task for hot function looped many times (script: %s, name: %s, start: %d)\n", j_instance->script->zasm_script->name.c_str(), fn.name().c_str(), fn.start_pc);
		create_compile_function_task(j_instance->j_script, j_instance->script->zasm_script.get(), fn);
	}
}

// When a function is called enough times, create a compile task.
static void profiler_increment_function_call(JittedExecutionContext* ctx, const ZasmFunction& fn)
{
	int threshold = hot_function_call_count_threshold;
	auto j_instance = ctx->j_instance;
	int n = ++j_instance->j_script->profiler_function_call_count[fn.id];
	if (n == threshold && !j_instance->j_script->functions_requested_to_be_compiled[fn.id])
	{
		jit_printf("[jit] created compilation task for hot function called many times (script: %s, name: %s, start: %d)\n", j_instance->script->zasm_script->name.c_str(), fn.name().c_str(), fn.start_pc);
		create_compile_function_task(j_instance->j_script, j_instance->script->zasm_script.get(), fn);
	}
}

static pc_t find_function_id_containing_pc(const std::vector<pc_t>& function_start_pcs, pc_t pc)
{
	pc_t fn_id;
	if (auto it = std::upper_bound(function_start_pcs.begin(), function_start_pcs.end(), pc); it != function_start_pcs.begin())
	{
		fn_id = std::distance(function_start_pcs.begin(), it) - 1;
	}
	else
	{
		// Shouldn't happen.
		jit_error("[jit ERROR] unknown function for pc: %d\n", pc);
		fn_id = -1;
	}

	return fn_id;
}

static int stub_exec_function(JittedExecutionContext* ctx)
{
	JittedScriptInstance* j_instance = ctx->j_instance;
	JittedScript* j_script = j_instance->j_script;
	pc_t fn_id = find_function_id_containing_pc(j_script->function_start_pcs, ctx->pc);
	CHECK(fn_id != (pc_t)-1);
	ZasmFunction& fn = ctx->j_instance->j_script->structured_zasm.functions[fn_id];

	profiler_increment_function_call(ctx, fn);

	if (int r = run_script_jit_until_call_or_return(ctx->j_instance, ctx->pc, ctx->sp))
	{
		ctx->pc = ctx->j_instance->ri->pc;
		ctx->sp = ctx->j_instance->ri->sp;
		ctx->ret_code = ctx->j_instance->should_wait ? RUNSCRIPT_OK : r;
		ctx->j_instance->should_wait = false;
		return EXEC_RESULT_EXIT;
	}

	ctx->pc = ctx->j_instance->ri->pc;
	ctx->sp = ctx->j_instance->ri->sp;
	return EXEC_RESULT_CONTINUE;
}

static JittedScript* init_jitted_script(zasm_script* script)
{
	StructuredZasm structured_zasm = zasm_construct_structured(script);
	std::vector<std::pair<pc_t, pc_t>> pc_ranges;
	for (const auto& fn : structured_zasm.functions)
	{
		pc_ranges.emplace_back(fn.start_pc, fn.final_pc);
	}

	auto j_script = new JittedScript{
		.structured_zasm = std::move(structured_zasm),
		.cfg = zasm_construct_cfg(script, pc_ranges),
	};

	// Populate ZasmFunction::may_yield (used once the register cache is ported;
	// harmless to compute now and keeps init identical to the x64 backend).
	zasm_find_yielding_functions(script, j_script->structured_zasm);

	j_script->liveness = zasm_run_liveness_analysis(script, j_script->cfg, true, &j_script->structured_zasm);

	j_script->block_predecessors.resize(j_script->cfg.block_starts.size());
	for (pc_t i = 0; i < j_script->block_predecessors.size(); i++)
	{
		for (pc_t edge : j_script->cfg.block_edges[i])
		{
			j_script->block_predecessors[edge].push_back(i);
		}
	}

	j_script->function_start_pcs.reserve(j_script->structured_zasm.functions.size());
	for (const auto& fn : j_script->structured_zasm.functions)
	{
		j_script->function_start_pcs.push_back(fn.start_pc);
	}

	j_script->compiled_functions.reserve(j_script->structured_zasm.functions.size());
	for (ZasmFunction& fn : j_script->structured_zasm.functions)
		j_script->compiled_functions.push_back({stub_exec_function, fn.id});

	if (DEBUG_JIT_PRINT_ASM)
		j_script->debug_handle = std::make_unique<ScriptDebugHandle>(script, ScriptDebugHandle::OutputSplit::ByScript, script->name);

	j_script->mutex = al_create_mutex();

	j_script->profiler_function_call_count.resize(j_script->structured_zasm.functions.size());
	j_script->profiler_function_back_edge_count.resize(j_script->structured_zasm.functions.size());
	j_script->functions_requested_to_be_compiled.resize(j_script->structured_zasm.functions.size());

	return j_script;
}

void jit_startup_impl()
{
	hot_function_loop_count_threshold = std::max(1, (int)get_flag_int("-jit-hot-function-loop-count").value_or(zc_get_config("ZSCRIPT", "jit_hot_function_loop_count", 1000)));
	hot_function_call_count_threshold = std::max(1, (int)get_flag_int("-jit-hot-function-call-count").value_or(zc_get_config("ZSCRIPT", "jit_hot_function_call_count", 10)));
}

// Doesn't actually compile anything (unless precompile is enabled).
// Sets up everything needed for the per-function compilation.
JittedScript* jit_compile_script(zasm_script* script)
{
	if (script->size <= 1)
		return nullptr;

	jit_printf("[jit] initializing script for compilation: %s, id: %d\n", script->name.c_str(), script->id);
	auto j_script = init_jitted_script(script);
	if (!j_script)
		return nullptr;

	if (jit_should_precompile())
	{
		jit_printf("[jit] compiling script: %s, id: %d, len: %zu\n", script->name.c_str(), script->id, script->size);
		for (ZasmFunction& fn : j_script->structured_zasm.functions)
			create_compile_function_task(j_script, script, fn);
	}

	return j_script;
}

JittedScriptInstance* jit_create_script_impl(script_data* script, refInfo* ri, JittedScript* j_script)
{
	return new JittedScriptInstance{
		.j_script = j_script,
		.script = script,
		.ri = ri,
	};
}

static bool exec_script(JittedExecutionContext* ctx)
{
	JittedScriptInstance* j_instance = ctx->j_instance;
	JittedScript* j_script = j_instance->j_script;

	pc_t fn_id = find_function_id_containing_pc(j_script->function_start_pcs, ctx->pc);

	int exec_result;
	if (fn_id != (pc_t)-1)
	{
		auto& j_fn = j_script->compiled_functions[fn_id];

		ctx->resume_address = 0;
		if (ctx->pc != j_script->structured_zasm.functions[fn_id].start_pc)
		{
			if (auto address = util::find(j_script->pc_to_resume_address, ctx->pc - 1))
				ctx->resume_address = *address;
		}

		exec_result = j_fn.exec(ctx);
	}
	else
	{
		// Fallback to the interpreter if no function was found (error case).
		if (int r = run_script_jit_until_call_or_return(j_instance, ctx->pc, ctx->sp))
		{
			ctx->ret_code = ctx->j_instance->should_wait ? RUNSCRIPT_OK : r;
			ctx->j_instance->should_wait = false;
			return false;
		}
		ctx->pc = j_instance->ri->pc;
		ctx->sp = j_instance->ri->sp;
		return true;
	}

	if (exec_result == EXEC_RESULT_CALL)
	{
		int ret_pc = ctx->pc + 1;
		ctx->pc = ctx->call_pc;
		ctx->call_pc = -1;

		if (j_instance->ri->retsp >= MAX_CALL_FRAMES)
		{
			ctx->ret_code = RUNSCRIPT_JIT_CALL_LIMIT;
			j_instance->ri->pc = ctx->pc;
			j_instance->ri->sp = ctx->sp;
			return false;
		}

		void retstack_push(int32_t val);
		retstack_push(ret_pc);
		return true;
	}
	else if (exec_result == EXEC_RESULT_RETURN)
	{
		std::optional<int32_t> retstack_pop(void);
		if (auto pc = retstack_pop())
		{
			ctx->pc = *pc;
			return true;
		}
		else
		{
			ctx->ret_code = RUNSCRIPT_JIT_QUIT;
			return false;
		}
	}
	else if (exec_result == EXEC_RESULT_EXIT)
	{
		j_instance->ri->pc = ctx->pc;
		j_instance->ri->sp = ctx->sp;
		return false;
	}
	else if (exec_result == EXEC_RESULT_CONTINUE)
	{
		return true;
	}
	else
	{
		Z_error_fatal("[jit ERROR] unknown exec result: %d\n", exec_result);
	}
}

bool jit_can_start_script()
{
	// This backend runs jitted scripts as ordinary native calls, so nested
	// jitted script execution is fine.
	return true;
}

bool jit_precompile_scripts_impl([[maybe_unused]] const std::vector<zasm_script*>& scripts, [[maybe_unused]] const std::function<void(zasm_script*, JittedScript*)>& on_compiled)
{
	// This backend precompiles via the engine worker pool.
	return false;
}

int jit_run_script(JittedScriptInstance* j_instance)
{
	// Commit any recently compiled functions.
	al_lock_mutex(j_instance->j_script->mutex);
	{
		auto& pending = j_instance->j_script->pending_compiled_jit_functions;
		while (!pending.empty())
		{
			JittedFunction& j_fn = pending.front();
			j_instance->j_script->pc_to_resume_address.insert(j_fn.pc_to_resume_address.begin(), j_fn.pc_to_resume_address.end());
			j_fn.pc_to_resume_address.clear();
			j_instance->j_script->compiled_functions[j_fn.id] = std::move(j_fn);
			pending.pop_front();
		}
	}
	al_unlock_mutex(j_instance->j_script->mutex);

	extern int32_t(*stack)[MAX_STACK_SIZE];

	JittedExecutionContext ctx{
		.j_instance = j_instance,
		.registers = j_instance->ri->d,
		.global_registers = game->global_d,
		.stack_base = *stack,
		.sp = j_instance->ri->sp,
		.pc = j_instance->ri->pc,
		.call_pc = (pc_t)-1,
	};

	while (true)
	{
		if (!exec_script(&ctx))
			break;
	}

	return ctx.ret_code;
}

void jit_release(JittedScript* j_script)
{
	if (!j_script)
		return;

	al_destroy_mutex(j_script->mutex);
	delete j_script;
}
