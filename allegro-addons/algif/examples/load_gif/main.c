#include <algif.h>

int
main (int argc, char **argv)
{
    char const *name = "example_gifs/alex.gif";
    if (argc == 2)
        name = argv[1];
    allegro_init ();
    set_color_depth (desktop_color_depth ());
    set_gfx_mode (GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
    clear_to_color (screen, makecol (255, 0, 255));
    install_keyboard ();
    algif_init ();
    BITMAP *bmp = load_bitmap (name, NULL);
    if (bmp)
    {
        draw_sprite(screen, bmp, 0, 0);
        readkey ();
    }
    else
    {
        allegro_message ("Could not open %s.\n", name);
        return 1;
    }
    return 0;
}
END_OF_MAIN ()

