// See docs/jit.md for implementation details.

// Many useful resources
// https://webassembly.github.io/spec/core/binary/modules.html
// https://pengowray.github.io/wasm-ops/
// https://github.com/WebAssembly/design/blob/main/BinaryEncoding.md
// https://github.com/sunfishcode/wasm-reference-manual/blob/master/WebAssembly.md
// https://webassembly.github.io/wabt/demo/wasm2wat/
// https://webassembly.github.io/wabt/demo/wat2wasm/
// https://openhome.cc/eGossip/WebAssembly/Block.html
// https://samrat.me/blog/2020-03-29-webassembly-control-instr-examples
// https://musteresel.github.io/posts/2020/01/webassembly-text-br_table-example.html
// https://stackoverflow.com/questions/63880579/how-to-convert-a-control-flow-graph-back-into-its-source-code-e-g-c-c
// https://stackoverflow.com/questions/8866032/flattening-a-control-flow-graph-to-structured-code

// Useful command for quickly verifying valid WASM is generated:
/*
	first, configure build w/ JIT_BACKEND=wasm

	cmake --build build --config Debug -t  zplayer &&
	rm -rf build/Debug/wasm/playground/playground.qst &&
	./build/Debug/zplayer -replay $PWD/tests/replays/playground/maths.zplay -headless -jit -jit-save-wasm -jit-precompile -frame 0 -replay-exit-when-done &&
	wasm2wat build/Debug/wasm/playground.qst/ffc-5-Maths.wasm --enable-threads --generate-names
*/

#include "zc/jit_wasm.h"

#include "allegro/debug.h"
#include "allegro/file.h"
#include "base/check.h"
#include "base/general.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "components/zasm/pc.h"
#include "components/zasm/table.h"
#include "zc/ffscript.h"
#include "zc/jit.h"
#include "zc/script_debug.h"
#include "zc/wasm_compiler.h"
#include "zc/wasm_structurer.h"
#include "zc/zasm_pipeline.h"
#include "zc/zasm_utils.h"
#include "zc/zelda.h"
#include "components/zasm/serialize.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <set>
#include <stdint.h>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

struct CompilationState
{
	WasmAssembler* wasm;
	bool runtime_debugging;
	// When true, compiled calls maintain ri->retsp + ret_stack (the script call
	// stack create_stack_trace reads) so error/Trace context shows the full caller
	// chain. Gated on stack traces being possible, to avoid the per-call cost
	// otherwise.
	bool maintain_call_stack;
	// Keep sp in a function-local instead of the "sp" global; see l_idx_sp_local.
	bool sp_local;
	// Keep the stack/ri/global_d pointers in function-locals; see l_idx_*_local.
	bool ptr_locals;
	// Inline side-effect-free hot engine registers (SWITCHKEY, the
	// GLOBALRAM/SCRIPTRAM array accessors) instead of calling get/set_register.
	bool inline_regs;

	pc_t pc;

	uint8_t f_idx_set_return_value;
	uint8_t f_idx_do_commands;
	uint8_t f_idx_do_commands_async;
	uint8_t f_idx_get_register;
	uint8_t f_idx_set_register;
	uint8_t f_idx_set_guarded_register;
	uint8_t f_idx_runtime_debug;
	uint8_t f_idx_log_error;
	uint8_t f_idx_pod_read;
	uint8_t f_idx_pod_write;
	uint8_t f_idx_allocatemem;
	uint8_t f_idx_writepodarr;

	uint8_t l_idx_j_instance;
	uint8_t l_idx_ri;
	uint8_t l_idx_global_d;
	uint8_t l_idx_stack;
	uint8_t l_idx_ret_stack;
	uint8_t l_idx_ret_stack_index;
	uint8_t l_idx_wait_index;
	uint32_t l_idx_start_pc;

	uint8_t g_idx_j_instance;
	uint8_t g_idx_ri;
	uint8_t g_idx_global_d;
	uint8_t g_idx_stack;
	uint8_t g_idx_ret_stack;
	uint8_t g_idx_ret_stack_index;
	// Base of ri's ret_stack[] (the pc call stack create_stack_trace reads).
	uint8_t g_idx_ret_stack_pc;
	uint8_t g_idx_wait_index;
	uint8_t g_idx_sp;
	uint8_t g_idx_target_block_id;
	uint32_t g_idx_start_pc;
};

#ifdef __EMSCRIPTEN__

EM_ASYNC_JS(int, em_compile_wasm_, (const char* name, void* ptr, size_t size), {
	name = UTF8ToString(name);
	return ZC.compileScriptWasmModule(name, ptr, size);
});
static int em_compile_wasm(const char* name, void* ptr, size_t size)
{
	return em_compile_wasm_(name, ptr, size);
}

// Compile many modules in ONE asyncify round-trip: the browser compiles them
// in parallel on its internal thread pool, and the per-module cost of
// unwinding/rewinding the whole C++ stack (which dominated serial precompile)
// is paid once. Writes a module id (0 = failure) per entry to out_ids.
EM_ASYNC_JS(void, em_compile_wasm_batch_, (int count, uintptr_t names, uintptr_t ptrs, uintptr_t sizes, uintptr_t out_ids), {
	await ZC.compileScriptWasmModuleBatch(count, names, ptrs, sizes, out_ids);
});
static void em_compile_wasm_batch(int count, const char** names, void** ptrs, uint32_t* sizes, int* out_ids)
{
	em_compile_wasm_batch_(count, (uintptr_t)names, (uintptr_t)ptrs, (uintptr_t)sizes, (uintptr_t)out_ids);
}

EM_JS(int, em_create_wasm_handle_, (int module_id), {
	return ZC.createScriptWasmHandle(module_id);
});
static int em_create_wasm_handle(int module_id)
{
	return em_create_wasm_handle_(module_id);
}

EM_ASYNC_JS(int, em_poll_wasm_handle_, (int id, uintptr_t ptr), {
	return ZC.pollScriptWasmHandle(id, ptr);
});
static int em_poll_wasm_handle(int id, uintptr_t ptr)
{
	return em_poll_wasm_handle_(id, ptr);
}

EM_ASYNC_JS(bool, em_destroy_wasm_handle_, (int id), {
	return ZC.destroyScriptWasmHandle(id);
});
bool em_destroy_wasm_handle(int id)
{
	return em_destroy_wasm_handle_(id);
}

EM_JS(bool, em_destroy_wasm_module_, (int id), {
	return ZC.destroyScriptWasmModule(id);
});
static bool em_destroy_wasm_module(int id)
{
	return em_destroy_wasm_module_(id);
}

#else

static int em_compile_wasm([[maybe_unused]] const char* name, [[maybe_unused]] void* ptr, [[maybe_unused]] size_t size)
{
	return 0;
}

static void em_compile_wasm_batch([[maybe_unused]] int count, [[maybe_unused]] const char** names, [[maybe_unused]] void** ptrs, [[maybe_unused]] uint32_t* sizes, [[maybe_unused]] int* out_ids)
{
}

static int em_create_wasm_handle([[maybe_unused]] int module_id)
{
	return 0;
}

static int em_poll_wasm_handle([[maybe_unused]] int id, [[maybe_unused]] uintptr_t ptr)
{
	return 0;
}

static bool em_destroy_wasm_handle([[maybe_unused]] int id)
{
	return false;
}

static bool em_destroy_wasm_module([[maybe_unused]] int id)
{
	return false;
}

#endif

static void error(const zasm_script* script, std::string str, bool expected = false)
{
	str = fmt::format("failed to compile zasm chunk: {} id: {}\nerror: {}\n",
					  script->name, script->id, str);

	al_trace("%s", str.c_str());

	if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL && !expected)
	{
		abort();
	}
}

static void modify_global_idx(WasmAssembler& wasm, int idx, int delta)
{
	wasm.emitGlobalGet(idx);
	wasm.emitI32Const(delta);
	wasm.emitI32Add();
	wasm.emitGlobalSet(idx);
}

// The stack pointer local (see l_idx_sp below): with -jit-wasm-sp-local (the
// default), sp lives in a function-local while inside compiled code - the
// optimizing tier keeps it in a machine register, where the "sp" global would
// be a memory access on every stack operation. The global remains the
// interchange between functions and with the run wrapper: each function loads
// it on entry (emit_sp_reload), and flushes it back before any point another
// function can observe it - a call to a sibling wasm function or a return
// (emit_sp_flush). Engine imports never read the global (they take sp as an
// argument or via ri->sp), and trap bails don't need a flush (the run
// wrapper's post-call code doesn't run after a trap).
constexpr uint8_t l_idx_sp_local = 3;

static void emit_get_sp(CompilationState& state)
{
	if (state.sp_local)
		state.wasm->emitLocalGet(l_idx_sp_local);
	else
		state.wasm->emitGlobalGet(state.g_idx_sp);
}

// Pops the new sp value from the wasm operand stack.
static void emit_set_sp(CompilationState& state)
{
	if (state.sp_local)
		state.wasm->emitLocalSet(l_idx_sp_local);
	else
		state.wasm->emitGlobalSet(state.g_idx_sp);
}

static void emit_sp_flush(CompilationState& state)
{
	if (!state.sp_local)
		return;
	state.wasm->emitLocalGet(l_idx_sp_local);
	state.wasm->emitGlobalSet(state.g_idx_sp);
}

static void emit_sp_reload(CompilationState& state)
{
	if (!state.sp_local)
		return;
	state.wasm->emitGlobalGet(state.g_idx_sp);
	state.wasm->emitLocalSet(l_idx_sp_local);
}

// Read-only pointer locals: the stack base, ri, and global_d pointers are
// loaded once per function (emit_ptr_locals_init) and never change inside
// compiled code - only the run wrapper writes their globals, before any
// function runs - so unlike sp there is nothing to flush. As locals the
// optimizing tier keeps them in registers across the whole function body.
// -jit-wasm-pointer-locals (default on) gates this for A/B runs.
constexpr uint8_t l_idx_stack_local = 4;
constexpr uint8_t l_idx_ri_local = 5;
constexpr uint8_t l_idx_global_d_local = 6;

static void emit_get_stack_base(CompilationState& state)
{
	if (state.ptr_locals)
		state.wasm->emitLocalGet(l_idx_stack_local);
	else
		state.wasm->emitGlobalGet(state.g_idx_stack);
}

static void emit_get_ri(CompilationState& state)
{
	if (state.ptr_locals)
		state.wasm->emitLocalGet(l_idx_ri_local);
	else
		state.wasm->emitGlobalGet(state.g_idx_ri);
}

static void emit_get_global_d(CompilationState& state)
{
	if (state.ptr_locals)
		state.wasm->emitLocalGet(l_idx_global_d_local);
	else
		state.wasm->emitGlobalGet(state.g_idx_global_d);
}

static void emit_ptr_locals_init(CompilationState& state)
{
	if (!state.ptr_locals)
		return;
	state.wasm->emitGlobalGet(state.g_idx_stack);
	state.wasm->emitLocalSet(l_idx_stack_local);
	state.wasm->emitGlobalGet(state.g_idx_ri);
	state.wasm->emitLocalSet(l_idx_ri_local);
	state.wasm->emitGlobalGet(state.g_idx_global_d);
	state.wasm->emitLocalSet(l_idx_global_d_local);
}

static void add_sp(CompilationState& state, int delta)
{
	emit_get_sp(state);
	state.wasm->emitI32Const(delta);
	state.wasm->emitI32Add();
	emit_set_sp(state);
}

static void check_sp(CompilationState& state)
{
	auto& wasm = *state.wasm;

	emit_get_sp(state);
	wasm.emitI32Const(MAX_STACK_SIZE);
	wasm.emitI32GeU();

	wasm.emitIf();
	// ri->pc = state.pc, so the stack-overflow error's innermost stack frame is the
	// command that overflowed (matching the interpreter), not a stale pc.
	emit_get_ri(state);
	wasm.emitI32Const(state.pc);
	wasm.emitI32Store(0); // ri->pc
	wasm.emitI32Const(RUNSCRIPT_JIT_STACK_OVERFLOW);
	wasm.emitCall(state.f_idx_set_return_value);
	wasm.emitUnreachable(); // Bail.
	wasm.emitEnd();
}

static void check_call_limit(CompilationState& state, pc_t target_pc)
{
	auto& wasm = *state.wasm;

	wasm.emitGlobalGet(state.g_idx_ret_stack_index);
	wasm.emitI32Const(MAX_CALL_FRAMES);
	wasm.emitI32GeU();

	wasm.emitIf();
	// Match the interpreter: when the call limit trips, ri->pc points at the entry
	// of the function being called, so the error's innermost stack frame is the
	// recursing function rather than the call site.
	emit_get_ri(state);
	wasm.emitI32Const(target_pc);
	wasm.emitI32Store(0); // ri->pc
	wasm.emitI32Const(RUNSCRIPT_JIT_CALL_LIMIT);
	wasm.emitCall(state.f_idx_set_return_value);
	wasm.emitUnreachable(); // Bail.
	wasm.emitEnd();
}

static void get_z_register(CompilationState& state, int r)
{
	if (r >= D(0) && r < D(INITIAL_D))
	{
		emit_get_ri(state);
		state.wasm->emitI32Load(4 + r*4);
	}
	else if (r >= GD(0) && r < GD(MAX_GLOBAL_VARIABLES))
	{
		emit_get_global_d(state);
		state.wasm->emitI32Load((r - GD(0)) * 4); // game->global_d[]
	}
	else if (r == SP)
	{
		emit_get_sp(state);
		state.wasm->emitI32Const(10000);
		state.wasm->emitI32Mul();
	}
	else if (r == SP2)
	{
		emit_get_sp(state);
	}
	else if (r == SWITCHKEY && state.inline_regs)
	{
		// A plain refInfo field with no side effects (the x64 backend inlines
		// it too); this is the hottest engine register in switch-heavy scripts.
		emit_get_ri(state);
		state.wasm->emitI32Load(offsetof(refInfo, switchkey));
	}
	else if ((r == GLOBALRAM || r == SCRIPTRAM || r == GLOBALRAMD || r == SCRIPTRAMD) && state.inline_regs)
	{
		// Old-style array reads (arrayptr in D0/rINDEX, element in D1/rINDEX2):
		// go straight to pod_read - the same ArrayH::getElement the engine's
		// get_register would reach, minus the giant register switch - with
		// no_neg set, because these registers never allow negative indices
		// (unlike the READPODARRAY family, which honors the QR).
		get_z_register(state, rINDEX);
		if (r == GLOBALRAMD || r == SCRIPTRAMD)
			state.wasm->emitI32Const(0);
		else
		{
			get_z_register(state, rINDEX2);
			state.wasm->emitI32Const(10000);
			state.wasm->emitI32DivS();
		}
		state.wasm->emitI32Const(state.pc);
		state.wasm->emitI32Const(1); // no_neg
		state.wasm->emitCall(state.f_idx_pod_read);
	}
	else
	{
		if (does_register_use_stack(r))
		{
			// ri->sp = sp
			emit_get_ri(state);
			emit_get_sp(state);
			state.wasm->emitI32Store(4*9); // ri->sp
		}

		// ri->pc = state.pc; needed for an accurate stack trace should the engine
		// call log an error. create_stack_trace is its only consumer, so skip it
		// when stack traces won't be shown - except for reads of the PC register,
		// which returns ri->pc itself (the interpreter and x64 backend keep it
		// current, so a stale value here would be a web-only divergence).
		if (state.maintain_call_stack || r == PC)
		{
			emit_get_ri(state);
			state.wasm->emitI32Const(state.pc);
			state.wasm->emitI32Store(0); // ri->pc
		}

		state.wasm->emitI32Const(r);
		state.wasm->emitCall(state.f_idx_get_register);
	}
}

// fn is a template (not std::function) so the per-command capturing lambdas
// don't heap-allocate - this is called for nearly every compiled instruction,
// and the allocations dominated compile-time profiles.
template <typename F>
static void set_z_register(CompilationState& state, int r, F&& fn)
{
	if (r >= D(0) && r < D(INITIAL_D))
	{
		emit_get_ri(state);
		fn();
		state.wasm->emitI32Store(4 + r*4);
	}
	else if (r >= GD(0) && r < GD(MAX_GLOBAL_VARIABLES))
	{
		emit_get_global_d(state);
		fn();
		state.wasm->emitI32Store((r - GD(0)) * 4); // game->global_d[]
	}
	else if (r == SWITCHKEY && state.inline_regs)
	{
		emit_get_ri(state);
		fn();
		state.wasm->emitI32Store(offsetof(refInfo, switchkey));
	}
	else if ((r == GLOBALRAM || r == SCRIPTRAM || r == GLOBALRAMD || r == SCRIPTRAMD) && state.inline_regs)
	{
		// See the matching get_z_register branch. Writes through these
		// registers hold plain values, never object references (type = none),
		// matching the engine's set_register lowering.
		get_z_register(state, rINDEX);
		if (r == GLOBALRAMD || r == SCRIPTRAMD)
			state.wasm->emitI32Const(0);
		else
		{
			get_z_register(state, rINDEX2);
			state.wasm->emitI32Const(10000);
			state.wasm->emitI32DivS();
		}
		fn();
		state.wasm->emitI32Const((int)script_object_type::none);
		state.wasm->emitI32Const(state.pc);
		state.wasm->emitI32Const(1); // no_neg
		state.wasm->emitCall(state.f_idx_pod_write);
	}
	else
	{
		if (does_register_use_stack(r))
		{
			// ri->sp = sp
			emit_get_ri(state);
			emit_get_sp(state);
			state.wasm->emitI32Store(4*9); // ri->sp
		}

		// ri->pc = state.pc; needed for an accurate stack trace should the engine
		// call log an error. create_stack_trace is its only consumer, so skip it
		// when stack traces won't be shown.
		if (state.maintain_call_stack)
		{
			emit_get_ri(state);
			state.wasm->emitI32Const(state.pc);
			state.wasm->emitI32Store(0); // ri->pc
		}

		state.wasm->emitI32Const(r);
		fn();

		// Some registers have an extra check when writing to them.
		if (is_guarded_script_register(r))
		{
			state.wasm->emitI32Const(state.pc); // Needed for accurate stack trace should an error occur.
			state.wasm->emitCall(state.f_idx_set_guarded_register);
		}
		else
		{
			state.wasm->emitCall(state.f_idx_set_register);
		}
	}
}

// Defer to the ZASM command interpreter for 1+ commands.
static void compile_command_interpreter(CompilationState& state, const zasm_script* script, int pc, int count, bool is_wait = false)
{
	// TODO: fast path for these commands?
	bool needs_sp = false;
	for (int j = 0; j < count; j++)
	{
		int index = pc + j;
		if (index >= script->size)
			break;

		if (script->zasm[index].command == STORE_OBJECT || script->zasm[index].command == REF_REMOVE)
		{
			needs_sp = true;
			break;
		}
	}
	if (needs_sp)
	{
		// ri->sp = sp
		emit_get_ri(state);
		emit_get_sp(state);
		state.wasm->emitI32Store(4*9); // ri->sp
	}

	state.wasm->emitGlobalGet(state.g_idx_j_instance);
	state.wasm->emitI32Const(pc);
	state.wasm->emitI32Const(count);
	emit_get_sp(state);
	state.wasm->emitCall(state.f_idx_do_commands);

	if (is_wait)
	{
		state.wasm->emitIf();
		emit_sp_flush(state); // the run wrapper reads the global after we return
		state.wasm->emitReturn(); // End yielder function, return to run function.
		state.wasm->emitEnd();
		return;
	}

	bool could_return_not_ok = false;
	for (int j = 0; j < count; j++)
	{
		int index = pc + j;
		if (index >= script->size)
			break;

		if (command_could_return_not_ok(script->zasm[index].command))
		{
			could_return_not_ok = true;
			break;
		}
	}

	if (could_return_not_ok)
	{
		// If command did not succeed, trap to end execution.
		state.wasm->emitLocalTee(0); // l_idx_scratch
		state.wasm->emitIf();
		state.wasm->emitLocalGet(0); // l_idx_scratch
		state.wasm->emitCall(state.f_idx_set_return_value);
		state.wasm->emitUnreachable(); // Bail.
		state.wasm->emitEnd();
	}
	else
	{
		state.wasm->emitDrop();
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
	case COMPAREV2:
	case GOTO:
	case QUIT:
	case CALLFUNC:
	case RETURNFUNC:
	// Yields back to the driver to run a nested frozen script (handled inline in
	// compile_function, not compile_single_command). Marked compiled so it isn't
	// batched into a do_commands() call.
	case RUNGENFRZSCR:

	// These commands modify the stack pointer, which is just a local copy. If these commands
	// were not compiled, then ri->sp would have to be restored after compile_command_interpreter.
	case POP:
	case POPARGS:
	case PUSHR:
	case PUSHV:
	case PUSHARGSR:
	case PUSHARGSV:

	// These can be commented out to instead run interpreted. Useful for
	// singling out problematic instructions.
	case ABS:
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
	case LOAD:
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
	case ORR:
	case ORR32:
	case ORV:
	case ORV32:
	case PEEK:
	case SETR:
	case SETV:
	case STORE:
	case STORED:
	case STOREI:
	case SUBR:
	case SUBV:
	case SUBV2:
	case BITNOT:
	case BITNOT32:
	case LSHIFTV:
	case LSHIFTV32:
	case RSHIFTV:
	case RSHIFTV32:
	case LSHIFTR:
	case LSHIFTR32:
	case RSHIFTR:
	case RSHIFTR32:
	case XORR:
	case XORV:
	case XORR32:
	case XORV32:
	case READPODARRAYR:
	case READPODARRAYV:
	case WRITEPODARRAYRR:
	case WRITEPODARRAYRV:
	case WRITEPODARRAYVR:
	case WRITEPODARRAYVV:
	case WRITEPODARRAY:
	case ALLOCATEMEMV:
		return true;
	}

	return false;
}

// Maintains ri->retsp + ret_stack (the pc call stack create_stack_trace walks)
// around a compiled call, so an error/Trace inside the callee reports the full
// caller chain - matching the interpreter and x64 JIT. The x64 JIT gets this for
// free by returning to run_script_int for CALLFUNC; the wasm JIT compiles calls
// inline, so it must replicate the push/pop. Only emitted when stack traces are
// possible (state.maintain_call_stack), to avoid the per-call cost otherwise.
static void emit_trace_call_stack_push(CompilationState& state, pc_t pc)
{
	if (!state.maintain_call_stack)
		return;
	auto& wasm = *state.wasm;
	// ret_stack[ri->retsp] = pc + 1. The interpreter pushes ri->pc+1; create_stack_trace
	// reads back (stored - 1), which maps to the CALLFUNC's call site.
	wasm.emitGlobalGet(state.g_idx_ret_stack_pc);
	emit_get_ri(state);
	wasm.emitI32Load(4 * 10); // ri->retsp
	wasm.emitI32Const(4);
	wasm.emitI32Mul();
	wasm.emitI32Add();
	wasm.emitI32Const(pc + 1);
	wasm.emitI32Store(0);
	// ri->retsp += 1
	emit_get_ri(state);
	emit_get_ri(state);
	wasm.emitI32Load(4 * 10);
	wasm.emitI32Const(1);
	wasm.emitI32Add();
	wasm.emitI32Store(4 * 10);
}

static void emit_trace_call_stack_pop(CompilationState& state)
{
	if (!state.maintain_call_stack)
		return;
	auto& wasm = *state.wasm;
	// if (ri->retsp) ri->retsp -= 1;  - mirrors retstack_pop, which no-ops at the
	// root frame (the entry function's top-level RETURN was never pushed).
	emit_get_ri(state);
	wasm.emitI32Load(4 * 10); // ri->retsp
	wasm.emitIf();
	{
		emit_get_ri(state);
		emit_get_ri(state);
		wasm.emitI32Load(4 * 10);
		wasm.emitI32Const(1);
		wasm.emitI32Sub();
		wasm.emitI32Store(4 * 10);
	}
	wasm.emitEnd();
}

// Locals reserved for snapshotting the two comparison operands (see the
// defineFunction calls that declare seven I32 locals). Local 0 is the general
// scratch; locals 1/2 double as the second scratch for two-operand commands
// (never live at the same time as a comparison snapshot, since a comparison's
// consumers immediately follow it). Local 3 is the sp local (l_idx_sp_local).
constexpr uint8_t l_idx_scratch = 0;
constexpr uint8_t l_idx_scratch2 = 1;
constexpr uint8_t l_idx_cmp1 = 1;
constexpr uint8_t l_idx_cmp2 = 2;

static bool command_is_goto_consumer(int command)
{
	return command == GOTOCMP || command == GOTOTRUE || command == GOTOFALSE ||
		   command == GOTOMORE || command == GOTOLESS;
}

// A comparison command (COMPARER/COMPAREV/COMPAREV2) is fused with the run of
// commands that consume its result. A single comparison can feed MULTIPLE
// consumers: the optimizer emits e.g. `COMPAREV D2 N; SETCMP D2; GOTOCMP T`,
// where the boolean is both stored to a register AND branched on, all from the
// one compare. Each consumer re-derives the comparison from the operands.
struct CmpGroup
{
	pc_t cmp_pc;
	bool arg1_is_imm;
	bool arg2_is_imm;
	// With more than one real consumer, snapshot the operand values into the
	// reserved locals up-front, so a SETx consumer overwriting an operand
	// register can't corrupt a later one.
	bool capture;
	std::vector<pc_t> consumers; // the NOP/SETx/GOTOx run following cmp_pc
};

static CmpGroup analyze_comparison(const zasm_script* script, pc_t i)
{
	CmpGroup g{};
	g.cmp_pc = i;

	int command = script->zasm[i].command;
	g.arg1_is_imm = false;
	g.arg2_is_imm = command != COMPARER;
	if (command == COMPAREV2)
		std::swap(g.arg1_is_imm, g.arg2_is_imm);

	for (pc_t j = i + 1; j < (pc_t)script->size; j++)
	{
		int c = script->zasm[j].command;
		if (c == NOP || command_uses_comparison_result(c))
			g.consumers.push_back(j);
		else
			break;
	}

	int real_consumers = 0;
	for (pc_t j : g.consumers)
		if (script->zasm[j].command != NOP)
			real_consumers++;
	g.capture = real_consumers > 1;

	return g;
}

// Emits the operand snapshot for a capturing group; called once, at the compare.
static void emit_cmp_capture(CompilationState& state, const zasm_script* script, const CmpGroup& g)
{
	if (!g.capture)
		return;
	WasmAssembler& wasm = *state.wasm;
	int arg1 = script->zasm[g.cmp_pc].arg1;
	int arg2 = script->zasm[g.cmp_pc].arg2;
	if (!g.arg1_is_imm) { get_z_register(state, arg1); wasm.emitLocalSet(l_idx_cmp1); }
	if (!g.arg2_is_imm) { get_z_register(state, arg2); wasm.emitLocalSet(l_idx_cmp2); }
}

// Pushes both comparison operands for one consumer.
static void emit_cmp_operands(CompilationState& state, const zasm_script* script, const CmpGroup& g, bool cmp_bool)
{
	WasmAssembler& wasm = *state.wasm;
	auto push_operand = [&](int arg, bool is_imm, uint8_t local){
		if (is_imm)
		{
			wasm.emitI32Const(cmp_bool ? !!arg : arg);
			return;
		}
		if (g.capture)
			wasm.emitLocalGet(local);
		else
			get_z_register(state, arg);
		if (cmp_bool)
		{
			wasm.emitI32Const(0);
			wasm.emitI32Ne();
		}
	};
	push_operand(script->zasm[g.cmp_pc].arg1, g.arg1_is_imm, l_idx_cmp1);
	push_operand(script->zasm[g.cmp_pc].arg2, g.arg2_is_imm, l_idx_cmp2);
}

// Consumes the two operands, leaving the boolean the CMP_FLAGS select.
static void emit_cmp_flags_op(WasmAssembler& wasm, int flags)
{
	switch (flags & CMP_FLAGS)
	{
		default: wasm.emitDrop(); wasm.emitDrop(); wasm.emitI32Const(0); break;
		case CMP_GT: wasm.emitI32GtS(); break;
		case CMP_GT|CMP_EQ: wasm.emitI32GeS(); break;
		case CMP_LT: wasm.emitI32LtS(); break;
		case CMP_LT|CMP_EQ: wasm.emitI32LeS(); break;
		case CMP_EQ: wasm.emitI32Eq(); break;
		case CMP_GT|CMP_LT: wasm.emitI32Ne(); break;
		case CMP_GT|CMP_LT|CMP_EQ: wasm.emitDrop(); wasm.emitDrop(); wasm.emitI32Const(1); break;
	}
}

// Emits one SETx consumer at cj.
static void emit_cmp_set_consumer(CompilationState& state, const zasm_script* script, const CmpGroup& g, pc_t cj)
{
	WasmAssembler& wasm = *state.wasm;
	int cc = script->zasm[cj].command;
	int c_arg1 = script->zasm[cj].arg1;
	int c_arg2 = script->zasm[cj].arg2;
	bool cmp_bool = c_arg2 & CMP_BOOL;

	if (cc == SETCMP)
	{
		set_z_register(state, c_arg1, [&](){
			emit_cmp_operands(state, script, g, cmp_bool);
			emit_cmp_flags_op(wasm, c_arg2);
			if (c_arg2 & CMP_SETI) { wasm.emitI32Const(10000); wasm.emitI32Mul(); }
		});
	}
	else if (cc == SETLESSI || cc == SETLESS)
	{
		set_z_register(state, c_arg1, [&](){
			emit_cmp_operands(state, script, g, cmp_bool); wasm.emitI32LeS();
			if (cc == SETLESSI) { wasm.emitI32Const(10000); wasm.emitI32Mul(); }
		});
	}
	else if (cc == SETMOREI || cc == SETMORE)
	{
		set_z_register(state, c_arg1, [&](){
			emit_cmp_operands(state, script, g, cmp_bool); wasm.emitI32GeS();
			if (cc == SETMOREI) { wasm.emitI32Const(10000); wasm.emitI32Mul(); }
		});
	}
	else if (cc == SETFALSEI || cc == SETFALSE)
	{
		set_z_register(state, c_arg1, [&](){
			emit_cmp_operands(state, script, g, cmp_bool); wasm.emitI32Ne();
			if (cc == SETFALSEI) { wasm.emitI32Const(10000); wasm.emitI32Mul(); }
		});
	}
	else if (cc == SETTRUEI || cc == SETTRUE)
	{
		set_z_register(state, c_arg1, [&](){
			emit_cmp_operands(state, script, g, cmp_bool); wasm.emitI32Eq();
			if (cc == SETTRUEI) { wasm.emitI32Const(10000); wasm.emitI32Mul(); }
		});
	}
	else
	{
		error(script, fmt::format("unexpected comparison consumer {} at index {}", zasm_op_to_string(cc).c_str(), (int)cj));
		CHECK(false);
	}
}

// Emits the take-branch value for one GOTOx consumer at cj: leaves an i32 on
// the stack, nonzero meaning the branch to the goto target is taken.
static void emit_cmp_goto_value(CompilationState& state, const zasm_script* script, const CmpGroup& g, pc_t cj)
{
	WasmAssembler& wasm = *state.wasm;
	int cc = script->zasm[cj].command;
	int c_arg2 = script->zasm[cj].arg2;
	bool cmp_bool = c_arg2 & CMP_BOOL;

	emit_cmp_operands(state, script, g, cmp_bool);
	switch (cc)
	{
		case GOTOCMP: emit_cmp_flags_op(wasm, c_arg2); break;
		case GOTOTRUE: wasm.emitI32Eq(); break;
		case GOTOFALSE: wasm.emitI32Ne(); break;
		case GOTOMORE: wasm.emitI32GeS(); break;
		case GOTOLESS: wasm.emitI32LeS(); break;
	}
}

// Compiles a comparison command at `i` together with every consumer, for the
// loop-switch lowering. Returns the pc of the last command consumed (the
// caller's loop advances past it); for a loose compare with no consumer,
// returns `i`. num_frames/current_rank/current_block_index are the caller's
// scaffold trackers (see compile_function); a consumer run may cross block
// boundaries, which closes scaffold frames.
static pc_t compile_comparison(CompilationState& state, const zasm_script* script, const ZasmCFG& cfg, int num_frames, int& current_rank, pc_t& current_block_index, pc_t i)
{
	WasmAssembler& wasm = *state.wasm;

	CmpGroup g = analyze_comparison(script, i);
	if (g.consumers.empty())
	{
		// A loose compare (e.g. the statement `x == 1;`). Ignore it.
		return i;
	}

	emit_cmp_capture(state, script, g);

	for (pc_t cj : g.consumers)
	{
		// O(1) equivalent of cfg.contains_block_start(cj): during block b's body
		// current_block_index == b + 1 (see compile_function).
		if (current_block_index < cfg.block_starts.size() && cfg.block_starts[current_block_index] == cj)
		{
			// Consumer blocks are never absorbed into a structured region
			// (detect_yielder_regions drops any region a run leaks into), so
			// every one of these has a scaffold frame.
			wasm.emitEnd();
			current_block_index += 1;
			current_rank += 1;
		}

		int cc = script->zasm[cj].command;
		if (cc == NOP)
		{
			// The optimizer can turn a consumer into a NOP without removing
			// the compare; nothing to emit.
			//
			// Example: hollow_forest.qst/zasm-global-1.txt
			// 11911: COMPAREV        D2              0            
			// 11912: GOTOTRUE        11913                        ---- turns into NOP
			continue;
		}

		if (command_is_goto_consumer(cc))
		{
			emit_cmp_goto_value(state, script, g, cj);

			size_t target_block_index = cfg.block_id_from_start_pc(script->zasm[cj].arg1) + 1;
			wasm.emitI32Const(target_block_index);
			wasm.emitGlobalSet(state.g_idx_target_block_id);
			wasm.emitBrIf(num_frames - current_rank);
		}
		else
		{
			emit_cmp_set_consumer(state, script, g, cj);
		}
	}

	return g.consumers.back();
}

// A CALLFUNC lowered as a native wasm call, for a callee compiled as its own
// (non-yielding) wasm function. The native call stack handles the return;
// g_idx_ret_stack_index is still maintained so check_call_limit sees the true
// depth.
static void compile_callfunc_native(CompilationState& state, const StructuredZasm& structured_zasm, const std::map<pc_t, pc_t>& function_id_to_idx, pc_t i, int target_pc)
{
	WasmAssembler& wasm = *state.wasm;

	check_call_limit(state, target_pc);

	// g_idx_ret_stack_index += 1
	modify_global_idx(wasm, state.g_idx_ret_stack_index, 1);
	emit_trace_call_stack_push(state, i);

	// Set the initial block id to 0 so a loop-switch callee starts at its
	// beginning. (A structured callee always starts at its entry and ignores
	// this.)
	wasm.emitI32Const(0);
	wasm.emitGlobalSet(state.g_idx_target_block_id);

	pc_t fn_id = structured_zasm.start_pc_to_function.at(target_pc);
	emit_sp_flush(state);
	wasm.emitCall(function_id_to_idx.at(fn_id));
	emit_sp_reload(state);

	// g_idx_ret_stack_index -= 1
	emit_trace_call_stack_pop(state);
	modify_global_idx(wasm, state.g_idx_ret_stack_index, -1);
}

// A GOTO whose target is the entry of a function in a *different* compile
// unit. Each (non-yielding) function is compiled as its own WASM function, and
// a branch can only reach code within the current function, so we cannot
// branch there. Instead, lower the GOTO as a tail-call into the target
// function.
//
// This is only expected for the "~Init" script of older (2.55-era) quests:
// that script is assembled by inlining init function bodies and tail-jumping
// over them (via GOTO) to the "run" continuation, rather than using a plain
// CALLFUNC. Modern compiles don't produce cross-function GOTOs.
// jit_compile_script validates the target is a separately-compiled,
// non-yielding function's entry, so emitCall is sufficient.
//
// Example: .tmp/replay_uploads/EB5E2CFAE26A97BAA15637C0A60D557A/6940ead2-143f-45b9-a894-1bc05c81b9e0-updated-main.zplay
static void compile_goto_tailcall(CompilationState& state, const StructuredZasm& structured_zasm, const std::map<pc_t, pc_t>& function_id_to_idx, int target_pc)
{
	WasmAssembler& wasm = *state.wasm;

	check_call_limit(state, target_pc);
	modify_global_idx(wasm, state.g_idx_ret_stack_index, 1);
	wasm.emitI32Const(0);
	wasm.emitGlobalSet(state.g_idx_target_block_id);
	emit_sp_flush(state);
	wasm.emitCall(function_id_to_idx.at(structured_zasm.start_pc_to_function.at(target_pc)));
	emit_sp_reload(state);
	modify_global_idx(wasm, state.g_idx_ret_stack_index, -1);
	// The GOTO does not return here, so end this function once the callee does.
	emit_sp_flush(state);
	wasm.emitReturn();
}

static void compile_quit(CompilationState& state, const zasm_script* script, pc_t i)
{
	compile_command_interpreter(state, script, i, 1);
	state.wasm->emitI32Const(RUNSCRIPT_STOPPED);
	state.wasm->emitCall(state.f_idx_set_return_value);
	state.wasm->emitUnreachable(); // Bail.
}

// Emits one straight-line ("plain") compiled command - everything except
// control flow, which each lowering (the loop-switch in compile_function, or
// the structured path in compile_function_structured) handles itself.
static void compile_plain_command(CompilationState& state, const zasm_script* script, pc_t i)
{
	WasmAssembler& wasm = *state.wasm;

	int command = script->zasm[i].command;
	int arg1 = script->zasm[i].arg1;
	int arg2 = script->zasm[i].arg2;

	// Every command here must be reflected in command_is_compiled!
	switch (command)
	{
		case NOP: break;

		case PUSHR:
		case PUSHV:
		{
			// Evaluate the value to push BEFORE decrementing sp, matching the
			// interpreter and x64 JIT. A stack-dependent register (e.g.
			// SCREENDATAEXDOOR) read after add_sp would see the decremented sp
			// and read the wrong stack slot. Capturing into a local also handles
			// SP/SP2 (which read the current sp) without a special case.
			if (command == PUSHR)
				get_z_register(state, arg1);
			else
				wasm.emitI32Const(arg1);
			wasm.emitLocalSet(l_idx_scratch);

			add_sp(state, -1);
			check_sp(state);

			emit_get_sp(state);
			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			emit_get_stack_base(state);
			wasm.emitI32Add(); // Add stack base offset.
			wasm.emitLocalGet(l_idx_scratch);
			wasm.emitI32Store();
		}
		break;

		case PUSHARGSR:
		case PUSHARGSV:
		{
			if (command == PUSHARGSR)
			{
				get_z_register(state, arg1);
				wasm.emitLocalSet(l_idx_scratch);
			}

			// TODO: there's certainly a better way to do this.
			for (int i = 0; i < arg2; i++)
			{
				add_sp(state, -1);
				check_sp(state);

				emit_get_sp(state);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add(); // Add stack base offset.
				if (command == PUSHARGSR)
					wasm.emitLocalGet(l_idx_scratch);
				else
					wasm.emitI32Const(arg1);
				wasm.emitI32Store();
			}
		}
		break;

		case SETR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg2);
			});
		}
		break;
		case SETV:
		{
			// For test_jit_runtime_debug_test.
			static bool jit_runtime_debug_test_force_bug = get_flag_bool("-jit-runtime-debug-test-force-bug").value_or(false);
			if (jit_runtime_debug_test_force_bug) arg2++;

			set_z_register(state, arg1, [&](){
				wasm.emitI32Const(arg2);
			});
		}
		break;
		case STACKWRITEATVV:
		{
			// Lit[arg1] -> Stack[arg2]
			emit_get_sp(state);
			wasm.emitI32Const(arg2);
			wasm.emitI32Add();
			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			emit_get_stack_base(state);
			wasm.emitI32Add();
			wasm.emitI32Const(arg1);
			wasm.emitI32Store();
		}
		break;
		case STORE:
		{
			// Reg[arg1] -> Stack[rSFRAME + arg2]
			get_z_register(state, rSFRAME);
			if (arg2)
			{
				wasm.emitI32Const(arg2);
				wasm.emitI32Add();
			}

			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			emit_get_stack_base(state);
			wasm.emitI32Add();
			get_z_register(state, arg1);
			wasm.emitI32Store();
		}
		break;
		case STORED:
		{
			// Reg[arg1] -> Stack[rSFRAME + arg2]
			get_z_register(state, rSFRAME);
			wasm.emitI32Const(10000);
			wasm.emitI32DivU();
			if (arg2)
			{
				wasm.emitI32Const(arg2 / 10000);
				wasm.emitI32Add();
			}

			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			emit_get_stack_base(state);
			wasm.emitI32Add();
			get_z_register(state, arg1);
			wasm.emitI32Store();
		}
		break;
		case LOAD:
		{
			// Stack[rSFRAME + arg2] -> Reg[arg1]
			set_z_register(state, arg1, [&](){
				get_z_register(state, rSFRAME);
				if (arg2)
				{
					wasm.emitI32Const(arg2);
					wasm.emitI32Add();
				}
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add();
				wasm.emitI32Load();
			});
		}
		break;
		case LOADD:
		{
			// Stack[rSFRAME + arg2] -> Reg[arg1]
			set_z_register(state, arg1, [&](){
				get_z_register(state, rSFRAME);
				wasm.emitI32Const(10000);
				wasm.emitI32DivU();
				if (arg2)
				{
					wasm.emitI32Const(arg2 / 10000);
					wasm.emitI32Add();
				}

				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add();
				wasm.emitI32Load();
			});
		}
		break;
		case POP:
		{
			set_z_register(state, arg1, [&](){
				emit_get_sp(state);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add(); // Add stack base offset.
				wasm.emitI32Load();

				add_sp(state, 1);
			});
		}
		break;

		case POPARGS:
		{
			add_sp(state, arg2);

			set_z_register(state, arg1, [&](){
				// ri->sp - 1
				emit_get_sp(state);
				wasm.emitI32Const(1);
				wasm.emitI32Sub();

				wasm.emitI32Const(4);
				wasm.emitI32Mul();

				emit_get_stack_base(state);
				wasm.emitI32Add(); // Add stack base offset.
				wasm.emitI32Load();
			});
		}
		break;

		case DIVR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg2);
				wasm.emitLocalTee(l_idx_scratch);

				wasm.emitIf(WasmSimpleBlockType::I32);
				get_z_register(state, arg1);
				wasm.emitI32ExtendS();
				wasm.emitI64Const(10000);
				wasm.emitI64Mul();
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32ExtendS();
				wasm.emitI64DivS();
				wasm.emitI64Wrap();

				wasm.emitElse();
				wasm.emitI32Const(0);
				wasm.emitCall(state.f_idx_log_error);
				wasm.emitI32Const(MAX_SIGNED_32);
				wasm.emitEnd();
			});
		}
		break;

		case PEEK:
		{
			set_z_register(state, arg1, [&](){
				emit_get_sp(state);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add(); // Add stack base offset.
				wasm.emitI32Load();
			});
		}
		break;

		case DIVV:
		{
			set_z_register(state, arg1, [&](){
				if (arg2 == 0)
				{
					wasm.emitI32Const(0);
					wasm.emitCall(state.f_idx_log_error);
					wasm.emitI32Const(MAX_SIGNED_32);
					return;
				}

				get_z_register(state, arg1);
				wasm.emitI32ExtendS();
				wasm.emitI64Const(10000);
				wasm.emitI64Mul();
				wasm.emitI64Const(arg2);
				wasm.emitI64DivS();
				wasm.emitI64Wrap();
			});
		}
		break;

		case ADDR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				get_z_register(state, arg2);
				wasm.emitI32Add();
			});
		}
		break;
		case ADDV:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(arg2);
				wasm.emitI32Add();
			});
		}
		break;

		case CASTBOOLF:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(0);
				wasm.emitI32Ne();
			});
		}
		break;
		case CASTBOOLI:
		{
			set_z_register(state, arg1, [&](){
				wasm.emitI32Const(10000);
				wasm.emitI32Const(0);
				get_z_register(state, arg1);
				wasm.emitSelect();
			});
		}
		break;

		case MODR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg2);
				wasm.emitLocalTee(l_idx_scratch);

				wasm.emitIf(WasmSimpleBlockType::I32);
				get_z_register(state, arg1);
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32RemS();

				wasm.emitElse();
				wasm.emitI32Const(1);
				wasm.emitCall(state.f_idx_log_error);
				wasm.emitI32Const(0);
				wasm.emitEnd();
			});
		}
		break;
		case MODV:
		{
			set_z_register(state, arg1, [&](){
				if (arg2 == 0)
				{
					wasm.emitI32Const(1);
					wasm.emitCall(state.f_idx_log_error);
					wasm.emitI32Const(0);
					return;
				}

				get_z_register(state, arg1);
				wasm.emitI32Const(arg2);
				wasm.emitI32RemS();
			});
		}
		break;

		case MULTR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32ExtendS();
				get_z_register(state, arg2);
				wasm.emitI32ExtendS();
				wasm.emitI64Mul();
				wasm.emitI64Const(10000);
				wasm.emitI64DivS();
				wasm.emitI64Wrap();
			});
		}
		break;
		case MULTV:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32ExtendS();
				wasm.emitI64Const(arg2);
				wasm.emitI64Mul();
				wasm.emitI64Const(10000);
				wasm.emitI64DivS();
				wasm.emitI64Wrap();
			});
		}
		break;

		case SUBR:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				get_z_register(state, arg2);
				wasm.emitI32Sub();
			});
		}
		break;
		case SUBV:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(arg2);
				wasm.emitI32Sub();
			});
		}
		break;
		case SUBV2:
		{
			// reg = arg1 - reg (note: the destination is arg2).
			set_z_register(state, arg2, [&](){
				wasm.emitI32Const(arg1);
				get_z_register(state, arg2);
				wasm.emitI32Sub();
			});
		}
		break;

		case ABS:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitLocalSet(l_idx_scratch);
				// val < 0 ? -val : val
				wasm.emitI32Const(0);
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32Sub();
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32Const(0);
				wasm.emitI32LtS();
				wasm.emitSelect();
			});
		}
		break;

		case MINR:
		case MINV:
		case MAXR:
		case MAXV:
		{
			bool is_min = command == MINR || command == MINV;
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitLocalSet(l_idx_scratch);
				if (command == MINR || command == MAXR)
					get_z_register(state, arg2);
				else
					wasm.emitI32Const(arg2);
				wasm.emitLocalSet(l_idx_scratch2);

				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitLocalGet(l_idx_scratch2);
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitLocalGet(l_idx_scratch2);
				if (is_min)
					wasm.emitI32LtS();
				else
					wasm.emitI32GtS();
				wasm.emitSelect();
			});
		}
		break;

		// AND/OR operate on the integer part: (a/10000 OP b/10000) * 10000.
		case ANDR:
		case ANDV:
		case ORR:
		case ORV:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				if (command == ANDR || command == ORR)
				{
					get_z_register(state, arg2);
					wasm.emitI32Const(10000);
					wasm.emitI32DivS();
				}
				else
					wasm.emitI32Const(arg2 / 10000);
				if (command == ANDR || command == ANDV)
					wasm.emitI32And();
				else
					wasm.emitI32Or();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case ORR32:
		case ORV32:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				if (command == ORR32)
					get_z_register(state, arg2);
				else
					wasm.emitI32Const(arg2);
				wasm.emitI32Or();
			});
		}
		break;

		// FLOOR/CEILING round the fixed-point value to a whole number
		// (a multiple of 10000), toward -/+ infinity. Matches zfix
		// getFloor/getCeil: q = val/10000 (truncating), then adjust q by
		// one when there is a remainder in the rounding direction.
		case FLOOR:
		case CEILING:
		{
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitLocalTee(l_idx_scratch);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				// (val % 10000 != 0) & (val < 0 for floor / val > 0 for ceil)
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32Const(10000);
				wasm.emitI32RemS();
				wasm.emitI32Const(0);
				wasm.emitI32Ne();
				wasm.emitLocalGet(l_idx_scratch);
				wasm.emitI32Const(0);
				if (command == FLOOR)
					wasm.emitI32LtS();
				else
					wasm.emitI32GtS();
				wasm.emitI32And();
				if (command == FLOOR)
					wasm.emitI32Sub();
				else
					wasm.emitI32Add();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;

		case LOADI:
		{
			// Stack[reg[arg2] / 10000] -> Reg[arg1]
			// Like the x64 JIT (but unlike the interpreter), no stack
			// bounds check - same as the LOAD/STORE family above.
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg2);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				emit_get_stack_base(state);
				wasm.emitI32Add();
				wasm.emitI32Load();
			});
		}
		break;
		case STOREI:
		{
			// Reg[arg1] -> Stack[reg[arg2] / 10000]
			get_z_register(state, arg2);
			wasm.emitI32Const(10000);
			wasm.emitI32DivS();
			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			emit_get_stack_base(state);
			wasm.emitI32Add();
			get_z_register(state, arg1);
			wasm.emitI32Store();
		}
		break;

		case BITNOT:
		{
			// reg = (~(reg / 10000)) * 10000  (~x == x ^ -1)
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				wasm.emitI32Const(-1);
				wasm.emitI32Xor();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case BITNOT32:
		{
			// reg = ~reg
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(-1);
				wasm.emitI32Xor();
			});
		}
		break;
		case LSHIFTV:
		case RSHIFTV:
		{
			// reg = ((reg / 10000) <</>> k) * 10000, k is a constant count.
			// wasm masks the shift count to 5 bits, matching the interpreter
			// (whose C++ shifts also lower to masked shifts); >> is arithmetic.
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				wasm.emitI32Const(arg2 / 10000);
				if (command == LSHIFTV) wasm.emitI32Shl();
				else wasm.emitI32ShrS();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case LSHIFTV32:
		case RSHIFTV32:
		{
			// reg = reg <</>> k (raw, no fixed-point scaling on the value).
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(arg2 / 10000);
				if (command == LSHIFTV32) wasm.emitI32Shl();
				else wasm.emitI32ShrS();
			});
		}
		break;
		case LSHIFTR:
		case RSHIFTR:
		{
			// reg = ((reg / 10000) <</>> (count / 10000)) * 10000, count is a register.
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				get_z_register(state, arg2);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				if (command == LSHIFTR) wasm.emitI32Shl();
				else wasm.emitI32ShrS();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case LSHIFTR32:
		case RSHIFTR32:
		{
			// reg = reg <</>> (count / 10000) (raw value, count is a register).
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				get_z_register(state, arg2);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				if (command == LSHIFTR32) wasm.emitI32Shl();
				else wasm.emitI32ShrS();
			});
		}
		break;
		case XORV:
		{
			// reg = ((reg / 10000) ^ (arg2 / 10000)) * 10000
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				wasm.emitI32Const(arg2 / 10000);
				wasm.emitI32Xor();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case XORR:
		{
			// reg = ((reg / 10000) ^ (arg2reg / 10000)) * 10000
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				get_z_register(state, arg2);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
				wasm.emitI32Xor();
				wasm.emitI32Const(10000);
				wasm.emitI32Mul();
			});
		}
		break;
		case XORV32:
		{
			// reg = reg ^ arg2 (raw)
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				wasm.emitI32Const(arg2);
				wasm.emitI32Xor();
			});
		}
		break;
		case XORR32:
		{
			// reg = reg ^ arg2reg (raw)
			set_z_register(state, arg1, [&](){
				get_z_register(state, arg1);
				get_z_register(state, arg2);
				wasm.emitI32Xor();
			});
		}
		break;

		case READPODARRAYR:
		case READPODARRAYV:
		{
			// reg[arg1] = array[rINDEX][index]. Calls a helper for the bounds-checked
			// access (array pointer + index passed as args), so no register flush.
			set_z_register(state, arg1, [&](){
				get_z_register(state, rINDEX);
				if (command == READPODARRAYR)
				{
					get_z_register(state, arg2);
					wasm.emitI32Const(10000);
					wasm.emitI32DivS();
				}
				else
				{
					wasm.emitI32Const(arg2 / 10000);
				}
				wasm.emitI32Const(state.pc);
				wasm.emitI32Const(0); // no_neg: honor the negative-index QR
				wasm.emitCall(state.f_idx_pod_read);
			});
		}
		break;
		case WRITEPODARRAYRR:
		case WRITEPODARRAYRV:
		case WRITEPODARRAYVR:
		case WRITEPODARRAYVV:
		{
			// array[rINDEX][index] = value. First suffix letter is the index operand
			// kind, second is the value operand kind (R=register, V=immediate).
			bool index_is_reg = command == WRITEPODARRAYRR || command == WRITEPODARRAYRV;
			bool value_is_reg = command == WRITEPODARRAYRR || command == WRITEPODARRAYVR;

			get_z_register(state, rINDEX);
			if (index_is_reg)
			{
				get_z_register(state, arg1);
				wasm.emitI32Const(10000);
				wasm.emitI32DivS();
			}
			else
			{
				wasm.emitI32Const(arg1 / 10000);
			}
			if (value_is_reg)
				get_z_register(state, arg2);
			else
				wasm.emitI32Const(arg2);
			wasm.emitI32Const(script->zasm[i].arg3);
			wasm.emitI32Const(state.pc);
			wasm.emitI32Const(0); // no_neg: honor the negative-index QR
			wasm.emitCall(state.f_idx_pod_write);
		}
		break;
		case WRITEPODARRAY:
		{
			// Bulk-initialize array (reg arg1) from the instruction's constant vector.
			get_z_register(state, arg1);
			wasm.emitI32Const(state.pc);
			wasm.emitCall(state.f_idx_writepodarr);
		}
		break;
		case ALLOCATEMEMV:
		{
			// reg[arg1] = allocate(size=arg2/10000, object_type=arg3).
			set_z_register(state, arg1, [&](){
				wasm.emitI32Const(arg2 / 10000);
				wasm.emitI32Const(script->zasm[i].arg3);
				wasm.emitI32Const(state.pc);
				wasm.emitCall(state.f_idx_allocatemem);
			});
		}
		break;

		default:
		{
			printf("unexpected command %s at index %d\n", zasm_op_to_string(command).c_str(), (int)i);
			CHECK(false);
		}
	}
}

// Structured ("relooped") compilation, for non-yielding functions.
//
// Instead of the loop-switch dispatch (see compile_function below), the
// function's CFG is reconstructed into real wasm block/loop/if control flow
// via WasmStructurer (Ramsey, "Beyond Relooper", ICFP 2022). Script loops
// become real wasm loops - no global write + br_table dispatch per iteration,
// and the browser's optimizing tier can see the loop nest.
//
// Only non-yielding functions qualify: the yielder's call/return/resume
// targets are runtime values (a computed goto), which structured control flow
// cannot express, so it stays on the loop-switch.
//
// compile_function_structured returns nullopt when the function can't be
// structured - an irreducible CFG, or a control shape the classifier doesn't
// model (e.g. a comparison-goto with no reaching compare) - and the caller
// falls back to the loop-switch lowering.

struct StructuredFnSink : StructSink
{
	CompilationState* state;
	const zasm_script* script;
	const StructuredZasm* structured_zasm;
	const std::map<pc_t, pc_t>* function_id_to_idx;
	const std::vector<pc_t>* starts;
	// Last pc each block's body emits; the block's terminating branch (if any)
	// is emitted by the structurer via BlockInfo / emit_cond instead.
	const std::vector<int>* body_last;
	// For a block terminated by an intra-function GOTO (emitted by the
	// structurer as a bare branch), the GOTO's pc - so its debug marker and
	// runtime_debug call still appear, matching the loop-switch lowering and
	// the interpreter's trace. -1 otherwise.
	const std::vector<int>* goto_debug_pc;
	const std::map<pc_t, CmpGroup>* cmp_groups;
	const std::map<pc_t, pc_t>* consumer_to_cmp;
	pc_t final_pc;
	// The entry function of a script that ends by falling off the end (global
	// init scripts have no QUIT) runs the script-end 0xFFFF handling.
	bool ffff_tail;

	// Term::Dispatch support, used only when this sink emits a structured
	// region nested inside the yielder's loop-switch (see YielderRegionPlan).
	// A dispatch terminator sets g_idx_target_block_id and branches out to the
	// enclosing dispatch loop.
	struct DispatchDesc
	{
		// The value stored into g_idx_target_block_id: global cfg block id + 1.
		// Unused when is_return (RETURNFUNC computes the target at runtime).
		int value = -1;
		// The terminator instruction's pc, for the debug marker / trace call.
		pc_t pc = 0;
		bool is_return = false;
		bool emit_debug = false;
	};
	// local block id -> descriptor. Null for non-yielder functions.
	const std::map<int, DispatchDesc>* dispatches = nullptr;
	// Depth of the enclosing dispatch loop from the region's scaffold position
	// (the structurer's live frame count is added per-branch).
	int dispatch_base_depth = 0;
	// Blocks >= this are synthesized dispatch trampolines with no body.
	int num_real_blocks = 0;

	pc_t block_final(int b) const
	{
		return b + 1 < (int)num_real_blocks ? (*starts)[b + 1] - 1 : final_pc;
	}

	void emit_block(int) override { state->wasm->emitBlock(); }
	void emit_loop(int) override { state->wasm->emitLoop(); }
	void emit_if() override { state->wasm->emitIf(); }
	void emit_else() override { state->wasm->emitElse(); }
	void emit_end() override { state->wasm->emitEnd(); }
	void emit_br(int depth, int) override { state->wasm->emitBr(depth); }
	void emit_br_if(int depth, int) override { state->wasm->emitBrIf(depth); }
	void emit_i32_eqz() override { state->wasm->emitI32Eqz(); }

	void emit_body(int b) override
	{
		WasmAssembler& wasm = *state->wasm;

		// Synthesized dispatch trampolines have no body.
		if (b >= num_real_blocks)
			return;

		for (int i = (*starts)[b]; i <= (*body_last)[b]; i++)
		{
			state->pc = i;
			int command = script->zasm[i].command;

#ifndef NDEBUG
			wasm.emitI32Const(i);
			wasm.emitDrop();
#endif

			if (state->runtime_debugging && !command_uses_comparison_result(command))
			{
				wasm.emitI32Const(i);
				emit_get_sp(*state);
				wasm.emitCall(state->f_idx_runtime_debug);
			}

			if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
			{
				// Emit the operand snapshot (if any); the consumers emit as
				// they are reached - SETx below, a GOTOx terminator in
				// emit_cond. A loose compare with no consumers emits nothing.
				auto it = cmp_groups->find(i);
				if (it != cmp_groups->end())
					emit_cmp_capture(*state, script, it->second);
				continue;
			}

			if (auto it = consumer_to_cmp->find(i); it != consumer_to_cmp->end())
			{
				if (command == NOP)
					continue;
				// GOTOx consumers are always block terminators (emit_cond).
				CHECK(!command_is_goto_consumer(command));
				emit_cmp_set_consumer(*state, script, cmp_groups->at(it->second), i);
				continue;
			}

			switch (command)
			{
				case NOP:
					break;

				case CALLFUNC:
					// In a non-yielder every callee is non-yielding (a yielding
					// callee would make this function yield), so this is always
					// a native call.
					compile_callfunc_native(*state, *structured_zasm, *function_id_to_idx, i, script->zasm[i].arg1);
					break;

				case GOTO:
					// Only the cross-function (tail-call) variant is body-emitted;
					// intra-function GOTOs are handled by the structurer.
					compile_goto_tailcall(*state, *structured_zasm, *function_id_to_idx, script->zasm[i].arg1);
					break;

				case QUIT:
					compile_quit(*state, script, i);
					break;

				case RETURNFUNC:
					// Totally ignore the return pc on the data stack; the wasm
					// call stack handles the return.
					emit_sp_flush(*state);
					wasm.emitReturn();
					break;

				default:
				{
					if (!command_is_compiled(command))
					{
						// Batch consecutive uncompiled commands into one
						// interpreter call, up to the end of this block.
						int count = 1;
						for (int j = i + 1; j <= (*body_last)[b]; j++)
						{
							if (command_is_compiled(script->zasm[j].command))
								break;
							count += 1;
						}
						compile_command_interpreter(*state, script, i, count);
						i += count - 1;
						break;
					}

					compile_plain_command(*state, script, i);
				}
			}
		}

		int gpc = (*goto_debug_pc)[b];
		if (gpc >= 0)
		{
			state->pc = gpc;
#ifndef NDEBUG
			wasm.emitI32Const(gpc);
			wasm.emitDrop();
#endif
			if (state->runtime_debugging)
			{
				wasm.emitI32Const(gpc);
				emit_get_sp(*state);
				wasm.emitCall(state->f_idx_runtime_debug);
			}
		}

		if (ffff_tail && b + 1 == (int)starts->size())
		{
			// This will run some 0xFFFF specific code, then trap.
			compile_command_interpreter(*state, script, script->size - 1, 1);
		}
	}

	void emit_cond(int b) override
	{
		// The block ends in a GOTOx comparison consumer; leave its take-branch
		// value (an i32, nonzero => branch taken) on the stack.
		pc_t term_pc = block_final(b);
		state->pc = term_pc;
		emit_cmp_goto_value(*state, script, cmp_groups->at(consumer_to_cmp->at(term_pc)), term_pc);
	}

	void emit_dispatch(int b, int ctx_depth) override
	{
		// Only the yielder's structured regions use Term::Dispatch; a
		// non-yielding function has no dispatch to branch to.
		CHECK(dispatches);
		WasmAssembler& wasm = *state->wasm;
		const DispatchDesc& d = dispatches->at(b);

		if (d.emit_debug)
		{
			state->pc = d.pc;
#ifndef NDEBUG
			wasm.emitI32Const(d.pc);
			wasm.emitDrop();
#endif
			if (state->runtime_debugging)
			{
				wasm.emitI32Const(d.pc);
				emit_get_sp(*state);
				wasm.emitCall(state->f_idx_runtime_debug);
			}
		}

		if (d.is_return)
		{
			// The yielder RETURNFUNC lowering (see compile_function): a return
			// at the root call frame ends the script; otherwise pop the
			// continuation block id from the return stack and dispatch to it.
			wasm.emitGlobalGet(state->g_idx_ret_stack_index);
			wasm.emitI32Eqz();
			wasm.emitIf();
			{
				wasm.emitI32Const(RUNSCRIPT_JIT_QUIT);
				wasm.emitCall(state->f_idx_set_return_value);
				wasm.emitUnreachable();
			}
			wasm.emitEnd();

			// g_idx_ret_stack_index -= 1
			modify_global_idx(wasm, state->g_idx_ret_stack_index, -1);
			emit_trace_call_stack_pop(*state);

			// g_idx_ret_stack[g_idx_ret_stack_index]
			wasm.emitGlobalGet(state->g_idx_ret_stack);
			wasm.emitGlobalGet(state->g_idx_ret_stack_index);
			wasm.emitI32Const(4);
			wasm.emitI32Mul();
			wasm.emitI32Add();
			wasm.emitI32Load(0);

			wasm.emitGlobalSet(state->g_idx_target_block_id);
		}
		else
		{
			wasm.emitI32Const(d.value);
			wasm.emitGlobalSet(state->g_idx_target_block_id);
		}

		// Branch out to the enclosing dispatch loop.
		wasm.emitBr(dispatch_base_depth + ctx_depth);
	}
};

// Refine a block partition so a control op is always the last instruction of
// its block: add a split after every control op the CFG doesn't split on
// (QUIT/RETURNFUNC never split a block, and a branch to a function's own
// entry is treated as a recursive call by zasm_construct_cfg, so it doesn't
// split either). `starts` holds the CFG's block starts on entry; sorted and
// unique on return.
static void refine_block_partition(const zasm_script* script, pc_t start_pc, pc_t final_pc, std::vector<pc_t>& starts)
{
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;
		bool is_control = command == GOTO || command_is_goto_consumer(command) ||
						  command == QUIT || command == RETURNFUNC;
		if (is_control && i + 1 <= final_pc)
			starts.push_back(i + 1);
	}
	std::sort(starts.begin(), starts.end());
	starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
}

static int partition_block_id(const std::vector<pc_t>& starts, int pc)
{
	auto it = std::lower_bound(starts.begin(), starts.end(), (pc_t)pc);
	if (it == starts.end() || *it != (pc_t)pc)
		return -1;
	return it - starts.begin();
}

static pc_t partition_block_final(const std::vector<pc_t>& starts, pc_t final_pc, int b)
{
	return b + 1 < (int)starts.size() ? starts[b + 1] - 1 : final_pc;
}

// Comparison groups: map each compare and each of its consumers in
// [start_pc, final_pc], so SETx consumers emit in the body and a GOTOx
// terminator can re-derive its condition in emit_cond. A consumer run may
// span blocks (e.g. `COMPAREV; GOTOTRUE a; GOTOFALSE b`), which the per-pc
// map handles uniformly. Returns false if a run escapes the range.
static bool collect_cmp_groups(const zasm_script* script, pc_t start_pc, pc_t final_pc, std::map<pc_t, CmpGroup>& cmp_groups, std::map<pc_t, pc_t>& consumer_to_cmp)
{
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;
		if (command != COMPARER && command != COMPAREV && command != COMPAREV2)
			continue;

		CmpGroup g = analyze_comparison(script, i);
		if (g.consumers.empty())
			continue;

		if (g.consumers.back() > final_pc)
			return false;

		for (pc_t cj : g.consumers)
			consumer_to_cmp[cj] = i;

		i = g.consumers.back();
		cmp_groups.emplace(g.cmp_pc, std::move(g));
	}

	return true;
}

static std::optional<WasmAssembler> compile_function_structured(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, pc_t fn_id, const std::map<pc_t, pc_t>& function_id_to_idx)
{
	const auto& fn = structured_zasm.functions[fn_id];
	pc_t start_pc = fn.start_pc;
	pc_t final_pc = fn.final_pc;

	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;

		// A non-yielding function can't contain these; bail defensively.
		if (command_is_suspend(command))
			return std::nullopt;
		// compile_callfunc_native requires a known function entry.
		if (command == CALLFUNC && !structured_zasm.start_pc_to_function.contains(script->zasm[i].arg1))
			return std::nullopt;
	}

	auto cfg = zasm_construct_cfg(script, {{start_pc, final_pc}});
	std::vector<pc_t> starts = cfg.block_starts;
	refine_block_partition(script, start_pc, final_pc, starts);

	int n = starts.size();
	auto block_id_of = [&](int pc) { return partition_block_id(starts, pc); };
	auto block_final = [&](int b) { return partition_block_final(starts, final_pc, b); };

	std::map<pc_t, CmpGroup> cmp_groups;
	std::map<pc_t, pc_t> consumer_to_cmp;
	// A consumer run that leaves the function; shouldn't happen.
	if (!collect_cmp_groups(script, start_pc, final_pc, cmp_groups, consumer_to_cmp))
		return std::nullopt;

	// Classify each block's terminator (see wasm_structurer.h).
	std::vector<BlockInfo> infos(n);
	std::vector<int> body_last(n);
	std::vector<int> goto_debug_pc(n, -1);
	bool ffff_tail = false;
	for (int b = 0; b < n; b++)
	{
		pc_t bfinal = block_final(b);
		int command = script->zasm[bfinal].command;
		int arg1 = script->zasm[bfinal].arg1;
		BlockInfo& bi = infos[b];
		body_last[b] = bfinal;

		if (command == GOTO)
		{
			if (arg1 >= (int)start_pc && arg1 <= (int)final_pc)
			{
				int target = block_id_of(arg1);
				if (target < 0)
					return std::nullopt;

				bi.term = Term::Uncond;
				bi.succ_true = target;
				body_last[b] = (int)bfinal - 1; // the structurer emits the branch
				goto_debug_pc[b] = (int)bfinal;
			}
			else if (structured_zasm.start_pc_to_function.contains(arg1))
			{
				bi.term = Term::Exit; // the body emits the tail-call + return
			}
			else
				return std::nullopt;
		}
		else if (command_is_goto_consumer(command))
		{
			if (!consumer_to_cmp.contains(bfinal))
				return std::nullopt; // a GOTOx with no reaching compare

			int target = arg1 >= (int)start_pc && arg1 <= (int)final_pc ? block_id_of(arg1) : -1;
			if (target < 0 || b + 1 >= n)
				return std::nullopt;

			bi.term = Term::Cond;
			bi.succ_true = target;
			bi.succ_false = b + 1;
			body_last[b] = (int)bfinal - 1; // emit_cond emits the condition
		}
		else if (command == QUIT || command == RETURNFUNC)
		{
			bi.term = Term::Exit; // the body emits the trap/return
		}
		else if (b + 1 < n)
		{
			// No control op; the block exists because the next pc is a branch
			// target. Plain fall-through.
			bi.term = Term::Uncond;
			bi.succ_true = b + 1;
		}
		else
		{
			// Falls off the function's end. For the entry function of a script
			// whose zasm ends right after (global init scripts have no QUIT),
			// run the script-end 0xFFFF handling; otherwise this is simply an
			// implicit return.
			bi.term = Term::Exit;
			if (fn_id == 0 && script->size - 1 == final_pc + 1 && script->zasm[script->size - 1].command == 0xFFFF)
				ffff_tail = true;
		}
	}

	WasmAssembler wasm;
	state.wasm = &wasm;
	wasm.reserve((final_pc - start_pc + 1) * 24); // see compile_function

	// Load sp into its local (see l_idx_sp_local).
	emit_sp_reload(state);
	emit_ptr_locals_init(state);

	StructuredFnSink sink{};
	sink.state = &state;
	sink.script = script;
	sink.structured_zasm = &structured_zasm;
	sink.function_id_to_idx = &function_id_to_idx;
	sink.starts = &starts;
	sink.body_last = &body_last;
	sink.goto_debug_pc = &goto_debug_pc;
	sink.cmp_groups = &cmp_groups;
	sink.consumer_to_cmp = &consumer_to_cmp;
	sink.final_pc = final_pc;
	sink.ffff_tail = ffff_tail;
	sink.num_real_blocks = n;

	// Irreducibility is detected before anything is emitted.
	WasmStructurer structurer(n, 0, infos);
	if (!structurer.analyze())
		return std::nullopt;
	structurer.emit(sink);

	// An Exit block can fall off the function's end (an implicit return).
	emit_sp_flush(state);
	wasm.emitEnd(); // Function end.

	return wasm;
}

// Yielder loop structuring.
//
// The yielder must keep the loop-switch for its call/return/resume edges
// (runtime-valued targets), but a natural loop whose body contains no
// dispatch entry point - no wait, no call to a yielding function; a loop that
// always runs to completion without suspending - can be compiled as a real
// wasm loop nested inside the dispatch. The loop's interior blocks are
// "absorbed": they lose their scaffold frames (their br_table entries go
// dead - nothing dispatches to them at runtime), and the whole region is
// emitted structured in the header's scaffold segment. Edges leaving the
// region (breaks past the loop, returns) become Term::Dispatch: set
// g_idx_target_block_id and branch out to the dispatch loop.

struct YielderRegionPlan
{
	pc_t header_block; // global cfg block id; keeps its scaffold frame
	pc_t tail_block;   // last global cfg block id absorbed into the region
	pc_t start_pc;
	pc_t final_pc;

	// Local structured-region data, same shape compile_function_structured
	// builds for a whole function. infos may have synthesized dispatch
	// trampolines appended past starts.size().
	std::vector<pc_t> starts;
	std::vector<BlockInfo> infos;
	std::vector<int> body_last;
	std::vector<int> goto_debug_pc;
	std::map<int, StructuredFnSink::DispatchDesc> dispatches;
	std::map<pc_t, CmpGroup> cmp_groups;
	std::map<pc_t, pc_t> consumer_to_cmp;

	// Analyzed by detect_yielder_regions (whose reducibility check accepts the
	// region); emit_yielder_region reuses it instead of re-analyzing.
	std::unique_ptr<WasmStructurer> structurer;
};

// Builds the local structured-region plan for candidate loop [header..tail]
// (global cfg block ids). Returns nullopt for a shape the classifier doesn't
// model; the caller then just leaves that loop on the dispatch.
static std::optional<YielderRegionPlan> plan_yielder_region(const zasm_script* script, const ZasmCFG& cfg, pc_t header_block, pc_t tail_block)
{
	pc_t num_global_blocks = cfg.block_starts.size();

	YielderRegionPlan plan{};
	plan.header_block = header_block;
	plan.tail_block = tail_block;
	plan.start_pc = cfg.block_starts[header_block];
	plan.final_pc = tail_block + 1 < num_global_blocks ? cfg.block_starts[tail_block + 1] - 1 : cfg.final_pc;

	pc_t start_pc = plan.start_pc;
	pc_t final_pc = plan.final_pc;

	// Refined partition, same as compile_function_structured's.
	std::vector<pc_t>& starts = plan.starts;
	for (pc_t b = header_block; b <= tail_block; b++)
		starts.push_back(cfg.block_starts[b]);
	refine_block_partition(script, start_pc, final_pc, starts);

	int n = starts.size();
	auto block_id_of = [&](int pc) { return partition_block_id(starts, pc); };
	auto block_final = [&](int b) { return partition_block_final(starts, final_pc, b); };

	// The caller's eligibility scan already rejected runs escaping the
	// region; re-check defensively.
	if (!collect_cmp_groups(script, start_pc, final_pc, plan.cmp_groups, plan.consumer_to_cmp))
		return std::nullopt;

	// The dispatch value naming a global cfg block (same "+1" convention as
	// the flat lowering; the br_table's entry list maps values to depths).
	auto dispatch_value = [&](int target_pc) -> int {
		return (int)cfg.block_id_from_start_pc(target_pc) + 1;
	};

	plan.infos.resize(n);
	// Trampolines are appended during classification; reserve so BlockInfo
	// writes through plan.infos[b] stay valid.
	plan.infos.reserve(n * 3 + 2);
	plan.body_last.resize(n);
	plan.goto_debug_pc.assign(n, -1);

	auto add_trampoline = [&](int value, pc_t pc) -> int {
		int id = plan.infos.size();
		plan.infos.push_back({Term::Dispatch, -1, -1});
		plan.dispatches[id] = {value, pc, false, false};
		return id;
	};
	// The region's fall-out target: the global block right after it.
	auto fall_out_value = [&]() -> int {
		if (final_pc + 1 >= (pc_t)script->size || !cfg.contains_block_start(final_pc + 1))
			return -1;
		return dispatch_value(final_pc + 1);
	};

	for (int b = 0; b < n; b++)
	{
		pc_t bfinal = block_final(b);
		int command = script->zasm[bfinal].command;
		int arg1 = script->zasm[bfinal].arg1;
		plan.body_last[b] = bfinal;

		if (command == GOTO)
		{
			if (arg1 >= (int)start_pc && arg1 <= (int)final_pc)
			{
				int target = block_id_of(arg1);
				if (target < 0)
					return std::nullopt;
				plan.infos[b] = {Term::Uncond, target, -1};
				plan.body_last[b] = (int)bfinal - 1;
				plan.goto_debug_pc[b] = (int)bfinal;
			}
			else
			{
				// Leaves the region; the target is a block elsewhere in the
				// yielder (cross-function GOTOs out of the yielder are
				// rejected before compilation).
				if (arg1 < 0 || !cfg.contains_block_start(arg1))
					return std::nullopt;
				plan.infos[b] = {Term::Dispatch, -1, -1};
				plan.dispatches[b] = {dispatch_value(arg1), bfinal, false, true};
				plan.body_last[b] = (int)bfinal - 1;
			}
		}
		else if (command_is_goto_consumer(command))
		{
			if (!plan.consumer_to_cmp.contains(bfinal))
				return std::nullopt; // a GOTOx with no reaching compare
			int taken;
			if (arg1 >= (int)start_pc && arg1 <= (int)final_pc)
			{
				taken = block_id_of(arg1);
				if (taken < 0)
					return std::nullopt;
			}
			else
			{
				if (arg1 < 0 || !cfg.contains_block_start(arg1))
					return std::nullopt;
				taken = add_trampoline(dispatch_value(arg1), bfinal);
			}
			int fall;
			if (b + 1 < n)
				fall = b + 1;
			else
			{
				int value = fall_out_value();
				if (value < 0)
					return std::nullopt;
				fall = add_trampoline(value, bfinal);
			}
			plan.infos[b] = {Term::Cond, taken, fall};
			plan.body_last[b] = (int)bfinal - 1;
		}
		else if (command == QUIT)
		{
			plan.infos[b] = {Term::Exit, -1, -1}; // the body emits the trap
		}
		else if (command == RETURNFUNC)
		{
			// The continuation is popped from the return stack at runtime.
			plan.infos[b] = {Term::Dispatch, -1, -1};
			plan.dispatches[b] = {-1, bfinal, true, true};
			plan.body_last[b] = (int)bfinal - 1;
		}
		else if (b + 1 < n)
		{
			plan.infos[b] = {Term::Uncond, b + 1, -1};
		}
		else
		{
			// Falls out of the region.
			int value = fall_out_value();
			if (value < 0)
				return std::nullopt;
			plan.infos[b] = {Term::Dispatch, -1, -1};
			plan.dispatches[b] = {value, block_final(b), false, false};
		}
	}

	return plan;
}

// Emits an accepted region as structured wasm at its scaffold position.
static void emit_yielder_region(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, const std::map<pc_t, pc_t>& function_id_to_idx, const YielderRegionPlan& plan, int dispatch_base_depth)
{
	StructuredFnSink sink{};
	sink.state = &state;
	sink.script = script;
	sink.structured_zasm = &structured_zasm;
	sink.function_id_to_idx = &function_id_to_idx;
	sink.starts = &plan.starts;
	sink.body_last = &plan.body_last;
	sink.goto_debug_pc = &plan.goto_debug_pc;
	sink.cmp_groups = &plan.cmp_groups;
	sink.consumer_to_cmp = &plan.consumer_to_cmp;
	sink.final_pc = plan.final_pc;
	sink.ffff_tail = false;
	sink.dispatches = &plan.dispatches;
	sink.dispatch_base_depth = dispatch_base_depth;
	sink.num_real_blocks = (int)plan.starts.size();

	plan.structurer->emit(sink);
}

// Finds the yielder's eligible loops. Conservative by construction: anything
// rejected simply stays on the loop-switch.
// Dispatch entry points: blocks the yielder's br_table can be asked to land on
// at runtime - function and script entries, wait/RUNGENFRZSCR resume points,
// and the return continuations of calls to yielding functions. Yielder regions
// must not absorb these (see detect_yielder_regions), and every pc the
// interpreter can be resumed at maps to one, which is what lets a mid-run
// script be adopted by the JIT (see jit_create_script_impl).
static std::set<pc_t> compute_dispatch_entries(const zasm_script* script, const StructuredZasm& structured_zasm, const ZasmCFG& cfg, const std::vector<std::pair<pc_t, pc_t>>& pc_ranges)
{
	std::set<pc_t> dispatch_entries;
	for (auto [start_pc, final_pc] : pc_ranges)
		dispatch_entries.insert(cfg.block_id_from_start_pc(start_pc));
	for (auto sd : script->script_datas)
		if (cfg.contains_block_start(sd->pc))
			dispatch_entries.insert(cfg.block_id_from_start_pc(sd->pc));
	for (auto [start_pc, final_pc] : pc_ranges)
	{
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			int command = script->zasm[i].command;
			bool resume_after = command_is_suspend(command);
			if (command == CALLFUNC)
			{
				auto it = structured_zasm.start_pc_to_function.find(script->zasm[i].arg1);
				if (it != structured_zasm.start_pc_to_function.end() && structured_zasm.functions[it->second].may_yield)
					resume_after = true; // the call's return continuation
			}
			if (resume_after && i + 1 <= final_pc && cfg.contains_block_start(i + 1))
				dispatch_entries.insert(cfg.block_id_from_start_pc(i + 1));
		}
	}
	return dispatch_entries;
}

static std::vector<YielderRegionPlan> detect_yielder_regions(const zasm_script* script, const StructuredZasm& structured_zasm, const ZasmCFG& cfg, const std::vector<std::pair<pc_t, pc_t>>& pc_ranges)
{
	pc_t num_blocks = cfg.block_starts.size();
	auto block_final = [&](pc_t b) -> pc_t {
		return b + 1 < num_blocks ? cfg.block_starts[b + 1] - 1 : cfg.final_pc;
	};

	// These must keep their scaffold frames, so no region may absorb them (a
	// region may still *start* at one - dispatching to the header lands at the
	// top of the structured loop).
	std::set<pc_t> dispatch_entries = compute_dispatch_entries(script, structured_zasm, cfg, pc_ranges);

	// Candidate loops: for each backward edge b -> h, take the largest such b
	// per header h. Also record each block's predecessor bounds; the
	// single-entry check below only needs "do all of an interior block's
	// predecessors lie within the region", which min/max answer in O(1).
	std::map<pc_t, pc_t> header_to_tail;
	std::vector<pc_t> min_pred(num_blocks, (pc_t)-1);
	std::vector<pc_t> max_pred(num_blocks, 0);
	for (pc_t b = 0; b < num_blocks; b++)
	{
		for (pc_t e : cfg.block_edges[b])
		{
			if (e <= b)
				header_to_tail[e] = std::max(header_to_tail[e], b);
			min_pred[e] = std::min(min_pred[e], b);
			max_pred[e] = std::max(max_pred[e], b);
		}
	}

	std::vector<YielderRegionPlan> plans;
	for (auto [h, t] : header_to_tail)
	{
		// Overlapping an accepted region: it's an inner loop, and the region's
		// own structurer already handles it.
		if (!plans.empty() && h <= plans.back().tail_block)
			continue;
		// The last block of the yielder is excluded so the fall-off-the-end
		// script exit (see the 0xFFFF handling below) stays reachable.
		if (t + 1 >= num_blocks)
			continue;

		// Must lie within a single function's range.
		pc_t h_pc = cfg.block_starts[h];
		pc_t t_final = block_final(t);
		bool same_range = false;
		for (auto [start_pc, final_pc] : pc_ranges)
			if (h_pc >= start_pc && t_final <= final_pc)
				same_range = true;
		if (!same_range)
			continue;

		// Interior blocks must not be dispatch entries.
		bool ok = true;
		for (pc_t x = h + 1; x <= t && ok; x++)
			if (dispatch_entries.contains(x))
				ok = false;

		// The loop must run to completion without suspending: no waits, no
		// frozen-script runs, no calls to yielding (or unknown) functions.
		// Comparison-consumer runs must not escape the region.
		for (pc_t i = h_pc; i <= t_final && ok; i++)
		{
			int command = script->zasm[i].command;
			if (command_is_suspend(command))
				ok = false;
			else if (command == CALLFUNC)
			{
				auto it = structured_zasm.start_pc_to_function.find(script->zasm[i].arg1);
				if (it == structured_zasm.start_pc_to_function.end() || structured_zasm.functions[it->second].may_yield)
					ok = false;
			}
			else if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
			{
				CmpGroup g = analyze_comparison(script, i);
				if (!g.consumers.empty())
				{
					if (g.consumers.back() > t_final)
						ok = false;
					else
						i = g.consumers.back();
				}
			}
		}

		// Single-entry: no edges from outside the region into its interior.
		// (min_pred of (pc_t)-1 means "no predecessors" and passes trivially.)
		for (pc_t x = h + 1; x <= t && ok; x++)
			if (min_pred[x] < h || max_pred[x] > t)
				ok = false;
		if (!ok)
			continue;

		auto plan = plan_yielder_region(script, cfg, h, t);
		if (!plan)
			continue;
		plan->structurer = std::make_unique<WasmStructurer>((int)plan->infos.size(), 0, plan->infos);
		if (!plan->structurer->analyze())
			continue; // irreducible

		plans.push_back(std::move(*plan));
	}

	// A comparison outside every region must not have a consumer run leaking
	// into one (the flat lowering would try to close absorbed frames). Drop
	// any region a run reaches into. plans is sorted by start_pc, so a single
	// cursor tracks the region ahead of the scan position.
	std::set<size_t> dropped;
	size_t next_plan = 0;
	for (auto [start_pc, final_pc] : pc_ranges)
	{
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			while (next_plan < plans.size() && plans[next_plan].final_pc < i)
				next_plan++;
			if (next_plan < plans.size() && i >= plans[next_plan].start_pc)
			{
				// Inside a region; its own classification already vetted it.
				i = plans[next_plan].final_pc;
				continue;
			}
			int command = script->zasm[i].command;
			if (command != COMPARER && command != COMPAREV && command != COMPAREV2)
				continue;
			CmpGroup g = analyze_comparison(script, i);
			if (g.consumers.empty())
				continue;
			if (next_plan < plans.size() && g.consumers.back() >= plans[next_plan].start_pc)
				dropped.insert(next_plan);
			i = g.consumers.back();
		}
	}
	if (!dropped.empty())
	{
		std::vector<YielderRegionPlan> kept;
		for (size_t r = 0; r < plans.size(); r++)
			if (!dropped.contains(r))
				kept.push_back(std::move(plans[r]));
		plans = std::move(kept);
	}

	return plans;
}

static WasmAssembler compile_function(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, std::set<pc_t> function_ids, const std::map<pc_t, pc_t>& function_id_to_idx, bool may_yield, std::map<pc_t, uint32_t>* resume_dispatch_out = nullptr)
{
	WasmAssembler wasm;
	state.wasm = &wasm;

	uint8_t g_idx_ret_stack = state.g_idx_ret_stack;
	uint8_t g_idx_ret_stack_index = state.g_idx_ret_stack_index;
	uint8_t g_idx_wait_index = state.g_idx_wait_index;
	uint8_t g_idx_target_block_id = state.g_idx_target_block_id;

	std::vector<std::pair<pc_t, pc_t>> pc_ranges;
	size_t num_instructions = 0;
	for (auto fn_id : function_ids)
	{
		auto& fn = structured_zasm.functions[fn_id];
		pc_ranges.emplace_back(fn.start_pc, fn.final_pc);
		num_instructions += fn.final_pc - fn.start_pc + 1;
	}
	// ~24 bytes of wasm per ZASM instruction (measured); over-reserving is fine,
	// the buffer is freed right after the module is assembled.
	wasm.reserve(num_instructions * 24);
	auto cfg = zasm_construct_cfg(script, pc_ranges);

	// Record where the yielder can be resumed, for adopting mid-run scripts
	// (see jit_create_script_impl). Dispatch values are (cfg block id + 1),
	// matching what the wait/call codegen below stores.
	if (may_yield && resume_dispatch_out)
		for (pc_t b : compute_dispatch_entries(script, structured_zasm, cfg, pc_ranges))
			(*resume_dispatch_out)[cfg.block_starts[b]] = b + 1;

	// All control flow is converted to a simple, but slow, loop-switch construct: a giant switch case,
	// implemented with "loop", "br_table", and a "block" for every block in the CFG.
	//
	// See: https://medium.com/leaningtech/solving-the-structured-control-flow-problem-once-and-for-all-5123117b1ee2#:~:text=A%20universal%20but%20inefficient%20solution
	//
	// Non-yielding functions don't use this - they get real structured control flow via
	// compile_function_structured (which falls back to this lowering for irreducible CFGs, and
	// -no-jit-wasm-structured forces it everywhere). The yielder must keep the dispatch for
	// its call/return/resume edges, whose targets are runtime values; but its *static*
	// non-suspending loops are compiled as real wasm loops nested inside the dispatch - see
	// the region detection just below, and https://dl.acm.org/doi/pdf/10.1145/3547621 for
	// the general technique.

	// -jit-wasm-structured-yielder: compile the yielder's non-suspending loops
	// as real wasm loops nested inside the dispatch (see YielderRegionPlan).
	// With no accepted regions this lowering is identical to the flat one.
	static bool yielder_structured = get_flag_bool("-jit-wasm-structured-yielder").value_or(true);
	std::vector<YielderRegionPlan> regions;
	if (may_yield && yielder_structured)
		regions = detect_yielder_regions(script, structured_zasm, cfg, pc_ranges);

	pc_t num_blocks = cfg.block_starts.size();

	// Interior blocks of accepted regions are absorbed: no scaffold frame, and
	// their br_table entries go dead (nothing dispatches to them at runtime).
	std::vector<uint8_t> absorbed(num_blocks, 0);
	std::vector<pc_t> absorbed_header(num_blocks, 0);
	std::map<pc_t, const YielderRegionPlan*> region_by_header;
	for (auto& r : regions)
	{
		region_by_header[r.header_block] = &r;
		for (pc_t b = r.header_block + 1; b <= r.tail_block; b++)
		{
			absorbed[b] = 1;
			absorbed_header[b] = r.header_block;
		}
	}
	// Scaffold rank of each non-absorbed block. In the flat case rank == id,
	// and every depth/entry below reduces to the classic loop-switch values.
	std::vector<int> rank(num_blocks, -1);
	int num_frames = 0;
	for (pc_t b = 0; b < num_blocks; b++)
		if (!absorbed[b])
			rank[b] = num_frames++;

	// Load sp into its local (see l_idx_sp_local); the caller/run wrapper keeps
	// the global current across function boundaries.
	emit_sp_reload(state);
	emit_ptr_locals_init(state);

	// Handle jumping to the correct initial block when calling the entry function.
	if (script->name == "@single")
	{
		wasm.emitGlobalGet(g_idx_target_block_id);
		wasm.emitI32Eqz();
		wasm.emitIf();

		for (auto sd : script->script_datas)
		{
			if (!cfg.contains_block_start(sd->pc))
				continue;

			auto block_id = cfg.block_id_from_start_pc(sd->pc);

			wasm.emitI32Const(sd->pc);
			wasm.emitGlobalGet(state.g_idx_start_pc);
			wasm.emitI32Eq();

			wasm.emitIf();
			wasm.emitI32Const(block_id + 1);
			wasm.emitGlobalSet(g_idx_target_block_id);
			wasm.emitEnd();
		}

		wasm.emitEnd();
	}

	wasm.emitLoop();
	for (pc_t i = 0; i < num_blocks; i++)
	{
		if (!absorbed[i])
			wasm.emitBlock();
	}

	// Jump to the next block.
	// For the first execution of this function, this initially jumps to the first block.
	// For subsequent executions (re-entering after `WaitX`), it initially jumps to the last block yielded from.
	// For subsequent loop iterations in the same call to the script, it jumps to the block specified by the
	// previous block. This is what handles branches (including if, calls, loops) in the CFG. If a block simply
	// falls through to the next, it does so directly (no additional loop iteration).
	//
	// Dispatch values are (cfg block id + 1); the entry list maps each value to
	// the branch depth landing at that block's code, which is (scaffold rank +
	// 1) because frames close innermost-first while code advances forward.
	wasm.emitBlock();
	wasm.emitGlobalGet(g_idx_target_block_id);
	wasm.emitBrTable();
	// Plus 2, because of the inner-most block (this most recently emitted one), and
	// the outer-most loop.
	pc_t br_table_len = num_blocks + 2;
	wasm.emitVarU32(br_table_len);
	for (pc_t v = 0; v < br_table_len; v++)
	{
		if (v == 0)
			wasm.emitVarU32(0);
		else if (v == num_blocks + 1)
			wasm.emitVarU32(num_frames + 1); // the dispatch loop itself
		else
		{
			pc_t j = v - 1;
			// Absorbed blocks are never dispatched to; route their dead
			// entries at the region header, which is harmless and in range.
			pc_t land = absorbed[j] ? absorbed_header[j] : j;
			wasm.emitVarU32(rank[land] + 1);
		}
	}
	wasm.emitVarU32(0);
	wasm.emitEnd();

	// (cfg block id of the current block) + 1 while emitting its code.
	pc_t current_block_index = 0;
	// (scaffold rank of the current block) + 1 while emitting its code. The
	// dispatch loop's branch depth from anywhere is num_frames - current_rank.
	int current_rank = 0;

	// During block b's body current_block_index == b + 1, so the next block
	// start is always block_starts[current_block_index] - an O(1) check in this
	// per-instruction loop, where contains_block_start's out-of-line binary
	// search showed up in compile-time profiles.
	auto next_block_start = [&]() -> pc_t {
		return current_block_index < num_blocks ? cfg.block_starts[current_block_index] : (pc_t)-1;
	};

	for (auto [start_pc, final_pc] : pc_ranges)
	{
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			state.pc = i;

			if (i == next_block_start())
			{
				wasm.emitEnd();
				current_block_index += 1;
				current_rank += 1;

				// A structured region: emit the whole loop as real control
				// flow in this scaffold segment, then continue after it.
				if (auto it = region_by_header.find(current_block_index - 1); it != region_by_header.end())
				{
					const YielderRegionPlan& plan = *it->second;
					emit_yielder_region(state, script, structured_zasm, function_id_to_idx, plan, num_frames - current_rank);
					i = plan.final_pc; // the loop's ++ resumes right after the region
					current_block_index = plan.tail_block + 1;
					continue;
				}
			}

#ifndef NDEBUG
			wasm.emitI32Const(i);
			wasm.emitDrop();
#endif

			int command = script->zasm[i].command;
			int arg1 = script->zasm[i].arg1;

			if (state.runtime_debugging && !command_uses_comparison_result(command))
			{
				wasm.emitI32Const(i);
				emit_get_sp(state);
				wasm.emitCall(state.f_idx_runtime_debug);
			}

			if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
			{
				i = compile_comparison(state, script, cfg, num_frames, current_rank, current_block_index, i);
				continue;
			}

			if (command_is_wait(command))
			{
				ASSERT(may_yield);

				// j_instance->wait_index = current_block_index + 1
				{
					wasm.emitGlobalGet(g_idx_wait_index);
					wasm.emitI32Const(current_block_index + 1);
					wasm.emitI32Store();
				}

				// Wait commands normally yield back to the engine, however there are some
				// special cases where it does not. For example, when WAITFRAMESR arg is 0.
				// This will return to the run function, but only if actually waiting.
				compile_command_interpreter(state, script, i, 1, true);
				continue;
			}

			// RUNGENFRZSCR runs a nested generic script which yields frames via
			// ASYNCIFY - which can't unwind through a (non-instrumented) JIT frame.
			// Instead, yield cleanly back to the C++ driver (run_script), which runs
			// the frozen script and writes its result to `arg1` before resuming us.
			// Modeled on the wait yield above (store the resume block in wait_index
			// and return). Signalled via frozen_dest_reg != -1 rather than a return
			// code, because a clean return always reports RUNSCRIPT_OK to the poller.
			if (command == RUNGENFRZSCR)
			{
				ASSERT(may_yield);

				// j_instance->frozen_dest_reg = arg1 (where the driver writes the result)
				wasm.emitGlobalGet(state.g_idx_j_instance);
				wasm.emitI32Const(arg1);
				wasm.emitI32Store(offsetof(JittedScriptInstance, frozen_dest_reg));

				// j_instance->wait_index = current_block_index + 1 (resume at the next block)
				wasm.emitGlobalGet(g_idx_wait_index);
				wasm.emitI32Const(current_block_index + 1);
				wasm.emitI32Store();

				emit_sp_flush(state); // run_script reads/writes sp around the frozen run
				wasm.emitReturn();
				continue;
			}

			if (!command_is_compiled(command))
			{
				// Every command that is not compiled to WASM must go through the regular interpreter function.
				// In order to reduce function call overhead, we call into the interpreter function in batches.
				int uncompiled_command_count = 1;
				for (pc_t j = i + 1; j < (pc_t)script->size; j++)
				{
					if (command_is_compiled(script->zasm[j].command))
						break;
					if (j == next_block_start()) // O(1) contains_block_start; see above
						break;

					uncompiled_command_count += 1;
				}

				compile_command_interpreter(state, script, i, uncompiled_command_count);
				i += uncompiled_command_count - 1;
				continue;
			}

			// Every command here must be reflected in command_is_compiled!
			switch (command)
			{
				case NOP: break;

				case CALLFUNC:
				case GOTO:
				{
					if (command == CALLFUNC)
					{
						pc_t fn_id = structured_zasm.start_pc_to_function.at(arg1);
						if (!may_yield || !structured_zasm.functions[fn_id].may_yield)
						{
							// The callee is compiled as a separate WASM function, so we can simply
							// call it and be done with this instruction.
							compile_callfunc_native(state, structured_zasm, function_id_to_idx, i, arg1);
							continue;
						}

						// The function call is to some other may-yield function which is inlined in the yielder function.
						// Before we "call" it, we need to remember where to return to. To do that, we push the index of the
						// subsequent block to a return call stack. RETURNFUNC will later pop that block index.
						check_call_limit(state, arg1);

						// g_idx_ret_stack[g_idx_ret_stack_index] = return_block
						{
							wasm.emitGlobalGet(g_idx_ret_stack);
							wasm.emitGlobalGet(g_idx_ret_stack_index);
							wasm.emitI32Const(4);
							wasm.emitI32Mul();
							wasm.emitI32Add();
							wasm.emitI32Const(current_block_index + 1);
							wasm.emitI32Store(0);
						}

						// g_idx_ret_stack_index += 1
						modify_global_idx(wasm, g_idx_ret_stack_index, 1);
						emit_trace_call_stack_push(state, i);
					}
					else if (structured_zasm.start_pc_to_function.contains(arg1) &&
							 !function_ids.contains(structured_zasm.start_pc_to_function.at(arg1)))
					{
						// A cross-function GOTO; see compile_goto_tailcall.
						compile_goto_tailcall(state, structured_zasm, function_id_to_idx, arg1);
						continue;
					}

					// In both the "call within the yielder function case", or the "branch within a function" case,
					// we set the target block index and jump back to the loop-switch.

					size_t target_block_index = cfg.block_id_from_start_pc(arg1) + 1;
					wasm.emitI32Const(target_block_index);
					wasm.emitGlobalSet(g_idx_target_block_id);

					// Branch by jumping to start of loop-switch.
					wasm.emitBr(num_frames - current_rank);
				}
				break;

				case RETURNFUNC:
				{
					if (may_yield)
					{
						// Note: the return pc is on the stack, but we just ignore it and instead use
						// g_idx_ret_stack.
						if (command == RETURN)
						{
							add_sp(state, 1);
						}

						// A RETURNFUNC at the root call frame is the entry function ("void
						// run()") falling off its end, which ends the script. Report it as
						// RUNSCRIPT_JIT_QUIT (same as the x64 JIT and what the interpreter does
						// via `scommand = 0xFFFF`) so the caller runs script_exit_cleanup - which
						// runs owned-object destructors - before stopping. Otherwise
						// g_idx_ret_stack_index would decrement below zero, and check_call_limit's
						// unsigned compare would later read that as a huge depth and falsely trip
						// the call limit.
						wasm.emitGlobalGet(g_idx_ret_stack_index);
						wasm.emitI32Eqz();
						wasm.emitIf();
						{
							wasm.emitI32Const(RUNSCRIPT_JIT_QUIT);
							wasm.emitCall(state.f_idx_set_return_value);
							wasm.emitUnreachable();
						}
						wasm.emitEnd();

						// g_idx_ret_stack_index -= 1
						modify_global_idx(wasm, g_idx_ret_stack_index, -1);
						emit_trace_call_stack_pop(state);

						// g_idx_ret_stack[g_idx_ret_stack_index]
						{
							wasm.emitGlobalGet(g_idx_ret_stack);
							wasm.emitGlobalGet(g_idx_ret_stack_index);
							wasm.emitI32Const(4);
							wasm.emitI32Mul();
							wasm.emitI32Add();
							wasm.emitI32Load(0);
						}

						// Branch to callee by jumping to start of loop-switch.
						wasm.emitGlobalSet(g_idx_target_block_id);
						wasm.emitBr(num_frames - current_rank);
					}
					else
					{
						// Totally ignore the return pc on the stack.
						if (command == RETURN)
						{
							add_sp(state, 1);
						}

						// Call stack is implicitly handled by WASM.
						emit_sp_flush(state);
						wasm.emitReturn();
					}
				}
				break;

				case QUIT:
					compile_quit(state, script, i);
					break;

				default:
					compile_plain_command(state, script, i);
			}
		}
	}

	// Handles scripts that are just a single function. When they execute the last command (0xFFFF),
	// that exits the script.
	// Normally a QUIT does this. But, global init scripts don't end with a QUIT.
	if (function_ids.contains((0)) && script->size - 1 == structured_zasm.functions.at(0).final_pc + 1)
	{
		if (script->zasm[script->size - 1].command == 0xFFFF)
		{
			// This will run some 0xFFFF specific code, then trap.
			compile_command_interpreter(state, script, script->size - 1, 1);
		}
	}

	// Finish loop construct.
	wasm.emitBr(0); // Unconditional branch to top of loop.
	wasm.emitEnd(); // Loop end.

	wasm.emitEnd(); // Function end.

	return wasm;
}

void jit_startup_impl()
{
}

// The thread-safe half of compiling a chunk: everything up to (but not
// including) handing the module bytes to the browser. Touches only the script
// and local state, so it can run on any thread (see
// jit_precompile_scripts_impl); em_compile_wasm must stay on the main thread.
struct WasmCodegenResult
{
	std::string module_name;
	WasmModule wm;
	std::map<pc_t, uint32_t> resume_dispatch;
};

static bool wasm_codegen(zasm_script* script, WasmCodegenResult& out)
{
	if (script->size <= 1)
		return false;

	// STACKWRITEATVV_IF is not yet supported by the wasm codegen - always bail.
	//
	// RUNGENFRZSCR (generic frozen scripts) used to bail to the interpreter by
	// default: quests using it hit a nested-script crash (fixed - see
	// jit_can_start_script / em_create_wasm_handle) and then an rng desync in
	// terror_of_necromancy_demo6 (fixed - a script that started on the interpreter
	// was later adopted mid-run by the JIT, restarting it; see
	// jit_create_script_impl). With both fixed the JIT handles this content, so
	// the bail is off by default; -jit-wasm-bail-on-frozen-generic remains as an
	// escape hatch. It is coarse: any quest using generic frozen scripts compiles
	// its whole @single chunk with it.
	static bool bail_on_frozen_generic = get_flag_bool("-jit-wasm-bail-on-frozen-generic").value_or(false);
	for (size_t i = 0; i < script->size; i++)
	{
		int command = script->zasm[i].command;
		if (command == STACKWRITEATVV_IF)
		{
			error(script, "STACKWRITEATVV_IF unsupported", true);
			return false;
		}
		if (command == RUNGENFRZSCR && bail_on_frozen_generic)
			return false;
	}

	auto structured_zasm = zasm_construct_structured(script);
	if (!structured_zasm.is_modern_function_calling())
	{
		al_trace("[jit] NOT compiling zasm chunk (unexpected function call mode): %s id: %d size: %zu\n", script->name.c_str(), script->id, script->size);
		return false;
	}

	al_trace("[jit] compiling script: %s id: %d\n", script->name.c_str(), script->id);

	WasmCompiler comp{};
	CompilationState state{};

	state.runtime_debugging = script_debug_is_runtime_debugging() == 2;
	// Maintain ri->pc and the pc call stack only when a stack trace could actually
	// be produced (their only consumer), so non-tracing runs pay nothing per call.
	state.maintain_call_stack = FFCore.should_display_stack_traces();
	// -jit-wasm-sp-local: keep sp in a wasm local inside compiled code (see
	// l_idx_sp_local). The flag exists for A/B runs and bisection.
	state.sp_local = get_flag_bool("-jit-wasm-sp-local").value_or(true);
	state.ptr_locals = get_flag_bool("-jit-wasm-pointer-locals").value_or(true);
	state.inline_regs = get_flag_bool("-jit-wasm-inline-regs").value_or(true);

	state.f_idx_set_return_value = comp.builder.importFunction("set_return_value", 1, 0);
	state.f_idx_do_commands = comp.builder.importFunction("do_commands", 4, 1);
	state.f_idx_do_commands_async = comp.builder.importFunction("do_commands_async", 4, 1);
	state.f_idx_get_register = comp.builder.importFunction("get_register", 1, 1);
	state.f_idx_set_register = comp.builder.importFunction("set_register", 2, 0);
	state.f_idx_set_guarded_register = comp.builder.importFunction("set_guarded_register", 3, 0);
	state.f_idx_runtime_debug = comp.builder.importFunction("runtime_debug", 2, 0);
	state.f_idx_log_error = comp.builder.importFunction("log_error", 1, 0);
	state.f_idx_pod_read = comp.builder.importFunction("pod_read", 4, 1);
	state.f_idx_pod_write = comp.builder.importFunction("pod_write", 6, 0);
	state.f_idx_allocatemem = comp.builder.importFunction("allocatemem", 3, 1);
	state.f_idx_writepodarr = comp.builder.importFunction("writepodarr", 2, 0);

	// params
	state.l_idx_j_instance = 0;
	state.l_idx_ri = 1;
	state.l_idx_global_d = 2;
	state.l_idx_stack = 3;
	state.l_idx_ret_stack = 4;
	state.l_idx_ret_stack_index = 5;
	state.l_idx_wait_index = 6;
	state.l_idx_start_pc = 7;
	// params ended
	// Note: if you add another one, also add to the call to declareFunction.

	state.g_idx_j_instance = comp.builder.addGlobal("j_instance");
	state.g_idx_ri = comp.builder.addGlobal("ri");
	state.g_idx_global_d = comp.builder.addGlobal("global_d");
	state.g_idx_stack = comp.builder.addGlobal("stack");
	state.g_idx_ret_stack = comp.builder.addGlobal("ret_stack");
	state.g_idx_ret_stack_pc = comp.builder.addGlobal("ret_stack_pc");
	state.g_idx_ret_stack_index = comp.builder.addGlobal("ret_stack_index");
	state.g_idx_wait_index = comp.builder.addGlobal("wait_index");
	state.g_idx_sp = comp.builder.addGlobal("sp");
	state.g_idx_target_block_id = comp.builder.addGlobal("target_block_id");
	state.g_idx_start_pc = comp.builder.addGlobal("start_pc");

	pc_t fn_run_idx = comp.builder.declareFunction({WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32}, {});
	comp.builder.exportFunction(fn_run_idx, "run");

	auto yielding_fns = zasm_find_yielding_functions(script, structured_zasm);

	// A GOTO whose target is outside its own function is lowered as a tail-call into
	// the target function (see compile_function). This is only expected for the "~Init"
	// script of older (2.55-era) quests, whose entry tail-jumps over inlined init
	// function bodies to the "run" continuation. The tail-call lowering only works when
	// the target is a separately-compiled, non-yielding function's entry. Bail to the
	// interpreter for anything else (e.g. a GOTO into a yielding function inlined in the
	// yielder, or a GOTO that doesn't land on a function entry), rather than miscompile it.
	for (auto& fn : structured_zasm.functions)
	{
		for (pc_t i = fn.start_pc; i <= fn.final_pc && i < (pc_t)script->size; i++)
		{
			if (script->zasm[i].command != GOTO)
				continue;
			pc_t tgt = script->zasm[i].arg1;
			if (tgt >= fn.start_pc && tgt <= fn.final_pc)
				continue; // intra-function GOTO, handled by the loop-switch
			bool tgt_is_entry = structured_zasm.start_pc_to_function.contains(tgt);
			pc_t tgt_fn = tgt_is_entry ? structured_zasm.start_pc_to_function.at(tgt) : (pc_t)-1;
			// Source and target both in the yielder => same WASM unit, handled by its combined CFG.
			if (tgt_is_entry && yielding_fns.contains(fn.id) && yielding_fns.contains(tgt_fn))
				continue;
			if (!tgt_is_entry || yielding_fns.contains(fn.id) || yielding_fns.contains(tgt_fn))
			{
				error(script, fmt::format("unsupported cross-function GOTO at pc {} -> {}", (int)i, (int)tgt), true);
				return false;
			}
		}
	}

	pc_t fn_yielder_idx = yielding_fns.empty() ? -1 : comp.builder.declareFunction({}, {});

	std::map<pc_t, pc_t> function_id_to_idx;
	for (auto& fn : structured_zasm.functions)
	{
		function_id_to_idx[fn.id] = yielding_fns.contains(fn.id) ? fn_yielder_idx :
			comp.builder.declareFunction({}, {});
	}

	// First compile the "yielder" function, which inlines all the ZASM functions that may possibly yield.
	// There may be no yielding function, in which case the script will run fully in a single execution.
	std::map<pc_t, uint32_t> resume_dispatch;
	if (!yielding_fns.empty())
	{
		bool may_yield = true;
		auto wasm = compile_function(state, script, structured_zasm, yielding_fns, function_id_to_idx, may_yield, &resume_dispatch);
		comp.builder.defineFunction(fn_yielder_idx, {WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32}, wasm.finish());
		comp.builder.setFunctionName(fn_yielder_idx, "yielder");
	}

	// -jit-wasm-structured: compile non-yielding functions with real structured
	// control flow instead of the loop-switch. On by default; the flag exists to
	// A/B and to bisect a suspected structuring bug.
	static bool structure_enabled = get_flag_bool("-jit-wasm-structured").value_or(true);
	for (const auto& fn : structured_zasm.functions)
	{
		if (yielding_fns.contains(fn.id))
			continue;

		std::optional<WasmAssembler> wasm;
		if (structure_enabled)
			wasm = compile_function_structured(state, script, structured_zasm, fn.id, function_id_to_idx);
		if (!wasm)
		{
			bool may_yield = false;
			wasm = compile_function(state, script, structured_zasm, {fn.id}, function_id_to_idx, may_yield);
		}
		pc_t fidx = function_id_to_idx.at(fn.id);
		comp.builder.defineFunction(fidx, {WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32}, wasm->finish());
		// Note: fn.name() can't be used for the emptiness check - it returns the
		// placeholder "<empty>" for unnamed functions, which used to end up as
		// dozens of functions all literally named "<empty>" in the name section.
		if (fn._name.empty())
			comp.builder.setFunctionName(fidx, fmt::format("fn_{}", fn.id));
		else
			comp.builder.setFunctionName(fidx, fn.name());
	}

	// Create "run" entry function.
	{
		WasmAssembler wasm;

		// Put all params into globals.
		wasm.emitLocalGet(state.l_idx_j_instance);
		wasm.emitGlobalSet(state.g_idx_j_instance);
		wasm.emitLocalGet(state.l_idx_ri);
		wasm.emitGlobalSet(state.g_idx_ri);
		// g_idx_ret_stack_pc = &data.ret_stack. ri == &data.ref (the first member of
		// ScriptEngineData), so this is the same pc call stack create_stack_trace reads.
		if (state.maintain_call_stack)
		{
			wasm.emitLocalGet(state.l_idx_ri);
			wasm.emitI32Const(offsetof(ScriptEngineData, ret_stack));
			wasm.emitI32Add();
			wasm.emitGlobalSet(state.g_idx_ret_stack_pc);
		}
		wasm.emitLocalGet(state.l_idx_global_d);
		wasm.emitGlobalSet(state.g_idx_global_d);
		wasm.emitLocalGet(state.l_idx_stack);
		wasm.emitGlobalSet(state.g_idx_stack);
		wasm.emitLocalGet(state.l_idx_ret_stack);
		wasm.emitGlobalSet(state.g_idx_ret_stack);
		wasm.emitLocalGet(state.l_idx_wait_index);
		wasm.emitGlobalSet(state.g_idx_wait_index);
		wasm.emitLocalGet(state.l_idx_start_pc);
		wasm.emitGlobalSet(state.g_idx_start_pc);

		// g_idx_sp = ri->sp
		{
			wasm.emitLocalGet(state.l_idx_ri);
			wasm.emitI32Load(4*9); // ri->sp
			wasm.emitGlobalSet(state.g_idx_sp);
		}

		// g_idx_target_block_id = j_instance->wait_index
		{
			wasm.emitLocalGet(state.l_idx_wait_index);
			wasm.emitI32Load(0);
			wasm.emitGlobalSet(state.g_idx_target_block_id);
		}

		// g_idx_ret_stack_index = j_instance->call_stack_ret_index
		{
			wasm.emitLocalGet(state.l_idx_ret_stack_index);
			wasm.emitI32Load(0);
			wasm.emitGlobalSet(state.g_idx_ret_stack_index);
		}

		// Dispatch to the correct starting function.
		if (script->name == "@single")
		{
			// Handle calling the entry function on the first call (when j_instance->wait_index is 0).
			wasm.emitGlobalGet(state.g_idx_target_block_id);
			wasm.emitI32Eqz();
			wasm.emitIf();

			// First handle synchronous functions - scripts that never call `WaitX`.
			for (auto sd : script->script_datas)
			{
				pc_t fn_id = structured_zasm.start_pc_to_function.at(sd->pc);
				if (yielding_fns.contains(fn_id))
					continue;

				pc_t fn_idx = function_id_to_idx.at(fn_id);

				wasm.emitI32Const(sd->pc);
				wasm.emitGlobalGet(state.g_idx_start_pc);
				wasm.emitI32Eq();

				wasm.emitIf();
				wasm.emitCall(fn_idx);
				// The synchronous entry function ran to completion (it never waits), so
				// the script is done. Report it like the x64 JIT's root return (and the
				// yielding RETURNFUNC) so the engine runs script_exit_cleanup and stops -
				// otherwise the script's `doscript` flag stays set and a frozen run
				// (runGenericFrozenEngine) loops re-running it until the game ends.
				wasm.emitI32Const(RUNSCRIPT_JIT_QUIT);
				wasm.emitCall(state.f_idx_set_return_value);
				wasm.emitUnreachable();
				wasm.emitEnd();
			}

			wasm.emitEnd();

			// If this script is not synchronous, call the yielder function (which will again handle jumping to the
			// correct entry block for the first call).
			if (fn_yielder_idx != -1)
				wasm.emitCall(fn_yielder_idx);
		}
		else
		{
			// The first ZASM function we compiled marks the entry point. This is typically fn_yielder_idx,
			// but if their is no yielding then it's the first function compiled. Regardless, it always
			// maps to the original ZASM at pc 0.
			wasm.emitCall(function_id_to_idx.begin()->second);
		}

		// ri->sp = g_idx_sp
		{
			wasm.emitLocalGet(state.l_idx_ri);
			wasm.emitGlobalGet(state.g_idx_sp);
			wasm.emitI32Store(4*9); // ri->sp
		}

		// j_instance->call_stack_ret_index = g_idx_ret_stack_index
		{
			wasm.emitLocalGet(state.l_idx_ret_stack_index);
			wasm.emitGlobalGet(state.g_idx_ret_stack_index);
			wasm.emitI32Store(0);
		}

		wasm.emitEnd();

		comp.builder.defineFunction(fn_run_idx, {}, wasm.finish());
	}

	comp.builder.moduleName = script->name;
	auto wm = comp.finish();

	static bool write_to_disk = get_flag_bool("-jit-save-wasm").value_or(false);
	if (write_to_disk)
	{
		fs::path folder = fs::current_path() / "wasm" / get_filename(qstpath);
		fs::create_directories(folder);
		fs::path filename = folder / fmt::format("{}.wasm", comp.builder.moduleName);
    	std::ofstream outfile(filename, std::ios::out | std::ios::binary);
		outfile.write(reinterpret_cast<const char*>(wm.data.data()), wm.data.size());
	}

	out.module_name = comp.builder.moduleName;
	out.wm = std::move(wm);
	out.resume_dispatch = std::move(resume_dispatch);
	return true;
}

// Wraps a browser-compiled module id (0 = the browser failed to compile).
static JittedScript* wasm_wrap_module(zasm_script* script, WasmCodegenResult&& r, int module_id)
{
	printf("success: %s\n", module_id ? "YES" : "NO");
	if (!module_id)
	{
		jit_printf("failure\n");
		error(script, "failed to compile");
		return nullptr;
	}

	auto fn = new JittedScript{module_id};
	fn->resume_dispatch = std::move(r.resume_dispatch);
	return fn;
}

// The main-thread half: hand the module bytes to the browser to compile.
static JittedScript* wasm_finalize(zasm_script* script, WasmCodegenResult&& r)
{
#ifndef __EMSCRIPTEN__
	// Native builds can only generate the module (e.g. for -jit-save-wasm
	// validation), not run it.
	if (script) return nullptr;
#endif

	int module_id = em_compile_wasm(r.module_name.c_str(), r.wm.data.data(), r.wm.data.size());
	return wasm_wrap_module(script, std::move(r), module_id);
}

JittedScript* jit_compile_script(zasm_script* script)
{
	WasmCodegenResult r;
	if (!wasm_codegen(script, r))
		return nullptr;
	return wasm_finalize(script, std::move(r));
}

// Precompile with backend-managed threading. Only used on the web build: the
// engine worker pool is unavailable there (see get_worker_thread_count), but
// raw pthreads work (the runtime preallocates a pool). Two serial costs are
// attacked: codegen runs on worker threads, and the browser-compile step -
// which must be issued from the main thread, and whose per-module asyncify
// round-trip dominated serial precompile - is issued as ONE batch
// (em_compile_wasm_batch), letting the browser compile every module in
// parallel on its own pool.
bool jit_precompile_scripts_impl([[maybe_unused]] const std::vector<zasm_script*>& scripts, [[maybe_unused]] const std::function<void(zasm_script*, JittedScript*)>& on_compiled)
{
#ifndef __EMSCRIPTEN__
	// Native builds precompile via the engine worker pool (or serially).
	return false;
#else
	// 0 forces synchronous compilation (debugging); see jit_requested_threads
	// for the shared -jit-threads semantics.
	int num_threads = jit_requested_threads(1);
	if (num_threads == 0)
		return false;
	num_threads = std::min({num_threads, (int)scripts.size(), 8});
	if (num_threads < 1)
		num_threads = 1;

	struct Item
	{
		bool compiled;
		WasmCodegenResult result;
	};
	std::vector<Item> items(scripts.size());
	std::atomic<size_t> next_index{0};
	std::atomic<size_t> num_done{0};

	auto worker = [&](){
		while (true)
		{
			size_t i = next_index.fetch_add(1);
			if (i >= scripts.size())
				break;
			items[i].compiled = wasm_codegen(scripts[i], items[i].result);
			num_done.fetch_add(1);
		}
	};
	std::vector<std::thread> threads;
	for (int t = 0; t < num_threads; t++)
		threads.emplace_back(worker);

	// The sleep yields to the browser, which is also what lets queued worker
	// threads spawn.
	while (num_done.load() < scripts.size())
		emscripten_sleep(1);
	for (auto& t : threads)
		t.join();

	std::vector<size_t> indices;
	std::vector<const char*> names;
	std::vector<void*> ptrs;
	std::vector<uint32_t> sizes;
	for (size_t i = 0; i < items.size(); i++)
	{
		if (!items[i].compiled)
			continue;
		indices.push_back(i);
		names.push_back(items[i].result.module_name.c_str());
		ptrs.push_back(items[i].result.wm.data.data());
		sizes.push_back(items[i].result.wm.data.size());
	}
	std::vector<int> module_ids(indices.size());
	em_compile_wasm_batch(indices.size(), names.data(), ptrs.data(), sizes.data(), module_ids.data());

	std::vector<JittedScript*> compiled(scripts.size());
	for (size_t k = 0; k < indices.size(); k++)
	{
		size_t i = indices[k];
		compiled[i] = wasm_wrap_module(scripts[i], std::move(items[i].result), module_ids[k]);
	}
	for (size_t i = 0; i < scripts.size(); i++)
		on_compiled(scripts[i], compiled[i]);
	return true;
#endif
}

JittedScriptInstance* jit_create_script_impl(script_data* script, refInfo* ri, JittedScript* j_script)
{
	// A fresh instance resumes from its own wait_index / call_stack_rets state,
	// which points at the script's entry - correct only for a script that has not
	// run yet. A script can also be mid-run here: it started on the interpreter,
	// nested inside another executing jitted script (see jit_can_start_script),
	// and this is a later slice. Adopting it with entry state would re-run the
	// entry on top of the stack frame the interpreter already pushed, corrupting
	// the script (this was the terror_of_necromancy rng desync). Instead,
	// translate the interpreter's resume state: at a yield, ri->pc and every live
	// ret_stack entry are wait/RUNGENFRZSCR resume points or yielding-call return
	// continuations - all dispatchable yielder blocks (see
	// compute_dispatch_entries). If anything doesn't map, decline and the script
	// just finishes its run on the interpreter. (The x64 backend needs none of
	// this: it dispatches on ri->pc and shares the interpreter's ret_stack.)
	//
	// ri->pc == script->pc exactly when the script has not run yet: it is set to
	// the entry when the script instance is first initialized, and no later wait
	// can yield with the pc back at the entry.
	uint32_t adopted_wait_index = 0;
	uint32_t adopted_ret_index = 0;
	pc_t adopted_rets[MAX_CALL_FRAMES];
	if (ri->pc != script->pc)
	{
		extern int32_t(*ret_stack)[MAX_CALL_FRAMES];

		auto decline = [&](pc_t pc) -> JittedScriptInstance* {
			// Should not happen (every interpreter yield point is a dispatch
			// entry) - error so -jit-fatal-compile-errors runs (CI) fail hard.
			// Log once per script: this is retried every slice for the rest of
			// the run.
			static std::set<std::string> logged;
			if (logged.insert(script->name()).second)
				error(script->zasm_script.get(), fmt::format("cannot adopt mid-run script (unmapped resume pc {}): {}", (int)pc, script->name()));
			return nullptr;
		};

		auto it = j_script->resume_dispatch.find(ri->pc);
		if (it == j_script->resume_dispatch.end())
			return decline(ri->pc);
		adopted_wait_index = it->second;

		if (ri->retsp > MAX_CALL_FRAMES)
			return decline(ri->pc);
		for (; adopted_ret_index < ri->retsp; adopted_ret_index++)
		{
			pc_t ret_pc = (*ret_stack)[adopted_ret_index];
			auto rit = j_script->resume_dispatch.find(ret_pc);
			if (rit == j_script->resume_dispatch.end())
				return decline(ret_pc);
			adopted_rets[adopted_ret_index] = rit->second;
		}

		jit_printf("[jit] adopting mid-run script: %s pc: %d depth: %d\n", script->name().c_str(), (int)ri->pc, (int)ri->retsp);
	}

	int handle_id = em_create_wasm_handle(j_script->module_id);
	if (!handle_id)
	{
		al_trace("[jit] Error creating wasm handle for script: %s\n", script->zasm_script->name.c_str());
		return nullptr;
	}

	auto j_instance = new JittedScriptInstance{
		.j_script = j_script,
		.script = script,
		.ri = ri,
		.handle_id = handle_id,
	};
	j_instance->wait_index = adopted_wait_index;
	j_instance->call_stack_ret_index = adopted_ret_index;
	for (uint32_t k = 0; k < adopted_ret_index; k++)
		j_instance->call_stack_rets[k] = adopted_rets[k];
	return j_instance;
}

void jit_profiler_increment_function_back_edge([[maybe_unused]] JittedScriptInstance* j_instance, [[maybe_unused]] pc_t pc)
{
	// Not used - the wasm backend currently only supports precompiling scripts.
}

// Depth of the currently-executing jitted wasm script(s). A jitted script that
// runs a nested script (e.g. an FFC that runs an enemy's script mid-frame) does
// so while its own em_poll_wasm_handle call is still on the stack; asyncify can
// only manage one script's unwind/rewind at a time, so the nested script must
// run on the interpreter. run_script consults jit_can_start_script() for this.
//
// This is the general-case escape hatch; the RUNGENFRZSCR yield lowering is
// the optimized protocol for the one command where nesting is the purpose.
// RUNGENFRZSCR deterministically hands a whole engine frame to the engine and
// has clean resume semantics (one result register, resume at the next pc), so
// it earns a dispatch entry: the compiled script returns cleanly and the
// run_script driver runs the frozen frame with NO script wasm on the stack -
// depth 0, so scripts inside that frame may use the JIT. By contrast,
// "commands whose engine side effects might start another script" (combo
// triggers, enemy spawns, item pickups, ...) is a huge set that depends on
// quest data, not opcodes - and those commands must complete synchronously,
// mid-batch; the wasm can't return out of the middle of an em_do_commands
// call. So their nested scripts run interpreted instead.
static int jit_wasm_execution_depth = 0;
bool jit_can_start_script()
{
	return jit_wasm_execution_depth == 0;
}

int jit_run_script(JittedScriptInstance* j_instance)
{
	extern int32_t(*stack)[MAX_STACK_SIZE];

	uintptr_t ptr[8];
	ptr[0] = (uintptr_t)&*j_instance;
	ptr[1] = (uintptr_t)&*j_instance->ri;
	ptr[2] = (uintptr_t)&game->global_d;
	ptr[3] = (uintptr_t)&*stack;
	ptr[4] = (uintptr_t)&j_instance->call_stack_rets;
	ptr[5] = (uintptr_t)&j_instance->call_stack_ret_index;
	ptr[6] = (uintptr_t)&j_instance->wait_index;
	ptr[7] = j_instance->script->pc;

	jit_wasm_execution_depth++;
	int return_code = em_poll_wasm_handle(j_instance->handle_id, (uintptr_t)&ptr);
	jit_wasm_execution_depth--;

	return return_code;
}

void jit_release(JittedScript* j_script)
{
	if (!j_script) return;

	bool success = em_destroy_wasm_module(j_script->module_id);
	ASSERT(success);
	delete j_script;
}

#ifdef __EMSCRIPTEN__
extern "C" int em_do_commands(JittedScriptInstance* j_instance, int pc, int len, int sp)
{
	return run_script_jit_sequence(j_instance, pc, sp, len);
}

extern "C" int em_get_register(int r)
{
	return get_register(r);
}

extern "C" void em_set_register(int r, int value)
{
	set_register(r, value);
}

extern "C" void em_set_guarded_register(int32_t arg, int32_t value, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	do_set(arg, value);
}

extern "C" void em_runtime_script_debug(int pc, int sp)
{
	extern refInfo *ri;
	extern ScriptDebugHandle* runtime_script_debug_handle;
	ri->pc = pc;
	ri->sp = sp;
	if (runtime_script_debug_handle)
		runtime_script_debug_handle->pre_command();
}

extern "C" void em_log_error(int code)
{
	if (code == 0)
		scripting_log_error_with_context("Attempted to divide by zero!");
	if (code == 1)
		scripting_log_error_with_context("Attempted to modulo by zero!");
}

extern "C" int em_pod_read(int arrayptr, int index, int pc, int no_neg)
{
	return jit_pod_read(arrayptr, index, pc, no_neg);
}

extern "C" void em_pod_write(int arrayptr, int index, int value, int type, int pc, int no_neg)
{
	jit_pod_write(arrayptr, index, value, type, pc, no_neg);
}

extern "C" int em_allocatemem(int size, int object_type, int uid)
{
	return jit_allocatemem(size, object_type, uid);
}

extern "C" void em_writepodarr(int id, int pc)
{
	jit_writepodarr(id, pc);
}

#endif

JittedScriptInstance::~JittedScriptInstance()
{
	ASSERT(em_destroy_wasm_handle(handle_id));
}
