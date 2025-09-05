#ifndef ZASM_UTILS_H_
#define ZASM_UTILS_H_

#include "base/zdefs.h"
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>

typedef uint32_t pc_t;

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

	bool contains_block_start(pc_t pc) const;
	pc_t block_id_from_start_pc(pc_t pc) const;
};

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

std::string zasm_to_string(const zasm_script* script, bool top_functions = false, bool generate_yielder = false);

// For older quests, this runs over multiple threads if `parallel` is true.
// Work is never parallelized for newer quests (3.0+) since they have just a single zasm_script.
void zasm_for_every_script(bool parallel, std::function<void(zasm_script*)> zasm_script);

std::pair<bool, bool> get_command_rw(int command, int arg);

#endif
