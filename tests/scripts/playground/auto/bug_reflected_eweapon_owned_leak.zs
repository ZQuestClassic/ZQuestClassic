// An eweapon that bounces off a mirror combo is converted into an lweapon in
// place. Its running script's state, and everything that script owned, stayed
// keyed to the eweapon script type, so removing the (now lweapon) weapon never
// released them: they lingered until the game was restarted.
//
// Observed through the bitmap pool: each eweapon script owns a batch of
// bitmaps. If reflection leaks them, the pool runs dry after a few weapons and
// an allocation fails.
//
// The hero's mirror shield also "reflects" weapons that are already lweapons
// (ex. a reflected arrow coming back at the hero). That is not a type change,
// so the running lweapon script must keep its state instead of restarting.
//
// Objects stored in the weapon's own arrays (Misc[], InitD[]) belong to the
// weapon rather than its script, so a reflection must not release them, and the
// weapon's removal must - including arrays only an eweapon has.

#include "std.zh"
#include "auto/test_runner.zs"

// With the leak, each reflected weapon strands this many bitmaps. The pool
// holds 256, so WEAPONS * BITMAPS_PER_WEAPON must exceed that.
const int BITMAPS_PER_WEAPON = 60;
const int WEAPONS = 6;

// Any combo id works: the mirror check only looks at the combo type.
const int MIRROR_COMBO = 4000;
const int MIRROR_TILE_X = 10;
const int MIRROR_TILE_Y = 5;

int weapons_started = 0;
bool bitmap_alloc_failed = false;
int lweapon_script_starts = 0;
int trackers_destroyed = 0;

class ReflectedEweaponTracker
{
	ReflectedEweaponTracker() {}

	~ReflectedEweaponTracker()
	{
		trackers_destroyed++;
	}
}

eweapon script bug_reflected_eweapon_owned_leak_arrow
{
	void run()
	{
		for (int i = 0; i < BITMAPS_PER_WEAPON; i++)
		{
			bitmap b = new bitmap(8, 8);
			if (!b->isAllocated())
				bitmap_alloc_failed = true;
			b->Own();
		}
		weapons_started++;
		while (true)
			Waitframe();
	}
}

lweapon script bug_reflected_eweapon_owned_leak_counter
{
	void run()
	{
		lweapon_script_starts++;
		while (true)
			Waitframe();
	}
}

generic script bug_reflected_eweapon_owned_leak
{
	void run()
	{
		Test::Init();

		// Arrows only reflect off mirrors under the new reflection behavior.
		Game->FFRules[qr_OLD_WEAPON_REFLECTION] = false;

		// Keep the hero out of the arrows' path.
		Hero->X = 16;
		Hero->Y = 144;

		int pos = MIRROR_TILE_X + MIRROR_TILE_Y * 16;
		Screen->ComboD[pos] = MIRROR_COMBO;
		Game->LoadComboData(MIRROR_COMBO)->Type = CT_MIRROR;

		for (int n = 0; n < WEAPONS; n++)
		{
			eweapon w = Screen->CreateEWeapon(EW_ARROW);
			w->X = (MIRROR_TILE_X - 3) * 16;
			w->Y = MIRROR_TILE_Y * 16;
			w->Dir = DIR_RIGHT;
			w->Step = 200;
			w->Script = Game->GetEWeaponScript("bug_reflected_eweapon_owned_leak_arrow");

			// Fly into the mirror. Reflection moves the weapon to the lweapon list.
			int frames = 0;
			while (Screen->NumLWeapons == 0)
			{
				if (++frames > 60)
					Test::Fail("eweapon never reflected off the mirror");
				Waitframe();
			}
			Test::AssertEqual(Screen->NumEWeapons, 0, "reflected weapon left the eweapon list");

			lweapon reflected = Screen->LWeapons[0];
			reflected->Remove();
			Waitframe();
			Test::AssertEqual(Screen->NumLWeapons, 0, "reflected weapon removed");
		}

		Test::AssertEqual(weapons_started, WEAPONS, "every eweapon ran its script");
		Test::Assert(!bitmap_alloc_failed, "owned bitmaps released when a reflected eweapon is removed");

		{
			eweapon w = Screen->CreateEWeapon(EW_ARROW);
			w->X = (MIRROR_TILE_X - 3) * 16;
			w->Y = MIRROR_TILE_Y * 16;
			w->Dir = DIR_RIGHT;
			w->Step = 200;
			w->Misc[0] = new ReflectedEweaponTracker();

			int frames = 0;
			while (Screen->NumLWeapons == 0)
			{
				if (++frames > 60)
					Test::Fail("eweapon with a Misc[] object never reflected off the mirror");
				Waitframe();
			}
			Waitframes(2);
			Test::AssertEqual(trackers_destroyed, 0, "object in Misc[] kept through a mirror reflection");

			Screen->LWeapons[0]->Remove();
			Waitframes(2);
			Test::AssertEqual(trackers_destroyed, 1, "object in Misc[] released when the reflected weapon is removed");
		}

		// Give the hero a passive shield that blocks and reflects arrows on every
		// side, then fire a scripted reflected arrow at them.
		int shield_id = GetHighestLevelItemOwned(IC_SHIELD);
		Test::Assert(shield_id >= 0, "hero owns a shield");
		itemdata shield = Game->LoadItemData(shield_id);
		shield->LAttributes[0] |= L_BLOCKFLAG_ARROW;
		shield->LAttributes[1] |= L_BLOCKFLAG_ARROW;
		for (int i = 0; i < 4; i++)
			shield->Flags[i] = true;
		shield->Flags[8] = false;

		Hero->X = 16;
		Hero->Y = MIRROR_TILE_Y * 16;
		Hero->Dir = DIR_RIGHT;

		lweapon arrow = Screen->CreateLWeapon(LW_REFARROW);
		arrow->X = Hero->X + 80;
		arrow->Y = Hero->Y;
		arrow->Dir = DIR_LEFT;
		arrow->Step = 200;
		arrow->Script = Game->GetLWeaponScript("bug_reflected_eweapon_owned_leak_counter");

		// The shield bounces it back the way it came.
		int frames = 0;
		while (arrow->Dir != DIR_RIGHT)
		{
			if (++frames > 60)
				Test::Fail("lweapon never reflected off the mirror shield");
			Waitframe();
		}
		Waitframes(2);
		Test::Assert(arrow->isValid(), "shield-reflected lweapon still exists");
		Test::AssertEqual(lweapon_script_starts, 1, "lweapon script kept its state through a shield reflection");
		arrow->Remove();
		Waitframe();

		// InitD[] is an eweapon-only array. Reflected through the shield rather than
		// a mirror combo, which would clear InitD[] as it converts the weapon.
		Game->FFRules[qr_BROKEN_FLAME_ARROW_REFLECTING] = false;
		eweapon ew = Screen->CreateEWeapon(EW_ARROW);
		ew->X = Hero->X + 80;
		ew->Y = Hero->Y;
		ew->Dir = DIR_LEFT;
		ew->Step = 200;
		ew->InitD[0] = new ReflectedEweaponTracker();

		frames = 0;
		while (Screen->NumLWeapons == 0)
		{
			if (++frames > 60)
				Test::Fail("eweapon never reflected off the mirror shield");
			Waitframe();
		}
		Waitframes(2);
		Test::AssertEqual(trackers_destroyed, 1, "object in InitD[] kept through a shield reflection");

		Screen->LWeapons[0]->Remove();
		Waitframes(2);
		Test::AssertEqual(trackers_destroyed, 2, "object in InitD[] released when the reflected weapon is removed");

		Test::End();
	}
}
