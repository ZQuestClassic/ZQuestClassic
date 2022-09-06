#include "al5img.h"
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <a5alleg.h>
#include <algif.h>

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
    if (ncolors > 0)
    {
        al_get_bitmap_palette(a5bmp, &a5pal[0]);
    }

    if (ncolors > 0)
    {
        bmp = create_bitmap_ex(8, al_get_bitmap_width(a5bmp), al_get_bitmap_height(a5bmp));
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
                // TODO: could do a more direct memory access instead.
                color = al_get_pixel(a5bmp, j, i);
                putpixel(bmp, j, i, color.r * 255.0f);
            }
        }
        al_unlock_bitmap(a5bmp);
    }
    else
    {
        bmp = create_bitmap_ex(8, al_get_bitmap_width(a5bmp), al_get_bitmap_height(a5bmp));
        if (!bmp)
        {
            goto fail;
        }

        int cur_pal_index = 0;
        bool truecolor_fallback = false;

        al_lock_bitmap(a5bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
        for (i = 0; i < al_get_bitmap_height(a5bmp); i++)
        {
            for (j = 0; j < al_get_bitmap_width(a5bmp); j++)
            {
                color = al_get_pixel(a5bmp, j, i);
                al_unmap_rgb(color, &r, &g, &b);

                bool found_existing_color = false;
                r /= 4;
                g /= 4;
                b /= 4;
                for (k = 0; k < cur_pal_index; k++)
                {
                    if (pal[k].r == r && pal[k].g == g && pal[k].b == b)
                    {
                        found_existing_color = true;
                        break;
                    }
                }
                if (!found_existing_color)
                {
                    if (cur_pal_index == 256)
                    {
                        truecolor_fallback = true;
                        break;
                    }

                    pal[cur_pal_index].r = r;
                    pal[cur_pal_index].g = g;
                    pal[cur_pal_index].b = b;
                    cur_pal_index += 1;
                    k = cur_pal_index;
                }

                putpixel(bmp, j, i, k);
            }
        }

        if (truecolor_fallback)
        {
            for (i = 0; i < al_get_bitmap_height(a5bmp); i++)
            {
                for (j = 0; j < al_get_bitmap_width(a5bmp); j++)
                {
                    color = al_get_pixel(a5bmp, j, i);
                    al_unmap_rgb(color, &r, &g, &b);
                    putpixel(bmp, j, i, makecol(r, g, b));
                }
            }

            generate_332_palette(pal);
        }

        al_unlock_bitmap(a5bmp);
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
    ALLEGRO_BITMAP *a5bmp = al_load_bitmap_flags(filename, ALLEGRO_KEEP_PALETTE | ALLEGRO_KEEP_INDEX);
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
    // allegro 4 bitmap loading code has a lot of stuff for converting palettes
    // of non-8bpp bitmaps that we want to utilize. But some of the newest bitmap
    // formats only work in allegro 5, so fallback to that if needed.
    BITMAP *bmp = load_bmp(filename, pal);
    // TODO: this doesn't actually work very well yet, so I'd rather fail to load new bitmaps
    // than load with wrong colors.
    // if (!bmp) bmp = load_al4_bitmap_through_al5(filename, pal);
    return bmp;
}

BITMAP *load_gif(AL_CONST char *filename, RGB *pal)
{
    ALGIF_ANIMATION *gif = algif_load_animation(filename);
    ALLEGRO_BITMAP *a5bmp = algif_get_bitmap(gif, 0);
    BITMAP *bmp = al5_bitmap_to_al4_bitmap(a5bmp, pal);
    algif_destroy_animation(gif);
    return bmp;
}

void al5img_init()
{
    register_bitmap_file_type("jpg", load_al4_bitmap_through_al5, save_al4_bitmap_through_al5);
    register_bitmap_file_type("jpeg", load_al4_bitmap_through_al5, save_al4_bitmap_through_al5);
    register_bitmap_file_type("bmp", load_bmp_al5, save_bmp);
    register_bitmap_file_type("png", load_al4_bitmap_through_al5, save_al4_bitmap_through_al5);
    register_bitmap_file_type("gif", load_gif, save_al4_bitmap_through_al5);
}
