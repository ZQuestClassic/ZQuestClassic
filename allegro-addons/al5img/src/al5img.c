#include "al5img.h"
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <a5alleg.h>
#include <load_gif.h>
#include <save_gif.h>

BITMAP *al5_bitmap_to_al4_bitmap(ALLEGRO_BITMAP *a5bmp, RGB *pal)
{
    BITMAP *bmp = NULL;
    ALLEGRO_COLOR color;
    unsigned char r, g, b;
    int i, j, k;
    ALLEGRO_COLOR a5pal[256];

    if (!a5bmp)
    {
        goto fail;
    }

    int ncolors = al_get_bitmap_palette(a5bmp, NULL);
    int bpp = al_get_bitmap_bit_count(a5bmp);
    if (ncolors > 0)
    {
        // This case is only hit when allegro 4 bmp loading code fails. See load_bmp_al5.

        al_get_bitmap_palette(a5bmp, &a5pal[0]);

        bmp = create_bitmap_ex(bpp, al_get_bitmap_width(a5bmp), al_get_bitmap_height(a5bmp));
        if (!bmp)
        {
            goto fail;
        }

        for (int i = 0; i < ncolors; i++)
        {
            pal[i].r = a5pal[i].r * 63.0f;
            pal[i].g = a5pal[i].g * 63.0f;
            pal[i].b = a5pal[i].b * 63.0f;
            // no alpha in allegro 4.
        }

        al_lock_bitmap(a5bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
        for (i = 0; i < al_get_bitmap_height(a5bmp); i++)
        {
            for (j = 0; j < al_get_bitmap_width(a5bmp); j++)
            {
                color = al_get_pixel(a5bmp, j, i);
                r = color.r * 255;
                g = color.g * 255;
                b = color.b * 255;
                putpixel(bmp, j, i, bestfit_color(pal, r>>2, g>>2, b>>2));
            }
        }
        al_unlock_bitmap(a5bmp);

        if (bpp != 8) bmp = _fixup_loaded_bitmap(bmp, pal, 8);
    }
    else
    {
        // No palette provided, so let's create our own by just writing as truecolor and
        // deferring to _fixup_loaded_bitmap.
        bmp = create_bitmap_ex(24, al_get_bitmap_width(a5bmp), al_get_bitmap_height(a5bmp));
        if (!bmp)
        {
            goto fail;
        }

        al_lock_bitmap(a5bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
        for (i = 0; i < al_get_bitmap_height(a5bmp); i++)
        {
            for (j = 0; j < al_get_bitmap_width(a5bmp); j++)
            {
                color = al_get_pixel(a5bmp, j, i);
                putpixel(bmp, j, i, makecol24(color.r*255, color.g*255, color.b*255));
            }
        }
        al_unlock_bitmap(a5bmp);

        bmp = _fixup_loaded_bitmap(bmp, pal, 8);
    }

    return bmp;

fail:
{
    if (bmp)
    {
        destroy_bitmap(bmp);
    }
}
    return NULL;
}

BITMAP *load_al4_bitmap_through_al5(AL_CONST char *filename, RGB *pal)
{
    ALLEGRO_BITMAP *a5bmp = al_load_bitmap_flags(filename, ALLEGRO_KEEP_PALETTE);
    BITMAP* bmp = al5_bitmap_to_al4_bitmap(a5bmp, pal);
    al_destroy_bitmap(a5bmp);
    return bmp;
}

int save_al4_bitmap_through_al5(AL_CONST char *filename, BITMAP *bmp, AL_CONST PALETTE pal)
{
    ALLEGRO_BITMAP *a5bmp;
    int i, j, c;

    a5bmp = al_create_bitmap(bmp->w, bmp->h);
    if (!a5bmp)
    {
        goto fail;
    }
    al_lock_bitmap(a5bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    al_set_target_bitmap(a5bmp);
    for (i = 0; i < bmp->h; i++)
    {
        for (j = 0; j < bmp->w; j++)
        {
            c = getpixel(bmp, j, i);
            al_put_pixel(j, i, al_map_rgb(pal[c].r * 4, pal[c].g * 4, pal[c].b * 4));
        }
    }
    al_unlock_bitmap(a5bmp);
    al_save_bitmap(filename, a5bmp);
    al_destroy_bitmap(a5bmp);
    al_set_target_bitmap(NULL);
    return 1;

fail:
{
    if (a5bmp)
    {
        al_destroy_bitmap(a5bmp);
    }
    al_set_target_bitmap(NULL);
}
    return 0;
}

BITMAP *load_bmp_al5(AL_CONST char *filename, RGB *pal)
{
    // First try allegro 4 bmp loading. It can't handle the newest bitmap formats, so on failure
    // fallback to our bmp loading code. It tries to do all the same palette shennanigans that
    // allegro 4 does.
    BITMAP *bmp = load_bmp(filename, pal);
    if (bmp) return bmp;
    return load_al4_bitmap_through_al5(filename, pal);
}

void al5img_init()
{
    register_bitmap_file_type("jpg", load_al4_bitmap_through_al5, save_al4_bitmap_through_al5);
    register_bitmap_file_type("jpeg", load_al4_bitmap_through_al5, save_al4_bitmap_through_al5);
    register_bitmap_file_type("bmp", load_bmp_al5, save_bmp);
    register_bitmap_file_type("gif",  load_gif, save_gif);
}
