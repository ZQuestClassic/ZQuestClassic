#include "algif.h"
#include "lzw.h"

#include <stdlib.h>
#include <string.h>

GIF_ANIMATION *
algif_create_raw_animation (int frames_count)
{
    GIF_ANIMATION *gif = calloc (1, sizeof *gif);
    /* Create frames. */
    gif->frames_count = frames_count;
    gif->frames = calloc (gif->frames_count, sizeof *gif->frames);
    return gif;
}

/* Destroy a complete gif, including all frames. */
void
algif_destroy_raw_animation (GIF_ANIMATION *gif)
{
    int i;

    for (i = 0; i < gif->frames_count; i++)
    {
        GIF_FRAME *frame = gif->frames + i;

        if (frame->bitmap_8_bit)
            destroy_bitmap (frame->bitmap_8_bit);
    }
    if (gif->store)
        destroy_bitmap (gif->store);
    free (gif->frames);
    free (gif);
}

static void
write_palette (PACKFILE *file, GIF_PALETTE *palette, int bits)
{
    int i;
    for (i = 0; i < (1 << bits); i++)
    {
        pack_putc (palette->colors[i].r, file);
        pack_putc (palette->colors[i].g, file);
        pack_putc (palette->colors[i].b, file);
    }
}

static void
read_palette (PACKFILE * file, GIF_PALETTE *palette)
{
    int i;

    for (i = 0; i < palette->colors_count; i++)
    {
        palette->colors[i].r = pack_getc (file);
        palette->colors[i].g = pack_getc (file);
        palette->colors[i].b = pack_getc (file);
    }
}

/*
 * Compresses all the frames in the animation and writes to a gif file.
 * Nothing else like extensions or comments is written.
 *
 * Returns 0 on success.
 *
 * Note: All bitmaps must have a color depth of 8.
 */
int
algif_save_raw_animation (const char *filename, GIF_ANIMATION *gif)
{
    int frame;
    int i, j;
    PACKFILE *file;

    file = pack_fopen (filename, "w");
    if (!file)
        return -1;

    pack_fwrite ("GIF89a", 6, file);
    pack_iputw (gif->width, file);
    pack_iputw (gif->height, file);
    /* 7 global palette
     * 456 color richness
     * 3 sorted
     * 012 palette bits
     */
    for (i = 1, j = 0; i < gif->palette.colors_count; i *= 2, j++);
    pack_putc ((j ? 128 : 0) + 64 + 32 + 16 + (j ? j - 1 : 0), file);
    pack_putc (gif->background_index, file);
    pack_putc (0, file);        /* No aspect ratio. */

    if (j)
        write_palette (file, &gif->palette, j);

    if (gif->loop != -1)
    /* Loop count extension. */
    {
        pack_putc (0x21, file); /* Extension Introducer. */
        pack_putc (0xff, file); /* Application Extension. */
        pack_putc (11, file);    /* Size. */
        pack_fwrite ("NETSCAPE2.0", 11, file);
        pack_putc (3, file); /* Size. */
        pack_putc (1, file);
        pack_iputw (gif->loop, file);
        pack_putc (0, file);
    }

    for (frame = 0; frame < gif->frames_count; frame++)
    {
        int w = gif->frames[frame].bitmap_8_bit->w;
        int h = gif->frames[frame].bitmap_8_bit->h;

        pack_putc (0x21, file); /* Extension Introducer. */
        pack_putc (0xf9, file); /* Graphic Control Extension. */
        pack_putc (4, file);    /* Size. */
        /* Disposal method, and enable transparency. */
        i = gif->frames[frame].disposal_method << 2;
        if (gif->frames[frame].transparent_index != -1)
            i |= 1;
        pack_putc (i, file);
        pack_iputw (gif->frames[frame].duration, file); /* In 1/100th seconds. */
        if (gif->frames[frame].transparent_index != -1)
            pack_putc (gif->frames[frame].transparent_index, file);       /* Transparent color index. */
        else
             pack_putc (0, file);
        pack_putc (0x00, file); /* Terminator. */

        pack_putc (0x2c, file); /* Image Descriptor. */
        pack_iputw (gif->frames[frame].xoff, file);
        pack_iputw (gif->frames[frame].yoff, file);
        pack_iputw (w, file);
        pack_iputw (h, file);

         /* 7: local palette
         * 6: interlaced
         * 5: sorted
         * 012: palette bits
         */

        for (i = 1, j = 0; i < gif->frames[frame].palette.colors_count; i *= 2, j++);
        pack_putc ((j ? 128 : 0) + (j ? j - 1 : 0), file);

        if (j)
            write_palette (file, &gif->frames[frame].palette, j);

        LZW_encode (file, gif->frames[frame].bitmap_8_bit);

        pack_putc (0x00, file); /* Terminator. */
    }

    pack_putc (0x3b, file);     /* Trailer. */

    pack_fclose (file);
    return 0;
}

static void
deinterlace (BITMAP *bmp)
{
    BITMAP *n = create_bitmap_ex (bitmap_color_depth (bmp), bmp->w, bmp->h);
    int y, i = 0;
    for (y = 0; y < n->h; y += 8)
    {
        blit (bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (y = 4; y < n->h; y += 8)
    {
        blit (bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (y = 2; y < n->h; y += 4)
    {
        blit (bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (y = 1; y < n->h; y += 2)
    {
        blit (bmp, n, 0, i++, 0, y, n->w, 1);
    }
    blit (n, bmp, 0, 0, 0, 0, n->w, n->h);
    destroy_bitmap (n);
}

static GIF_ANIMATION *
load_object (PACKFILE * file, long size)
{
    int version;
    BITMAP *bmp = NULL;
    int i, j;
    GIF_ANIMATION *gif = calloc (1, sizeof *gif);
    GIF_FRAME frame;
    int have_global_palette = 0;

    (void) size;

    gif->frames_count = 0;

    /* is it really a GIF? */
    if (pack_getc (file) != 'G')
        goto error;
    if (pack_getc (file) != 'I')
        goto error;
    if (pack_getc (file) != 'F')
        goto error;
    if (pack_getc (file) != '8')
        goto error;
    /* '7' or '9', for 87a or 89a. */
    version = pack_getc (file);
    if (version != '7' && version != '9')
        goto error;
    if (pack_getc (file) != 'a')
        goto error;

    gif->width = pack_igetw (file);
    gif->height = pack_igetw (file);
    i = pack_getc (file);
    /* Global color table? */
    if (i & 128)
        gif->palette.colors_count = 1 << ((i & 7) + 1);
    else
        gif->palette.colors_count = 0;
    /* Background color is only valid with a global palette. */
    gif->background_index = pack_getc (file);

    /* Skip aspect ratio. */
    pack_fseek (file, 1);

    if (gif->palette.colors_count)
    {
        read_palette (file, &gif->palette);
        have_global_palette = 1;
    }

    memset(&frame, 0, sizeof frame); /* For first frame. */
    frame.transparent_index = -1;

    do
    {
        i = pack_getc (file);

        switch (i)
        {
            case 0x2c:         /* Image Descriptor */
            {
                int w, h;
                int interlaced = 0;

                frame.xoff = pack_igetw (file);
                frame.yoff = pack_igetw (file);
                w = pack_igetw (file);
                h = pack_igetw (file);
                bmp = create_bitmap_ex (8, w, h);
                if (!bmp)
                    goto error;
                i = pack_getc (file);

                /* Local palette. */
                if (i & 128)
                {
                    frame.palette.colors_count = 1 << ((i & 7) + 1);
                    read_palette (file, &frame.palette);
                }
                else
                {
                    frame.palette.colors_count = 0;
                }

                if (i & 64)
                    interlaced = 1;

                if (LZW_decode (file, bmp))
                    goto error;

                if (interlaced)
                    deinterlace (bmp);

                frame.bitmap_8_bit = bmp;
                bmp = NULL;

                gif->frames_count++;
                gif->frames =
                    realloc (gif->frames,
                             gif->frames_count * sizeof *gif->frames);
                gif->frames[gif->frames_count - 1] = frame;

                memset(&frame, 0, sizeof frame); /* For next frame. */
                frame.transparent_index = -1;

                break;
            }
            case 0x21: /* Extension Introducer. */
                j = pack_getc (file); /* Extension Type. */
                i = pack_getc (file); /* Size. */
                if (j == 0xf9) /* Graphic Control Extension. */
                {
                    /* size must be 4 */
                    if (i != 4)
                        goto error;
                    i = pack_getc (file);
                    frame.disposal_method = (i >> 2) & 7;
                    frame.duration = pack_igetw (file);
                    if (i & 1)  /* Transparency? */
                    {
                        frame.transparent_index = pack_getc (file);
                    }
                    else
                    {
                        pack_fseek (file, 1); 
                        frame.transparent_index = -1;
                    }
                    i = pack_getc (file); /* Size. */
                }
                /* Application Extension. */
                else if (j == 0xff)
                {
                    if (i == 11)
                    {
                        char name[12];
                        pack_fread (name, 11, file);
                        i = pack_getc (file); /* Size. */
                        name[11] = '\0';
                        if (!strcmp (name, "NETSCAPE2.0"))
                        {
                            if (i == 3)
                            {
                                j = pack_getc (file);
                                gif->loop = pack_igetw (file);
                                if (j != 1)
                                    gif->loop = 0;
                                i = pack_getc (file); /* Size. */
                            }
                        }
                    }
                }

                /* Possibly more blocks until terminator block (0). */
                while (i)
                {
                    pack_fseek (file, i);
                    i = pack_getc (file);
                }
                break;
            case 0x3b:
                /* GIF Trailer. */
                pack_fclose (file);
                return gif;
        }
    }
    while (TRUE);
  error:
    if (file)
        pack_fclose (file);
    if (gif)
        algif_destroy_raw_animation (gif);
    if (bmp)
        destroy_bitmap (bmp);
    return NULL;
}

/*
 * Allocates and reads a GIF_ANIMATION structure, filling in all the
 * frames found in the file. On error, nothing is allocated, and NULL is
 * returned. No extensions or comments are read in. If the gif contains
 * a transparency index, and it is no 0, it is swapped with 0 - so index
 * 0 will be the transparent color. There is no way to know when a file
 * contained no transparency originally. Frame duration is specified in
 * 1/100th seconds.
 *
 * All bitmaps will have a color depth of 8.
 */
GIF_ANIMATION *
algif_load_raw_animation (const char *filename)
{
    PACKFILE *file;
    GIF_ANIMATION *gif = NULL;

    file = pack_fopen (filename, "r");
    if (file)
        gif = load_object (file, 0);
    return gif;
}

static int
get_rgbcolor (RGB *rgb)
{
    return makecol (rgb->r, rgb->g, rgb->b);
}

/* Renders the next frame in a GIF animation to the specified bitmap and
 * the given position. You need to call this in order on the same
 * destination for frames [0..gif->frames_count - 1] to properly render all
 * the frames in the GIF.
 */
void
algif_render_frame (GIF_ANIMATION *gif, BITMAP *bitmap, int frame, int xpos,
                    int ypos)
{
    int x, y, w, h;
    GIF_FRAME *f = &gif->frames[frame];
    GIF_PALETTE *pal;
    if (frame == 0)
        rectfill (bitmap, xpos, ypos, xpos + gif->width - 1,
              ypos + gif->height - 1, bitmap_mask_color (bitmap));
    else
    {
        GIF_FRAME *p = &gif->frames[frame - 1];
        if (p->disposal_method == 2)
            rectfill (bitmap, xpos + p->xoff, ypos + p->yoff,
                xpos + p->xoff + p->bitmap_8_bit->w - 1,
                ypos + p->yoff + p->bitmap_8_bit->h - 1,
                bitmap_mask_color (bitmap));
        else if (p->disposal_method == 3 && gif->store)
        {
            blit (gif->store, bitmap, 0, 0, xpos + p->xoff, ypos + p->yoff,
                gif->store->w, gif->store->h);
            destroy_bitmap (gif->store);
            gif->store = NULL;
        }
    }
    w = f->bitmap_8_bit->w;
    h = f->bitmap_8_bit->h;
    if (f->disposal_method == 3)
    {
        if (gif->store)
            destroy_bitmap (gif->store);
        gif->store = create_bitmap_ex (bitmap_color_depth (bitmap), w, h);
        blit (bitmap, gif->store, xpos + f->xoff, ypos + f->yoff, 0, 0, w, h);
    }
    pal = &gif->frames[frame].palette;
    if (pal->colors_count == 0)
        pal = &gif->palette;

    //int i;
    //for (i = 0; i < pal->colors_count; i++)
    //    printf("%d: %d %d %d\n", i, pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            int c = getpixel (f->bitmap_8_bit, x, y);
            if (c != f->transparent_index)
            {
                putpixel (bitmap, xpos + f->xoff + x, ypos + f->yoff + y,
                          get_rgbcolor (&pal->colors[c]));
            }
        }
    }
}

/* Registers gif as bitmap and datafile type. */
void
algif_init (void)
{
    register_bitmap_file_type ("gif", load_gif, save_gif);
    register_datafile_object (DAT_GIF,
                              (void *(*)(PACKFILE *, long)) load_object,
                              (void (*)(void *))
                              algif_destroy_raw_animation);
}
