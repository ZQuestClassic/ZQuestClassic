#include "std.zh"
#include "auto/test_runner.zs"

// A weapon checks the triggers of a combo it overlaps every frame. Only some
// effects (secrets, combo change, ...) or 'Once Per Weapon' make it skip that
// combo afterwards.
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
		w->MoveFlags[WPNMV_OBEYS_GRAVITY] = true;
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

		// The weapon looks for combos while it's still in the air on the frame it lands, and
		// again once it's on the ground.
		cd->TrigFlags[TRIGFLAG_ONLY_GROUNDED_WEAPONS] = true;
		Test::AssertEqual(dropWeapon(), 1, "a ground-only trigger fires as the weapon lands");
		cd->TrigFlags[TRIGFLAG_ONLY_GROUNDED_WEAPONS] = false;

		cd->TrigFlags[TRIGFLAG_ONCE_PER_WEAPON] = true;
		Test::AssertEqual(holdWeapon(10), 1, "with 'Once Per Weapon', the trigger fires once per weapon");
		Test::AssertEqual(holdWeapon(10), 1, "a new weapon fires it again");

		// Once a 'Once Per Weapon' trigger has fired, the weapon doesn't run any of it again,
		// not even its weapon effects.
		cd->TrigFlags[TRIGFLAG_IGNITE_WEAPONS_ANY] = true;
		for (int separate = 0; separate < 2; ++separate)
		{
			cd->TrigFlags[TRIGFLAG_SEPARATE_WEAPON_TRIGGERS] = separate;
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
