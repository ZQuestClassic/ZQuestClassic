#include "std.zh"

dmapdata script ScrollingDebug
{
	bool startedInRegion;

	void jankyOldScrollingVars()
	{
		int subscreenY = 232 - Viewport->Height;
		Screen->DrawString(7, 150, -subscreenY, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "Janky", 128);

		printf("OX/OY %d %d\n", Game->Scrolling[SCROLL_OX], Game->Scrolling[SCROLL_OY]);
		Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
		Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
		Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

		printf("NX/NY %d %d\n", Game->Scrolling[SCROLL_NX], Game->Scrolling[SCROLL_NY]);
		Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
		Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
		Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);

		printf("ORX/ORY %d %d\n", Game->Scrolling[SCROLL_ORX], Game->Scrolling[SCROLL_ORY]);
		printf("NRX/NRY %d %d\n", Game->Scrolling[SCROLL_NRX], Game->Scrolling[SCROLL_NRY]);

		int x = 0;
		int y = 0;
		int w = Viewport->Width - 1;
		int h = Viewport->Height - 9;

		Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, 0x72, 1, 0, 0, 0, false, 128);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// This draws where the hero is.
			Waitdraw(); // Without this, the Hero->X/Y variables are behind by a frame.
			x = Hero->X + Game->Scrolling[SCROLL_NRX];
			y = Hero->Y + Game->Scrolling[SCROLL_NRY];
			Screen->DrawCombo(3, x, y,
				20,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
			if (Viewport->X != -Game->Scrolling[SCROLL_ORX] || Viewport->Y != -Game->Scrolling[SCROLL_ORY])
			{
				printf("failed expectation! %d %d %d %d\n", Viewport->X, Game->Scrolling[SCROLL_NRX], Viewport->Y, Game->Scrolling[SCROLL_NRY]);
			}
			if (!startedInRegion)
			{
				if (Game->Scrolling[SCROLL_NX] != Game->Scrolling[SCROLL_NRX])
					printf("failed expectation! Game->Scrolling[SCROLL_NX]: %d Game->Scrolling[SCROLL_NRX]: %d\n", Game->Scrolling[SCROLL_NX], Game->Scrolling[SCROLL_NRX]);
				if (Game->Scrolling[SCROLL_NY] != Game->Scrolling[SCROLL_NRY])
					printf("failed expectation! Game->Scrolling[SCROLL_NY]: %d Game->Scrolling[SCROLL_NRY]: %d\n", Game->Scrolling[SCROLL_NY], Game->Scrolling[SCROLL_NRY]);
			}

			// This draws in the middle of the old screen.
			x = 256/2 + Game->Scrolling[SCROLL_OX];
			y = 176/2 + Game->Scrolling[SCROLL_OY];
			Screen->DrawCombo(3, x, y,
				21,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
			
			// This draws in the middle of the new screen.
			x = 256/2 + Game->Scrolling[SCROLL_NX];
			y = 176/2 + Game->Scrolling[SCROLL_NY];
			Screen->DrawCombo(3, x, y,
				44,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
		}
	}

	void freshNewDrawMode()
	{
		Screen->DrawOrigin = DRAW_ORIGIN_SCREEN;
		Screen->DrawString(7, 150, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "DrawOrigin", 128);

		Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;

		Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
		Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
		Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

		Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
		Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
		Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);

		int x = 0;
		int y = 0;
		int w = Viewport->Width - 1;
		int h = Viewport->Height - 9;
		Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, 0x72, 1, 0, 0, 0, false, 128);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// This draws where the hero is.
			// During scrolling, the hero coordinates are (unfortunately) relative to the new region.
			Screen->DrawOrigin = DRAW_ORIGIN_WORLD_SCROLLING_NEW;
			x = Hero->X;
			y = Hero->Y;
			Screen->DrawCombo(3, x, y,
				20,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			// This draws in the middle of the old screen.
			Screen->DrawOrigin = DRAW_ORIGIN_WORLD_SCROLLING_OLD;
			x = 256/2 + Game->Scrolling[SCROLL_OLD_SCREEN_X];
			y = 176/2 + Game->Scrolling[SCROLL_OLD_SCREEN_Y];
			Screen->DrawCombo(3, x, y,
				21,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
			
			// This draws in the middle of the new screen.
			Screen->DrawOrigin = DRAW_ORIGIN_WORLD_SCROLLING_NEW;
			x = 256/2 + Game->Scrolling[SCROLL_NEW_SCREEN_X];
			y = 176/2 + Game->Scrolling[SCROLL_NEW_SCREEN_Y];
			Screen->DrawCombo(3, x, y,
				44,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
		}
	}

    void run()
    {
		startedInRegion = Region->ID != 0;

        while (true)
		{
			// Alternate between what should be equivalent drawing calls.
			if (Game->Time % 2L == 0)
			{
				Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD; // old default
				jankyOldScrollingVars();
			}
			else
			{
				freshNewDrawMode();
			}

			Waitframe();
		}
	}
}
