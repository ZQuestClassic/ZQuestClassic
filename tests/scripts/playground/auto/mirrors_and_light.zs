#include "std.zh"
#include "auto/test_runner.zs"

// Covers mirror and prism combos reflecting and splitting weapons (as
// combos and as FFCs), and light beams (from combo and FFC spotlights)
// bouncing off mirrors, splitting through prisms, passing through glass and
// being stopped by blocks on their way to light targets and triggers.
generic script mirrors_and_light
{
	const int CMB_MIRROR = 3100;
	const int CMB_SPOTLIGHT = 3120;
	const int CMB_FF_SPOTLIGHT = 3121;
	const int CMB_GLASS = 3122;
	const int CMB_BLOCK = 3123;
	// Unlit light target, which turns into the next combo (the lit target) when lit.
	const int CMB_TARGET = 3130;
	// A plain combo with a 'Light On->' trigger, which changes into the next combo.
	const int CMB_LIGHT_TRIGGER = 3140;

	const int MIRROR_ROW_Y = 80;
	// Shield/reflection flag for arrows (a raw bitmask, hence long).
	const long SH_ARROW = 10bL;
	const int MIRROR_X = 128;

	// Bitmask of the directions the weapons on screen are moving in.
	int weaponDirs()
	{
		int dirs = 0;
		for (int i = 1; i <= Screen->NumLWeapons; i++)
			dirs |= 1 << Screen->LoadLWeapon(i)->Dir;
		for (int i = 1; i <= Screen->NumEWeapons; i++)
			dirs |= 1 << Screen->LoadEWeapon(i)->Dir;
		return dirs;
	}

	void printWeapons(char32[] label)
	{
		char32 buf[256];
		sprintf(buf, "%s:", label);
		for (int i = 1; i <= Screen->NumLWeapons; i++)
		{
			lweapon w = Screen->LoadLWeapon(i);
			sprintf(buf, "%s lw(id %d dir %d)", buf, w->ID, w->Dir);
		}
		for (int i = 1; i <= Screen->NumEWeapons; i++)
		{
			eweapon w = Screen->LoadEWeapon(i);
			sprintf(buf, "%s ew(id %d dir %d)", buf, w->ID, w->Dir);
		}
		printf("%s\n", buf);
	}

	void fire(int id, bool enemy)
	{
		if (enemy)
		{
			eweapon e = Screen->CreateEWeapon(id);
			e->Damage = 0;
			e->X = MIRROR_X - 64;
			e->Y = MIRROR_ROW_Y;
			e->Dir = DIR_RIGHT;
			e->Step = 200;
		}
		else
		{
			lweapon l = Screen->CreateLWeapon(id);
			l->Damage = 0;
			l->X = MIRROR_X - 64;
			l->Y = MIRROR_ROW_Y;
			l->Dir = DIR_RIGHT;
			l->Step = 200;
		}
	}

	// Shoots a weapon right at the mirror, and returns the directions of the
	// weapons once it gets there.
	int shoot(int id, bool enemy, char32[] label)
	{
		Test::Label(label);
		// Only this weapon should be on screen.
		Test::clearWeapons();
		Waitframe();
		fire(id, enemy);
		// Enough for the slowest weapon (fireballs wait 16 frames before moving) to
		// reach the mirror and get a few tiles past it.
		Waitframes(60);
		printWeapons(label);
		return weaponDirs();
	}

	void testMirrorType(int type, int expectedDirs, char32[] name)
	{
		Test::setupCombo(CMB_MIRROR, type);
		Screen->ComboD[ComboAt(MIRROR_X, MIRROR_ROW_Y)] = CMB_MIRROR;

		int lweapons[] = {LW_MAGIC, LW_BEAM, LW_ARROW, LW_SCRIPT1};
		int eweapons[] = {EW_MAGIC, EW_FIREBALL, EW_ROCK, EW_ARROW, EW_BEAM};
		char32 buf[64];
		for (int i = 0; i < SizeOfArray(lweapons); i++)
		{
			sprintf(buf, "%s / lw %d", name, lweapons[i]);
			Test::AssertEqual(shoot(lweapons[i], false, buf), expectedDirs, buf);
		}
		for (int i = 0; i < SizeOfArray(eweapons); i++)
		{
			sprintf(buf, "%s / ew %d", name, eweapons[i]);
			Test::AssertEqual(shoot(eweapons[i], true, buf), expectedDirs, buf);
		}
	}

	void testWeaponMirrors()
	{
		const int UP = 1 << DIR_UP;
		const int DOWN = 1 << DIR_DOWN;
		const int LEFT = 1 << DIR_LEFT;
		const int RIGHT = 1 << DIR_RIGHT;

		testMirrorType(CT_MIRROR, LEFT, "mirror");
		testMirrorType(CT_MIRRORSLASH, UP, "mirror /");
		testMirrorType(CT_MIRRORBACKSLASH, DOWN, "mirror \\");
		testMirrorType(CT_MAGICPRISM, UP | DOWN | RIGHT, "prism");
		testMirrorType(CT_MAGICPRISM4WAY, UP | DOWN | LEFT | RIGHT, "prism 4-way");

		// The custom mirror sends each incoming direction to its own direction:
		// attribute 8 + the weapon's direction.
		combodata cd = Test::setupCombo(CMB_MIRROR, CT_MIRROR_CUSTOM);
		cd->Attributes[8 + DIR_RIGHT] = DIR_RIGHTDOWN;
		Screen->ComboD[ComboAt(MIRROR_X, MIRROR_ROW_Y)] = CMB_MIRROR;
		Test::AssertEqual(shoot(LW_MAGIC, false, "custom mirror / lw magic"), 1 << DIR_RIGHTDOWN, "custom mirror redirects");
		Test::AssertEqual(shoot(EW_ARROW, true, "custom mirror / ew arrow"), 1 << DIR_RIGHTDOWN, "custom mirror redirects");
		// A direction past 7 destroys the weapon.
		cd->Attributes[8 + DIR_RIGHT] = 8;
		Test::AssertEqual(shoot(LW_MAGIC, false, "custom mirror kill / lw magic"), 0, "custom mirror destroys");

		// Attribute 0 limits what a mirror reflects: only arrows here.
		cd = Test::setupCombo(CMB_MIRROR, CT_MIRRORSLASH);
		cd->Attributes[0] = SH_ARROW;
		Test::AssertEqual(shoot(LW_ARROW, false, "arrow-only mirror / lw arrow"), 1 << DIR_UP, "arrow is reflected");
		Test::AssertEqual(shoot(LW_BEAM, false, "arrow-only mirror / lw beam"), 1 << DIR_RIGHT, "beam passes through");
	}

	void testFFCMirrors()
	{
		Game->FFRules[qr_MIRROR_PRISM_FFCS] = true;
		// Only the FFC should be in the weapon's path.
		Screen->ComboD[ComboAt(MIRROR_X, MIRROR_ROW_Y)] = 0;

		Test::setupCombo(CMB_MIRROR, CT_MIRRORSLASH);
		ffc f = Screen->LoadFFC(1);
		f->Data = CMB_MIRROR;
		f->X = MIRROR_X;
		f->Y = MIRROR_ROW_Y;
		Test::AssertEqual(shoot(LW_MAGIC, false, "ffc mirror / lw magic"), 1 << DIR_UP, "ffc mirror reflects");
		Test::AssertEqual(shoot(EW_FIREBALL, true, "ffc mirror / ew fireball"), 1 << DIR_UP, "ffc mirror reflects");

		Test::setupCombo(CMB_MIRROR, CT_MAGICPRISM4WAY);
		Test::AssertEqual(shoot(LW_BEAM, false, "ffc prism / lw beam"), 0xF, "ffc prism splits");
	}

	void testOldReflection()
	{
		// Start from an empty screen.
		clearCombos();
		// With the old reflection rule, only magic and sword beams reflect.
		Game->FFRules[qr_OLD_WEAPON_REFLECTION] = true;
		Test::setupCombo(CMB_MIRROR, CT_MIRRORSLASH);
		Screen->ComboD[ComboAt(MIRROR_X, MIRROR_ROW_Y)] = CMB_MIRROR;
		Test::AssertEqual(shoot(LW_MAGIC, false, "old reflection / lw magic"), 1 << DIR_UP, "old reflection: magic is reflected");
		Test::AssertEqual(shoot(LW_ARROW, false, "old reflection / lw arrow"), 1 << DIR_RIGHT, "old reflection: arrow passes through");
	}

	bool isLit(int col, int row)
	{
		return Screen->ComboD[Test::pos(col, row)] == CMB_TARGET + 1;
	}

	void placeTarget(int col, int row)
	{
		Screen->ComboD[Test::pos(col, row)] = CMB_TARGET;
	}

	void printTargets(char32[] label, int[] cols, int[] rows, int n)
	{
		char32 buf[256];
		sprintf(buf, "%s:", label);
		for (int i = 0; i < n; i++)
			sprintf(buf, "%s (%d,%d)=%d", buf, cols[i], rows[i], isLit(cols[i], rows[i]));
		printf("%s\n", buf);
	}

	// Labels a light beam step and gives the beams a couple of frames to settle.
	void settle(char32[] what)
	{
		Test::Label(what);
		Waitframes(2);
	}

	void clearCombos()
	{
		for (int i = 0; i < 176; i++)
			Screen->ComboD[i] = 0;
	}

	void setupLightCombos()
	{
		Test::setupCombo(CMB_GLASS, CT_GLASS);
		Test::setupCombo(CMB_BLOCK, CT_BLOCKALL);
		Test::setupCombo(CMB_TARGET, CT_LIGHT_TARGET);
		// Flag 1 marks the lit version, which reverts when no longer lit.
		combodata lit = Test::setupCombo(CMB_TARGET + 1, CT_LIGHT_TARGET);
		lit->Flags[0] = true;
	}

	// Lighting every target on the screen solves the light puzzle, after which
	// they all stay lit. Until the end, keep one target in a corner that no
	// beam reaches.
	void placeDecoyTarget()
	{
		placeTarget(0, 10);
	}

	void testFFCLightBeams()
	{
		// The last weapon test's beams may still be flying.
		Test::clearWeapons();
		setupLightCombos();
		placeDecoyTarget();

		// An FFC spotlight shines a pixel-precise beam.
		combodata spot = Test::setupCombo(CMB_FF_SPOTLIGHT, CT_SPOTLIGHT);
		spot->Attributes[8] = DIR_RIGHT;
		spot->Flags[1] = true;
		ffc f = Screen->LoadFFC(1);
		f->Data = CMB_FF_SPOTLIGHT;
		f->X = 16;
		f->Y = 128;

		int cols[] = {12, 6, 6};
		int rows[] = {8, 10, 6};
		for (int i = 0; i < 3; i++)
			placeTarget(cols[i], rows[i]);
		settle("ffc open path");
		printTargets("ffc open path", cols, rows, 3);
		Test::Assert(isLit(12, 8), "the ffc beam lights the target in front of it");
		Test::Assert(!isLit(6, 10) && !isLit(6, 6), "targets out of the ffc beam stay unlit");

		Test::setupCombo(CMB_MIRROR, CT_MIRRORBACKSLASH);
		Screen->ComboD[Test::pos(6, 8)] = CMB_MIRROR;
		settle("ffc mirror \\");
		printTargets("ffc mirror \\", cols, rows, 3);
		Test::Assert(isLit(6, 10) && !isLit(12, 8), "the ffc beam bounces down off the mirror");

		Test::setupCombo(CMB_MIRROR, CT_MAGICPRISM);
		settle("ffc prism");
		printTargets("ffc prism", cols, rows, 3);
		Test::Assert(isLit(6, 10) && isLit(6, 6) && isLit(12, 8), "the prism splits the ffc beam");

		Screen->ComboD[Test::pos(6, 8)] = CMB_BLOCK;
		settle("ffc block");
		printTargets("ffc block", cols, rows, 3);
		Test::Assert(!isLit(12, 8) && !isLit(6, 10) && !isLit(6, 6), "a block stops the ffc beam");
	}

	void testLightBeams()
	{
		// Start from an empty screen, without the FFC spotlight.
		clearCombos();
		Screen->LoadFFC(1)->Data = 0;
		setupLightCombos();
		placeDecoyTarget();

		combodata spot = Test::setupCombo(CMB_SPOTLIGHT, CT_SPOTLIGHT);
		spot->Attributes[8] = DIR_RIGHT;

		// A spotlight on the left of row 2, shining right.
		Screen->ComboD[Test::pos(1, 2)] = CMB_SPOTLIGHT;
		int cols[] = {10, 10, 14};
		int rows[] = {0, 4, 2};
		for (int i = 0; i < 3; i++)
			placeTarget(cols[i], rows[i]);

		settle("open path");
		printTargets("open path", cols, rows, 3);
		Test::Assert(isLit(14, 2), "the beam lights the target in front of it");
		Test::Assert(!isLit(10, 0) && !isLit(10, 4), "targets out of the beam stay unlit");

		// A '/' mirror bounces the beam up.
		Test::setupCombo(CMB_MIRROR, CT_MIRRORSLASH);
		Screen->ComboD[Test::pos(10, 2)] = CMB_MIRROR;
		settle("mirror /");
		printTargets("mirror /", cols, rows, 3);
		Test::Assert(isLit(10, 0), "the beam bounces up off the mirror");
		Test::Assert(!isLit(14, 2), "the target behind the mirror goes dark");

		// A '\' mirror bounces it down.
		Test::setupCombo(CMB_MIRROR, CT_MIRRORBACKSLASH);
		settle("mirror \\");
		printTargets("mirror \\", cols, rows, 3);
		Test::Assert(isLit(10, 4) && !isLit(10, 0), "the beam bounces down off the mirror");

		// Glass lets the beam through; a block stops it.
		Screen->ComboD[Test::pos(10, 2)] = CMB_GLASS;
		settle("glass");
		printTargets("glass", cols, rows, 3);
		Test::Assert(isLit(14, 2) && !isLit(10, 0) && !isLit(10, 4), "the beam passes through glass");
		Screen->ComboD[Test::pos(10, 2)] = CMB_BLOCK;
		settle("block");
		printTargets("block", cols, rows, 3);
		Test::Assert(!isLit(14, 2), "a block stops the beam");

		// A combo with a 'Light On->' trigger.
		combodata trig = Test::setupCombo(CMB_LIGHT_TRIGGER, CT_NONE);
		trig->NumTriggers = 1;
		trig->Triggers[0]->Flags[TRIGFLAG_LIGHTON] = true;
		trig->Triggers[0]->ComboChange = 1;
		Test::setupCombo(CMB_LIGHT_TRIGGER + 1, CT_NONE);
		Screen->ComboD[Test::pos(6, 2)] = CMB_LIGHT_TRIGGER;
		settle("a 'Light On->' trigger in the beam");
		Test::AssertEqual(Screen->ComboD[Test::pos(6, 2)], CMB_LIGHT_TRIGGER + 1, "light triggers the combo");
		Screen->ComboD[Test::pos(6, 2)] = 0;

		// A 4-way prism lights all three.
		Test::setupCombo(CMB_MIRROR, CT_MAGICPRISM4WAY);
		Screen->ComboD[Test::pos(10, 2)] = CMB_MIRROR;
		settle("prism 4-way");
		printTargets("prism 4-way", cols, rows, 3);
		Test::Assert(isLit(10, 0) && isLit(10, 4) && isLit(14, 2), "the prism splits the beam");
		Test::Assert(!Screen->State[ST_SECRET], "puzzle isn't solved while a target is unlit");

		// Lighting the last target solves the puzzle: secrets trigger, and
		// every target stays lit even once the light is gone.
		Screen->ComboD[Test::pos(0, 10)] = 0;
		settle("removing the decoy target solves the puzzle");
		Test::Assert(Screen->State[ST_SECRET], "solving the light puzzle triggers secrets");
		Screen->ComboD[Test::pos(1, 2)] = 0;
		settle("solved, light off");
		printTargets("solved, light off", cols, rows, 3);
		Test::Assert(isLit(10, 0) && isLit(10, 4) && isLit(14, 2), "solved targets stay lit");
	}

	void run()
	{
		Test::Init();

		// Out of the way of the weapons and beams.
		Hero->X = 240;
		Hero->Y = 160;
		Hero->Dir = DIR_UP;
		for (int i = 0; i < 176; i++)
			Screen->ComboD[i] = 0;
		Waitframe();

		// Sword beams only reflect with this rule, and most other weapons only
		// reflect without the old reflection rule.
		Game->FFRules[qr_SWORDMIRROR] = true;
		Game->FFRules[qr_OLD_WEAPON_REFLECTION] = false;
		testWeaponMirrors();
		testFFCMirrors();
		testFFCLightBeams();
		testLightBeams();
		testOldReflection();

		Test::End();
	}
}
