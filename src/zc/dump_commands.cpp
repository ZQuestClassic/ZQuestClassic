// Read-only inspection commands for quest files.

#include "zc/commands.h"
#include "base/zapp.h"
#include "core/dmap.h"
#include "core/mapscr.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "ffc.h"
#include "core/misctypes.h"
#include "zc/zc_sys.h"
#include "zc/zelda.h"
#include "zalleg/zalleg.h"
#include "zinfo.h"
#include <fmt/format.h>
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
		if (!ffc.data)
			continue;

		j["ffcs"].push_back({
			{"index", i},
			{"combo", ffc.data},
			{"cset", ffc.cset},
			{"x", ffc.x.getInt()},
			{"y", ffc.y.getInt()},
			{"vx", ffc.vx.getInt()},
			{"vy", ffc.vy.getInt()},
			{"layer", ffc.layer},
			{"script", ffc.scrconfig.script},
			{"initd", ffc.scrconfig.run_args},
		});
	}

	printf("%s\n", j.dump(2).c_str());
	zapp_exit(0);
}
