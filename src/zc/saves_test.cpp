#include "gamedata.h"
#include "test_runner/test_runner.h"
#include "zc/saves.h"
#include <cstring>

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }

template <typename T>
static bool gd_compare(T a, T b)
{
	return a == b;
}

static bool gd_compare(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

TestResults test_saves(bool verbose)
{
	TestResults tr{};

	int fake_errno = 0;
	allegro_errno = &fake_errno;

	// For some reason MSVC hangs on compiling gamedata==
#ifndef _WIN32
	// First make sure there are not accidentally equalities that are impossible,
	// like fields that have pointers to objects.
	gamedata* g1 = new gamedata();
	gamedata* g2 = new gamedata();
	g1->saved_mirror_portal.clearUID();
	g2->saved_mirror_portal.clearUID();
	if (*g1 != *g2)
	{
		delete g1;
		delete g2;
		printf("failed: g1 == g2\n");
		tr.total++;
		tr.failed++;
		return tr;
	}
	delete g1;
	delete g2;
#endif

	gamedata* game = new gamedata();
	game->header.qstpath = "somegame.qst";
	game->header.title = "I am a test";
	game->header.name = "test";
	game->header.deaths = 10;
	game->header.has_played = true;
	game->header.life = game->get_life();
	game->header.maxlife = game->get_maxlife();
	game->header.hp_per_heart_container = game->get_hp_per_heart();
	game->OverrideItems[0] = 0;
	game->OverrideItems[511] = 511;
	// Does not persist.
	// game->header.did_cheat = true;

	save_t save;
	save.game = game;
	save.header = &game->header;
	save.path = "test.sav";
	save.write_to_disk = true;

	std::string err;
	tr.total++;
	if (!_write_save(&save, err))
	{
		printf("failed write_save: %s\n", err.c_str());
		tr.failed++;
		return tr;
	}

	save.game = nullptr;
	save.header = nullptr;

	if (!_load_from_save_file_expect_one(ReadMode::All, save.path, save, err))
	{
		printf("failed load_from_save_file_expect_one: %s\n", err.c_str());
		return {1, 1};
	}

	#define SAVE_TEST_FIELD(field) tr.total++; if (!gd_compare(game->field, save.game->field)) {\
		printf("game->%s != save.game->%s\n", #field, #field);\
		printf("%s\n", fmt::format("{} != {}", game->field, save.game->field).c_str());\
		tr.failed++;\
	}

	#define SAVE_TEST_FIELD_NOFMT(field) tr.total++; if (!gd_compare(game->field, save.game->field)) {\
		printf("game->%s != save.game->%s\n", #field, #field);\
		tr.failed++;\
	}

	#define SAVE_TEST_VECTOR(field) tr.total++; for (int i = 0; i < game->field.size(); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			printf("%s\n", fmt::format("{} != {}", game->field[i], save.game->field[i]).c_str());\
			tr.failed++;\
		}\
	}

	#define SAVE_TEST_VECTOR_NOFMT(field) tr.total++; for (int i = 0; i < game->field.size(); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			tr.failed++;\
		}\
	}
	
	#define SAVE_TEST_VECTOR_2D(field) tr.total++; for (int i = 0; i < game->field.size(); ++i) {\
		for(int j = 0; j < game->field[i].size(); ++j) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				printf("%s\n", fmt::format("{} != {}", game->field[i][j], save.game->field[i][j]).c_str());\
				tr.failed++;\
			}\
		}\
	}
	
	#define SAVE_TEST_BITSTRING(field) \
		SAVE_TEST_VECTOR(field.inner())

	#define SAVE_TEST_ARRAY(field) tr.total++; for (int i = 0; i < countof(game->field); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			printf("%s\n", fmt::format("{} != {}", game->field[i], save.game->field[i]).c_str());\
			tr.failed++;\
		}\
	}

	#define SAVE_TEST_ARRAY_NOFMT(field) tr.total++; for (int i = 0; i < countof(game->field); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			tr.failed++;\
		}\
	}

	#define SAVE_TEST_ARRAY_2D(field) tr.total++; for (int i = 0; i < countof(game->field); i++) {\
		for (int j = 0; j < countof(game->field[0]); j++) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				printf("%s\n", fmt::format("{} != {}", game->field[i][j], save.game->field[i][j]).c_str());\
				tr.failed++;\
			}\
		}\
	}

	#define SAVE_TEST_ARRAY_2D_NOFMT(field) tr.total++; for (int i = 0; i < countof(game->field); i++) {\
		for (int j = 0; j < countof(game->field[0]); j++) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				tr.failed++;\
			}\
		}\
	}

	// Test some (but not all ... I got lazy) of the fields, in the order found in the save format.
	SAVE_TEST_FIELD(get_name());
	SAVE_TEST_FIELD(get_quest());
	SAVE_TEST_FIELD(get_deaths());
	SAVE_TEST_FIELD(_cheat);
	SAVE_TEST_BITSTRING(items_owned);
	SAVE_TEST_ARRAY(version);
	SAVE_TEST_FIELD(get_hasplayed());
	SAVE_TEST_FIELD(get_time());
	SAVE_TEST_FIELD(get_timevalid());
	SAVE_TEST_VECTOR(lvlitems);
	SAVE_TEST_FIELD(get_continue_scrn());
	SAVE_TEST_FIELD(get_continue_dmap());
	SAVE_TEST_ARRAY(visited);
	SAVE_TEST_VECTOR(bmaps);
	SAVE_TEST_VECTOR(maps);
	SAVE_TEST_VECTOR(guys);
	SAVE_TEST_VECTOR(lvlkeys);
	SAVE_TEST_VECTOR_2D(screen_d);
	SAVE_TEST_ARRAY(global_d);
	SAVE_TEST_ARRAY(_counter);
	SAVE_TEST_ARRAY(_maxcounter);
	SAVE_TEST_ARRAY(_dcounter);
	SAVE_TEST_FIELD(awpn);
	SAVE_TEST_FIELD(bwpn);
	SAVE_TEST_FIELD_NOFMT(globalRAM);
	SAVE_TEST_FIELD(forced_awpn);
	SAVE_TEST_FIELD(forced_bwpn);
	SAVE_TEST_FIELD(forced_xwpn);
	SAVE_TEST_FIELD(forced_ywpn);
	SAVE_TEST_FIELD(xwpn);
	SAVE_TEST_FIELD(ywpn);
	SAVE_TEST_VECTOR(lvlswitches);
	SAVE_TEST_BITSTRING(item_messages_played);
	SAVE_TEST_ARRAY(bottleSlots);

	game->saved_mirror_portal.clearUID();
	save.game->saved_mirror_portal.clearUID();
	tr.total++;
	if (game->saved_mirror_portal != save.game->saved_mirror_portal)
	{
		printf("game->saved_mirror_portal != save.game->saved_mirror_portal\n");
		tr.failed++;
		return tr;
	}

	SAVE_TEST_BITSTRING(gen_doscript);
	SAVE_TEST_VECTOR(gen_exitState);
	SAVE_TEST_VECTOR(gen_reloadState);
	SAVE_TEST_VECTOR_2D(gen_initd);
	SAVE_TEST_VECTOR_2D(gen_data);
	SAVE_TEST_VECTOR_2D(screen_data);
	SAVE_TEST_VECTOR(xstates);
	SAVE_TEST_VECTOR_2D(xdoors);
	SAVE_TEST_VECTOR(gen_eventstate);
	SAVE_TEST_VECTOR(gswitch_timers);
	SAVE_TEST_VECTOR_NOFMT(compat_saved_user_objects);
	SAVE_TEST_VECTOR_NOFMT(user_portals);
	SAVE_TEST_VECTOR(OverrideItems);

	// Now do the header.
	tr.total++;
	if (game->header != save.game->header)
	{
		printf("game->header != save.game->header\n");
		tr.failed++;
		return tr;
	}

#ifndef _WIN32
	// Now do the entire thing.
	tr.total++;
	if (*game != *save.game)
	{
		printf("game != save.game\n");
		tr.failed++;
		return tr;
	}
#endif

	return tr;
}
