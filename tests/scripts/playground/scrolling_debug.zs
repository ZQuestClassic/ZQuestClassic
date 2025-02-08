#include "std.zh"

dmapdata script ScrollingDebug
{
	int ORANGE = 0x0C;
	int BLUE = 0x72;
	int PINK = 0x74;
	bool startedInRegion;

	void drawSquare(int color, int x, int y)
	{
		int w = 6;
		int h = 6;
		Screen->Rectangle(6, x, y, x+w, y+h, color, 1, 0, 0, 0, true, 128);
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

		// Draw some text over the top of the screen (where the passive subscreen usually is).
		int subscreenY = 232 - Viewport->Height;
		Screen->DrawString(7, 150, -subscreenY, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "Janky", 128);

		// Draw a blue square around the edge of the playing field, using the Viewport.
		int x = 0;
		int y = 0;
		int w = Viewport->Width - 1;
		int h = Viewport->Height - 9;
		Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);

		// Draw some text over the top of the playing field.
		Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
		Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
		Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

		Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
		Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
		Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);

		// Draw pink square over the middle of the hero.
		drawSquare(PINK, engineRound(Hero->X) + 5 - Viewport->X, engineRound(Hero->Y) + 5 - Viewport->Y);

		// Draw blue square over a combo near the top-left of the region.
		drawSquare(BLUE, 5*16 - Viewport->X, 5*16 - Viewport->Y);

		// Draw an orange line across the top part of the region.
		x = 0 - Viewport->X;
		y = 32 - Viewport->Y;
		w = Region->Width - 1;
		h = 4;
		Screen->Rectangle(6, x, y, x+w, y+h, ORANGE, 1, 0, 0, 0, true, 128);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// Draw over where the hero is.
			x = engineRound(Hero->X) + Game->Scrolling[SCROLL_NX];
			y = engineRound(Hero->Y) + Game->Scrolling[SCROLL_NY];
			Screen->DrawCombo(3, x, y,
				20,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			// Draw in the middle of the old screen.
			x = 256/2 + Game->Scrolling[SCROLL_OX];
			y = 176/2 + Game->Scrolling[SCROLL_OY];
			Screen->DrawCombo(3, x, y,
				21,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
			
			// Draw in the middle of the new screen.
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
		// Draw some text over the top of the screen (where the passive subscreen usually is).
		Screen->DrawOrigin = DRAW_ORIGIN_SCREEN;
		Screen->DrawString(7, 150, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "DrawOrigin", 128);

		// Draw pink square over the middle of the hero.
		Screen->DrawOrigin = DRAW_ORIGIN_SPRITE;
		Screen->DrawOriginTarget = Hero;
		drawSquare(PINK, 5, 5);

		// Draw blue square over a combo near the top-left corner of the region.
		Screen->DrawOrigin = DRAW_ORIGIN_REGION;
		drawSquare(BLUE, 5*16, 5*16);

		// Draw an orange line across the top part of the region.
		int x = 0;
		int y = 32;
		int w = Region->Width - 1;
		int h = 4;
		Screen->Rectangle(6, x, y, x+w, y+h, ORANGE, 1, 0, 0, 0, true, 128);

		if (Game->Scrolling[SCROLL_DIR] != -1)
		{
			// Draw over where the hero is.
			Screen->DrawOrigin = DRAW_ORIGIN_SPRITE;
			Screen->DrawOriginTarget = Hero;
			x = 0;
			y = 0;
			Screen->DrawCombo(3, x, y,
				20,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			// Draw in the middle of the old screen.
			Screen->DrawOrigin = DRAW_ORIGIN_REGION_SCROLLING_OLD;
			x = 256/2 + Game->Scrolling[SCROLL_OLD_SCREEN_X];
			y = 176/2 + Game->Scrolling[SCROLL_OLD_SCREEN_Y];
			Screen->DrawCombo(3, x, y,
				21,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			// Draw in the middle of the new screen.
			// Either DrawOrigin works here.
			Screen->DrawOrigin = (Game->Time % 3L) == 0 ? DRAW_ORIGIN_REGION : DRAW_ORIGIN_REGION_SCROLLING_NEW;
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
			x = 0;
			y = 0;
			w = Viewport->Width - 1;
			h = Viewport->Height - 9;
			Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);
		}
		else
		{
			int x,y,w,h;
			Waitdraw(); // During scrolling, the viewport animates, requiring this Waitdraw().
			Screen->DrawOrigin = DRAW_ORIGIN_REGION;
			x = Viewport->X;
			y = Viewport->Y;
			w = Viewport->Width - 1;
			h = Viewport->Height - 9;
			Screen->Rectangle(7, x+4, y+4, x+w-4, y+h-4, BLUE, 1, 0, 0, 0, false, 128);
		}

		// Note: Game->Scrolling values are only accurate after Waitdraw.

		// Draw some text over the top of the playing field.
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
