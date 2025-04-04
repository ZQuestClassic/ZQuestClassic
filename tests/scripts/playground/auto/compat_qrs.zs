#include "auto/test_runner.zs"

generic script compat_qrs
{
	void run()
	{
		Test::Init();

		Test::loadRegion(0, 4);
		Hero->Warp(Test::TestingDmap, 0);
		Waitframe();
		Hero->X = 130;
		Hero->Y = 50;

		bitmap b1 = Game->CreateBitmap(16, 16);
		bitmap b2 = Game->LoadBitmapID(RT_SCREEN);

		Game->FFRules[qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN] = true;
		int i = 0;
		int frames = 60 * 1.5;

		// This bitmap will be inadvertently cropped. Theoretically, a 2.55.9 quest could
		// work around this and have it draw correctly, so must keep supporting it.

		while (i++ < frames)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, RT_SCREEN, 0, 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		i = 0;
		while (i++ < frames)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, b2, 0 , 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		Game->FFRules[qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN] = false;
		i = 0;

		// Should now draw correctly - a shadow of the player just above it.

		while (i++ < frames)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, RT_SCREEN, 0, 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}

		i = 0;
		while (i++ < frames)
		{
			b1->Clear(0);
			b1->FastTile(0, 0, 0, Hero->Tile, Hero->CSet, OP_OPAQUE);
			b1->ReplaceColors(0, 0x71, 0x01, 0xFF);
			b1->Blit(6, b2, 0 , 0, 16, 16, Hero->X, Hero->Y-16, 16, 16, 0, 0, 0, BITDX_NORMAL, 0, true);
			Waitframe();
		}
	}
}
