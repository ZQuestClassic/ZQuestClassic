#include "std.zh"

ffc script TriggerOnCursor
{
	void run(int combo, int screen_index)
	{
		printf("Region->Width: %d\n", Region->Width);
		printf("Region->Height: %d\n", Region->Height);
		printf("Region->ScreenWidth: %d\n", Region->ScreenWidth);
		printf("Region->ScreenHeight: %d\n", Region->ScreenHeight);
		printf("Region->NumCombos: %d\n", Region->NumCombos);

		while (true)
		{
			int x = Input->Mouse[MOUSE_X];
			int y = Input->Mouse[MOUSE_Y];
			Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;
			Screen->DrawCombo(3, x, y,
				combo,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			x += Viewport->X;
			y += Viewport->Y;
			if (this->X < x && this->Y < y && this->X + 16 >= x && this->Y + 16 >= y)
			{
				Screen->TriggerSecrets(screen_index);
			}

			Waitframe();
		}
	}
}
