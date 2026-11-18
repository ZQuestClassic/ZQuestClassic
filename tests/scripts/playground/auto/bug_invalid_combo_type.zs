#include "auto/test_runner.zs"

// A combo type past the last one used to be accepted, and then read past the end of the combo type
// table (for example when checking whether the combo is water).

generic script bug_invalid_combo_type
{
	void run()
	{
		Test::Init();

		int map = Game->GetCurMap();
		int screen = Game->GetCurScreen();
		int pos = 0;
		int type = Screen->ComboT[pos];

		Screen->ComboT[pos] = CT_MAX;
		Test::AssertEqual(Screen->ComboT[pos], type, "Screen->ComboT[]");

		Game->SetComboType(map, screen, pos, CT_MAX);
		Test::AssertEqual(Screen->ComboT[pos], type, "Game->SetComboType()");

		combodata cd = Game->LoadComboData(Screen->ComboD[pos]);
		cd->Type = 200;
		Test::AssertEqual(cd->Type, type, "ComboData->Type");

		Test::End();
	}
}
