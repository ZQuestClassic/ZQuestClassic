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

		// Give the hero a shield that blocks and reflects fireballs (shFIREBALL)
		// from the front.
		int shield_flag_fireball = 8;
		itemdata shield = Game->LoadItemData(I_SHIELD3);
		shield->Family = IC_SHIELD;
		shield->Attributes[0] = shield_flag_fireball;
		shield->Attributes[1] = shield_flag_fireball;
		shield->Flags[0] = true;
		Hero->Item[I_SHIELD3] = true;

		Hero->X = 64;
		Hero->Y = 80;
		Hero->Dir = DIR_RIGHT;

		Game->Suspend[susptEWEAPONSCRIPTS] = true;
		int counter_script = Game->GetLWeaponScript("bug_reflected_eweapon_kept_script_counter");
		Test::Assert(counter_script > 0, "counter lweapon script exists");

		eweapon w = Screen->CreateEWeapon(EW_FIREBALL);
		w->X = 160;
		w->Y = 80;
		w->Angular = false;
		w->Dir = DIR_LEFT;
		w->Step = 200;
		w->Script = counter_script;

		int frames = 0;
		while (Screen->NumLWeapons() == 0)
		{
			if (++frames > 90)
				Test::Fail("eweapon never reflected off the shield");
			Waitframe();
		}
		Test::AssertEqual(Screen->NumEWeapons(), 0, "reflected weapon left the eweapon list");
		Waitframes(2);

		lweapon reflected = Screen->LoadLWeapon(1);
		Test::AssertEqual(reflected_kept_script_starts, 0, "reflected weapon did not start an lweapon script");
		Test::AssertEqual(reflected->Script, 0, "reflected weapon dropped its eweapon script number");
		reflected->Remove();
		Game->Suspend[susptEWEAPONSCRIPTS] = false;

		Test::End();
	}
}
