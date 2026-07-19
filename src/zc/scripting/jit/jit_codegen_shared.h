#ifndef ZC_JIT_CODEGEN_SHARED_H_
#define ZC_JIT_CODEGEN_SHARED_H_

// Code-generation logic shared between the two native JIT backends
// (jit_x64.cpp, jit_a64.cpp) that must stay semantically identical even
// though the backends emit through different asmjit compilers. The
// *decisions* - what to cache, what to flush, when a dead register's write
// can be dropped, how the emit loop is ordered - are single-sourced here;
// the backends supply only the primitive emissions through small adapters.
//
// This header is included ONLY by the two backend translation units, is
// header-only by necessity (templates over two asmjit compiler types), and
// is free to have heavy includes. It is one of two shared JIT layers:
//
//   jit_shared.{h,cpp}    - the runtime layer: plain data types the whole
//                           engine sees (via jit.h) and the driver .cpp
//                           (compile pipeline, run loop, lifecycle).
//   jit_codegen_shared.h  - this file: compile-time codegen policy that only
//                           the backends instantiate.
//
// Rule of thumb: something the engine or driver needs goes in jit_shared;
// something both backends need while emitting code goes here; something only
// one backend needs stays in that backend.
//
// The cache adapter ("Ops") must provide:
//
//   using Reg = <asmjit 32-bit GP virtual register type>;
//   Reg  new_reg32();                  // fresh virtual register
//   void emit_load_d(Reg dst, int r);  // dst <- ri->d[r]
//   void emit_store_d(Reg src, int r); // ri->d[r] <- src
//   void emit_mov(Reg dst, Reg src);
//   void emit_mov(Reg dst, int32_t imm);
//   void annotate(const char* text);   // inline comment when printing asm
//   void flush_cache();                // the backend's full flush: this
//                                      // D-register cache plus any extra
//                                      // backend caches (x64's stack cache)
//   bool is_command_compiled(int command);

#include "base/util.h"
#include "components/zasm/defines.h"
#include "components/zasm/serialize.h"
#include "components/zasm/table.h"
#include "zc/ffscript.h"
#include "zc/scripting/jit/jit_shared.h"
#include "zconsole/ConsoleLogger.h"

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <map>
#include <string>

// From zc/script_debug.h, which cannot be included here (it lacks an include
// guard and the backends include it themselves).
extern bool DEBUG_JIT_PRINT_ASM;

// Caches D0..D7 in host registers between flush points instead of loading and
// storing ri->d[] at every access. A register returned by get() may be shared
// with other users - treat it as READ-ONLY unless the mutated value is
// immediately set() back to the same D register.
template <typename Reg>
class DRegCache
{
public:
	bool empty() const
	{
		return entries.empty();
	}

	bool contains(int r) const
	{
		return entries.contains(r);
	}

	// Returns the register caching D-register r, loading it first if needed.
	template <typename Ops>
	Reg get(Ops& ops, int r)
	{
		if (!entries.contains(r))
		{
			Reg val = ops.new_reg32();
			ops.emit_load_d(val, r);
			entries[r] = {.reg=val};
			return val;
		}

		return entries[r].reg;
	}

	// Makes `val` (a register or an immediate) the cached value of D-register
	// r, marking it dirty so a later flush writes it to ri->d[].
	template <typename Ops, typename V>
	void set(Ops& ops, int r, V val)
	{
		if (entries.contains(r))
		{
			auto& entry = entries[r];
			ops.emit_mov(entry.reg, val);
			entry.dirty = true;
		}
		else
		{
			Reg reg = ops.new_reg32();
			entries[r] = {.reg = reg, .dirty = true};
			ops.emit_mov(reg, val);
		}
	}

	// Writes every dirty register back to ri->d[] and empties the cache.
	// Emits only stores, so a pending comparison result survives it.
	template <typename Ops>
	void flush(Ops& ops)
	{
		if (entries.empty())
			return;

		ops.annotate("flush cached registers");

		for (auto& [r, entry] : entries)
		{
			if (entry.dirty)
				ops.emit_store_d(entry.reg, r);
		}
		entries.clear();
	}

	// Some non-D registers are computed from D registers when accessed through
	// the C++ helpers (get_register/set_register), so those helpers must see
	// current values in ri->d[] for the D registers they depend on.
	template <typename Ops>
	void flush_dependents(Ops& ops, int r)
	{
		if (entries.empty())
			return;

		auto dep_regs = get_register_dependencies(r);
		if (dep_regs.empty())
			return;

		std::erase_if(entries, [&](const auto& pair){
			if (util::contains(dep_regs, pair.first))
			{
				if (pair.second.dirty)
					ops.emit_store_d(pair.second.reg, pair.first);
				return true;
			}

			return false;
		});
	}

	// Clears the dirty flag of every cached register not in live_mask, so the
	// next flush skips the (dead) write. Only sound at the points decided by
	// jit_reg_cache_flush_policy below.
	void drop_dead(uint8_t live_mask)
	{
		for (auto& [r, entry] : entries)
		{
			if (!(live_mask & (1 << r)))
				entry.dirty = false;
		}
	}

private:
	struct Entry
	{
		Reg reg;
		bool dirty = false;
	};

	std::map<int, Entry> entries;
};

// The D-register cache flush/invalidation points, applied before the command
// at pc `i` is compiled. This logic is subtle and has a history of dead-drop
// bugs (see the suspend-aware liveness work) - it must stay identical between
// the backends, which is why it lives here.
//
// This can run between a comparison and its consumer (e.g. at a GOTOCMP), so
// everything emitted here must leave the flags untouched. DRegCache emits
// only stores; the x64 adapter's chained stack-cache flush would emit a
// bounds-check cmp, but its loop_extras hook drains the stack cache before
// every compare precisely so that it is empty by the time this runs.
template <typename Ops, typename Reg>
void jit_reg_cache_flush_policy(Ops& ops, DRegCache<Reg>& cache, JittedScript* j_script,
	zasm_script* script, pc_t i, pc_t current_block_id, bool is_block_start)
{
	int command = script->zasm[i].command;

	if (command_is_wait(command))
	{
		ops.flush_cache();
	}
	else if (!ops.is_command_compiled(command))
	{
		// The interpreter reads and writes ri->d[] directly.
		ops.flush_cache();
	}
	else if (command_is_goto(command) || command == CALLFUNC || command == RETURNFUNC)
	{
		// A CALLFUNC into a function that can suspend (a WaitX/RUNGENFRZSCR reached
		// transitively) needs the caller's whole modified register state in memory: at the
		// callee's suspend the full D-register file is serialized to ri->d[] and restored
		// on resume, and this caller's intra-procedural liveness cannot see that its
		// registers are observed there. So flush every dirty register (skip the dead-drop)
		// before such a call. A CALLFUNC ends a block, so the cache holds a single-path
		// value here - flushing it is always correct (unlike a merge block start).
		bool callee_may_yield = false;
		if (command == CALLFUNC)
		{
			auto it = j_script->structured_zasm.start_pc_to_function.find(script->zasm[i].arg1);
			callee_may_yield = it != j_script->structured_zasm.start_pc_to_function.end() &&
				j_script->structured_zasm.functions[it->second].may_yield;
		}

		// A RETURNFUNC hands control back to the caller, whose successors this function's
		// intra-procedural liveness cannot see. ZASM D-registers are global, so every
		// register this function wrote (dirty in the cache) is observed by the caller once
		// it resumes - exactly as the interpreter leaves them in ri->d[]. The block's own
		// .out for a returns-block is only D2 (the return-value convention), so the dead-drop
		// would strand any other register written here (e.g. a POP D3 that a setter helper
		// does before returning). Skip the dead-drop and flush every dirty register.
		bool is_returnfunc = command == RETURNFUNC;

		if (!is_returnfunc && !callee_may_yield && j_script->cfg.contains_block_start(i + 1))
		{
			uint8_t out = j_script->liveness[current_block_id].out;

			// For a CALLFUNC the block's own .out is the callee's live-in (the CFG edge
			// goes to the callee), which does not capture what the caller needs once the
			// call returns. Execution resumes at i+1, and a register live there must
			// survive the call: the callee is not required to write every register, so a
			// value it leaves untouched has to already be in ri->d[] (e.g. a loop/array
			// index kept in a register across a helper call). Keep those too.
			if (command == CALLFUNC)
				out |= j_script->liveness[j_script->cfg.block_id_from_start_pc(i + 1)].in;

			cache.drop_dead(out);
		}

		ops.flush_cache();
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
				cache.drop_dead(j_script->liveness[current_block_id - 1].out);

			ops.flush_cache();
		}
	}
}

// Counts a function's non-NOP instructions and rejects functions too large to
// be worth compiling. Returns std::nullopt if the function should stay
// interpreted.
inline std::optional<size_t> jit_function_size_within_cap(zasm_script* script, const ZasmFunction& fn)
{
	size_t size_no_nops = 0;
	for (pc_t i = fn.start_pc; i <= fn.final_pc; i++)
	{
		if (script->zasm[i].command != NOP)
			size_no_nops += 1;
	}

	// ~170k is the largest function I've seen (from Yuurand, but if optimizer is on that script is ~100k).
	if (size_no_nops > 150000)
	{
		al_trace("[jit] not compiling function because it is too big (name: %s, start: %d, len: %zu)\n", fn.name().c_str(), fn.start_pc, size_no_nops);
		return std::nullopt;
	}

	return size_no_nops;
}

// Creates the control flow labels: one per in-function GOTO target, and one
// resume label per call/wait (where execution re-enters the function).
template <typename State, typename Compiler>
void jit_create_labels(State& state, Compiler& cc, zasm_script* script, pc_t start_pc, pc_t final_pc)
{
	for (size_t i = start_pc; i <= final_pc; i++)
	{
		int command = script->zasm[i].command;
		if (command_is_goto(command))
		{
			int pc = script->zasm[i].arg1;
			if (pc >= (int)start_pc && pc <= (int)final_pc)
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
}

// Scans the contiguous run of PUSH/POP commands starting at pc, returning the
// largest stack growth within it and where the run ends - so the backend can
// bounds-check the stack pointer once per run instead of at every push.
struct JitPushRunScan
{
	int max_stack_delta;
	pc_t end_pc;
};

inline JitPushRunScan jit_scan_push_run(const zasm_script* script, pc_t pc)
{
	// The first command may itself be a multi-slot PUSHARGS (the optimizer's
	// push-combining makes that the common case). Counting it as 1 understated
	// the run's depth by arg2-1 slots, so a script within that many slots of
	// the limit would write out of bounds instead of exiting with
	// RUNSCRIPT_JIT_STACK_OVERFLOW.
	const auto& op0 = script->zasm[pc];
	bool starts_with_pushargs = op0.command == PUSHARGSV || op0.command == PUSHARGSR;
	int stack_delta = starts_with_pushargs ? std::max(1, op0.arg2) : 1;
	int max_stack_delta = stack_delta;

	pc_t j = pc + 1;
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

	return {max_stack_delta, j};
}

// Maps each resume point to the native address of its label, for re-entering
// the compiled function after a call or wait. `code` is the finalized asmjit
// CodeHolder (templated to avoid naming asmjit types here).
template <typename Labels, typename CodeHolderT>
std::map<pc_t, uintptr_t> jit_extract_resume_addresses(const Labels& resume_labels, CodeHolderT& code)
{
	uintptr_t base = code.baseAddress();

	std::map<pc_t, uintptr_t> pc_to_resume_address;
	for (const auto& it : resume_labels)
	{
		pc_to_resume_address[it.first] = base + code.labelOffsetFromBase(it.second);
	}
	return pc_to_resume_address;
}

// Writes the per-function compile report (stats, uncompiled command counts,
// and the asmjit log/assembly) to the script debug file.
template <typename DebugHandle>
void jit_print_compile_debug_dump(DebugHandle* debug_handle, const std::string& fn_name,
	pc_t start_pc, int32_t compile_ms, size_t code_size_bytes, size_t size_no_nops,
	const std::map<int, int>& uncompiled_command_counts, const char* logger_data)
{
	debug_handle->printf("function:           %s\n", fn_name.c_str());
	debug_handle->printf("start pc:           %d\n", start_pc);
	debug_handle->printf("time to compile:    %d ms\n", compile_ms);
	debug_handle->printf("Code size:          %.1f kb\n", code_size_bytes / 1024.0);
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
		logger_data);
	debug_handle->print("\n");

	debug_handle->update_file();
}

// Drives the per-command emission for one function: block tracking, the
// cache flush policy, label binding, dispatch to the compare/wait/uncompiled/
// compiled paths (with uncompiled commands batched into single interpreter
// calls), and the debug annotations. The loop structure and its ordering are
// load-bearing (labels must bind after the flush policy so every jump lands
// on a consistent, empty cache state) and identical between the backends, so
// they live here; the backend supplies the emission hooks:
//
//   void set_pc(pc_t i);
//   void cache_flush_policy(pc_t i, pc_t block_id, bool is_block_start);
//   void loop_extras(int command);      // backend-specific per-command prep
//   bool has_goto_label(pc_t i);
//   void bind_goto_label(pc_t i);
//   void emit_comment_nop(const char* text);
//   void emit_debug_pre_command(pc_t i);
//   bool is_command_compiled(int command);
//   void compile_compare(const ffscript& op);
//   void compile_wait();
//   void compile_uncompiled_batch(int count);
//   void compile_command(const ffscript& op);
//
// `comment` must outlive finalize (asmjit keeps the inline-comment pointer
// for the logger), so the caller owns it.
template <typename Backend>
void jit_emit_function_body(Backend& b, zasm_script* script, JittedScript* j_script,
	pc_t start_pc, pc_t final_pc, bool runtime_debugging,
	std::string& comment, std::map<int, int>& uncompiled_command_counts)
{
	pc_t current_block_id = j_script->cfg.block_id_from_start_pc(start_pc);

	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		b.set_pc(i);

		const auto& op = script->zasm[i];
		int command = op.command;

		bool is_block_start;
		if (i == start_pc)
		{
			is_block_start = true;
		}
		else
		{
			is_block_start = j_script->cfg.contains_block_start(i);
			if (is_block_start)
				current_block_id++;
		}

		b.cache_flush_policy(i, current_block_id, is_block_start);
		b.loop_extras(command);

		if (b.has_goto_label(i))
			b.bind_goto_label(i);

		if (DEBUG_JIT_PRINT_ASM && j_script->structured_zasm.start_pc_to_function.contains(i))
			b.emit_comment_nop((comment = fmt::format("function {}", j_script->structured_zasm.start_pc_to_function[i])).c_str());

		if (DEBUG_JIT_PRINT_ASM)
			b.emit_comment_nop((comment = fmt::format("{} {}", i, zasm_op_to_string(op))).c_str());

		// Debugging tool used by scripts/jit_runtime_debug.py.
		//
		// We can't invoke functions between COMPARE and the instructions that use the comparison
		// result, because that would destroy the flags. So we must skip the debug printout for
		// these instructions, which results in them being grouped together in the output and the
		// stack/register trace being printed just once for the entire group of instructions.
		if (runtime_debugging && !command_uses_comparison_result(command))
			b.emit_debug_pre_command(i);

		if (command_uses_comparison_result(command))
		{
			b.compile_compare(op);
			continue;
		}

		if (command_is_wait(command))
		{
			// This returns only if actually waiting (some wait commands may be deemed invalid and
			// ignored, so waiting is conditional).
			b.compile_wait();
			continue;
		}

		if (!b.is_command_compiled(command))
		{
			if (DEBUG_JIT_PRINT_ASM && command != 0xFFFF)
				uncompiled_command_counts[command]++;

			// Every command that is not compiled to assembly must go through the regular
			// interpreter function. In order to reduce function call overhead, we call into the
			// interpreter function in batches.
			int uncompiled_command_count = 1;
			for (pc_t j = i + 1; j <= final_pc; j++)
			{
				if (b.is_command_compiled(script->zasm[j].command))
					break;
				if (b.has_goto_label(j))
					break;

				if (DEBUG_JIT_PRINT_ASM && script->zasm[j].command != 0xFFFF)
					uncompiled_command_counts[script->zasm[j].command]++;

				uncompiled_command_count += 1;
				if (DEBUG_JIT_PRINT_ASM)
					b.emit_comment_nop((comment = fmt::format("{} {}", j, zasm_op_to_string(script->zasm[j]))).c_str());
			}

			b.compile_uncompiled_batch(uncompiled_command_count);
			i += uncompiled_command_count - 1;
			continue;
		}

		b.compile_command(op);
	}
}

// The commands both native backends compile (everything else runs through the
// interpreter in batches). FLOOR and CEILING are decided by the backend:
// x64 gates them on SSE4.1, a64 compiles them unconditionally.
inline bool jit_command_is_compiled_shared(int command)
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
	case DIVR:
	case DIVV:
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

#endif
