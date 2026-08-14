#include "auto/test_runner.zs"
#include "std.zh"

// Repro for walkable water at a screen edge blocking scrolling when the water
// can't actually drown the hero - no 'Hero Drowns in Walkable Water' rule and
// no sideview swimming (fixed in 2.55.16 by 41889b1097). The pre-scroll drown
// check treated any water underfoot as "would drown before scrolling" and
// softlocked the hero against the edge.
generic script bug_water_edge_scroll
{
	void run()
	{
		Test::Init();

		int start_screen = Game->GetCurScreen();
		dmapdata dm = Game->LoadDMapData(Game->GetCurDMap());
		mapdata neighbor = Game->LoadMapData(dm->Map, start_screen + 1);
		Test::Assert(neighbor->Valid, "fixture: need a valid screen to the right");

		// The bug needs water that can't drown the hero.
		Game->FFRules[qr_DROWN] = false;

		// Repurpose an unused combo as fully-walkable water.
		combodata cd = Game->LoadComboData(6000);
		cd->Type = CT_WATER;
		cd->Walk = 0;   // no solidity
		cd->Effect = 0xF; // water effect in all quadrants

		// Pave the rightmost column with it, on the rows around the hero.
		for (int row = 3; row <= 5; row++)
			Screen->ComboD[row * 16 + 15] = 6000;

		// Place the hero directly on the edge water: without the drown rule
		// the engine won't let a flipperless hero walk onto deep water, but
		// standing on it is exactly the state the bug softlocked.
		Hero->X = 240;
		Hero->Y = 64;

		// Walk right, off the screen edge.
		for (int i = 0; i < 600 && Game->GetCurScreen() == start_screen; i++)
		{
			Hero->InputRight = true;
			Waitframe();
		}

		Test::Assert(Game->GetCurScreen() != start_screen,
			"hero never scrolled off the walkable water at the screen edge");
		Test::End();
	}
}
