#include "std.zh"
#include "auto/test_runner.zs"

// Covers switch and switch block combos (global states, with and without a
// timer, across screens; level states; triggering a block directly), combo
// triggers that read and write global states, timed combo triggers, and the
// 'Step->Effects' and step combos - both stepped on and triggered.
generic script switches_and_step_effects
{
	const int GSTATE = 5;
	const int LSTATE = 3;

	// Each switch/block toggles between its 'off' combo (cid) and its 'on'
	// combo (cid + 1).
	const int CMB_GSWITCH = 3200;
	const int CMB_GBLOCK = 3202;
	const int CMB_GSWITCH_TIMED = 3204;
	const int CMB_LSWITCH = 3210;
	const int CMB_LBLOCK = 3212;
	// Toggles the global state through its trigger, rather than as a switch.
	const int CMB_GSTATE_TRIGGER = 3220;
	// Changes into the next combo when the global state changes.
	const int CMB_GSTATE_WATCHER = 3222;
	// Changes into the next combo on a timer.
	const int CMB_TIMED = 3224;

	const int CMB_STEPFX = 3230;
	const int CMB_STEP = 3240;
	const int CMB_STEPSAME = 3242;
	const int CMB_STEPALL = 3244;

	combodata addTrigger(combodata cd)
	{
		cd->NumTriggers = cd->NumTriggers + 1;
		return cd;
	}

	// A switch or block that toggles between cid and cid + 1.
	void setupToggle(int cid, int type, int state, bool global)
	{
		for (int i = 0; i < 2; i++)
		{
			combodata cd = Test::setupCombo(cid + i, type);
			cd->Attributes[0] = i == 0 ? 1 : -1;
			cd->Attributes[8] = state;
			cd->NumTriggers = 1;
			cd->Triggers[0]->Flags[TRIGFLAG_CTYPE_EFFECTS] = true;
			// Written last on purpose. Every combodata setter rebuilds the engine's
			// cached view of the combo's on-screen copies, which the switch and
			// block scan reads, so this flag's setter alone must do that refresh.
			cd->Flags[10] = global;
		}
		// Raised blocks are solid.
		if (type == CT_SWITCHBLOCK)
			Game->LoadComboData(cid)->Walk = 1111b;
	}

	void trigger(int p)
	{
		Test::Assert(Screen->TriggerCombo(0, p, 0), "TriggerCombo succeeds");
		Waitframe();
	}

	void testGlobalSwitch()
	{
		Test::Label("global switch at (2,2) lowers and raises the blocks at (5,5) (6,5)");
		// Placed in the screen data, so that it survives leaving and returning.
		setupToggle(CMB_GSWITCH, CT_SWITCH, GSTATE, true);
		setupToggle(CMB_GBLOCK, CT_SWITCHBLOCK, GSTATE, true);
		mapdata scr = Game->LoadMapData(Game->LoadDMapData(Test::TestingDmap)->Map, 0);
		for (int i = 0; i < 176; i++)
			scr->ComboD[i] = 0;
		scr->ComboD[Test::pos(2, 2)] = CMB_GSWITCH;
		scr->ComboD[Test::pos(5, 5)] = CMB_GBLOCK;
		scr->ComboD[Test::pos(6, 5)] = CMB_GBLOCK;
		Hero->Warp(Test::TestingDmap, 0);
		Waitframes(2);
		Hero->X = 32;
		Hero->Y = 128;
		Waitframe();
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "the blocks start raised");

		trigger(Test::pos(2, 2));
		Test::AssertEqual(Game->GlobalStates[GSTATE], -1, "the switch turns the global state on");
		Test::AssertEqual(Screen->ComboD[Test::pos(2, 2)], CMB_GSWITCH + 1, "the switch flips");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK + 1, "the blocks lower");
		Test::AssertEqual(Screen->ComboD[Test::pos(6, 5)], CMB_GBLOCK + 1, "the blocks lower");

		trigger(Test::pos(2, 2));
		Test::AssertEqual(Game->GlobalStates[GSTATE], 0, "the switch turns the global state off");
		Test::AssertEqual(Screen->ComboD[Test::pos(2, 2)], CMB_GSWITCH, "the switch flips back");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "the blocks rise");
	}

	void testTimedGlobalSwitch()
	{
		Test::Label("timed global switch at (3,2): the blocks rise again on their own");
		setupToggle(CMB_GSWITCH_TIMED, CT_SWITCH, GSTATE, true);
		Game->LoadComboData(CMB_GSWITCH_TIMED)->Attributes[2] = 30;
		Screen->ComboD[Test::pos(3, 2)] = CMB_GSWITCH_TIMED;
		Waitframe();

		trigger(Test::pos(3, 2));
		Test::Assert(Game->GlobalStates[GSTATE] > 0, "a timed switch turns the state on with a timer");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK + 1, "the blocks lower");
		Waitframes(40);
		Test::AssertEqual(Game->GlobalStates[GSTATE], 0, "the state times out");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "the blocks rise again");
		Test::AssertEqual(Screen->ComboD[Test::pos(3, 2)], CMB_GSWITCH_TIMED, "the switch flips back");
	}

	void testGlobalStateAcrossScreens()
	{
		Test::Label("global state across screens: a block on screen 1 matches it");
		// A block on another screen, which hasn't been loaded since the state changed.
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		mapdata other = Game->LoadMapData(map, 1);
		other->ComboD[Test::pos(5, 5)] = CMB_GBLOCK;

		trigger(Test::pos(2, 2));
		Test::AssertEqual(Game->GlobalStates[GSTATE], -1, "the state is on");

		Hero->Warp(Test::TestingDmap, 1);
		Waitframes(2);
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK + 1, "blocks on a newly loaded screen match the state");

		Hero->Warp(Test::TestingDmap, 0);
		Waitframes(2);
		Hero->X = 32;
		Hero->Y = 128;
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK + 1, "blocks stay lowered when returning");
		Test::AssertEqual(Screen->ComboD[Test::pos(2, 2)], CMB_GSWITCH + 1, "the switch stays flipped when returning");

		trigger(Test::pos(2, 2));
		Test::AssertEqual(Game->GlobalStates[GSTATE], 0, "the state is off");
	}

	void testGlobalStateTriggers()
	{
		Test::Label("'->GlobalState' at (8,2) toggles it, 'GlobalState->' at (9,2) fires");
		// '->GlobalState' toggles the state from any combo.
		combodata toggler = addTrigger(Test::setupCombo(CMB_GSTATE_TRIGGER, CT_NONE));
		toggler->Triggers[0]->Flags[TRIGFLAG_GLOBALSTATE] = true;
		toggler->Triggers[0]->GlobalState = GSTATE;
		Screen->ComboD[Test::pos(8, 2)] = CMB_GSTATE_TRIGGER;

		// 'GlobalState->' fires when the state changes.
		Test::setupCombo(CMB_GSTATE_WATCHER + 1, CT_NONE);
		combodata watcher = addTrigger(Test::setupCombo(CMB_GSTATE_WATCHER, CT_NONE));
		Screen->ComboD[Test::pos(9, 2)] = CMB_GSTATE_WATCHER;
		// Written after the combo is placed on purpose. Every combotrigger setter
		// rebuilds the engine's cached view of the combo's on-screen copies, so
		// these writes alone must make the global-state scan see the trigger.
		watcher->Triggers[0]->Flags[TRIGFLAG_TRIGGLOBALSTATE] = true;
		watcher->Triggers[0]->GlobalState = GSTATE;
		watcher->Triggers[0]->ComboChange = 1;
		Waitframe();

		trigger(Test::pos(8, 2));
		Test::AssertEqual(Game->GlobalStates[GSTATE], -1, "'->GlobalState' turns the state on");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK + 1, "the blocks lower");
		Test::AssertEqual(Screen->ComboD[Test::pos(9, 2)], CMB_GSTATE_WATCHER + 1, "'GlobalState->' fires");

		// With a timer, the state turns off on its own.
		toggler->Triggers[0]->GStateTimer = 20;
		trigger(Test::pos(8, 2));
		Test::Assert(Game->GlobalStates[GSTATE] > 0, "'->GlobalState' with a timer sets the timer");
		Waitframes(25);
		Test::AssertEqual(Game->GlobalStates[GSTATE], 0, "the state times out");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "the blocks rise");
	}

	void testLevelSwitch()
	{
		Test::Label("level switch at (2,7) toggles only the level block at (5,7)");
		setupToggle(CMB_LSWITCH, CT_SWITCH, LSTATE, false);
		setupToggle(CMB_LBLOCK, CT_SWITCHBLOCK, LSTATE, false);
		Screen->ComboD[Test::pos(2, 7)] = CMB_LSWITCH;
		Screen->ComboD[Test::pos(5, 7)] = CMB_LBLOCK;
		Waitframe();

		trigger(Test::pos(2, 7));
		Test::Assert(Game->LevelStates[Game->CurLevel] & (1Lb << LSTATE), "the switch turns the level state on");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 7)], CMB_LBLOCK + 1, "the level block lowers");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "global blocks are unaffected");

		trigger(Test::pos(2, 7));
		Test::Assert(!(Game->LevelStates[Game->CurLevel] & (1Lb << LSTATE)), "the switch turns the level state off");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 7)], CMB_LBLOCK, "the level block rises");
	}

	void testTriggeringABlock()
	{
		Test::Label("triggering the block at (6,5) directly toggles it and its ffc");
		// Triggering a block directly toggles just that block - and, for a
		// global one, any FFC on it.
		ffc f = Screen->LoadFFC(1);
		f->Data = CMB_GBLOCK;
		f->X = 6 * 16;
		f->Y = 5 * 16;
		Waitframe();
		trigger(Test::pos(6, 5));
		Test::AssertEqual(Screen->ComboD[Test::pos(6, 5)], CMB_GBLOCK + 1, "the block toggles");
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 5)], CMB_GBLOCK, "other blocks don't");
		Test::AssertEqual(f->Data, CMB_GBLOCK + 1, "the ffc on it toggles too");
	}

	void testTimedTrigger()
	{
		Test::Label("a timed trigger at (12,2) fires after 10 frames");
		Test::setupCombo(CMB_TIMED + 1, CT_NONE);
		combodata cd = addTrigger(Test::setupCombo(CMB_TIMED, CT_NONE));
		cd->Triggers[0]->ComboChange = 1;
		Screen->ComboD[Test::pos(12, 2)] = CMB_TIMED;
		Waitframe();
		// Written after the combo is placed on purpose. The timer's setter alone
		// must rebuild the engine's cached view of the combo's on-screen copies,
		// or the timer never starts counting.
		cd->Triggers[0]->Timer = 10;
		Waitframes(15);
		Test::AssertEqual(Screen->ComboD[Test::pos(12, 2)], CMB_TIMED + 1, "a timed trigger fires");
	}

	// Walks the Hero in a direction for some frames, as if the button were held.
	void walk(int button, int frames)
	{
		for (int i = 0; i < frames; i++)
		{
			WaitTo(SCR_TIMING_POST_POLL_INPUT);
			Input->Button[button] = true;
			Waitframe();
		}
	}

	combodata setupStepFX(int weapon, int dir)
	{
		Test::setupCombo(CMB_STEPFX + 1, CT_NONE);
		combodata cd = Test::setupCombo(CMB_STEPFX, CT_STEPFX);
		// A landmine, which spawns a weapon (with this damage) and then advances
		// to the next combo.
		cd->Flags[0] = true;
		cd->Attributes[0] = 2;
		cd->Attributes[9] = weapon;
		cd->Attributes[10] = dir;
		cd->NumTriggers = 1;
		cd->Triggers[0]->Flags[TRIGFLAG_CTYPE_EFFECTS] = true;
		return cd;
	}

	void testStepEffects()
	{
		Hero->X = 32;
		Hero->Y = 128;
		Hero->Dir = DIR_RIGHT;
		Waitframe();

		// Stepped on: an enemy arrow shot down, away from the Hero.
		Test::Label("step effect: walking onto it shoots an arrow down");
		setupStepFX(EW_ARROW, DIR_DOWN);
		Screen->ComboD[Test::pos(4, 8)] = CMB_STEPFX;
		for (int i = 0; i < 60 && Screen->ComboD[Test::pos(4, 8)] == CMB_STEPFX; i++)
			walk(CB_RIGHT, 1);
		printf("stepfx stepped: hero at %d, %d; %d arrows; combo %d\n", Hero->X, Hero->Y, Test::countEWeapons(EW_ARROW), Screen->ComboD[Test::pos(4, 8)]);
		Test::AssertEqual(Test::countEWeapons(EW_ARROW), 1, "stepping on it spawns its weapon");
		Test::AssertEqual(Screen->ComboD[Test::pos(4, 8)], CMB_STEPFX + 1, "and advances it");
		Test::clearWeapons();

		// Triggered: a Hero arrow.
		Test::Label("step effect: triggered, it shoots a hero arrow up");
		setupStepFX(LW_ARROW, DIR_UP);
		Screen->ComboD[Test::pos(10, 4)] = CMB_STEPFX;
		trigger(Test::pos(10, 4));
		Test::AssertEqual(Test::countLWeapons(LW_ARROW), 1, "triggering it spawns its weapon");
		Test::AssertEqual(Screen->ComboD[Test::pos(10, 4)], CMB_STEPFX + 1, "and advances it");
		Test::clearWeapons();

		// 'Don't Advance', and an unlisted weapon, which becomes an enemy bomb.
		Test::Label("step effect: an unlisted weapon becomes a bomb; 'Don't Advance'");
		combodata cd = setupStepFX(LW_SWORD, DIR_UP);
		cd->Flags[2] = true;
		Screen->ComboD[Test::pos(10, 4)] = CMB_STEPFX;
		trigger(Test::pos(10, 4));
		Test::AssertEqual(Test::countEWeapons(EW_BOMB), 1, "an unlisted weapon becomes an enemy bomb");
		Test::AssertEqual(Screen->ComboD[Test::pos(10, 4)], CMB_STEPFX, "'Don't Advance' keeps the combo");
		Test::clearWeapons();

		// A script weapon with 'direct damage' hurts the Hero only when stepped on.
		Test::Label("step effect: 'direct damage' hurts only when stepped on");
		cd = setupStepFX(LW_SCRIPT1, DIR_UP);
		cd->Flags[1] = true;
		cd->Flags[3] = true;
		cd->Attributes[0] = 8;
		Screen->ComboD[Test::pos(10, 4)] = CMB_STEPFX;
		int hp = Hero->HP;
		trigger(Test::pos(10, 4));
		Test::AssertEqual(Hero->HP, hp, "triggering doesn't hurt the Hero");
		Test::AssertEqual(Screen->NumLWeapons, 0, "and makes no weapon");
		Hero->X = 7 * 16;
		Hero->Y = 4 * 16;
		Hero->Dir = DIR_RIGHT;
		Screen->ComboD[Test::pos(8, 4)] = CMB_STEPFX;
		Waitframe();
		walk(CB_RIGHT, 20);
		printf("stepfx direct damage: lost %d hp\n", hp - Hero->HP);
		Test::AssertEqual(hp - Hero->HP, 8, "stepping on it hurts the Hero");
	}

	void testStepCombos()
	{
		for (int i = 0; i < 3; i++)
		{
			combodata cd = Test::setupCombo(CMB_STEP + i * 2, CT_STEP + i);
			cd->NumTriggers = 1;
			cd->Triggers[0]->Flags[TRIGFLAG_CTYPE_EFFECTS] = true;
			Test::setupCombo(CMB_STEP + i * 2 + 1, CT_NONE);
		}

		// Step->Next changes just itself.
		Test::Label("step->next at (2,9) changes only itself");
		Screen->ComboD[Test::pos(2, 9)] = CMB_STEP;
		Screen->ComboD[Test::pos(3, 9)] = CMB_STEP;
		trigger(Test::pos(2, 9));
		Test::AssertEqual(Screen->ComboD[Test::pos(2, 9)], CMB_STEP + 1, "step->next changes");
		Test::AssertEqual(Screen->ComboD[Test::pos(3, 9)], CMB_STEP, "the other doesn't");

		// Step->Next (Same) changes every combo like it.
		Test::Label("step->next (same) at (5,9) also changes (6,9)");
		Screen->ComboD[Test::pos(5, 9)] = CMB_STEPSAME;
		Screen->ComboD[Test::pos(6, 9)] = CMB_STEPSAME;
		trigger(Test::pos(5, 9));
		Test::AssertEqual(Screen->ComboD[Test::pos(5, 9)], CMB_STEPSAME + 1, "step->next (same) changes");
		Test::AssertEqual(Screen->ComboD[Test::pos(6, 9)], CMB_STEPSAME + 1, "and so does the same combo");
		Test::AssertEqual(Screen->ComboD[Test::pos(3, 9)], CMB_STEP, "but not a different one");

		// Step->Next (All) changes every step combo.
		Test::Label("step->next (all) at (8,9) changes every step combo");
		Screen->ComboD[Test::pos(8, 9)] = CMB_STEPALL;
		trigger(Test::pos(8, 9));
		Test::AssertEqual(Screen->ComboD[Test::pos(8, 9)], CMB_STEPALL + 1, "step->next (all) changes");
		Test::AssertEqual(Screen->ComboD[Test::pos(3, 9)], CMB_STEP + 1, "and so does every other step combo");
	}

	void run()
	{
		Test::Init();

		testGlobalSwitch();
		testTimedGlobalSwitch();
		testGlobalStateAcrossScreens();
		testGlobalStateTriggers();
		testLevelSwitch();
		testTriggeringABlock();
		testTimedTrigger();
		testStepEffects();
		testStepCombos();

		Test::End();
	}
}
