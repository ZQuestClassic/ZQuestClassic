// A script array that refers to engine data (like `combodata->Flags`) carries
// the object it came from, so it keeps working after the script touches some
// other object of the same type. Three combodata arrays instead looked at the
// combodata the script touched most recently: Flags[] refreshed the engine's
// cached view of that combo rather than the one written to, and TrigFlags[] /
// TrigButton[] read and wrote that combo's first trigger outright.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_combodata_arrays_stale_ref
{
	const int GSTATE = 7;

	// A global switch, toggling between 'off' (cid) and 'on' (cid + 1).
	const int CMB_SWITCH = 3300;
	// Changes into the next combo when the global state changes.
	const int CMB_WATCHER = 3302;
	// Scratch combos the script touches in between.
	const int CMB_A = 3310;
	const int CMB_B = 3311;
	const int CMB_OTHER = 3312;

	int tile;

	combodata setupCombo(int cid, int type)
	{
		combodata cd = Game->LoadComboData(cid);
		cd->Type = type;
		cd->Tile = tile;
		cd->Walk = 0;
		for (int i = 0; i < 16; i++)
			cd->Flags[i] = false;
		for (int i = 0; i < 24; i++)
			cd->Attributes[i] = 0;
		cd->NumTriggers = 1;
		return cd;
	}

	// Points the engine's combodata register at a different combo.
	void touchOther()
	{
		combodata other = Game->LoadComboData(CMB_OTHER);
		other->Walk = other->Walk;
	}

	int pos(int col, int row)
	{
		return col + row * 16;
	}

	void testFlagsRefreshTheRightCombo()
	{
		for (int i = 0; i < 2; i++)
		{
			combodata cd = setupCombo(CMB_SWITCH + i, CT_SWITCH);
			cd->Attributes[0] = i == 0 ? 1 : -1;
			cd->Attributes[8] = GSTATE;
			cd->Triggers[0]->Flags[TRIGFLAG_CTYPE_EFFECTS] = true;

			// Turn on 'Global State' through a stored array, after moving on to
			// another combo. The flag itself always landed on the right combo;
			// the engine's cached view of it is what went stale.
			bool[] flags = cd->Flags;
			touchOther();
			flags[10] = true;
			Test::AssertEqual(cd->Flags[10], true, "the flag is set on the right combo");
			Test::AssertEqual(Game->LoadComboData(CMB_OTHER)->Flags[10], false, "and not on the other");
		}

		Screen->ComboD[pos(2, 2)] = CMB_SWITCH;
		Waitframe();
		Test::Assert(Screen->TriggerCombo(0, pos(2, 2), 0), "TriggerCombo succeeds");
		Waitframe();
		Test::AssertEqual(Game->GlobalStates[GSTATE], -1, "the switch toggles the global state, not a level state");
		Test::AssertEqual(Screen->ComboD[pos(2, 2)], CMB_SWITCH + 1, "the switch flips");
	}

	void testTrigFlagsUseTheRightCombo()
	{
		combodata a = setupCombo(CMB_A, CT_NONE);
		combodata b = setupCombo(CMB_B, CT_NONE);

		bool[] aflags = a->TrigFlags;
		b->Walk = b->Walk;
		aflags[TRIGFLAG_LIGHTON] = true;
		Test::AssertEqual(a->Triggers[0]->Flags[TRIGFLAG_LIGHTON], true, "the write lands on a");
		Test::AssertEqual(b->Triggers[0]->Flags[TRIGFLAG_LIGHTON], false, "and not on b");

		b->Triggers[0]->Flags[TRIGFLAG_SHUTTER] = true;
		Test::AssertEqual(aflags[TRIGFLAG_SHUTTER], false, "the read comes from a");
		Test::AssertEqual(aflags[TRIGFLAG_LIGHTON], true, "the read comes from a");

		// The engine's cached view of the combo must follow the written combo
		// too: a 'GlobalState->' trigger set this way has to fire.
		setupCombo(CMB_WATCHER + 1, CT_NONE);
		combodata watcher = setupCombo(CMB_WATCHER, CT_NONE);
		watcher->Triggers[0]->GlobalState = GSTATE;
		watcher->Triggers[0]->ComboChange = 1;
		Screen->ComboD[pos(4, 2)] = CMB_WATCHER;
		Waitframe();
		bool[] wflags = watcher->TrigFlags;
		touchOther();
		wflags[TRIGFLAG_TRIGGLOBALSTATE] = true;

		Test::Assert(Screen->TriggerCombo(0, pos(2, 2), 0), "TriggerCombo succeeds");
		Waitframe();
		Test::AssertEqual(Game->GlobalStates[GSTATE], 0, "the switch turns the global state off");
		Test::AssertEqual(Screen->ComboD[pos(4, 2)], CMB_WATCHER + 1, "'GlobalState->' fires");
	}

	void testTrigButtonUsesTheRightCombo()
	{
		combodata a = Game->LoadComboData(CMB_A);
		combodata b = Game->LoadComboData(CMB_B);

		bool[] abuttons = a->TrigButton;
		b->Walk = b->Walk;
		abuttons[0] = true;
		Test::AssertEqual(a->Triggers[0]->Button[0], true, "the write lands on a");
		Test::AssertEqual(b->Triggers[0]->Button[0], false, "and not on b");

		b->Triggers[0]->Button[1] = true;
		Test::AssertEqual(abuttons[1], false, "the read comes from a");
		Test::AssertEqual(abuttons[0], true, "the read comes from a");
	}

	void run()
	{
		Test::Init();
		tile = Game->LoadComboData(12)->Tile;

		testFlagsRefreshTheRightCombo();
		testTrigFlagsUseTheRightCombo();
		testTrigButtonUsesTheRightCombo();

		Test::End();
	}
}
