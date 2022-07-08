// Not used at the moment. Would be preferred to use a5's bmp directly but there are bugs
// such as bitmap being rotated when grabbing.

#include "albmp.h"
#include <allegro.h>
#include <a5alleg.h>

BITMAP *load_bmp(AL_CONST char *filename, RGB *pal)
{
    ALLEGRO_BITMAP *a5bmp = NULL;
    BITMAP *bmp = NULL;
    ALLEGRO_COLOR color;
    unsigned char r, g, b;
    int i, j;

    a5bmp = al_load_bitmap(filename);
    if (!a5bmp)
    {
        goto fail;
    }
    bmp = create_bitmap(al_get_bitmap_width(a5bmp), al_get_bitmap_height(a5bmp));
    if (!bmp)
    {
        goto fail;
    }

    int format = al_get_bitmap_format(a5bmp);

    al_lock_bitmap(a5bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
    for (i = 0; i < al_get_bitmap_height(a5bmp); i++)
    {
        for (j = 0; j < al_get_bitmap_width(a5bmp); j++)
        {
            color = al_get_pixel(a5bmp, j, i);
            al_unmap_rgb(color, &r, &g, &b);
            putpixel(bmp, j, i, makecol(r, g, b));
        }
    }
    al_unlock_bitmap(a5bmp);
    al_destroy_bitmap(a5bmp);

    generate_332_palette(pal);

    return bmp;

fail:
{
    if (bmp)
    {
        destroy_bitmap(bmp);
    }
    if (a5bmp)
    {
        al_destroy_bitmap(a5bmp);
    }
}
    return NULL;
}

int save_bmp(AL_CONST char *filename, BITMAP *bmp, AL_CONST PALETTE pal)
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
            al_put_pixel(j, i, al_map_rgb(getr(c), getg(c), getb(c)));
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

void albmp_init()
{
    register_bitmap_file_type("bmp", load_bmp, save_bmp);
}
