/*
 *      JPGalleg: JPEG image decoding routines for Allegro
 *
 *      version 1.1, by Angelo Mottola, May/June 2000
 *
 *      Include file
 */

#ifndef JPGALLEG_H
#define JPGALLEG_H

#include "zc_alleg.h"

/* load_memory_jpg:
 *  Decodes a JPG image from a block of memory data.
 */
BITMAP *load_memory_jpg(void *data, RGB *pal);

/* load_jpg:
 *  Decodes a JPG image from a standard JPG file.
 */
BITMAP *load_jpg(const char *filename, RGB *pal);
#endif
