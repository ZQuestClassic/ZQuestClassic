// Covers mapdata->isSolidLayer() for a screen from Game->LoadMapData(). It used to look
// up the next layer up, and the wrong map.

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

		combodata cd = Game->LoadComboData(CMB_SOLID);
		cd->Type = CT_NONE;
		cd->Walk = 0xF;

		mapdata layer1 = Game->LoadMapData(map, SCR_LAYER1);
		layer1->Valid = 1;
		layer1->ComboD[ComboAt(X1, Y1)] = CMB_SOLID;
		mapdata layer2 = Game->LoadMapData(map, SCR_LAYER2);
		layer2->Valid = 1;
		layer2->ComboD[ComboAt(X2, Y2)] = CMB_SOLID;

		mapdata scr = Game->LoadMapData(map, 2);
		scr->Valid = 1;
		scr->LayerMap[1] = map;
		scr->LayerScreen[1] = SCR_LAYER1;
		scr->LayerMap[2] = map;
		scr->LayerScreen[2] = SCR_LAYER2;

		// A layer the screen doesn't have is solid.
		char32 buf[100];
		for (int layer = 0; layer <= 6; layer++)
		{
			bool missing = layer > 2;
			sprintf(buf, "layer %d, spot 1", layer);
			Test::AssertEqual(scr->isSolidLayer(X1, Y1, layer), missing || layer == 1, buf);
			sprintf(buf, "layer %d, spot 2", layer);
			Test::AssertEqual(scr->isSolidLayer(X2, Y2, layer), missing || layer == 2, buf);
		}

		Test::End();
	}
}
