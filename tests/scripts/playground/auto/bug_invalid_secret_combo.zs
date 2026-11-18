#include "std.zh"
#include "auto/test_runner.zs"

// A screen's secret and under combos, and a combo's next combo, used to accept combo ids past the
// last combo. Triggering secrets or cycling the combo then read past the end of the combo list.

generic script bug_invalid_secret_combo
{
	void run()
	{
		Test::Init();

		int secret = Screen->SecretCombo[0];
		Screen->SecretCombo[0] = MAX_COMBOS + 1;
		Test::AssertEqual(Screen->SecretCombo[0], secret, "Screen->SecretCombo[]");
		Screen->SecretCombo[0] = MAX_COMBOS;
		Test::AssertEqual(Screen->SecretCombo[0], MAX_COMBOS, "Screen->SecretCombo[] takes the last combo");
		Screen->SecretCombo[0] = secret;

		int under = Screen->UnderCombo;
		Screen->UnderCombo = MAX_COMBOS + 1;
		Test::AssertEqual(Screen->UnderCombo, under, "Screen->UnderCombo");

		mapdata md = Game->LoadTempScreen(0);
		secret = md->SecretCombo[0];
		md->SecretCombo[0] = MAX_COMBOS + 1;
		Test::AssertEqual(md->SecretCombo[0], secret, "mapdata->SecretCombo[]");
		under = md->UnderCombo;
		md->UnderCombo = MAX_COMBOS + 1;
		Test::AssertEqual(md->UnderCombo, under, "mapdata->UnderCombo");

		combodata cd = Game->LoadComboData(1);
		int next = cd->NextData;
		cd->NextData = MAX_COMBOS + 1;
		Test::AssertEqual(cd->NextData, next, "ComboData->NextData");

		Test::End();
	}
}
