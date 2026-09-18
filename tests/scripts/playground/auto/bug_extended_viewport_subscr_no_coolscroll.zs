#include "auto/test_runner.zs"

// With every opening/closing wipe rule off (so the playing field slides away
// instead of the subscreen crawling over it) and the dmap's "extended
// viewport" flag on, opening the active subscreen must not jump the playing
// field down by the passive subscreen's height.
generic script bug_extended_viewport_subscr_no_coolscroll
{
	void run()
	{
		Test::Init();
		Game->FFRules[qr_COOLSCROLL] = false;
		Game->FFRules[qr_OVALWIPE] = false;
		Game->FFRules[qr_TRIANGLEWIPE] = false;
		Game->FFRules[qr_SMASWIPE] = false;
		Game->FFRules[qr_FADEBLACKWIPE] = false;
		dmapdata dm = Game->LoadDMapData(Test::TestingDmap);
		dm->Flagset[DMFS_EXTENDEDVIEWPORT] = true;

		// Give the playing field a visible pattern so the snapshots show where
		// it is: every row of every screen in the region gets a different combo.
		int map = dm->Map;
		for (int i = 0; i < 4; i++)
		{
			int screen = (i % 2) + (i / 2) * 16;
			mapdata scr = Game->LoadMapData(map, screen);
			for (int pos = 0; pos < 176; pos++)
				scr->ComboD[pos] = 1 + (pos / 16 + i * 11) % 30;
		}
		Test::loadRegion(0, 2);
		Waitframes(10);
		Game->LoadASubData(dm->ActiveSubscreen)->Open();
		// The subscreen blocks this script until it closes (replay input).
		Waitframes(10);
		Test::End();
	}
}
