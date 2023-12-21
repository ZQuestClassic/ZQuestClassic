#ifndef _ZASM_UTILS_H_
#define _ZASM_UTILS_H_

#include "base/zdefs.h"
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

typedef uint32_t pc_t;

struct ZasmFunction
{
	pc_t id;
	std::string name;
	pc_t start_pc;
	pc_t final_pc;
	bool may_yield;
	std::set<pc_t> called_by_functions;
};

struct StructuredZasm
{
	std::vector<ZasmFunction> functions;
	std::set<pc_t> function_calls;
	std::map<pc_t, pc_t> start_pc_to_function;
};

struct ZasmCFG
{
	std::set<pc_t> block_starts;
	std::map<pc_t, pc_t> start_pc_to_block_id;
	// Block id => vec of block ids it can go to
	std::vector<std::vector<pc_t>> block_edges;
};

// Given a ZASM script, discover all functions within (including function names, start/end pcs,
// and function calls).
// ZasmFunction `may_yield` will not be set until `zasm_find_yielding_functions` is called.
// Pass specific ranges from a ZasmFunction to `zasm_construct_cfg` to generate a
// control flow graph.
// TODO: we don't actually have function names in the ZASM, so they are generated for now.
StructuredZasm zasm_construct_structured(const script_data* script);

// Returns the function id of every function that itself uses `WaitX`, or calls some function that
// may possibly use `WaitX`.
// Modifies given StructuredZasm by setting `may_yield`.
std::set<pc_t> zasm_find_yielding_functions(const script_data* script, StructuredZasm& structured_zasm);

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
ZasmCFG zasm_construct_cfg(const script_data* script, std::vector<std::pair<pc_t, pc_t>> pc_ranges);

std::string zasm_to_string(const script_data* script, bool generate_yielder = false);

std::string zasm_script_unique_name(const script_data* script);

#endif
