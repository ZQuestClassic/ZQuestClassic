#include "std.zh"
#include "auto/test_runner.zs"

// An insta-warp from a screen with "Sprites Carry Over In Warps" moved every kept sprite by its
// offset from the old region's origin, clamped to one screen past the new region's edge. That
// also moved carryover ffcs, which the new screen had already placed, so a script on one reading
// `Screen->` crashed (seen in Yuurand's Lunar Tower portals).
generic script bug_warp_kept_sprites_screen
{
	const int CMB_WARP = 3310;

	void run()
	{
		Test::Init();

		// A 3x1 region at screen 0, warping to the 2x1 region at screen 3.
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		for (int screen = 0; screen < 5; screen++)
		{
			mapdata scr = Game->LoadMapData(map, screen);
			scr->Valid = 1;
			scr->RegionID = screen < 3 ? 1 : 2;
		}
		Player->Warp(Test::TestingDmap, 0);
		Waitframe();

		ffc f = Screen->LoadFFC(1);
		f->Data = 1;
		f->X = 64;
		f->Y = 64;
		f->Flags[FFCF_CARRYOVER] = true;

		npc n = Screen->CreateNPC(NPC_DARKNUT1);
		n->X = 120;
		n->Y = 32;
		n->SpawnScreen = 2;
		Test::AssertEqual(n->SpawnScreen, 2, "npc's screen before warping");

		Screen->Flag[SFL_WARP_SPRITE_CARRYOVER] = true;
		Screen->SideWarpType[0] = WT_IWARP;
		Screen->SideWarpDMap[0] = Test::TestingDmap;
		Screen->SideWarpScreen[0] = 3;

		combodata cd = Game->LoadComboData(CMB_WARP);
		cd->Type = CT_AWARPA;
		Screen->ComboD[0] = CMB_WARP;

		while (Game->CurScreen != 3)
			Waitframe();
		Waitframe();

		Test::AssertEqual(Screen->LoadFFC(1)->Data, 1, "ffc carried over");
		// The carried ffc belongs to the new origin screen, and the npc to the screen matching its
		// old one, clamped into the new region.
		Test::AssertEqual(Screen->LoadFFC(1)->SpawnScreen, 3, "ffc's screen after warping");
		Test::Assert(n->isValid(), "npc kept");
		Test::AssertEqual(n->SpawnScreen, 4, "npc's screen after warping");

		Test::End();
	}
}
