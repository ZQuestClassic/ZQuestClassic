#include "std.zh"

ffc script Cursor
{
    void run(int combo)
    {
        while (true)
        {
			int x = Input->Mouse[MOUSE_X];
			int y = Input->Mouse[MOUSE_Y];
			Screen->DrawCombo(3, x, y,
				combo,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

            Waitframe();
        }
    }
}
