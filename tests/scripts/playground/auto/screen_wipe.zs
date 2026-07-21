// https://discord.com/channels/876899628556091432/1319166577660395633
//
// Runs a closing + opening wipe of every type. Notably, a fade-to-black
// closing wipe must end black (no bright flash on its last frame) and hold
// the screen black until the opening wipe fades it back in. The replay's
// gfx hashes guard this.

#include "auto/test_runner.zs"

generic script screen_wipe
{
	void run()
	{
		Test::Init();

		for (int effect = 0; effect < WIPE_MAX; effect++)
		{
			Screen->ClosingWipe(effect);
			Screen->OpeningWipe(effect);
		}

		Test::End();
	}
}
