#include "algif.h"
#include <string.h>

int
main (int argc, char **argv)
{
    char const *name = "example_gifs/allefant.gif";
    int x, y, c, i, th;
    if (argc == 2)
        name = argv[1];
    allegro_init ();
    set_color_depth (32);
    set_gfx_mode (GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);

    install_keyboard ();
    algif_init ();
    GIF_ANIMATION *gif = algif_load_raw_animation (name);
    if (!gif)
    {
        allegro_message ("Could not open %s.\n", name);
        return -1;
    }

    i = 0;
    while (1)
    {
        int k;
        char const *disposal_names[] =
        {
            "none", "keep", "background", "previous",
            "unknown", "unknown", "unknown", "unknown"
        };
        clear_to_color (screen, makecol (0, 0, 0));
        x = 0;
        y = 0;
        th = text_height (font);
        c = makecol (255, 255, 255);

        textprintf (screen, font, x, y, c, "Gif information for %s.", name);
        y += th;

        textprintf (screen, font, x, y, c, "size: %dx%d",
            gif->width, gif->height);
        y += th;

        textprintf (screen, font, x, y, c, gif->palette.colors_count ?
            "global palette: %d colors" : "no global palette",
            gif->palette.colors_count);
        y += th;

        textprintf (screen, font, x, y, c, "background index: %d",
            gif->background_index);
        y += th;

        textprintf (screen, font, x, y, c, "Frames: %d", gif->frames_count);
        y += th;

        rectfill (screen, x, y, x + gif->width + 1, y + gif->height + 1,
            makecol (255, 0, 255));
        algif_render_frame (gif, screen, i, x + 1, y + 1);
        rect (screen, x + gif->frames[i].xoff, y + gif->frames[i].yoff,
            x + 1 + gif->frames[i].xoff + gif->frames[i].bitmap_8_bit->w,
            y + 1 + gif->frames[i].yoff + gif->frames[i].bitmap_8_bit->h,
            makecol (60, 60, 255));
        y += gif->height + 2;

        textprintf (screen, font, x, y, c, "Frame #%i.", i);
        y += th;

        textprintf (screen, font, x, y, c, "offset: %d/%d",
            gif->frames[i].xoff, gif->frames[i].yoff);
        y += th;

        textprintf (screen, font, x, y, c, "size: %dx%d",
            gif->frames[i].bitmap_8_bit->w, gif->frames[i].bitmap_8_bit->h);
        y += th;

        textprintf (screen, font, x, y, c, gif->frames[i].palette.colors_count ?
            "local palette: %d colors" : "no local palette",
            gif->frames[i].palette.colors_count);
        y += th;

        textprintf (screen, font, x, y, c, "transparent index: %d (-1 for none)",
            gif->frames[i].transparent_index);
        y += th;

        textprintf (screen, font, x, y, c, "disposal method: %d (%s)",
            gif->frames[i].disposal_method,
            disposal_names[ gif->frames[i].disposal_method]);
        y += th;

        textprintf (screen, font, x, y, c, "duration: %d ms",
            gif->frames[i].duration * 10);
        y += th;

        y += th;

        textprintf (screen, font, x, y, c, "Press Esc to Exit, Cursor Left/Right to switch frames.");
        y += th;

        k = readkey () >> 8;
        if (k == KEY_ESC)
            break;
        if (k == KEY_LEFT)
            i--;
        if (k == KEY_RIGHT)
            i++;
        if (i < 0)
            i += gif->frames_count;
        if (i >= gif->frames_count)
            i -= gif->frames_count;
    }

    return 0;
}
END_OF_MAIN ()

