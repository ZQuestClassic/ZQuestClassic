#include "std.zh"
#include "auto/test_runner.zs"

// 2.50.0 and 2.50.1 let the player turn around while charging the quake
// hammer. 2.50.2 stopped that, without a compat rule for quests made before.
generic script compat_hammer_charge_turn
{
	// Charges the hammer (holding A), then presses left while still charging.
	// Returns the direction the player faces after that.
	int chargeAndTurn()
	{
		Hero->Dir = DIR_DOWN;
		int i = 0;
		while (i++ < 40)
		{
			// Input is polled right before this timing, so this write is what the player sees.
			WaitTo(SCR_TIMING_POST_POLL_INPUT);
			Input->Button[CB_B] = true;
			Waitframe();
		}
		Test::AssertEqual(Hero->Action, LA_CHARGING, "hammer is charging");
		i = 0;
		while (i++ < 10)
		{
			WaitTo(SCR_TIMING_POST_POLL_INPUT);
			Input->Button[CB_B] = true;
			Input->Button[CB_LEFT] = true;
			Waitframe();
		}
		int dir = Hero->Dir;
		// Let go, and wait for the hammer swing to finish.
		Waitframes(90);
		return dir;
	}

	void run()
	{
		Test::Init();

		Hero->Item[I_HAMMER] = true;
		Hero->Item[I_QUAKESCROLL1] = true;
		Hero->ItemB = I_HAMMER;
		Hero->MaxMP = 256;
		Hero->MP = 256;
		Waitframe();

		Game->FFRules[qr_TURN_WHILE_CHARGING_HAMMER] = false;
		Test::AssertEqual(chargeAndTurn(), DIR_DOWN, "can't turn while charging the hammer");

		Game->FFRules[qr_TURN_WHILE_CHARGING_HAMMER] = true;
		Test::AssertEqual(chargeAndTurn(), DIR_LEFT, "can turn while charging the hammer with the compat rule");

		Test::End();
	}
}
