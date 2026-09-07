// Regression test for a solid ffc on a non-origin region screen shoving the
// hero on the first frame after entering the region. Loading a region adds
// the screen's world offset to each ffc's position, but the ffc's stored
// previous position (used by the solid-collision sweep) kept the screen-local
// value. The first solid update then read the offset as a real move and
// pushed anything in the sweep's path.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_region_ffc_solid_first_frame
{
	void run()
	{
		Test::Init();

		// Enter a 2x2 region at screen 0 first, so the warp below resolves its
		// destination inside the region.
		Test::loadRegion(0, 2);
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;

		// Solid ffc on screen 1 (one to the right of the origin), to the right
		// of where the hero will arrive. Its world x is 256 + 176: the phantom
		// move from 176 to 432 sweeps across the hero at 288.
		mapdata scr = Game->LoadMapData(map, 1);
		scr->FFCData[1] = 1;
		scr->FFCX[1] = 176;
		scr->FFCY[1] = 32;
		scr->FFCFlags[1] = FFCBF_SOLID;

		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 1, 32, 32, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframes(3);

		Test::AssertEqual(Hero->X, 256 + 32);
		Test::AssertEqual(Hero->Y, 32);

		Test::End();
	}
}
