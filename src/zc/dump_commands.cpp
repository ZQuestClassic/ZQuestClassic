// Read-only inspection commands for quest files.

#include "zc/commands.h"
#include "base/util.h"
#include "base/zapp.h"
#include "core/dmap.h"
#include "core/mapscr.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "core/trigger_graph.h"
#include "ffc.h"
#include "core/misctypes.h"
#include "zc/zc_sys.h"
#include "zc/zelda.h"
#include "zalleg/zalleg.h"
#include "zinfo.h"
#include <fmt/format.h>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Loads just enough to inspect quest data: no scripts compiled, no JIT, no save file.
static void load_quest_for_inspection(const char* quest_path)
{
	set_headless_mode();

	// Console output is muted for -dump-* commands during startup so that stdout holds
	// only the dump; the load still writes to allegro.log if something needs diagnosing.
	byte skip_flags[] = {0, 0, 0, 0};
	int ret = loadquest(quest_path, &QHeader, &QMisc, tunes + ZC_MIDI_COUNT, false, skip_flags, 0, false);
	if (ret)
	{
		fprintf(stderr, "failed to load %s: %s\n", quest_path, qst_error[ret]);
		zapp_exit(ret);
	}

	strcpy(qstpath, quest_path);
}

void do_dump_qrs_command(const char* quest_path)
{
	load_quest_for_inspection(quest_path);

	// Everything identifying goes to stderr so that stdout is a clean, diffable list.
	fprintf(stderr, "%s: %s (zelda_version=0x%x build=%d)\n", quest_path,
		QHeader.getVerStr(), QHeader.zelda_version, QHeader.build);

	for (int q = 0; q < qr_MAX; q++)
	{
		// The enum has reserved gaps with no constant; report them by value so the
		// output stays a complete, stable list to diff against.
		if (const char* name = qr_name(q))
			printf("%s %d\n", name, get_qr(q) ? 1 : 0);
		else
			printf("qr_%d %d\n", q, get_qr(q) ? 1 : 0);
	}

	zapp_exit(0);
}

void do_dump_dmaps_command(const char* quest_path)
{
	load_quest_for_inspection(quest_path);

	// One line per dmap: index, map, level, type and name. Handy for finding which
	// dmap to start a test-mode replay on for a given map.
	for (int i = 0; i < MAXDMAPS; i++)
	{
		const dmap& d = DMaps[i];
		if (d.map == 0 && d.level == 0 && d.name[0] == 0 && d.title.empty())
			continue;
		printf("%d map=%d level=%d type=%d xoff=%d name=\"%s\" title=\"%s\"\n",
			i, d.map, d.level, d.type, d.xoff, d.name, d.title.c_str());
	}

	zapp_exit(0);
}

static json dump_combo_position(const mapscr& scr, int pos)
{
	int cid = scr.data[pos];
	const newcombo& cmb = combobuf[cid];

	json j;
	j["pos"] = pos;
	j["x"] = pos % 16;
	j["y"] = pos / 16;
	j["combo"] = cid;
	j["cset"] = scr.cset[pos];
	j["type"] = cmb.type;
	j["type_name"] = ZI.getComboTypeName(cmb.type);
	j["sflag"] = scr.sflag[pos];
	j["sflag_name"] = ZI.getMapFlagName(scr.sflag[pos]);
	// The combo's own inherent flag, which is easy to miss and behaves like a flag
	// placed on every screen position using that combo.
	j["cflag"] = cmb.flag;
	j["cflag_name"] = ZI.getMapFlagName(cmb.flag);
	j["walk"] = cmb.walk;
	j["usrflags"] = cmb.usrflags;
	return j;
}

static json ffc_flag_names(uint32_t flags)
{
	static const std::pair<uint32_t, const char*> names[] = {
		{ffc_overlay, "overlay"}, {ffc_trans, "trans"}, {ffc_solid, "solid"},
		{ffc_carryover, "carryover"}, {ffc_stationary, "stationary"}, {ffc_changer, "changer"},
		{ffc_preload, "preload"}, {ffc_lensvis, "lensvis"}, {ffc_scriptreset, "scriptreset"},
		{ffc_ethereal, "ethereal"}, {ffc_ignoreholdup, "ignoreholdup"},
		{ffc_ignorechanger, "ignorechanger"}, {ffc_imprecisionchanger, "imprecisionchanger"},
		{ffc_lensinvis, "lensinvis"}, {ffc_platform, "platform"}, {ffc_cmb_solid, "cmb_solid"},
		{ffc_changespeed, "changespeed"}, {ffc_changethis, "changethis"},
		{ffc_changeprev, "changeprev"}, {ffc_changenext, "changenext"},
		{ffc_swapprev, "swapprev"}, {ffc_swapnext, "swapnext"},
	};

	json j = json::array();
	for (auto [flag, name] : names)
	{
		if (flags & flag)
		{
			j.push_back(name);
			flags &= ~flag;
		}
	}
	// Bits with no name, so nothing set goes unreported.
	if (flags)
		j.push_back(fmt::format("0x{:x}", flags));
	return j;
}

void do_dump_screen_command(const char* quest_path, int map, int screen)
{
	load_quest_for_inspection(quest_path);

	if (unsigned(map) >= map_count || unsigned(screen) >= MAPSCRS)
	{
		fprintf(stderr, "-dump-screen: quest has %d maps and %d screens per map; got map %d, screen %d\n",
			map_count, MAPSCRS, map, screen);
		zapp_exit(1);
	}

	mapscr& scr = TheMaps[map * MAPSCRS + screen];

	json j;
	j["quest"] = quest_path;
	j["version"] = QHeader.getVerStr();
	j["map"] = map;
	j["screen"] = screen;
	j["valid"] = scr.is_valid();

	// Screen flags are raw bytes; the fSOMETHING names are plain #defines with no
	// table to decode against, so emit the numbers and let the caller consult mapscr.h.
	j["flags"] = {scr.flags, scr.flags2, scr.flags3, scr.flags4, scr.flags5, scr.flags6,
		scr.flags7, scr.flags8, scr.flags9, scr.flags10, scr.flags11};

	j["undercombo"] = scr.undercombo;
	j["undercset"] = scr.undercset;
	j["guy"] = scr.guy;
	j["room"] = scr.room;
	j["item"] = scr.item;
	j["hasitem"] = scr.hasitem;
	j["enemies"] = json::array();
	for (int i = 0; i < 10; i++)
		if (scr.enemy[i])
			j["enemies"].push_back(scr.enemy[i]);

	j["layers"] = json::array();
	for (int i = 0; i < 6; i++)
	{
		// Stored one-based, with 0 meaning "no layer".
		if (!scr.layermap[i])
			continue;

		j["layers"].push_back({{"layer", i + 1}, {"map", scr.layermap[i] - 1}, {"screen", scr.layerscreen[i]}});
	}

	// Only positions with something on them, so a screen is readable at a glance.
	j["combos"] = json::array();
	for (int pos = 0; pos < 176; pos++)
	{
		if (!scr.data[pos] && !scr.sflag[pos])
			continue;

		j["combos"].push_back(dump_combo_position(scr, pos));
	}

	j["secrets"] = json::array();
	for (int i = 0; i < 128; i++)
	{
		if (!scr.secretcombo[i])
			continue;

		const newcombo& cmb = combobuf[scr.secretcombo[i]];
		j["secrets"].push_back({
			{"index", i},
			{"combo", scr.secretcombo[i]},
			{"cset", scr.secretcset[i]},
			{"type", cmb.type},
			{"type_name", ZI.getComboTypeName(cmb.type)},
			// The flag the position ends up with once the secret triggers, and the
			// revealed combo's own inherent flag.
			{"flag", scr.secretflag[i]},
			{"flag_name", ZI.getMapFlagName(scr.secretflag[i])},
			{"cflag", cmb.flag},
			{"cflag_name", ZI.getMapFlagName(cmb.flag)},
		});
	}

	j["ffcs"] = json::array();
	for (word i = 0; i < scr.numFFC(); i++)
	{
		ffcdata& ffc = scr.getFFC(i);
		// A changer can be placed with no combo, so keep any ffc with flags set.
		if (!ffc.data && !ffc.flags)
			continue;

		const newcombo& cmb = combobuf[ffc.data];
		j["ffcs"].push_back({
			{"index", i},
			{"combo", ffc.data},
			{"cset", ffc.cset},
			{"type", cmb.type},
			{"type_name", ZI.getComboTypeName(cmb.type)},
			{"flags", ffc_flag_names(ffc.flags)},
			{"x", ffc.x.getInt()},
			{"y", ffc.y.getInt()},
			{"vx", ffc.vx.getFloat()},
			{"vy", ffc.vy.getFloat()},
			{"ax", ffc.ax.getFloat()},
			{"ay", ffc.ay.getFloat()},
			// The editor's "Combo W./H." - the area, in pixels, where the combo type
			// takes effect (damage, solidity, ...). Separate from the drawn tile size.
			{"effect_width", scr.ffEffectWidth(i)},
			{"effect_height", scr.ffEffectHeight(i)},
			{"tile_width", scr.ffTileWidth(i)},
			{"tile_height", scr.ffTileHeight(i)},
			{"delay", ffc.delay},
			{"link", ffc.link},
			{"layer", ffc.layer},
			{"script", ffc.scrconfig.script},
			{"initd", ffc.scrconfig.run_args},
		});
	}

	// Triggers of every combo the screen uses, keyed by combo id. Triggers can act on
	// their own every frame (trigger groups, timers, ...), so they explain a lot of
	// "this screen does something odd" reports.
	std::set<int> cids;
	for (int pos = 0; pos < 176; pos++)
		cids.insert(scr.data[pos]);
	for (int i = 0; i < 128; i++)
		cids.insert(scr.secretcombo[i]);
	for (word i = 0; i < scr.numFFC(); i++)
		cids.insert(scr.getFFC(i).data);
	cids.insert(scr.undercombo);

	j["combo_triggers"] = json::object();
	for (int cid : cids)
	{
		const newcombo& cmb = combobuf[cid];
		if (!cid || cmb.triggers.empty())
			continue;

		json triggers = json::array();
		for (const auto& trig : cmb.triggers)
			triggers.push_back(util::split(trig.summarize(cmb), "\n"));
		j["combo_triggers"][std::to_string(cid)] = triggers;
	}

	// The same triggers as a graph of the combos, their combo changes and what links them
	// (copycats, trigger groups, states, secrets), with every layer and FFC: what the
	// editor's Combo Triggers diagram draws.
	std::array<mapscr*, 7> layers = {&scr};
	for (int i = 0; i < 6; i++)
		if (scr.layermap[i] && scr.layermap[i] <= map_count)
			layers[i + 1] = &TheMaps[(scr.layermap[i] - 1) * MAPSCRS + scr.layerscreen[i]];
	auto graph = build_trigger_graph(layers);
	j["trigger_graph"] = trigger_graph_json(graph);

	printf("%s\n", j.dump(2).c_str());
	zapp_exit(0);
}
