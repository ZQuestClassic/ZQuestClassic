#include "components/zasm/inst_var_remap.h"

#include "components/zasm/debug_data.h"

#include <algorithm>
#include <climits>

static std::optional<std::vector<InstVar>> get_inst_vars(const DebugData& data, const std::string& script_name)
{
	int script_scope_index = -1;
	for (int i = 0; i < (int)data.scopes.size(); i++)
	{
		auto& scope = data.scopes[i];
		if (scope.tag != TAG_SCRIPT || scope.name != script_name)
			continue;
		if (script_scope_index != -1)
			return std::nullopt;
		script_scope_index = i;
	}
	if (script_scope_index == -1)
		return std::nullopt;

	// The script's `this` is instance variable 0. It is never exported, and
	// is always id 0, so leave it out.
	std::vector<InstVar> vars;
	for (auto* symbol : data.getChildSymbols(&data.scopes[script_scope_index]))
	{
		if (symbol->storage != LOC_SCRIPT_INSTANCE || symbol->offset == 0)
			continue;
		auto* type = data.getType(symbol->type_id);
		auto kind = InstVarKind::Other;
		if (type && type->isFixed(data))
			kind = InstVarKind::Int;
		else if (type && type->isLong(data))
			kind = InstVarKind::Long;
		vars.push_back({(word)symbol->offset, symbol->name, data.getTypeName(type), kind});
	}
	std::sort(vars.begin(), vars.end(), [](auto& a, auto& b){ return a.id < b.id; });
	return vars;
}

std::vector<InstVar> InstVarRemap::unmatched_old() const
{
	std::vector<InstVar> vars;
	for (auto& [id, var] : old_vars)
		if (!old_to_new.contains(id))
			vars.push_back(var);
	return vars;
}

std::vector<InstVar> InstVarRemap::unmatched_new() const
{
	std::vector<InstVar> vars;
	for (auto& [id, var] : new_vars)
		if ((!new_candidates || new_candidates->contains(id))
			&& std::none_of(old_to_new.begin(), old_to_new.end(), [&](auto& p){ return p.second == id; }))
			vars.push_back(var);
	return vars;
}

bool InstVarRemap::is_identity() const
{
	if (old_to_new.size() != old_vars.size())
		return false;
	for (auto [old_id, new_id] : old_to_new)
		if (old_id != new_id || old_vars.at(old_id).kind != new_vars.at(new_id).kind)
			return false;
	return true;
}

int convert_inst_var_value(int value, InstVarKind from, InstVarKind to)
{
	if (from == InstVarKind::Int && to == InstVarKind::Long)
		return value / 10000;
	if (from == InstVarKind::Long && to == InstVarKind::Int)
		return (int)std::clamp<int64_t>((int64_t)value * 10000, INT_MIN, INT_MAX);
	return value;
}

std::optional<InstVarRemap> build_inst_var_remap(const DebugData& old_data,
	const DebugData& new_data, const std::string& script_name)
{
	auto old_vars = get_inst_vars(old_data, script_name);
	auto new_vars = get_inst_vars(new_data, script_name);
	if (!old_vars || !new_vars)
		return std::nullopt;

	InstVarRemap remap;
	for (auto& var : *old_vars)
		remap.old_vars[var.id] = var;
	for (auto& var : *new_vars)
		remap.new_vars[var.id] = var;
	for (auto& old_var : *old_vars)
	{
		auto it = std::find_if(new_vars->begin(), new_vars->end(),
			[&](auto& v){ return v.name == old_var.name; });
		if (it != new_vars->end())
			remap.old_to_new[old_var.id] = it->id;
	}
	return remap;
}

InstVarRemapResult apply_inst_var_remap(std::map<word, int>& inst_init, const InstVarRemap& remap)
{
	InstVarRemapResult result;
	std::map<word, int> remapped;
	for (auto [id, val] : inst_init)
	{
		auto it = remap.old_to_new.find(id);
		if (it == remap.old_to_new.end())
		{
			result.dropped++;
			continue;
		}
		auto [old_id, new_id] = *it;
		int new_val = convert_inst_var_value(val, remap.old_vars.at(old_id).kind, remap.new_vars.at(new_id).kind);
		if (new_id != old_id || new_val != val)
			result.updated++;
		remapped[new_id] = new_val;
	}
	inst_init = std::move(remapped);
	return result;
}
