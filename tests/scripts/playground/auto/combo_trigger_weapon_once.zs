#include "std.zh"
#include "auto/test_runner.zs"

// A weapon checks the triggers of a combo it overlaps every frame. Only some
// effects (secrets, combo change, copycat, ...) make it skip that combo
// afterwards. The combo triggers guide documents giving a trigger a copycat ID
// nothing else uses, with 'Copycat doesn't trigger this', to fire it once per
// weapon without any other effect.
generic script combo_trigger_weapon_once
{
	const int CMB = 3300;
	const int UNUSED_COPYCAT = 200;

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

		combodata cd = Test::setupCombo(CMB, CT_NONE);
		cd->NumTriggers = 1;
		combotrigger trig = cd->Triggers[0];
		trig->Flags[TRIGFLAG_SCRIPT1] = true;
		trig->Counter = CR_RUPEES;
		trig->CounterAmount = 1;
		trig->Flags[TRIGFLAG_COUNTER_EAT] = true;

		Hero->X = 32;
		Hero->Y = 128;
		Screen->ComboD[Test::pos(5, 5)] = CMB;
		// A second copy of the combo, which the weapon never touches.
		Screen->ComboD[Test::pos(10, 5)] = CMB;
		Waitframe();

		int fired = holdWeapon(10);
		printf("no copycat: fired %d times\n", fired);
		Test::Assert(fired > 1, "without a copycat, the trigger fires every frame");

		trig->Copycat = UNUSED_COPYCAT;
		trig->Flags[TRIGFLAG_NO_COPYCAT_CAUSE] = true;
		Test::AssertEqual(holdWeapon(10), 1, "with an unused copycat, the trigger fires once per weapon");
		Test::AssertEqual(holdWeapon(10), 1, "a new weapon fires it again");

		trig->Flags[TRIGFLAG_NO_COPYCAT_CAUSE] = false;
		Test::AssertEqual(holdWeapon(10), 2, "without 'Copycat doesn't trigger this', the other copy fires too");

		Test::End();
	}
}
