#include "std.zh"

dmapdata script ScrollingDebug
{
	int BLUE = 0x72;
	int PINK = 0x74;
	bool startedInRegion;

	void drawPinkSquare(int x, int y)
	{
		int w = 6;
		int h = 6;
		Screen->Rectangle(6, x, y, x+w, y+h, PINK, 1, 0, 0, 0, true, 128);
	}

	// The engine does `zfix::getInt()` when drawing sprites - but this method does rounding
	// in a way not natively supported by zscript. In order to get the exact results in
	// [jankyOldScrollingVars], the rounding is reimplemented here.
	int engineRound(int val)
	{
		return (val<<0) + Clamp(Floor((val%1)/0.5), (val < 0 ? -1 : 0), (val<0 ? 0 : 1));
	}

	void jankyOldScrollingVars()
	{
		Waitdraw();

		int subscreenY = 232 - Viewport->Height;
		Screen->DrawString(7, 150, -subscreenY, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "Janky", 128);

		// Draw a blue square around the edge of the playing field, using the Viewport.
		int x = 0;
		int y = 0;
		int w = Viewport->Width - 1;
		int h = Viewport->Height - 9;
		Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);

		Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
		Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
		Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

		Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
		Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
		Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);

		// Draw pink square over the middle of the hero.
		drawPinkSquare(engineRound(Hero->X) + 5 - Viewport->X, engineRound(Hero->Y) + 5 - Viewport->Y);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// This draws where the hero is.
			x = engineRound(Hero->X) + Game->Scrolling[SCROLL_NRX];
			y = engineRound(Hero->Y) + Game->Scrolling[SCROLL_NRY];
			Screen->DrawCombo(3, x, y,
				20,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

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

	void freshNewDrawOrigin()
	{
		Screen->DrawOrigin = DRAW_ORIGIN_SCREEN;
		Screen->DrawString(7, 150, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "DrawOrigin", 128);

		// Draw pink square over the middle of the hero.
		Screen->DrawOrigin = DRAW_ORIGIN_SPRITE;
		Screen->DrawOriginTarget = Hero;
		drawPinkSquare(5, 5);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// This draws where the hero is.
			Screen->DrawOrigin = DRAW_ORIGIN_SPRITE;
			int x = 0;
			int y = 0;
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
			// Either DrawOrigin works here.
			Screen->DrawOrigin = (Game->Time % 3L) == 0 ? DRAW_ORIGIN_WORLD : DRAW_ORIGIN_WORLD_SCROLLING_NEW;
			x = 256/2 + Game->Scrolling[SCROLL_NEW_SCREEN_X];
			y = 176/2 + Game->Scrolling[SCROLL_NEW_SCREEN_Y];
			Screen->DrawCombo(3, x, y,
				44,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
		}

		// Draw a blue square around the edge of the playing field, using the Viewport.
		// These two branches should be equivalent.
		if ((Game->Time % 3L) == 0)
		{
			Waitdraw(); // During scrolling, the viewport height may animate, requiring this Waitdraw().
			Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;
			int x = 0;
			int y = 0;
			int w = Viewport->Width - 1;
			int h = Viewport->Height - 9;
			Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);
		}
		else
		{
			Waitdraw(); // During scrolling, the viewport animates, requiring this Waitdraw().
			Screen->DrawOrigin = DRAW_ORIGIN_WORLD;
			int x = Viewport->X;
			int y = Viewport->Y;
			int w = Viewport->Width - 1;
			int h = Viewport->Height - 9;
			Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);
		}

		// Game->Scrolling values are only accurate after Waitdraw.
		Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;

		Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
		Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
		Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

		Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
		Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
		Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);
	}

    void run()
    {
		startedInRegion = Region->ID != 0;

        while (true)
		{
			printf("OX/OY %d %d\n", Game->Scrolling[SCROLL_OX], Game->Scrolling[SCROLL_OY]);
			printf("NX/NY %d %d\n", Game->Scrolling[SCROLL_NX], Game->Scrolling[SCROLL_NY]);
			printf("ORX/ORY %d %d\n", Game->Scrolling[SCROLL_ORX], Game->Scrolling[SCROLL_ORY]);
			printf("NRX/NRY %d %d\n", Game->Scrolling[SCROLL_NRX], Game->Scrolling[SCROLL_NRY]);

			if (Game->Scrolling[SCROLL_DIR] != -1)
			{
				if (Viewport->X != -Game->Scrolling[SCROLL_NRX] || Viewport->Y != -Game->Scrolling[SCROLL_NRY])
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
			}

			// Alternate between what should be equivalent drawing calls.
			if (Game->Time % 2L == 0)
			{
				Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD; // old default
				jankyOldScrollingVars();
			}
			else
			{
				freshNewDrawOrigin();
			}

			Waitframe();
		}
	}
}
