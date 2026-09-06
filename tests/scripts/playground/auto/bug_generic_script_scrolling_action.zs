#include "std.zh"
#include "auto/test_runner.zs"

// Generic scripts only saw Hero->Action == LA_SCROLLING when they ran from the
// scrolling-script timings. At the other timings during a scroll (ex: the start
// and end of each frame), the action was whatever the player was doing before
// the scroll started.
generic script bug_generic_script_scrolling_action
{
	void run()
	{
		Test::Init();

		// Make the screen to the right valid, so a scrolling warp can go there.
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 1)->Valid = 1;

		Hero->Dir = DIR_RIGHT;
		Hero->WarpEx(WT_SCROLLING, Test::TestingDmap, 1, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);

		int frames_scrolling = 0;
		int frames_scrolling_end = 0;
		int i = 0;
		while (i++ < 120)
		{
			if (Game->Scrolling[SCROLL_DIR] > -1)
			{
				frames_scrolling++;
				Test::AssertEqual(Hero->Action, LA_SCROLLING, "start of frame while scrolling");
				WaitTo(SCR_TIMING_END_FRAME);
				// The scroll may have just finished, in which case the player is no longer scrolling.
				if (Game->Scrolling[SCROLL_DIR] > -1)
				{
					frames_scrolling_end++;
					Test::AssertEqual(Hero->Action, LA_SCROLLING, "end of frame while scrolling");
				}
			}
			Waitframe();
		}

		Test::Assert(frames_scrolling > 10, "never saw scrolling");
		Test::Assert(frames_scrolling_end > 10, "never saw scrolling at the end of a frame");
		Test::AssertEqual(Game->GetCurScreen(), 1, "warp scrolled to screen 1");

		Test::End();
	}
}
