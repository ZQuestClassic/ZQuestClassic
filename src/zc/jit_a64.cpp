// AArch64 JIT backend.
//
// Ported from jit_x64.cpp. The driver lives in jit_shared.cpp and the
// codegen policy both backends share (register cache, emit loop, compiled
// command list) in jit_codegen_shared.h; this file provides only the
// instruction selection. Differences from the x64 backend:
//
// - Division by constants uses sdiv: AArch64 sdiv rounds toward zero, exactly
//   matching C semantics and the x64 backend's magic-multiply sequences.
// - No SSE4.1 gate for CEILING/FLOOR: frintp/frintm are baseline.
// - Resume after a call/wait is a pc-comparison dispatch at entry rather than
//   x64's indirect branch (see the comment at the dispatch).
// - Conditional branches in very large functions go through emit_cond_branch,
//   since b.cond only reaches +/-1 MB.
// - The constant 10000 is kept in a function-wide register (vTenK).

#include "zc/jit_a64.h"
#include "zc/jit_codegen_shared.h"
#include "zc/jit_shared.h"
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
#include <asmjit/a64.h>

using namespace asmjit;

static JitRuntime rt;

struct CompilationState
{
	zasm_script* script;
	JittedScript* j_script;
	pc_t pc;
	pc_t start_pc;
	pc_t final_pc;
	CallConvId calling_convention;
	Label L_End;
	a64::Gp vResult;
	a64::Gp vSp;
	a64::Gp vSwitchKey;
	// Holds the constant 10000 for the whole function (read-only). Fixed-point
	// ZASM multiplies/divides by it constantly, and AArch64 has no ALU or mul
	// immediates that fit it, so sharing one register avoids materializing it
	// at every use (and avoids the extra virtual registers that implies).
	a64::Gp vTenK;
	// D-register cache; the caching/flush/dead-drop logic is shared with the
	// x64 backend (jit_codegen_shared.h). A register returned by
	// get_z_register may be a cache entry - treat it as READ-ONLY unless the
	// mutated value is immediately set_z_register'd back to the same D
	// register.
	bool use_cached_regs;
	DRegCache<a64::Gp> dreg_cache;
	a64::Gp ptrCtx;
	a64::Gp ptrRegisters;
	a64::Gp ptrStackBase;
	std::map<int, Label> goto_labels;
	// Labels for both function calls and wait frame commands.
	std::map<int, Label> resume_labels;
	// When to end the "lookahead" for stack pointer bounds checking (and start checking again).
	pc_t num_push_commands_in_row_end_pc;
	bool modified_stack;
	bool runtime_debugging;
	// AArch64 conditional branches only reach +/-1 MB, which very large
	// functions exceed; see emit_cond_branch.
	bool far_branches;
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
	// Set if any error occurred during emission; the emitted code is broken and
	// the function must be discarded (it runs in the interpreter instead, which
	// is always correct - just slower for that one function).
	bool had_error = false;
	// A function whose spill frame outgrows the range a scaled load/store can
	// address fails with InvalidDisplacement. That is an expected capacity
	// limit for very register-heavy functions, not a codegen bug, so it is
	// tracked separately and doesn't trip -jit-fatal-compile-errors.
	bool only_capacity_errors = true;

	void handleError(Error err, const char *message, BaseEmitter*) override
	{
		had_error = true;
		if (err != kErrorInvalidDisplacement)
			only_capacity_errors = false;
		al_trace("[jit ERROR] AsmJit error: %s\n", message);
	}
};

// AArch64 str needs the value in a register; this materializes an immediate.
static a64::Gp imm_to_reg(a64::Compiler& cc, int32_t value)
{
	a64::Gp reg = cc.newInt32();
	cc.mov(reg, value);
	return reg;
}

// AArch64 has no call-to-immediate-address instruction (x64 `call imm64`), so
// the target must be materialized into a register before cc.invoke.
template <typename Fn>
static void invoke(a64::Compiler& cc, InvokeNode** invokeNode, Fn* fn, const FuncSignature& signature)
{
	a64::Gp target = cc.newIntPtr();
	cc.mov(target, (uint64_t)fn);
	cc.invoke(invokeNode, target, signature);
}

template <typename T>
static void set_ctx_pc(CompilationState& state, a64::Compiler& cc, T pc)
{
	if constexpr (std::is_integral_v<T>)
		cc.str(imm_to_reg(cc, pc), a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, pc)));
	else
		cc.str(pc, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, pc)));
}

static void set_ctx_call_pc(CompilationState& state, a64::Compiler& cc, pc_t call_pc)
{
	cc.str(imm_to_reg(cc, call_pc), a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, call_pc)));
}

template <typename T>
static void set_ctx_sp(CompilationState& state, a64::Compiler& cc, T sp)
{
	if constexpr (std::is_integral_v<T>)
		cc.str(imm_to_reg(cc, sp), a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, sp)));
	else
		cc.str(sp, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, sp)));
}

template <typename T>
static void set_ctx_ret_code(CompilationState& state, a64::Compiler& cc, T ret_code)
{
	if constexpr (std::is_integral_v<T>)
		cc.str(imm_to_reg(cc, ret_code), a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, ret_code)));
	else
		cc.str(ret_code, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, ret_code)));
}

// Adds a constant to a register in place. AArch64 add/sub immediates are
// limited to 12 bits, so larger values are materialized first.
static void add_constant(a64::Compiler& cc, a64::Gp reg, int value)
{
	if (value == 0)
		return;

	int abs_value = value < 0 ? -value : value;
	if (abs_value < (1 << 12))
	{
		if (value > 0)
			cc.add(reg, reg, value);
		else
			cc.sub(reg, reg, abs_value);
	}
	else
	{
		a64::Gp tmp = cc.newInt32();
		cc.mov(tmp, value);
		cc.add(reg, reg, tmp);
	}
}

static a64::Gp immutable_add_constant(a64::Compiler& cc, a64::Gp reg, int value)
{
	if (value == 0)
		return reg;

	a64::Gp r = cc.newInt32();
	cc.mov(r, reg);
	add_constant(cc, r, value);
	return r;
}

// Compares a register to a constant, guarding the 12-bit immediate limit.
static void cmp_constant(a64::Compiler& cc, a64::Gp reg, int value)
{
	if (value >= 0 && value < (1 << 12))
	{
		cc.cmp(reg, value);
	}
	else
	{
		a64::Gp tmp = cc.newInt32();
		cc.mov(tmp, value);
		cc.cmp(reg, tmp);
	}
}

// Returns a memory operand for the stack slot `index + offset_words`, where
// index is a uint32 word index into stack_base (equivalent to the x64
// backend's [stack_base + index*4 + offset_words*4] addressing).
static a64::Mem stack_mem(CompilationState& state, a64::Compiler& cc, a64::Gp index, int offset_words = 0)
{
	if (offset_words == 0)
		return a64::ptr(state.ptrStackBase, index, a64::uxtw(2));

	a64::Gp adjusted = immutable_add_constant(cc, index, offset_words);
	return a64::ptr(state.ptrStackBase, adjusted, a64::uxtw(2));
}

static void modify_sp(CompilationState& state, a64::Compiler& cc, a64::Gp vStackIndex, int delta)
{
	add_constant(cc, vStackIndex, delta);
	state.modified_stack = true;
}

static void check_sp(CompilationState& state, a64::Compiler& cc, a64::Gp vStackIndex, int offset = 0)
{
	if (offset == 0)
	{
		cmp_constant(cc, vStackIndex, MAX_STACK_SIZE);
	}
	else
	{
		a64::Gp val = cc.newUInt32();
		cc.mov(val, vStackIndex);
		add_constant(cc, val, offset);
		cmp_constant(cc, val, MAX_STACK_SIZE);
	}

	Label label = cc.newLabel();
	cc.b_lo(label); // unsigned <, same as x64's jb
	set_ctx_ret_code(state, cc, RUNSCRIPT_JIT_STACK_OVERFLOW);
	cc.mov(state.vResult, EXEC_RESULT_EXIT);
	// Route through the single L_End epilog rather than emitting a mid-function
	// ret. asmjit's AArch64 Compiler miscompiles a cc.ret() that is immediately
	// followed by a bound label which is an indirect-branch (JumpAnnotation)
	// target - it emits a branch-to-self instead of the return - so all exits
	// funnel through one ret at L_End.
	cc.b(state.L_End);
	cc.bind(label);
}

static bool command_is_compiled(int command);

// Adapter giving the shared D-register cache (jit_codegen_shared.h) its
// primitive emissions.
struct CacheOps
{
	using Reg = a64::Gp;

	CompilationState& state;
	a64::Compiler& cc;

	Reg new_reg32() { return cc.newInt32(); }
	void emit_load_d(Reg dst, int r) { cc.ldr(dst, a64::ptr(state.ptrRegisters, r * 4)); }
	void emit_store_d(Reg src, int r) { cc.str(src, a64::ptr(state.ptrRegisters, r * 4)); }
	void emit_mov(Reg dst, Reg src) { cc.mov(dst, src); }
	void emit_mov(Reg dst, int32_t imm) { cc.mov(dst, imm); }
	void annotate(const char* text) { if (DEBUG_JIT_PRINT_ASM) cc.setInlineComment(text); }
	void flush_cache() { state.dreg_cache.flush(*this); }
	bool is_command_compiled(int command) { return command_is_compiled(command); }
};

static void flush_cache_for_dependent_registers(CompilationState& state, a64::Compiler& cc, int r)
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
static a64::Gp get_tmp_sp(CompilationState& state, a64::Compiler& cc)
{
	a64::Gp sp = cc.newInt32();
	cc.mov(sp, state.vSp);
	return sp;
}

// Returns src / 10000 in a FRESH register, rounding toward zero (C
// semantics); sdiv matches the x64 backend's magic-multiply sequences
// bit-for-bit for both 32-bit and 64-bit operands. Never mutates src: sdiv is
// 3-operand, so writing to a fresh register is free, and src is frequently a
// cached D register that must not change (the in-place x64 equivalent is a
// recurring source of register-cache corruption).
static a64::Gp div_10000(CompilationState& state, a64::Compiler& cc, a64::Gp src)
{
	if (src.isGpX())
	{
		a64::Gp out = cc.newInt64();
		cc.sdiv(out, src, state.vTenK);
		return out;
	}

	a64::Gp out = cc.newInt32();
	cc.sdiv(out, src, state.vTenK.w());
	return out;
}

static a64::Gp get_z_register(CompilationState& state, a64::Compiler& cc, int r)
{
	if (state.use_cached_regs && r >= D(0) && r < D(INITIAL_D))
	{
		CacheOps ops{state, cc};
		return state.dreg_cache.get(ops, r);
	}

	a64::Gp val = cc.newInt32();
	if (r >= D(0) && r < D(INITIAL_D))
	{
		cc.ldr(val, a64::ptr(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		a64::Gp address = cc.newIntPtr();
		cc.mov(address, (uint64_t)&game->global_d); // Note: this is only OK b/c the `game` global pointer is never reassigned.
		cc.ldr(val, a64::ptr(address, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.mov(val, state.vSp);
		a64::Gp ten_k = state.vTenK.w();
		cc.mul(val, val, ten_k);
	}
	else if (r == SP2)
	{
		cc.mov(val, state.vSp);
	}
	else if (r == SWITCHKEY)
	{
		cc.mov(val, state.vSwitchKey);
	}
	else if (does_register_use_stack(r))
	{
		flush_cache_for_dependent_registers(state, cc, r);
		a64::Gp stackIndex = get_tmp_sp(state, cc);

		// Call external get_register_and_restore_sp.
		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, get_register_and_restore_sp, FuncSignature::build<int32_t, int32_t, uint32_t, pc_t>(state.calling_convention));
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
		invoke(cc, &invokeNode, get_register_jit, FuncSignature::build<int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, state.pc); // Needed for accurate stack trace should an error occur.
		invokeNode->setRet(0, val);
	}
	return val;
}

static a64::Gp get_z_register_64(CompilationState& state, a64::Compiler& cc, int r)
{
	a64::Gp val32 = get_z_register(state, cc, r);
	a64::Gp val = cc.newInt64();
	cc.sxtw(val, val32);
	return val;
}

template <typename T>
static void set_z_register(CompilationState& state, a64::Compiler& cc, int r, T val)
{
	if (state.use_cached_regs && r >= D(0) && r < D(INITIAL_D))
	{
		CacheOps ops{state, cc};
		state.dreg_cache.set(ops, r, val);
		return;
	}

	a64::Gp val_reg;
	if constexpr (std::is_integral_v<T>)
		val_reg = imm_to_reg(cc, val);
	else
		val_reg = val;

	if (r >= D(0) && r < D(INITIAL_D))
	{
		cc.str(val_reg, a64::ptr(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		a64::Gp address = cc.newIntPtr();
		cc.mov(address, (uint64_t)&game->global_d); // Note: this is only OK b/c the `game` global pointer is never reassigned.
		cc.str(val_reg, a64::ptr(address, (r - GD(0)) * 4));
	}
	else if (r == SP || r == SP2)
	{
		// TODO
		Z_error_fatal("Unimplemented: set SP");
	}
	else if (r == SWITCHKEY)
	{
		state.vSwitchKey = cc.newInt32();
		cc.mov(state.vSwitchKey, val_reg);
	}
	else if (does_register_use_stack(r))
	{
		flush_cache_for_dependent_registers(state, cc, r);
		a64::Gp stackIndex = get_tmp_sp(state, cc);

		// Call external set_register_and_restore_sp.
		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, set_register_and_restore_sp, FuncSignature::build<void, int32_t, int32_t, uint32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val_reg);
		invokeNode->setArg(2, stackIndex);
		invokeNode->setArg(3, state.pc); // Needed for accurate stack trace should an error occur.
	}
	else if (is_guarded_script_register(r))
	{
		// Some registers have an extra check when writing to them.
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external set_guarded_register.
		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, set_guarded_register, FuncSignature::build<void, int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val_reg);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
	}
	else
	{
		flush_cache_for_dependent_registers(state, cc, r);

		// Call external set_register_jit.
		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, set_register_jit, FuncSignature::build<void, int32_t, int32_t, pc_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val_reg);
		invokeNode->setArg(2, state.pc); // Needed for accurate stack trace should an error occur.
	}
}

static void set_z_register(CompilationState& state, a64::Compiler& cc, int r, a64::Mem mem)
{
	a64::Gp val = cc.newInt32();
	cc.ldr(val, mem);
	set_z_register(state, cc, r, val);
}

static a64::Gp get_ctx_script_instance(CompilationState& state, a64::Compiler& cc)
{
	a64::Gp ptr = cc.newIntPtr();
	cc.ldr(ptr, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, j_instance)));
	return ptr;
}

// Returns 0/1 based on whether reg is nonzero, in a FRESH register - reg is
// often a cached D register that must not be mutated.
static a64::Gp immutable_cast_bool(a64::Compiler& cc, a64::Gp reg)
{
	a64::Gp out = cc.newInt32();
	cc.cmp(reg, 0);
	cc.cset(out, a64::CondCode::kNE);
	return out;
}

// Emits a conditional branch that can reach any label in the function. A bare
// b.cond only reaches +/-1 MB of code, which the largest functions exceed
// (the failure shows up as InvalidDisplacement when the label is bound), so
// far branches go over an unconditional b, which reaches +/-128 MB. Only used
// for branches whose target can be arbitrarily far away (script GOTOs, the
// resume dispatch, waits); short local branches stay bare.
//
// pc_distance is how many ZASM instructions sit between the branch and its
// target; the far form is only needed when that distance could plausibly
// exceed b.cond's reach. Emitted code averages well under 100 bytes per ZASM
// instruction (including register-allocator spill code), so 4000 instructions
// leaves a 250 bytes/instruction margin against the +/-1 MB limit.
//
// Distance-based selection is only sound because third_party/asmjit.patch
// places register-state fixup trampolines inline (right after the branch)
// on AArch64. With upstream asmjit's end-of-function trampoline placement,
// a b.cond retargeted to a fixup trampoline goes out of range in a multi-MB
// function no matter how near its original target was - that variant was
// measured to compile fewer functions (16 discarded vs 11 on Yuurand).
static void emit_cond_branch(CompilationState& state, a64::Compiler& cc, a64::CondCode cond, const Label& target, pc_t pc_distance)
{
	if (!state.far_branches || pc_distance <= 4000)
	{
		cc.b(cond, target);
		return;
	}

	Label skip = cc.newLabel();
	cc.b(a64::negateCond(cond), skip);
	cc.b(target);
	cc.bind(skip);
}

// pc distance from the current command to a GOTO target.
static pc_t goto_distance(CompilationState& state, int target_pc)
{
	return (pc_t)(target_pc > (int)state.pc ? target_pc - (int)state.pc : (int)state.pc - target_pc);
}

static void compile_compare_goto(CompilationState& state, a64::Compiler& cc, int command, int arg1, int arg2, int arg3)
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
				emit_cond_branch(state, cc, a64::CondCode::kGT, lbl, goto_distance(state, arg1));
				break;
			case CMP_GT|CMP_EQ:
				emit_cond_branch(state, cc, a64::CondCode::kGE, lbl, goto_distance(state, arg1));
				break;
			case CMP_LT:
				emit_cond_branch(state, cc, a64::CondCode::kLT, lbl, goto_distance(state, arg1));
				break;
			case CMP_LT|CMP_EQ:
				emit_cond_branch(state, cc, a64::CondCode::kLE, lbl, goto_distance(state, arg1));
				break;
			case CMP_EQ:
				emit_cond_branch(state, cc, a64::CondCode::kEQ, lbl, goto_distance(state, arg1));
				break;
			case CMP_GT|CMP_LT:
				emit_cond_branch(state, cc, a64::CondCode::kNE, lbl, goto_distance(state, arg1));
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				cc.b(lbl);
				break;
		}
	}
	else if (command == GOTOTRUE)
	{
		emit_cond_branch(state, cc, a64::CondCode::kEQ, goto_labels[arg1], goto_distance(state, arg1));
	}
	else if (command == GOTOFALSE)
	{
		emit_cond_branch(state, cc, a64::CondCode::kNE, goto_labels[arg1], goto_distance(state, arg1));
	}
	else if (command == GOTOMORE)
	{
		emit_cond_branch(state, cc, a64::CondCode::kGE, goto_labels[arg1], goto_distance(state, arg1));
	}
	else if (command == GOTOLESS)
	{
		if (get_qr(qr_GOTOLESSNOTEQUAL))
			emit_cond_branch(state, cc, a64::CondCode::kLE, goto_labels[arg1], goto_distance(state, arg1));
		else
			emit_cond_branch(state, cc, a64::CondCode::kLT, goto_labels[arg1], goto_distance(state, arg1));
	}
	else
	{
		Z_error_fatal("Unimplemented: %s", zasm_op_to_string(command, arg1, arg2, arg3, nullptr, nullptr).c_str());
	}
}

// Emits `dest = cond ? value : dest` using the current flags (cmov equivalent).
static void csel_constant(a64::Compiler& cc, a64::Gp dest, a64::Gp value, a64::CondCode cond)
{
	cc.csel(dest, value, dest, cond);
}

static void compile_compare(CompilationState& state, a64::Compiler& cc, int command, int arg1, int arg2, int arg3)
{
	if (command_is_goto(command))
	{
		compile_compare_goto(state, cc, command, arg1, arg2, arg3);
		return;
	}

	a64::Gp val = cc.newInt32();

	if (command == SETCMP)
	{
		bool i10k = (arg2 & CMP_SETI);
		a64::Gp val2;
		cc.mov(val, 0);
		if(i10k)
		{
			val2 = state.vTenK.w();
		}
		switch(arg2 & CMP_FLAGS)
		{
			default:
				break;
			case CMP_GT:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kGT);
				else cc.cset(val, a64::CondCode::kGT);
				break;
			case CMP_GT|CMP_EQ:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kGE);
				else cc.cset(val, a64::CondCode::kGE);
				break;
			case CMP_LT:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kLT);
				else cc.cset(val, a64::CondCode::kLT);
				break;
			case CMP_LT|CMP_EQ:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kLE);
				else cc.cset(val, a64::CondCode::kLE);
				break;
			case CMP_EQ:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kEQ);
				else cc.cset(val, a64::CondCode::kEQ);
				break;
			case CMP_GT|CMP_LT:
				if(i10k)
					csel_constant(cc, val, val2, a64::CondCode::kNE);
				else cc.cset(val, a64::CondCode::kNE);
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
		cc.cset(val, a64::CondCode::kEQ);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETTRUEI)
	{
		cc.mov(val, 0);
		a64::Gp val2 = state.vTenK.w();
		csel_constant(cc, val, val2, a64::CondCode::kEQ);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETFALSE)
	{
		cc.mov(val, 0);
		cc.cset(val, a64::CondCode::kNE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETFALSEI)
	{
		cc.mov(val, 0);
		a64::Gp val2 = state.vTenK.w();
		csel_constant(cc, val, val2, a64::CondCode::kNE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETMOREI)
	{
		cc.mov(val, 0);
		a64::Gp val2 = state.vTenK.w();
		csel_constant(cc, val, val2, a64::CondCode::kGE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETLESSI)
	{
		cc.mov(val, 0);
		a64::Gp val2 = state.vTenK.w();
		csel_constant(cc, val, val2, a64::CondCode::kLE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETMORE)
	{
		cc.mov(val, 0);
		cc.cset(val, a64::CondCode::kGE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == SETLESS)
	{
		cc.mov(val, 0);
		cc.cset(val, a64::CondCode::kLE);
		set_z_register(state, cc, arg1, val);
	}
	else if (command == STACKWRITEATVV_IF)
	{
		// TODO: needs to check for stack overflow.
		// Write directly value on the stack (arg1 to offset arg2)
		auto cmp = arg3 & CMP_FLAGS;
		switch(cmp) //but only conditionally
		{
			case 0:
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
			{
				a64::Gp value = imm_to_reg(cc, arg1);
				cc.str(value, stack_mem(state, cc, state.vSp, arg2));
				break;
			}
			default:
			{
				// Note: the loads/stores here must not disturb the comparison
				// flags; ldr/str/mov do not affect NZCV.
				a64::Mem mem = stack_mem(state, cc, state.vSp, arg2);
				a64::Gp tmp = cc.newInt32();
				a64::Gp value = cc.newInt32();
				cc.ldr(tmp, mem);
				cc.mov(value, arg1);
				switch(cmp)
				{
					case CMP_GT:
						csel_constant(cc, tmp, value, a64::CondCode::kGT);
						break;
					case CMP_GT|CMP_EQ:
						csel_constant(cc, tmp, value, a64::CondCode::kGE);
						break;
					case CMP_LT:
						csel_constant(cc, tmp, value, a64::CondCode::kLT);
						break;
					case CMP_LT|CMP_EQ:
						csel_constant(cc, tmp, value, a64::CondCode::kLE);
						break;
					case CMP_EQ:
						csel_constant(cc, tmp, value, a64::CondCode::kEQ);
						break;
					case CMP_GT|CMP_LT:
						csel_constant(cc, tmp, value, a64::CondCode::kNE);
						break;
					default:
						assert(false);
				}
				cc.str(tmp, mem);
			}
		}
	}
	else
	{
		Z_error_fatal("[jit ERROR] Unimplemented command: %s", zasm_op_to_string(command, arg1, arg2, arg3, nullptr, nullptr).c_str());
	}
}

static void ret_if_not_ok(CompilationState& state, a64::Compiler& cc, a64::Gp reg)
{
	Label L_noret = cc.newLabel();
	cmp_constant(cc, reg, RUNSCRIPT_OK);
	cc.b_eq(L_noret);

	set_ctx_ret_code(state, cc, reg);
	cc.b(state.L_End);

	cc.bind(L_noret);
}

// Defer to the ZASM command interpreter for 1+ commands.
static void compile_command_interpreter(CompilationState& state, a64::Compiler& cc, zasm_script *script, int count, bool is_wait = false)
{
	a64::Gp scriptInstancePtr = get_ctx_script_instance(state, cc);
	a64::Gp stackIndex = get_tmp_sp(state, cc);

	InvokeNode *invokeNode;
	if (count == 1)
	{
		invoke(cc, &invokeNode, run_script_jit_one, FuncSignature::build<int32_t, JittedScriptInstance*, pc_t, uint32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, state.pc);
		invokeNode->setArg(2, stackIndex);
	}
	else
	{
		invoke(cc, &invokeNode, run_script_jit_sequence, FuncSignature::build<int32_t, JittedScriptInstance*, pc_t, uint32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, scriptInstancePtr);
		invokeNode->setArg(1, state.pc);
		invokeNode->setArg(2, stackIndex);
		invokeNode->setArg(3, count);
	}

	if (is_wait)
	{
		set_ctx_pc(state, cc, state.pc + 1);

		a64::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
		// Only wait if the return value is RUNSCRIPT_STOPPED (this supports conditional waits).
		cmp_constant(cc, retVal, RUNSCRIPT_STOPPED);
		// If actually waiting, the return value will be RUNSCRIPT_OK. set_ctx_ret_code isn't called
		// here because RUNSCRIPT_OK is the default value.
		emit_cond_branch(state, cc, a64::CondCode::kEQ, state.L_End, state.final_pc - state.pc);

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
		a64::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.mov(state.vResult, EXEC_RESULT_EXIT);
		ret_if_not_ok(state, cc, retVal);
	}
}

static bool command_is_compiled(int command)
{
	// Unlike the x64 backend, no CPU feature gate needed: frintp/frintm are
	// baseline AArch64.
	if (command == FLOOR || command == CEILING)
		return true;

	return jit_command_is_compiled_shared(command);
}

// Check for stack overflows, but only once per contiguous series of PUSH (or POP) commands.
static void handle_check_sp_push(CompilationState& state, a64::Compiler& cc, const zasm_script* script)
{
	if (state.pc >= state.num_push_commands_in_row_end_pc)
	{
		auto scan = jit_scan_push_run(script, state.pc);
		check_sp(state, cc, state.vSp, -scan.max_stack_delta);
		state.num_push_commands_in_row_end_pc = scan.end_pc;
	}
}

// Pushes val onto the stack [amount] times; vSp has already been decremented,
// so slots [vSp + offset - amount + 1, vSp + offset] are written.
static void do_stack_push_many(CompilationState& state, a64::Compiler& cc, int offset, int amount, a64::Gp val)
{
	if (amount < 8)
	{
		for (int i = 0; i < amount; i++)
			cc.str(val, stack_mem(state, cc, state.vSp, offset - i));
	}
	else
	{
		// Loop, storing ascending from the lowest slot two words at a time
		// with a post-indexed store-pair (there is no bulk store like x64's
		// rep stos). The address must be a fresh register: the loop mutates
		// it in place via the write-back.
		a64::Gp index = cc.newUInt32();
		cc.mov(index, state.vSp);
		add_constant(cc, index, offset - amount + 1);
		// Writes to a w register zero the upper bits, so the x view of index
		// is the zero-extended word index.
		a64::Gp addr = cc.newIntPtr();
		cc.lsl(addr, index.x(), 2);
		cc.add(addr, addr, state.ptrStackBase);
		a64::Gp count = imm_to_reg(cc, amount / 2);
		Label L_loop = cc.newLabel();
		cc.bind(L_loop);
		cc.stp(val, val, a64::ptr_post(addr, 8));
		cc.sub(count, count, 1);
		cc.cbnz(count, L_loop);
		if (amount & 1)
			cc.str(val, a64::ptr(addr));
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

static a64::Gp compile_modv(CompilationState& state, a64::Compiler& cc, a64::Gp arg1, int arg2)
{
	if (arg2 == 0)
	{
		a64::Gp val = imm_to_reg(cc, 0);

		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, log_error_mod_0, FuncSignature::build<void>(state.calling_convention));
		return val;
	}

	if (arg2 == -1)
	{
		// x % -1 is always 0.
		return imm_to_reg(cc, 0);
	}

	if (arg2 == 1)
	{
		// x % 1 masks to 0 (see power-of-2 case below); a zero mask is not an
		// encodable AArch64 logical immediate, so produce the 0 directly.
		return imm_to_reg(cc, 0);
	}

	if (arg2 > 0 && (arg2 & (-arg2)) == arg2)
	{
		// Power of 2. Same masking behavior as the x64 backend (which differs
		// from C % for negative dividends; kept for parity - fixed-point
		// values mean this is essentially never hit).
		cc.and_(arg1, arg1, arg2 - 1);
		return arg1;
	}
	else
	{
		// rem = arg1 - (arg1 / divisor) * divisor. sdiv rounds toward zero,
		// so this matches C (and x86 idiv) remainder semantics.
		a64::Gp divisor = imm_to_reg(cc, arg2);
		a64::Gp quot = cc.newInt32();
		a64::Gp rem = cc.newInt32();
		cc.sdiv(quot, arg1, divisor);
		cc.msub(rem, quot, divisor, arg1);
		return rem;
	}
}

// Computes (base + value) / 10000 into a fresh register for a stack offset.
// `base` may be shared (e.g. a cached register from get_z_register) and is
// never mutated: immutable_add_constant copies (or passes through for 0) and
// div_10000 writes a fresh register.
static a64::Gp compute_stack_offset(CompilationState& state, a64::Compiler& cc, a64::Gp base, int value)
{
	a64::Gp sum = immutable_add_constant(cc, base, value);
	return div_10000(state, cc, sum);
}

// Every command here must be reflected in command_is_compiled!
static void compile_single_command(CompilationState& state, a64::Compiler& cc, const ffscript& instr, zasm_script *script)
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
			cc.b(state.L_End);
		}
		break;
		case GOTO:
		{
			if (arg1 >= (int)state.start_pc && arg1 <= (int)state.final_pc)
			{
				cc.b(state.goto_labels[arg1]);
			}
			else
			{
				// Mostly all GOTOs should be within the same function. The only exception is the
				// end of the compiler-generated Init script (the part that sets up globals). When
				// that section ends, it unconditionally jumps to the user-defined init script.
				set_ctx_pc(state, cc, arg1);
				cc.mov(state.vResult, EXEC_RESULT_CONTINUE);
				cc.b(state.L_End);
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
			cc.b(state.L_End);
			if (state.pc != state.final_pc)
				cc.bind(state.resume_labels[state.pc]);
		}
		break;
		case RETURNFUNC:
		{
			if (state.modified_stack)
				set_ctx_sp(state, cc, state.vSp);
			cc.mov(state.vResult, EXEC_RESULT_RETURN);
			cc.b(state.L_End);
		}
		break;
		case PUSHV:
		{
			handle_check_sp_push(state, cc, script);

			modify_sp(state, cc, state.vSp, -1);
			a64::Gp val = imm_to_reg(cc, arg1);
			cc.str(val, stack_mem(state, cc, state.vSp));
		}
		break;
		case PUSHR:
		{
			handle_check_sp_push(state, cc, script);

			// Grab value from register and push onto stack.
			a64::Gp val = get_z_register(state, cc, arg1);
			modify_sp(state, cc, state.vSp, -1);
			cc.str(val, stack_mem(state, cc, state.vSp));
		}
		break;
		case PUSHARGSR:
		{
			handle_check_sp_push(state, cc, script);

			if(arg2 < 1) break; //do nothing

			a64::Gp val = get_z_register(state, cc, arg1);
			modify_sp(state, cc, state.vSp, -arg2);
			do_stack_push_many(state, cc, arg2 - 1, arg2, val);
		}
		break;
		case PUSHARGSV:
		{
			handle_check_sp_push(state, cc, script);

			if(arg2 < 1) break; //do nothing

			a64::Gp val = imm_to_reg(cc, arg1);
			modify_sp(state, cc, state.vSp, -arg2);
			do_stack_push_many(state, cc, arg2 - 1, arg2, val);
		}
		break;
		case POP:
		{
			a64::Gp val = cc.newInt32();
			cc.ldr(val, stack_mem(state, cc, state.vSp));
			modify_sp(state, cc, state.vSp, 1);
			set_z_register(state, cc, arg1, val);
		}
		break;
		// Note: I'm pretty sure this always POPs to D5, which is the null register and can be
		// ignored.
		case POPARGS:
		{
			// int32_t num = sarg2;
			// ri->sp += num;
			modify_sp(state, cc, state.vSp, arg2);

			// word read = ri->sp - 1;
			a64::Gp read = cc.newUInt32();
			cc.mov(read, state.vSp);
			cc.sub(read, read, 1);

			check_sp(state, cc, read);

			// int32_t value = SH::read_stack(read);
			// set_register(sarg1, value);
			if (arg1 != D(5) || state.runtime_debugging) // Skip setting the "null" register (unless runtime debugging).
			{
				a64::Gp val = cc.newInt32();
				cc.ldr(val, stack_mem(state, cc, read));
				set_z_register(state, cc, arg1, val);
			}
		}
		break;
		case STACKWRITEATVV:
		{
			// TODO: needs to check for stack overflow.
			// Write directly value on the stack (arg1 to offset arg2)
			a64::Gp value = imm_to_reg(cc, arg1);
			cc.str(value, stack_mem(state, cc, state.vSp, arg2));
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
			a64::Gp val = get_z_register(state, cc, arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LOAD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp sframe = get_z_register(state, cc, rSFRAME);
			set_z_register(state, cc, arg1, stack_mem(state, cc, sframe, arg2));
		}
		break;
		case LOADD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp offset = compute_stack_offset(state, cc, get_z_register(state, cc, rSFRAME), arg2);
			set_z_register(state, cc, arg1, stack_mem(state, cc, offset));
		}
		break;
		case LOADI:
		{
			// Set register to a value on the stack (offset is register at arg2).
			a64::Gp offset = compute_stack_offset(state, cc, get_z_register(state, cc, arg2), 0);
			set_z_register(state, cc, arg1, stack_mem(state, cc, offset));
		}
		break;
		case REF_REMOVE:
		{
			a64::Gp sframe = get_z_register(state, cc, rSFRAME);
			a64::Gp offset = immutable_add_constant(cc, sframe, arg1);

			InvokeNode *invokeNode;
			void script_remove_object_ref(int32_t offset);
			invoke(cc, &invokeNode, script_remove_object_ref, FuncSignature::build<void, int32_t>(state.calling_convention));
			invokeNode->setArg(0, offset);
		}
		break;
		case STORE:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp sframe = get_z_register(state, cc, rSFRAME);
			a64::Gp val = get_z_register(state, cc, arg1);
			cc.str(val, stack_mem(state, cc, sframe, arg2));
		}
		break;
		case STORE_OBJECT:
		{
			// Same as STORE, but for a ref-counted object.
			a64::Gp sframe = get_z_register(state, cc, rSFRAME);
			a64::Gp offset = immutable_add_constant(cc, sframe, arg2);

			a64::Gp val = get_z_register(state, cc, arg1);

			InvokeNode *invokeNode;
			void script_store_object(uint32_t offset, uint32_t new_id);
			invoke(cc, &invokeNode, script_store_object, FuncSignature::build<void, uint32_t, uint32_t>(state.calling_convention));
			invokeNode->setArg(0, offset);
			invokeNode->setArg(1, val);
		}
		break;
		case STOREV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp sframe = get_z_register(state, cc, rSFRAME);
			a64::Gp val = imm_to_reg(cc, arg1);
			cc.str(val, stack_mem(state, cc, sframe, arg2));
		}
		break;
		case STORED:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp offset = compute_stack_offset(state, cc, get_z_register(state, cc, rSFRAME), arg2);
			a64::Gp val = get_z_register(state, cc, arg1);
			cc.str(val, stack_mem(state, cc, offset));
		}
		break;
		case STOREDV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			a64::Gp offset = compute_stack_offset(state, cc, get_z_register(state, cc, rSFRAME), arg2);
			a64::Gp val = imm_to_reg(cc, arg1);
			cc.str(val, stack_mem(state, cc, offset));
		}
		break;
		case STOREI:
		{
			// Write from register to a value on the stack (offset is register at arg2).
			a64::Gp offset = compute_stack_offset(state, cc, get_z_register(state, cc, arg2), 0);
			a64::Gp val = get_z_register(state, cc, arg1);
			cc.str(val, stack_mem(state, cc, offset));
		}
		break;
		case PEEK:
		{
			a64::Gp val = cc.newInt32();
			cc.ldr(val, stack_mem(state, cc, state.vSp));
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ABS:
		{
			// Wraps for INT_MIN, same as the x64 backend's sar/xor/sub sequence.
			a64::Gp val = get_z_register(state, cc, arg1);
			cc.cmp(val, 0);
			cc.cneg(val, val, a64::CondCode::kLT);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case CASTBOOLI:
		{
			// val = val ? 10000 : 0
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp ten_k = state.vTenK.w();
			cc.cmp(val, 0);
			cc.csel(val, ten_k, a64::wzr, a64::CondCode::kNE);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case CASTBOOLF:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			val = immutable_cast_bool(cc, val);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ADDV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			add_constant(cc, val, arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ADDR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.add(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MAXR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.cmp(val2, val);
			cc.csel(val, val2, val, a64::CondCode::kGE);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MAXV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2);
			cc.cmp(val2, val);
			cc.csel(val, val2, val, a64::CondCode::kGE);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MINR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.cmp(val, val2);
			cc.csel(val, val2, val, a64::CondCode::kGE);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MINV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2);
			cc.cmp(val, val2);
			cc.csel(val, val2, val, a64::CondCode::kGE);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case SUBV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			add_constant(cc, val, -arg2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case SUBR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.sub(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case SUBV2:
		{
			a64::Gp val = get_z_register(state, cc, arg2);
			a64::Gp result = imm_to_reg(cc, arg1);
			cc.sub(result, result, val);
			set_z_register(state, cc, arg2, result);
		}
		break;
		case ANDV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2 / 10000);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			cc.and_(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ANDR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			val2 = div_10000(state, cc, val2);
			cc.and_(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2 / 10000);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			cc.orr(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			val2 = div_10000(state, cc, val2);
			cc.orr(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORR32:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.orr(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case ORV32:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2);
			cc.orr(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORV:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2 / 10000);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			cc.eor(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORR:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			a64::Gp ten_k = state.vTenK.w();

			val = div_10000(state, cc, val);
			val2 = div_10000(state, cc, val2);
			cc.eor(val, val, val2);
			cc.mul(val, val, ten_k);

			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORR32:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = get_z_register(state, cc, arg2);
			cc.eor(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case XORV32:
		{
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp val2 = imm_to_reg(cc, arg2);
			cc.eor(val, val, val2);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case BITNOT:
		{
			// reg = (~(reg / 10000)) * 10000
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp ten_k = state.vTenK.w();
			val = div_10000(state, cc, val);
			cc.mvn(val, val);
			cc.mul(val, val, ten_k);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case BITNOT32:
		{
			// reg = ~reg
			a64::Gp val = get_z_register(state, cc, arg1);
			cc.mvn(val, val);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTV:
		case RSHIFTV:
		{
			// reg = ((reg / 10000) <</>> k) * 10000, where k is the shift count.
			// The interpreter's `<<`/`>>` compile to shifts that mask the count
			// to 5 bits on both x86 and AArch64 (for 32-bit operands), so the
			// mask here matches; `>>` on a signed int is arithmetic.
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp ten_k = state.vTenK.w();
			val = div_10000(state, cc, val);
			int k = (arg2 / 10000) & 31;
			if (k)
			{
				if (command == LSHIFTV) cc.lsl(val, val, k);
				else cc.asr(val, val, k);
			}
			cc.mul(val, val, ten_k);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTV32:
		case RSHIFTV32:
		{
			// reg = reg <</>> k (raw, no fixed-point scaling on the value).
			a64::Gp val = get_z_register(state, cc, arg1);
			int k = (arg2 / 10000) & 31;
			if (k)
			{
				if (command == LSHIFTV32) cc.lsl(val, val, k);
				else cc.asr(val, val, k);
			}
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTR:
		case RSHIFTR:
		{
			// reg = ((reg / 10000) <</>> (count / 10000)) * 10000, count is a register.
			// AArch64 register shifts on 32-bit operands use count mod 32, same
			// as x86's 5-bit CL mask, so behavior matches the x64 backend.
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp count = get_z_register(state, cc, arg2);
			a64::Gp ten_k = state.vTenK.w();
			val = div_10000(state, cc, val);
			count = div_10000(state, cc, count);
			if (command == LSHIFTR) cc.lsl(val, val, count);
			else cc.asr(val, val, count);
			cc.mul(val, val, ten_k);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case LSHIFTR32:
		case RSHIFTR32:
		{
			// reg = reg <</>> (count / 10000) (raw value, count is a register).
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp count = get_z_register(state, cc, arg2);
			count = div_10000(state, cc, count);
			if (command == LSHIFTR32) cc.lsl(val, val, count);
			else cc.asr(val, val, count);
			set_z_register(state, cc, arg1, val);
		}
		break;
		case MODV:
		{
			a64::Gp result = compile_modv(state, cc, get_z_register(state, cc, arg1), arg2);
			set_z_register(state, cc, arg1, result);
		}
		break;
		case MODR:
		{
			a64::Gp dividend = get_z_register(state, cc, arg1);
			a64::Gp divisor = get_z_register(state, cc, arg2);

			Label do_modulo = cc.newLabel();
			Label do_set_register = cc.newLabel();

			a64::Gp rem = imm_to_reg(cc, 0);

			cc.cbnz(divisor, do_modulo);

			InvokeNode *invokeNode;
			invoke(cc, &invokeNode, log_error_mod_0, FuncSignature::build<void>(state.calling_convention));
			cc.b(do_set_register);

			cc.bind(do_modulo);
			// x % -1 is always 0 (rem is already 0). On AArch64 sdiv+msub would
			// compute 0 for INT_MIN % -1 anyway, but branch like the x64
			// backend for identical structure. cmp_constant handles -1, which
			// is not a valid bare cmp immediate on AArch64.
			cmp_constant(cc, divisor, -1);
			cc.b_eq(do_set_register);
			a64::Gp quot = cc.newInt32();
			cc.sdiv(quot, dividend, divisor);
			cc.msub(rem, quot, divisor, dividend);

			cc.bind(do_set_register);
			set_z_register(state, cc, arg1, rem);
		}
		break;
		case MULTV:
		{
			a64::Gp val = get_z_register_64(state, cc, arg1);
			a64::Gp val2 = cc.newInt64();
			cc.mov(val2, arg2);
			cc.mul(val, val, val2);
			val = div_10000(state, cc, val);
			set_z_register(state, cc, arg1, val.w());
		}
		break;
		case MULTR:
		{
			a64::Gp val = get_z_register_64(state, cc, arg1);
			a64::Gp val2 = get_z_register_64(state, cc, arg2);
			cc.mul(val, val, val2);
			val = div_10000(state, cc, val);
			set_z_register(state, cc, arg1, val.w());
		}
		break;
		case DIVV:
		{
			// Note: like the x64 backend, no zero guard for a constant divisor
			// (the compiler never emits DIVV 0).
			a64::Gp dividend = get_z_register_64(state, cc, arg1);
			cc.mul(dividend, dividend, state.vTenK);
			a64::Gp divisor = cc.newInt64();
			cc.mov(divisor, arg2);
			cc.sdiv(dividend, dividend, divisor);
			set_z_register(state, cc, arg1, dividend.w());
		}
		break;
		case DIVR:
		{
			a64::Gp dividend = get_z_register_64(state, cc, arg1);
			a64::Gp divisor = get_z_register_64(state, cc, arg2);

			Label do_division = cc.newLabel();
			Label do_set_register = cc.newLabel();

			cc.cbnz(divisor, do_division);

			// Division by zero: log an error and produce sign(dividend) * MAX_SIGNED_32,
			// exactly like the x64 backend and the interpreter.
			InvokeNode *invokeNode;
			invoke(cc, &invokeNode, log_error_div_0, FuncSignature::build<void>(state.calling_convention));

			a64::Gp sign = cc.newInt64();
			cc.asr(sign, dividend, 63);
			a64::Gp one = cc.newInt64();
			cc.mov(one, 1);
			cc.orr(sign, sign, one);
			a64::Gp max_signed = imm_to_reg(cc, MAX_SIGNED_32);
			cc.mul(dividend.w(), sign.w(), max_signed);
			cc.b(do_set_register);

			// Else do the actual division.
			cc.bind(do_division);
			cc.mul(dividend, dividend, state.vTenK);
			cc.sdiv(dividend, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, arg1, dividend.w());
		}
		break;
		case READPODARRAYR:
		case READPODARRAYV:
		{
			// reg[arg1] = array[rINDEX][index]. Calls a helper for the bounds-checked
			// access; the array pointer (rINDEX) and index are passed as arguments.
			a64::Gp arrayptr = get_z_register(state, cc, rINDEX);
			a64::Gp index;
			if (command == READPODARRAYR)
			{
				index = div_10000(state, cc, get_z_register(state, cc, arg2));
			}

			a64::Gp result = cc.newInt32();
			InvokeNode* node;
			invoke(cc, &node, jit_pod_read, FuncSignature::build<int32_t, int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
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

			a64::Gp arrayptr = get_z_register(state, cc, rINDEX);
			a64::Gp index;
			if (index_is_reg)
			{
				index = div_10000(state, cc, get_z_register(state, cc, arg1));
			}
			a64::Gp value;
			if (value_is_reg)
				value = get_z_register(state, cc, arg2);

			InvokeNode* node;
			invoke(cc, &node, jit_pod_write, FuncSignature::build<void, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
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
			a64::Gp id = get_z_register(state, cc, arg1);
			InvokeNode* node;
			invoke(cc, &node, jit_writepodarr, FuncSignature::build<void, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, id);
			node->setArg(1, state.pc);
		}
		break;
		case ALLOCATEMEMV:
		{
			// reg[arg1] = allocate(size=arg2/10000, object_type=arg3).
			a64::Gp result = cc.newInt32();
			InvokeNode* node;
			invoke(cc, &node, jit_allocatemem, FuncSignature::build<int32_t, int32_t, int32_t, int32_t>(state.calling_convention));
			node->setArg(0, arg2 / 10000);
			node->setArg(1, instr.arg3);
			node->setArg(2, state.pc);
			node->setRet(0, result);
			set_z_register(state, cc, arg1, result);
		}
		break;
		case FLOOR:
		case CEILING:
		{
			// reg = round(reg * 1e-4) * 10000, rounding toward -inf (FLOOR) or
			// +inf (CEILING). Matches the x64 backend (cvtsi2sd / mulsd /
			// roundsd / cvttsd2si): the convert and multiply are
			// IEEE-identical, and fcvtms/fcvtps converts to integer rounding
			// toward -inf/+inf in one step, which equals x64's round-then-
			// truncate because the rounded value is integral.
			a64::Gp val = get_z_register(state, cc, arg1);
			a64::Gp bits = cc.newInt64();
			cc.mov(bits, (int64_t)4547007122018943789LL); // 1e-4
			a64::Vec scale = cc.newVecD();
			cc.fmov(scale, bits);
			a64::Vec y = cc.newVecD();
			cc.scvtf(y, val);
			cc.fmul(y, y, scale);
			if (command == FLOOR)
				cc.fcvtms(val, y);
			else
				cc.fcvtps(val, y);
			cc.mul(val, val, state.vTenK.w());
			set_z_register(state, cc, arg1, val);
		}
		break;
		case COMPAREV:
		{
			int val = arg2;
			a64::Gp val2 = get_z_register(state, cc, arg1);

			if (script->zasm[state.pc + 1].command == GOTOCMP || script->zasm[state.pc + 1].command == SETCMP)
			{
				if (script->zasm[state.pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					val2 = immutable_cast_bool(cc, val2);
				}
			}

			cmp_constant(cc, val2, val);
		}
		break;
		case COMPAREV2:
		{
			int val = arg1;
			a64::Gp val2 = get_z_register(state, cc, arg2);

			if (script->zasm[state.pc + 1].command == GOTOCMP || script->zasm[state.pc + 1].command == SETCMP)
			{
				if (script->zasm[state.pc + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					val2 = immutable_cast_bool(cc, val2);
				}
			}

			// This is a little silly. Could instead do `cmp(val2, val)`, but would have to teach
			// compile_compare to invert the conditional instruction it emits.
			a64::Gp val1 = imm_to_reg(cc, val);
			cc.cmp(val1, val2);
		}
		break;
		case COMPARER:
		{
			a64::Gp val = get_z_register(state, cc, arg2);
			a64::Gp val2 = get_z_register(state, cc, arg1);

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
		default:
		{
			Z_error_fatal("[jit ERROR] Unimplemented command: %s", zasm_op_to_string(command, arg1, arg2, instr.arg3, nullptr, nullptr).c_str());
		}
	}
}

// Hooks for the shared emit loop (jit_emit_function_body in
// jit_codegen_shared.h).
struct LoopOps
{
	CompilationState& state;
	a64::Compiler& cc;
	zasm_script* script;
	JittedScript* j_script;

	void set_pc(pc_t i) { state.pc = i; }
	void cache_flush_policy(pc_t i, pc_t block_id, bool is_block_start)
	{
		// All emission in the policy is str only, so a comparison result in
		// NZCV survives it.
		CacheOps ops{state, cc};
		jit_reg_cache_flush_policy(ops, state.dreg_cache, j_script, script, i, block_id, is_block_start);
	}
	void loop_extras([[maybe_unused]] int command) {}
	bool has_goto_label(pc_t i) { return state.goto_labels.contains(i); }
	void bind_goto_label(pc_t i) { cc.bind(state.goto_labels[i]); }
	void emit_comment_nop(const char* text)
	{
		cc.setInlineComment(text);
		cc.nop();
	}
	void emit_debug_pre_command(pc_t i)
	{
		a64::Gp sp = get_tmp_sp(state, cc);

		InvokeNode *invokeNode;
		invoke(cc, &invokeNode, debug_pre_command, FuncSignature::build<void, int32_t, uint32_t>(state.calling_convention));
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

	auto size_no_nops_opt = jit_function_size_within_cap(script, fn);
	if (!size_no_nops_opt)
		return std::nullopt;
	size_t size_no_nops = *size_no_nops_opt;

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	bool runtime_debugging = script_debug_is_runtime_debugging() == 2;

	CompilationState state{
		.script = script,
		.j_script = j_script,
		.start_pc = start_pc,
		.final_pc = final_pc,
		.runtime_debugging = runtime_debugging,
		// Emitted code averages well under 100 bytes per ZASM instruction, so
		// this keeps every possibly-far branch comfortably within b.cond's
		// +/-1 MB reach; bigger functions pay one extra unconditional branch
		// per taken-away conditional (see emit_cond_branch).
		.far_branches = size_no_nops > 8000,
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
		env._arch = Arch::kAArch64;
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

	a64::Compiler cc(&code);

	jit_create_labels(state, cc, script, start_pc, final_pc);

	auto fnNode = cc.addFunc(FuncSignature::build<int, JittedExecutionContext*>(state.calling_convention));
	state.ptrCtx = cc.newIntPtr("ctx");
	fnNode->setArg(0, state.ptrCtx);

	// These are plain virtual registers; asmjit's allocator handles them across
	// the whole function. (An earlier version pinned physical callee-saved
	// registers to survive an indirect-branch resume edge, but the resume
	// dispatch is now ordinary pc-comparison branches, so the allocator can
	// reconcile virtual registers across it normally.)
	state.ptrRegisters = cc.newIntPtr("registers");
	cc.ldr(state.ptrRegisters, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, registers)));

	state.ptrStackBase = cc.newIntPtr("stack_base");
	cc.ldr(state.ptrStackBase, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, stack_base)));

	state.vSp = cc.newUInt32("sp");
	cc.ldr(state.vSp, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, sp)));

	// A SWITCHKEY read can occur linearly before any write (e.g. a branch
	// target that saves the outer switch's key). The x64 backend gets away
	// with an uninitialized virtual register there; the a64 emitter rejects
	// the invalid operand, so give it a defined start value.
	state.vSwitchKey = cc.newInt32("switch_key");
	cc.mov(state.vSwitchKey, 0);

	// Initialized once in the entry block so it dominates every use, including
	// blocks entered via the resume dispatch below. Read-only after this.
	state.vTenK = cc.newInt64("ten_k");
	cc.mov(state.vTenK, 10000);

	state.vResult = cc.newUInt32("result");
	state.L_End = cc.newLabel();

	// If resuming after a call or wait, dispatch to the resume label. The x64
	// backend uses an indirect branch through a JumpAnnotation
	// (ctx->resume_address holds the native address); asmjit's AArch64 Compiler
	// miscompiles that pattern (the control flow around the annotation targets
	// collapses to a branch-to-self), so instead compare ctx->pc against each
	// resume point directly. A resume label bound at the command with pc K is
	// entered when execution resumes at K+1 (the interpreter sets ctx->pc there
	// after the call/wait completes). These direct branches also keep the
	// resume blocks reachable, so no annotation is needed.
	if (!state.resume_labels.empty())
	{
		a64::Gp pc_reg = cc.newInt32("resume_pc");
		cc.ldr(pc_reg, a64::ptr(state.ptrCtx, offsetof(JittedExecutionContext, pc)));
		for (auto& [k, label] : state.resume_labels)
		{
			cmp_constant(cc, pc_reg, k + 1);
			emit_cond_branch(state, cc, a64::CondCode::kEQ, label, k + 1 - start_pc);
		}
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

	// The single function epilog. All exits (QUIT, CALLFUNC, RETURNFUNC, stack
	// overflow, fall-through) branch here so there is exactly one ret; asmjit
	// restores callee-saved registers as part of the return.
	cc.bind(state.L_End);

	if (state.modified_stack)
		set_ctx_sp(state, cc, state.vSp);

	cc.ret(state.vResult);

	// Optional gate on the virtual register count, bailing to the interpreter
	// before finalize. Off by default: the failure modes that once made
	// register-heavy functions unsafe or uncompilable are handled (emit errors
	// discard the function; spill slots and far branches encode at any size),
	// and functions up to ~155k registers compile and run correctly. A few
	// giants still fail at label-bind time when asmjit places a branch fixup
	// trampoline out of b.cond range - that is not predictable from the
	// register count, and the discard below handles it. The flag remains as a
	// bisection/debugging knob.
	static size_t max_virt_regs = get_flag_int("-jit-a64-max-vregs").value_or(SIZE_MAX);
	if (cc.virtRegs().size() > max_virt_regs)
	{
		jit_printf("[jit] not compiling function, too many registers (name: %s, start: %d, regs: %u)\n", fn.name().c_str(), start_pc, cc.virtRegs().size());
		return std::nullopt;
	}

	cc.endFunc();
	cc.finalize();

	if (myErrorHandler.had_error)
	{
		// Emission hit an error; the generated code is unusable, so fall back
		// to the interpreter. A spill frame too large to encode is an expected
		// capacity limit and stays quiet; anything else is a codegen bug, and
		// jit_error makes it abort under -jit-fatal-compile-errors (CI).
		if (myErrorHandler.only_capacity_errors)
			jit_printf("[jit] not compiling function, spill frame too large (name: %s, start: %d, len: %zu)\n", fn.name().c_str(), start_pc, size_no_nops);
		else
			jit_error("[jit ERROR] discarding function due to emit error (name: %s, start: %d, len: %zu)\n", fn.name().c_str(), start_pc, size_no_nops);
		return std::nullopt;
	}

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

	std::map<pc_t, uintptr_t> pc_to_resume_address = jit_extract_resume_addresses(state.resume_labels, code);

	end_time = std::chrono::steady_clock::now();
	int32_t compile_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	jit_printf("[jit] compile function end   (name: %s, start: %d, len: %zu, ms: %d)\n", fn.name().c_str(), start_pc, size_no_nops, compile_ms);

	if (auto debug_handle = j_script->debug_handle.get())
		jit_print_compile_debug_dump(debug_handle, fn.name(), start_pc, compile_ms, code.codeSize(), size_no_nops, uncompiled_command_counts, logger.data());

	JittedFunction j_fn{
		.exec = compiled_fn,
		.id = fn.id,
		.pc_to_resume_address = std::move(pc_to_resume_address),
	};
	return j_fn;
}
