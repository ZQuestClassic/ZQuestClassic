// https://discord.com/channels/876899628556091432/1449284207783317658
//
// Draw commands on the classic-only special draw layers (SPLAYER_NPC_DRAW,
// etc.) never render when the 'Classic Drawing Order' quest rule is off -
// draw_screen only flushes those queues in its classic branch. Each line of
// script code that draws to one now logs an error, once. The replay's
// recorded comments lock the error firing exactly once per draw location,
// and not at all for working layers or under classic drawing order.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_deprecated_draw_layer
{
	void run()
	{
		Test::Init();

		// playground.qst uses classic drawing order, where this timing
		// renders normally - no error.
		Screen->FastTile(SPLAYER_NPC_DRAW, 0, 0, 100, 1);
		Waitframe();

		Game->FFRules[qr_CLASSIC_DRAWING_ORDER] = false;

		// Now this timing never renders. Errors once, despite queueing a
		// draw every frame.
		for (int i = 0; i < 3; ++i)
		{
			Screen->FastTile(SPLAYER_NPC_DRAW, 0, 0, 100, 1);
			Waitframe();
		}

		// A second code location gets its own error.
		Screen->FastTile(SPLAYER_ITEMSPRITE_DRAW, 16, 0, 100, 1);

		// This special layer still draws under the new drawing order - no error.
		Screen->FastTile(SPLAYER_FAIRYITEM_DRAW, 32, 0, 100, 1);
		Waitframe();

		Test::End();
	}
}
