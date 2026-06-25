// Validates that bitmaps a subscreen script takes ownership of are released when the engine
// subscreen closes. Previously the active subscreen engine did not clear its script-owned
// objects on close, so repeatedly opening a subscreen that owns bitmaps would exhaust the
// 256-bitmap pool (the originally reported bug).

#include "std.zh"
#include "auto/test_runner.zs"

// Each open owns this many bitmaps. With the fix they are freed on close (peak stays well
// under MAX_USER_BITMAPS == 256); without it they accumulate and exhaust the pool within a
// few opens.
const int BITMAPS_PER_OPEN = 100;
const int TARGET_OPENS = 4;

int engine_sub_opens = 0;
bool engine_sub_bitmap_alloc_failed = false;

// Runs as the engine active subscreen. Owns a batch of bitmaps, then closes itself. The engine
// must release those owned bitmaps when the subscreen closes.
subscreendata script EngineSubOwnSubscreen
{
	void run()
	{
		for (int i = 0; i < BITMAPS_PER_OPEN; i++)
		{
			bitmap b = new bitmap(8, 8);
			if (!b->isAllocated())
				engine_sub_bitmap_alloc_failed = true;
			b->Own();
		}
		engine_sub_opens++;
		this->Close();
	}
}

generic script subscreen_owned_clear
{
	// Presses a button so the engine sees it this frame. The subscreen-open check runs in
	// Hero.animate(), and POST_OLD_ITEMDATA_SCRIPT is the timing phase immediately before it,
	// so the press isn't consumed by anything in between.
	void press(int button)
	{
		WaitTo(SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT);
		Input->Button[button] = true;
	}

	void run()
	{
		Test::Init();

		// Point the current dmap's engine active subscreen at our script.
		int active_sub = Game->LoadDMapData(Game->CurDMap)->ActiveSubscreen;
		Game->LoadASubData(active_sub)->Script =
			Game->GetSubscreenScript("EngineSubOwnSubscreen");

		// Open (and self-close) the subscreen several times. Each open owns a batch of bitmaps;
		// if closing leaks them, the bitmap pool exhausts and an allocation eventually fails.
		while (engine_sub_opens < TARGET_OPENS)
		{
			press(CB_START);
			Waitframe();
		}

		Test::Assert(!engine_sub_bitmap_alloc_failed, "owned bitmaps released when subscreen closes");

		Test::End();
	}
}
