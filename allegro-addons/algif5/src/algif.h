#ifndef _GIF_H_
#define _GIF_H_

#include <allegro5/allegro5.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALGIF_ANIMATION ALGIF_ANIMATION;
typedef struct ALGIF_FRAME ALGIF_FRAME;
typedef struct ALGIF_PALETTE ALGIF_PALETTE;
typedef struct ALGIF_BITMAP ALGIF_BITMAP;
typedef struct ALGIF_RGB ALGIF_RGB;

struct ALGIF_RGB {
    uint8_t r, g, b;
};

struct ALGIF_PALETTE {
    int colors_count;
    ALGIF_RGB colors[256];
};

struct ALGIF_BITMAP {
    int w, h;
    uint8_t *data;
};

struct ALGIF_ANIMATION {
    int width, height;
    int frames_count;
    int background_index;
    int loop; /* -1 = no, 0 = forever, 1..65535 = that many times */
    ALGIF_PALETTE palette;
    ALGIF_FRAME *frames;

    int duration;
    ALLEGRO_BITMAP *store;
};

struct ALGIF_FRAME {
    ALGIF_BITMAP *bitmap_8_bit;
    ALGIF_PALETTE palette;
    int xoff, yoff;
    int duration;               /* in 1/100th seconds */
    int disposal_method;        /* 0 = don't care, 1 = keep, 2 = background, 3 = previous */
    int transparent_index;

    ALLEGRO_BITMAP *rendered;
};

ALGIF_ANIMATION *algif_load_raw(ALLEGRO_FILE *file);
ALGIF_ANIMATION *algif_load_animation_f(ALLEGRO_FILE *file);
ALGIF_ANIMATION *algif_load_animation(char const *filename);
void algif_render_frame(ALGIF_ANIMATION *gif, int frame, int xpos, int ypos);
void algif_destroy_animation (ALGIF_ANIMATION *gif);

ALGIF_BITMAP *algif_create_bitmap(int w, int h);
void algif_destroy_bitmap(ALGIF_BITMAP *bitmap);
void algif_blit(ALGIF_BITMAP *from, ALGIF_BITMAP *to, int xf, int yf, int xt, int yt,
        int w, int h);
ALLEGRO_BITMAP *algif_get_bitmap(ALGIF_ANIMATION *gif, double seconds);
ALLEGRO_BITMAP *algif_get_frame_bitmap(ALGIF_ANIMATION *gif, int i);
double algif_get_frame_duration(ALGIF_ANIMATION *gif, int i);

#ifdef __cplusplus
}
#endif

#endif
