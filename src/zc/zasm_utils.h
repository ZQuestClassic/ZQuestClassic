#ifndef ZASM_UTILS_H_
#define ZASM_UTILS_H_

#include "core/qst.h"
#include "core/zdefs.h"
#include "components/zasm/pc.h"
#include <map>
#include <optional>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>

struct CommandArgumentInfo
{
	bool is_reg = false;
	bool reads = false;
	bool writes = false;
};

struct CommandMetadata
{
	uint8_t implicit_read_mask = 0;
	uint8_t implicit_write_mask = 0;
	CommandArgumentInfo args[3];
};

void zasm_init_meta_cache();
const std::vector<CommandMetadata>& zasm_get_command_meta_cache();
const std::vector<uint8_t>& zasm_get_register_dependency_mask_cache();

struct ZasmFunction
{
	pc_t id;
	std::string _name;
	pc_t start_pc;
	pc_t final_pc;
	bool may_yield;
	bool is_entry_function;
	std::set<pc_t> called_by_functions;
	// Currently nothing needs this.
	// std::set<pc_t> calls_functions;

	const std::string& name() const
	{
		static std::string empty = "<empty>";
		if (_name.empty())
			return empty;
		else
			return _name;
	}

	std::string signature() const
	{
		if (zasm_debug_data.exists())
			return zasm_debug_data.getFunctionSignature(zasm_debug_data.resolveFunctionScope(start_pc));
		return name();
	}
};

struct StructuredZasm
{
	std::vector<ZasmFunction> functions;
	std::set<pc_t> function_calls;
	std::map<pc_t, pc_t> start_pc_to_function;
	enum CallingMode {
		CALLING_MODE_UNKNOWN,
		CALLING_MODE_GOTO_GOTOR,
		CALLING_MODE_GOTO_RETURN,
		CALLING_MODE_CALLFUNC_RETURNFUNC,
	} calling_mode;

	bool is_modern_function_calling();
};

struct ZasmCFG
{
	std::vector<pc_t> block_starts;
	// Block id => vec of block ids it can go to
	std::vector<std::vector<pc_t>> block_edges;
	pc_t final_pc;

	bool contains_block_start(pc_t pc) const;
	pc_t block_id_from_start_pc(pc_t pc) const;
	pc_t get_block_final(int block) const;
	std::pair<pc_t, pc_t> get_block_bounds(int block) const;
};

struct ZasmBlockVars
{
	uint8_t in, out;
};

using ZasmLiveness = std::vector<ZasmBlockVars>;

// What the liveness analysis knows about a function, as seen from its call sites and returns.
struct ZasmFunctionLiveness
{
	// The D registers the function (or a function it calls) may read before writing them. Not
	// counting what is read after it returns.
	uint8_t live_in;
	// The D registers written on every path from the function's entry to a return.
	uint8_t must_write;
	// The D registers that may be read after the function returns, before being written - by a
	// caller, or by the callers of that caller. Always includes D2, the return value.
	uint8_t read_after_return;
};

// Indexed by function id.
using ZasmFunctionLivenessList = std::vector<ZasmFunctionLiveness>;

// Given a ZASM script, discover all functions within (including function names, start/end pcs,
// and function calls).
// ZasmFunction `may_yield` will not be set until `zasm_find_yielding_functions` is called.
// Pass specific ranges from a ZasmFunction to `zasm_construct_cfg` to generate a
// control flow graph.
// TODO: we don't actually have function names in the ZASM, so they are generated for now.
StructuredZasm zasm_construct_structured(const zasm_script* script);

// Returns the function id of every function that itself uses `WaitX`, or calls some function that
// may possibly use `WaitX`.
// Modifies given StructuredZasm by setting `may_yield`.
std::set<pc_t> zasm_find_yielding_functions(const zasm_script* script, StructuredZasm& structured_zasm);

// Returns a control flow graph - splits the given range into basic blocks (which only possibly transfers
// to another block as their last instruction), and marks all possible blocks each block may transfer
// control to.
//
// For example:
//
// An "if" control flow will produce a basic block that goes to one of two other blocks - either following
// its "if" condition being true and transfering controll to the corresponding block, or by failing that
// condition and transfering control to the next block by "falling through" (doing nothing, really).
//
// The resulting control flow graph should be reducible, which basically means it does not contain
// any loops with more than one entrypoint. That only happens with:
//   1) unstructured control flow constructs like goto
//   2) intense compiler optimizations manipulating the control flow
// Neither should be the case for the ZASM we observe from the zscript compiler, which greatly simplifies things
// as we don't need to convert the graph to be reducible.
//
// https://www2.cs.arizona.edu/~collberg/Teaching/453/2009/Handouts/Handout-15.pdf
ZasmCFG zasm_construct_cfg(const zasm_script* script, std::vector<std::pair<pc_t, pc_t>> pc_ranges);

// Returns the control flow graph of one function, with the same blocks and edges the function has
// in the CFG of every function in its script. Control that leaves the function (a call, a jump,
// or falling through into the next function) still ends a block and has an edge, to an extra
// block for the pc it goes to - one with no edges of its own. Give those blocks their live-in to
// zasm_run_liveness_analysis (fixed_live_in).
//
// This only matches the script's CFG if control never enters a function anywhere but its start.
ZasmCFG zasm_construct_function_cfg(const zasm_script* script, const struct StructuredZasm& structured_zasm, const ZasmFunction& fn);

// Computes what the liveness analysis needs to know about each function of a script, from the
// CFG of the whole script (one range per function, see zasm_construct_cfg). If script_liveness is
// given, it is set to the liveness of every block of script_cfg.
//
// When suspend_uses_all_registers is set, a suspend point (WaitX/RUNGENFRZSCR) is treated as
// reading every D register, and so is a call into a function that may_yield (its may_yield flag
// must already be populated). The JIT needs this because a suspend serializes the whole register
// file to ri->d[]; the optimizer does not.
ZasmFunctionLivenessList zasm_analyze_function_liveness(const zasm_script* script, const ZasmCFG& script_cfg, const StructuredZasm& structured_zasm, bool suspend_uses_all_registers, ZasmLiveness* script_liveness = nullptr);

// Computes, for every block of `cfg`, which D registers may be read before being written after
// the start (in) and the end (out) of the block. `function_liveness` must come from
// zasm_analyze_function_liveness, given the same suspend_uses_all_registers.
//
// Blocks with a value in fixed_live_in (indexed by block) are not analyzed - their live-in is
// that value. See zasm_construct_function_cfg.
ZasmLiveness zasm_run_liveness_analysis(const zasm_script* script, const ZasmCFG& cfg, const StructuredZasm& structured_zasm, const ZasmFunctionLivenessList& function_liveness, bool suspend_uses_all_registers = false, const std::vector<std::optional<uint8_t>>* fixed_live_in = nullptr);

// The D registers live just before `instr`, given the ones live just after it: the analysis's
// transfer function for one instruction, for code that walks a block's instructions itself (the
// JIT's register cache flush policy, the optimizer's dead code pass) so that it agrees with the
// analysis at calls.
uint8_t zasm_live_before(const ffscript& instr, uint8_t live_after, const StructuredZasm& structured_zasm, const ZasmFunctionLivenessList& function_liveness, bool suspend_uses_all_registers);

std::string zasm_to_string(const zasm_script* script, bool top_functions = false, bool generate_yielder = false);

// For older quests, this runs over multiple threads if `parallel` is true.
// Work is never parallelized for newer quests (3.0+) since they have just a single zasm_script.
void zasm_for_every_script(bool parallel, std::function<void(zasm_script*)> zasm_script);

// Hash of every currently loaded zasm script's contents (commands, args, and
// attached vector/string literals). Detects whether the scripts actually
// changed from one quest load to the next.
uint64_t zasm_scripts_hash();

std::pair<bool, bool> get_command_rw(int command, int arg);

#endif
