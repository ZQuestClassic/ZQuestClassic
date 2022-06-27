#include <algif.h>

static volatile int ticks = 0;
static void ticker(void)
{
    ticks++;
}

int
main (int argc, char **argv)
{
    BITMAP **frames;
    int *durations;
    int i, n;
    char const *name = "example_gifs/allefant.gif";
    if (argc == 2)
        name = argv[1];
    allegro_init ();
    install_timer();
    set_color_depth (desktop_color_depth ());
    set_gfx_mode (GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
    clear_to_color (screen, makecol (255, 0, 255));
    install_keyboard ();
    algif_init ();
    n = algif_load_animation (name, &frames, &durations);
    if (!n)
    {
        allegro_message ("Could not open %s.\n", name);
        return -1;
    }
    install_int_ex(ticker, BPS_TO_TIMER(100));
    while (!keypressed ())
    {
        for (i = 0; i < n; i++)
        {
            int e;
            draw_sprite(screen, frames[i], 0, 0);
            e = ticks + durations[i];
            while (ticks < e && !keypressed ())
            {
                rest (1);
                textprintf_ex(screen, font, 0, frames[i]->h + 4,
                    makecol(255, 255, 255), makecol(255, 0, 255), "%-4d: %-4d",
                    i, e - ticks);
            }
            if (keypressed ())
                break;
        }
    }
    return 0;
}
END_OF_MAIN ()

