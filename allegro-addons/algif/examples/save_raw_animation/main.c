#include <algif.h>

static void
create_dispose_gif (void)
{
    /* Create a new gif animation. */
    GIF_ANIMATION *gif = algif_create_raw_animation (4);
    /* Fill in global data. */
    gif->width = 100;
    gif->height = 100;
    gif->loop = 0; /* loop forever */
    gif->palette.colors_count = 2;
    /* Make color 0 red. */
    gif->palette.colors[0].r = 255;
    gif->palette.colors[0].g = 0;
    gif->palette.colors[0].b = 0;
    /* Make color 1 blue. */
    gif->palette.colors[1].r = 0;
    gif->palette.colors[1].g = 0;
    gif->palette.colors[1].b = 255;
    /* Frame 0. */
    gif->frames[0].xoff = 20;
    gif->frames[0].yoff = 20;
    gif->frames[0].duration = 100; /* one second */
    gif->frames[0].disposal_method = 1; /* keep */
    gif->frames[0].bitmap_8_bit = create_bitmap (40, 40);
    gif->frames[0].transparent_index = -1;
    clear_to_color (gif->frames[0].bitmap_8_bit, 0);
    rectfill (gif->frames[0].bitmap_8_bit, 5, 5, 34, 34, 1);
    /* Frame 1. */
    gif->frames[1].xoff = 40;
    gif->frames[1].yoff = 20;
    gif->frames[1].duration = 100; /* one second */
    gif->frames[1].disposal_method = 3; /* previous */
    gif->frames[1].bitmap_8_bit = create_bitmap (40, 40);
    gif->frames[1].transparent_index = -1;
    clear_to_color (gif->frames[1].bitmap_8_bit, 0);
    rectfill (gif->frames[1].bitmap_8_bit, 5, 5, 34, 34, 1);
    /* Frame 2. */
    gif->frames[2].xoff = 20;
    gif->frames[2].yoff = 40;
    gif->frames[2].duration = 100; /* one second */
    gif->frames[2].disposal_method = 2; /* background */
    gif->frames[2].bitmap_8_bit = create_bitmap (40, 40);
    gif->frames[2].transparent_index = -1;
    clear_to_color (gif->frames[2].bitmap_8_bit, 0);
    rectfill (gif->frames[2].bitmap_8_bit, 5, 5, 34, 34, 1);
    /* Frame 3. */
    gif->frames[3].xoff = 40;
    gif->frames[3].yoff = 40;
    gif->frames[3].duration = 100; /* one second */
    gif->frames[3].disposal_method = 0;
    gif->frames[3].bitmap_8_bit = create_bitmap (40, 40);
    gif->frames[3].transparent_index = -1;
    clear_to_color (gif->frames[3].bitmap_8_bit, 0);
    rectfill (gif->frames[3].bitmap_8_bit, 5, 5, 34, 34, 1);
    /* Save it. */
    algif_save_raw_animation ("dispose.gif", gif);
    /* Clean up. */
    algif_destroy_raw_animation (gif);
}

static void
create_rgb_gif (void)
{
    int x, y, i;
    /* Create a new gif animation. */
    GIF_ANIMATION *gif = algif_create_raw_animation (3);
    /* Fill in global data. */
    gif->width = 32;
    gif->height = 32;
    gif->loop = -1; /* do not loop */
    for (i = 0; i < 3; i++)
    {
        gif->frames[i].palette.colors_count = 256;
        gif->frames[i].disposal_method = 1; /* keep */
        gif->frames[i].transparent_index = -1; /* no transparency. */
        gif->frames[i].bitmap_8_bit = create_bitmap (16, 16);
        gif->frames[i].xoff = (i & 1) * 16;
        gif->frames[i].yoff = (i / 2) * 16;
    }
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            int c = 16 * y + x;
            gif->frames[0].palette.colors[c].r = 255 - y * 255 / 15;
            gif->frames[0].palette.colors[c].g = 0;
            gif->frames[0].palette.colors[c].b = 255 - x * 255 / 15;
            gif->frames[1].palette.colors[c].r = 255 - y * 255 / 15;
            gif->frames[1].palette.colors[c].g = x * 255 / 15;
            gif->frames[1].palette.colors[c].b = 0;
            gif->frames[2].palette.colors[c].r = 0;
            gif->frames[2].palette.colors[c].g = y * 255 / 15;
            gif->frames[2].palette.colors[c].b = 255 - x * 255 / 15; 
            putpixel (gif->frames[0].bitmap_8_bit, x, y, c);
            putpixel (gif->frames[1].bitmap_8_bit, x, y, c);
            putpixel (gif->frames[2].bitmap_8_bit, x, y, c);
        }
    }

    /* Save it. */
    algif_save_raw_animation ("rgb.gif", gif);
    /* Clean up. */
    algif_destroy_raw_animation (gif);
}

int
main (void)
{
    allegro_init ();
    algif_init ();
    create_dispose_gif ();
    create_rgb_gif ();
    return 0;
}
END_OF_MAIN ()

