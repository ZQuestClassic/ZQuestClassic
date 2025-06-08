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
	}
}
