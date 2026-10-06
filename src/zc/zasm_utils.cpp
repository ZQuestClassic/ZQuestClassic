#include "zc/zasm_utils.h"
#include "core/qst.h"
#include "core/zdefs.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/parallel.h"
#include "zc/script_debug.h"
#include "components/zasm/serialize.h"
#include "components/zasm/table.h"
#include "base/util.h"
#include <algorithm>
#include <cstdint>
#include <span>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <xxhash.h>
#include <vector>
#include <sstream>

static std::vector<CommandMetadata> command_meta_cache;
static std::vector<uint8_t> register_dependency_mask_cache;

void zasm_init_meta_cache()
{
	if (command_meta_cache.size())
		return;

	command_meta_cache.resize(NUMCOMMANDS);
	register_dependency_mask_cache.resize(NUMVARIABLES);

	for (int i = 0; i < NUMCOMMANDS; i++)
	{
		auto sc = get_script_command(i);
		if (!sc)
			continue;

		auto& meta = command_meta_cache[i];

		for (auto [reg, rw] : get_command_implicit_dependencies(i))
		{
			if (reg >= 8)
				continue;

			if (rw == ARGTY::READ_REG || rw == ARGTY::READWRITE_REG)
				meta.implicit_read_mask |= (1 << reg);
			if (rw == ARGTY::WRITE_REG || rw == ARGTY::READWRITE_REG)
				meta.implicit_write_mask |= (1 << reg);
		}

		for (int argn = 0; argn < 3; ++argn)
		{
			if (sc->is_register(argn))
			{
				meta.args[argn].is_reg = true;
				auto [read, write] = get_command_rw(i, argn);
				meta.args[argn].reads = read;
				meta.args[argn].writes = write;
			}
		}
	}

	for (int i = 0; i < NUMVARIABLES; i++)
	{
		uint8_t mask = 0;
		for (int r : get_register_dependencies(i))
		{
			if (r < 8)
				mask |= (1 << r);
		}
		register_dependency_mask_cache[i] = mask;
	}
}

const std::vector<CommandMetadata>& zasm_get_command_meta_cache()
{
	return command_meta_cache;
}

const std::vector<uint8_t>& zasm_get_register_dependency_mask_cache()
{
	return register_dependency_mask_cache;
}

bool StructuredZasm::is_modern_function_calling()
{
	// UNKNOWN is included b/c scripts like "global init" don't typically have any function calls,
	// so might as well consider it "modern".
	return calling_mode == CALLING_MODE_CALLFUNC_RETURNFUNC || calling_mode == CALLING_MODE_UNKNOWN;
}

StructuredZasm zasm_construct_structured(const zasm_script* script)
{
	std::vector<ZasmFunction> functions;
	std::set<pc_t> function_calls;
	std::map<pc_t, pc_t> start_pc_to_function;

	if (zasm_debug_data.exists())
	{
		for (const auto& scope : zasm_debug_data.scopes)
		{
			if (scope.tag == TAG_FUNCTION)
			{
				if (scope.start_pc == 0 && scope.end_pc == 0)
					continue;

				functions.push_back({0, zasm_debug_data.getFullScopeName(&scope), scope.start_pc, scope.end_pc});
			}
		}

		std::sort(functions.begin(), functions.end(), [](auto& a, auto& b){
			return a.start_pc < b.start_pc;
		});

		for (int i = 0; i < functions.size(); i++)
		{
			functions[i].id = i;
			start_pc_to_function[functions[i].start_pc] = i;
		}

		// Find all function calls, and build the call graph (called_by_functions).
		// The latter is needed by zasm_find_yielding_functions to propagate "may
		// yield" up to transitive callers - e.g. a function that calls a function
		// containing a wait or RUNGENFRZSCR. (The legacy path below builds this too.)
		auto fn_containing = [&](pc_t pc) -> int {
			int lo = 0, hi = (int)functions.size() - 1, res = -1;
			while (lo <= hi)
			{
				int mid = (lo + hi) / 2;
				if (functions[mid].start_pc <= pc) { res = mid; lo = mid + 1; }
				else hi = mid - 1;
			}
			if (res != -1 && pc <= functions[res].final_pc) return res;
			return -1;
		};
		for (pc_t i = 0; i < script->size; i++)
		{
			int command = script->zasm[i].command;
			if (command != CALLFUNC)
				continue;
			function_calls.insert(i);

			auto callee_it = start_pc_to_function.find(script->zasm[i].arg1);
			if (callee_it == start_pc_to_function.end())
				continue;
			int caller = fn_containing(i);
			if (caller != -1)
				functions[callee_it->second].called_by_functions.insert(caller);
		}

		for (auto sd : script->script_datas)
		{
			auto& fn = functions.at(start_pc_to_function.at(sd->pc));
			fn.is_entry_function = true;
		}

		return {functions, function_calls, start_pc_to_function, StructuredZasm::CALLING_MODE_CALLFUNC_RETURNFUNC};
	}

	// Three forms of function calls over the ages:

	// 1) GOTO/GOTOR
	// The oldest looks like this:
	//
	//    SETV D2 (pc two after the GOTO)*10000
	//    PUSHR D2
	//    ... other ops to push the function args ...
	//    GOTO x
	// 
	// x: ...
	//    POP D3
	//    GOTOR D3
	//
	// GOTOR only ever used D3. POP D3 could instead be POPARGS.
	// Could also use D3 to set/push the return address.

	// 2) GOTO/RETURN
	//
	//    PUSHV (pc two after the GOTO)
	//    ... other ops to push the function args ...
	//    GOTO x
	// 
	// x: ...
	//    RETURN

	// 3) CALLFUNC/RETURNFUNC
	//
	//    CALLFUNC x
	// 
	// x: ...
	//    RETURNFUNC
	//
	// CALLFUNC pushes the return address onto a function call statck. RETURNFUNC pops from that stack.

	// There is nothing special marking the start or end of a function in ZASM. So,
	// the only way to contruct the bounds of each function is to search for function calls,
	// which gets the function starts. Then, the function ends are derived with these.
	// Note - if a function is not called at all, then the instructions for that function will
	// be part of an unreachable sequence of blocks in the prior function that was called.
	// Therefore, the instructions of uncalled functions should be pruned as part of zasm_optimize.

	// First determine if we have the simpler CALLFUNC instructions.
	auto calling_mode = StructuredZasm::CALLING_MODE_UNKNOWN;
	for (pc_t i = 0; i < script->size && !calling_mode; i++)
	{
		int command = script->zasm[i].command;
		switch (command)
		{
			case GOTOR:
				calling_mode = StructuredZasm::CALLING_MODE_GOTO_GOTOR;
				break;

			case RETURN:
				calling_mode = StructuredZasm::CALLING_MODE_GOTO_RETURN;
				break;

			case CALLFUNC:
			case RETURNFUNC:
				calling_mode = StructuredZasm::CALLING_MODE_CALLFUNC_RETURNFUNC;
				break;
		}
	}
	bool legacy_calling_mode =
		calling_mode == StructuredZasm::CALLING_MODE_GOTO_GOTOR || calling_mode == StructuredZasm::CALLING_MODE_GOTO_RETURN;

	// All the entry points are obviously functions (ex: "run");
	std::set<pc_t> function_start_pcs_set = {0};
	for (auto sd : script->script_datas)
		function_start_pcs_set.insert(sd->pc);

	// Find all function calls.
	std::map<pc_t, pc_t> function_calls_pc_to_pc;
	for (pc_t i = 0; i < script->size; i++)
	{
		int command = script->zasm[i].command;

		bool is_function_call_like = false;
		if (command == CALLFUNC)
		{
			is_function_call_like = true;
		}
		else if (command == STARTDESTRUCTOR)
		{
			function_start_pcs_set.insert(i);
			continue;
		}
		else if (command == RETURNFUNC && i + 1 < script->size)
		{
			int next_command = script->zasm[i + 1].command;
			if (next_command != 0xFFFF)
				function_start_pcs_set.insert(i + 1);
			continue;
		}
		else if (legacy_calling_mode && command == GOTO)
		{
			// Function calls are directly followed with a POP to restore the stack frame pointer.
			// PEEK is also possible via an optimization done by the compiler.
			int next_command = script->zasm[i + 1].command;
			is_function_call_like = (next_command == POP || next_command == PEEK) && script->zasm[i + 1].arg1 == D(4);
		}
		else
		{
			continue;
		}

		if (is_function_call_like && script->zasm[i].arg1 != -1)
		{
			function_calls.insert(i);
			function_start_pcs_set.insert(script->zasm[i].arg1);
			function_calls_pc_to_pc[i] = script->zasm[i].arg1;
		}
	}

	std::vector<pc_t> function_start_pcs(function_start_pcs_set.begin(), function_start_pcs_set.end());
	std::vector<pc_t> function_final_pcs;
	{
		start_pc_to_function[0] = 0;

		pc_t next_fn_id = 1;
		for (int i = 1; i < function_start_pcs.size(); i++)
		{
			pc_t function_start_pc = function_start_pcs[i];
			function_final_pcs.push_back(function_start_pc - 1);
			start_pc_to_function[function_start_pc] = next_fn_id++;
		}

		// Don't include 0xFFFF as part of the last function. Guard against an
		// empty script (no real instructions, just the 0xFFFF terminator), where
		// `size - 2` would underflow and produce an out-of-bounds final pc.
		if (script->size <= 1)
			function_final_pcs.push_back(0);
		else if (script->zasm.back().command == 0xFFFF)
			function_final_pcs.push_back(script->size - 2);
		else
			function_final_pcs.push_back(script->size - 1);

		// Just so std::lower_bound below will work for last function.
		function_start_pcs.push_back(script->size);
	}

	for (pc_t i = 0; i < function_final_pcs.size(); i++)
	{
		functions.push_back({i, "", function_start_pcs.at(i), function_final_pcs.at(i)});
	}

	for (auto [a, b] : function_calls_pc_to_pc)
	{
		auto it = std::lower_bound(function_start_pcs.begin(), function_start_pcs.end(), a);
		ASSERT(it != function_start_pcs.end());
		pc_t callee_pc = std::distance(function_start_pcs.begin(), it) - 1;

		it = std::lower_bound(function_start_pcs.begin(), function_start_pcs.end(), b);
		ASSERT(it != function_start_pcs.end());
		ASSERT(*it == b);
		pc_t call_pc = std::distance(function_start_pcs.begin(), it);

		functions.at(call_pc).called_by_functions.insert(callee_pc);
		// functions[callee_pc].calls_functions.insert(call_pc);
	}

	for (auto sd : script->script_datas)
	{
		auto& fn = functions.at(start_pc_to_function.at(sd->pc));
		fn._name = fmt::format("run_{}", sd->name());
		fn.is_entry_function = true;
	}

	return {functions, function_calls, start_pc_to_function, calling_mode};
}

std::set<pc_t> zasm_find_yielding_functions(const zasm_script* script, StructuredZasm& structured_zasm)
{
	std::set<pc_t> yielding_function_ids;
	for (const auto& fn : structured_zasm.functions)
	{
		for (pc_t i = fn.start_pc; i <= fn.final_pc; i++)
		{
			int command = script->zasm[i].command;
			if (command_is_suspend(command))
			{
				yielding_function_ids.insert(fn.id);
				break;
			}
		}
	}

	std::set<pc_t> seen_ids;
	std::set<pc_t> pending_ids = yielding_function_ids;
	while (pending_ids.size())
	{
		pc_t id = *pending_ids.begin();
		pending_ids.erase(pending_ids.begin());
		seen_ids.insert(id);
		structured_zasm.functions[id].may_yield = true;

		for (auto called_by_id : structured_zasm.functions[id].called_by_functions)
		{
			if (!seen_ids.contains(called_by_id))
				pending_ids.insert(called_by_id);
		}
	}

	return seen_ids;
}

static bool is_in_ranges(pc_t pc, const std::vector<std::pair<pc_t, pc_t>>& pc_ranges)
{
	// Fast path for common case.
	if (pc_ranges.size() == 1)
	{
		const auto& range = pc_ranges.front();
		return pc >= range.first && pc <= range.second;
	}

	for (auto [start_pc, final_pc] : pc_ranges)
	{
		if (pc >= start_pc && pc <= final_pc)
		{
			return true;
		}
	}

	return false;
}

bool ZasmCFG::contains_block_start(pc_t pc) const
{
	return std::binary_search(block_starts.begin(), block_starts.end(), pc);
}

pc_t ZasmCFG::block_id_from_start_pc(pc_t pc) const
{
	auto it = std::lower_bound(block_starts.begin(), block_starts.end(), pc);
	return std::distance(block_starts.begin(), it);
}

pc_t ZasmCFG::get_block_final(int block) const
{
	return block == block_starts.size() - 1 ?
		final_pc :
		block_starts.at(block + 1) - 1;
}

std::pair<pc_t, pc_t> ZasmCFG::get_block_bounds(int block) const
{
	return {block_starts.at(block), get_block_final(block)};
}

// Splits `pc_ranges` into blocks. A jump or call to a pc outside of `target_range` (when given;
// otherwise outside of pc_ranges) is ignored: it neither ends its block nor gets an edge (except
// a conditional jump, whose edge goes to whichever block id that pc sorts to). With a
// target_range, a target outside of pc_ranges still gets a block, with no edges of its own, and so
// does the instruction after a range (fallen through to) when target_range has it.
static ZasmCFG construct_cfg(const zasm_script* script, const std::vector<std::pair<pc_t, pc_t>>& pc_ranges, std::optional<std::pair<pc_t, pc_t>> target_range)
{
	auto is_target = [&](pc_t pc) {
		if (target_range)
			return pc >= target_range->first && pc <= target_range->second;
		return is_in_ranges(pc, pc_ranges);
	};

	ZasmCFG cfg{};

	cfg.final_pc = pc_ranges.back().second;

	// Reserve an amount proportional to the number of instructions.
	// Note: picked randomly, one could do more research here.
	auto& block_starts = cfg.block_starts;
	size_t num_instructions = 0;
	for (auto [start_pc, final_pc] : pc_ranges)
		num_instructions += final_pc - start_pc + 1;
	block_starts.reserve(num_instructions / 4);

	for (auto [start_pc, final_pc] : pc_ranges)
	{
		block_starts.push_back(start_pc);
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			int command = script->zasm[i].command;
			int arg1 = script->zasm[i].arg1;

			if (command == CALLFUNC || command == GOTO || command == GOTOCMP || command == GOTOTRUE || command == GOTOFALSE || command == GOTOLESS || command == GOTOMORE)
			{
				// Ignore GOTO jumps outside provided bounds.
				// This allows for creating a CFG that is internal to this function only.
				if (!is_target(arg1))
				{
					continue;
				}

				// This is a recursive function call to itself!
				if (arg1 == start_pc)
				{
					continue;
				}

				block_starts.push_back(arg1);
				if (i + 1 <= final_pc)
					block_starts.push_back(i + 1);
			}
			else if (command == GOTOTABLE || command == GOTORANGES)
			{
				for (pc_t target : zasm_jump_targets(command, script->literals.vec(script->zasm[i])))
				{
					if (is_target(target))
						block_starts.push_back(target);
				}
				if (i + 1 <= final_pc)
					block_starts.push_back(i + 1);
			}
			else if (command_is_suspend(command))
			{
				// A suspend point ends its block so execution can resume at the
				// next instruction.
				if (i + 1 <= final_pc)
					block_starts.push_back(i + 1);
			}
		}

		// The range may fall through to the instruction after it.
		if (target_range && is_target(final_pc + 1))
			block_starts.push_back(final_pc + 1);
	}

	// Sort and remove duplicates.
	std::sort(block_starts.begin(), block_starts.end());
	block_starts.erase(std::unique(block_starts.begin(), block_starts.end()), block_starts.end());
	size_t num_blocks = block_starts.size();

	auto& block_edges = cfg.block_edges;
	block_edges.resize(num_blocks);

	for (pc_t j = 1; j <= num_blocks; j++)
	{
		// A block outside of pc_ranges is just somewhere a jump lands.
		if (target_range && !is_in_ranges(block_starts[j - 1], pc_ranges))
			continue;

		auto& edges = block_edges[j - 1];
		edges.reserve(2);
		int i = j < num_blocks ? block_starts[j] - 1 : cfg.final_pc;
		int prev_command = script->zasm[i].command;
		int prev_arg1 = script->zasm[i].arg1;
		if (prev_command == GOTO || prev_command == CALLFUNC)
		{
			if (is_target(prev_arg1))
			{
				// Previous block unconditionally continues to some other block.
				auto other_block = cfg.block_id_from_start_pc(prev_arg1);
				edges.push_back(other_block);
			}
		}
		else if (prev_command == GOTOCMP || prev_command == GOTOTRUE || prev_command == GOTOFALSE || prev_command == GOTOLESS || prev_command == GOTOMORE)
		{
			// Previous block conditionally continues to this one, or some other block.
			edges.push_back(j);
			auto other_block = cfg.block_id_from_start_pc(prev_arg1);
			edges.push_back(other_block);
		}
		else if (prev_command == GOTOTABLE || prev_command == GOTORANGES)
		{
			// Previous block continues to one of the dispatch targets (default
			// included, no fallthrough). Several keys can share a target, so
			// dedupe the edges.
			for (pc_t target : zasm_jump_targets(prev_command, script->literals.vec(script->zasm[i])))
			{
				if (!is_target(target))
					continue;

				auto other_block = cfg.block_id_from_start_pc(target);
				if (std::find(edges.begin(), edges.end(), other_block) == edges.end())
					edges.push_back(other_block);
			}
		}
		else if (prev_command != QUIT && prev_command != RETURN && prev_command != RETURNFUNC && prev_command != GOTOR && prev_command != GAMEEXIT && j != num_blocks)
		{
			// Previous block unconditionally continues to this one.
			edges.push_back(j);
		}
	}

	return cfg;
}

ZasmCFG zasm_construct_cfg(const zasm_script* script, std::vector<std::pair<pc_t, pc_t>> pc_ranges)
{
	return construct_cfg(script, pc_ranges, std::nullopt);
}

ZasmCFG zasm_construct_function_cfg(const zasm_script* script, const StructuredZasm& structured_zasm, const ZasmFunction& fn)
{
	// Functions are contiguous, so a pc in any of their ranges is one in the script's range.
	std::pair<pc_t, pc_t> script_range = {structured_zasm.functions.front().start_pc, structured_zasm.functions.back().final_pc};
	return construct_cfg(script, {{fn.start_pc, fn.final_pc}}, script_range);
}

// Liveness: for every block of a CFG, which of the eight D registers hold a value that some later
// instruction may still read - `in` at the block's first instruction, `out` after its last. A
// register not in `out` is dead at the end of the block, so whatever value it holds there can be
// discarded.
//
// Two things consume this:
//
//  - The ZASM optimizer uses `out` to tell whether a comparison still has to leave its result in
//    D2 (and, behind -optimize-zasm-experimental, to remove writes nobody reads).
//  - The JIT's D-register cache uses it to decide what must be written back to ri->d[] at a block
//    boundary, a call, or a return (see jit_reg_cache_flush_policy). A register that is dead there
//    is dropped without being written back. So a register wrongly considered dead is a
//    miscompile, while one wrongly considered live only costs a store: the analysis must only
//    ever err toward live.
//
// Within a function this is textbook backward dataflow. Each block's transfer function is built
// from a forward scan of its instructions (a read counts unless the block already wrote that
// register), then
//
//     out = union of `in` over the block's successors
//     in  = transfer(out)
//
// is iterated to a fixpoint with a worklist: whenever a block's `in` changes, its predecessors are
// queued again. The sets are 8-bit masks, so each step is a few bitwise ops.
//
// Calls and returns need more, because ZASM D registers are global: a callee reads and writes the
// same ri->d[] as its caller, so values flow into a callee, through it untouched, and back out of it
// in any register (not only D2, the return value). Each function is summarized once for the whole
// script (ZasmFunctionLiveness), and then:
//
//  - A CALLFUNC is an ordinary instruction that continues to the instruction after it. It reads the
//    callee's live_in, and writes only the callee's must_write - a register the callee writes on
//    just some paths may still hold the caller's value after the call.
//  - A RETURNFUNC exposes the function's read_after_return: everything any of its callers may read
//    after calling it, before writing it. That always includes D2, the return value, even when no
//    caller reads it: the script debugger calls functions itself and reads the result from D2.
//
// zasm_analyze_function_liveness computes the summaries over the CFG of the whole script:
//
//  1. must_write: a forward "must" analysis (intersection at merges) from each function's entry to
//     its returns, with each call writing its callee's must_write. Every function starts out
//     writing everything, and that is iterated down to a fixpoint, which handles recursion.
//  2. live_in: the backward analysis, with returns exposing nothing and each call reading the `in`
//     of its callee's entry block (so a call is queued again when that changes). What a caller
//     reads after the call is accounted for at the call site instead.
//  3. read_after_return, and the liveness of every block: the backward analysis again, with calls
//     reading the live_in from (2), and returns exposing the union of what is live after every
//     call to their function - accumulated as those calls are evaluated.
//
// Keeping (2) apart from (3) keeps one caller's needs from leaking into every other caller through
// the callee's live_in.
//
// When control leaves a function other than by a call or a return - falling through into the next
// function, or jumping to another function's start - the function it enters returns on its behalf.
// So the entered function's read_after_return includes the left one's, and the left function is
// taken to write nothing for sure.
//
// A suspend serializes the whole register file, so for the JIT (suspend_uses_all_registers) a
// suspend - and a call into a function that may suspend - reads every register.
//
// Reads can also be implicit: an instruction's implicit_read_mask covers registers the command
// always touches, and register_dependency_mask_cache covers non-D arguments that read a D register
// (an indexed register like COMBODD reads rINDEX), so those stay live too.
//
// There are two entry points. zasm_analyze_function_liveness runs the steps above over the whole
// script, which also yields the liveness of every block. zasm_run_liveness_analysis runs the
// backward analysis over one function's CFG, given the summaries; the blocks of other functions
// it jumps or falls through to are not analyzed but fixed to a given live-in. The JIT uses the
// first at init and keeps only its per-function results (the summaries, and each function's entry
// live-in), then uses the second just before compiling each function, because keeping the whole
// script's CFG and liveness until its last function is compiled costs too much memory
// (d6cffc8ae5). That relies on the second giving the same `in` and `out` for the function's blocks
// as the first did - which it does, being the same analysis on the same blocks with the same
// values at every call, return and edge out of the function - and on control entering each
// function at its start; otherwise the JIT keeps the whole-script analysis
// (JittedScript::script_analysis). The optimizer unit tests check the agreement
// (analyze_liveness in zasm_optimize_test.cpp).
//
// https://en.wikipedia.org/wiki/Data-flow_analysis
// https://www.cs.cornell.edu/courses/cs4120/2022sp/notes.html?id=livevar
// https://www.cs.cmu.edu/afs/cs/academic/class/15745-s19/www/lectures/L5-Intro-to-Dataflow.pdf

namespace {

// A block's transfer function is a sequence of steps, in program order: the instructions between
// calls collapse into one gen/kill step, and each call is a step of its own. A Call reads its
// callee's live_in and writes its must_write. A CallReadsAll reads every register: its target is
// not a function, or (for the JIT) is one that may suspend.
struct LivenessStep
{
	enum Kind : uint8_t { GenKill, Call, CallReadsAll } kind;
	uint8_t gen, kill;
	// The function a call goes to, or -1 if its target is not the start of a function.
	int32_t callee;
};

struct LivenessGraph
{
	const ZasmCFG* cfg;
	std::vector<LivenessStep> steps;
	// Block b's steps are steps[step_start[b], step_start[b + 1]).
	std::vector<uint32_t> step_start;
	// Per block: whether it returns. Its steps stop at the RETURNFUNC.
	std::vector<bool> returns;
	// Per block: whether it may jump somewhere outside of the CFG, where anything may be read.
	std::vector<bool> leaves_cfg;
	// Per block: whether its live-in is fixed (not analyzed).
	std::vector<bool> fixed;
	// Per block: the function containing it, or -1.
	std::vector<int32_t> block_fn;
	// Per block: its successors are the CFG's edges (SUCC_CFG), none (SUCC_NONE), or this one
	// block - a block ending in a call continues after the call, not into the callee.
	std::vector<pc_t> single_succ;
	// Block b's predecessors are pred[pred_start[b], pred_start[b + 1]).
	std::vector<pc_t> pred;
	std::vector<uint32_t> pred_start;

	static constexpr pc_t SUCC_CFG = UINT32_MAX;
	static constexpr pc_t SUCC_NONE = UINT32_MAX - 1;

	size_t num_blocks() const
	{
		return step_start.size() - 1;
	}

	std::span<const pc_t> succ(pc_t b) const
	{
		if (single_succ[b] == SUCC_CFG)
			return cfg->block_edges[b];
		if (single_succ[b] == SUCC_NONE)
			return {};
		return {&single_succ[b], 1};
	}

	std::span<const pc_t> preds(pc_t b) const
	{
		return {pred.data() + pred_start[b], pred.data() + pred_start[b + 1]};
	}
};

}

static int32_t callee_of(const ffscript& instr, const StructuredZasm& structured_zasm)
{
	auto it = structured_zasm.start_pc_to_function.find(instr.arg1);
	return it == structured_zasm.start_pc_to_function.end() ? -1 : (int32_t)it->second;
}

static bool call_reads_all(int32_t callee, const StructuredZasm& structured_zasm, bool suspend_uses_all_registers)
{
	return callee == -1 || (suspend_uses_all_registers && structured_zasm.functions[callee].may_yield);
}

static void instruction_reads_writes(const ffscript& instr, bool suspend_uses_all_registers, uint8_t& reads, uint8_t& writes)
{
	auto& meta = command_meta_cache[instr.command];

	// An instruction's reads all happen before its writes, so collect the masks for every arg
	// first. Interleaving them per-arg would drop the read when one arg writes a register another
	// arg reads (e.g. READPODARRAYR D2 D2: arg1 writes D2, arg2 reads it).
	reads = meta.implicit_read_mask;
	writes = meta.implicit_write_mask;

	const int32_t* p_arg = &instr.arg1;
	for (int argn = 0; argn < 3; ++argn)
	{
		const auto& arg_info = meta.args[argn];
		if (!arg_info.is_reg) continue;

		int reg = p_arg[argn];
		if (reg < 8)
		{
			if (arg_info.reads)
				reads |= (1 << reg);
			if (arg_info.writes)
				writes |= (1 << reg);
		}

		reads |= register_dependency_mask_cache[reg];
	}

	if (suspend_uses_all_registers && command_is_suspend(instr.command))
		reads = 0xFF;
}

static LivenessGraph build_liveness_graph(const zasm_script* script, const ZasmCFG& cfg, const StructuredZasm& structured_zasm, bool suspend_uses_all_registers, const std::vector<std::optional<uint8_t>>* fixed_live_in)
{
	size_t num_blocks = cfg.block_starts.size();

	LivenessGraph g;
	g.cfg = &cfg;
	g.steps.reserve(num_blocks * 2);
	g.step_start.resize(num_blocks + 1);
	g.returns.resize(num_blocks);
	g.leaves_cfg.resize(num_blocks);
	g.fixed.resize(num_blocks);
	g.block_fn.resize(num_blocks);
	g.single_succ.resize(num_blocks, LivenessGraph::SUCC_CFG);

	// Blocks and functions are both sorted by pc, so walk them together.
	const auto& functions = structured_zasm.functions;
	size_t fn = 0;
	for (pc_t b = 0; b < num_blocks; b++)
	{
		pc_t block_start = cfg.block_starts[b];
		while (fn < functions.size() && functions[fn].final_pc < block_start)
			fn++;
		g.block_fn[b] = fn < functions.size() && functions[fn].start_pc <= block_start ? (int32_t)fn : -1;

		g.step_start[b] = g.steps.size();
		if (fixed_live_in && (*fixed_live_in)[b])
		{
			g.fixed[b] = true;
			g.single_succ[b] = LivenessGraph::SUCC_NONE;
			continue;
		}

		auto [start_pc, final_pc] = cfg.get_block_bounds(b);
		uint8_t gen = 0;
		uint8_t kill = 0;
		bool returns = false;
		for (pc_t i = start_pc; i <= final_pc; i++)
		{
			const auto& instr = script->zasm[i];
			if (instr.command == CALLFUNC)
			{
				g.steps.push_back({LivenessStep::GenKill, gen, kill, -1});
				gen = kill = 0;

				int32_t callee = callee_of(instr, structured_zasm);
				auto kind = call_reads_all(callee, structured_zasm, suspend_uses_all_registers) ? LivenessStep::CallReadsAll : LivenessStep::Call;
				g.steps.push_back({kind, 0, 0, callee});
				continue;
			}

			if (instr.command == RETURNFUNC)
			{
				// Nothing after it in this block runs.
				returns = true;
				break;
			}

			// A CFG made of only some functions has no block (or edge) for a jump out of them.
			if (command_is_goto(instr.command) && !cfg.contains_block_start(instr.arg1))
			{
				g.leaves_cfg[b] = true;
				if (instr.command == GOTO)
					break;
			}
			else if (instr.command == GOTOTABLE || instr.command == GOTORANGES)
			{
				for (pc_t target : zasm_jump_targets(instr.command, script->literals.vec(instr)))
				{
					if (!cfg.contains_block_start(target))
						g.leaves_cfg[b] = true;
				}
			}

			uint8_t reads, writes;
			instruction_reads_writes(instr, suspend_uses_all_registers, reads, writes);
			gen |= reads & ~kill;
			kill |= writes;
		}
		g.steps.push_back({LivenessStep::GenKill, gen, kill, -1});
		g.returns[b] = returns;

		if (returns)
		{
			g.single_succ[b] = LivenessGraph::SUCC_NONE;
		}
		else if (script->zasm[final_pc].command == CALLFUNC)
		{
			// The CFG's edge goes to the callee, but its effect is in the call's step. Execution
			// continues after the call.
			g.single_succ[b] = cfg.contains_block_start(final_pc + 1) ?
				cfg.block_id_from_start_pc(final_pc + 1) : LivenessGraph::SUCC_NONE;
		}
	}
	g.step_start[num_blocks] = g.steps.size();

	// Count each block's predecessors, then place them.
	g.pred_start.resize(num_blocks + 1);
	for (pc_t b = 0; b < num_blocks; b++)
	{
		for (pc_t s : g.succ(b))
			g.pred_start[s + 1]++;
	}
	for (pc_t b = 0; b < num_blocks; b++)
		g.pred_start[b + 1] += g.pred_start[b];
	g.pred.resize(g.pred_start[num_blocks]);
	std::vector<uint32_t> next_pred(g.pred_start.begin(), g.pred_start.end() - 1);
	for (pc_t b = 0; b < num_blocks; b++)
	{
		for (pc_t s : g.succ(b))
			g.pred[next_pred[s]++] = b;
	}

	return g;
}

// Applies block b's transfer function to `out`. For each call, on_call(callee, live after the call)
// is called first.
template <typename CalleeLiveIn, typename OnCall>
static uint8_t liveness_transfer(const LivenessGraph& g, pc_t b, uint8_t out, const ZasmFunctionLivenessList& fl, CalleeLiveIn callee_live_in, OnCall on_call)
{
	uint8_t live = out;
	for (uint32_t s = g.step_start[b + 1]; s-- > g.step_start[b];)
	{
		const auto& step = g.steps[s];
		switch (step.kind)
		{
			case LivenessStep::GenKill:
				live = step.gen | (live & ~step.kill);
				break;

			case LivenessStep::Call:
				on_call(step.callee, live);
				live = callee_live_in(step.callee) | (live & ~fl[step.callee].must_write);
				break;

			case LivenessStep::CallReadsAll:
				if (step.callee != -1)
					on_call(step.callee, live);
				live = 0xFF;
				break;
		}
	}
	return live;
}

// Step 1 of zasm_analyze_function_liveness: sets each function's must_write.
static void compute_must_write(const LivenessGraph& g, const StructuredZasm& structured_zasm, ZasmFunctionLivenessList& fl)
{
	size_t num_blocks = g.num_blocks();
	size_t num_fns = fl.size();

	// Nothing is written for sure on entry to a function, or to a block entered from another
	// function. A function that control leaves other than by a call or a return writes nothing for
	// sure (the function it enters returns on its behalf).
	std::vector<bool> enters(num_blocks);
	std::vector<bool> leaves(num_fns);
	for (pc_t b = 0; b < num_blocks; b++)
	{
		int32_t fn = g.block_fn[b];
		if (fn == -1 || g.cfg->block_starts[b] == structured_zasm.functions[fn].start_pc)
			enters[b] = true;
		for (pc_t s : g.succ(b))
		{
			if (g.block_fn[s] != fn)
			{
				enters[s] = true;
				if (fn != -1)
					leaves[fn] = true;
			}
		}
	}

	for (auto& f : fl)
		f.must_write = 0xFF;

	// Two nested fixpoints: the inner loop runs the blocks with the functions' current must_write
	// until nothing changes, then each function's must_write is taken from its returning blocks.
	// When that lowered one (a call to it writes less than assumed), the blocks run again.
	//
	// def_out: written for sure at the end of each block (at the RETURNFUNC, for a returning
	// block).
	std::vector<uint8_t> def_out(num_blocks, 0xFF);
	std::vector<uint8_t> must_write(num_fns);
	while (true)
	{
		bool changed = true;
		while (changed)
		{
			changed = false;
			for (pc_t b = 0; b < num_blocks; b++)
			{
				uint8_t def = 0;
				if (!enters[b])
				{
					def = 0xFF;
					for (pc_t p : g.preds(b))
						def &= def_out[p];
				}

				for (uint32_t s = g.step_start[b]; s < g.step_start[b + 1]; s++)
				{
					const auto& step = g.steps[s];
					if (step.kind == LivenessStep::GenKill)
						def |= step.kill;
					else if (step.callee != -1)
						def |= fl[step.callee].must_write;
				}

				if (def != def_out[b])
				{
					def_out[b] = def;
					changed = true;
				}
			}
		}

		std::fill(must_write.begin(), must_write.end(), 0xFF);
		for (pc_t b = 0; b < num_blocks; b++)
		{
			if (g.returns[b] && g.block_fn[b] != -1)
				must_write[g.block_fn[b]] &= def_out[b];
		}

		bool fl_changed = false;
		for (size_t fn = 0; fn < num_fns; fn++)
		{
			uint8_t value = leaves[fn] ? 0 : must_write[fn];
			if (value != fl[fn].must_write)
			{
				fl[fn].must_write = value;
				fl_changed = true;
			}
		}
		if (!fl_changed)
			break;
	}
}

namespace {

enum class LivenessMode
{
	// Calls read the summaries' live_in; returns expose their read_after_return.
	Summaries,
	// Step 2 of zasm_analyze_function_liveness: calls read the `in` of their callee's entry block;
	// returns expose nothing.
	ComputeLiveIn,
	// Step 3 of zasm_analyze_function_liveness: like Summaries, but what returns expose is
	// accumulated (into read_after_return) from what is live after each call.
	ComputeReadAfterReturn,
};

}

// Runs the backward analysis over every block of `g`. In ComputeReadAfterReturn mode,
// *read_after_return (indexed by function) accumulates what is live after each call to the
// function, so the caller must initialize it with what is read after every return regardless of
// callers (D2, for the debugger), and takes the result from it.
static ZasmLiveness run_backward_liveness(const LivenessGraph& g, const StructuredZasm& structured_zasm, const std::vector<std::optional<uint8_t>>* fixed_live_in, const ZasmFunctionLivenessList& fl, LivenessMode mode, std::vector<uint8_t>* read_after_return = nullptr)
{
	size_t num_blocks = g.num_blocks();
	size_t num_fns = fl.size();

	ZasmLiveness vars(num_blocks, {0, 0});
	for (pc_t b = 0; b < num_blocks; b++)
	{
		if (g.fixed[b])
			vars[b].in = *(*fixed_live_in)[b];
	}

	std::vector<pc_t> worklist;
	worklist.reserve(num_blocks);
	std::vector<bool> in_worklist(num_blocks);
	auto queue = [&](pc_t b){
		if (!in_worklist[b] && !g.fixed[b])
		{
			worklist.push_back(b);
			in_worklist[b] = true;
		}
	};

	// ComputeLiveIn: each function's entry block, and the blocks calling it (which depend on it).
	std::vector<int32_t> entry_block;
	std::vector<std::vector<pc_t>> call_blocks;
	if (mode == LivenessMode::ComputeLiveIn)
	{
		entry_block.resize(num_fns, -1);
		for (size_t fn = 0; fn < num_fns; fn++)
		{
			pc_t start_pc = structured_zasm.functions[fn].start_pc;
			if (g.cfg->contains_block_start(start_pc))
				entry_block[fn] = g.cfg->block_id_from_start_pc(start_pc);
		}

		call_blocks.resize(num_fns);
		for (pc_t b = 0; b < num_blocks; b++)
		{
			for (uint32_t s = g.step_start[b]; s < g.step_start[b + 1]; s++)
			{
				if (g.steps[s].kind == LivenessStep::Call)
					call_blocks[g.steps[s].callee].push_back(b);
			}
		}
	}

	// ComputeReadAfterReturn: each function's returning blocks, and the functions it enters other
	// than by a call (which return on its behalf).
	std::vector<std::vector<pc_t>> return_blocks;
	std::vector<std::vector<int32_t>> enters_functions;
	std::vector<std::pair<int32_t, uint8_t>> rar_work;
	if (mode == LivenessMode::ComputeReadAfterReturn)
	{
		return_blocks.resize(num_fns);
		enters_functions.resize(num_fns);
		for (pc_t b = 0; b < num_blocks; b++)
		{
			int32_t fn = g.block_fn[b];
			if (fn == -1)
				continue;

			if (g.returns[b])
				return_blocks[fn].push_back(b);
			for (pc_t s : g.succ(b))
			{
				int32_t entered = g.block_fn[s];
				if (entered != -1 && entered != fn && !util::contains(enters_functions[fn], entered))
					enters_functions[fn].push_back(entered);
			}
		}
	}

	auto add_read_after_return = [&](int32_t fn, uint8_t mask){
		rar_work.push_back({fn, mask});
		while (!rar_work.empty())
		{
			auto [fn, mask] = rar_work.back();
			rar_work.pop_back();

			uint8_t& rar = (*read_after_return)[fn];
			if ((rar | mask) == rar)
				continue;

			rar |= mask;
			for (pc_t b : return_blocks[fn])
				queue(b);
			for (int32_t entered : enters_functions[fn])
				rar_work.push_back({entered, rar});
		}
	};

	auto callee_live_in = [&](int32_t callee) -> uint8_t {
		if (mode != LivenessMode::ComputeLiveIn)
			return fl[callee].live_in;
		return entry_block[callee] == -1 ? 0xFF : vars[entry_block[callee]].in;
	};

	auto on_call = [&](int32_t callee, uint8_t live_after){
		if (mode == LivenessMode::ComputeReadAfterReturn)
			add_read_after_return(callee, live_after);
	};

	// Process the blocks last to first the first time through, which suits a backward analysis.
	for (pc_t b = 0; b < num_blocks; b++)
		queue(b);

	while (!worklist.empty())
	{
		pc_t b = worklist.back();
		worklist.pop_back();
		in_worklist[b] = false;

		uint8_t out = 0;
		if (g.returns[b])
		{
			int32_t fn = g.block_fn[b];
			if (fn == -1)
				out = 0xFF;
			else if (mode == LivenessMode::Summaries)
				out = fl[fn].read_after_return;
			else if (mode == LivenessMode::ComputeReadAfterReturn)
				out = (*read_after_return)[fn];
		}
		else
		{
			for (pc_t s : g.succ(b))
				out |= vars[s].in;
		}
		if (g.leaves_cfg[b])
			out = 0xFF;
		vars[b].out = out;

		uint8_t in = liveness_transfer(g, b, out, fl, callee_live_in, on_call);
		if (in == vars[b].in)
			continue;

		vars[b].in = in;
		for (pc_t p : g.preds(b))
			queue(p);

		if (mode == LivenessMode::ComputeLiveIn)
		{
			int32_t fn = g.block_fn[b];
			if (fn != -1 && entry_block[fn] == (int32_t)b)
			{
				for (pc_t c : call_blocks[fn])
					queue(c);
			}
		}
	}

	return vars;
}

ZasmFunctionLivenessList zasm_analyze_function_liveness(const zasm_script* script, const ZasmCFG& script_cfg, const StructuredZasm& structured_zasm, bool suspend_uses_all_registers, ZasmLiveness* script_liveness)
{
	ZasmFunctionLivenessList fl(structured_zasm.functions.size());
	auto g = build_liveness_graph(script, script_cfg, structured_zasm, suspend_uses_all_registers, nullptr);

	compute_must_write(g, structured_zasm, fl);

	auto vars = run_backward_liveness(g, structured_zasm, nullptr, fl, LivenessMode::ComputeLiveIn);
	for (size_t fn = 0; fn < fl.size(); fn++)
	{
		pc_t start_pc = structured_zasm.functions[fn].start_pc;
		fl[fn].live_in = script_cfg.contains_block_start(start_pc) ? vars[script_cfg.block_id_from_start_pc(start_pc)].in : 0xFF;
	}

	// The debugger reads the return value of a function it calls (see VM::executeSandboxed).
	std::vector<uint8_t> read_after_return(fl.size(), 1 << D(2));
	vars = run_backward_liveness(g, structured_zasm, nullptr, fl, LivenessMode::ComputeReadAfterReturn, &read_after_return);
	for (size_t fn = 0; fn < fl.size(); fn++)
		fl[fn].read_after_return = read_after_return[fn];
	if (script_liveness)
		*script_liveness = std::move(vars);

	return fl;
}

ZasmLiveness zasm_run_liveness_analysis(const zasm_script* script, const ZasmCFG& cfg, const StructuredZasm& structured_zasm, const ZasmFunctionLivenessList& function_liveness, bool suspend_uses_all_registers, const std::vector<std::optional<uint8_t>>* fixed_live_in)
{
	auto g = build_liveness_graph(script, cfg, structured_zasm, suspend_uses_all_registers, fixed_live_in);
	return run_backward_liveness(g, structured_zasm, fixed_live_in, function_liveness, LivenessMode::Summaries);
}

uint8_t zasm_live_before(const ffscript& instr, uint8_t live_after, const StructuredZasm& structured_zasm, const ZasmFunctionLivenessList& function_liveness, bool suspend_uses_all_registers)
{
	if (instr.command == CALLFUNC)
	{
		int32_t callee = callee_of(instr, structured_zasm);
		if (call_reads_all(callee, structured_zasm, suspend_uses_all_registers))
			return 0xFF;

		const auto& f = function_liveness[callee];
		return f.live_in | (live_after & ~f.must_write);
	}

	uint8_t reads, writes;
	instruction_reads_writes(instr, suspend_uses_all_registers, reads, writes);
	return reads | (live_after & ~writes);
}

static std::string zasm_fn_get_label(const ZasmFunction& function)
{
	if (function._name.empty())
		return fmt::format("Function #{}", function.id);
	else
		return function.signature();
}

static std::string zasm_to_string(const zasm_script* script, const StructuredZasm& structured_zasm, const ZasmCFG& cfg, std::set<pc_t> function_ids)
{
	std::stringstream ss;

	int block_id = 0;
	for (auto fn_id : function_ids)
	{
		auto& function = structured_zasm.functions[fn_id];

		for (pc_t i = function.start_pc; i <= function.final_pc; i++)
		{
			const auto& op = script->zasm[i];
			pc_t command = op.command;
			std::string str = zasm_op_to_string(op, script->literals);

			// A long op (like a jump table) still gets a space before its annotations.
			std::stringstream line_ss;
			line_ss <<
				std::setw(5) << std::right << i << ": " <<
				std::left << std::setw(44) << str << ' ';

			if (cfg.contains_block_start(i))
			{
				auto& edges = cfg.block_edges[block_id];
				line_ss << fmt::format("[Block {} -> {}]", block_id++, fmt::join(edges, ", "));
			}
			if ((command == GOTO && structured_zasm.start_pc_to_function.contains(op.arg1)) || command == CALLFUNC)
			{
				line_ss << fmt::format("[{}]", zasm_fn_get_label(structured_zasm.functions[structured_zasm.start_pc_to_function.at(op.arg1)]));
			}

			std::string line_content = line_ss.str();
			ss << line_content;

			if (zasm_debug_data.exists())
			{
				auto [source_file, line] = zasm_debug_data.resolveLocation(i);
				int current_len = line_content.length();
				int padding = std::max(1, 95 - current_len);
				ss << std::string(padding, ' ') << fmt::format("| {}:{}", source_file, line);
			}
			ss << '\n';
		}
		ss << '\n';
	}

	return ss.str();
}

static size_t count_non_nop_instructions(const zasm_script* script, size_t start_pc, size_t final_pc)
{
	size_t count = 0;
	for (size_t pc = start_pc; pc <= final_pc; pc++)
	{
		if (script->zasm[pc].command != NOP)
			count++;
	}
	return count;
}

std::string zasm_to_string(const zasm_script* script, bool top_functions, bool generate_yielder)
{
	std::stringstream ss;

	auto structured_zasm = zasm_construct_structured(script);

	// fn id, length
	std::vector<std::pair<pc_t, size_t>> fn_lengths;
	// fn id, start_pc, len
	std::vector<std::tuple<pc_t, pc_t, size_t>> block_lengths;

	std::set<pc_t> yielding_fns;
	size_t yielding_fn_length = 0;
	if (generate_yielder)
	{
		yielding_fns = zasm_find_yielding_functions(script, structured_zasm);
	}
	if (!yielding_fns.empty())
	{
		std::vector<std::pair<pc_t, pc_t>> pc_ranges;
		for (auto fn_id : yielding_fns)
		{
			auto& fn = structured_zasm.functions[fn_id];
			pc_ranges.emplace_back(fn.start_pc, fn.final_pc);
			yielding_fn_length += count_non_nop_instructions(script, fn.start_pc, fn.final_pc);
		}
		auto cfg = zasm_construct_cfg(script, pc_ranges);
		ss << "yielder" << '\n';
		ss << zasm_to_string(script, structured_zasm, cfg, yielding_fns);
		ss << '\n';

		fn_lengths.emplace_back(-1, yielding_fn_length);
	}

	for (pc_t fn_id = 0; fn_id < structured_zasm.functions.size(); fn_id++)
	{
		if (yielding_fns.contains(fn_id))
			continue;

		auto& fn = structured_zasm.functions[fn_id];
		auto cfg = zasm_construct_cfg(script, {{fn.start_pc, fn.final_pc}});
		ss << "Function " << zasm_fn_get_label(fn) << '\n';
		ss << zasm_to_string(script, structured_zasm, cfg, {fn_id});
		ss << '\n';

		fn_lengths.emplace_back(fn_id, count_non_nop_instructions(script, fn.start_pc, fn.final_pc));

		auto block_starts = std::vector<pc_t>(cfg.block_starts.begin(), cfg.block_starts.end());
		for (pc_t i = 0; i < cfg.block_starts.size(); i++)
		{
			pc_t start_pc = block_starts[i];
			pc_t final_pc = i == block_starts.size() - 1 ?
				fn.final_pc :
				block_starts[i + 1] - 1;
			size_t len = count_non_nop_instructions(script, start_pc, final_pc);
			block_lengths.emplace_back(fn_id, start_pc, len);
		}
	}

	if (top_functions)
	{
		ss << "Top functions:\n\n";
		std::sort(fn_lengths.begin(), fn_lengths.end(), [](auto &left, auto &right) {
			return left.second > right.second;
		});
		int lengths_printed = 0;
		for (auto [fn_id, length] : fn_lengths)
		{
			std::string name = fn_id == -1 ? "yielder" : zasm_fn_get_label(structured_zasm.functions.at(fn_id));
			double percent = (double)length / script->size * 100;
			ss << std::setw(15) << std::left << name + ": " << std::setw(6) << std::left << length << " " << (int)percent << '%' << '\n';
			if (++lengths_printed == 5) break;
		}
		ss << '\n';

		ss << "Top blocks:\n\n";
		std::sort(block_lengths.begin(), block_lengths.end(), [](auto &left, auto &right) {
			return std::get<2>(left) > std::get<2>(right);
		});
		lengths_printed = 0;
		for (auto [fn_id, start_pc, length] : block_lengths)
		{
			std::string name = fn_id == -1 ? "yielder" : zasm_fn_get_label(structured_zasm.functions.at(fn_id));
			name += fmt::format(" #{}", start_pc);
			double percent = (double)length / script->size * 100;
			ss << std::setw(15) << std::left << name + ": " << std::setw(6) << std::left << length << " " << (int)percent << '%' << '\n';
			if (++lengths_printed == 5) break;
		}
		ss << '\n';
	}

	return ss.str();
}

struct xxh3_state_deleter
{
	void operator()(XXH3_state_t* state) const { XXH3_freeState(state); }
};

uint64_t zasm_scripts_hash()
{
	// The instructions are split into fixed-size chunks that are hashed on
	// multiple threads, then the per-chunk hashes are folded together in order.
	// The chunk boundaries depend only on how many instructions each script has,
	// so the result is still stable across loads of the same scripts.
	static constexpr size_t chunk_length = 16 * 1024;

	struct chunk
	{
		const zasm_script* script;
		size_t start, end;
		uint64_t hash;
	};

	std::vector<chunk> chunks;
	for (auto& script : zasm_scripts)
	{
		size_t size = script->zasm.size();
		for (size_t start = 0; start < size; start += chunk_length)
			chunks.push_back({script.get(), start, std::min(start + chunk_length, size), 0});
		// Scripts with no instructions still contribute their name.
		if (!size)
			chunks.push_back({script.get(), 0, 0, 0});
	}

	auto hash_chunk = [](chunk& c) {
		// Instructions are staged into a small buffer and hashed a few thousand
		// at a time. Updating the hash with each instruction's 16 bytes on its own
		// is several times slower, and gathering the whole chunk before hashing it
		// is slower still - the buffer wants to stay in cache. Both the buffer and
		// the hash state are kept per thread because quests have thousands of
		// small scripts, and an allocation per chunk costs more than the hashing.
		static constexpr size_t batch_length = 4096;
		static thread_local std::vector<int32_t> buffer;
		static thread_local std::unique_ptr<XXH3_state_t, xxh3_state_deleter> state_holder(XXH3_createState());
		XXH3_state_t* state = state_holder.get();
		XXH3_64bits_reset(state);
		buffer.clear();
		buffer.reserve(batch_length + 8);

		auto flush = [&]() {
			if (!buffer.empty())
			{
				XXH3_64bits_update(state, buffer.data(), buffer.size() * sizeof(int32_t));
				buffer.clear();
			}
		};

		if (c.start == 0)
			XXH3_64bits_update(state, c.script->name.data(), c.script->name.size());

		for (size_t i = c.start; i < c.end; i++)
		{
			const ffscript& instr = c.script->zasm[i];
			buffer.push_back(instr.command);
			buffer.push_back(instr.arg1);
			buffer.push_back(instr.arg2);
			buffer.push_back(instr.arg3);
			if (auto vec = c.script->literals.vec(instr))
				buffer.insert(buffer.end(), vec->begin(), vec->end());
			if (auto str = c.script->literals.str(instr))
			{
				flush();
				XXH3_64bits_update(state, str->data(), str->size());
			}
			if (buffer.size() >= batch_length)
				flush();
		}
		flush();

		c.hash = XXH3_64bits_digest(state);
	};

	// TODO: debug issues on web build.
	if (!is_web())
		std::for_each(std::execution::par_unseq, chunks.begin(), chunks.end(), hash_chunk);
	else
		std::for_each(std::execution::seq, chunks.begin(), chunks.end(), hash_chunk);

	std::vector<uint64_t> hashes;
	hashes.reserve(chunks.size());
	for (const chunk& c : chunks)
		hashes.push_back(c.hash);
	return XXH3_64bits(hashes.data(), hashes.size() * sizeof(uint64_t));
}

void zasm_for_every_script(bool parallel, std::function<void(zasm_script*)> fn)
{
	std::vector<zasm_script*> scripts;
	scripts.reserve(zasm_scripts.size());
	for (auto& script : zasm_scripts)
		if (script->valid())
			scripts.push_back(script.get());

	// TODO: debug issues on web build.
	if (parallel && !is_web())
		std::for_each(std::execution::par_unseq, scripts.begin(), scripts.end(), fn);
	else
		std::for_each(std::execution::seq, scripts.begin(), scripts.end(), fn);
}

// TODO: Broken / not yet needed code for determining how many params a function has, and if it returns something.
// size_t entry_user_fn = -1;
// for (int fid = 0; fid < function_start_pcs.size(); fid++)
// {
// 	size_t start_pc = function_start_pcs[fid];
// 	size_t last_pc = function_last_pcs[fid];

// 	printf("fn: %zu - %zu\n", start_pc, last_pc);
// 	int num_params_v1 = 0;
// 	int max = std::min(script->size, last_pc);
// 	for (int i = start_pc; i < max; i++)
// 	{
// 		int command = script->zasm[i].command;
// 		int arg1 = script->zasm[i].arg1;
// 		if (command != LOADD || arg1 != D(2))
// 			continue;

// 		int arg2 = script->zasm[i].arg2;
// 		num_params_v1 = std::max(num_params_v1, arg2 / 10000);
// 	}

// 	int num_params_v2 = 0;
// 	max = std::min(script->size - 1, last_pc);
// 	for (int i = start_pc; i < max; i++)
// 	{
// 		if (script->zasm[i].command != POPARGS || (script->zasm[i + 1].command != RETURN && script->zasm[i + 1].command != QUIT))
// 			continue;

// 		int arg2 = script->zasm[i].arg2;
// 		num_params_v2 = arg2 - 1;
// 		break;
// 	}

// 	printf("num_params_v1: %d\n", num_params_v1);
// 	printf("num_params_v2: %d\n", num_params_v2);
// 	if (num_params_v1 != num_params_v2)
// 		printf("SAD\n");

// 	bool has_ret_value = false;
// 	max = std::min(script->size - 1, last_pc);
// 	for (int i = start_pc; i < max; i++)
// 	{
// 		int command = script->zasm[i].command;
// 		int arg1 = script->zasm[i].arg1;
// 		if (command != SETV || arg1 != D(2))
// 			continue;

// 		i++;
// 		command = script->zasm[i].command;
// 		arg1 = script->zasm[i].arg1;
// 		if (command != GOTO || arg1 >= last_pc)
// 			continue;

// 		int j = arg1;
// 		if (j >= script->size)
// 			continue;

// 		if (script->zasm[j].command == POPARGS && (script->zasm[j + 1].command == RETURN || script->zasm[j + 1].command == QUIT))
// 		{
// 			has_ret_value = true;
// 			break;
// 		}
// 	}

// 	printf("has_ret_val: %d\n", has_ret_value?1:0);

// 	int num_returns = has_ret_value?1:0;
// 	int num_params = num_params_v1;

std::pair<bool, bool> get_command_rw(int command, int arg)
{
	bool read = false;
	bool write = false;

	auto sc = get_script_command(command);
	if (sc->args >= arg)
	{
		if (sc->arg_type[arg] == ARGTY::READ_REG)
			read = true;
		if (sc->arg_type[arg] == ARGTY::WRITE_REG)
			write = true;
		if (sc->arg_type[arg] == ARGTY::READWRITE_REG)
			read = write = true;
	}
	
	return {read, write};
}
