#ifndef SAVE_GIF_H
#define SAVE_GIF_H

#include <allegro.h>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t save_gif(const char *filename, BITMAP *bmp, const RGB *pal);

#ifdef __cplusplus
}
#endif
#endif

