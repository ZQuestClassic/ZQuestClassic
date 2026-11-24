// An lweapon script that lifts itself stops there for the frame (like a Waitframe), since the
// weapon was just taken out of the weapon list. Once the weapon is released back into the weapon
// list, its script continues after the lift.
//
// With the JIT, lifting by assigning Hero->LiftedWeapon didn't stop the script, and left the stop
// request pending for whatever script ran an interpreted command next. Lifting with
// Hero->LiftWeapon() resumed the script at the wrong place once released (re-running the code
// before the call).
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

		run_test("bug_lifted_weapon_self_assign", 1, 2);
		run_test("bug_lifted_weapon_self_call", 11, 12);

		Test::End();
	}

	void run_test(char32[] script_name, int steps_lifted, int steps_released)
	{
		lweapon w = Screen->CreateLWeapon(LW_SCRIPT1);
		w->X = 64;
		w->Y = 64;
		w->Script = Game->GetLWeaponScript(script_name);
		for (int i = 0; i < 4; ++i)
		{
			printf("lifted frame %d: steps %d\n", i, bug_lifted_weapon_self_steps);
			Waitframe();
		}
		// The weapon's script doesn't run while it is lifted.
		Test::AssertEqual(Hero->LiftedWeapon, w);
		Test::AssertEqual(bug_lifted_weapon_self_steps, steps_lifted);

		// Back in the weapon list, its script continues after the lift.
		Test::AssertEqual(Hero->ReleaseLiftWeapon(), w);
		for (int i = 0; i < 4; ++i)
		{
			printf("released frame %d: steps %d\n", i, bug_lifted_weapon_self_steps);
			Waitframe();
		}
		Test::AssertEqual(Hero->LiftedWeapon, NULL);
		Test::AssertEqual(bug_lifted_weapon_self_steps, steps_released);

		w->Remove();
		Waitframe();
	}
}
