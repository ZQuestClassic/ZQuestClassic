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
#include "base/general.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "zc/ffscript.h"
#include "zc/jit.h"
#include "zc/script_debug.h"
#include "zc/wasm_compiler.h"
#include "zc/zasm_utils.h"
#include "zc/zelda.h"
#include "zasm/serialize.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

struct CompilationState
{
	WasmAssembler* wasm;
	bool runtime_debugging;

	uint8_t f_idx_set_return_value;
	uint8_t f_idx_do_commands;
	uint8_t f_idx_do_commands_async;
	uint8_t f_idx_get_register;
	uint8_t f_idx_set_register;
	uint8_t f_idx_runtime_debug;
	uint8_t f_idx_log_error;

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
	uint8_t g_idx_wait_index;
	uint8_t g_idx_sp;
	uint8_t g_idx_target_block_id;
	uint32_t g_idx_start_pc;

	std::map<pc_t, uint32_t> pc_to_block_id;
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

EM_ASYNC_JS(int, em_create_wasm_handle_, (int module_id), {
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

static int em_compile_wasm(const char* name, void* ptr, size_t size)
{
	return 0;
}

static int em_create_wasm_handle(int module_id)
{
	return 0;
}

static int em_poll_wasm_handle(int id, uintptr_t ptr)
{
	return 0;
}

static bool em_destroy_wasm_handle(int id)
{
	return false;
}

static bool em_destroy_wasm_module(int id)
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
	wasm.emitI32Const(RUNSCRIPT_JIT_STACK_OVERFLOW);
	wasm.emitCall(state.f_idx_set_return_value);
	wasm.emitUnreachable(); // Bail.
	wasm.emitEnd();
}

static void check_call_limit(CompilationState& state)
{
	auto& wasm = *state.wasm;

	wasm.emitGlobalGet(state.g_idx_ret_stack_index);
	wasm.emitI32Const(MAX_CALL_FRAMES);
	wasm.emitI32GeU();

	wasm.emitIf();
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
		state.wasm->emitI32Const(r);
		fn();
		state.wasm->emitCall(state.f_idx_set_register);
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
	// case ABS:
	case ADDR:
	case ADDV:
	// case ANDR:
	// case ANDV:
	case CASTBOOLF:
	case CASTBOOLI:
	// case CEILING:
	case DIVR:
	case DIVV:
	// case FLOOR:
	case LOAD:
	case LOADD:
	// case LOADI:
	// case MAXR:
	// case MAXV:
	// case MINR:
	// case MINV:
	case MODR:
	case MODV:
	case MULTR:
	case MULTV:
	case NOP:
	// case ORR:
	// case ORR32:
	// case ORV:
	// case ORV32:
	case PEEK:
	case SETR:
	case SETV:
	case STORE:
	case STORED:
	// case STOREI:
	case SUBR:
	case SUBV:
	// case SUBV2:
		return true;
	}

	return false;
}

static WasmAssembler compile_function(CompilationState& state, const zasm_script* script, const StructuredZasm& structured_zasm, std::set<pc_t> function_ids, const std::map<pc_t, pc_t>& function_id_to_idx, bool may_yield)
{
	WasmAssembler wasm;
	state.wasm = &wasm;

	uint8_t g_idx_stack = state.g_idx_stack;
	uint8_t g_idx_ret_stack = state.g_idx_ret_stack;
	uint8_t g_idx_ret_stack_index = state.g_idx_ret_stack_index;
	uint8_t g_idx_wait_index = state.g_idx_wait_index;
	uint8_t g_idx_sp = state.g_idx_sp;
	uint8_t g_idx_target_block_id = state.g_idx_target_block_id;

	uint8_t l_idx_scratch = 0;

	std::vector<std::pair<pc_t, pc_t>> pc_ranges;
	for (auto fn_id : function_ids)
	{
		auto& fn = structured_zasm.functions[fn_id];
		pc_ranges.emplace_back(fn.start_pc, fn.final_pc);
	}
	auto cfg = zasm_construct_cfg(script, pc_ranges);

	// In this naive approach, all control flow is converted to a very simple, but slow, loop-switch construct.
	// The main control flow construct is a giant switch case, implemented with "loop", "br_table", and a "block"
	// for every block in the CFG.
	//
	// See: https://medium.com/leaningtech/solving-the-structured-control-flow-problem-once-and-for-all-5123117b1ee2#:~:text=A%20universal%20but%20inefficient%20solution
	//
	// TODO: reduce CFG
	// 
	// this is gold: https://dl.acm.org/doi/pdf/10.1145/3547621
	// this may help too :https://dl.acm.org/doi/pdf/10.1145/512976.512979

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
			if (cfg.contains_block_start(i))
			{
				wasm.emitEnd();
				current_block_index += 1;
			}

#ifndef _NDEBUG
			wasm.emitI32Const(i);
			wasm.emitDrop();
#endif

			int command = script->zasm[i].command;
			int arg1 = script->zasm[i].arg1;
			int arg2 = script->zasm[i].arg2;

			if (state.runtime_debugging && !command_uses_comparison_result(command))
			{
				wasm.emitI32Const(i);
				wasm.emitGlobalGet(g_idx_sp);
				wasm.emitCall(state.f_idx_runtime_debug);
			}

			if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
			{
				if (cfg.contains_block_start(i + 1))
				{
					wasm.emitEnd();
					current_block_index += 1;
				}

				#define VAL(x, v) {\
					if (v) wasm.emitI32Const((next_arg2 & CMP_BOOL) ? !!x : x);\
					else\
					{\
						get_z_register(state, x);\
						if (next_arg2 & CMP_BOOL)\
						{\
							wasm.emitI32Const(0);\
							wasm.emitI32Ne();\
						}\
					}\
				}
				bool arg1_from_register = false;
				bool arg2_from_register = command != COMPARER;
				if (command == COMPAREV2)
					std::swap(arg1_from_register, arg2_from_register);

				int next_command = script->zasm[i + 1].command;
				int next_arg1 = script->zasm[i + 1].arg1;
				int next_arg2 = script->zasm[i + 1].arg2;

				if (next_command == NOP)
				{
					// The optimizer may turn the usage of the comparison into a NOP, but not actually remove
					// the comparison command.
					// Example: hollow_forest.qst/zasm-global-1.txt
					// 11911: COMPAREV        D2              0            
					// 11912: GOTOTRUE        11913                        ---- turns into NOP
				}
				else if (next_command == SETCMP)
				{
					set_z_register(state, next_arg1, [&](){
						VAL(arg1, arg1_from_register);
						VAL(arg2, arg2_from_register);
						switch(next_arg2 & CMP_FLAGS)
						{
							default:
								wasm.emitDrop();
								wasm.emitDrop();
								wasm.emitI32Const(0);
								break;
							case CMP_GT:
								wasm.emitI32GtS();
								break;
							case CMP_GT|CMP_EQ:
								wasm.emitI32GeS();
								break;
							case CMP_LT:
								wasm.emitI32LtS();
								break;
							case CMP_LT|CMP_EQ:
								wasm.emitI32LeS();
								break;
							case CMP_EQ:
								wasm.emitI32Eq();
								break;
							case CMP_GT|CMP_LT:
								wasm.emitI32Ne();
								break;
							case CMP_GT|CMP_LT|CMP_EQ:
								// TODO could avoid getting values ...
								wasm.emitDrop();
								wasm.emitDrop();
								wasm.emitI32Const(1);
								break;
						}
						if (next_arg2 & CMP_SETI)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
					});
				}
				else if (next_command == SETLESSI || next_command == SETLESS)
				{
					set_z_register(state, next_arg1, [&](){
						VAL(arg1, arg1_from_register);
						VAL(arg2, arg2_from_register);
						wasm.emitI32LeS();
						if (next_command == SETLESSI)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
					});
				}
				else if (next_command == SETMOREI || next_command == SETMORE)
				{
					set_z_register(state, next_arg1, [&](){
						VAL(arg1, arg1_from_register);
						VAL(arg2, arg2_from_register);
						wasm.emitI32GeS();
						if (next_command == SETMOREI)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
					});
				}
				else if (next_command == SETFALSEI || next_command == SETFALSE)
				{
					set_z_register(state, next_arg1, [&](){
						VAL(arg1, arg1_from_register);
						VAL(arg2, arg2_from_register);
						wasm.emitI32Ne();
						if (next_command == SETFALSEI)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
					});
				}
				else if (next_command == SETTRUEI || next_command == SETTRUE)
				{
					set_z_register(state, next_arg1, [&](){
						VAL(arg1, arg1_from_register);
						VAL(arg2, arg2_from_register);
						wasm.emitI32Eq();
						if (next_command == SETTRUEI)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
					});
				}
				else if (next_command == GOTOCMP || next_command == GOTOTRUE || next_command == GOTOFALSE || next_command == GOTOMORE || next_command == GOTOLESS)
				{
					VAL(arg1, arg1_from_register);
					VAL(arg2, arg2_from_register);
					switch (next_command)
					{
						case GOTOCMP:
						{
							switch(next_arg2 & CMP_FLAGS)
							{
								default:
									// TODO this is a nop...
									wasm.emitDrop();
									wasm.emitDrop();
									wasm.emitI32Const(0);
									break;
								case CMP_GT:
									wasm.emitI32GtS();
									break;
								case CMP_GT|CMP_EQ:
									wasm.emitI32GeS();
									break;
								case CMP_LT:
									wasm.emitI32LtS();
									break;
								case CMP_LT|CMP_EQ:
									wasm.emitI32LeS();
									break;
								case CMP_EQ:
									wasm.emitI32Eq();
									break;
								case CMP_GT|CMP_LT:
									wasm.emitI32Ne();
									break;
								case CMP_GT|CMP_LT|CMP_EQ:
									// TODO could avoid getting values ...
									wasm.emitDrop();
									wasm.emitDrop();
									wasm.emitI32Const(1);
									break;
							}
						}
						break;
						case GOTOTRUE: wasm.emitI32Eq(); break;
						case GOTOFALSE: wasm.emitI32Ne(); break;
						case GOTOMORE: wasm.emitI32GeS(); break;
						case GOTOLESS: wasm.emitI32LeS(); break;
					}

					// Set target block index, in case we must branch.
					size_t target_block_index = cfg.block_id_from_start_pc(next_arg1) + 1;
					wasm.emitI32Const(target_block_index);
					wasm.emitGlobalSet(g_idx_target_block_id);

					// Maybe branch by jumping to start of loop-switch.
					wasm.emitBrIf(num_blocks - current_block_index);
				}
				else
				{
					printf("unexpected command %s at index %d\n", zasm_op_to_string(next_command).c_str(), i + 1);
					ASSERT(false);
				}

				i += 1;
				continue;
			}

			if (command_is_wait(command))
			{
				ASSERT(may_yield);

				state.pc_to_block_id[i] = current_block_index + 1;

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
						// When calling functions that do not yield, we can use wasm.emitCall to use
						// a native function call. This does not require g_idx_ret_stack_index.
						// However, we still increment g_idx_ret_stack_index so we can check for the
						// call limit.
						check_call_limit(state);

						pc_t fn_id = structured_zasm.start_pc_to_function.at(arg1);
						if (!may_yield || !structured_zasm.functions[fn_id].may_yield)
						{
							// g_idx_ret_stack_index += 1
							modify_global_idx(wasm, g_idx_ret_stack_index, 1);

							// This is a function call, the function is compiled as a separate WASM function, so we can simply
							// call it and be done with this instruction. Set the initial block id to 0 so the function starts
							// at its beginning.
							wasm.emitI32Const(0);
							wasm.emitGlobalSet(state.g_idx_target_block_id);

							pc_t fn_idx = function_id_to_idx.at(fn_id);
							wasm.emitCall(fn_idx);

							// g_idx_ret_stack_index -= 1
							modify_global_idx(wasm, g_idx_ret_stack_index, -1);
							continue;
						}

						state.pc_to_block_id[i] = current_block_index + 1;

						// The function call is to some other may-yield function which is inlined in the yielder function.
						// Before we "call" it, we need to remember where to return to. To do that, we push the index of the
						// subsequent block to a return call stack. RETURNFUNC will later pop that block index.

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

						// g_idx_ret_stack_index -= 1
						modify_global_idx(wasm, g_idx_ret_stack_index, -1);

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
				{
					compile_command_interpreter(state, script, i, 1);
					state.wasm->emitI32Const(RUNSCRIPT_STOPPED);
					state.wasm->emitCall(state.f_idx_set_return_value);
					state.wasm->emitUnreachable(); // Bail.
				}
				break;

				case PUSHR:
				case PUSHV:
				{
					if (command == PUSHR && (arg1 == SP || arg1 == SP2))
					{
						wasm.emitGlobalGet(g_idx_sp);
						if (arg1 == SP)
						{
							wasm.emitI32Const(10000);
							wasm.emitI32Mul();
						}
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
						break;
					}

					add_sp(state, -1);
					check_sp(state);

					wasm.emitGlobalGet(g_idx_sp);
					wasm.emitI32Const(4);
					wasm.emitI32Mul(); // Multiply by 4 to get byte offset.
					wasm.emitGlobalGet(g_idx_stack);
					wasm.emitI32Add(); // Add stack base offset.
					if (command == PUSHR)
						get_z_register(state, arg1);
					else
						wasm.emitI32Const(arg1);
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

				default:
				{
					printf("unexpected command %s at index %d\n", zasm_op_to_string(command).c_str(), i);
					ASSERT(false);
				}
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

JittedScript* jit_compile_script(zasm_script* script)
{
	if (script->size <= 1)
		return nullptr;

	// TODO: support RUNGENFRZSCR by using do_commands_async, which returns a promise. Need a way to defer execution until promise resolves...
	// https://emscripten.org/docs/porting/asyncify.html
	// Might need to use atomics. First pass for WASM compiler used that, for reference: https://github.com/connorjclark/ZeldaClassic/commit/eb5fd2c7d83ce084569fe3e73be1a69383416f58
	for (size_t i = 0; i < script->size; i++)
	{
		int command = script->zasm[i].command;
		if (command == RUNGENFRZSCR)
		{
			// error(script, "RUNGENFRZSCR unsupported", true);
			return nullptr;
		}
		else if (command == STACKWRITEATVV_IF)
		{
			error(script, "STACKWRITEATVV_IF unsupported", true);
			return nullptr;
		}
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

	state.f_idx_set_return_value = comp.builder.importFunction("set_return_value", 1, 0);
	state.f_idx_do_commands = comp.builder.importFunction("do_commands", 4, 1);
	state.f_idx_do_commands_async = comp.builder.importFunction("do_commands_async", 4, 1);
	state.f_idx_get_register = comp.builder.importFunction("get_register", 1, 1);
	state.f_idx_set_register = comp.builder.importFunction("set_register", 2, 0);
	state.f_idx_runtime_debug = comp.builder.importFunction("runtime_debug", 2, 0);
	state.f_idx_log_error = comp.builder.importFunction("log_error", 1, 0);

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
	state.g_idx_ret_stack_index = comp.builder.addGlobal("ret_stack_index");
	state.g_idx_wait_index = comp.builder.addGlobal("wait_index");
	state.g_idx_sp = comp.builder.addGlobal("sp");
	state.g_idx_target_block_id = comp.builder.addGlobal("target_block_id");
	state.g_idx_start_pc = comp.builder.addGlobal("start_pc");

	pc_t fn_run_idx = comp.builder.declareFunction({WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32, WasmValType::I32}, {});
	comp.builder.exportFunction(fn_run_idx, "run");

	auto yielding_fns = zasm_find_yielding_functions(script, structured_zasm);
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
		comp.builder.defineFunction(fn_yielder_idx, {WasmValType::I32}, wasm.finish());
		comp.builder.setFunctionName(fn_yielder_idx, "yielder");
	}

	for (const auto& fn : structured_zasm.functions)
	{
		if (yielding_fns.contains(fn.id))
			continue;

		bool may_yield = false;
		auto wasm = compile_function(state, script, structured_zasm, {fn.id}, function_id_to_idx, may_yield);
		pc_t fidx = function_id_to_idx.at(fn.id);
		comp.builder.defineFunction(fidx, {WasmValType::I32}, wasm.finish());
		if (fn.name().empty())
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
				wasm.emitCall(fn_idx); // call then return.
				wasm.emitReturn();
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
	fn->pc_to_block_id = std::move(state.pc_to_block_id);

	return fn;
}

JittedScriptInstance* jit_create_script_impl(script_data* script, refInfo* ri, JittedScript* j_script, bool just_initialized)
{
	// Since we currently can't compile WASM scripts on background threads, there's never a need to
	// upgrade an existing script. Instead, when jit is enabled on the web, we precompile all
	// scripts when the quest first loads.
	if (!just_initialized)
	{
		// Should never actually happen.
		CHECK(false);
		return nullptr;
	}

	JittedScriptInstance* j_instance = new JittedScriptInstance{};

	j_instance->j_script = j_script;
	j_instance->script = script;
	j_instance->ri = ri;

	// TODO: this probably works, but not tested (see above).
	if (!just_initialized)
	{
		if (auto r = util::find(j_script->pc_to_block_id, ri->pc - 1))
		{
			j_instance->wait_index = *r;
		}
		else
		{
			al_trace("[jit] bail on upgrade, ri->pc = %d\n", ri->pc);
			delete j_instance;
			if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL) abort();
			DCHECK(false);
			return nullptr;
		}

		extern int32_t(*ret_stack)[MAX_CALL_FRAMES];
		j_instance->call_stack_ret_index = ri->retsp;
		for (int i = 0; i < ri->retsp; i++)
		{
			pc_t pc = (*ret_stack)[i];
			if (auto r = util::find(j_script->pc_to_block_id, pc - 1))
			{
				j_instance->call_stack_rets[i] = *r;
			}
			else
			{
				al_trace("[jit] bail on upgrade, bad call stack return pc = %d\n", pc);
				delete j_instance;
				if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL) abort();
				DCHECK(false);
				return nullptr;
			}
		}

		jit_printf("[jit] running script upgraded to jit: %s\n", script->name().c_str());
		ri->retsp = 0;
	}

	int handle_id = em_create_wasm_handle(j_script->module_id);
	if (!handle_id)
	{
		al_trace("[jit] Error creating wasm handle for script: %s\n", script->zasm_script->name.c_str());
		return nullptr;
	}

	j_instance->handle_id = handle_id;

	return j_instance;
}

void jit_profiler_increment_function_back_edge(JittedScriptInstance* j_instance, pc_t pc)
{
	// Not used - the wasm backend currently only supports precompiling scripts.
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

	int return_code = em_poll_wasm_handle(j_instance->handle_id, (uintptr_t)&ptr);

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
	// TODO: should select the correct command at compile time, like jix_x64 does. Just being lazy.
	do_set(r, value);
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

#endif

JittedScriptInstance::~JittedScriptInstance()
{
	ASSERT(em_destroy_wasm_handle(handle_id));
}
