//jwin for a5

#ifndef _JWIN_A5_H_
#define _JWIN_A5_H_

#include "jwin.h"

extern ALLEGRO_COLOR jwin_a5_colors[9];
extern ALLEGRO_COLOR db_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc);

void jwin_set_a5_colors(ALLEGRO_COLOR* colors);
void jwin_get_a5_colors(ALLEGRO_COLOR* colors);

void start_db_proc();
void end_db_proc();

ALLEGRO_COLOR a5color(RGB c);
ALLEGRO_COLOR a5color(int index);

void _handle_jwin_scrollable_scroll_click_a5(DIALOG *d, int32_t listsize, int32_t *offset, ALLEGRO_FONT *fnt);
void _jwin_draw_scrollable_frame_a5(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type);

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_tab_proc_a5(int32_t msg, DIALOG *d, int32_t c);

#endif                                                      // _JWIN_H_
