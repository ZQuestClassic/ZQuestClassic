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
#include "zc/zasm_utils.h"
#include "zc/zelda.h"
#include "components/zasm/serialize.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <stdint.h>

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

EM_ASYNC_JS(bool, em_destroy_wasm_module_, (int id), {
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

static void add_sp(CompilationState& state, int delta)
{
	modify_global_idx(*state.wasm, state.g_idx_sp, delta);
}

static void check_sp(CompilationState& state)
{
	auto& wasm = *state.wasm;

	wasm.emitGlobalGet(state.g_idx_sp);
	wasm.emitI32Const(MAX_STACK_SIZE);
	wasm.emitI32GeU();

	wasm.emitIf();
	// ri->pc = state.pc, so the stack-overflow error's innermost stack frame is the
	// command that overflowed (matching the interpreter), not a stale pc.
	wasm.emitGlobalGet(state.g_idx_ri);
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
	wasm.emitGlobalGet(state.g_idx_ri);
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
		state.wasm->emitGlobalGet(state.g_idx_ri);
		state.wasm->emitI32Load(4 + r*4);
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		state.wasm->emitGlobalGet(state.g_idx_global_d);
		state.wasm->emitI32Load((r - GD(0)) * 4); // game->global_d[]
	}
	else if (r == SP)
	{
		state.wasm->emitGlobalGet(state.g_idx_sp);
		state.wasm->emitI32Const(10000);
		state.wasm->emitI32Mul();
	}
	else if (r == SP2)
	{
		state.wasm->emitGlobalGet(state.g_idx_sp);
	}
	else
	{
		if (does_register_use_stack(r))
		{
			// ri->sp = g_idx_sp
			state.wasm->emitGlobalGet(state.g_idx_ri);
			state.wasm->emitGlobalGet(state.g_idx_sp);
			state.wasm->emitI32Store(4*9); // ri->sp
		}

		// ri->pc = state.pc; needed for an accurate stack trace should the engine
		// call log an error. create_stack_trace is its only consumer, so skip it
		// when stack traces won't be shown - except for reads of the PC register,
		// which returns ri->pc itself (the interpreter and x64 backend keep it
		// current, so a stale value here would be a web-only divergence).
		if (state.maintain_call_stack || r == PC)
		{
			state.wasm->emitGlobalGet(state.g_idx_ri);
			state.wasm->emitI32Const(state.pc);
			state.wasm->emitI32Store(0); // ri->pc
		}

		state.wasm->emitI32Const(r);
		state.wasm->emitCall(state.f_idx_get_register);
	}
}

static void set_z_register(CompilationState& state, int r, std::function<void()> fn)
{
	if (r >= D(0) && r < D(INITIAL_D))
	{
		state.wasm->emitGlobalGet(state.g_idx_ri);
		fn();
		state.wasm->emitI32Store(4 + r*4);
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		state.wasm->emitGlobalGet(state.g_idx_global_d);
		fn();
		state.wasm->emitI32Store((r - GD(0)) * 4); // game->global_d[]
	}
	else
	{
		if (does_register_use_stack(r))
		{
			// ri->sp = g_idx_sp
			state.wasm->emitGlobalGet(state.g_idx_ri);
			state.wasm->emitGlobalGet(state.g_idx_sp);
			state.wasm->emitI32Store(4*9); // ri->sp
		}

		// ri->pc = state.pc; needed for an accurate stack trace should the engine
		// call log an error. create_stack_trace is its only consumer, so skip it
		// when stack traces won't be shown.
		if (state.maintain_call_stack)
		{
			state.wasm->emitGlobalGet(state.g_idx_ri);
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
		// ri->sp = g_idx_sp
		state.wasm->emitGlobalGet(state.g_idx_ri);
		state.wasm->emitGlobalGet(state.g_idx_sp);
		state.wasm->emitI32Store(4*9); // ri->sp
	}

	state.wasm->emitGlobalGet(state.g_idx_j_instance);
	state.wasm->emitI32Const(pc);
	state.wasm->emitI32Const(count);
	state.wasm->emitGlobalGet(state.g_idx_sp);
	state.wasm->emitCall(state.f_idx_do_commands);

	if (is_wait)
	{
		state.wasm->emitIf();
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
	wasm.emitGlobalGet(state.g_idx_ri);
	wasm.emitI32Load(4 * 10); // ri->retsp
	wasm.emitI32Const(4);
	wasm.emitI32Mul();
	wasm.emitI32Add();
	wasm.emitI32Const(pc + 1);
	wasm.emitI32Store(0);
	// ri->retsp += 1
	wasm.emitGlobalGet(state.g_idx_ri);
	wasm.emitGlobalGet(state.g_idx_ri);
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
	wasm.emitGlobalGet(state.g_idx_ri);
	wasm.emitI32Load(4 * 10); // ri->retsp
	wasm.emitIf();
	{
		wasm.emitGlobalGet(state.g_idx_ri);
		wasm.emitGlobalGet(state.g_idx_ri);
		wasm.emitI32Load(4 * 10);
		wasm.emitI32Const(1);
		wasm.emitI32Sub();
		wasm.emitI32Store(4 * 10);
	}
	wasm.emitEnd();
}

// Locals reserved for snapshotting the two comparison operands (see the
// defineFunction calls that declare {I32, I32, I32}). Local 0 is the general
// scratch; locals 1/2 double as the second scratch for two-operand commands
// (never live at the same time as a comparison snapshot, since a comparison's
// consumers immediately follow it).
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
// returns `i`.
static pc_t compile_comparison(CompilationState& state, const zasm_script* script, const ZasmCFG& cfg, pc_t num_blocks, pc_t& current_block_index, pc_t i)
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
		if (cfg.contains_block_start(cj))
		{
			wasm.emitEnd();
			current_block_index += 1;
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
			wasm.emitBrIf(num_blocks - current_block_index);
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
	wasm.emitCall(function_id_to_idx.at(fn_id));

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
	wasm.emitCall(function_id_to_idx.at(structured_zasm.start_pc_to_function.at(target_pc)));
	modify_global_idx(wasm, state.g_idx_ret_stack_index, -1);
	// The GOTO does not return here, so end this function once the callee does.
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
	uint8_t g_idx_stack = state.g_idx_stack;
	uint8_t g_idx_sp = state.g_idx_sp;

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

			wasm.emitGlobalGet(g_idx_sp);
			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			wasm.emitGlobalGet(g_idx_stack);
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

				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				wasm.emitGlobalGet(g_idx_stack);
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
			wasm.emitGlobalGet(g_idx_sp);
			wasm.emitI32Const(arg2);
			wasm.emitI32Add();
			wasm.emitI32Const(4);
			wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
			wasm.emitGlobalGet(g_idx_stack);
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
			wasm.emitGlobalGet(g_idx_stack);
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
			wasm.emitGlobalGet(g_idx_stack);
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
				wasm.emitGlobalGet(g_idx_stack);
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
				wasm.emitGlobalGet(g_idx_stack);
				wasm.emitI32Add();
				wasm.emitI32Load();
			});
		}
		break;
		case POP:
		{
			set_z_register(state, arg1, [&](){
				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				wasm.emitGlobalGet(g_idx_stack);
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
				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitI32Const(1);
				wasm.emitI32Sub();

				wasm.emitI32Const(4);
				wasm.emitI32Mul();

				wasm.emitGlobalGet(g_idx_stack);
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
				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitI32Const(4);
				wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
				wasm.emitGlobalGet(g_idx_stack);
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
				wasm.emitGlobalGet(g_idx_stack);
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
			wasm.emitGlobalGet(g_idx_stack);
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

	pc_t block_final(int b) const
	{
		return b + 1 < (int)starts->size() ? (*starts)[b + 1] - 1 : final_pc;
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
				wasm.emitGlobalGet(state->g_idx_sp);
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
				wasm.emitGlobalGet(state->g_idx_sp);
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
};

static std::optional<WasmAssembler> compile_function_structured(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, pc_t fn_id, const std::map<pc_t, pc_t>& function_id_to_idx)
{
	const auto& fn = structured_zasm.functions[fn_id];
	pc_t start_pc = fn.start_pc;
	pc_t final_pc = fn.final_pc;

	// Refined block partition: the CFG's block starts, plus a split after every
	// control op the CFG doesn't split on (QUIT/RETURNFUNC never split a block,
	// and a branch to the function's own entry is treated as a recursive call
	// by zasm_construct_cfg, so it doesn't split either). With the refinement,
	// a control op is always the last instruction of its block.
	auto cfg = zasm_construct_cfg(script, {{start_pc, final_pc}});
	std::vector<pc_t> starts = cfg.block_starts;
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;

		// A non-yielding function can't contain these; bail defensively.
		if (command_is_wait(command) || command == RUNGENFRZSCR)
			return std::nullopt;
		// compile_callfunc_native requires a known function entry.
		if (command == CALLFUNC && !structured_zasm.start_pc_to_function.contains(script->zasm[i].arg1))
			return std::nullopt;

		bool is_control = command == GOTO || command_is_goto_consumer(command) ||
						  command == QUIT || command == RETURNFUNC;
		if (is_control && i + 1 <= final_pc)
			starts.push_back(i + 1);
	}
	std::sort(starts.begin(), starts.end());
	starts.erase(std::unique(starts.begin(), starts.end()), starts.end());

	int n = starts.size();
	auto block_id_of = [&](int pc) -> int {
		auto it = std::lower_bound(starts.begin(), starts.end(), (pc_t)pc);
		if (it == starts.end() || *it != (pc_t)pc)
			return -1;
		return it - starts.begin();
	};
	auto block_final = [&](int b) -> pc_t {
		return b + 1 < n ? starts[b + 1] - 1 : final_pc;
	};

	// Comparison groups: map each compare and each of its consumers, so SETx
	// consumers emit in the body and a GOTOx terminator can re-derive its
	// condition in emit_cond. A consumer run may span blocks (e.g. `COMPAREV;
	// GOTOTRUE a; GOTOFALSE b`), which this per-pc map handles uniformly.
	std::map<pc_t, CmpGroup> cmp_groups;
	std::map<pc_t, pc_t> consumer_to_cmp;
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;
		if (command != COMPARER && command != COMPAREV && command != COMPAREV2)
			continue;

		CmpGroup g = analyze_comparison(script, i);
		if (g.consumers.empty())
			continue;

		// A consumer run that leaves the function; shouldn't happen.
		if (g.consumers.back() > final_pc)
			return std::nullopt;

		for (pc_t cj : g.consumers)
			consumer_to_cmp[cj] = i;

		i = g.consumers.back();
		cmp_groups.emplace(g.cmp_pc, std::move(g));
	}

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

	// Irreducibility is detected before anything is emitted.
	WasmStructurer structurer(n, 0, infos, sink);
	if (!structurer.run())
		return std::nullopt;

	wasm.emitEnd(); // Function end.

	return wasm;
}

static WasmAssembler compile_function(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, std::set<pc_t> function_ids, const std::map<pc_t, pc_t>& function_id_to_idx, bool may_yield)
{
	WasmAssembler wasm;
	state.wasm = &wasm;

	uint8_t g_idx_ret_stack = state.g_idx_ret_stack;
	uint8_t g_idx_ret_stack_index = state.g_idx_ret_stack_index;
	uint8_t g_idx_wait_index = state.g_idx_wait_index;
	uint8_t g_idx_sp = state.g_idx_sp;
	uint8_t g_idx_target_block_id = state.g_idx_target_block_id;

	std::vector<std::pair<pc_t, pc_t>> pc_ranges;
	for (auto fn_id : function_ids)
	{
		auto& fn = structured_zasm.functions[fn_id];
		pc_ranges.emplace_back(fn.start_pc, fn.final_pc);
	}
	auto cfg = zasm_construct_cfg(script, pc_ranges);

	// All control flow is converted to a simple, but slow, loop-switch construct: a giant switch case,
	// implemented with "loop", "br_table", and a "block" for every block in the CFG.
	//
	// See: https://medium.com/leaningtech/solving-the-structured-control-flow-problem-once-and-for-all-5123117b1ee2#:~:text=A%20universal%20but%20inefficient%20solution
	//
	// Non-yielding functions don't use this - they get real structured control flow via
	// compile_function_structured (which falls back to this lowering for irreducible CFGs, and
	// -no-jit-wasm-structured forces it everywhere). The yielder is here to stay on the dispatch for
	// its call/return/resume edges, whose targets are runtime values; but its *static* stretches
	// (loops between suspension points) could still be structured on top of a residual dispatch.
	// If attempting that, this is gold: https://dl.acm.org/doi/pdf/10.1145/3547621

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
	pc_t num_blocks = cfg.block_starts.size();
	for (int i = 0; i < num_blocks; i++)
	{
		wasm.emitBlock();
	}

	// Jump to the next block.
	// For the first execution of this function, this initially jumps to the first block.
	// For subsequent executions (re-entering after `WaitX`), it initially jumps to the last block yielded from.
	// For subsequent loop iterations in the same call to the script, it jumps to the block specified by the
	// previous block. This is what handles branches (including if, calls, loops) in the CFG. If a block simply
	// falls through to the next, it does so directly (no additional loop iteration).
	wasm.emitBlock();
	wasm.emitGlobalGet(g_idx_target_block_id);
	wasm.emitBrTable();
	// Plus 2, because of the inner-most block (this most recently emitted one), and
	// the outer-most loop.
	pc_t br_table_len = num_blocks + 2;
	wasm.emitVarU32(br_table_len);
	for (int i = 0; i < br_table_len; i++)
	{
		wasm.emitVarU32(i);
	}
	wasm.emitVarU32(0);
	wasm.emitEnd();

	pc_t current_block_index = 0;

	for (auto [start_pc, final_pc] : pc_ranges)
	{
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			state.pc = i;

			if (cfg.contains_block_start(i))
			{
				wasm.emitEnd();
				current_block_index += 1;
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
				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitCall(state.f_idx_runtime_debug);
			}

			if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
			{
				i = compile_comparison(state, script, cfg, num_blocks, current_block_index, i);
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

				wasm.emitReturn();
				continue;
			}

			if (!command_is_compiled(command))
			{
				// Every command that is not compiled to WASM must go through the regular interpreter function.
				// In order to reduce function call overhead, we call into the interpreter function in batches.
				int uncompiled_command_count = 1;
				for (int j = i + 1; j < script->size; j++)
				{
					if (command_is_compiled(script->zasm[j].command))
						break;
					if (cfg.contains_block_start(j))
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
					wasm.emitBr(num_blocks - current_block_index);
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
						wasm.emitBr(num_blocks - current_block_index);
					}
					else
					{
						// Totally ignore the return pc on the stack.
						if (command == RETURN)
						{
							add_sp(state, 1);
						}

						// Call stack is implicitly handled by WASM.
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

JittedScript* jit_compile_script(zasm_script* script)
{
	if (script->size <= 1)
		return nullptr;

	// STACKWRITEATVV_IF is not yet supported by the wasm codegen - always bail.
	//
	// RUNGENFRZSCR (generic frozen scripts) still has a resume-codegen desync on the
	// wasm JIT (see terror_of_necromancy_demo6: rng desync at frame 9237, isolated to
	// npc-6-Candlehead's WAITFRAME resume re-running its entry and leaking its stack
	// frame), so bail on it for normal play - this is coarse, since any quest that
	// uses generic frozen scripts compiles its whole @single chunk with it. The
	// replay tests pass -no-jit-wasm-bail-on-frozen-generic so they actually exercise
	// the wasm backend for that content. (The separate nested-script crash it used to
	// hit is fixed - see jit_can_start_script / em_create_wasm_handle.)
	static bool bail_on_frozen_generic = get_flag_bool("-jit-wasm-bail-on-frozen-generic").value_or(true);
	for (size_t i = 0; i < script->size; i++)
	{
		int command = script->zasm[i].command;
		if (command == STACKWRITEATVV_IF)
		{
			error(script, "STACKWRITEATVV_IF unsupported", true);
			return nullptr;
		}
		if (command == RUNGENFRZSCR && bail_on_frozen_generic)
			return nullptr;
	}

	auto structured_zasm = zasm_construct_structured(script);
	if (!structured_zasm.is_modern_function_calling())
	{
		al_trace("[jit] NOT compiling zasm chunk (unexpected function call mode): %s id: %d size: %zu\n", script->name.c_str(), script->id, script->size);
		return nullptr;
	}

	al_trace("[jit] compiling script: %s id: %d\n", script->name.c_str(), script->id);

	WasmCompiler comp{};
	CompilationState state{};

	state.runtime_debugging = script_debug_is_runtime_debugging() == 2;
	// Maintain ri->pc and the pc call stack only when a stack trace could actually
	// be produced (their only consumer), so non-tracing runs pay nothing per call.
	state.maintain_call_stack = FFCore.should_display_stack_traces();

	state.f_idx_set_return_value = comp.builder.importFunction("set_return_value", 1, 0);
	state.f_idx_do_commands = comp.builder.importFunction("do_commands", 4, 1);
	state.f_idx_do_commands_async = comp.builder.importFunction("do_commands_async", 4, 1);
	state.f_idx_get_register = comp.builder.importFunction("get_register", 1, 1);
	state.f_idx_set_register = comp.builder.importFunction("set_register", 2, 0);
	state.f_idx_set_guarded_register = comp.builder.importFunction("set_guarded_register", 3, 0);
	state.f_idx_runtime_debug = comp.builder.importFunction("runtime_debug", 2, 0);
	state.f_idx_log_error = comp.builder.importFunction("log_error", 1, 0);
	state.f_idx_pod_read = comp.builder.importFunction("pod_read", 3, 1);
	state.f_idx_pod_write = comp.builder.importFunction("pod_write", 5, 0);
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
				return nullptr;
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
	if (!yielding_fns.empty())
	{
		bool may_yield = true;
		auto wasm = compile_function(state, script, structured_zasm, yielding_fns, function_id_to_idx, may_yield);
		comp.builder.defineFunction(fn_yielder_idx, {WasmValType::I32, WasmValType::I32, WasmValType::I32}, wasm.finish());
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
		comp.builder.defineFunction(fidx, {WasmValType::I32, WasmValType::I32, WasmValType::I32}, wasm->finish());
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

#ifndef __EMSCRIPTEN__
	if (script) return nullptr;
#endif

	int module_id = em_compile_wasm(comp.builder.moduleName.c_str(), wm.data.data(), wm.data.size());
	printf("success: %s\n", module_id ? "YES" : "NO");
	if (!module_id)
	{
		jit_printf("failure\n");
		error(script, "failed to compile");
		return nullptr;
	}

	auto fn = new JittedScript(module_id);
	return fn;
}

JittedScriptInstance* jit_create_script_impl(script_data* script, refInfo* ri, JittedScript* j_script)
{
	int handle_id = em_create_wasm_handle(j_script->module_id);
	if (!handle_id)
	{
		al_trace("[jit] Error creating wasm handle for script: %s\n", script->zasm_script->name.c_str());
		return nullptr;
	}

	return new JittedScriptInstance{
		.j_script = j_script,
		.script = script,
		.ri = ri,
		.handle_id = handle_id,
	};
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

extern "C" int em_pod_read(int arrayptr, int index, int pc)
{
	return jit_pod_read(arrayptr, index, pc);
}

extern "C" void em_pod_write(int arrayptr, int index, int value, int type, int pc)
{
	jit_pod_write(arrayptr, index, value, type, pc);
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
