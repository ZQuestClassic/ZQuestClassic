#include "std.zh"
#include "auto/test_runner.zs"

// A weapon checks the triggers of a combo it overlaps every frame. Only some
// effects (secrets, combo change, ...) make it skip that combo afterwards.
generic script combo_trigger_weapon_once
{
	const int CMB = 3300;

	// Holds a still weapon on the combo at (5,5) and returns how many times the
	// trigger fired, counted by the rupee each firing consumes.
	int holdWeapon(int frames)
	{
		Game->Counter[CR_RUPEES] = 200;
		lweapon w = Screen->CreateLWeapon(LW_SCRIPT1);
		w->X = 5 * 16;
		w->Y = 5 * 16;
		w->Step = 0;
		Waitframes(frames);
		Remove(w);
		Waitframe();
		return 200 - Game->Counter[CR_RUPEES];
	}

	void run()
	{
		Test::Init();

		combodata cd = Game->LoadComboData(CMB);
		cd->Type = CT_NONE;
		cd->TrigFlags[TRIGFLAG_SCRIPT1] = true;
		cd->TrigCounter = CR_RUPEES;
		cd->TrigCtrAmount = 1;
		cd->TrigFlags[TRIGFLAG_COUNTER_EAT] = true;

		Hero->X = 32;
		Hero->Y = 128;
		Screen->ComboD[5 + 5 * 16] = CMB;
		Waitframe();

		// The weapon's hitbox has 4 corners on the same combo, but only triggers it once a frame.
		Test::AssertEqual(holdWeapon(10), 10, "the trigger fires once every frame");

		Test::End();
	}
}
