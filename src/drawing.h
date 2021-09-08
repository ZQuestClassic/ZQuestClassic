#ifndef DRAW_H
#define DRAW_H

#define DITHER_FACTOR 5
void dithercircfill(BITMAP* dest, int x, int y, int rad, int color, int ditherType);

#endif