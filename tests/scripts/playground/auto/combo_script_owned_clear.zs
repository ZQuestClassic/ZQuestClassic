// Validates that objects a script takes ownership of are released when the script's engine
// data is cleared while the script is still running. Previously only scripts that terminated
// normally released their owned objects; a script slot cleared mid-run (combo script killed
// by a combo change, engine subscreen script killed by the subscreen closing) leaked
// everything it owned, so repeating that would exhaust the 256-bitmap pool.
//
// Note: main's version of this test (subscreen_owned_clear.zs) kills an engine active
// subscreen script via this->Close(), which doesn't exist in 2.55. Killing a running combo
// script by changing the combo under it exercises the same cleanup path: the script's
// engine data is cleared mid-run, and that must release the script's owned objects.

#include "std.zh"
#include "auto/test_runner.zs"

// Each placement owns this many bitmaps. With the fix they are freed when the combo change
// clears the running script's engine data (peak stays well under the 256-bitmap pool);
// without it they accumulate and exhaust the pool within a few placements.
const int BITMAPS_PER_PLACEMENT = 100;
const int TARGET_PLACEMENTS = 4;

// A combo id we can safely attach the test script to; the test screen doesn't use it.
const int SCRIPTED_CID = 4000;

int combo_placements = 0;
bool combo_bitmap_alloc_failed = false;

// Runs as a combo script. Owns a batch of bitmaps, then waits forever - it never ends on
// its own, so it is always killed mid-run when the combo under it changes. The engine must
// release the owned bitmaps when it clears this script's engine data.
combodata script ComboOwnsBitmaps
{
	void run()
	{
		for (int i = 0; i < BITMAPS_PER_PLACEMENT; i++)
		{
			bitmap b = Game->CreateBitmap(8, 8);
			if (!b->isAllocated())
				combo_bitmap_alloc_failed = true;
			b->Own();
		}
		combo_placements++;
		while (true)
			Waitframe();
	}
}

generic script combo_script_owned_clear
{
	void run()
	{
		Test::Init();

		int script_id = Game->GetComboScript("ComboOwnsBitmaps");
		if (script_id == -1)
			Test::Fail("could not find script");
		Game->LoadComboData(SCRIPTED_CID)->Script = script_id;

		int pos = 0;
		int orig_cid = Screen->ComboD[pos];

		// Repeatedly place the scripted combo (its script owns a batch of bitmaps and then
		// waits forever) and replace it (killing the running script). Each kill must release
		// the owned bitmaps; if it leaks them, the bitmap pool exhausts and an allocation
		// eventually fails.
		for (int placement = 1; placement <= TARGET_PLACEMENTS; placement++)
		{
			Screen->ComboD[pos] = SCRIPTED_CID;
			while (combo_placements < placement)
				Waitframe();
			Screen->ComboD[pos] = orig_cid;
			Waitframe();
		}

		Test::Assert(!combo_bitmap_alloc_failed, "owned bitmaps released when running combo script is killed");

		Test::End();
	}
}
