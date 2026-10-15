#ifndef ZASM_INST_VAR_REMAP_H_
#define ZASM_INST_VAR_REMAP_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/ints.h"

struct DebugData;

// A script's instance variables are stored by index, assigned in declaration
// order. Values set per-instance in the editor (`@Export`ed variables) are
// keyed by that index, so adding, removing or reordering variables would move
// every stored value onto a different variable. This matches each variable
// across two compiles of a script by name, so stored values can follow them.

// How a variable's value is stored. An `int` stores its value * 10000, and a
// `long` stores its value as is.
enum class InstVarKind
{
	Other,
	Int,
	Long,
};

struct InstVar
{
	word id;
	std::string name;
	std::string type_name;
	InstVarKind kind;
};

struct InstVarRemap
{
	// Old id -> new id, for each variable found in both compiles.
	std::map<word, word> old_to_new;
	// Every variable of each compile, by id.
	std::map<word, InstVar> old_vars, new_vars;

	// Variables not in `old_to_new` - removed or renamed, and added or
	// renamed, respectively. In id order.
	std::vector<InstVar> unmatched_old() const;
	std::vector<InstVar> unmatched_new() const;

	// If set, only these new variables can take an unmatched old variable's
	// values (ex. the exported ones), so `unmatched_new()` lists only these.
	std::optional<std::set<word>> new_candidates;

	// True if stored values are already where they belong.
	bool is_identity() const;
};

// Returns nullopt if either compile lacks a single script named `script_name`.
std::optional<InstVarRemap> build_inst_var_remap(const DebugData& old_data,
	const DebugData& new_data, const std::string& script_name);

// Converts a stored value for a variable that changed between `int` and `long`,
// so it keeps the same number. Otherwise returns the value as is.
int convert_inst_var_value(int value, InstVarKind from, InstVarKind to);

struct InstVarRemapResult
{
	// Values moved to a new id, or converted between `int` and `long`.
	int updated = 0;
	int dropped = 0;
};

// Moves each value in `inst_init` to its variable's new id (converting it if
// the variable changed between `int` and `long`), and drops values whose
// variable has no new id.
InstVarRemapResult apply_inst_var_remap(std::map<word, int>& inst_init, const InstVarRemap& remap);

#endif
