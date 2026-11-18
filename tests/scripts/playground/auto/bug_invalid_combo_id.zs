#include "std.zh"
#include "auto/test_runner.zs"

// Combo ids past the last combo used to be accepted, and then read past the end of the combo list.

generic script bug_invalid_combo_id
{
	void run()
	{
		Test::Init();

		int map = Game->GetCurMap();
		int screen = Game->GetCurScreen();

		mapdata md = Game->LoadTempScreen(0);
		int ffc_combo = md->FFCData[1];
		md->FFCData[1] = MAX_COMBOS + 1;
		Test::AssertEqual(md->FFCData[1], ffc_combo, "mapdata->FFCData[]");

		int combo = Screen->ComboD[0];
		Game->SetComboData(map, screen, 0, MAX_COMBOS);
		Test::AssertEqual(Screen->ComboD[0], MAX_COMBOS, "Game->SetComboData() is clamped");
		Screen->ComboD[0] = combo;

		combodata cd = Game->LoadComboData(1);
		cd->LiftGFXCombo = MAX_COMBOS + 1;
		Test::AssertEqual(cd->LiftGFXCombo, MAX_COMBOS, "ComboData->LiftGFXCombo is clamped");
		cd->LiftUnderCombo = MAX_COMBOS + 1;
		Test::AssertEqual(cd->LiftUnderCombo, MAX_COMBOS, "ComboData->LiftUnderCombo is clamped");

		Test::End();
	}
}
