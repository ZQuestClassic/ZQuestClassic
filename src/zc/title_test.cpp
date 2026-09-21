// Exercises the title screen's reload paths (`Quit == qRELOAD`, standalone mode), which
// replays cannot reach: they run in test mode, which skips titlescreen() entirely.
//
// Run with `zplayer -test-zc <tests dir>`.

#include "gamedata.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "test_runner/test_runner.h"
#include "zc/saves.h"
#include "zc/title.h"
#include "zc/zelda.h"
#include "zconfig.h"
#include <filesystem>
#include <fmt/format.h>

TestResults test_title_reload([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

	init_zplayer_for_test();

	// A save slot for a game that has already been played, so a reload restores its saved
	// state instead of starting a fresh game.
	gamedata* saved = new gamedata();
	saved->header.qstpath = test_dir + "/replays/playground/playground.qst";
	saved->set_quest(0xFF);
	saved->set_name("Hero");
	saved->set_timevalid(1);
	saved->set_continue_scrn(0xFF);
	saved->set_hasplayed(1);
	saved->set_counter(123, crMONEY);
	save_t* save = saves_create_test_slot(saved, "");

	tr.total++;
	if (auto r = saves_select(save); !r)
	{
		fmt::println("failed: saves_select: {}", r.error());
		tr.failed++;
		return tr;
	}

	// Diverge the running game from its save, then reload the way instant reload on death
	// and Game->Reload() do.
	game->set_counter(7, crMONEY);
	Quit = qRELOAD;
	titlescreen(0);

	tr.total++;
	if (Quit != 0 || !GameLoaded)
	{
		fmt::println("failed: reload did not start the game (Quit={}, GameLoaded={})", Quit, GameLoaded);
		tr.failed++;
	}

	tr.total++;
	if (saves_current_selection() != save->index)
	{
		fmt::println("failed: reload lost the selected save slot ({})", saves_current_selection());
		tr.failed++;
	}

	tr.total++;
	if (game->get_counter(crMONEY) != 123)
	{
		fmt::println("failed: reload did not restore the saved state (money={})", game->get_counter(crMONEY));
		tr.failed++;
	}

	return tr;
}

// Standalone mode (`-standalone quest.qst`) has one save slot and no file select screen, so
// every return to titlescreen() - quitting without saving, dying without a continue screen -
// must load that slot again. This used to survive exactly one such return (the startup code
// set `-slot 1` as a side effect, which titlescreen() consumes on first use) and then die
// with "Failed to load save: init_game" on the next one.
TestResults test_title_standalone([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

	init_zplayer_for_test();

	// A fresh config would otherwise pop the "Upload replays?" prompt from titlescreen().
	zc_set_config("zeldadx", "replay_upload_prompt", 1);

	standalone_mode = true;
	standalone_quest = test_dir + "/replays/playground/playground.qst";
	standalone_save_path = "test-title-standalone.sav";
	std::filesystem::path save_path = std::filesystem::path(zc_get_config("zeldadx", "save_folder", "saves")) / standalone_save_path;
	std::filesystem::remove(save_path);

	// The save must really reach disk: quitting unloads the slot and reads it back. The
	// debugger tests run the main loop with a replay, which turns disk writes off.
	bool prev_disable_save_to_disk = disable_save_to_disk;
	disable_save_to_disk = false;

	auto cleanup = [&] {
		saves_unselect();
		disable_save_to_disk = prev_disable_save_to_disk;
		std::filesystem::remove(save_path);
		standalone_mode = false;
		standalone_quest.clear();
		standalone_save_path.clear();
	};

	// Drop whatever slot the previous test left selected; saves_load() replaces the slots.
	saves_unselect();

	std::string err;
	tr.total++;
	if (!saves_load(err))
	{
		fmt::println("failed: saves_load: {}", err);
		tr.failed++;
		cleanup();
		return tr;
	}

	auto check_loaded = [&] (const char* what) {
		tr.total++;
		if (Quit != 0 || !GameLoaded || saves_current_selection() != 0)
		{
			fmt::println("failed: {} did not load the standalone save (Quit={}, GameLoaded={}, selection={})",
				what, Quit, GameLoaded, saves_current_selection());
			tr.failed++;
		}
	};

	// First launch.
	titlescreen(0);
	check_loaded("first launch");

	// Quit without saving, twice - the sequence from the bug report.
	for (int i = 0; i < 2; i++)
	{
		Quit = qQUIT;
		titlescreen(0);
		check_loaded(i == 0 ? "first quit" : "second quit");
	}

	cleanup();
	return tr;
}
