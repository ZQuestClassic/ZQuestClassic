#include "auto/test_runner.zs"

generic script instant_reload_on_death
{
	void run()
	{
		Test::Init();

		// This replay test is funky - let it run once manually (die, reload game)
		// then hold down the A button to actually end the test.
		if (Input->Button[CB_A]) return;

		Game->FFRules[qr_INSTANT_RESPAWN] = true;
		Hero->HP = 0;
		while (true) Waitframe();
	}
}
