// An eweapon that a mirror shield reflects is converted into an lweapon in
// place. Its running script's state, and everything that script owned, stayed
// keyed to the eweapon script type, so removing the (now lweapon) weapon never
// released them: they lingered until the game was restarted.
//
// Observed through the bitmap pool: each eweapon script owns a batch of
// bitmaps. If reflection leaks them, the pool runs dry after a few weapons and
// an allocation fails.
//
// Note: main's version of this test bounces enemy arrows off a mirror combo.
// In 2.55 mirror combos reflect by spawning a copy of the weapon, so shield
// reflection is the only path that converts a weapon in place here.

#include "std.zh"
#include "auto/test_runner.zs"

// With the leak, each reflected weapon strands this many bitmaps. The pool
// holds 256, so WEAPONS * BITMAPS_PER_WEAPON must exceed that.
const int BITMAPS_PER_WEAPON = 60;
const int WEAPONS = 6;

// Shield block / reflect flag for fireballs (shFIREBALL).
const int SHIELD_FLAG_FIREBALL = 8;

int weapons_started = 0;
bool bitmap_alloc_failed = false;

eweapon script bug_reflected_eweapon_owned_leak_fireball
{
	void run()
	{
		for (int i = 0; i < BITMAPS_PER_WEAPON; i++)
		{
			bitmap b = Game->CreateBitmap(8, 8);
			if (!b->isAllocated())
				bitmap_alloc_failed = true;
			b->Own();
		}
		weapons_started++;
		while (true)
			Waitframe();
	}
}

generic script bug_reflected_eweapon_owned_leak
{
	void run()
	{
		Test::Init();

		// Give the hero a shield that blocks and reflects fireballs from the front.
		itemdata shield = Game->LoadItemData(I_SHIELD3);
		shield->Family = IC_SHIELD;
		shield->Attributes[0] = SHIELD_FLAG_FIREBALL;
		shield->Attributes[1] = SHIELD_FLAG_FIREBALL;
		shield->Flags[0] = true;
		Hero->Item[I_SHIELD3] = true;

		Hero->X = 64;
		Hero->Y = 80;
		Hero->Dir = DIR_RIGHT;

		for (int n = 0; n < WEAPONS; n++)
		{
			eweapon w = Screen->CreateEWeapon(EW_FIREBALL);
			w->X = 160;
			w->Y = 80;
			w->Angular = false;
			w->Dir = DIR_LEFT;
			w->Step = 200;
			w->Script = Game->GetEWeaponScript("bug_reflected_eweapon_owned_leak_fireball");

			// Fly into the shield. Reflection moves the weapon to the lweapon list.
			int frames = 0;
			while (Screen->NumLWeapons() == 0)
			{
				if (++frames > 90)
					Test::Fail("eweapon never reflected off the shield");
				Waitframe();
			}
			Test::AssertEqual(Screen->NumEWeapons(), 0, "reflected weapon left the eweapon list");

			lweapon reflected = Screen->LoadLWeapon(1);
			reflected->Remove();
			Waitframe();
			Test::AssertEqual(Screen->NumLWeapons(), 0, "reflected weapon removed");
		}

		Test::AssertEqual(weapons_started, WEAPONS, "every eweapon ran its script");
		Test::Assert(!bitmap_alloc_failed, "owned bitmaps released when a reflected eweapon is removed");

		Test::End();
	}
}
