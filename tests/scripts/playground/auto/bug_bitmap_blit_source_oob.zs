// Regression test for bitmap->Blit / bitmap->BlitTo crashing when the
// source rect reaches outside the source bitmap. Allegro's stretch blits
// don't clip the source rect, so a source rect starting above or left of
// the bitmap read out of bounds (a rect past the right/bottom edge was
// already handled). A negative source size also crashed, since it was
// used as the size of a temporary bitmap. All of these should just draw
// the in-bounds part of the rect, or nothing, and the script should run
// to completion.

#include "auto/test_runner.zs"

generic script bug_bitmap_blit_source_oob
{
	void run()
	{
		Test::Init();

		bitmap b = Game->CreateBitmap(256, 176);
		b->ClearToColor(0, 0x5);
		b->Rectangle(0, 0, 0, 255, 175, 0x14, 1, 0, 0, 0, false, OP_OPAQUE);

		int i = 0;
		while (i++ < 4)
		{
			// Off the top edge, stretched.
			b->Blit(6, RT_SCREEN, 0, -8, 256, 168, 0, 0, 256, 176, 0, 0, 0, BITDX_NORMAL, 0, true);
			// Off the left edge, stretched, translucent.
			b->Blit(6, RT_SCREEN, -8, 0, 240, 176, 16, 16, 256, 176, 0, 0, 0, BITDX_TRANS, 0, true);
			// Off the top-left corner, rotated.
			b->Blit(6, RT_SCREEN, -16, -16, 64, 64, 64, 64, 64, 64, 45, 0, 0, BITDX_NORMAL, 0, true);
			// Off the right edge, unmasked.
			b->Blit(6, RT_SCREEN, 200, 0, 100, 50, 0, 100, 100, 50, 0, 0, 0, BITDX_NORMAL, 0, false);
			// Negative source height, off the right edge.
			b->Blit(6, RT_SCREEN, 200, 0, 100, -8, 0, 0, 100, 8, 0, 0, 0, BITDX_NORMAL, 0, true);
			// Negative source width, off the top edge.
			b->Blit(6, RT_SCREEN, 0, -8, -100, 50, 0, 0, 100, 50, 0, 0, 0, BITDX_NORMAL, 0, true);
			// Entirely outside the source bitmap.
			b->Blit(6, RT_SCREEN, 300, 300, 32, 32, 0, 0, 64, 64, 0, 0, 0, BITDX_NORMAL, 0, true);
			b->Blit(6, RT_SCREEN, -100, -100, 32, 32, 0, 0, 64, 64, 0, 0, 0, BITDX_NORMAL, 0, true);
			// Same cases for BlitTo (this bitmap is the destination).
			bitmap scr = Game->LoadBitmapID(RT_SCREEN);
			b->BlitTo(6, scr, 0, -8, 256, 168, 0, 0, 256, 176, 0, 0, 0, BITDX_NORMAL, 0, true);
			b->BlitTo(6, scr, -8, -8, 240, 100, 0, 0, 240, 100, 0, 0, 0, BITDX_NORMAL, 0, true);
			b->BlitTo(6, scr, 200, 0, 100, -8, 0, 0, 100, 8, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		Test::End();
	}
}
