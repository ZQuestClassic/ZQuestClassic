// A scripted eweapon that the hero's mirror shield reflects becomes an lweapon
// in place. It kept its script number, which then picked a script from the
// lweapon script table: whichever lweapon script had that number started
// running on it.
//
// To see that regardless of how scripts are numbered, the eweapon is given the
// number of this test's lweapon script, with eweapon scripts suspended so that
// number never runs as an eweapon script.

#include "std.zh"
#include "auto/test_runner.zs"

int reflected_kept_script_starts = 0;

lweapon script bug_reflected_eweapon_kept_script_counter
{
	void run()
	{
		reflected_kept_script_starts++;
		while (true)
			Waitframe();
	}
}

generic script bug_reflected_eweapon_kept_script
{
	void run()
	{
		Test::Init();

		// Otherwise the shield blocks enemy arrows instead of reflecting them.
		Game->FFRules[qr_BROKEN_FLAME_ARROW_REFLECTING] = false;

		// Give the hero a passive shield that blocks and reflects arrows on every side.
		int shield_id = GetHighestLevelItemOwned(IC_SHIELD);
		Test::Assert(shield_id >= 0, "hero owns a shield");
		itemdata shield = Game->LoadItemData(shield_id);
		shield->LAttributes[0] |= L_BLOCKFLAG_ARROW;
		shield->LAttributes[1] |= L_BLOCKFLAG_ARROW;
		for (int i = 0; i < 4; i++)
			shield->Flags[i] = true;
		shield->Flags[8] = false;

		Hero->X = 16;
		Hero->Y = 80;
		Hero->Dir = DIR_RIGHT;

		Game->Suspend[susptEWEAPONSCRIPTS] = true;
		int counter_script = Game->GetLWeaponScript("bug_reflected_eweapon_kept_script_counter");
		Test::Assert(counter_script > 0, "counter lweapon script exists");

		eweapon w = Screen->CreateEWeapon(EW_ARROW);
		w->X = Hero->X + 80;
		w->Y = Hero->Y;
		w->Dir = DIR_LEFT;
		w->Step = 200;
		w->Script = counter_script;

		int frames = 0;
		while (Screen->NumLWeapons == 0)
		{
			if (++frames > 60)
				Test::Fail("eweapon never reflected off the mirror shield");
			Waitframe();
		}
		Test::AssertEqual(Screen->NumEWeapons, 0, "reflected weapon left the eweapon list");
		Waitframes(2);

		lweapon reflected = Screen->LWeapons[0];
		Test::AssertEqual(reflected_kept_script_starts, 0, "reflected weapon did not start an lweapon script");
		Test::AssertEqual(reflected->Script, 0, "reflected weapon dropped its eweapon script number");
		reflected->Remove();
		Game->Suspend[susptEWEAPONSCRIPTS] = false;

		Test::End();
	}
}
