// Regression test for an out-of-bounds read in Screen->DrawMiniTile /
// bitmap->DrawMiniTile. The script-supplied tile and corner formed
// minitile = (tile << 2) + corner, which was passed straight to
// overtile8 / overtiletranslucent8. Those helpers (unlike overtile16,
// used by DrawTile) did not range-check the index before reading
// blank_tile_quarters_table[minitile] and newtilebuf[minitile>>2], so a
// large or negative tile read out of bounds and crashed the player. The
// helpers now ignore out-of-range quarter-tile indices, so these draws
// are no-ops and the script runs to completion.

#include "auto/test_runner.zs"

generic script bug_draw_minitile_oob
{
	void run()
	{
		Test::Init();

		// NEWMAXTILES is 214500; valid tiles are 0..214499. Exercise both
		// the opaque (overtile8) and translucent (overtiletranslucent8)
		// paths with out-of-range tile values, positive and negative, at
		// an on-screen position so the draws actually render.
		Screen->DrawMiniTile(6, 40, 40, 214500, CORNER_UL, 0, 128);   // tile == NEWMAXTILES
		Screen->DrawMiniTile(6, 40, 40, 999999, CORNER_BR, 0, 128);   // tile far out of range
		Screen->DrawMiniTile(6, 40, 40, -1, CORNER_UL, 0, 128);       // negative tile
		Screen->DrawMiniTile(6, 40, 40, 214500, CORNER_UL, 0, 64);    // translucent, out of range
		Screen->DrawMiniTile(6, 40, 40, -5, CORNER_BR, 0, 64);        // translucent, negative

		// The out-of-bounds read fired at render time, so let the queued
		// draws render before the test exits.
		Waitframe();
		Waitframe();

		Test::End();
	}
}
