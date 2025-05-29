#include "std.zh"

#include "armos.zs"
#include "bumper.zs"
#include "circle.zs"
#include "credits.zs"
#include "cursor.zs"
#include "dmap_trace.zs"
#include "maths.zs"
#include "modify_input.zs"
#include "prime.zs"
#include "screen_script.zs"
#include "scrolling_debug.zs"

#include "auto/auto.zs"

ffc script EarlyExit
{
    void run(int id, int x)
    {
        printf("id: %d\n", id);
        if (x == 0)
            return;
        
        printf("x: %d\n", x);
    }
}

ffc script WinTheGame
{
    void run()
    {
        while (true)
        {
			if (Abs(this->X - Link->X) <= 5 && Abs(this->Y - Link->Y) <= 5)
			{
                printf("ending game\n");
				Game->End();
                printf("never print\n");
			}
            Waitframe();
        }
    }
}
