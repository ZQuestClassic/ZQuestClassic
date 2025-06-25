#include "auto/test_runner.zs"

generic script bitmap_load_rt_screen
{
	void run()
	{
		Test::Init();

		bitmap b1 = Game->CreateBitmap(16, 16);
		bitmap b2 = Game->LoadBitmapID(RT_SCREEN);

		int i = 0;
		while (i++ < 60)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, RT_SCREEN, 0, 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		// Passing b2 to Blit should be exact same as passing RT_SCREEN.
		i = 0;
		while (i++ < 60)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, b2, 0, 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		// Clear the screen bitmap - brownout for 60 frames.
		i = 0;
		while (i++ < 60)
		{
			b2->ClearToColor(1, 5); // brown
			Waitframe();
		}

		// Blit from the screen bitmap to a user bitmap, then back to the screen.
		i = 0;
		while (i++ < 60)
		{
			int lyr = 1;
			b2->ClearToColor(lyr, 5); // brown
			// Blit the top-left on the screen to the 16x16 user bitmap.
			b2->Blit(lyr, b1, 0, 0, 16, 16, 0, 0, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			// b1 is brown now
			b2->ClearToColor(lyr, 1); // white
			// draw b1 to the top-left.
			b1->Blit(lyr, b2, 0, 0, 16, 16, 0, 0, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			// result: white screen, but top-left is brown.
			Waitframe();
		}
	}
}
