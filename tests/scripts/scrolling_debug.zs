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

			Waitframe();
		}
    }
}
