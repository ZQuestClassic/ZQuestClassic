#ifndef _GIF_H_
#define _GIF_H_

#include <allegro.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DAT_GIF  DAT_ID('G','I','F',' ')

typedef struct GIF_ANIMATION GIF_ANIMATION;
typedef struct GIF_FRAME GIF_FRAME;
typedef struct GIF_PALETTE GIF_PALETTE;

struct GIF_PALETTE
{
    int colors_count;
    RGB colors[256];
};

struct GIF_ANIMATION
{
    int width, height;
    int frames_count;
    int background_index;
    int loop; /* -1 = no, 0 = forever, 1..65535 = that many times */
    GIF_PALETTE palette;
    GIF_FRAME *frames;

    BITMAP *store;
};

struct GIF_FRAME
{
    BITMAP *bitmap_8_bit;
    GIF_PALETTE palette;
    int xoff, yoff;
    int duration;               /* in 1/100th seconds */
    int disposal_method;        /* 0 = don't care, 1 = keep, 2 = background, 3 = previous */
    int transparent_index;
};

/* Simple use. */
void algif_init (void);
int algif_load_animation (char const *filename, BITMAP ***frames, int **durations);
BITMAP *load_gif (AL_CONST char *filename, RGB *pal);
int save_gif (AL_CONST char *filename, BITMAP *bmp, AL_CONST RGB *pal);

/* Advanced use. */
GIF_ANIMATION *algif_load_raw_animation (char const *filename);
void algif_render_frame (GIF_ANIMATION *gif, BITMAP *bitmap, int frame, int xpos, int ypos);
GIF_ANIMATION *algif_create_raw_animation (int frames_count);
int algif_save_raw_animation (const char *filename, GIF_ANIMATION *gif);
void algif_destroy_raw_animation (GIF_ANIMATION *gif);

#ifdef __cplusplus
}
#endif

#endif
