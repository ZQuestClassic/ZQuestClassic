import "std.zh"

int BLUE = 0x72;
int PINK = 0x74;

ffc script HelloWorld
{
    void run(int num)
    {
        printf("FFC script started, ID: %d x: %d y: %d\n", this->ID, this->X, this->Y);
        printf("num: %d\n", num);

        int i = 0;
        while(true)
        {
            Waitdraw();

            if (i++ == 30)
                printf("FFC script is running, ID: %d x: %d y: %d\n", this->ID, this->X, this->Y);

            // FFC scripts don't run during scrolling, so this blue outline is not drawn then.
            int x = this->X - 1;
            int y = this->Y - 1;
            int w = 18;
            int h = 18;

            // DRAW_ORIGIN_WORLD is the default in a scrolling region.
            // But swap between so we can visually confirm that.
            Screen->DrawOrigin = (Game->Time & 2L) == 0 ? DRAW_ORIGIN_WORLD : DRAW_ORIGIN_DEFAULT;
            Screen->Rectangle(6, x, y, x+w, y+h, BLUE, 1, 0, 0, 0, false, 128);

            Waitframe();
        }
    }
}

// The purpose of this is to ensure that the circle gets drawn in the same location,
// even during screen scrolling.
screendata script DotFFCs
{
    void run()
    {
        Screen->DrawOrigin = DRAW_ORIGIN_WORLD;

        while (true)
        {
            Waitdraw();

            // Draw a pink circle above every FFC.
            // During scrolling, the old FFCs are removed (they aren't carryovers), so this
            // dot won't render then.
            for (int i = 1; i < MAX_FFC; i++)
            {
                ffc f = Screen->LoadFFC(i);
                if (!f->Data) continue;

                int r = 5;
                int x = f->X + 8;
                int y = f->Y - r;

                Screen->Circle(6, x, y, r, PINK, 1, 0, 0, 0, true, 128);
            }

            Waitframe();
        }
    }
}

dmapdata script DotFFCs2
{
    void run()
    {
        Screen->DrawOrigin = DRAW_ORIGIN_WORLD;

        while (true)
        {
            Waitdraw();

            // Draw a pink circle below every FFC.
            // During scrolling, the old FFCs are removed (they aren't carryovers), so this
            // dot won't render then.
            for (int i = 1; i < MAX_FFC; i++)
            {
                ffc f = Screen->LoadFFC(i);
                if (!f->Data) continue;

                int r = 3;
                int x = f->X + 8;
                int y = f->Y + 18;

                Screen->Circle(6, x, y, r, PINK, 1, 0, 0, 0, true, 128);
            }

            Waitframe();
        }
    }
}
