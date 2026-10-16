#include "zq/script_slot_remap.h"

#include "components/zasm/debug_data.h"
#include "core/combo.h"
#include "core/dmap.h"
#include "core/initdata.h"
#include "core/mapscr.h"
#include "core/misctypes.h"
#include "core/qst.h"
#include "core/zdefs.h"
#include "ffc.h"
#include "items.h"
#include "new_subscr.h"
#include "save_menu.h"
#include "subscr.h"
#include "zq/zquest.h"

#include <set>

extern script_data *ffscripts[NUMSCRIPTFFC];
extern script_data *itemscripts[NUMSCRIPTITEM];
extern script_data *guyscripts[NUMSCRIPTGUYS];
extern script_data *lwpnscripts[NUMSCRIPTWEAPONS];
extern script_data *ewpnscripts[NUMSCRIPTWEAPONS];
extern script_data *globalscripts[NUMSCRIPTGLOBAL];
extern script_data *genericscripts[NUMSCRIPTSGENERIC];
extern script_data *playerscripts[NUMSCRIPTHERO];
extern script_data *screenscripts[NUMSCRIPTSCREEN];
extern script_data *dmapscripts[NUMSCRIPTSDMAP];
extern script_data *itemspritescripts[NUMSCRIPTSITEMSPRITE];
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];
extern script_data *subscreenscripts[NUMSCRIPTSSUBSCREEN];

bool is_misc_lweapon(newcombo const& cmb);

namespace {

struct SlotType
{
	std::map<int32_t, script_slot_data>* slots;
	// Index into `scripts` of `slots[0]`.
	int offset;
	script_data** scripts;
	const char* name;
};

// Must match the slot handling in do_slots.
const SlotType slot_types[] = {
	{&ffcmap, 1, ffscripts, "FFC"},
	{&globalmap, 0, globalscripts, "Global"},
	{&itemmap, 1, itemscripts, "Item"},
	{&npcmap, 1, guyscripts, "NPC"},
	{&lwpnmap, 1, lwpnscripts, "LWeapon"},
	{&ewpnmap, 1, ewpnscripts, "EWeapon"},
	{&playermap, 1, playerscripts, "Hero"},
	{&dmapmap, 1, dmapscripts, "DMap"},
	{&screenmap, 1, screenscripts, "Screen"},
	{&itemspritemap, 1, itemspritescripts, "Item Sprite"},
	{&comboscriptmap, 1, comboscripts, "Combo"},
	{&genericmap, 1, genericscripts, "Generic"},
	{&subscreenmap, 1, subscreenscripts, "Subscreen"},
};

} // namespace

bool SlotRemap::needs_user_mapping() const
{
	// A conversion is confirmed by the user, as long as there's a variable
	// the InitD values could go to.
	if (!remap.old_params.empty() && !initd_value_counts.empty()
		&& (!remap.unmatched_new().empty() || !remap.initd_moves.empty()))
		return true;
	if (remap.unmatched_new().empty())
		return false;
	for (auto& var : remap.unmatched_old())
		if (unmatched_value_counts.contains(var.id))
			return true;
	return false;
}

void for_each_inst_init(InstInitVisitor fn)
{
	auto visit = [&](script_data** scripts, script_config& cfg) {
		if (cfg.script)
			fn(scripts, cfg.script, cfg.inst_init, cfg.run_args);
	};

	for (auto& scr : TheMaps)
	{
		visit(screenscripts, scr.scrconfig);
		for (auto& ffc : scr.ffcs)
			visit(ffscripts, ffc.scrconfig);
	}
	for (auto& combo : combobuf)
	{
		visit(comboscripts, combo.scrconfig);
		// Most combos keep the default weapon data; don't allocate theirs.
		if (combo.lift_weap_data.get().scrconfig.script)
			visit(lwpnscripts, combo.lift_weap_data.mut().scrconfig);
		if (combo.misc_weap_data.get().scrconfig.script)
			visit(is_misc_lweapon(combo) ? lwpnscripts : ewpnscripts, combo.misc_weap_data.mut().scrconfig);
	}
	for (int i = 0; i < MAXDMAPS; i++)
	{
		auto& dmap = DMaps[i];
		visit(dmapscripts, dmap.active_scrconfig);
		visit(dmapscripts, dmap.active_sub_scrconfig);
		visit(dmapscripts, dmap.passive_sub_scrconfig);
		visit(dmapscripts, dmap.onmap_scrconfig);
	}
	for (auto& item : itemsbuf.mut_inner())
	{
		visit(itemscripts, item.scrconfig);
		visit(itemscripts, item.collect_scrconfig);
		visit(itemspritescripts, item.sprite_scrconfig);
		visit(lwpnscripts, item.weap_data.scrconfig);
	}
	for (auto& guy : guysbuf)
	{
		visit(guyscripts, guy.scrconfig);
		visit(ewpnscripts, guy.weap_data.scrconfig);
	}
	for (auto* subscreens : {&subscreens_active, &subscreens_passive, &subscreens_overlay, &subscreens_map})
	{
		for (auto& subscreen : *subscreens)
		{
			visit(subscreenscripts, subscreen.scrconfig);
			for (auto& page : subscreen.pages)
				for (auto* widget : page.contents)
					visit(genericscripts, widget->generic_scrconfig);
		}
	}
	for (auto& save_menu : QMisc.save_menus)
		for (auto& option : save_menu.options)
			visit(genericscripts, option.gen_scrconfig);

	// These configs don't store their slot - it's their index.
	for (int i = 0; i < NUMSCRIPTGLOBAL; i++)
		fn(globalscripts, i, zinit.global_scrconfig[i].inst_init, zinit.global_scrconfig[i].run_args);
	for (int i = 0; i < NUMSCRIPTHERO; i++)
		fn(playerscripts, i, zinit.hero_scrconfig[i].inst_init, zinit.hero_scrconfig[i].run_args);
	// Generic scripts' Init Data keeps their InitD and variable values apart.
	std::set<word> generic_slots;
	for (auto& [slot, _] : zinit.gen_inst_init.inner())
		generic_slots.insert(slot);
	for (auto& [slot, _] : zinit.gen_initd.inner())
		generic_slots.insert(slot);
	for (word slot : generic_slots)
	{
		std::array<int, 8> run_args;
		auto const& initd = zinit.gen_initd.get(slot);
		for (int q = 0; q < 8; ++q)
			run_args[q] = initd.get(q);
		// A copy, so slots with only InitD don't gain an empty entry.
		auto inst_init = zinit.gen_inst_init.get(slot);
		fn(genericscripts, slot, inst_init, run_args);
		if (inst_init != zinit.gen_inst_init.get(slot))
			zinit.gen_inst_init[slot] = inst_init;
	}
}

std::vector<SlotRemap> plan_slot_remaps(const DebugData& old_debug_data, const DebugData& new_debug_data,
	std::function<const ScriptExports*(const std::string& script_name)> find_new_exports)
{
	std::vector<SlotRemap> remaps;
	for (auto& type : slot_types)
	{
		for (auto& [index, slot_data] : *type.slots)
		{
			if (!slot_data.hasScriptData())
				continue;

			int slot = index + type.offset;
			script_data* old_script = type.scripts[slot];
			auto const& name = slot_data.scriptname;
			// A slot given a different script keeps its values as they are.
			if (!old_script || old_script->meta.get().script_name != name)
				continue;

			auto new_exports = find_new_exports(name);
			if (!new_exports)
				continue;

			auto remap = build_inst_var_remap(old_debug_data, new_debug_data, name);
			if (!remap)
				continue;
			// Only exported variables have values set per config. And a variable
			// taking an old variable's values (of the same name) can't also take InitD's.
			std::erase_if(remap->initd_moves, [&](auto& move){
				return !new_exports->inner().contains(move.new_id)
					|| std::any_of(remap->old_to_new.begin(), remap->old_to_new.end(),
						[&](auto& p){ return p.second == move.new_id; });
			});
			if (remap->is_identity())
				continue;
			remap->new_candidates.emplace();
			for (auto& [id, _] : new_exports->inner())
				remap->new_candidates->insert(id);

			SlotRemap& r = remaps.emplace_back();
			r.scripts = type.scripts;
			r.slot = slot;
			r.slot_type_name = type.name;
			r.script_name = name;
			r.remap = std::move(*remap);
			for (auto& [id, exported] : old_script->script_d_exports.inner())
				r.old_labels[id] = exported.name;
			for (auto& [id, exported] : new_exports->inner())
				r.new_labels[id] = exported.name;
		}
	}

	if (remaps.empty())
		return remaps;

	for_each_inst_init([&](script_data** scripts, int slot, std::map<word, int>& inst_init, const std::array<int, 8>& run_args) {
		for (auto& r : remaps)
		{
			if (r.scripts != scripts || r.slot != slot)
				continue;
			for (auto& var : r.remap.unmatched_old())
				if (inst_init.contains(var.id))
					r.unmatched_value_counts[var.id]++;
			for (auto& param : r.remap.old_params)
				if (run_args[param.id])
					r.initd_value_counts[param.id]++;
		}
	});

	return remaps;
}

InstVarRemapResult apply_slot_remaps(const std::vector<SlotRemap>& remaps)
{
	InstVarRemapResult result;
	if (remaps.empty())
		return result;

	int initd_copied = 0;
	for_each_inst_init([&](script_data** scripts, int slot, std::map<word, int>& inst_init, const std::array<int, 8>& run_args) {
		for (auto& r : remaps)
		{
			if (r.scripts != scripts || r.slot != slot)
				continue;
			auto r_result = apply_inst_var_remap(inst_init, r.remap);
			result.updated += r_result.updated;
			result.dropped += r_result.dropped;
			initd_copied += apply_initd_moves(inst_init, run_args, r.remap);
			break;
		}
	});

	for (auto& r : remaps)
	{
		for (auto& var : r.remap.unmatched_old())
			if (r.unmatched_value_counts.contains(var.id))
				zprint2("%s script '%s': discarded the values of variable '%s'\n",
					r.slot_type_name.c_str(), r.script_name.c_str(), var.name.c_str());
		for (auto& param : r.remap.old_params)
		{
			bool moved = std::any_of(r.remap.initd_moves.begin(), r.remap.initd_moves.end(),
				[&](auto& move){ return move.initd == param.id; });
			if (!moved && r.initd_value_counts.contains(param.id))
				zprint2("%s script '%s': the InitD values of parameter '%s' weren't copied to a variable\n",
					r.slot_type_name.c_str(), r.script_name.c_str(), param.name.c_str());
		}
	}
	if (result.updated || result.dropped)
		zprint2("Updated %d and discarded %d script variable values to match the recompiled scripts\n",
			result.updated, result.dropped);
	if (initd_copied)
		zprint2("Copied %d InitD values to the script variables of the same name\n", initd_copied);

	return result;
}
