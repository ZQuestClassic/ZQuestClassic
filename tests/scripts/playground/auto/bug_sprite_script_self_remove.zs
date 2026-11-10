// A weapon or enemy script that removes its own sprite. The sprite's script
// state is freed along with the sprite, so the script engine must not touch
// either again on its way out of the script.
//
// A sprite in a sprite list is only marked for deletion while it runs, but a
// weapon the hero has lifted is deleted on the spot.

#include "std.zh"
#include "auto/test_runner.zs"

int self_remove_runs = 0;
int self_remove_resumes = 0;

lweapon script bug_sprite_script_self_remove_lweapon
{
	void run()
	{
		Waitframe();
		self_remove_runs++;
		this->Remove();
		self_remove_resumes++;
	}
}

eweapon script bug_sprite_script_self_remove_eweapon
{
	void run()
	{
		Waitframe();
		self_remove_runs++;
		this->Remove();
		self_remove_resumes++;
	}
}

npc script bug_sprite_script_self_remove_npc
{
	void run()
	{
		Waitframe();
		self_remove_runs++;
		this->Remove();
		self_remove_resumes++;
	}
}

// Lets go of itself, which puts it back on the screen, then removes itself.
lweapon script bug_sprite_script_self_remove_release
{
	void run()
	{
		Waitframe();
		self_remove_runs++;
		Hero->ReleaseLiftWeapon();
		Waitframe();
		this->Remove();
		self_remove_resumes++;
	}
}

// Deletes itself by clearing the hero's lifted weapon. That doesn't stop the
// script, which runs to its end.
lweapon script bug_sprite_script_self_remove_clear
{
	void run()
	{
		Waitframe();
		Hero->LiftedWeapon = NULL;
		self_remove_runs++;
	}
}

generic script bug_sprite_script_self_remove
{
	void run()
	{
		Test::Init();

		Hero->X = 16;
		Hero->Y = 16;

		int lweapon_script = Game->GetLWeaponScript("bug_sprite_script_self_remove_lweapon");
		int eweapon_script = Game->GetEWeaponScript("bug_sprite_script_self_remove_eweapon");
		int npc_script = Game->GetNPCScript("bug_sprite_script_self_remove_npc");
		Test::Assert(lweapon_script > 0, "lweapon script exists");
		Test::Assert(eweapon_script > 0, "eweapon script exists");
		Test::Assert(npc_script > 0, "npc script exists");

		// Several rounds, so that sprites made in a later round take the place of
		// the script state freed in an earlier one.
		const int ROUNDS = 4;
		int runs = 0;
		for (int round = 0; round < ROUNDS; round++)
		{
			lweapon lw = Screen->CreateLWeapon(LW_SCRIPT1);
			lw->X = 128;
			lw->Y = 80;
			lw->Script = lweapon_script;

			eweapon ew = Screen->CreateEWeapon(EW_SCRIPT1);
			ew->X = 160;
			ew->Y = 80;
			ew->Script = eweapon_script;

			npc n = Screen->CreateNPC(NPC_OCTOROCK1S);
			n->X = 192;
			n->Y = 80;
			n->Script = npc_script;

			Waitframes(4);

			runs += 3;
			Test::AssertEqual(self_remove_runs, runs, "every sprite script ran");
			Test::AssertEqual(Screen->NumLWeapons, 0, "lweapon removed itself");
			Test::AssertEqual(Screen->NumEWeapons, 0, "eweapon removed itself");
			Test::AssertEqual(Screen->NumNPCs, 0, "npc removed itself");
		}

		// The same lweapon script, on a weapon the hero has lifted.
		Game->FFRules[qr_LIFTED_WEAPONS_RUN_SCRIPTS] = true;
		for (int round = 0; round < ROUNDS; round++)
		{
			lweapon lw = Screen->CreateLWeapon(LW_SCRIPT1);
			lw->Script = lweapon_script;
			Hero->LiftedWeapon = lw;
			Test::Assert(Hero->LiftedWeapon, "hero lifted the weapon");

			Waitframes(4);

			runs++;
			Test::AssertEqual(self_remove_runs, runs, "the lifted weapon's script ran");
			Test::Assert(!Hero->LiftedWeapon, "lifted weapon removed itself");
			Test::AssertEqual(Screen->NumLWeapons, 0, "lifted weapon is not on the screen");
		}

		int release_script = Game->GetLWeaponScript("bug_sprite_script_self_remove_release");
		int clear_script = Game->GetLWeaponScript("bug_sprite_script_self_remove_clear");
		Test::Assert(release_script > 0, "release lweapon script exists");
		Test::Assert(clear_script > 0, "clear lweapon script exists");
		for (int round = 0; round < ROUNDS; round++)
		{
			lweapon lw = Screen->CreateLWeapon(LW_SCRIPT1);
			lw->Script = round % 2 ? release_script : clear_script;
			Hero->LiftedWeapon = lw;

			Waitframes(6);

			runs++;
			Test::AssertEqual(self_remove_runs, runs, "the lifted weapon's script ran");
			Test::Assert(!Hero->LiftedWeapon, "lifted weapon is gone");
			Test::AssertEqual(Screen->NumLWeapons, 0, "lifted weapon is not on the screen");
		}
		Game->FFRules[qr_LIFTED_WEAPONS_RUN_SCRIPTS] = false;

		Test::AssertEqual(self_remove_resumes, 0, "no script ran past removing its sprite");

		Test::End();
	}
}
