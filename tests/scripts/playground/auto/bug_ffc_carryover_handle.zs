#include "std.zh"
#include "auto/test_runner.zs"

// A carryover ffc got a new sprite uid every time the screen changed, so a handle to it stopped
// working after a scroll or warp ("Invalid sprite using UID"), and what it owned was released.
generic script bug_ffc_carryover_handle
{
	void run()
	{
		Test::Init();

		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 1)->Valid = 1;
		Game->LoadMapData(map, 2)->Valid = 1;

		ffc f = Screen->LoadFFC(1);
		f->Data = 1;
		f->X = 64;
		f->Y = 64;
		f->Flags[FFCF_CARRYOVER] = true;

		int[] owned = {1, 2, 3};
		f->OwnArray(owned);
		Waitframe(); // Drain the autorelease pool: only `owned` and the ffc's ownership remain.
		Test::AssertEqual(RefCount(owned), 2L, "refs before scrolling");

		Hero->Dir = DIR_RIGHT;
		Hero->WarpEx(WT_SCROLLING, Test::TestingDmap, 1, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		while (Game->GetCurScreen() != 1 || Game->Scrolling[SCROLL_DIR] > -1)
			Waitframe();
		Waitframe(); // Let the old screens be deleted.

		Test::Assert(f == Screen->LoadFFC(1), "handle changed after scrolling");
		Test::AssertEqual(f->Data, 1, "data after scrolling");
		Test::AssertEqual(RefCount(owned), 2L, "refs after scrolling");

		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 2, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		while (Game->GetCurScreen() != 2)
			Waitframe();
		Waitframe(); // Let the old screens be deleted.

		Test::Assert(f == Screen->LoadFFC(1), "handle changed after warping");
		Test::AssertEqual(f->Data, 1, "data after warping");
		Test::AssertEqual(RefCount(owned), 2L, "refs after warping");

		Test::End();
	}
}
