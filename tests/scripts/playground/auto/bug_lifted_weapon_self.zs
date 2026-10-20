// An lweapon script that lifts itself asks the engine to stop the script there for the frame
// (like a Waitframe), since the weapon was just taken out of the weapon list.
//
// Lifting by assigning Hero->LiftedWeapon aborted the program: the interpreter only allowed that
// request from commands (like Hero->LiftWeapon()), and the JIT never saw it, leaving it pending
// for whatever script ran an interpreted command next.
//
// Lifting with Hero->LiftWeapon() under the JIT resumed the script at the wrong place the next
// frame (re-running the code before the call).
//
// The replay's recorded printf output locks where the scripts stop and resume.

#include "std.zh"
#include "auto/test_runner.zs"

int bug_lifted_weapon_self_steps;

lweapon script bug_lifted_weapon_self_assign
{
	void run()
	{
		printf("assign: before\n");
		bug_lifted_weapon_self_steps = 1;
		Hero->LiftedWeapon = this;
		// The script stops at the assignment; this runs on the next frame, when the hero runs
		// the lifted weapon's script (qr_LIFTED_WEAPONS_RUN_SCRIPTS).
		printf("assign: after\n");
		bug_lifted_weapon_self_steps = 2;
		while (true)
			Waitframe();
	}
}

lweapon script bug_lifted_weapon_self_call
{
	void run()
	{
		printf("call: before\n");
		bug_lifted_weapon_self_steps = 11;
		Hero->LiftWeapon(this, 0, 0);
		printf("call: after\n");
		bug_lifted_weapon_self_steps = 12;
		while (true)
			Waitframe();
	}
}

generic script bug_lifted_weapon_self
{
	void run()
	{
		Test::Init();

		// So the hero runs the lifted weapon's script, letting it continue past the lift.
		Game->FFRules[qr_LIFTED_WEAPONS_RUN_SCRIPTS] = true;

		lweapon w = Screen->CreateLWeapon(LW_SCRIPT1);
		w->X = 64;
		w->Y = 64;
		w->Script = Game->GetLWeaponScript("bug_lifted_weapon_self_assign");
		for (int i = 0; i < 4; ++i)
		{
			printf("frame %d: steps %d\n", i, bug_lifted_weapon_self_steps);
			Waitframe();
		}
		Test::AssertEqual(Hero->LiftedWeapon, w);
		Test::AssertEqual(bug_lifted_weapon_self_steps, 2);

		Hero->ReleaseLiftWeapon()->Remove();
		Waitframe();

		w = Screen->CreateLWeapon(LW_SCRIPT1);
		w->X = 64;
		w->Y = 64;
		w->Script = Game->GetLWeaponScript("bug_lifted_weapon_self_call");
		for (int i = 0; i < 4; ++i)
		{
			printf("frame %d: steps %d\n", i, bug_lifted_weapon_self_steps);
			Waitframe();
		}
		Test::AssertEqual(Hero->LiftedWeapon, w);
		Test::AssertEqual(bug_lifted_weapon_self_steps, 12);

		Test::End();
	}
}
