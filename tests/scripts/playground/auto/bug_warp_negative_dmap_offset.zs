#include "std.zh"
#include "auto/test_runner.zs"

// Small DMaps can have a negative map offset, which moves their left columns off the
// edge of the map. Warping to one of those columns crashed the player.
generic script bug_warp_negative_dmap_offset
{
	void run()
	{
		Test::Init();

		dmapdata dm = Game->LoadDMapData(Test::TestingDmap);
		dm->Offset = -7;

		// A scripted warp to screen 0x00 is cancelled.
		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 0x00, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframes(5);
		Test::AssertEqual(Game->CurScreen, 0, "screen after WarpEx");

		// So is a side warp, here from an auto warp combo.
		Screen->SetSideWarp(0, 0x00, Test::TestingDmap, WT_IWARP);
		combodata cd = Game->LoadComboData(65000);
		cd->Type = CT_AWARPA;
		Screen->ComboD[0] = 65000;
		Waitframes(5);
		Test::AssertEqual(Game->CurScreen, 0, "screen after auto warp");

		// Warps that stay on the map still work.
		Screen->ComboD[0] = 0;
		Game->LoadMapData(dm->Map, 2)->Valid = 1;
		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 0x09, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		while (Game->CurScreen != 2)
			Waitframe();

		Test::End();
	}
}
