#include "algif.h"

/* This will load a gif animation into an array of BITMAP pointers. Each
 * bitmap will have the size of the complete animation. The bitmaps will all
 * use Allegro's current color depth. Returns the number of stored frames,
 * 0 on error. You are responsible for freeing all the bitmaps as well as
 * the arrays yourself.
 *
 * E.g.
 * BITMAP **frames = NULL;
 * int **durations = NULL;
 * int n = algif_load_animation ("my.gif", &frames, &durations);
 * if (n)
 * {
 *     ...
 *     for (i = 0; i < n; i++)
 *         destroy_bitmap (frames[i]);
 *     free (frames);
 *     free (durations);
 * }
 */
int
algif_load_animation (const char *filename, BITMAP ***frames, int **durations)
{
    GIF_ANIMATION *gif = algif_load_raw_animation (filename);
    int i, n;
    BITMAP *prev = NULL;

    if (!gif)
        return 0;

    n = gif->frames_count;
    *frames = calloc (n, sizeof **frames);
    if (durations)
        *durations = calloc (n, sizeof **durations);
    for (i = 0; i < n; i++)
    {
        BITMAP *b = (*frames)[i] = create_bitmap (gif->width, gif->height);
        if (prev)
            blit (prev, b, 0, 0, 0, 0, b->w, b->h);
        algif_render_frame (gif, b, i, 0, 0);
        if (durations)
            (*durations)[i] = gif->frames[i].duration;
        prev = b;
    }
    algif_destroy_raw_animation (gif);
    return n;
}

/* Allegrified version. Puts all frames into a single bitmap,
 * with the current color depth. */
BITMAP *
load_gif (AL_CONST char *filename, RGB *pal)
{
    int i;
    GIF_ANIMATION *gif = algif_load_raw_animation (filename);
    BITMAP *bmp = NULL;
    GIF_PALETTE gifpal;
    PALETTE tmppal;

    if (!gif || gif->frames_count == 0)
        return NULL;

    /* Either use the global palette, or the palette of the first frame. */
    gifpal = gif->palette;
    if (gifpal.colors_count == 0)
    {
        gifpal = gif->frames[0].palette;
    }

    if (!pal)
        pal = tmppal;

    for (i = 0; i < gifpal.colors_count; i++)
    {
        pal[i].r = gifpal.colors[i].r / 4;
        pal[i].g = gifpal.colors[i].g / 4;
        pal[i].b = gifpal.colors[i].b / 4;
    }

    for ( ; i < PAL_SIZE; i++) {
        pal[i].r = 0;
        pal[i].g = 0;
        pal[i].b = 0;
    }

    if (gif)
    {
        bmp = create_bitmap (gif->width, gif->height);

        select_palette(pal);

        for (i = 0; i < gif->frames_count; i++)
        {
            algif_render_frame (gif, bmp, i, 0, 0);
        }

        unselect_palette();

        algif_destroy_raw_animation (gif);
    }

    return bmp;
}

/* Allegrified version. Saves only a single bitmap. */
int
save_gif (AL_CONST char *filename, BITMAP *bmp, AL_CONST PALETTE pal)
{
    GIF_ANIMATION gif;
    GIF_FRAME frame;
    int ret, i;
    PALETTE cp;

    gif.width = bmp->w;
    gif.height = bmp->h;
    gif.frames_count = 1;
    gif.background_index = 0;
    gif.loop = -1;
    gif.palette.colors_count = 0;
    
    gif.frames = &frame;
    frame.bitmap_8_bit = create_bitmap_ex (8, bmp->w, bmp->h);
    frame.palette.colors_count = 0;
    frame.xoff = 0;
    frame.yoff = 0;
    frame.duration = 0;
    frame.disposal_method = 0;
    frame.transparent_index = -1;

    blit (bmp, frame.bitmap_8_bit, 0, 0, 0, 0, bmp->w, bmp->h);

    if (!pal)
    {
        get_palette (cp);
        pal = cp;
    }

    frame.palette.colors_count = 256;
    for (i = 0; i < 256; i++)
    {
        frame.palette.colors[i].r = _rgb_scale_6[pal[i].r];
        frame.palette.colors[i].g = _rgb_scale_6[pal[i].g];
        frame.palette.colors[i].b = _rgb_scale_6[pal[i].b];
    }

    ret = algif_save_raw_animation (filename, &gif);
    destroy_bitmap (frame.bitmap_8_bit);
    return ret;
}

