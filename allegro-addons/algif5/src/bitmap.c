#include "algif.h"

ALGIF_BITMAP *algif_create_bitmap(int w, int h) {
    ALGIF_BITMAP *bitmap = calloc(1, sizeof *bitmap);
    bitmap->w = w;
    bitmap->h = h;
    bitmap->data = calloc(1, w * h);
    return bitmap;
}

void algif_destroy_bitmap(ALGIF_BITMAP *bitmap) {
    free(bitmap->data);
    free(bitmap);
}

void algif_blit(ALGIF_BITMAP *from, ALGIF_BITMAP *to, int xf, int yf, int xt, int yt,
        int w, int h) {

    if (w <= 0 || h <= 0)
        return;

    /* source clipping */
    if (xf < 0) {
        w += xf;
        xt -= xf;
        xf = 0;
    }
    if (yf < 0) {
        h += yf;
        yt -= yf;
        yf = 0;
    }
    int wf = from->w;
    int hf = from->h;
    if (xf + w > wf) {
        w = wf - xf;
    }
    if (yf + h > hf) {
        h = hf - yf;
    }

    /* destination clipping */
    if (xt < 0) {
        w += xt;
        xf -= xt;
        xt = 0;
    }
    if (yt < 0) {
        h += yt;
        yf -= yt;
        yt = 0;
    }
    int wt = to->w;
    int ht = to->h;
    if (xt + w > wt) {
        w = wt - xt;
    }
    if (yt + h > ht) {
        h = ht - yt;
    }

    if (w <= 0 || h <= 0)
        return;

    /* copy row by row */
    uint8_t *pf = from->data + yf * from->w;
    uint8_t *pt = to->data + yt * to->w;
    int i;
    for (i = 0; i < h; i++) {
        memmove(pt + xt, pf + xf, w);
        pf += from->w;
        pt += to->w;
    }
}
