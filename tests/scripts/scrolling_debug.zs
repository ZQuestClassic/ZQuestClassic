#include "std.zh"

dmapdata script ScrollingDebug
{
    void run()
    {
        while (true)
		{
			printf("OX/OY %d %d\n", Game->Scrolling[SCROLL_OX], Game->Scrolling[SCROLL_OY]);
			Screen->DrawString(7, 0, 0, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "OX/OY", 128);
			Screen->DrawInteger(7, 40, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OX], 0, 128);
			Screen->DrawInteger(7, 120, 0, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_OY], 0, 128);

			printf("NX/NY %d %d\n", Game->Scrolling[SCROLL_NX], Game->Scrolling[SCROLL_NY]);
			Screen->DrawString(7, 0, 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, "NX/NY", 128);
			Screen->DrawInteger(7, 40, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NX], 0, 128);
			Screen->DrawInteger(7, 120, 8, FONT_Z3SMALL, 0x01, 0x0F, -1, -1, Game->Scrolling[SCROLL_NY], 0, 128);

			if (Region->ID)
			{
				printf("ORX/ORY %d %d\n", Game->Scrolling[SCROLL_ORX], Game->Scrolling[SCROLL_ORY]);
				printf("NRX/NRY %d %d\n", Game->Scrolling[SCROLL_NRX], Game->Scrolling[SCROLL_NRY]);
			}

			if (Game->Scrolling[SCROLL_DIR] != -1)
			{
				// This draws where link is.
				int x = Link->X - Viewport->X;
				int y = Link->Y - Viewport->Y;
				Screen->DrawCombo(3, x, y,
					20,
					1,
					1,
					3,
					-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);
				if (Viewport->X != -Game->Scrolling[SCROLL_NRX] || Viewport->Y != -Game->Scrolling[SCROLL_NRY])
				{
					printf("failed expectation! %d %d %d %d\n", Viewport->X, Game->Scrolling[SCROLL_NRX], Viewport->Y, Game->Scrolling[SCROLL_NRY]);
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

			Waitframe();
		}
    }
}
