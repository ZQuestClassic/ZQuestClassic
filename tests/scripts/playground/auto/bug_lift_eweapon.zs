#include "auto/test_runner.zs"

// Lifting an eweapon kept it in the list of eweapons while held, and releasing it added it to the
// list of lweapons too - so freeing it from one list left it in the other.

generic script bug_lift_eweapon
{
	void run()
	{
		Test::Init();

		eweapon e = Screen->CreateEWeapon(EW_ARROW);
		Hero->LiftWeapon(<lweapon>(e), 10, 16);
		Test::Assert(!Hero->LiftedWeapon, "an eweapon can't be lifted");

		lweapon l = Screen->CreateLWeapon(LW_ARROW);
		Hero->LiftWeapon(l, 10, 16);
		Test::Assert(Hero->LiftedWeapon == l, "an lweapon can be lifted");
		Hero->ReleaseLiftWeapon();

		Screen->ClearSprites(SL_EWPNS);
		Waitframe();
		Test::AssertEqual(Screen->NumLWeapons(), 1);

		Test::End();
	}
}
