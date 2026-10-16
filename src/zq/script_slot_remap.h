#ifndef ZQ_SCRIPT_SLOT_REMAP_H_
#define ZQ_SCRIPT_SLOT_REMAP_H_

#include "base/containers.h"
#include "base/general.h"
#include "base/ints.h"
#include "components/zasm/inst_var_remap.h"

#include <array>
#include <functional>
#include <map>
#include <string>
#include <vector>

struct DebugData;
struct script_data;

// When scripts are recompiled, moves the values set for each script's
// exported variables (on placed FFCs, combos, items, ...) to follow their
// variables. See components/zasm/inst_var_remap.h.

struct SlotRemap
{
	script_data** scripts = nullptr; // The slot array, ex. `ffscripts`.
	int slot = 0;
	std::string slot_type_name;
	std::string script_name;
	InstVarRemap remap;
	// Export labels, by id.
	std::map<word, std::string> old_labels, new_labels;
	// For each id in `remap.unmatched_old()`, how many configs have a value set.
	std::map<word, int> unmatched_value_counts;
	// For each InitD index in `remap.old_params`, how many configs have a
	// non-zero value.
	std::map<int, int> initd_value_counts;

	// True if the user should be asked: a variable with values set is gone,
	// but a new variable could be what it was renamed to; or the script was
	// converted from InitD, with variables InitD values could go to.
	bool needs_user_mapping() const;
};

// Calls `fn` for every script config in the quest that uses a slot. `scripts`
// is the slot array (ex. `ffscripts`), and `slot` the index into it.
using InstInitVisitor = std::function<void(script_data** scripts, int slot,
	std::map<word, int>& inst_init, const std::array<int, 8>& run_args)>;
void for_each_inst_init(InstInitVisitor fn);

using ScriptExports = bounded_map<word, exported_variable>;

// Finds the slots whose stored values need to move. The slots must still hold
// the previous compile's scripts, and `old_debug_data` must be its debug data.
// `find_new_exports` gives the exports of a script in the new compile, or
// nullptr if it has no such script.
std::vector<SlotRemap> plan_slot_remaps(const DebugData& old_debug_data, const DebugData& new_debug_data,
	std::function<const ScriptExports*(const std::string& script_name)> find_new_exports);

InstVarRemapResult apply_slot_remaps(const std::vector<SlotRemap>& remaps);

#endif
