// The per-screen trigger group counts (Game->TrigGroups[]) were only adjusted
// when a position's combo changed, not when a script changed a placed combo's
// 'Contributes To TrigGroup' flag or its TrigGroup. Removing the combo
// afterwards then subtracted using the new settings, so a count could go
// negative and stay wrong for the rest of the screen.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_trig_group_count_script_change
{
	const int CMB = 3500;
	const int CMB_FFC = 3501;

	void run()
	{
		Test::Init();
		int p = 3 + 3 * 16;

		combodata cd = Game->LoadComboData(CMB);
		cd->Type = CT_NONE;
		// A placed combo is counted on the next cpos update, which runs after
		// generic scripts, so give it a frame to settle.
		Screen->ComboD[p] = CMB;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[3], 0, "nothing contributes yet");

		cd->TrigGroup = 3;
		cd->TrigFlags[TRIGFLAG_TGROUP_CONTRIB] = true;
		Waitframe();
		Test::AssertEqual(Game->TrigGroups[3], 1, "setting the flag on a placed combo counts it");

		cd->TrigGroup = 4;
		Waitframe();
		Test::AssertEqual(Game->TrigGroups[3], 0, "changing the group uncounts the old group");
		Test::AssertEqual(Game->TrigGroups[4], 1, "and counts the new one");

		Screen->ComboD[p] = 0;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[4], 0, "removing the combo uncounts it exactly once");
		Screen->ComboD[p] = CMB;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[4], 1, "placing it again counts it");

		cd->TrigFlags[TRIGFLAG_TGROUP_CONTRIB] = false;
		Waitframe();
		Test::AssertEqual(Game->TrigGroups[4], 0, "clearing the flag uncounts it");
		Screen->ComboD[p] = 0;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[4], 0, "and removing it afterwards leaves the count alone");

		// The same through an FFC.
		combodata fc = Game->LoadComboData(CMB_FFC);
		fc->Type = CT_NONE;
		ffc f = Screen->LoadFFC(1);
		f->Data = CMB_FFC;
		f->X = 64;
		f->Y = 64;
		Waitframes(2);
		fc->TrigGroup = 5;
		fc->TrigFlags[TRIGFLAG_TGROUP_CONTRIB] = true;
		Waitframe();
		Test::AssertEqual(Game->TrigGroups[5], 1, "setting the flag on a placed FFC's combo counts it");
		f->Data = 0;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[5], 0, "removing the FFC's combo uncounts it exactly once");

		// Two FFCs cleared lowest-first in the same frame: the first is still
		// in range when it is cleared, and the second clearing moves it out of
		// range before the next cpos update can uncount it.
		ffc f2 = Screen->LoadFFC(2);
		f->Data = CMB_FFC;
		f2->Data = CMB_FFC;
		f2->X = 96;
		f2->Y = 64;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[5], 2, "both FFCs count");
		f->Data = 0;
		f2->Data = 0;
		Waitframes(2);
		Test::AssertEqual(Game->TrigGroups[5], 0, "clearing both in one frame uncounts both");

		Test::End();
	}
}
