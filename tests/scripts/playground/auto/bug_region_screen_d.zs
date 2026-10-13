// Regression test for Screen->D[] in a scrolling region. Like the other Screen
// variables, it refers to the script's screen (an ffc's is the screen it was
// spawned on) - but it used to always refer to the region's origin screen.

#include "std.zh"
#include "auto/test_runner.zs"

ffc script bug_region_screen_d_ffc
{
	void run()
	{
		Screen->D[0] = Screen->Index + 100;
	}
}

generic script bug_region_screen_d
{
	void run()
	{
		Test::Init();

		// A 2x2 region at screen 0, with a scripted ffc on screen 1 (one to the
		// right of the origin).
		Test::loadRegion(0, 2);
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		mapdata scr = Game->LoadMapData(map, 1);
		scr->FFCData[1] = 1;
		scr->FFCScript[1] = Game->GetFFCScript("bug_region_screen_d_ffc");

		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 1, 32, 32, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframes(3);

		Test::AssertEqual(Game->GetScreenD(1, 0), 101, "screen 1's D[0]");
		Test::AssertEqual(Game->GetScreenD(0, 0), 0, "origin screen's D[0]");

		Test::End();
	}
}
