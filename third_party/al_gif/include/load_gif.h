#ifndef LOAD_GIF_H
#define LOAD_GIF_H

#include <allegro.h>

#ifdef __cplusplus
extern "C"
{
#endif

BITMAP *load_gif(const char *filename, RGB *pal);

#ifdef __cplusplus
}
#endif
#endif

