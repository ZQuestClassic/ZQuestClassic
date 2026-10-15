// Editor tests. Run via `zeditor -test-zc <tests dir>` (see
// zeditor_handle_commands); tests/test_zeditor.py drives that.

#include "core/combo.h"
#include "core/mapscr.h"
#include "core/misctypes.h"
#include "core/qst.h"
#include "ffc.h"
#include "items.h"
#include "sprite_data.h"
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
extern script_data *lwpnscripts[NUMSCRIPTWEAPONS];
extern script_data *ewpnscripts[NUMSCRIPTWEAPONS];

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
	for_each_inst_init([&](script_data** scripts, int slot, std::map<word, int>& inst_init) {
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
		{ "inst_init_walker_combo_weapons", test_inst_init_walker_combo_weapons },
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
