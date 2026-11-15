// Setting the number of subscreens from a script resizes the engine's subscreen lists. That moves
// the subscreens in memory, and the engine kept using the old location of the dmap's current
// subscreens - drawing them, and opening the active one - which could crash. Shrinking a list
// past the current subscreen likewise left the engine using a subscreen that no longer exists.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_subscreen_count_resize
{
	// The subscreen-open check runs in Hero.animate(), right after this timing phase.
	void press_start()
	{
		WaitTo(SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT);
		Input->Button[CB_START] = true;
	}

	void run()
	{
		Test::Init();

		dmapdata dm = Game->LoadDMapData(Game->CurDMap);
		int active_sub = dm->ActiveSubscreen;
		int passive_sub = dm->PassiveSubscreen;
		Test::Assert(active_sub >= 0 && active_sub < Game->NumActiveSubscreens, "dmap has an active subscreen");
		Test::Assert(passive_sub >= 0 && passive_sub < Game->NumPassiveSubscreens, "dmap has a passive subscreen");

		// Grow every list as far as it goes, so each one moves to new memory.
		Game->NumActiveSubscreens = 256;
		Game->NumPassiveSubscreens = 256;
		Game->NumOverlaySubscreens = 256;
		Test::AssertEqual(Game->NumActiveSubscreens, 256);

		// The passive and overlay subscreens are drawn every frame.
		Waitframes(5);

		// Removing the dmap's active subscreen leaves no active subscreen to open. (If one opened,
		// this script would not run again until it closed.)
		Game->NumActiveSubscreens = active_sub;
		for (int i = 0; i < 10; i++)
		{
			press_start();
			Waitframe();
		}
		Test::Assert(!Game->ActiveSubscreenOpen, "doesn't open an active subscreen that was removed");

		// Removing the dmap's passive subscreen leaves none to draw.
		Game->NumPassiveSubscreens = passive_sub;
		Waitframes(5);

		Test::End();
	}
}
