//jwin for a5

#ifndef _JWIN_A5_H_
#define _JWIN_A5_H_

#include "jwin.h"

extern ALLEGRO_COLOR jwin_a5_colors[9];
extern ALLEGRO_COLOR db_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc);

void jwin_set_a5_colors(ALLEGRO_COLOR* colors);

ALLEGRO_COLOR a5color(RGB c);
ALLEGRO_COLOR a5color(int index);

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_tab_proc_a5(int32_t msg, DIALOG *d, int32_t c);

#endif                                                      // _JWIN_H_
