// Exercises the title screen's reload path (`Quit == qRELOAD`), which replays cannot reach:
// they run in test mode, which skips titlescreen() entirely.
//
// Run with `zplayer -test-zc <tests dir>`.

#include "gamedata.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "test_runner/test_runner.h"
#include "zc/saves.h"
#include "zc/title.h"
#include "zc/zelda.h"
#include <fmt/format.h>

TestResults test_title_reload([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

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
