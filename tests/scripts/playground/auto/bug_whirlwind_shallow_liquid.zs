#include "std.zh"
#include "auto/test_runner.zs"

// Shallow liquid hurt the Hero while a whirlwind carried them over it. When the
// damage had hitstun, the Hero was knocked out of the whirlwind but was never
// made visible again.
// https://discord.com/channels/876899628556091432/1551691909120917616
generic script bug_whirlwind_shallow_liquid
{
	const int WATER_COMBO = 13;
	const int WATER_ROW = 5;

	void ride(bool sensitive)
	{
		Game->FFRules[qr_SHALLOW_SENSITIVE] = sensitive;
		Hero->X = 16;
		Hero->Y = WATER_ROW * 16;
		Hero->Dir = DIR_RIGHT;
		Waitframe();
		int hp = Hero->HP;

		lweapon wind = Screen->CreateLWeapon(LW_WIND);
		wind->X = Hero->X;
		wind->Y = Hero->Y;
		wind->Dir = DIR_RIGHT;
		while (Hero->Action != LA_INWIND)
			Waitframe();

		// Carry the Hero across the liquid, then drop them on dry land.
		while (wind->isValid() && wind->X < 12 * 16)
		{
			Test::AssertEqual(Hero->Action, LA_INWIND, "Hero stays in the whirlwind");
			Waitframe();
		}
		Test::Assert(wind->isValid(), "whirlwind reached dry land");
		// Not Remove(), which moves the whirlwind (and the Hero) off screen first.
		wind->DeadState = WDS_DEAD;
		for (int i = 0; i < 10 && Hero->Action == LA_INWIND; i++)
			Waitframe();

		Test::AssertEqual(Hero->HP, hp, "shallow liquid does not hurt the Hero in a whirlwind");
		Test::AssertEqual(Hero->Action, LA_NONE, "Hero left the whirlwind");
		Test::AssertEqual(Hero->DrawXOffset, 0, "Hero is visible");
	}

	void run()
	{
		Test::Init();

		combodata cd = Game->LoadComboData(WATER_COMBO);
		cd->Type = CT_SHALLOWWATER;
		cd->Walk = 0;
		// Passive damage, with hitstun.
		cd->Flags[1] = true;
		cd->Flags[6] = true;
		cd->Attributes[1] = -2;
		// Required item level to be immune, which the Hero doesn't have.
		cd->Attributes[11] = 100;
		for (int x = 3; x < 11; x++)
			Screen->ComboD[ComboAt(x * 16, WATER_ROW * 16)] = WATER_COMBO;

		Hero->MaxHP = 16 * 10;
		Hero->HP = Hero->MaxHP;

		ride(true);
		ride(false);

		Test::End();
	}
}
