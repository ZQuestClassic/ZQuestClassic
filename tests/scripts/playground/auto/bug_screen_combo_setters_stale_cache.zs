// The engine reads a combo's type through a cached copy of the combo data,
// which every writer has to refresh. mapdata->ComboT[] refreshed the copy of
// combo 1 instead of the combo it wrote to (and no copy at all for a screen
// that isn't currently loaded), so the engine kept using the old type.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_screen_combo_setters_stale_cache
{
	const int CMB_A = 3510;
	const int CMB_B = 3511;
	const int CMB_C = 3512;
	const int CMB_D = 3513;
	const int POS = 2 + 2 * 16;

	void testComboT()
	{
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;

		// Through the current region.
		Screen->ComboD[POS] = CMB_A;
		Waitframe();
		Game->LoadTempScreen(0)->ComboT[POS] = CT_SWITCH;
		Test::AssertEqual(Game->LoadComboData(CMB_A)->Type, CT_SWITCH, "the type is written");
		Test::AssertEqual(Screen->ComboT[POS], CT_SWITCH, "the engine sees the new type");

		// Through a screen that isn't loaded.
		mapdata md = Game->LoadMapData(map, 1);
		md->ComboD[POS] = CMB_B;
		md->ComboT[POS] = CT_SWITCH;
		Test::AssertEqual(Game->LoadComboData(CMB_B)->Type, CT_SWITCH, "the type is written");
		Test::AssertEqual(md->ComboT[POS], CT_SWITCH, "the engine sees the new type");
		Screen->ComboD[POS] = CMB_B;
		Waitframe();
		Test::AssertEqual(Screen->ComboT[POS], CT_SWITCH, "wherever the combo is placed");
	}

	void run()
	{
		Test::Init();
		testComboT();
		Test::End();
	}
}
