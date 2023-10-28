// See docs/jit.md

// Opportunities for improvement:
// * Preprocess the ZASM byte code into function sections, and only compile one function at a time when deemed "hot".
// * Multiple PUSHR (for example: Maths in playground.qst) commands could be combined to only modify the stack index pointer
//   just once. Could be problematic for cases where an overflow might happen (detect this?). Same for POP.
// * Compile: LSHIFTR RSHIFTR

#include "allegro.h"
#include "base/qrs.h"
#include "zc/jit.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zelda.h"
#include "zconsole/ConsoleLogger.h"
#include <fmt/format.h>
#include <algorithm>
#include <memory>
#include <chrono>
#include <optional>
#include <array>
#include <filesystem>

typedef int32_t (*JittedFunction)(int32_t *registers, int32_t *global_registers,
								  int32_t *stack, uint32_t *stack_index, uint32_t *pc,
								  intptr_t *call_stack_rets, uint32_t *call_stack_ret_index,
								  uint32_t *wait_index);

static bool is_enabled;
static bool jit_log_enabled;
static std::map<script_id, JittedFunction> compiled_functions;

void jit_printf(const char *format, ...)
{
	if (!jit_log_enabled)
		return;

	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	al_trace("%s", buffer);
}

#ifdef ZC_JIT
#include <asmjit/asmjit.h>
#include <thread>

using namespace asmjit;

enum class ThreadState
{
	Inactive,
	Starting,
	Running,
	Stopping,
	Done,
};

struct ThreadInfo
{
	int id;
	ThreadState state = ThreadState::Inactive;
	ALLEGRO_THREAD* thread;
};

enum class TaskState
{
	Pending,
	Active,
	Done,
};

static const int MAX_THREADS = 16;
static std::array<ThreadInfo, MAX_THREADS> thread_infos;
static std::map<script_id, TaskState> task_states;
static std::vector<script_id> active_tasks;
static std::vector<script_data*> pending_scripts;
static ALLEGRO_MUTEX* tasks_mutex;
static ALLEGRO_COND* tasks_cond;
static ALLEGRO_COND* task_finish_cond;

static JittedFunction compile_script(script_data *script);

// Returns a JittedFunction. If already compiled, returns the cached reference.
// If a thread is currently compiling this script, waits for that thread to finish.
// Otherwise compile the script on this thread.
// If null is returned, the script failed to compile.
static JittedFunction compile_if_needed(script_data *script)
{
	JittedFunction fn;

	// First check if script is already compiled.
	al_lock_mutex(tasks_mutex);
	auto it = compiled_functions.find(script->id);
	if (it != compiled_functions.end())
	{
		fn = compiled_functions.at(script->id);
		al_unlock_mutex(tasks_mutex);
		return fn;
	}

	// Next check if a thread is currently compiling this script.
	if (task_states[script->id] == TaskState::Active)
	{
		// Wait for task to finish.
		jit_printf("jit: [*] waiting for thread to compile script type: %s index: %d name: %s\n", ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str());
		while (compiled_functions.find(script->id) == compiled_functions.end())
		{
			al_wait_cond(task_finish_cond, tasks_mutex);
		}

		fn = compiled_functions[script->id];
		al_unlock_mutex(tasks_mutex);
		return fn;
	}

	// Finally, just compile the script here. No need to keep lock during this.
	auto pending_it = std::find(pending_scripts.begin(), pending_scripts.end(), script);
	if (pending_it != pending_scripts.end())
		pending_scripts.erase(pending_it);
	al_unlock_mutex(tasks_mutex);

	jit_printf("jit: [*] compiling script type: %s index: %d name: %s\n", ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str());
	fn = compile_script(script);

	al_lock_mutex(tasks_mutex);
	compiled_functions[script->id] = fn;
	al_unlock_mutex(tasks_mutex);

	return fn;
}

static int thread_pool_generation_count;

static void * compile_script_proc(ALLEGRO_THREAD *thread, void *arg)
{
	ThreadInfo* thread_info = (ThreadInfo*)arg;
	int id = thread_info->id;
	jit_printf("jit: [%d] thread started\n", id);

	al_lock_mutex(tasks_mutex);
	thread_info->state = ThreadState::Running;
	while (true)
	{
		while (!(al_get_thread_should_stop(thread) || pending_scripts.size()))
		{
			al_wait_cond(tasks_cond, tasks_mutex);
		}

		if (al_get_thread_should_stop(thread))
		{
			break;
		}

		int generation = thread_pool_generation_count;
		auto script = pending_scripts.back();
		pending_scripts.pop_back();
		task_states[script->id] = TaskState::Active;
		active_tasks.push_back(script->id);
		al_unlock_mutex(tasks_mutex);

		jit_printf("jit: [%d] compiling script type: %s index: %d name: %s\n", id, ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str());
		auto fn = compile_script(script);

		al_lock_mutex(tasks_mutex);
		if (auto it = std::find(active_tasks.begin(), active_tasks.end(), script->id); it != active_tasks.end())
			active_tasks.erase(it);
		if (thread_pool_generation_count != generation)
		{
			// This task is now useless, since a new quest was loaded.
			continue;
		}
		task_states[script->id] = TaskState::Done;
		compiled_functions[script->id] = fn;
		// This is what signals the main thread that this script is ready.
		al_broadcast_cond(task_finish_cond);
		// Go back to top of loop, still with the mutex locked.
	}

	thread_info->state = ThreadState::Done;
	al_unlock_mutex(tasks_mutex);
	jit_printf("jit: [%d] done\n", id);
	return nullptr;
}

static void create_compile_tasks(script_data *scripts[], size_t start, size_t max, ScriptType type)
{
	for (size_t i = start; i < max; i++)
	{
		auto script = scripts[i];
		if (script && script->valid() && !compiled_functions.contains({type, (int)i}))
		{
			pending_scripts.push_back(script);
		}
	}
}

static void create_compile_tasks(script_data *scripts[], size_t len, ScriptType type)
{
	create_compile_tasks(scripts, 0, len, type);
}

static bool set_compilation_thread_pool_size(int target_size)
{
	if (target_size > MAX_THREADS)
		target_size = MAX_THREADS;

	al_lock_mutex(tasks_mutex);
	int num_active_threads = 0;
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting)
			num_active_threads += 1;
	}
	al_unlock_mutex(tasks_mutex);

	int delta = target_size - num_active_threads;
	if (delta == 0)
		return true;

	jit_printf("jit: changing from %d to %d threads\n", num_active_threads, target_size);

	if (delta > 0)
	{
		for (auto& thread_info : thread_infos)
		{
			if (delta == 0)
				break;
			if (thread_info.state != ThreadState::Inactive)
				continue;

			ALLEGRO_THREAD *t = al_create_thread(compile_script_proc, &thread_info);
			if (!t)
			{
				return false;
			}
			thread_info.thread = t;
			thread_info.state = ThreadState::Starting;
			al_start_thread(t);
			delta--;
		}
	}
	else
	{
		al_lock_mutex(tasks_mutex);

		for (auto& thread_info : thread_infos)
		{
			if (delta == 0)
				break;
			if (!(thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting))
				continue;

			al_set_thread_should_stop(thread_info.thread);
			thread_info.state = ThreadState::Stopping;
			++delta;
		}

		al_broadcast_cond(tasks_cond);
		al_unlock_mutex(tasks_mutex);
	}

	return true;
}

static void create_compile_tasks()
{
	al_lock_mutex(tasks_mutex);
	task_states.clear();
	active_tasks.clear();
	pending_scripts.clear();
	create_compile_tasks(ffscripts, NUMSCRIPTFFC, ScriptType::FFC);
	create_compile_tasks(itemscripts, NUMSCRIPTITEM, ScriptType::Item);
	create_compile_tasks(guyscripts, NUMSCRIPTGUYS, ScriptType::NPC);
	create_compile_tasks(screenscripts, NUMSCRIPTSCREEN, ScriptType::Screen);
	create_compile_tasks(lwpnscripts, NUMSCRIPTWEAPONS, ScriptType::Lwpn);
	create_compile_tasks(ewpnscripts, NUMSCRIPTWEAPONS, ScriptType::Ewpn);
	create_compile_tasks(dmapscripts, NUMSCRIPTSDMAP, ScriptType::DMap);
	create_compile_tasks(itemspritescripts, NUMSCRIPTSITEMSPRITE, ScriptType::ItemSprite);
	create_compile_tasks(comboscripts, NUMSCRIPTSCOMBODATA, ScriptType::Combo);
	create_compile_tasks(genericscripts, NUMSCRIPTSGENERIC, ScriptType::Generic);
	create_compile_tasks(subscreenscripts, NUMSCRIPTSSUBSCREEN, ScriptType::EngineSubscreen);
	create_compile_tasks(globalscripts, GLOBAL_SCRIPT_GAME+1, NUMSCRIPTGLOBAL, ScriptType::Global);
	// Sort by # of commands, so that biggest scripts get compiled first.
	std::sort(pending_scripts.begin(), pending_scripts.end(), [](script_data* a, script_data* b) {
		return a->size < b->size;
	});
	// Make sure player and global scripts (just the INIT and GAME ones) are compiled first, as they
	// are needed on frame 1.
	create_compile_tasks(playerscripts, NUMSCRIPTPLAYER, ScriptType::Player);
	create_compile_tasks(globalscripts, GLOBAL_SCRIPT_GAME, NUMSCRIPTGLOBAL, ScriptType::Global);
	if (jit_log_enabled)
	{
		for (auto a : pending_scripts) 
		{
			jit_printf("jit: %d: %d\n", a->id, (int)a->size);
		}
	}

	al_broadcast_cond(tasks_cond);
	al_unlock_mutex(tasks_mutex);
}

static JitRuntime rt;

struct CompilationState
{
	// Some globals to prevent passing around everywhere
	size_t size;
	x86::Gp vRetVal;
	x86::Gp vSwitchKey;
	Label L_End;
	// Registers for the compiled function parameters.
	x86::Gp ptrRegisters;
	x86::Gp ptrGlobalRegisters;
	x86::Gp ptrStack;
	x86::Gp ptrStackIndex;
	x86::Gp ptrPc;
	x86::Gp ptrCallStackRets;
	x86::Gp ptrCallStackRetIndex;
	x86::Gp ptrWaitIndex;
};
#endif

struct JittedScriptHandle
{
	JittedFunction fn;
	script_data *script;
	refInfo *ri;
	intptr_t call_stack_rets[100];
	uint32_t call_stack_ret_index;
};

extern ScriptDebugHandle* runtime_script_debug_handle;

static void print(int32_t n)
{
	if (runtime_script_debug_handle)
		runtime_script_debug_handle->printf(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
							CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
						"JIT: %d\n", n);
}

static void print64(int64_t n)
{
	if (runtime_script_debug_handle)
		runtime_script_debug_handle->printf(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
							CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
						"JIT: %ld\n", n);
}

static void debug_pre_command(int32_t pc, uint16_t sp)
{
	extern refInfo *ri;

	ri->pc = pc;
	ri->sp = sp;
	if (runtime_script_debug_handle)
		runtime_script_debug_handle->pre_command();
}

void set_register(int32_t arg, int32_t value);

#ifdef ZC_JIT

class MyErrorHandler : public ErrorHandler
{
public:
	void handleError(Error err, const char *message, BaseEmitter *origin) override
	{
		al_trace("AsmJit error: %s\n", message);
	}
};

static x86::Gp get_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r)
{
	x86::Gp val = cc.newInt32();
	if (r >= D(0) && r <= A(1))
	{
		cc.mov(val, x86::ptr_32(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.mov(val, x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.mov(val, vStackIndex);
		cc.imul(val, 10000);
	}
	else if (r == SWITCHKEY)
	{
		cc.mov(val, state.vSwitchKey);
	}
	else
	{
		// Call external get_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(CallConvId::kHost));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val);
	}
	return val;
}

static x86::Gp get_z_register_64(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r)
{
	x86::Gp val = cc.newInt64();
	if (r >= D(0) && r <= A(1))
	{
		cc.movsxd(val, x86::ptr_32(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.movsxd(val, x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.movsxd(val, vStackIndex);
		cc.imul(val, 10000);
	}
	else if (r == SWITCHKEY)
	{
		cc.movsxd(val, state.vSwitchKey);
	}
	else
	{
		// Call external get_register.
		x86::Gp val32 = cc.newInt32();
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(CallConvId::kHost));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val32);
		cc.movsxd(val, val32);
	}
	return val;
}

template <typename T>
static void set_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r, T val)
{
	if (r >= D(0) && r <= A(1))
	{
		cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), val);
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.mov(x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4), val);
	}
	else if (r == SP)
	{
		// TODO
		Z_error_fatal("Unimplemented: set SP");
	}
	else if (r == SWITCHKEY)
	{
		state.vSwitchKey = cc.newInt32();
		cc.mov(state.vSwitchKey, val);
	}
	else
	{
		// Call external set_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_register, FuncSignatureT<void, int32_t, int32_t>(CallConvId::kHost));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
	}
}

static void set_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r, x86::Mem mem)
{
	x86::Gp val = cc.newInt32();
	cc.mov(val, mem);
	set_z_register(state, cc, vStackIndex, r, val);
}

static void modify_sp(x86::Compiler &cc, x86::Gp vStackIndex, int delta)
{
	cc.add(vStackIndex, delta);
	cc.and_(vStackIndex, MASK_SP);
}

static void div_10000(x86::Compiler &cc, x86::Gp dividend)
{
	// Perform division by invariant multiplication.
	// https://clang.godbolt.org/z/c4qG3s9nW
	if (dividend.isType(RegType::kGp64))
	{
		x86::Gp input = cc.newInt64();
		cc.mov(input, dividend);

		x86::Gp r = cc.newInt64();
		cc.movabs(r, 3777893186295716171);
		cc.imul(r, r, dividend);
		cc.sar(r, 11);

		x86::Gp b = cc.newInt64();
		cc.mov(b, input);
		cc.sar(b, 63);
		cc.sub(r, b);

		cc.mov(dividend, r);
	}
	else if (dividend.isType(RegType::kGp32))
	{
		x86::Gp r = cc.newInt64();
		cc.movsxd(r, dividend);
		cc.sar(dividend, 31);
		cc.imul(r, r, 1759218605);
		cc.sar(r, 44);

		cc.sub(r.r32(), dividend);
		cc.mov(dividend, r.r32());
	}
	else
	{
		ASSERT(false);
		abort();
	}
}

static void zero(x86::Compiler &cc, x86::Gp reg)
{
	cc.xor_(reg, reg);
}

static void compile_compare(CompilationState& state, x86::Compiler &cc, std::map<int, Label> &goto_labels, x86::Gp vStackIndex, int command, int arg)
{
	x86::Gp val = cc.newInt32();

	if (command == GOTOTRUE)
	{
		cc.je(goto_labels.at(arg));
	}
	else if (command == GOTOFALSE)
	{
		cc.jne(goto_labels.at(arg));
	}
	else if (command == GOTOMORE)
	{
		cc.jge(goto_labels.at(arg));
	}
	else if (command == GOTOLESS)
	{
		if (get_qr(qr_GOTOLESSNOTEQUAL))
			cc.jle(goto_labels.at(arg));
		else
			cc.jl(goto_labels.at(arg));
	}
	else if (command == SETTRUE)
	{
		cc.mov(val, 0);
		cc.sete(val);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETTRUEI)
	{
		// https://stackoverflow.com/a/45183084/2788187
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmove(val, val2);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETFALSE)
	{
		cc.mov(val, 0);
		cc.setne(val);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETFALSEI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovne(val, val2);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETMOREI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovge(val, val2);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETLESSI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovle(val, val2);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETMORE)
	{
		cc.mov(val, 0);
		cc.setge(val);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else if (command == SETLESS)
	{
		cc.mov(val, 0);
		cc.setle(val);
		set_z_register(state, cc, vStackIndex, arg, val);
	}
	else
	{
		Z_error_fatal("Unimplemented: %s", script_debug_command_to_string(command, arg, 0).c_str());
	}
}

// Defer to the ZASM command interpreter for 1+ commands.
static void compile_command(CompilationState& state, x86::Compiler &cc, script_data *script, int i, int count, x86::Gp vStackIndex)
{
	extern int32_t jitted_uncompiled_command_count;

	x86::Gp reg = cc.newIntPtr();
	cc.mov(reg, (uint64_t)&jitted_uncompiled_command_count);
	cc.mov(x86::ptr_32(reg), count);

	cc.mov(x86::ptr_32(state.ptrPc), i);
	cc.mov(x86::ptr_32(state.ptrStackIndex), vStackIndex);

	InvokeNode *invokeNode;
	cc.invoke(&invokeNode, run_script_int, FuncSignatureT<int32_t, bool>(CallConvId::kHost));
	invokeNode->setArg(0, true);

	bool could_return_not_ok = false;
	for (int j = 0; j < count; j++)
	{
		int index = i + j;
		if (index >= state.size)
			break;

		if (command_could_return_not_ok(script->zasm[index].command))
		{
			could_return_not_ok = true;
			break;
		}
	}

	if (could_return_not_ok)
	{
		invokeNode->setRet(0, state.vRetVal);
		cc.cmp(state.vRetVal, RUNSCRIPT_OK);
		cc.jne(state.L_End);
	}
}

static bool command_is_compiled(int command)
{
	if (command_is_wait(command))
		return true;

	if (command_uses_comparison_result(command))
		return true;

	switch (command)
	{
	// These commands are critical to control flow.
	case COMPARER:
	case COMPAREV:
	case GOTO:
	case GOTOR:
	case QUIT:
	case RETURN:

	// These commands modify the stack pointer, which is just a local copy. If these commands
	// were not compiled, then vStackIndex would have to be restored in compile_command.
	case POP:
	case POPARGS:
	case PUSHR:
	case PUSHV:

	// These can be commented out to instead run interpreted. Useful for
	// singling out problematic instructions.
	case ABSR:
	case ADDR:
	case ADDV:
	case ANDR:
	case ANDV:
	case CASTBOOLF:
	case CASTBOOLI:
	case CEILING:
	case DIVR:
	case DIVV:
	case FLOOR:
	case LOADD:
	case LOADI:
	case MAXR:
	case MAXV:
	case MINR:
	case MINV:
	case MODR:
	case MODV:
	case MULTR:
	case MULTV:
	case NOP:
	case SETR:
	case SETV:
	case STORED:
	case STOREI:
	case SUBR:
	case SUBV:
	case SUBV2:
		return true;
	}

	return false;
}

static void error(ScriptDebugHandle* debug_handle, script_data *script, std::string str)
{
	str = fmt::format("failed to compile type: {} index: {} name: {}\nerror: {}\n",
					  ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str(), str);

	al_trace("%s", str.c_str());
	if (debug_handle)
		debug_handle->print(CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK, str.c_str());

	if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL)
	{
		ASSERT(false);
		abort();
	}
}

// Compile the entire ZASM script at once, into a single function.
static JittedFunction compile_script(script_data *script)
{
	CompilationState state;
	state.size = script->size;
	size_t size = state.size;

	al_trace("[jit] compiling script type: %s index: %d size: %zu name: %s\n", ScriptTypeToString(script->id.type), script->id.index, size, script->meta.script_name.c_str());

	if (size <= 1)
		return nullptr;

	// Check if script is like, really big.
	// Anything over 20,000 takes ~5s, which is an unacceptable delay. Until scripts can be compiled w/o pausing execution of the engine,
	// or a way to speed up compilation is found, set a hard limit on script size.
	size_t limit = 20000;
	if (!is_ci() && size >= limit)
	{
		al_trace("[jit] script type %s index %d too large to compile quickly, skipping\n", ScriptTypeToString(script->id.type), script->id.index);
		return nullptr;
	}

	std::optional<ScriptDebugHandle> debug_handle_ = std::nullopt;
	if (DEBUG_JIT_PRINT_ASM)
	{
		debug_handle_.emplace(ScriptDebugHandle::OutputSplit::ByScript, script);
	}
	auto debug_handle = debug_handle_ ? std::addressof(debug_handle_.value()) : nullptr;

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	bool runtime_debugging = script_debug_is_runtime_debugging() == 2;

	// Verify an assumption that comparison commands always happen in groups.
	{
		bool comparing_state = false;
		for (size_t i = 0; i < size; i++)
		{
			int command = script->zasm[i].command;

			if (command == COMPARER || command == COMPAREV)
			{
				comparing_state = true;
			}
			else if (comparing_state)
			{
				if (!command_uses_comparison_result(command))
					comparing_state = false;
			}
			else if (command_uses_comparison_result(command))
			{
				error(debug_handle, script, fmt::format("comparison assumption failed! i: {}", i));
				return nullptr;
			}
		}
	}

	CodeHolder code;
	JittedFunction fn;

	code.init(rt.environment());
	MyErrorHandler myErrorHandler;
	code.setErrorHandler(&myErrorHandler);

	StringLogger logger;
	if (DEBUG_JIT_PRINT_ASM)
	{
		code.setLogger(&logger);
	}

	x86::Compiler cc(&code);
	// cc.addDiagnosticOptions(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);

	// Setup parameters.
	cc.addFunc(FuncSignatureT<int32_t, int32_t *, int32_t *, int32_t *, uint16_t *, uint32_t *, intptr_t *, uint32_t *, uint32_t *>(CallConvId::kHost));
	state.ptrRegisters = cc.newIntPtr("registers_ptr");
	state.ptrGlobalRegisters = cc.newIntPtr("global_registers_ptr");
	state.ptrStack = cc.newIntPtr("stack_ptr");
	state.ptrStackIndex = cc.newIntPtr("stack_index_ptr");
	state.ptrPc = cc.newIntPtr("pc_ptr");
	state.ptrCallStackRets = cc.newIntPtr("call_stack_rets_ptr");
	state.ptrCallStackRetIndex = cc.newIntPtr("call_stack_ret_index_ptr");
	state.ptrWaitIndex = cc.newIntPtr("wait_index_ptr");
	cc.setArg(0, state.ptrRegisters);
	cc.setArg(1, state.ptrGlobalRegisters);
	cc.setArg(2, state.ptrStack);
	cc.setArg(3, state.ptrStackIndex);
	cc.setArg(4, state.ptrPc);
	cc.setArg(5, state.ptrCallStackRets);
	cc.setArg(6, state.ptrCallStackRetIndex);
	cc.setArg(7, state.ptrWaitIndex);

	state.vRetVal = cc.newInt32("return_val");
	zero(cc, state.vRetVal); // RUNSCRIPT_OK

	x86::Gp vStackIndex = cc.newUInt32("stack_index");
	cc.mov(vStackIndex, x86::ptr_32(state.ptrStackIndex));

	x86::Gp vCallStackRetIndex = cc.newUInt32("call_stack_ret_index");
	cc.mov(vCallStackRetIndex, x86::ptr_32(state.ptrCallStackRetIndex));

	// Scripts yield on calls to WaitX, which means the function
	// this returns needs to take a parameter that represents which WaitX to jump to on re-entry.
	// Each WaitX instruction is a unique label. We use a jump table for this.
	// If jumpto is 0, that starts from the beginning of the entrypoint function.
	// If jumpto is >0, it uses the nth "WaitX" label.
	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("re-entry jump table");

	std::vector<Label> wait_frame_labels;
	Label L_Table = cc.newLabel();
	Label L_Start = cc.newLabel();
	state.L_End = cc.newLabel();
	JumpAnnotation *annotation = cc.newJumpAnnotation();
	annotation->addLabel(L_Start);
	for (size_t i = 0; i < size; i++)
	{
		int command = script->zasm[i].command;
		if (!command_is_wait(command))
			continue;

		Label label = cc.newLabel();
		wait_frame_labels.push_back(label);
		annotation->addLabel(label);
	}

	x86::Gp target = cc.newIntPtr("target");
	x86::Gp offset = cc.newIntPtr("offset");
	x86::Gp wait_index = cc.newUInt32("wait_index");
	cc.mov(wait_index, x86::ptr_32(state.ptrWaitIndex));
	cc.lea(offset, x86::ptr(L_Table));
	if (cc.is64Bit())
		cc.movsxd(target, x86::ptr_32(offset, wait_index.cloneAs(offset), 2));
	else
		cc.mov(target, x86::ptr_32(offset, wait_index.cloneAs(offset), 2));
	cc.add(target, offset);

	// Find all GOTOs.
	std::map<int, Label> goto_labels;
	for (size_t i = 0; i < size; i++)
	{
		int command = script->zasm[i].command;
		if (command != GOTO && command != GOTOTRUE && command != GOTOFALSE && command != GOTOMORE && command != GOTOLESS)
			continue;

		goto_labels[script->zasm[i].arg1] = cc.newLabel();
	}

	// Find all function calls.
	std::map<int, Label> call_pc_to_return_label;
	std::set<int> function_calls;
	std::set<int> function_calls_goto_pc;
	for (size_t i = 1; i < size; i++)
	{
		int command = script->zasm[i].command;
		int prev_command = script->zasm[i - 1].command;
		if (command != GOTO) continue;

		bool is_function_call_like =
			// Typical function calls push the return address to the stack just before the GOTO.
			prev_command == PUSHR || prev_command == PUSHV ||
			// Class construction function calls do `SETR CLASS_THISKEY, D2` just before its GOTO.
			(prev_command == SETR && script->zasm[i - 1].arg1 == CLASS_THISKEY);
		if (is_function_call_like)
		{
			call_pc_to_return_label[i] = cc.newLabel();
			function_calls.insert(i);
			function_calls_goto_pc.insert(script->zasm[i].arg1);
		}
	}

	std::vector<int> function_start_pcs;
	std::map<int, int> start_pc_to_function;
	std::vector<JumpAnnotation *> function_jump_annotations;
	{
		int next_fn_id = 0;
		for (int function_start_pc : function_calls_goto_pc)
		{
			function_start_pcs.push_back(function_start_pc);
			start_pc_to_function[function_start_pc] = next_fn_id++;
			function_jump_annotations.push_back(cc.newJumpAnnotation());
		}
	}

	// Map all RETURN to the enclosing function.
	std::map<int, int> return_to_function_id;
	{
		int cur_function_id = 0;
		for (size_t i = 0; i < size; i++)
		{
			if (function_start_pcs.size() > cur_function_id + 1 && function_start_pcs.at(cur_function_id + 1) == i)
				cur_function_id += 1;

			int command = script->zasm[i].command;
			if (command == RETURN || (command == GOTOR && script->zasm[i - 1].command == POP))
			{
				return_to_function_id[i] = cur_function_id;
			}
		}
	}

	// Annotate all function RETURNs to their calls, to help asmjit with liveness analysis.
	for (int function_call_pc : function_calls)
	{
		int goto_pc = script->zasm[function_call_pc].arg1;
		auto it = start_pc_to_function.find(goto_pc);
		int function_index = it->second;
		function_jump_annotations[function_index]->addLabel(call_pc_to_return_label.at(function_call_pc));
	}

	cc.jmp(target, annotation);
	cc.bind(L_Start);

	// Next, transform each ZASM command to the equivalent assembly.
	size_t label_index = 0;

	// cc.setInlineComment does not make a copy of the string, so we need to keep
	// comment strings around a bit longer than the invocation.
	std::string comment;

	std::map<int, int> uncompiled_command_counts;

	for (size_t i = 0; i < size; i++)
	{
		int command = script->zasm[i].command;
		int arg1 = script->zasm[i].arg1;
		int arg2 = script->zasm[i].arg2;

		if (goto_labels.find(i) != goto_labels.end())
		{
			cc.bind(goto_labels.at(i));
		}

		if (DEBUG_JIT_PRINT_ASM && start_pc_to_function.find(i) != start_pc_to_function.end())
		{
			cc.setInlineComment((comment = fmt::format("function {}", start_pc_to_function.at(i))).c_str());
			cc.nop();
		}

		if (DEBUG_JIT_PRINT_ASM)
		{
			cc.setInlineComment((comment = fmt::format("{} {}", i, script_debug_command_to_string(command, arg1, arg2))).c_str());
		}

		// Can be useful for debugging.
		// {
		// 	InvokeNode* invokeNode;
		// 	cc.invoke(&invokeNode, print, FuncSignatureT<void, int32_t>(CallConvId::kHost));
		// 	invokeNode->setArg(0, i); // or any int32 register
		// }

		// We can't invoke functions between COMPARE and the instructions that use the comparison result,
		// because that would destroy EFLAGS. And asmjit compiler has no way to save the EFLAGS because we
		// can't use stack-modifying instructions like pushfq. So we must skip the debug printout for these
		// instructions, which results in them being grouped together in the output and the stack/register
		// trace being printed just once for the entire group of instructions.
		if (runtime_debugging && !command_uses_comparison_result(command))
		{
			InvokeNode *invokeNode;
			cc.invoke(&invokeNode, debug_pre_command, FuncSignatureT<void, int32_t, uint16_t>(CallConvId::kHost));
			invokeNode->setArg(0, i);
			invokeNode->setArg(1, vStackIndex);
		}

		if (command_uses_comparison_result(command))
		{
			compile_compare(state, cc, goto_labels, vStackIndex, command, arg1);
			continue;
		}

		if (command_is_wait(command))
		{
			compile_command(state, cc, script, i, 1, vStackIndex);
			cc.mov(x86::ptr_32(state.ptrWaitIndex), label_index + 1);
			cc.jmp(state.L_End);
			cc.bind(wait_frame_labels[label_index]);
			label_index += 1;
			continue;
		}

		if (!command_is_compiled(command))
		{
			if (DEBUG_JIT_PRINT_ASM && command != 0xFFFF)
				uncompiled_command_counts[command]++;

			if (DEBUG_JIT_PRINT_ASM)
			{
				std::string command_str =
					script_debug_command_to_string(command, arg1, arg2);
				cc.setInlineComment((comment = fmt::format("{} {}", i, command_str)).c_str());
				cc.nop();
			}

			// Every command that is not compiled to assembly must go through the regular interpreter function.
			// In order to reduce function call overhead, we call into the interpreter function in batches.
			int uncompiled_command_count = 1;
			for (int j = i + 1; j < size; j++)
			{
				if (command_is_compiled(script->zasm[j].command))
					break;
				if (goto_labels.find(j) != goto_labels.end())
					break;
				if (start_pc_to_function.find(j) != start_pc_to_function.end())
					break;

				if (DEBUG_JIT_PRINT_ASM && script->zasm[j].command != 0xFFFF)
					uncompiled_command_counts[script->zasm[j].command]++;

				uncompiled_command_count += 1;
				if (DEBUG_JIT_PRINT_ASM)
				{
					std::string command_str =
						script_debug_command_to_string(script->zasm[j].command, script->zasm[j].arg1, script->zasm[j].arg2);
					cc.setInlineComment((comment = fmt::format("{} {}", j, command_str)).c_str());
					cc.nop();
				}
			}

			compile_command(state, cc, script, i, uncompiled_command_count, vStackIndex);
			i += uncompiled_command_count - 1;
			continue;
		}

		// Every command here must be reflected in command_is_compiled!
		switch (command)
		{
		case NOP:
			if (DEBUG_JIT_PRINT_ASM)
				cc.nop();
			break;
		case QUIT:
		{
			compile_command(state, cc, script, i, 1, vStackIndex);
			cc.mov(x86::ptr_32(state.ptrWaitIndex), 0);
			cc.jmp(state.L_End);
		}
		break;
		case GOTO:
		{
			if (function_calls.find(i) != function_calls.end())
			{
				// https://github.com/asmjit/asmjit/issues/286
				x86::Gp address = cc.newIntPtr();
				cc.lea(address, x86::qword_ptr(call_pc_to_return_label.at(i)));
				cc.mov(x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3), address);
				cc.add(vCallStackRetIndex, 1);
				cc.jmp(goto_labels.at(arg1));
				cc.bind(call_pc_to_return_label.at(i));
			}
			else
			{
				cc.jmp(goto_labels.at(arg1));
			}
		}
		break;
		case RETURN:
		{
			// Note: the return pc is on the stack, but we just ignore it and instead use
			// the function call return label.
			modify_sp(cc, vStackIndex, 1);

			cc.sub(vCallStackRetIndex, 1);
			x86::Gp address = cc.newIntPtr();
			cc.mov(address, x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3));

			int function_index = return_to_function_id.at(i);
			if (function_jump_annotations.size() <= function_index)
			{
				error(debug_handle, script, fmt::format("failed to resolve function return! i: {} function_index: {}", i, function_index));
				return nullptr;
			}
			cc.jmp(address, function_jump_annotations[function_index]);
		}
		break;
		case GOTOR:
		{
			// This is pretty much RETURN - was only used to return to the call location in scripts
			// compiled before RETURN existed.
			// Note: the return pc is in a register, but we just ignore it and instead use
			// the function call return label.
			cc.sub(vCallStackRetIndex, 1);
			x86::Gp address = cc.newIntPtr();
			cc.mov(address, x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3));

			int function_index = return_to_function_id.at(i);
			if (function_jump_annotations.size() <= function_index)
			{
				error(debug_handle, script, fmt::format("failed to resolve function return! i: {} function_index: {}", i, function_index));
				return nullptr;
			}
			cc.jmp(address, function_jump_annotations[function_index]);
		}
		break;
		case PUSHV:
		{
			modify_sp(cc, vStackIndex, -1);
			cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), arg1);
		}
		break;
		case PUSHR:
		{
			// Grab value from register and push onto stack.
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			modify_sp(cc, vStackIndex, -1);
			cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), val);
		}
		break;
		case SETV:
		{
			// Set register to immediate value.
			set_z_register(state, cc, vStackIndex, arg1, arg2);
		}
		break;
		case SETR:
		{
			// Set register arg1 to value of register arg2.
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case LOADD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, arg2);
			cc.add(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			div_10000(cc, offset);

			set_z_register(state, cc, vStackIndex, arg1, x86::ptr_32(state.ptrStack, offset, 2));
		}
		break;
		case LOADI:
		{
			// Set register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, vStackIndex, arg2);
			div_10000(cc, offset);

			set_z_register(state, cc, vStackIndex, arg1, x86::ptr_32(state.ptrStack, offset, 2));
		}
		break;
		case STORED:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, arg2);
			cc.add(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			div_10000(cc, offset);

			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), val);
		}
		break;
		case STOREI:
		{
			// Write from register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, vStackIndex, arg2);
			div_10000(cc, offset);

			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), val);
		}
		break;
		case POP:
		{
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStack, vStackIndex, 2));
			modify_sp(cc, vStackIndex, 1);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case POPARGS:
		{
			// int32_t num = sarg2;
			// ri->sp += num;
			modify_sp(cc, vStackIndex, arg2);

			// word read = (ri->sp-1) & MASK_SP;
			x86::Gp read = cc.newInt32();
			cc.mov(read, vStackIndex);
			cc.sub(read, 1);
			cc.and_(read, MASK_SP);

			// int32_t value = SH::read_stack(read);
			// set_register(sarg1, value);
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStack, read));
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ABSR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp y = cc.newInt32();
			cc.mov(y, val);
			cc.sar(y, 31);
			cc.xor_(val, y);
			cc.sub(val, y);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CASTBOOLI:
		{
			// https://clang.godbolt.org/z/W8PM4j33b
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.neg(val);
			cc.sbb(val, val);
			cc.and_(val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CASTBOOLF:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp result = cc.newInt32();
			zero(cc, result);
			cc.test(val, val);
			cc.setne(result.r8());
			set_z_register(state, cc, vStackIndex, arg1, result);
		}
		break;
		case ADDV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.add(val, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ADDR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.add(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ANDV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);

			div_10000(cc, val);
			cc.and_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ANDR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);

			div_10000(cc, val);
			div_10000(cc, val2);
			cc.and_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MAXR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MAXV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MINR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MINV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MODV:
		{
			if (arg2 == 0)
			{
				x86::Gp val = cc.newInt32();
				zero(cc, val);
				set_z_register(state, cc, vStackIndex, arg1, val);
				continue;
			}

			// https://stackoverflow.com/a/8022107/2788187
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			if (arg2 > 0 && (arg2 & (-arg2)) == arg2)
			{
				// Power of 2.
				// Because numbers in zscript are fixed point, "2" is really "20000"... so this won't
				// ever really be utilized.
				cc.and_(val, arg2 - 1);
				set_z_register(state, cc, vStackIndex, arg1, val);
			}
			else
			{
				x86::Gp divisor = cc.newInt32();
				cc.mov(divisor, arg2);
				x86::Gp rem = cc.newInt32();
				zero(cc, rem);
				cc.cdq(rem, val);
				cc.idiv(rem, val, divisor);
				set_z_register(state, cc, vStackIndex, arg1, rem);
			}
		}
		break;
		case MODR:
		{
			x86::Gp dividend = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp divisor = get_z_register(state, cc, vStackIndex, arg2);

			Label do_set_register = cc.newLabel();

			x86::Gp rem = cc.newInt32();
			zero(cc, rem);

			// Prevent division by zero. Result will be zero.
			cc.test(divisor, divisor);
			cc.jz(do_set_register);

			cc.cdq(rem, dividend);
			cc.idiv(rem, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, vStackIndex, arg1, rem);
		}
		break;
		case SUBV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.sub(val, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case SUBR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.sub(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case SUBV2:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			x86::Gp result = cc.newInt32();
			cc.mov(result, arg1);
			cc.sub(result, val);
			set_z_register(state, cc, vStackIndex, arg2, result);
		}
		break;
		case MULTV:
		{
			x86::Gp val = get_z_register_64(state, cc, vStackIndex, arg1);
			cc.imul(val, arg2);
			div_10000(cc, val);
			set_z_register(state, cc, vStackIndex, arg1, val.r32());
		}
		break;
		case MULTR:
		{
			x86::Gp val = get_z_register_64(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register_64(state, cc, vStackIndex, arg2);
			cc.imul(val, val2);
			div_10000(cc, val);
			set_z_register(state, cc, vStackIndex, arg1, val.r32());
		}
		break;
		// TODO guard for div by zero
		case DIVV:
		{
			x86::Gp dividend = get_z_register_64(state, cc, vStackIndex, arg1);
			int val2 = arg2;

			cc.imul(dividend, 10000);
			x86::Gp divisor = cc.newInt64();
			cc.mov(divisor, val2);
			x86::Gp dummy = cc.newInt64();
			zero(cc, dummy);
			cc.cqo(dummy, dividend);
			cc.idiv(dummy, dividend, divisor);

			set_z_register(state, cc, vStackIndex, arg1, dividend.r32());
		}
		break;
		case DIVR:
		{
			x86::Gp dividend = get_z_register_64(state, cc, vStackIndex, arg1);
			x86::Gp divisor = get_z_register_64(state, cc, vStackIndex, arg2);

			Label do_division = cc.newLabel();
			Label do_set_register = cc.newLabel();

			// If zero, result is (sign(dividend) * MAX_SIGNED_32)
			// TODO how expensive is this check? Maybe we can add a new QR that makes div-by-zero an error
			// and omit these safeguards.
			cc.test(divisor, divisor);
			cc.jnz(do_division);
			x86::Gp sign = cc.newInt64();
			cc.mov(sign, dividend);
			cc.sar(sign, 63);
			cc.or_(sign, 1);
			cc.mov(dividend.r32(), sign.r32());
			cc.imul(dividend.r32(), MAX_SIGNED_32);
			cc.jmp(do_set_register);

			// Else do the actual division.
			cc.bind(do_division);
			cc.imul(dividend, 10000);
			x86::Gp dummy = cc.newInt64();
			zero(cc, dummy);
			cc.cqo(dummy, dividend);
			cc.idiv(dummy, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, vStackIndex, arg1, dividend.r32());
		}
		break;
		case COMPAREV:
		{
			int val = arg2;
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg1);
			cc.cmp(val2, val);
		}
		break;
		case COMPARER:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg1);
			cc.cmp(val2, val);
		}
		break;
		// https://gcc.godbolt.org/z/r9zq67bK1
		case FLOOR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 9);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CEILING:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 10);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		default:
		{
			error(debug_handle, script, fmt::format("unhandled command: {}", command));
			return nullptr;
		}
		}
	}

	if (DEBUG_JIT_PRINT_ASM)
	{
		cc.setInlineComment("end commands");
		cc.nop();
	}

	cc.bind(state.L_End);

	// Persist stack pointer.
	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("persist stack pointer");
	cc.mov(x86::ptr_32(state.ptrStackIndex), vStackIndex);

	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("persist call stack ret pointer");
	cc.mov(x86::ptr_32(state.ptrCallStackRetIndex), vCallStackRetIndex);

	cc.ret(state.vRetVal);
	cc.endFunc();

	// Relative int32_t offsets of `L_XXX - L_Table`.
	cc.bind(L_Table);
	cc.embedLabelDelta(L_Start, L_Table, 4);
	for (auto label : wait_frame_labels)
	{
		cc.embedLabelDelta(label, L_Table, 4);
	}

	for (auto &it : call_pc_to_return_label)
	{
		cc.embedLabelDelta(it.second, L_Table, 4);
	}

	end_time = std::chrono::steady_clock::now();
	int32_t preprocess_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	start_time = end_time;

	cc.finalize();
	rt.add(&fn, &code);

	end_time = std::chrono::steady_clock::now();
	int32_t compile_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	if (debug_handle)
	{
		debug_handle->printf("time to preprocess: %d ms\n", preprocess_ms);
		debug_handle->printf("time to compile:    %d ms\n", compile_ms);
		debug_handle->printf("ZASM instructions:  %zu\n", size);
		debug_handle->printf("Code size:          %d kb\n", code.codeSize() / 1024);
		debug_handle->print("\n");

		if (!uncompiled_command_counts.empty())
		{
			debug_handle->print("=== uncompiled commands:\n");
			for (auto &it : uncompiled_command_counts)
			{
				debug_handle->printf("%s: %d\n", script_debug_command_to_string(it.first).c_str(), it.second);
			}
			debug_handle->print("\n");
		}

		debug_handle->print(
			CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_INTENSITY |
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
			"\nasmjit log / assembly:\n\n");
		debug_handle->print(
			CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
			logger.data());
	}

	al_trace("[jit] finished script %s %d. time: %d ms\n", ScriptTypeToString(script->id.type), script->id.index, preprocess_ms + compile_ms);

	if (fn)
	{
		jit_printf("success\n");
	}
	else
	{
		error(debug_handle, script, "failed to compile");
		jit_printf("failure\n");
	}

	return fn;
}

#endif

bool jit_is_enabled()
{
	return is_enabled;
}

void jit_set_enabled(bool enabled)
{
	is_enabled = enabled;
}

JittedScriptHandle *jit_create_script_handle(script_data *script, refInfo *ri)
{
#ifndef ZC_JIT
	return nullptr;
#else
	JittedScriptHandle *jitted_script = new JittedScriptHandle;
	jit_reinit(jitted_script);
	jitted_script->script = script;
	jitted_script->ri = ri;
	jitted_script->fn = compile_if_needed(script);
	if (!jitted_script->fn)
	{
		delete jitted_script;
		return nullptr;
	}

	return jitted_script;
#endif
}

void jit_delete_script_handle(JittedScriptHandle *jitted_script)
{
	delete jitted_script;
}

int jit_run_script(JittedScriptHandle *jitted_script)
{
	extern int32_t(*stack)[MAX_SCRIPT_REGISTERS];

	return jitted_script->fn(
		jitted_script->ri->d, game->global_d,
		*stack, &jitted_script->ri->sp,
		&jitted_script->ri->pc,
		jitted_script->call_stack_rets, &jitted_script->call_stack_ret_index,
		&jitted_script->ri->wait_index);
}

void jit_reinit(JittedScriptHandle *jitted_script)
{
	jitted_script->call_stack_ret_index = 0;
}

void jit_startup()
{
	if (!is_enabled)
		return;

#ifdef ZC_JIT
	jit_log_enabled = zc_get_config("ZSCRIPT", "jit_log", false) || is_ci();
	bool precompile = zc_get_config("ZSCRIPT", "jit_precompile", false);
	int num_threads = zc_get_config("ZSCRIPT", "jit_threads", -2);

	auto processor_count = std::thread::hardware_concurrency();
	if (num_threads < 0)
		num_threads = std::max(1, (int)processor_count / -num_threads);

	for (int i = 0; i < thread_infos.size(); i++)
	{
		thread_infos[i].id = i;
	}

	if (!tasks_mutex)
		tasks_mutex = al_create_mutex();
	if (!tasks_cond)
		tasks_cond = al_create_cond();
	if (!task_finish_cond)
		task_finish_cond = al_create_cond();

	// Only clear compiled functions if quest has changed since last quest load.
	// TODO: could get even smarter and hash each ZASM script, only recompiling if something really changed.
	static std::pair<std::string, std::filesystem::file_time_type> previous_state;
	static std::pair<std::string, std::filesystem::file_time_type> state = {qstpath, std::filesystem::last_write_time(qstpath)};
	bool should_clear = state != previous_state;
	if (should_clear)
	{
		previous_state = state;
		al_lock_mutex(tasks_mutex);
		thread_pool_generation_count++;
		for (auto &it : compiled_functions)
		{
			rt.release(it.second);
		}
		compiled_functions.clear();
		al_unlock_mutex(tasks_mutex);
	}

	create_compile_tasks();

	// Never make more threads than there are tasks.
	if (pending_scripts.size() + active_tasks.size() < num_threads)
		num_threads = pending_scripts.size() + active_tasks.size();
	set_compilation_thread_pool_size(num_threads);

	al_lock_mutex(tasks_mutex);
	al_broadcast_cond(tasks_cond);
	al_unlock_mutex(tasks_mutex);

	if (precompile)
	{
		al_lock_mutex(tasks_mutex);
		while (pending_scripts.size() || active_tasks.size())
		{
			al_wait_cond(task_finish_cond, tasks_mutex);
		}
		al_unlock_mutex(tasks_mutex);
	}
#endif
}

void jit_poll()
{
	if (!is_enabled)
		return;

#ifdef ZC_JIT
	if (tasks_mutex == nullptr)
		return;

	al_lock_mutex(tasks_mutex);

	int active_threads = 0;
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.state == ThreadState::Done)
		{
			jit_printf("jit: [%d] destroy\n", thread_info.id);
			al_destroy_thread(thread_info.thread);
			thread_info.thread = nullptr;
			thread_info.state = ThreadState::Inactive;
		}

		if (thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting)
		{
			active_threads += 1;
		}
	}

	int tasks_left = active_tasks.size() + pending_scripts.size();
	al_unlock_mutex(tasks_mutex);

	if (active_threads > tasks_left)
		set_compilation_thread_pool_size(tasks_left);
#endif
}

void jit_shutdown()
{
	if (!is_enabled)
		return;

#ifdef ZC_JIT
	if (tasks_mutex == nullptr)
		return;

	set_compilation_thread_pool_size(0);
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.thread)
			al_destroy_thread(thread_info.thread);
	}
	thread_infos = {};

	if (tasks_mutex)
	{
		al_destroy_mutex(tasks_mutex);
		tasks_mutex = nullptr;
	}
	if (tasks_cond)
	{
		al_destroy_cond(tasks_cond);
		tasks_cond = nullptr;
	}
	if (task_finish_cond)
	{
		al_destroy_cond(task_finish_cond);
		task_finish_cond = nullptr;
	}
#endif
}
