#include "std.zh"
#include "auto/test_runner.zs"

// A weapon checks the triggers of a combo it overlaps every frame. Only some
// effects (secrets, combo change, copycat, ...) make it skip that combo
// afterwards, and 'Once Per Weapon' that trigger, as the combo triggers guide
// documents.
generic script combo_trigger_weapon_once
{
	const int CMB = 3300;
	const int COPYCAT = 200;

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

	// Drops a weapon onto the combo at (5,5) that breaks when it lands, and returns
	// how many times the trigger fired.
	int dropWeapon()
	{
		Game->Counter[CR_RUPEES] = 200;
		lweapon w = Screen->CreateLWeapon(LW_SCRIPT1);
		w->X = 5 * 16;
		w->Y = 5 * 16;
		w->Z = 8;
		w->Step = 0;
		w->MoveFlags[MV_OBEYS_GRAVITY] = true;
		w->Flags[WFLAG_BREAKS_ON_LANDING] = true;
		for (int i = 0; i < 60 && w->isValid(); ++i)
			Waitframe();
		Test::Assert(!w->isValid(), "the weapon broke when it landed");
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

		// The weapon's hitbox has 4 corners on the same combo, but only triggers it once a frame.
		Test::AssertEqual(holdWeapon(10), 10, "the trigger fires once every frame");

		// The weapon looks for combos while it's still in the air on the frame it lands, and
		// again once it's on the ground.
		trig->Flags[TRIGFLAG_ONLY_GROUNDED_WEAPONS] = true;
		Test::AssertEqual(dropWeapon(), 1, "a ground-only trigger fires as the weapon lands");
		trig->Flags[TRIGFLAG_ONLY_GROUNDED_WEAPONS] = false;

		trig->Flags[TRIGFLAG_ONCE_PER_WEAPON] = true;
		Test::AssertEqual(holdWeapon(10), 1, "with 'Once Per Weapon', the trigger fires once per weapon");
		Test::AssertEqual(holdWeapon(10), 1, "a new weapon fires it again");
		trig->Flags[TRIGFLAG_ONCE_PER_WEAPON] = false;

		// The copy of the combo at (10,5) shares the copycat.
		trig->Copycat = COPYCAT;
		trig->Flags[TRIGFLAG_NO_COPYCAT_CAUSE] = true;
		Test::AssertEqual(holdWeapon(10), 1, "a copycat makes the weapon skip the combo afterwards");

		trig->Flags[TRIGFLAG_NO_COPYCAT_CAUSE] = false;
		Test::AssertEqual(holdWeapon(10), 2, "without 'Copycat doesn't trigger this', the other copy fires too");
		trig->Copycat = 0;

		// 'Once Per Weapon' only stops its own trigger, wherever it is in the list.
		cd->NumTriggers = 2;
		combotrigger first = cd->Triggers[0];
		first->Flags[TRIGFLAG_ONCE_PER_WEAPON] = true;
		combotrigger second = cd->Triggers[1];
		second->Flags[TRIGFLAG_SCRIPT1] = true;
		second->Counter = CR_RUPEES;
		second->CounterAmount = 1;
		second->Flags[TRIGFLAG_COUNTER_EAT] = true;
		Test::AssertEqual(holdWeapon(10), 11, "the trigger after a 'Once Per Weapon' one fires every frame");
		first->Flags[TRIGFLAG_ONCE_PER_WEAPON] = false;

		// A 'Weapon Separate Triggers' trigger only runs its weapon effects, and doesn't make
		// the weapon skip the combo's other triggers, with 'Once Per Weapon' or not.
		first->Flags[TRIGFLAG_SEPARATE_WEAPON_TRIGGERS] = true;
		Test::AssertEqual(holdWeapon(10), 10, "a 'Weapon Separate Triggers' trigger doesn't stop the others");
		first->Flags[TRIGFLAG_ONCE_PER_WEAPON] = true;
		Test::AssertEqual(holdWeapon(10), 10, "nor with 'Once Per Weapon'");

		// Once a 'Once Per Weapon' trigger has fired, the weapon doesn't run any of it again:
		// not its weapon effects, and it doesn't 'Cancel Trigger' the triggers after it.
		cd = Test::setupCombo(CMB + 1, CT_NONE);
		cd->NumTriggers = 2;
		first = cd->Triggers[0];
		first->Flags[TRIGFLAG_SCRIPT1] = true;
		first->Flags[TRIGFLAG_ONCE_PER_WEAPON] = true;
		first->Flags[TRIGFLAG_CANCEL_TRIGGER] = true;
		first->Counter = CR_RUPEES;
		first->CounterAmount = 1;
		first->Flags[TRIGFLAG_COUNTER_EAT] = true;
		second = cd->Triggers[1];
		second->Flags[TRIGFLAG_SCRIPT1] = true;
		second->Counter = CR_RUPEES;
		second->CounterAmount = 1;
		second->Flags[TRIGFLAG_COUNTER_EAT] = true;
		Screen->ComboD[Test::pos(5, 5)] = CMB + 1;
		Screen->ComboD[Test::pos(10, 5)] = CMB + 1;
		Waitframe();
		Test::AssertEqual(holdWeapon(10), 10, "a fired 'Once Per Weapon' trigger stops cancelling the ones after it");

		cd->NumTriggers = 1;
		first->Flags[TRIGFLAG_CANCEL_TRIGGER] = false;
		first->Flags[TRIGFLAG_IGNITE_WEAPONS_ANY] = true;
		for (int separate = 0; separate < 2; ++separate)
		{
			first->Flags[TRIGFLAG_SEPARATE_WEAPON_TRIGGERS] = separate;
			lweapon w = Screen->CreateLWeapon(LW_SCRIPT1);
			w->X = 5 * 16;
			w->Y = 5 * 16;
			w->Step = 0;
			Waitframes(2);
			Test::Assert(w->Flags[WFLAG_BURN_ANYFIRE], "the trigger ignites the weapon");
			w->Flags[WFLAG_BURN_ANYFIRE] = false;
			Waitframes(5);
			Test::Assert(!w->Flags[WFLAG_BURN_ANYFIRE], "a 'Once Per Weapon' trigger doesn't ignite the weapon again");
			Remove(w);
			Waitframe();
		}

		Test::End();
	}
}
