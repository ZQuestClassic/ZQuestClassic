// Setting the number of subscreens from a script resizes the engine's subscreen lists. That moves
// the subscreens in memory, and the engine kept using the old location of the dmap's current
// subscreens - drawing them, and opening the active one - which could crash. Shrinking a list
// past the current subscreen likewise left the engine using a subscreen that no longer exists.
// Both can also happen while the active subscreen is open, from its own script.

#include "std.zh"
#include "auto/test_runner.zs"

// What ResizeSubscreenProbe does once the subscreen is open.
enum ResizeProbeMode
{
	RESIZE_PROBE_CLOSE,  // Just close.
	RESIZE_PROBE_GROW,   // Grow every list, then keep the subscreen open for a few frames.
	RESIZE_PROBE_REMOVE, // Remove this subscreen, which closes it.
};

ResizeProbeMode resize_probe_mode = RESIZE_PROBE_CLOSE;
int resize_probe_sub = 0;
int resize_sub_opens = 0;
int resize_sub_frames_after_remove = 0;

subscreendata script ResizeSubscreenProbe
{
	void run()
	{
		resize_sub_opens++;
		if (resize_probe_mode == RESIZE_PROBE_GROW)
		{
			// Every list has to grow from its original size, so each one moves to new memory.
			Game->NumActiveSubscreens = 256;
			Game->NumPassiveSubscreens = 256;
			Game->NumOverlaySubscreens = 256;
			Game->NumMapSubscreens = 256;
			// The open subscreen and the passive and overlay ones are drawn every frame.
			Waitframes(5);
		}
		else if (resize_probe_mode == RESIZE_PROBE_REMOVE)
		{
			Game->NumActiveSubscreens = resize_probe_sub;
			// The subscreen is gone, so this script must not run again.
			Waitframe();
			resize_sub_frames_after_remove++;
		}
		this->Close();
	}
}

generic script bug_subscreen_count_resize
{
	// The subscreen-open check runs in Hero.animate(), right after this timing phase.
	void press_start()
	{
		WaitTo(SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT);
		Input->Button[CB_START] = true;
	}

	// Presses Start until the active subscreen opens, or gives up. Returns whether it opened.
	bool open_active_subscreen()
	{
		int opens = resize_sub_opens;
		for (int i = 0; i < 30 && resize_sub_opens == opens; i++)
		{
			press_start();
			Waitframe();
		}
		return resize_sub_opens != opens;
	}

	void run()
	{
		Test::Init();

		int active_sub = Game->LoadDMapData(Game->CurDMap)->ActiveSubscreen;
		Test::Assert(active_sub >= 0 && active_sub < Game->NumActiveSubscreens, "dmap has an active subscreen");
		resize_probe_sub = active_sub;
		Game->LoadASubData(active_sub)->Script = Game->GetSubscreenScript("ResizeSubscreenProbe");

		// Grow every list while the subscreen is open.
		resize_probe_mode = RESIZE_PROBE_GROW;
		Test::Assert(open_active_subscreen(), "opens the active subscreen");
		Test::AssertEqual(Game->NumActiveSubscreens, 256);
		Test::AssertEqual(Game->NumMapSubscreens, 256);

		// The engine must open the subscreen that is in the list now, which has this script.
		resize_probe_mode = RESIZE_PROBE_CLOSE;
		Test::Assert(open_active_subscreen(), "opens the active subscreen after growing the list");

		// Remove the subscreen while it is open.
		resize_probe_mode = RESIZE_PROBE_REMOVE;
		Test::Assert(open_active_subscreen(), "opens the active subscreen before removing it");
		Test::AssertEqual(Game->NumActiveSubscreens, active_sub);
		Test::AssertEqual(resize_sub_frames_after_remove, 0);

		// Removing the dmap's active subscreen leaves no active subscreen to open.
		Waitframes(5);
		Test::Assert(!open_active_subscreen(), "doesn't open an active subscreen that was removed");

		Test::End();
	}
}
