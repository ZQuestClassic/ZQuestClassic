//jwin for a5

#ifndef JWIN_A5_H_
#define JWIN_A5_H_

#include "gui/jwin.h"

extern ALLEGRO_COLOR jwin_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc);

void jwin_set_a5_colors(ALLEGRO_COLOR* colors);

ALLEGRO_COLOR a5color(RGB c);
ALLEGRO_COLOR a5color(int index);

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t);

void al_draw_int_rectangle(int x1, int y1, int x2, int y2, ALLEGRO_COLOR col, double width);

#endif                                                      // _JWIN_H_
