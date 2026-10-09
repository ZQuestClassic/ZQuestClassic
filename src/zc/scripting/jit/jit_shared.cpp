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

static std::vector<std::vector<pc_t>> find_block_predecessors(const ZasmCFG& cfg)
{
	std::vector<std::vector<pc_t>> block_predecessors(cfg.block_starts.size());
	for (pc_t i = 0; i < block_predecessors.size(); i++)
	{
		for (pc_t edge : cfg.block_edges[i])
		{
			block_predecessors[edge].push_back(i);
		}
	}
	return block_predecessors;
}

// True if the functions cover the script without gaps, and control enters each one only at its
// start (by a call, or falling through from the function before it). Then the analysis of each
// function on its own (see zasm_construct_function_cfg) matches its part of the script's.
static bool control_enters_functions_only_at_start(const zasm_script* script, const StructuredZasm& structured_zasm, const ZasmCFG& cfg)
{
	const auto& functions = structured_zasm.functions;
	for (size_t i = 1; i < functions.size(); i++)
	{
		if (functions[i].start_pc != functions[i - 1].final_pc + 1)
			return false;
	}

	// A conditional jump outside of the script has an edge to an arbitrary block.
	pc_t script_start_pc = functions.front().start_pc;
	pc_t script_final_pc = functions.back().final_pc;
	for (pc_t i = script_start_pc; i <= script_final_pc; i++)
	{
		int command = script->zasm[i].command;
		int arg1 = script->zasm[i].arg1;
		if ((command == GOTOCMP || command == GOTOTRUE || command == GOTOFALSE || command == GOTOLESS || command == GOTOMORE) &&
			(arg1 < (int)script_start_pc || arg1 > (int)script_final_pc))
			return false;
	}

	std::vector<pc_t> block_function_ids(cfg.block_starts.size());
	pc_t fn_id = 0;
	for (pc_t block = 0; block < cfg.block_starts.size(); block++)
	{
		while (cfg.block_starts[block] > functions[fn_id].final_pc)
			fn_id++;
		block_function_ids[block] = fn_id;
	}

	for (pc_t block = 0; block < cfg.block_starts.size(); block++)
	{
		for (pc_t edge : cfg.block_edges[block])
		{
			if (edge >= cfg.block_starts.size())
				return false;

			pc_t edge_fn_id = block_function_ids[edge];
			if (edge_fn_id != block_function_ids[block] && cfg.block_starts[edge] != functions[edge_fn_id].start_pc)
				return false;
		}
	}

	return true;
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
	};

	// Populate ZasmFunction::may_yield. A call into a function that may yield is treated as
	// reading every register by the liveness analysis below (so the register cache flushes
	// everything before it), and is never emitted as a direct native call.
	zasm_find_yielding_functions(script, j_script->structured_zasm);

	// Only finding the yielding functions needs the call graph, and only the ZASM optimizer needs
	// the call sites.
	for (auto& fn : j_script->structured_zasm.functions)
		fn.called_by_functions.clear();
	j_script->structured_zasm.function_calls.clear();

	// What is live in a function depends on every function it calls and every function that
	// calls it, so that takes analyzing the whole script. Only keep the summary of each function,
	// and what is live on its entry.
	JitFunctionAnalysis analysis{
		.cfg = zasm_construct_cfg(script, pc_ranges),
	};
	j_script->function_liveness = zasm_analyze_function_liveness(script, analysis.cfg, j_script->structured_zasm, true, &analysis.liveness);

	j_script->function_live_in.reserve(j_script->structured_zasm.functions.size());
	for (const auto& fn : j_script->structured_zasm.functions)
		j_script->function_live_in.push_back(analysis.liveness[analysis.cfg.block_id_from_start_pc(fn.start_pc)].in);

	if (!control_enters_functions_only_at_start(script, j_script->structured_zasm, analysis.cfg))
	{
		jit_printf("[jit] keeping the analysis of the whole script, since control enters a function somewhere other than its start: %s\n", script->name.c_str());
		analysis.block_predecessors = find_block_predecessors(analysis.cfg);
		j_script->script_analysis = std::make_unique<JitFunctionAnalysis>(std::move(analysis));
	}

	j_script->function_start_pcs.reserve(j_script->structured_zasm.functions.size());
	for (const auto& fn : j_script->structured_zasm.functions)
	{
		j_script->function_start_pcs.push_back(fn.start_pc);
	}

	j_script->compiled_functions.reserve(j_script->structured_zasm.functions.size());
	for (ZasmFunction& fn : j_script->structured_zasm.functions)
		j_script->compiled_functions.push_back({stub_exec_function, fn.id});

	// No function may be direct-called until it is compiled and committed.
	size_t num_functions = j_script->structured_zasm.functions.size();
	j_script->direct_entry_table = std::make_unique<uintptr_t[]>(num_functions);
	for (size_t i = 0; i < num_functions; i++)
		j_script->direct_entry_table[i] = (uintptr_t)jit_direct_not_compiled;

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

const JitFunctionAnalysis& jit_analyze_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn, JitFunctionAnalysis& storage)
{
	if (j_script->script_analysis)
		return *j_script->script_analysis;

	const auto& structured_zasm = j_script->structured_zasm;
	storage.cfg = zasm_construct_function_cfg(script, structured_zasm, fn);

	// The blocks outside of this function are where it calls, jumps or falls through to - the
	// start of some function. Calls get their effect from function_liveness instead.
	size_t num_blocks = storage.cfg.block_starts.size();
	std::vector<std::optional<uint8_t>> fixed_live_in(num_blocks);
	for (pc_t block = 0; block < num_blocks; block++)
	{
		pc_t pc = storage.cfg.block_starts[block];
		if (pc < fn.start_pc || pc > fn.final_pc)
			fixed_live_in[block] = j_script->function_live_in[structured_zasm.start_pc_to_function.at(pc)];
	}

	storage.liveness = zasm_run_liveness_analysis(script, storage.cfg, structured_zasm, j_script->function_liveness, true, &fixed_live_in);
	storage.block_predecessors = find_block_predecessors(storage.cfg);
	return storage;
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

		ctx->entry_mode = 0;
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
			pc_t id = j_fn.id;
			JittedFunctionImpl exec = j_fn.exec;
			j_instance->j_script->compiled_functions[id] = std::move(j_fn);
			// Publish for direct calls only once the function is committed, and
			// never for functions that can yield (see the field's comment).
			if (!j_instance->j_script->structured_zasm.functions[id].may_yield)
				j_instance->j_script->direct_entry_table[id] = (uintptr_t)exec;
			pending.pop_front();
		}
	}
	al_unlock_mutex(j_instance->j_script->mutex);

	extern int32_t(*stack)[MAX_STACK_SIZE];

	JittedExecutionContext ctx{
		.j_instance = j_instance,
		.registers = j_instance->ri->d,
		.global_registers = game->global_d,
		.script_registers = j_instance->ri->script_d.data(),
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

// Helpers for direct native calls between compiled functions. The ZASM return stack is maintained
// around each direct call so that any unwind back to the driver (interpreter bail, quit,
// driver-path fallback) sees exactly the state a driver-made call would have left.
//
// Called at the top of a direct-entered function; the call site left the ZASM return pc in
// ctx->call_pc. Returns 0 to proceed. Each direct call is a real machine stack frame, so past a
// depth cap this instead rewrites ctx into a driver-path call (the same one jit_direct_not_compiled
// makes) and returns 1; the callee propagates EXEC_RESULT_CALL without executing.
int32_t jit_direct_enter(JittedExecutionContext* ctx, int32_t callee_start_pc)
{
	extern refInfo *ri;

	int32_t ret_pc = ctx->call_pc;
	if (ri->retsp >= 256)
	{
		ctx->pc = ret_pc - 1;
		ctx->call_pc = callee_start_pc;
		return 1;
	}

	// Mirror the driver, which sets ctx->pc to the call target on every call: an error bail inside
	// the callee reports its trace from ctx->pc.
	ctx->pc = callee_start_pc;
	retstack_push(ret_pc);
	return 0;
}

// The direct entry of every function not (yet) compiled. Rewrites ctx into the driver-path call the
// call site would otherwise have made, like jit_direct_enter does past its depth cap, so call sites
// need no branch of their own for this case. The call site stored the return pc in ctx->call_pc;
// the CALLFUNC just before it names the callee.
int32_t jit_direct_not_compiled(JittedExecutionContext* ctx)
{
	pc_t call_pc = ctx->call_pc - 1;
	ctx->pc = call_pc;
	ctx->call_pc = ctx->j_instance->script->zasm_script->zasm[call_pc].arg1;
	return EXEC_RESULT_CALL;
}

// Called at a direct-entered function's RETURNFUNC (the driver pops for driver-entered functions).
void jit_direct_retstack_pop()
{
	retstack_pop();
}
