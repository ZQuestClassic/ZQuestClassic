// Editor tests. Run via `zeditor -test-zc <tests dir>` (see
// zeditor_handle_commands); tests/test_zeditor.py drives that.

#include "core/combo.h"
#include "core/dmap.h"
#include "core/initdata.h"
#include "core/mapscr.h"
#include "core/misctypes.h"
#include "core/qst.h"
#include "ffc.h"
#include "items.h"
#include "new_subscr.h"
#include "save_menu.h"
#include "sprite_data.h"
#include "subscr.h"
#include "test_runner/assert.h"
#include "test_runner/test_runner.h"
#include "tiles.h"
#include "zalleg/colors.h"
#include "zq/dialog/compilezscript.h"
#include "zq/script_slot_remap.h"
#include "zq/zq_class.h"
#include "zq/zquest.h"

#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

extern std::string zScript;
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

static fs::path test_dir;

void set_zquest_test_dir(std::string const& dir)
{
	test_dir = dir;
}

namespace
{

// What Quest > Defaults resets, captured from the loaded quest so two
// quest loads can be compared.
struct ComboKey
{
	int32_t tile;
	byte flip, walk, type, csets, frames, speed;
	word nextcombo;
	byte nextcset, flag;
	bool operator==(ComboKey const&) const = default;
};

struct Tile
{
	byte format;
	std::vector<byte> data;
	bool operator==(Tile const&) const = default;
};

struct DefaultsSnapshot
{
	std::vector<ComboKey> combos;
	std::vector<Tile> tiles;
	std::vector<byte> colors;
	bounded_vec<word, itemdata> items;
	bounded_vec<word, sprite_data> sprites;
};

DefaultsSnapshot snapshot_defaults()
{
	DefaultsSnapshot s;
	for (auto const& c : combobuf)
		s.combos.push_back({c.tile, c.flip, c.walk, c.type, c.csets, c.frames, c.speed,
			c.nextcombo, c.nextcset, c.flag});
	for (int32_t t = 0; t < NEWMAXTILES; ++t)
	{
		Tile tile{newtilebuf[t].format, {}};
		if (newtilebuf[t].data)
			tile.data.assign(newtilebuf[t].data, newtilebuf[t].data + tilesize(tile.format));
		s.tiles.push_back(std::move(tile));
	}
	s.colors.assign(colordata, colordata + psTOTAL255);
	s.items = itemsbuf;
	s.sprites = sprite_data_buf;
	return s;
}

void load(fs::path const& path)
{
	int32_t ret = load_quest(path.string().c_str(), false);
	if (ret)
		throw std::runtime_error(fmt::format("failed to load {}: ret == {}", path.string(), ret));
}

// Quest > Defaults reads sections out of the template quest with another
// quest open. Each must succeed and leave that part of the open quest
// matching the template.
void test_defaults_apply()
{
	load(test_dir / "replays/classic_1st/classic_1st.qst");
	auto before = snapshot_defaults();

	assertTrue(init_combos(true, &header));
	assertTrue(init_tiles(true, &header));
	assertTrue(init_colordata(true, &header, &QMisc));
	assertTrue(reset_items(true, &header));
	assertTrue(reset_wpns(true, &header));
	auto after = snapshot_defaults();

	// The two quests differ, so a reset that did nothing shows up here.
	assertTrue(after.combos != before.combos);
	assertTrue(after.tiles != before.tiles);
	assertTrue(after.colors != before.colors);
	assertTrue(after.items != before.items);
	assertTrue(after.sprites != before.sprites);

	// Same path init_section opens the template by, relative to the cwd.
	load("modules/classic/default.qst");
	auto expected = snapshot_defaults();
	assertTrue(after.combos == expected.combos);
	assertTrue(after.tiles == expected.tiles);
	assertTrue(after.colors == expected.colors);
	assertTrue(after.items == expected.items);
	assertTrue(after.sprites == expected.sprites);
}

int find_ffc_slot(std::string const& name)
{
	for (int i = 1; i < NUMSCRIPTFFC; i++)
		if (ffscripts[i] && ffscripts[i]->meta.get().script_name == name)
			return i;
	throw std::runtime_error(fmt::format("script {} not assigned to a slot", name));
}

void compile(std::string const& source)
{
	zScript = source;
	// Smart assign, to put new scripts in slots.
	if (!do_compile_and_slots(2, false))
		throw std::runtime_error("failed to compile");
}

// Values set for a script's exported variables follow the variables when the
// script is recompiled with its variables moved around.
void test_inst_var_remap()
{
	load(test_dir / "replays/classic_1st/classic_1st.qst");

	// Another script with the same variable names is left alone.
	std::string other_script = R"zs(
		ffc script remap_other
		{
			@Export("Speed")
			nonstatic int speed;
			void run() { Trace(speed); }
		}
	)zs";
	compile(other_script + R"zs(
		ffc script remap_test
		{
			@Export("Speed")
			nonstatic int speed;
			@Export("Health")
			nonstatic int health;
			@Export("Old Name")
			nonstatic int old_name;
			void run() { Trace(speed + health + old_name); }
		}
	)zs");

	int slot = find_ffc_slot("remap_test");
	int other_slot = find_ffc_slot("remap_other");
	auto& ffc = TheMaps[0].getFFC(0);
	ffc.scrconfig.script = slot;
	ffc.scrconfig.inst_init = {{1, 10}, {2, 20}, {3, 30}};
	auto& other_ffc = TheMaps[0].getFFC(1);
	other_ffc.scrconfig.script = other_slot;
	other_ffc.scrconfig.inst_init = {{1, 10}};
	// An FFC on another screen.
	auto& ffc_on_other_screen = TheMaps[5].getFFC(3);
	ffc_on_other_screen.scrconfig.script = slot;
	ffc_on_other_screen.scrconfig.inst_init = {{2, 25}};

	// Inserts a variable at the top, swaps two, and renames one. Headless, so
	// the renamed variable's values are discarded rather than asking.
	compile(other_script + R"zs(
		ffc script remap_test
		{
			nonstatic int counter;
			@Export("Health")
			nonstatic int health;
			@Export("Speed")
			nonstatic int speed;
			@Export("New Name")
			nonstatic int new_name;
			void run() { counter++; Trace(speed + health + new_name + counter); }
		}
	)zs");

	assertEqual(find_ffc_slot("remap_test"), slot);
	assertEqual(TheMaps[0].getFFC(0).scrconfig.inst_init, (std::map<word, int>{{2, 20}, {3, 10}}));
	assertEqual(TheMaps[0].getFFC(1).scrconfig.inst_init, (std::map<word, int>{{1, 10}}));
	assertEqual(TheMaps[5].getFFC(3).scrconfig.inst_init, (std::map<word, int>{{2, 25}}));
}

int find_slot(script_data** scripts, int count, std::string const& name)
{
	for (int i = 0; i < count; i++)
		if (scripts[i] && scripts[i]->meta.get().script_name == name)
			return i;
	throw std::runtime_error(fmt::format("script {} not assigned to a slot", name));
}

// Values set on every kind of placement that has script configs follow their
// variables, using the right slot type for each.
void test_inst_var_remap_all_placements()
{
	load(test_dir / "replays/playground/playground.qst");

	struct ScriptType { const char* keyword; const char* name; script_data** scripts; int count; };
	// Global and hero scripts are only smart-assigned to the slot of their name.
	ScriptType types[] = {
		{"ffc", "remap_ffc", ffscripts, NUMSCRIPTFFC},
		{"itemdata", "remap_item", itemscripts, NUMSCRIPTITEM},
		{"npc", "remap_npc", guyscripts, NUMSCRIPTGUYS},
		{"lweapon", "remap_lweapon", lwpnscripts, NUMSCRIPTWEAPONS},
		{"eweapon", "remap_eweapon", ewpnscripts, NUMSCRIPTWEAPONS},
		{"global", "onLaunch", globalscripts, NUMSCRIPTGLOBAL},
		{"generic", "remap_generic", genericscripts, NUMSCRIPTSGENERIC},
		{"player", "onWin", playerscripts, NUMSCRIPTHERO},
		{"screendata", "remap_screen", screenscripts, NUMSCRIPTSCREEN},
		{"dmapdata", "remap_dmap", dmapscripts, NUMSCRIPTSDMAP},
		{"itemsprite", "remap_itemsprite", itemspritescripts, NUMSCRIPTSITEMSPRITE},
		{"combodata", "remap_combo", comboscripts, NUMSCRIPTSCOMBODATA},
		{"subscreendata", "remap_subscreen", subscreenscripts, NUMSCRIPTSSUBSCREEN},
	};
	auto source = [&](bool reordered) {
		std::string s;
		for (auto& type : types)
			s += fmt::format(R"zs(
				{} script {}
				{{
					{}
					@Export("A")
					nonstatic int a;
					@Export("B")
					nonstatic int b;
					void run() {{ Trace(a + b); }}
				}}
			)zs", type.keyword, type.name, reordered ? "nonstatic int pad;" : "");
		// A generic script converted from InitD, for Init Data's generic InitD.
		s += reordered
			? "generic script remap_initd { @Export(\"A\") nonstatic int a; void run() { Trace(a); } }"
			: "generic script remap_initd { void run(int a) { Trace(a); } }";
		return s;
	};

	compile(source(false));
	std::map<std::string, int> slots;
	for (auto& type : types)
		slots[type.name] = find_slot(type.scripts, type.count, type.name);
	int initd_slot = find_slot(genericscripts, NUMSCRIPTSGENERIC, "remap_initd");

	assertTrue(!subscreens_active.empty() && !subscreens_active[0].pages.empty()
		&& !subscreens_active[0].pages[0].contents.empty());
	if (QMisc.save_menus[0].options.empty())
		QMisc.save_menus[0].options.emplace_back();

	auto& shooter = combobuf[3];
	shooter.type = cSHOOTER;
	shooter.c_attributes[9] = wEnemyWeapons; // fires an eweapon

	struct Placement { const char* what; const char* script; std::function<script_config&()> config; };
	Placement placements[] = {
		{"screen", "remap_screen", [&]() -> script_config& { return TheMaps[0].scrconfig; }},
		{"ffc", "remap_ffc", [&]() -> script_config& { return TheMaps[0].getFFC(0).scrconfig; }},
		{"combo", "remap_combo", [&]() -> script_config& { return combobuf[1].scrconfig; }},
		{"combo lift weapon", "remap_lweapon", [&]() -> script_config& { return combobuf[2].lift_weap_data.mut().scrconfig; }},
		{"combo misc weapon", "remap_eweapon", [&]() -> script_config& { return shooter.misc_weap_data.mut().scrconfig; }},
		{"dmap active", "remap_dmap", [&]() -> script_config& { return DMaps[0].active_scrconfig; }},
		{"dmap active subscreen", "remap_dmap", [&]() -> script_config& { return DMaps[0].active_sub_scrconfig; }},
		{"dmap passive subscreen", "remap_dmap", [&]() -> script_config& { return DMaps[0].passive_sub_scrconfig; }},
		{"dmap map", "remap_dmap", [&]() -> script_config& { return DMaps[0].onmap_scrconfig; }},
		{"item action", "remap_item", [&]() -> script_config& { return itemsbuf[1].scrconfig; }},
		{"item pickup", "remap_item", [&]() -> script_config& { return itemsbuf[1].collect_scrconfig; }},
		{"item sprite", "remap_itemsprite", [&]() -> script_config& { return itemsbuf[1].sprite_scrconfig; }},
		{"item weapon", "remap_lweapon", [&]() -> script_config& { return itemsbuf[1].weap_data.scrconfig; }},
		{"enemy", "remap_npc", [&]() -> script_config& { return guysbuf[1].scrconfig; }},
		{"enemy weapon", "remap_eweapon", [&]() -> script_config& { return guysbuf[1].weap_data.scrconfig; }},
		{"subscreen", "remap_subscreen", [&]() -> script_config& { return subscreens_active[0].scrconfig; }},
		{"subscreen widget", "remap_generic", [&]() -> script_config& { return subscreens_active[0].pages[0].contents[0]->generic_scrconfig; }},
		{"save menu option", "remap_generic", [&]() -> script_config& { return QMisc.save_menus[0].options[0].gen_scrconfig; }},
	};
	for (auto& p : placements)
	{
		auto& cfg = p.config();
		cfg.script = slots[p.script];
		cfg.inst_init = {{1, 10}, {2, 20}};
	}
	// Init Data's configs are indexed by slot, rather than storing it.
	zinit.global_scrconfig[slots["onLaunch"]].inst_init = {{1, 10}, {2, 20}};
	zinit.hero_scrconfig[slots["onWin"]].inst_init = {{1, 10}, {2, 20}};
	zinit.gen_inst_init[slots["remap_generic"]] = {{1, 10}, {2, 20}};
	zinit.gen_initd[initd_slot][0] = 30000;

	// `pad` moves a and b from ids 1, 2 to 2, 3.
	compile(source(true));
	std::map<word, int> expected = {{2, 10}, {3, 20}};
	for (auto& p : placements)
	{
		if (p.config().inst_init != expected)
			throw std::runtime_error(fmt::format("{}: got {}", p.what, p.config().inst_init));
	}
	assertEqual(zinit.global_scrconfig[slots["onLaunch"]].inst_init, expected);
	assertEqual(zinit.hero_scrconfig[slots["onWin"]].inst_init, expected);
	assertEqual(zinit.gen_inst_init.get(slots["remap_generic"]), expected);
	assertEqual(zinit.gen_inst_init.get(initd_slot), (std::map<word, int>{{1, 30000}}));
}

// A script converted from run() parameters to exported variables gets each
// placement's InitD values copied to the variable of the same name.
void test_initd_to_variables()
{
	load(test_dir / "replays/classic_1st/classic_1st.qst");

	compile(R"zs(
		ffc script initd_test
		{
			void run(int speed, int count) { Trace(speed + count); }
		}
	)zs");
	int slot = find_ffc_slot("initd_test");
	auto& ffc = TheMaps[0].getFFC(0);
	ffc.scrconfig.script = slot;
	ffc.scrconfig.run_args = {30000, 0};

	compile(R"zs(
		ffc script initd_test
		{
			@Export("Count")
			nonstatic int count = 4;
			@Export("Speed")
			nonstatic int speed;
			void run() { Trace(speed + count); }
		}
	)zs");

	// count's InitD of 0 is copied too, so the script behaves as before.
	assertEqual(TheMaps[0].getFFC(0).scrconfig.inst_init, (std::map<word, int>{{1, 0}, {2, 30000}}));
	// InitD is left as is.
	assertEqual(TheMaps[0].getFFC(0).scrconfig.run_args[0], 30000);
}

// A script converted from InitD asks the user to confirm, unless there's no
// exported variable the values could go to.
void test_initd_conversion_needs_user_mapping()
{
	SlotRemap r;
	r.remap.old_params = {{0, "speed", "int", InstVarKind::Int}};
	r.initd_value_counts = {{0, 3}};
	r.remap.new_candidates.emplace();
	assertTrue(!r.needs_user_mapping());

	// A misspelled export can be picked for it.
	r.remap.new_vars[1] = {1, "sped", "int", InstVarKind::Int};
	r.remap.new_candidates->insert(1);
	assertTrue(r.needs_user_mapping());

	// No InitD values set anywhere: nothing to confirm.
	r.initd_value_counts.clear();
	assertTrue(!r.needs_user_mapping());
}

// Weapon scripts set on combos (lifting, and step/shooter weapons) are visited
// with the right slot array.
void test_inst_init_walker_combo_weapons()
{
	load(test_dir / "replays/classic_1st/classic_1st.qst");

	auto& lifter = combobuf[1];
	lifter.lift_weap_data.mut().scrconfig.script = 2;
	auto& shooter = combobuf[2];
	shooter.type = cSHOOTER;
	shooter.c_attributes[9] = wEnemyWeapons; // fires an eweapon
	shooter.misc_weap_data.mut().scrconfig.script = 3;

	bool saw_lift = false, saw_shooter = false;
	int combo_weapon_configs = 0;
	for_each_inst_init([&](script_data** scripts, int slot, std::map<word, int>& inst_init, auto&) {
		if (&inst_init == &lifter.lift_weap_data.get().scrconfig.inst_init)
			saw_lift = scripts == lwpnscripts && slot == 2;
		if (&inst_init == &shooter.misc_weap_data.get().scrconfig.inst_init)
			saw_shooter = scripts == ewpnscripts && slot == 3;
		for (auto& combo : combobuf)
			if (&inst_init == &combo.lift_weap_data.get().scrconfig.inst_init
				|| &inst_init == &combo.misc_weap_data.get().scrconfig.inst_init)
				combo_weapon_configs++;
	});
	assertTrue(saw_lift);
	assertTrue(saw_shooter);
	// Combos left with the default weapon data are skipped.
	assertEqual(combo_weapon_configs, 2);
}

} // namespace

TestResults test_zquest(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "defaults_apply", test_defaults_apply },
		{ "inst_var_remap", test_inst_var_remap },
		{ "inst_var_remap_all_placements", test_inst_var_remap_all_placements },
		{ "inst_init_walker_combo_weapons", test_inst_init_walker_combo_weapons },
		{ "initd_to_variables", test_initd_to_variables },
		{ "initd_conversion_needs_user_mapping", test_initd_conversion_needs_user_mapping },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}
