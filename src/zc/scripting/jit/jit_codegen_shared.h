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
#include "base/zapp.h"
#include "components/zasm/defines.h"
#include "components/zasm/serialize.h"
#include "components/zasm/table.h"
#include "zc/ffscript.h"
#include "zc/scripting/jit/jit_shared.h"
#include "zconsole/ConsoleLogger.h"

#include <algorithm>
#include <climits>
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
// at pc `i` is compiled. Dropping a dirty register is only as sound as the
// liveness analysis it relies on (see zasm_analyze_function_liveness) - this
// must stay identical between the backends, which is why it lives here.
//
// This can run between a comparison and its consumer (e.g. at a GOTOCMP), so
// everything emitted here must leave the flags untouched. DRegCache emits
// only stores; the x64 adapter's chained stack-cache flush would emit a
// bounds-check cmp, but its loop_extras hook drains the stack cache before
// every compare precisely so that it is empty by the time this runs.
template <typename Ops, typename Reg>
void jit_reg_cache_flush_policy(Ops& ops, DRegCache<Reg>& cache, JittedScript* j_script,
	const JitFunctionAnalysis& analysis, zasm_script* script, pc_t i, pc_t current_block_id, bool is_block_start)
{
	int command = script->zasm[i].command;

	if (command_is_wait(command))
	{
		ops.flush_cache();
	}
	else if (!ops.is_instruction_compiled(script->zasm[i]))
	{
		// The interpreter reads and writes ri->d[] directly.
		ops.flush_cache();
	}
	else if (command_is_goto(command) || command == GOTOTABLE || command == GOTORANGES || command == CALLFUNC || command == RETURNFUNC)
	{
		// Control leaves the block, so flush. A register that is dead just before this
		// instruction needs no write-back - which is known when the instruction ends its block,
		// as then what is live after it is the block's live-out. For a CALLFUNC that is what is
		// live once the call returns, and for a RETURNFUNC what the callers may read (see
		// zasm_analyze_function_liveness).
		if (analysis.cfg.contains_block_start(i + 1))
		{
			uint8_t live = zasm_live_before(script->zasm[i], analysis.liveness[current_block_id].out,
				j_script->structured_zasm, j_script->function_liveness, true);
			cache.drop_dead(live);
		}

		ops.flush_cache();
	}
	else if (is_block_start)
	{
		pc_t block_id = current_block_id;
		bool is_linear_flow =
			block_id > 0 &&
			analysis.cfg.block_edges[block_id - 1].size() == 1 &&
			analysis.cfg.block_edges[block_id - 1][0] == block_id &&
			analysis.block_predecessors[block_id].size() == 1;
		if (!is_linear_flow)
		{
			if (current_block_id > 0)
				cache.drop_dead(analysis.liveness[current_block_id - 1].out);

			ops.flush_cache();
		}
	}
}

// Flushes the register cache before falling off the end of a piece into the function's next piece
// (see JitPiece). What may still be read there is what is live just before the next piece's first
// command: the live-out of the block the piece ends in, less what the rest of that block writes
// first (when the next piece starts in the middle of the block).
template <typename Ops, typename Reg>
void jit_reg_cache_flush_at_piece_end(Ops& ops, DRegCache<Reg>& cache, JittedScript* j_script,
	const JitFunctionAnalysis& analysis, zasm_script* script, pc_t final_pc)
{
	pc_t block_id = analysis.cfg.block_id_containing(final_pc);
	uint8_t live = analysis.liveness[block_id].out;
	for (pc_t i = analysis.cfg.get_block_final(block_id); i > final_pc; i--)
		live = zasm_live_before(script->zasm[i], live, j_script->structured_zasm, j_script->function_liveness, true);
	cache.drop_dead(live);
	ops.flush_cache();
}

// Counts a piece's non-NOP instructions.
inline size_t jit_piece_size_no_nops(zasm_script* script, pc_t start_pc, pc_t final_pc)
{
	size_t size_no_nops = 0;
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		if (script->zasm[i].command != NOP)
			size_no_nops += 1;
	}
	return size_no_nops;
}

// Creates the control flow labels: one per in-piece GOTO target, and one
// resume label per call/wait (where execution re-enters the function). A piece
// of a function (see JitPiece) can also be entered at the targets of jumps from
// its function's other pieces: those are registered like resume points, keyed by
// the pc before the target - the key the driver looks up on entering at a pc.
template <typename State, typename Compiler>
void jit_create_labels(State& state, Compiler& cc, zasm_script* script, const ZasmFunction& fn, pc_t start_pc, pc_t final_pc)
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
		else if (command == GOTOTABLE || command == GOTORANGES)
		{
			for (int pc : zasm_jump_targets(command, script->literals.vec(script->zasm[i])))
			{
				if (pc >= (int)start_pc && pc <= (int)final_pc && !state.goto_labels.contains(pc))
					state.goto_labels[pc] = cc.newLabel();
			}
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

	auto add_entry = [&](int pc) {
		if (pc <= (int)start_pc || pc > (int)final_pc)
			return;
		if (!state.goto_labels.contains(pc))
			state.goto_labels[pc] = cc.newLabel();
		// A call or wait just before the target already resumes at the same place: the register
		// cache is empty after either, and there is nothing to flush on the way to the target.
		if (!state.resume_labels.contains(pc - 1))
			state.resume_labels[pc - 1] = state.goto_labels[pc];
	};
	for (pc_t i = fn.start_pc; i <= fn.final_pc; i++)
	{
		if (i == start_pc)
		{
			i = final_pc;
			continue;
		}

		int command = script->zasm[i].command;
		if (command_is_goto(command))
		{
			add_entry(script->zasm[i].arg1);
		}
		else if (command == GOTOTABLE || command == GOTORANGES)
		{
			for (int pc : zasm_jump_targets(command, script->literals.vec(script->zasm[i])))
				add_entry(pc);
		}
	}
}

// Scans the contiguous run of stack commands (pushes, POP, POPARGS, PEEK) starting at pc,
// returning which stack slots it reads and writes, and where the run ends - so the backend can
// bounds-check the stack pointer once per run instead of at every push, pop and peek.
//
// Each slot is given by its depth below the stack pointer at the start of the run: a push writes
// the slot at the depth it leaves the stack at, and a pop or peek reads the one at the depth it
// finds it at. POPARGS only moves the stack pointer here, as it checks its own read.
struct JitPushRunScan
{
	int min_depth;
	int max_depth;
	pc_t end_pc;
};

inline JitPushRunScan jit_scan_push_run(const zasm_script* script, pc_t pc, pc_t final_pc)
{
	int depth = 0;
	int min_depth = INT_MAX;
	int max_depth = INT_MIN;
	auto access = [&](int lo, int hi) {
		min_depth = std::min(min_depth, lo);
		max_depth = std::max(max_depth, hi);
	};

	pc_t j = pc;
	for (; j <= final_pc; j++)
	{
		const auto& op = script->zasm[j];
		// A count out of range is left to the interpreter (jit_instruction_runs_in_interpreter),
		// which ends the run. Every count here is then at most MAX_STACK_SIZE, so the sums can't
		// overflow.
		if (jit_instruction_runs_in_interpreter(op))
			break;
		if (op.command == PUSHV || op.command == PUSHR)
		{
			depth++;
			access(depth, depth);
		}
		else if (op.command == PUSHARGSV || op.command == PUSHARGSR)
		{
			if (op.arg2 > 0)
				access(depth + 1, depth + op.arg2);
			depth += op.arg2;
		}
		else if (op.command == POP)
		{
			access(depth, depth);
			depth--;
		}
		else if (op.command == PEEK)
		{
			access(depth, depth);
		}
		else if (op.command == POPARGS)
		{
			depth -= op.arg2;
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

	// A run with no access (only a PUSHARGS of 0) checks nothing beyond sp <= MAX_STACK_SIZE.
	if (min_depth > max_depth)
		min_depth = max_depth = 0;
	return {min_depth, max_depth, j};
}

// The limit for checking a run that touches the slots at depths [min_depth, max_depth] below the
// stack pointer, with check_sp(sp, -max_depth, limit). Every slot sp - depth must be in
// [0, MAX_STACK_SIZE), so sp - max_depth >= 0 and sp - min_depth < MAX_STACK_SIZE. Checking the
// first alone would let a stack pointer the interpreter left out of range (SETR SP can set it to
// anything) write past the stack. One unsigned compare checks both.
inline int jit_stack_run_sp_limit(int min_depth, int max_depth)
{
	return std::max(0, MAX_STACK_SIZE + min_depth - max_depth);
}

// The limit for checking `depth` pushes with check_sp(sp, -depth, limit): they write the slots at
// depths [1, depth].
inline int jit_push_sp_limit(int depth)
{
	return jit_stack_run_sp_limit(1, depth);
}

// How to bounds-check the stack accesses at a constant offset from rSFRAME (LOAD, STORE, STOREV).
//
// rSFRAME holds the same value from when a function sets up its frame until it calls a function
// (which sets up its own, and the caller's is popped back after). So rather than check every
// access, one check that rSFRAME plus every such offset in the piece is in range covers each
// access after it, until rSFRAME may change again. The interpreter checks each access instead,
// but a compiled function's frame is all set up when it starts, so every offset it uses is in
// range of it. So for compiler output this never fails. Only hand-written ZASM can fail it, and
// that may stop sooner than the interpreter would, or even stop a script the interpreter
// wouldn't (if the out of range access is on a path that never runs).
struct JitFrameAccessPlan
{
	enum Mode : uint8_t
	{
		// Check this access alone.
		CHECK_ACCESS,
		// Check that rSFRAME plus every offset up to max_offset is in range, then this access
		// needs no check.
		CHECK_RANGE,
		// A range check already covers this access.
		NO_CHECK,
	};

	// By pc - start_pc. Empty if every access is checked alone.
	std::vector<Mode> modes;
	pc_t start_pc = 0;
	// MAX_STACK_SIZE less the largest offset of any access in the piece (the smallest is at least
	// 0). Checking that rSFRAME < limit covers them all, with no wraparound.
	int32_t limit = MAX_STACK_SIZE;

	Mode mode(pc_t pc) const
	{
		return modes.empty() ? CHECK_ACCESS : modes[pc - start_pc];
	}
};

inline bool jit_is_frame_access(int command)
{
	return command == LOAD || command == STORE || command == STOREV;
}

// True if rSFRAME may hold a different value after this instruction.
inline bool jit_may_change_sframe(const ffscript& op)
{
	// A call sets up the callee's frame, and nothing stops a script from changing registers while
	// suspended (it may resume elsewhere, too).
	if (op.command == CALLFUNC || instruction_is_suspend(op))
		return true;

	const auto& meta = zasm_get_command_meta(op.command);
	if (meta.implicit_write_mask & (1 << rSFRAME))
		return true;

	const int32_t* args = &op.arg1;
	for (int i = 0; i < 3; i++)
	{
		if (meta.args[i].writes && args[i] == rSFRAME)
			return true;
	}
	return false;
}

// `entries` are where control enters the piece other than at start_pc: the resume labels, keyed
// by the pc before the one execution continues at.
template <typename Labels>
JitFrameAccessPlan jit_plan_frame_accesses(const zasm_script* script, const JittedScript* j_script,
	pc_t start_pc, pc_t final_pc, const Labels& entries)
{
	JitFrameAccessPlan plan;

	// Control only enters a function at its start (or the resume points of its pieces) for modern
	// function calls.
	if (j_script->script_analysis || !j_script->structured_zasm.is_modern_function_calling())
		return plan;

	int32_t max_offset = -1;
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		const auto& op = script->zasm[i];
		if (!jit_is_frame_access(op.command))
			continue;
		if (op.arg2 < 0 || op.arg2 >= MAX_STACK_SIZE)
			return plan;
		max_offset = std::max(max_offset, op.arg2);
	}
	if (max_offset == -1)
		return plan;

	size_t n = final_pc - start_pc + 1;

	// Whether rSFRAME has been range checked since it last may have changed, before (in) and after
	// (out) each instruction: a forward "must" dataflow, from every value set (then lowered where
	// a path in has no check) until nothing changes.
	std::vector<uint8_t> is_entry(n), checked_out(n, 1), falls_through(n, 1);
	std::vector<std::vector<pc_t>> jumps_in(n);
	is_entry[0] = 1;
	for (const auto& [pc, label] : entries)
	{
		if (pc + 1 >= start_pc && pc + 1 <= final_pc)
			is_entry[pc + 1 - start_pc] = 1;
	}
	for (pc_t i = start_pc; i <= final_pc; i++)
	{
		const auto& op = script->zasm[i];
		auto add_jump = [&](int target) {
			if (target >= (int)start_pc && target <= (int)final_pc)
				jumps_in[target - start_pc].push_back(i - start_pc);
		};
		if (command_is_goto(op.command))
			add_jump(op.arg1);
		else if (op.command == GOTOTABLE || op.command == GOTORANGES)
		{
			for (int pc : zasm_jump_targets(op.command, script->literals.vec(op)))
				add_jump(pc);
		}

		if (op.command == GOTO || op.command == RETURNFUNC || op.command == QUIT)
			falls_through[i - start_pc] = 0;
	}

	plan.modes.resize(n);
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (size_t k = 0; k < n; k++)
		{
			bool checked = !is_entry[k];
			if (checked && k > 0 && falls_through[k - 1])
				checked = checked_out[k - 1];
			for (pc_t from : jumps_in[k])
				checked = checked && checked_out[from];

			const auto& op = script->zasm[start_pc + k];
			if (jit_is_frame_access(op.command))
			{
				plan.modes[k] = checked ? JitFrameAccessPlan::NO_CHECK : JitFrameAccessPlan::CHECK_RANGE;
				checked = true;
			}
			if (jit_may_change_sframe(op))
				checked = false;

			if ((bool)checked_out[k] != checked)
			{
				checked_out[k] = checked;
				changed = true;
			}
		}
	}

	plan.start_pc = start_pc;
	plan.limit = MAX_STACK_SIZE - max_offset;
	return plan;
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
	const JitFunctionAnalysis& analysis, pc_t start_pc, pc_t final_pc, bool runtime_debugging,
	std::string& comment, std::map<int, int>& uncompiled_command_counts)
{
	// A piece of a function may start in the middle of a block (see jit_split_function).
	pc_t current_block_id = analysis.cfg.block_id_containing(start_pc);

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
			is_block_start = analysis.cfg.contains_block_start(i);
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
			b.emit_comment_nop((comment = fmt::format("{} {}", i, zasm_op_to_string(op, script->literals))).c_str());

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

		if (!b.is_instruction_compiled(op))
		{
			if (DEBUG_JIT_PRINT_ASM && command != 0xFFFF)
				uncompiled_command_counts[command]++;

			// Every command that is not compiled to assembly must go through the regular
			// interpreter function. In order to reduce function call overhead, we call into the
			// interpreter function in batches.
			int uncompiled_command_count = 1;
			for (pc_t j = i + 1; j <= final_pc; j++)
			{
				if (b.is_instruction_compiled(script->zasm[j]))
					break;
				if (b.has_goto_label(j))
					break;

				if (DEBUG_JIT_PRINT_ASM && script->zasm[j].command != 0xFFFF)
					uncompiled_command_counts[script->zasm[j].command]++;

				uncompiled_command_count += 1;
				if (DEBUG_JIT_PRINT_ASM)
					b.emit_comment_nop((comment = fmt::format("{} {}", j, zasm_op_to_string(script->zasm[j], script->literals))).c_str());
			}

			b.compile_uncompiled_batch(uncompiled_command_count);
			i += uncompiled_command_count - 1;
			continue;
		}

		b.compile_command(op);
	}
}

// The byte offset of a refInfo member from refInfo::d, which compiled code holds a pointer to
// (JittedExecutionContext::registers).
inline int32_t jit_ri_offset(size_t member_offset)
{
	return (int32_t)(member_offset - offsetof(refInfo, d));
}

// True if any of the `count` instructions at `pc` moves sp (see command_moves_sp).
inline bool jit_instructions_move_sp(const zasm_script* script, pc_t pc, int count)
{
	for (int j = 0; j < count; j++)
	{
		if (command_moves_sp(script->zasm[pc + j].command))
			return true;
	}
	return false;
}

// The byte offset of a script ref (see SCRIPT_REFS) from refInfo::d, which compiled code holds a
// pointer to (JittedExecutionContext::registers). Lets compiled code read and write the REF*
// registers directly, instead of calling into get_register/set_register.
inline int32_t jit_script_ref_offset(ScriptRef ref)
{
	return jit_ri_offset(offsetof(refInfo, refs)) + (int32_t)ref * 4;
}

// The commands both native backends compile (everything else runs through the
// interpreter in batches). FLOOR and CEILING are decided by the backend:
// x64 gates them on SSE4.1, a64 compiles them unconditionally.
inline bool jit_command_is_compiled_shared(int command)
{
	if (command_is_wait(command))
		return true;

	// Comparison producers and consumers are critical to control flow. That includes STRCMPR/
	// STRICMPR (string switch): an interpreted STRCMPR leaves the host flags (and the JIT-held
	// SWITCHKEY) unrelated to the result the following GOTOCMP branches on.
	if (command_uses_comparison_result(command) || command_sets_comparison_result(command))
		return true;

	switch (command)
	{
	// These commands are critical to control flow.
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
	case ZCLASS_READ:
	case ZCLASS_WRITE:
	case GOTOTABLE:
	case GOTORANGES:
	case REF_INC:
	case REF_DEC:
	case MARK_TYPE_STACK:
	case ARRAYSIZE:
		return true;
	}

	return false;
}

#endif
