#include "auto/test_runner.zs"

int bgdr_phase = 0;

// Saves made while saving to disk is off (replays, test mode) left out the
// generic scripts' state, so Reload restored a stale Data[].
generic script bug_generic_data_reload
{
	void run()
	{
		Test::Init();

		genericdata gd = Game->LoadGenericData(Game->GetGenericScript("bug_generic_data_reload"));
		if (bgdr_phase == 0)
		{
			bgdr_phase = 1;
			gd->DataSize = 2;
			gd->Data[1] = 5;
			Game->Save();
			Game->Reload();
		}
		else
		{
			// Reloading relaunches this script right away, within the frame it saved on. A
			// replay stops once it reaches its last frame, so end on a later one, or
			// replaying would cut off the reload.
			Waitframe();
			Test::AssertEqual(gd->DataSize, 2, "genericdata->DataSize");
			Test::AssertEqual(gd->Data[1], 5, "genericdata->Data");
			Test::End();
		}
	}
}
