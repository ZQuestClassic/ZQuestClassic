#include "std.zh"
#include "auto/test_runner.zs"

// A sprite created by a script while the screen scrolled, before the new screen loaded, kept
// belonging to the old screen after the scroll. A script on it using `Screen->` then crashed.
generic script bug_scroll_created_sprite_screen
{
	void run()
	{
		Test::Init();

		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 1)->Valid = 1;
		Waitframe();

		Hero->Dir = DIR_RIGHT;
		Hero->WarpEx(WT_SCROLLING, Test::TestingDmap, 1, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframe();
		Test::AssertEqual(Game->GetCurScreen(), 0, "screen when creating the eweapon");
		Test::Assert(Game->Scrolling[SCROLL_DIR] > -1, "scrolling when creating the eweapon");

		eweapon e = Screen->CreateEWeapon(EW_SCRIPT1);
		e->X = 120;
		e->Y = 80;
		e->Script = Game->GetEWeaponScript("bug_scroll_created_sprite_screen_ewpn");

		while (Game->GetCurScreen() != 1 || Game->Scrolling[SCROLL_DIR] > -1)
			Waitframe();
		Waitframe();

		Test::Assert(e->isValid(), "eweapon kept");
		Test::AssertEqual(e->Misc[0], 1, "eweapon's screen after scrolling");

		Test::End();
	}
}

eweapon script bug_scroll_created_sprite_screen_ewpn
{
	void run()
	{
		while (true)
		{
			this->Misc[0] = Screen->Index;
			Waitframe();
		}
	}
}
