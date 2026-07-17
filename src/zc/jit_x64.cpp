#include "zc/jit_x64.h"
#include "zc/jit_native_shared.h"
#include "zc/jit_shared.h"
#include "asmjit/x86/x86operand.h"
#include "base/general.h"
#include "core/qrs.h"
#include "base/util.h"
#include "core/zdefs.h"
#include "components/zasm/defines.h"
#include "components/zasm/pc.h"
#include "components/zasm/table.h"
#include "zc/jit.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zasm_pipeline.h"
#include "zc/zasm_utils.h"
#include "components/zasm/serialize.h"
#include "zconsole/ConsoleLogger.h"
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <chrono>
#include <optional>
#include <asmjit/asmjit.h>

using namespace asmjit;

static JitRuntime rt;

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
	pc_t pc;
	pc_t start_pc;
	pc_t final_pc;
	CallConvId calling_convention;
	Label L_End;
	x86::Gp vResult;
	x86::Gp vSp;
	x86::Gp vSwitchKey;
	// D-register cache; the caching/flush/dead-drop logic is shared with the
	// a64 backend (jit_native_shared.h). A register returned by
	// get_z_register may be a cache entry - treat it as READ-ONLY unless the
	// mutated value is immediately set_z_register'd back to the same D
	// register.
	bool use_cached_regs;
	DRegCache<x86::Gp> dreg_cache;
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
	void handleError(Error, const char *message, BaseEmitter*) override
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

static bool command_is_compiled(int command);

// Adapter giving the shared D-register cache (jit_native_shared.h) its
// primitive emissions. flush_cache also flushes the stack-value cache, which
// only this backend has.
struct CacheOps
{
	using Reg = x86::Gp;

	CompilationState& state;
	x86::Compiler& cc;

	Reg new_reg32() { return cc.newInt32(); }
	void emit_load_d(Reg dst, int r) { cc.mov(dst, x86::ptr_32(state.ptrRegisters, r * 4)); }
	void emit_store_d(Reg src, int r) { cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), src); }
	void emit_mov(Reg dst, Reg src) { cc.mov(dst, src); }
	void emit_mov(Reg dst, int32_t imm) { cc.mov(dst, imm); }
	void annotate(const char* text) { if (DEBUG_JIT_PRINT_ASM) cc.setInlineComment(text); }
	void flush_cache()
	{
		state.dreg_cache.flush(*this);
		flush_stack_cache(state, cc);
	}
	bool is_command_compiled(int command) { return command_is_compiled(command); }
};

static void flush_cache_for_dependent_registers(CompilationState& state, x86::Compiler& cc, int r)
{
	CacheOps ops{state, cc};
	state.dreg_cache.flush_dependents(ops, r);
}

static int32_t get_register_and_restore_sp(int32_t arg, uint32_t sp, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	ri->sp = sp;
	return get_register(arg);
}

static int32_t get_register_jit(int32_t arg, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	return get_register(arg);
}

static void set_register_and_restore_sp(int32_t arg, int32_t value, uint32_t sp, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	ri->sp = sp;
	set_register(arg, value);
}

static void set_register_jit(int32_t arg, int32_t value, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	set_register(arg, value);
}

static void set_guarded_register(int32_t arg, int32_t value, pc_t pc)
{
	extern refInfo *ri;

	ri->pc = pc;
	do_set(arg, value);
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
		CacheOps ops{state, cc};
		return state.dreg_cache.get(ops, r);
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
		cc.invoke(&invokeNode, get_register_and_restore_sp, FuncSignature::build<int32_t, int32_t, uint32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, stackIndex);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
		invokeNode->setRet(0, val);
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external get_register_jit.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register_jit, FuncSignature::build<int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, state.pc); // Needed for accurate stack trace should an error occur.
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
			CacheOps ops{state, cc};
			cc.movsxd(val, state.dreg_cache.get(ops, r));
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
		cc.invoke(&invokeNode, get_register_and_restore_sp, FuncSignature::build<int32_t, int32_t, uint32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, stackIndex);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
		invokeNode->setRet(0, val32);
		cc.movsxd(val, val32);
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external get_register_jit.
		x86::Gp val32 = cc.newInt32();
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register_jit, FuncSignature::build<int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, state.pc); // Needed for accurate stack trace should an error occur.
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
			CacheOps ops{state, cc};
			state.dreg_cache.set(ops, r, val);
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
		cc.invoke(&invokeNode, set_register_and_restore_sp, FuncSignature::build<void, int32_t, int32_t, uint32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
		invokeNode->setArg(2, stackIndex);
		invokeNode->setArg(3, state.pc); // Needed for accurate stack trace should an error occur.
	}
	else if (is_guarded_script_register(r))
	{
		// Some registers have an extra check when writing to them.
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external set_guarded_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_guarded_register, FuncSignature::build<void, int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external set_register_jit.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_register_jit, FuncSignature::build<void, int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
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

// Returns src / 10000 in a FRESH register (division by invariant
// multiplication, https://clang.godbolt.org/z/c4qG3s9nW). Never mutates src:
// it is frequently a cached D register (from get_z_register) or a stack-cache
// entry, and the previous in-place version corrupted the cached value of any
// operand that wasn't written back (e.g. the second operand of ANDR, or a
// LSHIFTR shift count).
static x86::Gp div_10000(x86::Compiler& cc, x86::Gp src)
{
	if (src.isType(RegType::kGp64))
	{
		x86::Gp r = cc.newInt64();
		cc.movabs(r, 3777893186295716171);
		cc.imul(r, r, src);
		cc.sar(r, 11);

		x86::Gp b = cc.newInt64();
		cc.mov(b, src);
		cc.sar(b, 63);
		cc.sub(r, b);
		return r;
	}
	else if (src.isType(RegType::kGp32))
	{
		x86::Gp r = cc.newInt64();
		cc.movsxd(r, src);
		cc.imul(r, r, 1759218605);
		cc.sar(r, 44);

		x86::Gp b = cc.newInt32();
		cc.mov(b, src);
		cc.sar(b, 31);
		cc.sub(r.r32(), b);
		return r.r32();
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

// Computes (base + value) / 10000 into a fresh register for a stack offset.
// `base` may be a cached register (rSFRAME, or a D-register from
// get_z_register) and is never mutated: immutable_add_constant copies (or
// passes through for 0) and div_10000 writes a fresh register.
static x86::Gp compute_stack_offset(x86::Compiler& cc, x86::Gp base, int value)
{
	x86::Gp sum = immutable_add_constant(cc, base, value);
	return div_10000(cc, sum);
}

// Returns 0/1 based on whether reg is nonzero, in a FRESH register - reg is
// often a cached D register that must not be mutated (the previous in-place
// version corrupted the cached values of CMP_BOOL comparison operands).
static x86::Gp immutable_cast_bool(x86::Compiler& cc, x86::Gp reg)
{
	x86::Gp out = cc.newInt32();
	cc.test(reg, reg);
	cc.mov(out, 0);
	cc.setne(out.r8());
	return out;
}

static void compile_compare_goto(CompilationState& state, x86::Compiler& cc, int command, int arg1, int arg2, int arg3)
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

static void compile_compare(CompilationState& state, x86::Compiler& cc, int command, int arg1, int arg2, int arg3)
{
	if (command_is_goto(command))
	{
		compile_compare_goto(state, cc, command, arg1, arg2, arg3);
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
static void compile_command_interpreter(CompilationState& state, x86::Compiler& cc, zasm_script *script, int count, bool is_wait = false)
{
	x86::Gp scriptInstancePtr = get_ctx_script_instance(state, cc);
	x86::Gp stackIndex = get_tmp_sp(state, cc);

	InvokeNode *invokeNode;
	if (count == 1)
	{
		cc.invoke(&invokeNode, run_script_jit_one, FuncSignature::build<int32_t, JittedScriptInstance*, pc_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, state.pc);
		invokeNode->setArg(2, stackIndex);
	}
	else
	{
		cc.invoke(&invokeNode, run_script_jit_sequence, FuncSignature::build<int32_t, JittedScriptInstance*, pc_t, uint32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, state.pc);
		invokeNode->setArg(2, stackIndex);
		invokeNode->setArg(3, count);
	}

	if (is_wait)
	{
		set_ctx_pc(state, cc, state.pc + 1);

		x86::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
		// Only wait if the return value is RUNSCRIPT_STOPPED (this supports conditional waits).
		cc.cmp(retVal, RUNSCRIPT_STOPPED);
		// If actually waiting, the return value will be RUNSCRIPT_OK. set_ctx_ret_code isn't called
		// here because RUNSCRIPT_OK is the default value.
		cc.je(state.L_End);

		cc.bind(state.resume_labels[state.pc]);
		return;
	}

	bool could_return_not_ok = false;
	for (int j = 0; j < count; j++)
	{
		int index = state.pc + j;
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

static bool _cpu_supports_sse41()
{
	const asmjit::CpuInfo& cpu = asmjit::CpuInfo::host();
    return cpu.hasFeature(asmjit::CpuFeatures::X86::kSSE4_1);
}

static bool cpu_supports_sse41 = _cpu_supports_sse41();

static bool command_is_compiled(int command)
{
	// These require SSE4.1.
	if (command == FLOOR || command == CEILING)
		return cpu_supports_sse41;

	return jit_command_is_compiled_shared(command);
}

// Check for stack overflows, but only once per contiguous series of PUSH (or POP) commands.
static void handle_check_sp_push(CompilationState& state, x86::Compiler& cc, const zasm_script* script)
{
	if (state.pc >= state.num_push_commands_in_row_end_pc)
	{
		int stack_delta = 1;
		int max_stack_delta = 1;

		int j = state.pc + 1;
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
		cc.invoke(&invokeNode, log_error_mod_0, FuncSignature::build<void>(state.calling_convention));
		return val;
	}

	if (arg2 == -1)
	{
		// idiv overflows computing INT_MIN / -1, but x % -1 is always 0.
		x86::Gp val = cc.newInt32();
		zero(cc, val);
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
static void compile_single_command(CompilationState& state, x86::Compiler& cc, const ffscript& instr, zasm_script *script)
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
			compile_command_interpreter(state, cc, script, 1);
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
			set_ctx_pc(state, cc, state.pc);
			set_ctx_call_pc(state, cc, arg1);
			if (state.modified_stack)
				set_ctx_sp(state, cc, state.vSp);
			cc.mov(state.vResult, EXEC_RESULT_CALL);
			restore_regs(state, cc);
			cc.ret(state.vResult);
			if (state.pc != state.final_pc)
				cc.bind(state.resume_labels[state.pc]);
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

			handle_check_sp_push(state, cc, script);

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

			handle_check_sp_push(state, cc, script);

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
				if(arg2 < 1) break; //do nothing
				// Must copy the register value, as PUSHR does. If we stored the cached D-register
				// virtual register directly, a later arithmetic instruction (e.g. ADDR modifying the
				// same register in-place) would corrupt the stack cache entry before the flush.
				x86::Gp copy = cc.newInt32();
				cc.mov(copy, get_z_register(state, cc, arg1));
				state.cached_d_reg_stack.push_back({.reg=copy, .amount=arg2});
				break;
			}

			handle_check_sp_push(state, cc, script);

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

			handle_check_sp_push(state, cc, script);

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
			x86::Gp offset = compute_stack_offset(cc, get_z_register(state, cc, rSFRAME), arg2);
			set_z_register(state, cc, arg1, x86::ptr_32(state.ptrStackBase, offset, 2));
		}
		break;
		case LOADI:
		{
			// Set register to a value on the stack (offset is register at arg2).
			x86::Gp offset = compute_stack_offset(cc, get_z_register(state, cc, arg2), 0);
			set_z_register(state, cc, arg1, x86::ptr_32(state.ptrStackBase, offset, 2));
		}
		break;
		case REF_REMOVE:
		{
			x86::Gp sframe = get_z_register(state, cc, rSFRAME);
			x86::Gp offset = immutable_add_constant(cc, sframe, arg1);

			InvokeNode *invokeNode;
			void script_remove_object_ref(int32_t offset);
			cc.invoke(&invokeNode, script_remove_object_ref, FuncSignature::build<void, int32_t>(state.calling_convention));
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
			cc.invoke(&invokeNode, script_store_object, FuncSignature::build<void, uint32_t, uint32_t>(state.calling_convention));
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
			x86::Gp offset = compute_stack_offset(cc, get_z_register(state, cc, rSFRAME), arg2);
			x86::Gp val = get_z_register(state, cc, arg1);
			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), val);
		}
		break;
		case STOREDV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = compute_stack_offset(cc, get_z_register(state, cc, rSFRAME), arg2);
			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), arg1);
		}
		break;
		case STOREI:
		{
			// Write from register to a value on the stack (offset is register at arg2).
			x86::Gp offset = compute_stack_offset(cc, get_z_register(state, cc, arg2), 0);
			x86::Gp val = get_z_register(state, cc, arg1);
			cc.mov(x86::ptr_32(state.ptrStackBase, offset, 2), val);
		}
		break;
		case READPODARRAYR:
		case READPODARRAYV:
		{
			// reg[arg1] = array[rINDEX][index]. Calls a helper for the bounds-checked
			// access; the array pointer (rINDEX) and index are passed as arguments, so
			// the register cache does not need flushing (unlike the interpreter path).
			x86::Gp arrayptr = get_z_register(state, cc, rINDEX);
			x86::Gp index;
			if (command == READPODARRAYR)
			{
				index = div_10000(cc, get_z_register(state, cc, arg2));
			}

			x86::Gp result = cc.newInt32();
			InvokeNode* node;
			cc.invoke(&node, jit_pod_read, FuncSignature::build<int32_t, int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, arrayptr);
			if (command == READPODARRAYR)
				node->setArg(1, index);
			else
				node->setArg(1, arg2 / 10000);
			node->setArg(2, state.pc);
			node->setArg(3, 0); // no_neg: honor the negative-index QR
			node->setRet(0, result);
			set_z_register(state, cc, arg1, result);
		}
		break;
		case WRITEPODARRAYRR:
		case WRITEPODARRAYRV:
		case WRITEPODARRAYVR:
		case WRITEPODARRAYVV:
		{
			// array[rINDEX][index] = value. First letter of the suffix is the index
			// operand kind, second is the value operand kind (R=register, V=immediate).
			bool index_is_reg = command == WRITEPODARRAYRR || command == WRITEPODARRAYRV;
			bool value_is_reg = command == WRITEPODARRAYRR || command == WRITEPODARRAYVR;

			x86::Gp arrayptr = get_z_register(state, cc, rINDEX);
			x86::Gp index;
			if (index_is_reg)
			{
				index = div_10000(cc, get_z_register(state, cc, arg1));
			}
			x86::Gp value;
			if (value_is_reg)
				value = get_z_register(state, cc, arg2);

			InvokeNode* node;
			cc.invoke(&node, jit_pod_write, FuncSignature::build<void, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, arrayptr);
			if (index_is_reg) node->setArg(1, index); else node->setArg(1, arg1 / 10000);
			if (value_is_reg) node->setArg(2, value); else node->setArg(2, arg2);
			node->setArg(3, instr.arg3);
			node->setArg(4, state.pc);
			node->setArg(5, 0); // no_neg: honor the negative-index QR
		}
		break;
		case WRITEPODARRAY:
		{
			// Bulk-initialize array (reg arg1) from the instruction's constant vector.
			x86::Gp id = get_z_register(state, cc, arg1);
			InvokeNode* node;
			cc.invoke(&node, jit_writepodarr, FuncSignature::build<void, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, id);
			node->setArg(1, state.pc);
		}
		break;
		case ALLOCATEMEMV:
		{
			// reg[arg1] = allocate(size=arg2/10000, object_type=arg3).
			x86::Gp result = cc.newInt32();
			InvokeNode* node;
			cc.invoke(&node, jit_allocatemem, FuncSignature::build<int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, arg2 / 10000);
			node->setArg(1, instr.arg3);
			node->setArg(2, state.pc);
			node->setRet(0, result);
			set_z_register(state, cc, arg1, result);
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
			val = immutable_cast_bool(cc, val);
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

			val = div_10000(cc, val);
			cc.and_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ANDR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			val = div_10000(cc, val);
			val2 = div_10000(cc, val2);
			cc.and_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			val = div_10000(cc, val);
			cc.or_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			val = div_10000(cc, val);
			val2 = div_10000(cc, val2);
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
		case XORV:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			val = div_10000(cc, val);
			cc.xor_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORR:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			val = div_10000(cc, val);
			val2 = div_10000(cc, val2);
			cc.xor_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORR32:
		{
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp val2 = get_z_register(state, cc, arg2);

			cc.xor_(val, val2);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORV32:
		{
			x86::Gp val = get_z_register(state, cc, arg1);

			cc.xor_(val, arg2);

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
			cc.invoke(&invokeNode, log_error_mod_0, FuncSignature::build<void>(state.calling_convention));
			cc.jmp(do_set_register);

			cc.bind(do_modulo);
			// idiv overflows computing INT_MIN / -1, but x % -1 is always 0 (rem is already 0).
			cc.cmp(divisor, -1);
			cc.je(do_set_register);
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
		case BITNOT:
		{
			// reg = (~(reg / 10000)) * 10000
			x86::Gp val = get_z_register(state, cc, arg1);
			val = div_10000(cc, val);
			cc.not_(val);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case BITNOT32:
		{
			// reg = ~reg
			x86::Gp val = get_z_register(state, cc, arg1);
			cc.not_(val);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTV:
		case RSHIFTV:
		{
			// reg = ((reg / 10000) <</>> k) * 10000, where k is the shift count.
			// x86 masks the shift count to 5 bits, matching the interpreter (whose
			// `<<`/`>>` also lower to x86 shifts); `>>` on a signed int is arithmetic.
			x86::Gp val = get_z_register(state, cc, arg1);
			val = div_10000(cc, val);
			int k = (arg2 / 10000) & 31;
			if (k)
			{
				if (command == LSHIFTV) cc.shl(val, k);
				else cc.sar(val, k);
			}
			cc.imul(val, val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTV32:
		case RSHIFTV32:
		{
			// reg = reg <</>> k (raw, no fixed-point scaling on the value).
			x86::Gp val = get_z_register(state, cc, arg1);
			int k = (arg2 / 10000) & 31;
			if (k)
			{
				if (command == LSHIFTV32) cc.shl(val, k);
				else cc.sar(val, k);
			}
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTR:
		case RSHIFTR:
		{
			// reg = ((reg / 10000) <</>> (count / 10000)) * 10000, count is a register.
			// x86 only reads the low 5 bits of CL, matching the interpreter (whose
			// `<<`/`>>` also lower to x86 shifts); `>>` on a signed int is arithmetic.
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp count = get_z_register(state, cc, arg2);
			val = div_10000(cc, val);
			count = div_10000(cc, count);
			if (command == LSHIFTR) cc.shl(val, count.r8());
			else cc.sar(val, count.r8());
			cc.imul(val, val, 10000);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTR32:
		case RSHIFTR32:
		{
			// reg = reg <</>> (count / 10000) (raw value, count is a register).
			x86::Gp val = get_z_register(state, cc, arg1);
			x86::Gp count = get_z_register(state, cc, arg2);
			count = div_10000(cc, count);
			if (command == LSHIFTR32) cc.shl(val, count.r8());
			else cc.sar(val, count.r8());
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MULTV:
		{
			x86::Gp val = get_z_register_64(state, cc, arg1);
			cc.imul(val, arg2);
			val = div_10000(cc, val);
			set_z_register(state, cc, arg1, val.r32());
		}
		break;
		case MULTR:
		{
			x86::Gp val = get_z_register_64(state, cc, arg1);
			x86::Gp val2 = get_z_register_64(state, cc, arg2);
			cc.imul(val, val2);
			val = div_10000(cc, val);
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
			cc.invoke(&invokeNode, log_error_div_0, FuncSignature::build<void>(state.calling_convention));

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

			if (script->zasm[state.pc + 1].command == GOTOCMP || script->zasm[state.pc + 1].command == SETCMP)
			{
				if (script->zasm[state.pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					val2 = immutable_cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		case COMPAREV2:
		{
			int val = arg1;
			x86::Gp val2 = get_z_register(state, cc, arg2);

			if (script->zasm[state.pc + 1].command == GOTOCMP || script->zasm[state.pc + 1].command == SETCMP)
			{
				if (script->zasm[state.pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					val2 = immutable_cast_bool(cc, val2);
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

			if (script->zasm[state.pc + 1].command == GOTOCMP || script->zasm[state.pc + 1].command == SETCMP)
			{
				if (script->zasm[state.pc + 1].arg2 & CMP_BOOL)
				{
					val = immutable_cast_bool(cc, val);
					val2 = immutable_cast_bool(cc, val2);
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

// Hooks for the shared emit loop (jit_emit_function_body in
// jit_native_shared.h).
struct LoopOps
{
	CompilationState& state;
	x86::Compiler& cc;
	zasm_script* script;
	JittedScript* j_script;

	void set_pc(pc_t i) { state.pc = i; }
	void cache_flush_policy(pc_t i, pc_t block_id, bool is_block_start)
	{
		CacheOps ops{state, cc};
		jit_reg_cache_flush_policy(ops, state.dreg_cache, j_script, script, i, block_id, is_block_start);
	}
	void loop_extras(int command)
	{
		if (command == COMPAREV || command == COMPARER || command == COMPAREV2)
		{
			// Compare commands emit a cmp, but so does the stack check. So flush the stack here so
			// that it doesn't need to be done in the cache flush policy above, which would interrupt
			// the compare's command cmp result.
			flush_stack_cache(state, cc);
		}

#ifdef JIT_DEBUG_CRASH
		{
			x86::Gp reg = cc.newIntPtr();
			cc.mov(reg, (uint64_t)&debug_last_pc);
			cc.mov(x86::ptr_32(reg), state.pc);
		}
#endif
	}
	bool has_goto_label(pc_t i) { return state.goto_labels.contains(i); }
	void bind_goto_label(pc_t i) { cc.bind(state.goto_labels[i]); }
	void emit_comment_nop(const char* text)
	{
		cc.setInlineComment(text);
		cc.nop();
	}
	void emit_debug_pre_command(pc_t i)
	{
		x86::Gp sp = get_tmp_sp(state, cc);

		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, debug_pre_command, FuncSignature::build<void, int32_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, i);
		invokeNode->setArg(1, sp);
	}
	bool is_command_compiled(int command) { return command_is_compiled(command); }
	void compile_compare(const ffscript& op) { ::compile_compare(state, cc, op.command, op.arg1, op.arg2, op.arg3); }
	void compile_wait() { compile_command_interpreter(state, cc, script, 1, true); }
	void compile_uncompiled_batch(int count) { compile_command_interpreter(state, cc, script, count); }
	void compile_command(const ffscript& op) { compile_single_command(state, cc, op, script); }
};

std::optional<JittedFunction> jit_backend_compile_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn)
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

	auto fnNode = cc.addFunc(FuncSignature::build<int, JittedExecutionContext*>(state.calling_convention));
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

	state.use_cached_regs = jit_is_use_cached_regs_enabled() && !runtime_debugging;

	LoopOps loop_ops{state, cc, script, j_script};
	jit_emit_function_body(loop_ops, script, j_script, start_pc, final_pc, runtime_debugging, comment, uncompiled_command_counts);

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
