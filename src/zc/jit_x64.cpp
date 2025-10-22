#include "zc/jit_x64.h"
#include "asmjit/x86/x86operand.h"
#include "base/general.h"
#include "base/qrs.h"
#include "base/util.h"
#include "base/zdefs.h"
#include "zasm/table.h"
#include "zc/jit.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zasm_pipeline.h"
#include "zc/zasm_utils.h"
#include "zasm/serialize.h"
#include "zconsole/ConsoleLogger.h"
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <chrono>
#include <optional>
#include <asmjit/asmjit.h>

using namespace asmjit;

static JitRuntime rt;

static int EXEC_RESULT_UNKNOWN = 0;
static int EXEC_RESULT_CONTINUE = 1;
static int EXEC_RESULT_CALL = 2;
static int EXEC_RESULT_RETURN = 3;
static int EXEC_RESULT_EXIT = 4;

static int hot_function_loop_count_threshold;
static int hot_function_call_count_threshold;

struct CachedRegister
{
	x86::Gp reg;
	bool dirty;
};

struct CachedStackValue
{
	x86::Gp reg;
	bool is_constant = false;
	int value = -1;
	int amount = 1;
};

struct CompilationState
{
	zasm_script* script;
	JittedScript* j_script;
	pc_t start_pc;
	pc_t final_pc;
	CallConvId calling_convention;
	Label L_End;
	x86::Gp vResult;
	x86::Gp vSp;
	x86::Gp vSwitchKey;
	bool use_cached_regs;
	std::map<int, CachedRegister> cached_d_regs;
	std::vector<CachedStackValue> cached_d_reg_stack;
	x86::Gp ptrCtx;
	x86::Gp ptrRegisters;
	x86::Gp ptrStackBase;
	std::map<int, Label> goto_labels;
	// Labels for both function calls and wait frame commands.
	std::map<int, Label> resume_labels;
	// When to end the "lookahead" for stack pointer bounds checking (and start checking again).
	pc_t num_push_commands_in_row_end_pc;
	bool modified_stack;
	bool runtime_debugging;
};

extern ScriptDebugHandle* runtime_script_debug_handle;

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

static void debug_pre_command(int32_t pc, uint32_t sp)
{
	extern refInfo *ri;

	ri->pc = pc;
	ri->sp = sp;
	if (runtime_script_debug_handle)
		runtime_script_debug_handle->pre_command();
}

class MyErrorHandler : public ErrorHandler
{
public:
	void handleError(Error err, const char *message, BaseEmitter *origin) override
	{
		al_trace("[jit ERROR] AsmJit error: %s\n", message);
	}
};

template <typename T>
static void set_ctx_pc(CompilationState& state, x86::Compiler& cc, T pc)
{
	cc.mov(x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, pc)), pc);
}

static void set_ctx_call_pc(CompilationState& state, x86::Compiler& cc, pc_t call_pc)
{
	cc.mov(x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, call_pc)), call_pc);
}

template <typename T>
static void set_ctx_sp(CompilationState& state, x86::Compiler& cc, T sp)
{
	cc.mov(x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, sp)), sp);
}

template <typename T>
static void set_ctx_ret_code(CompilationState& state, x86::Compiler& cc, T ret_code)
{
	cc.mov(x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, ret_code)), ret_code);
}

static void restore_regs(CompilationState& state, x86::Compiler& cc)
{
	cc.mov(x86::rbx, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 24));
	cc.mov(x86::r15, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 16));
	cc.mov(x86::r14, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 8));
	cc.mov(x86::r13, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 0));
}

static void modify_sp(CompilationState& state, x86::Compiler& cc, x86::Gp vStackIndex, int delta)
{
	cc.add(vStackIndex, delta);
	state.modified_stack = true;
}

static void check_sp(CompilationState& state, x86::Compiler& cc, x86::Gp vStackIndex, int offset = 0)
{
	if (offset == 0)
	{
		cc.cmp(vStackIndex, MAX_STACK_SIZE);
	}
	else
	{
		x86::Gp val = cc.newUInt32();
		cc.mov(val, vStackIndex);
		// Prefer inc/dec for smaller code size.
		if (offset == 1)
			cc.inc(val);
		else if (offset == -1)
			cc.dec(val);
		else
			cc.add(val, offset);
		cc.cmp(val, MAX_STACK_SIZE);
	}

	Label label = cc.newLabel();
	cc.jb(label);
	set_ctx_ret_code(state, cc, RUNSCRIPT_JIT_STACK_OVERFLOW);
	cc.mov(state.vResult, EXEC_RESULT_EXIT);
	restore_regs(state, cc);
	cc.ret(state.vResult);
	cc.bind(label);
}

static void do_stack_push_many(CompilationState& state, x86::Compiler& cc, int offset, int amount, x86::Gp val)
{
	// Push onto stack [amount] times.
	if (amount < 8)
	{
		// For small [amount], it's likely faster to emit a bunch of movs.
		for (int i = 0; i < amount; i++)
			cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2, (offset - i) * 4), val);
	}
	else
	{
		// Otherwise, rep stos is probably faster.
		// See: https://reviews.llvm.org/D32002
		x86::Gp num = cc.newInt32();
		cc.mov(num, amount);
		int start_offset_bytes = (offset - amount + 1) * 4;
		x86::Gp address = cc.newIntPtr();
		cc.lea(address, x86::ptr_32(state.ptrStackBase, state.vSp, 2, start_offset_bytes));
		cc.rep(num).stos(x86::ptr_32(address), val);
	}
}

static void flush_stack_cache(CompilationState& state, x86::Compiler& cc)
{
	if (!state.cached_d_reg_stack.empty())
	{
		if (DEBUG_JIT_PRINT_ASM)
			cc.setInlineComment("flush cached stack");

		int stack_change = 0;
		for (auto& [reg, is_constant, value, amount] : state.cached_d_reg_stack)
		{
			stack_change += amount;
		}

		check_sp(state, cc, state.vSp, -stack_change);
		modify_sp(state, cc, state.vSp, -stack_change);

		int i = stack_change - 1;
		for (auto& [reg, is_constant, value, amount] : state.cached_d_reg_stack)
		{
			if (is_constant)
			{
				if (amount < 8)
				{
					for (int j = 0; j < amount; j++)
						cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2, (i - j) * 4), value);
				}
				else
				{
					x86::Gp reg = cc.newInt32();
					cc.mov(reg, value);
					do_stack_push_many(state, cc, i, amount, reg);
				}
			}
			else
			{
				do_stack_push_many(state, cc, i, amount, reg);
			}

			i -= amount;
		}
		state.cached_d_reg_stack.clear();
	}
}

static void flush_cache(CompilationState& state, x86::Compiler& cc, uint8_t register_mask = -1)
{
	if (!state.cached_d_regs.empty())
	{
		if (DEBUG_JIT_PRINT_ASM)
			cc.setInlineComment("flush cached registers");

		for (auto& [r, cached_reg] : state.cached_d_regs)
		{
			if (cached_reg.dirty && (register_mask & (1 << r)))
				cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), cached_reg.reg);
		}
		state.cached_d_regs.clear();
	}

	flush_stack_cache(state, cc);
}

static void write_some_cached_registers(CompilationState& state, x86::Compiler& cc, uint8_t register_mask)
{
	if (state.cached_d_regs.empty())
		return;

	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("write cached registers");

	for (auto& [r, cached_reg] : state.cached_d_regs)
	{
		if (cached_reg.dirty && (register_mask & (1 << r)))
			cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), cached_reg.reg);
	}
}

static void flush_cache_for_dependent_registers(CompilationState& state, x86::Compiler& cc, int r)
{
	if (state.cached_d_regs.empty())
		return;

	auto dep_regs = get_register_dependencies(r);
	if (dep_regs.empty())
		return;

	std::erase_if(state.cached_d_regs, [&](const auto& pair){
		if (util::contains(dep_regs, pair.first))
		{
			if (pair.second.dirty)
				cc.mov(x86::ptr_32(state.ptrRegisters, pair.first * 4), pair.second.reg);
			return true;
		}

		return false;
	});
}

static int32_t get_register_and_restore_sp(int32_t arg, uint32_t sp)
{
	extern refInfo *ri;

	ri->sp = sp;
	return get_register(arg);
}

static void set_register_and_restore_sp(int32_t arg, int32_t value, uint32_t sp)
{
	extern refInfo *ri;

	ri->sp = sp;
	set_register(arg, value);
}

// Must use virtual register to pass as function argument via cc.invoke.
static x86::Gp get_tmp_sp(CompilationState& state, x86::Compiler& cc)
{
	x86::Gp sp = cc.newInt32();
	cc.mov(sp, state.vSp);
	return sp;
}

static x86::Gp get_z_register(CompilationState& state, x86::Compiler& cc, int r)
{
	if (state.use_cached_regs && r >= D(0) && r < D(INITIAL_D))
	{
		if (!state.cached_d_regs.contains(r))
		{
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrRegisters, r * 4));
			state.cached_d_regs[r] = {.reg=val};
			return val;
		}

		return state.cached_d_regs[r].reg;
	}

	x86::Gp val = cc.newInt32();
	if (r >= D(0) && r < D(INITIAL_D))
	{
		cc.mov(val, x86::ptr_32(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		x86::Gp address = cc.newIntPtr();
		cc.mov(address, &game->global_d); // Note: this is only OK b/c the `game` global pointer is never reassigned.
		cc.mov(val, x86::ptr_32(address, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		flush_stack_cache(state, cc);
		cc.mov(val, state.vSp);
		cc.imul(val, 10000);
	}
	else if (r == SP2)
	{
		flush_stack_cache(state, cc);
		cc.mov(val, state.vSp);
	}
	else if (r == SWITCHKEY)
	{
		cc.mov(val, state.vSwitchKey);
	}
	else if (does_register_use_stack(r))
	{
		flush_cache_for_dependent_registers(state, cc, r);
		flush_stack_cache(state, cc);
		x86::Gp stackIndex = get_tmp_sp(state, cc);

		// Call external get_register_and_restore_sp.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register_and_restore_sp, FuncSignatureT<int32_t, int32_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, stackIndex);
		invokeNode->setRet(0, val);
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external get_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val);
	}
	return val;
}

static x86::Gp get_z_register_64(CompilationState& state, x86::Compiler& cc, int r)
{
	x86::Gp val = cc.newInt64();
	if (r >= D(0) && r < D(INITIAL_D))
	{
		if (state.use_cached_regs)
		{
			x86::Gp val32;
			if (!state.cached_d_regs.contains(r))
			{
				x86::Gp val32 = cc.newInt32();
				cc.mov(val32, x86::ptr_32(state.ptrRegisters, r * 4));
				state.cached_d_regs[r] = {.reg=val32};
			}

			val32 = state.cached_d_regs[r].reg;
			cc.movsxd(val, val32);
		}
		else
		{
			cc.movsxd(val, x86::ptr_32(state.ptrRegisters, r * 4));
		}
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		x86::Gp address = cc.newIntPtr();
		cc.mov(address, &game->global_d); // Note: this is only OK b/c the `game` global pointer is never reassigned.
		cc.movsxd(val, x86::ptr_32(address, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.movsxd(val, state.vSp);
		cc.imul(val, 10000);
	}
	else if (r == SP2)
	{
		cc.movsxd(val, state.vSp);
	}
	else if (r == SWITCHKEY)
	{
		cc.movsxd(val, state.vSwitchKey);
	}
	else if (does_register_use_stack(r))
	{
		flush_cache_for_dependent_registers(state, cc, r);
		flush_stack_cache(state, cc);
		x86::Gp stackIndex = get_tmp_sp(state, cc);

		// Call external get_register_and_restore_sp.
		x86::Gp val32 = cc.newInt32();
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register_and_restore_sp, FuncSignatureT<int32_t, int32_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, stackIndex);
		invokeNode->setRet(0, val32);
		cc.movsxd(val, val32);
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external get_register.
		x86::Gp val32 = cc.newInt32();
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val32);
		cc.movsxd(val, val32);
	}
	return val;
}

template <typename T>
static void set_z_register(CompilationState& state, x86::Compiler& cc, int r, T val)
{
	if (r >= D(0) && r < D(INITIAL_D))
	{
		if (state.use_cached_regs)
		{
			for (auto& [reg, is_constant, value, amount] : state.cached_d_reg_stack)
			{
				if (!is_constant && value == r)
				{
					state.cached_d_regs.erase(r);
					break;
				}
			}

			if (state.cached_d_regs.contains(r))
			{
				auto& cached_reg = state.cached_d_regs[r];
				cc.mov(cached_reg.reg, val);
				cached_reg.dirty = true;
			}
			else
			{
				x86::Gp reg = cc.newInt32();
				state.cached_d_regs[r] = {.reg = reg, .dirty = true};
				cc.mov(reg, val);
			}
		}
		else
		{
			cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), val);
		}
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		x86::Gp address = cc.newIntPtr();
		cc.mov(address, &game->global_d); // Note: this is only OK b/c the `game` global pointer is never reassigned.
		cc.mov(x86::ptr_32(address, (r - GD(0)) * 4), val);
	}
	else if (r == SP || r == SP2)
	{
		// TODO
		Z_error_fatal("Unimplemented: set SP");
	}
	else if (r == SWITCHKEY)
	{
		state.vSwitchKey = cc.newInt32();
		cc.mov(state.vSwitchKey, val);
	}
	else if (does_register_use_stack(r))
	{
		flush_cache_for_dependent_registers(state, cc, r);
		flush_stack_cache(state, cc);
		x86::Gp stackIndex = get_tmp_sp(state, cc);

		// Call external set_register_and_restore_sp.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_register_and_restore_sp, FuncSignatureT<void, int32_t, int32_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
		invokeNode->setArg(2, stackIndex);
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Only some registers have an extra check when writing to them.
		auto set_fn = set_register;
		if (is_guarded_script_register(r))
			set_fn = do_set;

		// Call external set_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_fn, FuncSignatureT<void, int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
	}
}

static void set_z_register(CompilationState& state, x86::Compiler& cc, int r, x86::Mem mem)
{
	x86::Gp val = cc.newInt32();
	cc.mov(val, mem);
	set_z_register(state, cc, r, val);
}

static x86::Gp get_ctx_script_instance(CompilationState& state, x86::Compiler& cc)
{
	x86::Gp ptr = cc.newIntPtr();
	cc.mov(ptr, x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, j_instance)));
	return ptr;
}

static void div_10000(x86::Compiler& cc, x86::Gp dividend)
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
		abort();
	}
}

static void zero(x86::Compiler& cc, x86::Gp reg)
{
	cc.xor_(reg, reg);
}

static void add_constant(x86::Compiler& cc, x86::Gp reg, int value)
{
	if (value == 0)
		return;

	// Prefer inc/dec for smaller code size.
	if (value == 1)
		cc.inc(reg);
	else if (value == -1)
		cc.dec(reg);
	else
		cc.add(reg, value);
}

static x86::Gp immutable_add_constant(x86::Compiler& cc, x86::Gp reg, int value)
{
	if (value == 0)
		return reg;

	x86::Gp r = cc.newInt32();
	cc.mov(r, reg);
	add_constant(cc, r, value);
	return r;
}

static void cast_bool(x86::Compiler& cc, x86::Gp reg)
{
	cc.test(reg, reg);
	cc.mov(reg, 0);
	cc.setne(reg.r8());
}

static void compile_compare_goto(CompilationState& state, x86::Compiler& cc, int pc, int command, int arg1, int arg2, int arg3)
{
	auto& goto_labels = state.goto_labels;
	
	if(command == GOTOCMP)
	{
		auto lbl = goto_labels[arg1];
		switch(arg2 & CMP_FLAGS)
		{
			default:
				break;
			case CMP_GT:
				cc.jg(lbl);
				break;
			case CMP_GT|CMP_EQ:
				cc.jge(lbl);
				break;
			case CMP_LT:
				cc.jl(lbl);
				break;
			case CMP_LT|CMP_EQ:
				cc.jle(lbl);
				break;
			case CMP_EQ:
				cc.je(lbl);
				break;
			case CMP_GT|CMP_LT:
				cc.jne(lbl);
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				cc.jmp(lbl);
				break;
		}
	}
	else if (command == GOTOTRUE)
	{
		cc.je(goto_labels[arg1]);
	}
	else if (command == GOTOFALSE)
	{
		cc.jne(goto_labels[arg1]);
	}
	else if (command == GOTOMORE)
	{
		cc.jge(goto_labels[arg1]);
	}
	else if (command == GOTOLESS)
	{
		if (get_qr(qr_GOTOLESSNOTEQUAL))
			cc.jle(goto_labels[arg1]);
		else
			cc.jl(goto_labels[arg1]);
	}
	else
	{
		Z_error_fatal("Unimplemented: %s", zasm_op_to_string(command, arg1, arg2, arg3, nullptr, nullptr).c_str());
	}
}

static void compile_compare(CompilationState& state, x86::Compiler& cc, int pc, int command, int arg1, int arg2, int arg3)
{
	if (command_is_goto(command))
	{
		compile_compare_goto(state, cc, pc, command, arg1, arg2, arg3);
		return;
	}

	x86::Gp val = cc.newInt32();

	if (command == SETCMP)
	{
		cc.mov(val, 0);
		bool i10k = (arg2 & CMP_SETI);
		x86::Gp val2;
		if(i10k)
		{
			val2 = cc.newInt32();
			cc.mov(val2, 10000);
		}
		switch(arg2 & CMP_FLAGS)
		{
			default:
				break;
			case CMP_GT:
				if(i10k)
					cc.cmovg(val, val2);
				else cc.setg(val);
				break;
			case CMP_GT|CMP_EQ:
				if(i10k)
					cc.cmovge(val, val2);
				else cc.setge(val);
				break;
			case CMP_LT:
				if(i10k)
					cc.cmovl(val, val2);
				else cc.setl(val);
				break;
			case CMP_LT|CMP_EQ:
				if(i10k)
					cc.cmovle(val, val2);
				else cc.setle(val);
				break;
			case CMP_EQ:
				if(i10k)
					cc.cmove(val, val2);
				else cc.sete(val);
				break;
			case CMP_GT|CMP_LT:
				if(i10k)
					cc.cmovne(val, val2);
				else cc.setne(val);
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				if(i10k)
					cc.mov(val, 10000);
				else cc.mov(val, 1);
				break;
		}
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETTRUE)
	{
		cc.mov(val, 0);
		cc.sete(val);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETTRUEI)
	{
		// https://stackoverflow.com/a/45183084/2788187
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmove(val, val2);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETFALSE)
	{
		cc.mov(val, 0);
		cc.setne(val);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETFALSEI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovne(val, val2);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETMOREI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovge(val, val2);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETLESSI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovle(val, val2);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETMORE)
	{
		cc.mov(val, 0);
		cc.setge(val);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETLESS)
	{
		cc.mov(val, 0);
		cc.setle(val);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == STACKWRITEATVV_IF)
	{
		// TODO: needs to check for stack overflow.
		// Write directly value on the stack (arg1 to offset arg2)
		x86::Gp offset = immutable_add_constant(cc, state.vSp, arg2);
		auto cmp = arg3 & CMP_FLAGS;
		switch(cmp) //but only conditionally
		{
			case 0:
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), arg1);
				break;
			default:
			{
				x86::Gp tmp = cc.newInt32();
				x86::Gp val = cc.newInt32();
				cc.mov(tmp, x86::ptr_32(state.ptrStackBase, offset, 2));
				cc.mov(val, arg1);
				switch(cmp)
				{
					case CMP_GT:
						cc.cmovg(tmp, val);
						break;
					case CMP_GT|CMP_EQ:
						cc.cmovge(tmp, val);
						break;
					case CMP_LT:
						cc.cmovl(tmp, val);
						break;
					case CMP_LT|CMP_EQ:
						cc.cmovle(tmp, val);
						break;
					case CMP_EQ:
						cc.cmove(tmp, val);
						break;
					case CMP_GT|CMP_LT:
						cc.cmovne(tmp, val);
						break;
					default:
						assert(false);
				}
				cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), tmp);
			}
		}
	}
	else
	{
		Z_error_fatal("[jit ERROR] Unimplemented command: %s", zasm_op_to_string(command, arg1, arg2, arg3, nullptr, nullptr).c_str());
	}
}

static void ret_if_not_ok(CompilationState& state, x86::Compiler& cc, x86::Gp reg)
{
	Label L_noret = cc.newLabel();
	cc.cmp(reg, RUNSCRIPT_OK);
	cc.je(L_noret);

	set_ctx_ret_code(state, cc, reg);
	cc.jmp(state.L_End);

	cc.bind(L_noret);
}

// Defer to the ZASM command interpreter for 1+ commands.
static void compile_command_interpreter(CompilationState& state, x86::Compiler& cc, zasm_script *script, int i, int count, bool is_wait = false)
{
	x86::Gp scriptInstancePtr = get_ctx_script_instance(state, cc);
	x86::Gp stackIndex = get_tmp_sp(state, cc);

	InvokeNode *invokeNode;
	if (count == 1)
	{
		cc.invoke(&invokeNode, run_script_jit_one, FuncSignatureT<int32_t, JittedScriptInstance*, int32_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, i);
		invokeNode->setArg(2, stackIndex);
	}
	else
	{
		cc.invoke(&invokeNode, run_script_jit_sequence, FuncSignatureT<int32_t, JittedScriptInstance*, int32_t, uint32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, i);
		invokeNode->setArg(2, stackIndex);
		invokeNode->setArg(3, count);
	}

	if (is_wait)
	{
		set_ctx_pc(state, cc, i + 1);

		x86::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
		// Only wait if the return value is RUNSCRIPT_STOPPED (this supports conditional waits).
		cc.cmp(retVal, RUNSCRIPT_STOPPED);
		// If actually waiting, the return value will be RUNSCRIPT_OK. set_ctx_ret_code isn't called
		// here because RUNSCRIPT_OK is the default value.
		cc.je(state.L_End);

		cc.bind(state.resume_labels[i]);
		return;
	}

	bool could_return_not_ok = false;
	for (int j = 0; j < count; j++)
	{
		int index = i + j;
		if (command_could_return_not_ok(script->zasm[index].command))
		{
			could_return_not_ok = true;
			break;
		}
	}

	if (could_return_not_ok)
	{
		x86::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
		ret_if_not_ok(state, cc, retVal);
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
	// were not compiled, then vStackIndex would have to be restored after compile_command_interpreter.
	case POP:
	case POPARGS:
	case PUSHR:
	case PUSHV:
	case PUSHARGSR:
	case PUSHARGSV:

	// These can be commented out to instead run interpreted. Useful for
	// singling out problematic commands.
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
	case REF_REMOVE:
	case SETR:
	case SETV:
	case STACKWRITEATVV:
	case STORE_OBJECT:
	case STORE:
	case STORED:
	case STOREDV:
	case STOREI:
	case STOREV:
	case SUBR:
	case SUBV:
	case SUBV2:
		return true;
	}

	return false;
}

// Check for stack overflows, but only once per contiguous series of PUSH (or POP) commands.
static void handle_check_sp_push(CompilationState& state, x86::Compiler& cc, const zasm_script* script, pc_t cur, x86::Gp vStackIndex)
{
	if (cur >= state.num_push_commands_in_row_end_pc)
	{
		int stack_delta = 1;
		int max_stack_delta = 1;

		int j = cur + 1;
		for (; j < script->size; j++)
		{
			const auto& op = script->zasm[j];
			if (op.command == PUSHV || op.command == PUSHR)
			{
				stack_delta++;
				max_stack_delta = std::max(max_stack_delta, stack_delta);
			}
			else if (op.command == PUSHARGSV || op.command == PUSHARGSR)
			{
				stack_delta += op.arg2;
				max_stack_delta = std::max(max_stack_delta, stack_delta);
			}
			else if (op.command == POP)
			{
				stack_delta -= 1;
			}
			else if (op.command == POPARGS)
			{
				stack_delta -= op.arg2;
			}
			else if (op.command == NOP)
			{
				continue;
			}
			else
			{
				break;
			}
		}

		check_sp(state, cc, state.vSp, -max_stack_delta);
		state.num_push_commands_in_row_end_pc = j;
	}
}

static void log_error_div_0()
{
	scripting_log_error_with_context("Attempted to divide by zero!");
}

static void log_error_mod_0()
{
	scripting_log_error_with_context("Attempted to modulo by zero!");
}

// Useful if crashing at runtime to find the last command that ran.
// #define JIT_DEBUG_CRASH
#ifdef JIT_DEBUG_CRASH
static size_t debug_last_pc;
#endif

static x86::Gp compile_modv(CompilationState& state, x86::Compiler& cc, x86::Gp arg1, int arg2)
{
	if (arg2 == 0)
	{
		x86::Gp val = cc.newInt32();
		zero(cc, val);

		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, log_error_div_0, FuncSignatureT<void>(state.calling_convention));
		return val;
	}

	// https://stackoverflow.com/a/8022107/2788187
	if (arg2 > 0 && (arg2 & (-arg2)) == arg2)
	{
		// Power of 2.
		// Because numbers in zscript are fixed point, "2" is really "20000"... so this won't
		// ever really be utilized.
		cc.and_(arg1, arg2 - 1);
		return arg1;
	}
	else
	{
		x86::Gp divisor = cc.newInt32();
		cc.mov(divisor, arg2);
		x86::Gp rem = cc.newInt32();
		zero(cc, rem);
		cc.cdq(rem, arg1);
		cc.idiv(rem, arg1, divisor);
		return rem;
	}
}

// Every command here must be reflected in command_is_compiled!
static void compile_single_command(CompilationState& state, x86::Compiler& cc, const ffscript& instr, pc_t pc, zasm_script *script)
{
	int command = instr.command;
	int arg1 = instr.arg1;
	int arg2 = instr.arg2;

	switch (command)
	{
		case NOP:
			break;
		case QUIT:
		{
			compile_command_interpreter(state, cc, script, pc, 1);
			cc.mov(state.vResult, EXEC_RESULT_EXIT);
			set_ctx_ret_code(state, cc, RUNSCRIPT_STOPPED);
			cc.jmp(state.L_End);
		}
		break;
		case GOTO:
		{
			if (arg1 >= state.start_pc && arg1 <= state.final_pc)
			{
				cc.jmp(state.goto_labels[arg1]);
			}
			else
			{
				// Mostly all GOTOs should be within the same function. The only exception is the
				// end of the compiler-generated Init script (the part that sets up globals). When
				// that section ends, it unconditionally jumps to the user-defined init script.
				set_ctx_pc(state, cc, arg1);
				cc.mov(state.vResult, EXEC_RESULT_CONTINUE);
				cc.jmp(state.L_End);
			}
		}
		break;
		case CALLFUNC:
		{
			if (pc == state.final_pc)
			{
				// If CALLFUNC is the last command, then it is calling a function that never
				// returns. Let's just move the pc to the target function.
				set_ctx_pc(state, cc, arg1);
				cc.mov(state.vResult, EXEC_RESULT_CONTINUE);
				cc.jmp(state.L_End);
			}
			else
			{
				set_ctx_pc(state, cc, pc);
				set_ctx_call_pc(state, cc, arg1);
				if (state.modified_stack)
					set_ctx_sp(state, cc, state.vSp);
				cc.mov(state.vResult, EXEC_RESULT_CALL);
				restore_regs(state, cc);
				cc.ret(state.vResult);
				cc.bind(state.resume_labels[pc]);
			}
		}
		break;
		case RETURNFUNC:
		{
			if (state.modified_stack)
				set_ctx_sp(state, cc, state.vSp);
			cc.mov(state.vResult, EXEC_RESULT_RETURN);
			restore_regs(state, cc);
			cc.ret(state.vResult);
		}
		break;
		case STACKWRITEATVV:
		{
			// TODO: needs to check for stack overflow.
			// Write directly value on the stack (arg1 to offset arg2)
			cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2, arg2 * 4), arg1);
		}
		break;
		case PUSHV:
		{
			if (state.use_cached_regs)
			{
				state.cached_d_reg_stack.push_back({.is_constant=true, .value=arg1});
				break;
			}

			handle_check_sp_push(state, cc, script, pc, state.vSp);

			modify_sp(state, cc, state.vSp, -1);
			cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2), arg1);
		}
		break;
		case PUSHR:
		{
			if (state.use_cached_regs)
			{
				// Usually don't need to copy the register, but code like `while (i++ < frames)`
				// will push the value of `i` via D2 first, then modify D2 and store it back. asmjit
				// should remove the extra register copy when not necessary during register
				// allocation.
				x86::Gp copy = cc.newInt32();
				cc.mov(copy, get_z_register(state, cc, arg1));
				state.cached_d_reg_stack.push_back({.reg=copy});
				break;
			}

			handle_check_sp_push(state, cc, script, pc, state.vSp);

			// Grab value from register and push onto stack.
			x86::Gp val = get_z_register(state, cc, arg1);
			modify_sp(state, cc, state.vSp, -1);
			cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2), val);
		}
		break;
		case PUSHARGSR:
		{
			if (state.use_cached_regs)
			{
				state.cached_d_reg_stack.push_back({.reg=get_z_register(state, cc, arg1), .value=arg1, .amount=arg2});
				break;
			}

			handle_check_sp_push(state, cc, script, pc, state.vSp);

			if(arg2 < 1) break; //do nothing

			x86::Gp val = get_z_register(state, cc, arg1);
			modify_sp(state, cc, state.vSp, -arg2);

			// Push onto stack [arg2] times.
			if (arg2 < 8)
			{
				// For small [arg2], it's likely faster to emit a bunch of movs.
				for (int i = 0; i < arg2; i++)
					cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2, i * 4), val);
			}
			else
			{
				// Otherwise, rep stos is probably faster.
				// See: https://reviews.llvm.org/D32002
				x86::Gp num = cc.newInt32();
				cc.mov(num, arg2);
				x86::Gp address = cc.newIntPtr();
				cc.lea(address, x86::ptr_32(state.ptrStackBase, state.vSp, 2));
				cc.rep(num).stos(x86::ptr_32(address), val);
			}
		}
		break;
		case PUSHARGSV:
		{
			if (state.use_cached_regs)
			{
				state.cached_d_reg_stack.push_back({.is_constant=true, .value=arg1, .amount=arg2});
				break;
			}

			handle_check_sp_push(state, cc, script, pc, state.vSp);

			if(arg2 < 1) break; //do nothing

			modify_sp(state, cc, state.vSp, -arg2);

			// Push onto stack [arg2] times.
			if (arg2 < 8)
			{
				// For small [arg2], it's likely faster to emit a bunch of movs.
				for (int i = 0; i < arg2; i++)
					cc.mov(x86::ptr_32(state.ptrStackBase, state.vSp, 2, i * 4), arg1);
			}
			else
			{
				// Otherwise, rep stos is probably faster.
				// See: https://reviews.llvm.org/D32002
				x86::Gp val = cc.newInt32();
				cc.mov(val, arg1);
	
				x86::Gp num = cc.newInt32();
				cc.mov(num, arg2);
				x86::Gp address = cc.newIntPtr();
				cc.lea(address, x86::ptr_32(state.ptrStackBase, state.vSp, 2));
				cc.rep(num).stos(x86::ptr_32(address), val);
			}
		}
		break;
		case SETV:
		{
			// For test_jit_runtime_debug_test.
			static bool jit_runtime_debug_test_force_bug = get_flag_bool("-jit-runtime-debug-test-force-bug").value_or(false);
			if (jit_runtime_debug_test_force_bug) arg2++;

			// Set register to immediate value.
			set_z_register(state, cc, arg1, arg2);
		}
		break;
		case SETR:
		{
			// Set register arg1 to value of register arg2.
			x86::Gp val = get_z_register(state, cc, arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LOAD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			set_z_register(state, cc, arg1, x86::ptr_32(state.ptrStackBase, sframe, 2, arg2 * 4));
		}
		break;
		case LOADD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg2);
			div_10000(cc, offset);

			set_z_register(state, cc, arg1, x86::ptr_32(state.ptrStackBase, offset, 2));
		}
		break;
		case LOADI:
		{
			// Set register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, arg2);
			div_10000(cc, offset);

			set_z_register(state, cc, arg1, x86::ptr_32(state.ptrStackBase, offset, 2));
		}
		break;
		case REF_REMOVE:
		{
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg1);

			InvokeNode *invokeNode;
			void script_remove_object_ref(int32_t offset);
			cc.invoke(&invokeNode, script_remove_object_ref, FuncSignatureT<void, int32_t>(state.calling_convention));
			invokeNode->setArg(0, offset);
		}
		break;
		case STORE:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp val = get_z_register(state, cc, arg1);
			cc.mov(x86::ptr_32(state.ptrStackBase, sframe, 2, arg2 * 4), val);
		}
		break;
		case STORE_OBJECT:
		{
			// Same as STORE, but for a ref-counted object.
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg2);

			x86::Gp val = get_z_register(state, cc, arg1);

			InvokeNode *invokeNode;
			void script_store_object(uint32_t offset, uint32_t new_id);
			cc.invoke(&invokeNode, script_store_object, FuncSignatureT<void, uint32_t, uint32_t>(state.calling_convention));
			invokeNode->setArg(0, offset);
			invokeNode->setArg(1, val);
		}
		break;
		case STOREV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			cc.mov(x86::ptr_32(state.ptrStackBase, sframe, 2, arg2 * 4), arg1);
		}
		break;
		case STORED:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg2);
			div_10000(cc, offset);

			x86::Gp val = get_z_register(state, cc, arg1);
			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), val);
		}
		break;
		case STOREDV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg2);
			div_10000(cc, offset);

			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), arg1);
		}
		break;
		case STOREI:
		{
			// Write from register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, arg2);
			div_10000(cc, offset);

			x86::Gp val = get_z_register(state, cc, arg1);
			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), val);
		}
		break;
		case POP:
		{
			if (state.use_cached_regs && !state.cached_d_reg_stack.empty())
			{
				auto& cached_value = state.cached_d_reg_stack.back();
				if (cached_value.is_constant)
					set_z_register(state, cc, arg1, cached_value.value);
				else
					set_z_register(state, cc, arg1, cached_value.reg);

				if (--cached_value.amount == 0)
					state.cached_d_reg_stack.pop_back();
				break;
			}

			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStackBase, state.vSp, 2));
			modify_sp(state, cc, state.vSp, 1);
			set_z_register(state, cc, arg1, val);
		}
		break;
		// Note: I'm pretty sure this always POPs to D5, which is the null register and can be
		// ignored.
		case POPARGS:
		{
			flush_stack_cache(state, cc);

			// int32_t num = sarg2;
			// ri->sp += num;
			modify_sp(state, cc, state.vSp, arg2);

			// word read = ri->sp - 1;
			x86::Gp read = cc.newUInt32();
			cc.mov(read, state.vSp);
			cc.sub(read, 1);

			check_sp(state, cc, read);

			// int32_t value = SH::read_stack(read);
			// set_register(sarg1, value);
			if (arg1 != D(5) || state.runtime_debugging) // Skip setting the "null" register (unless runtime debugging).
			{
				x86::Gp val = cc.newInt32();
				cc.mov(val, x86::ptr_32(state.ptrStackBase, read, 2));
				set_z_register(state, cc, arg1, val);
			}
		}
		break;
		case PEEK:
		{
			if (state.use_cached_regs && !state.cached_d_reg_stack.empty())
			{
				auto& cached_value = state.cached_d_reg_stack.back();
				if (cached_value.is_constant)
				{
					set_z_register(state, cc, arg1, cached_value.value);
					break;
				}
				else if (cached_value.reg.isValid())
				{
					set_z_register(state, cc, arg1, cached_value.reg);
					break;
				}
			}

			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStackBase, state.vSp, 2));
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ABS:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp y = cc.newInt32();
			cc.mov(y, val);
			cc.sar(y, 31);
			cc.xor_(val, y);
			cc.sub(val, y);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case CASTBOOLI:
		{
			// https://clang.godbolt.org/z/W8PM4j33b
			x86::Gp val = get_z_register(state, cc, arg1);
			cc.neg(val);
			cc.sbb(val, val);
			cc.and_(val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case CASTBOOLF:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			cast_bool(cc, val);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ADDV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			add_constant(cc, val, arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ADDR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);
			cc.add(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ANDV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			div_10000(cc, val);
			cc.and_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ANDR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			div_10000(cc, val);
			div_10000(cc, val2);
			cc.and_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			div_10000(cc, val);
			cc.or_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			div_10000(cc, val);
			div_10000(cc, val2);
			cc.or_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORR32:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			cc.or_(val, val2);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORV32:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			cc.or_(val, arg2);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case MAXR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MAXV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MINR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MINV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MODV:
		{
			x86::Gp result = compile_modv(state, cc, get_z_register(state, cc, arg1), arg2);
			set_z_register(state, cc, arg1, result);
		}
		break;
		case MODR:
		{
			x86::Gp dividend = get_z_register(state, cc, arg1);
			x86::Gp divisor = get_z_register(state, cc, arg2);

			Label do_modulo = cc.newLabel();
			Label do_set_register = cc.newLabel();

			x86::Gp rem = cc.newInt32();
			zero(cc, rem);

			cc.test(divisor, divisor);
			cc.jnz(do_modulo);

			InvokeNode *invokeNode;
			cc.invoke(&invokeNode, log_error_mod_0, FuncSignatureT<void>(state.calling_convention));
			cc.jmp(do_set_register);

			cc.bind(do_modulo);
			cc.cdq(rem, dividend);
			cc.idiv(rem, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, arg1, rem);
		}
		break;
		case SUBV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			add_constant(cc, val, -arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case SUBR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);
			cc.sub(val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case SUBV2:
		{
			x86::Gp val = get_z_register(state, cc, arg2);
			x86::Gp result = cc.newInt32();
			cc.mov(result, arg1);
			cc.sub(result, val);
			set_z_register(state, cc, arg2, result);
		}
		break;
		case MULTV:
		{
			x86::Gp val = get_z_register_64(state, cc, arg1);
			cc.imul(val, arg2);
			div_10000(cc, val);
			set_z_register(state, cc, arg1, val.r32());
		}
		break;
		case MULTR:
		{
			x86::Gp val = get_z_register_64(state, cc, arg1);
			x86::Gp val2 = get_z_register_64(state, cc, arg2);
			cc.imul(val, val2);
			div_10000(cc, val);
			set_z_register(state, cc, arg1, val.r32());
		}
		break;
		// TODO guard for div by zero
		case DIVV:
		{
			x86::Gp dividend = get_z_register_64(state, cc, arg1);
			int val2 = arg2;

			cc.imul(dividend, 10000);
			x86::Gp divisor = cc.newInt64();
			cc.mov(divisor, val2);
			x86::Gp dummy = cc.newInt64();
			zero(cc, dummy);
			cc.cqo(dummy, dividend);
			cc.idiv(dummy, dividend, divisor);

			set_z_register(state, cc, arg1, dividend.r32());
		}
		break;
		case DIVR:
		{
			x86::Gp dividend = get_z_register_64(state, cc, arg1);
			x86::Gp divisor = get_z_register_64(state, cc, arg2);

			Label do_division = cc.newLabel();
			Label do_set_register = cc.newLabel();

			cc.test(divisor, divisor);
			cc.jnz(do_division);

			InvokeNode *invokeNode;
			cc.invoke(&invokeNode, log_error_div_0, FuncSignatureT<void>(state.calling_convention));

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
			set_z_register(state, cc, arg1, dividend.r32());
		}
		break;
		case COMPAREV:
		{
			int val = arg2;
			x86::Gp val2 = get_z_register(state, cc, arg1);

			if (script->zasm[pc + 1].command == GOTOCMP || script->zasm[pc + 1].command == SETCMP)
			{
				if (script->zasm[pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		case COMPAREV2:
		{
			int val = arg1;
			x86::Gp val2 = get_z_register(state, cc, arg2);

			if (script->zasm[pc + 1].command == GOTOCMP || script->zasm[pc + 1].command == SETCMP)
			{
				if (script->zasm[pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					cast_bool(cc, val2);
				}
			}

			// This is a little silly. Could instead do `cc.cmp(val2, val)`, but would have to teach
			// compile_compare to invert the conditional instruction it emits.
			x86::Gp val1 = cc.newInt32();
			cc.mov(val1, val);
			cc.cmp(val1, val2);
		}
		break;
		case COMPARER:
		{
			x86::Gp val = get_z_register(state, cc, arg2);
			x86::Gp val2 = get_z_register(state, cc, arg1);

			if (script->zasm[pc + 1].command == GOTOCMP || script->zasm[pc + 1].command == SETCMP)
			{
				if (script->zasm[pc + 1].arg2 & CMP_BOOL)
				{
					cast_bool(cc, val);
					cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		// https://gcc.godbolt.org/z/r9zq67bK1
		case FLOOR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 9);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case CEILING:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 10);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		default:
		{
			Z_error_fatal("[jit ERROR] Unimplemented command: %s", zasm_op_to_string(command, arg1, arg2, instr.arg3, nullptr, nullptr).c_str());
		}
	}
}

static std::optional<JittedFunction> compile_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn)
{
	pc_t start_pc = fn.start_pc;
	pc_t final_pc = fn.final_pc;

	size_t size_no_nops = 0;
	for (int i = start_pc; i <= final_pc; i++)
	{
		if (script->zasm[i].command != NOP)
			size_no_nops += 1;
	}

	// ~170k is the largest function I've seen (from Yuurand, but if optimizer is on that script is ~100k).
	if (size_no_nops > 150000)
	{
		al_trace("[jit] not compiling function because it is too big (name: %s, start: %d, len: %zu)\n", fn.name().c_str(), start_pc, size_no_nops);
		return std::nullopt;
	}

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	bool runtime_debugging = script_debug_is_runtime_debugging() == 2;

	CompilationState state{
		.script = script,
		.j_script = j_script,
		.start_pc = start_pc,
		.final_pc = final_pc,
		.runtime_debugging = runtime_debugging,
	};

	CodeHolder code;
	JittedFunctionImpl compiled_fn;

	static bool jit_env_test = get_flag_bool("-jit-env-test").value_or(false);
	state.calling_convention = CallConvId::kHost;
	if (jit_env_test)
	{
		// This is only for testing purposes, to ensure the same output regardless of
		// the host machine. Used by test_jit.py
		Environment env{};
		env._arch = Arch::kX64;
		env._platform = Platform::kOSX;
		env._platformABI = PlatformABI::kGNU;
		env._objectFormat = ObjectFormat::kJIT;
		code.init(env);
		state.calling_convention = CallConvId::kCDecl;
	}
	else
	{
		code.init(rt.environment());
	}

	MyErrorHandler myErrorHandler;
	code.setErrorHandler(&myErrorHandler);

	StringLogger logger;
	if (DEBUG_JIT_PRINT_ASM)
		code.setLogger(&logger);

	jit_printf("[jit] compile function start (name: %s, start: %d, len: %zu)\n", fn.name().c_str(), start_pc, size_no_nops);

	x86::Compiler cc(&code);

	// Create control flow labels.
	JumpAnnotation* resume_annotation = cc.newJumpAnnotation();
	for (size_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;
		if (command_is_goto(command))
		{
			int pc = script->zasm[i].arg1;
			if (pc >= start_pc && pc <= final_pc)
				state.goto_labels[pc] = cc.newLabel();
		}
		else if (command == CALLFUNC)
		{
			// If the last command in a function, it will never return.
			if (i != final_pc)
				state.resume_labels[i] = cc.newLabel();
		}
		else if (command_is_wait(command))
		{
			state.resume_labels[i] = cc.newLabel();
		}
	}

	for (auto& [pc, label] : state.resume_labels)
		resume_annotation->addLabel(label);

	auto fnNode = cc.addFunc(FuncSignatureT<int, JittedExecutionContext*>(state.calling_convention));
	state.ptrCtx = cc.newIntPtr("ctx");
	fnNode->setArg(0, state.ptrCtx);

	// - r13: JittedExecutionContext*
	// - r14: int32_t* registers
	// - r15: int32_t* stack_base
	// - ebx: int32_t sp
	{
		auto tmp = cc.newUInt64();
		cc.mov(tmp, x86::r13);
		cc.mov(x86::r13, state.ptrCtx);
		state.ptrCtx = x86::r13;

		cc.mov(x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 0), tmp);
		cc.mov(x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 8), x86::r14);
		cc.mov(x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 16), x86::r15);
		cc.mov(x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, saved_regs) + 24), x86::rbx);

		state.ptrRegisters = x86::r14;
		cc.mov(state.ptrRegisters, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, registers)));

		state.ptrStackBase = x86::r15;
		cc.mov(state.ptrStackBase, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, stack_base)));

		state.vSp = x86::ebx;
		cc.mov(state.vSp, x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, sp)));

		// Note: the below code does the same, but uses virtual registers instead. I've found that
		// asmjit's binpack register allocator takes too long to run, so I replaced it with a linear
		// allocator. However, a linear allocator works really bad for registers used throughout the
		// entire function, so it's better to explicitly assign them like above.
		// If the below were used instead, we could get rid of the "saved_regs" too. But I don't
		// think there is much of a performance difference.

		// state.ptrRegisters = cc.newIntPtr("registers");
		// cc.mov(state.ptrRegisters, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, registers)));
		// state.ptrStackBase = cc.newIntPtr("stack_base");
		// cc.mov(state.ptrStackBase, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, stack_base)));
		// state.vSp = cc.newUInt32("sp");
		// cc.mov(state.vSp, x86::ptr_32(state.ptrCtx, offsetof(JittedExecutionContext, sp)));
	}

	state.vResult = cc.newUInt32("result");
	state.L_End = cc.newLabel();

	// If needed, jump to the resume address.
	{
		Label L_normal_entry = cc.newLabel();

		x86::Gp resume_addr_reg = cc.newIntPtr("resume_address");
		cc.mov(resume_addr_reg, x86::ptr(state.ptrCtx, offsetof(JittedExecutionContext, resume_address)));
		cc.test(resume_addr_reg, resume_addr_reg); 
		cc.jz(L_normal_entry); // If it's zero, this is a normal function start.
		cc.jmp(resume_addr_reg, resume_annotation);
		cc.bind(L_normal_entry);
	}

	// cc.setInlineComment does not make a copy of the string, so we need to keep
	// comment strings around a bit longer than the invocation.
	std::string comment;
	std::map<int, int> uncompiled_command_counts;

	static bool use_cached_regs_enabled = is_feature_enabled("-jit-cache-registers", "ZSCRIPT", "jit_cache_registers", true);
	state.use_cached_regs = use_cached_regs_enabled && !runtime_debugging;

	pc_t current_block_id = j_script->cfg.block_id_from_start_pc(start_pc);

	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		const auto& op = script->zasm[i];
		int command = op.command;

		bool is_block_start;
		if (i == start_pc)
		{
			is_block_start = true;
		}
		else
		{
			is_block_start = state.j_script->cfg.contains_block_start(i);
			if (is_block_start)
				current_block_id++;
		}

		if (command_is_wait(command))
		{
			flush_cache(state, cc);
		}
		else if (!command_is_compiled(command))
		{
			flush_cache(state, cc);

			// Note: I tried to be more clever here, only flushing registers
			// that may be used by the compiled commands, but it wasn't working.
			// Below was my attempt.

			// uint8_t reads = 0;
			// uint8_t writes = 0;
			// for (pc_t j = i; j <= final_pc; j++)
			// {
			// 	const auto& instr = script->zasm[j];
			// 	if (!command_is_compiled(instr.command))
			// 	{
			// 		for_every_register_side_effect(instr, [&](bool read, bool write, int reg, int argn){
			// 			if (reg < 8 && read && !((1 << reg) | write))
			// 				reads |= 1 << reg;
			// 			if (reg < 8 && write)
			// 				writes |= 1 << reg;
			// 		});
			// 	}
			// 	else break;
			// }

			// write_some_cached_registers(state, cc, reads);
			// flush_stack_cache(state, cc);
			// std::erase_if(state.cached_d_regs, [&](const auto& pair){
			// 	return writes | (1 << pair.first);
			// });
		}
		else if (command_is_goto(command) || command == CALLFUNC || command == RETURNFUNC)
		{
			if (j_script->cfg.contains_block_start(i + 1))
			{
				uint8_t out = j_script->liveness[current_block_id].out;
				for (auto& [r, cached_reg] : state.cached_d_regs)
				{
					if (!(out & (1 << r)))
						cached_reg.dirty = false;
				}
			}

			flush_cache(state, cc);
		}
		else if (is_block_start)
		{
			pc_t block_id = current_block_id;
			bool is_linear_flow =
				block_id > 0 &&
				j_script->cfg.block_edges[block_id - 1].size() == 1 &&
				j_script->cfg.block_edges[block_id - 1][0] == block_id &&
				j_script->block_predecessors[block_id].size() == 1;
			if (!is_linear_flow)
			{
				if (current_block_id > 0)
				{
					uint8_t out = j_script->liveness[current_block_id - 1].out;
					for (auto& [r, cached_reg] : state.cached_d_regs)
					{
						if (!(out & (1 << r)))
							cached_reg.dirty = false;
					}
				}

				flush_cache(state, cc);
			}
		}

		if (command == COMPAREV || command == COMPARER)
		{
			// Compare commands emit a cmp, but so does the stack check. So flush the stack here so
			// that it doesn't need to be done in the "flush_cache" above, which would interrupt the
			// compare's command cmp result.
			flush_stack_cache(state, cc);
		}

		if (state.goto_labels.contains(i))
		{
			cc.bind(state.goto_labels[i]);
		}

		// Debugging tip: if you're desperate to debug some assembly, uncomment this to stop the program
		// at a specific script instruction (or place it anywhere you like).
		// if (i == ...)
		// {
		// 	// Only the first trap instruction actuallly runs. The debugger (at least for lldb) only
		// 	// shows the dissasembly for the code after the trap, so it can be unclear if the
		// 	// program stopped at the expected spot. The second command makes it clear since lldb
		// 	// will at least then show that as the next instruction.
		// 	cc.int_(3);
		// 	cc.int_(3);
		// }

		if (DEBUG_JIT_PRINT_ASM && j_script->structured_zasm.start_pc_to_function.contains(i))
		{
			cc.setInlineComment((comment = fmt::format("function {}", j_script->structured_zasm.start_pc_to_function[i])).c_str());
			cc.nop();
		}

		if (DEBUG_JIT_PRINT_ASM)
		{
			cc.setInlineComment((comment = fmt::format("{} {}", i, zasm_op_to_string(op))).c_str());
			cc.nop();
		}

#ifdef JIT_DEBUG_CRASH
		if (true)
		{
			x86::Gp reg = cc.newIntPtr();
			cc.mov(reg, (uint64_t)&debug_last_pc);
			cc.mov(x86::ptr_32(reg), i);
		}
#endif

		// Debugging tool used by scripts/jit_runtime_debug.py.
		//
		// We can't invoke functions between COMPARE and the instructions that use the comparison result,
		// because that would destroy EFLAGS. And asmjit compiler has no way to save the EFLAGS because we
		// can't use stack-modifying instructions like pushfq. So we must skip the debug printout for these
		// instructions, which results in them being grouped together in the output and the stack/register
		// trace being printed just once for the entire group of instructions.
		if (runtime_debugging && !command_uses_comparison_result(command))
		{
			x86::Gp sp = get_tmp_sp(state, cc);

			InvokeNode *invokeNode;
			cc.invoke(&invokeNode, debug_pre_command, FuncSignatureT<void, int32_t, uint32_t>(state.calling_convention));
			invokeNode->setArg(0, i);
			invokeNode->setArg(1, sp);
		}

		if (command_uses_comparison_result(command))
		{
			compile_compare(state, cc, i, command, op.arg1, op.arg2, op.arg3);
			continue;
		}

		if (command_is_wait(command))
		{
			// This returns only if actually waiting (some wait commands may be deemed invalid and
			// ignored, so waiting is conditional).
			compile_command_interpreter(state, cc, script, i, 1, true);
			continue;
		}

		if (!command_is_compiled(command))
		{
			if (DEBUG_JIT_PRINT_ASM && command != 0xFFFF)
				uncompiled_command_counts[command]++;

			// Every command that is not compiled to assembly must go through the regular interpreter function.
			// In order to reduce function call overhead, we call into the interpreter function in batches.
			int uncompiled_command_count = 1;
			for (int j = i + 1; j <= final_pc; j++)
			{
				if (command_is_compiled(script->zasm[j].command))
					break;
				if (state.goto_labels.contains(j))
					break;

				if (DEBUG_JIT_PRINT_ASM && script->zasm[j].command != 0xFFFF)
					uncompiled_command_counts[script->zasm[j].command]++;

				uncompiled_command_count += 1;
				if (DEBUG_JIT_PRINT_ASM)
				{
					const auto& op = script->zasm[j];
					std::string op_str = zasm_op_to_string(op);
					cc.setInlineComment((comment = fmt::format("{} {}", j, op_str)).c_str());
					cc.nop();
				}
			}

			compile_command_interpreter(state, cc, script, i, uncompiled_command_count);
			i += uncompiled_command_count - 1;
			continue;
		}

		compile_single_command(state, cc, op, i, script);
	}

	if (DEBUG_JIT_PRINT_ASM)
	{
		cc.setInlineComment("fall-thru");
		cc.nop();
	}

	if (fn.id == j_script->structured_zasm.functions.back().id)
	{
		// The last command for the last function is 0xFFFF. But, it's not included as part of the
		// function bounds (final_pc will be the one just before it–see zasm_construct_structured).
		// Global init scripts rely on this to actually end the script.
		set_ctx_ret_code(state, cc, RUNSCRIPT_STOPPED);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
	}
	else
	{
		cc.mov(state.vResult, EXEC_RESULT_UNKNOWN);
	}

	cc.bind(state.L_End);

	if (state.modified_stack)
		set_ctx_sp(state, cc, state.vSp);

	restore_regs(state, cc);
	cc.ret(state.vResult);

	cc.endFunc();
	cc.finalize();

	Error err = rt.add(&compiled_fn, &code);
	if (err)
	{
		jit_error("[jit ERROR] compile function failed: %s (name: %s, start: %d)\n", asmjit::DebugUtils::errorAsString(err), fn.name().c_str(), start_pc);
		return std::nullopt;
	}
	else if (jit_env_test)
	{
		jit_printf("discarding compiled function because of -jit-env-test\n");
		return std::nullopt;
	}

	uintptr_t base = code.baseAddress();

	std::map<pc_t, uintptr_t> pc_to_resume_address;
	for (const auto& it : state.resume_labels)
	{
		pc_to_resume_address[it.first] = base + code.labelOffsetFromBase(it.second);
	}

	end_time = std::chrono::steady_clock::now();
	int32_t compile_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	jit_printf("[jit] compile function end   (name: %s, start: %d, len: %zu, ms: %d)\n", fn.name().c_str(), start_pc, size_no_nops, compile_ms);

	if (auto debug_handle = j_script->debug_handle.get())
	{
		debug_handle->printf("function:           %s\n", fn.name().c_str());
		debug_handle->printf("start pc:           %d\n", start_pc);
		debug_handle->printf("time to compile:    %d ms\n", compile_ms);
		debug_handle->printf("Code size:          %.1f kb\n", code.codeSize() / 1024.0);
		debug_handle->printf("ZASM instructions:  %zu\n", size_no_nops);
		debug_handle->print("\n");

		if (!uncompiled_command_counts.empty())
		{
			debug_handle->print("=== uncompiled commands:\n");
			for (auto &it : uncompiled_command_counts)
			{
				debug_handle->printf("%s: %d\n", zasm_op_to_string(it.first).c_str(), it.second);
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
		debug_handle->print("\n");

		debug_handle->update_file();
	}

	JittedFunction j_fn{
		.exec = compiled_fn,
		.id = fn.id,
		.pc_to_resume_address = std::move(pc_to_resume_address),
	};
	return j_fn;
}

static bool compile_and_queue_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn)
{
	j_script->functions_requested_to_be_compiled[fn.id] = true;

	if (bisect_tool_should_skip())
		return false;

	auto j_fn = compile_function(script, j_script, fn);
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
	if (fn_id == -1)
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
	CHECK(fn_id != -1);
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

	j_script->liveness = zasm_run_liveness_analysis(script, j_script->cfg);

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
	if (fn_id != -1)
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
		if (j_instance->ri->retsp >= MAX_CALL_FRAMES)
		{
			ctx->ret_code = RUNSCRIPT_JIT_CALL_LIMIT;
			j_instance->ri->pc = ctx->pc;
			j_instance->ri->sp = ctx->sp;
			return false;
		}

		void retstack_push(int32_t val);
		retstack_push(ctx->pc + 1);
		ctx->pc = ctx->call_pc;
		ctx->call_pc = -1;
		return true;
	}
	else if (exec_result == EXEC_RESULT_RETURN)
	{
		std::optional<int32_t> retstack_pop(void);
		ctx->pc = *retstack_pop();
		return true;
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
