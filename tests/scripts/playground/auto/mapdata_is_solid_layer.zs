// Covers mapdata->isSolidLayer() for every kind of mapdata. It used to look up the
// next layer up (and the wrong map) for a canonical screen. For a temporary screen it
// ignored the layer, read the editor's copy of the layer instead of the live one, or
// read the region's first screen for a spot on another of its screens.

#include "std.zh"
#include "auto/test_runner.zs"

generic script mapdata_is_solid_layer
{
	const int CMB_SOLID = 3400;

	// The screens that hold the layers' combos.
	const int SCR_LAYER1 = 0x51;
	const int SCR_LAYER2 = 0x50;

	// Layer 1 is solid at (80, 80), layer 2 at (48, 48).
	const int X1 = 80;
	const int Y1 = 80;
	const int X2 = 48;
	const int Y2 = 48;

	void run()
	{
		Test::Init();

		int map = Game->LoadDMapData(Test::TestingDmap)->Map;

		combodata cd = Test::setupCombo(CMB_SOLID, CT_NONE);
		cd->Walk = 0xF;

		mapdata layer1 = Game->LoadMapData(map, SCR_LAYER1);
		layer1->Valid = 1;
		layer1->ComboD[Test::pos(X1 / 16, Y1 / 16)] = CMB_SOLID;
		mapdata layer2 = Game->LoadMapData(map, SCR_LAYER2);
		layer2->Valid = 1;
		layer2->ComboD[Test::pos(X2 / 16, Y2 / 16)] = CMB_SOLID;

		// Screen 2 has both layers, and screen 3 is valid so that a scroll can go there.
		addLayers(Game->LoadMapData(map, 2), map);
		Game->LoadMapData(map, 3)->Valid = 1;

		Test::Label("canonical");
		checkBoth(Game->LoadMapData(map, 2), 0, true, true, true);

		Player->Warp(Test::TestingDmap, 2);
		Waitframe();

		Test::Label("Screen");
		checkScreen(0, true, true);
		Test::Label("temp region");
		checkBoth(Game->LoadTempScreen(0), 0, true, true);
		Test::Label("temp screen");
		checkBoth(Game->LoadTempScreen(0, 2), 0, true, true);

		// Change the live layer 2. The temporary screens see it, the canonical one doesn't.
		Game->LoadTempScreen(2)->ComboD[Test::pos(X2 / 16, Y2 / 16)] = 0;
		Test::Label("Screen, changed");
		checkScreen(0, true, false);
		Test::Label("temp region, changed");
		checkBoth(Game->LoadTempScreen(0), 0, true, false);
		Test::Label("temp screen, changed");
		checkBoth(Game->LoadTempScreen(0, 2), 0, true, false);
		Test::Label("canonical, changed");
		checkBoth(Game->LoadMapData(map, 2), 0, true, true, true);

		// While scrolling away from screen 2, its screens are the scrolling ones.
		Hero->Dir = DIR_RIGHT;
		Hero->WarpEx(WT_SCROLLING, Test::TestingDmap, 3, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		int i = 0;
		while (Game->Scrolling[SCROLL_DIR] == -1 && i++ < 60)
			Waitframe();
		Test::Assert(Game->Scrolling[SCROLL_DIR] > -1, "never saw scrolling");
		// The scrolling screens are only there once the scroll gets past the frames (up to
		// 32) where it waits before moving.
		Waitframes(32);
		Test::Assert(Game->Scrolling[SCROLL_DIR] > -1, "still scrolling");
		Test::Label("scrolling region");
		checkBoth(Game->LoadScrollingScreen(0), 0, true, false);
		Test::Label("scrolling screen");
		checkBoth(Game->LoadScrollingScreen(0, 2), 0, true, false);
		while (Game->Scrolling[SCROLL_DIR] > -1)
			Waitframe();

		// In a 2x2 region, the layers of screen 1 (to the right of the first screen) are
		// found from the region's mapdata by their world position.
		addLayers(Game->LoadMapData(map, 1), map);
		Test::loadRegion(0, 2);
		Test::Label("Screen, region");
		checkScreen(256, true, true);
		checkScreen(0, false, false);
		Test::Label("temp region, region");
		checkBoth(Game->LoadTempScreen(0), 256, true, true);
		checkBoth(Game->LoadTempScreen(0), 0, false, false);
		Test::Label("temp screen, region");
		checkBoth(Game->LoadTempScreen(0, 1), 0, true, true);
		checkBoth(Game->LoadTempScreen(0, 0), 0, false, false);

		Test::End();
	}

	void addLayers(mapdata scr, int map)
	{
		scr->Valid = 1;
		scr->LayerMap[1] = map;
		scr->LayerScreen[1] = SCR_LAYER1;
		scr->LayerMap[2] = map;
		scr->LayerScreen[2] = SCR_LAYER2;
	}

	// Checks every layer at both spots, where `solid1` and `solid2` say whether layer 1
	// is solid at (X1, Y1) and layer 2 at (X2, Y2). Nothing else is solid, except that a
	// canonical screen says a layer it doesn't have is solid.
	void checkBoth(mapdata m, int dx, bool solid1, bool solid2, bool canonical = false)
	{
		Test::Assert(m, "mapdata is valid");
		for (int layer = 0; layer <= 6; layer++)
		{
			bool missing = canonical && layer > 2;
			Test::AssertEqual(m->isSolidLayer(dx + X1, Y1, layer), missing || (layer == 1 && solid1), what(layer, 1));
			Test::AssertEqual(m->isSolidLayer(dx + X2, Y2, layer), missing || (layer == 2 && solid2), what(layer, 2));
		}
	}

	void checkScreen(int dx, bool solid1, bool solid2)
	{
		for (int layer = 0; layer <= 6; layer++)
		{
			Test::AssertEqual(Screen->isSolidLayer(dx + X1, Y1, layer), layer == 1 && solid1, what(layer, 1));
			Test::AssertEqual(Screen->isSolidLayer(dx + X2, Y2, layer), layer == 2 && solid2, what(layer, 2));
		}
	}

	char32[] what(int layer, int spot)
	{
		char32 buf[100];
		sprintf(buf, "%s: layer %d, spot %d", Test::labelText, layer, spot);
		return buf;
	}
}
