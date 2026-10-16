#include "components/zasm/inst_var_remap.h"

#include "components/zasm/debug_data.h"

#include <algorithm>
#include <climits>

static std::optional<int> find_script_scope(const DebugData& data, const std::string& script_name)
{
	std::optional<int> script_scope_index;
	for (int i = 0; i < (int)data.scopes.size(); i++)
	{
		auto& scope = data.scopes[i];
		if (scope.tag != TAG_SCRIPT || scope.name != script_name)
			continue;
		if (script_scope_index)
			return std::nullopt;
		script_scope_index = i;
	}
	return script_scope_index;
}

static InstVar make_var(const DebugData& data, const DebugSymbol* symbol, word id)
{
	auto* type = data.getType(symbol->type_id);
	auto kind = InstVarKind::Other;
	if (type && type->isFixed(data))
		kind = InstVarKind::Int;
	else if (type && type->isLong(data))
		kind = InstVarKind::Long;
	return {id, symbol->name, data.getTypeName(type), kind};
}

static std::vector<InstVar> get_inst_vars(const DebugData& data, int script_scope_index)
{
	// The script's `this` is instance variable 0. It is never exported, and
	// is always id 0, so leave it out.
	std::vector<InstVar> vars;
	for (auto* symbol : data.getChildSymbols(&data.scopes[script_scope_index]))
		if (symbol->storage == LOC_SCRIPT_INSTANCE && symbol->offset != 0)
			vars.push_back(make_var(data, symbol, symbol->offset));
	std::sort(vars.begin(), vars.end(), [](auto& a, auto& b){ return a.id < b.id; });
	return vars;
}

// The parameters of the script's run(), with their InitD index as the id.
static std::vector<InstVar> get_run_params(const DebugData& data, int script_scope_index)
{
	for (auto* scope : data.getChildScopes(&data.scopes[script_scope_index]))
	{
		if (scope->tag != TAG_FUNCTION || scope->name != "run")
			continue;

		// Parameters are on the stack, the first one deepest.
		std::vector<const DebugSymbol*> symbols;
		for (auto* symbol : data.getChildSymbols(scope))
			if (symbol->storage == LOC_STACK)
				symbols.push_back(symbol);
		std::sort(symbols.begin(), symbols.end(), [](auto* a, auto* b){ return a->offset > b->offset; });

		std::vector<InstVar> params;
		for (size_t i = 0; i < symbols.size(); i++)
			params.push_back(make_var(data, symbols[i], i));
		return params;
	}
	return {};
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
			&& std::none_of(old_to_new.begin(), old_to_new.end(), [&](auto& p){ return p.second == id; })
			&& std::none_of(initd_moves.begin(), initd_moves.end(), [&](auto& m){ return m.new_id == id; }))
			vars.push_back(var);
	return vars;
}

bool InstVarRemap::is_identity() const
{
	if (old_to_new.size() != old_vars.size() || !old_params.empty())
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
	auto old_scope = find_script_scope(old_data, script_name);
	auto new_scope = find_script_scope(new_data, script_name);
	if (!old_scope || !new_scope)
		return std::nullopt;

	auto old_vars = get_inst_vars(old_data, *old_scope);
	auto new_vars = get_inst_vars(new_data, *new_scope);
	InstVarRemap remap;
	for (auto& var : old_vars)
		remap.old_vars[var.id] = var;
	for (auto& var : new_vars)
		remap.new_vars[var.id] = var;
	auto find_new_var = [&](const std::string& name) -> const InstVar* {
		auto it = std::find_if(new_vars.begin(), new_vars.end(), [&](auto& v){ return v.name == name; });
		return it == new_vars.end() ? nullptr : &*it;
	};
	for (auto& old_var : old_vars)
		if (auto* new_var = find_new_var(old_var.name))
			remap.old_to_new[old_var.id] = new_var->id;

	// Only for a script converted entirely from run() parameters, so InitD
	// indices never need to shift.
	auto old_params = get_run_params(old_data, *old_scope);
	if (!old_params.empty() && get_run_params(new_data, *new_scope).empty())
	{
		remap.old_params = old_params;
		for (auto& param : old_params)
			if (auto* new_var = find_new_var(param.name))
				remap.initd_moves.push_back({(int)param.id, param.kind, new_var->id});
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

int apply_initd_moves(std::map<word, int>& inst_init, const std::array<int, 8>& run_args, const InstVarRemap& remap)
{
	int copied = 0;
	for (auto& move : remap.initd_moves)
	{
		if (inst_init.contains(move.new_id))
			continue;
		inst_init[move.new_id] = convert_inst_var_value(run_args[move.initd], move.kind, remap.new_vars.at(move.new_id).kind);
		copied++;
	}
	return copied;
}
