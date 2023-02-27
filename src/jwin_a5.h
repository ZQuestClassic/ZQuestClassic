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

void jwin_draw_frame_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t style);
void jwin_draw_win_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t frame);
void dotted_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR fg, ALLEGRO_COLOR bg);
void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str);
void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc);
void jwin_textout_a5_dis(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR dis_c);
void jwin_draw_button_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t state,int32_t type);
void jwin_draw_text_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, int32_t flags, bool show_dotted_rect);
bool do_text_button_reset_a5(int32_t x,int32_t y,int32_t w,int32_t h,const char *text);
void draw_question_button_a5(int32_t x, int32_t y, int32_t state);
void draw_x_button_a5(int32_t x, int32_t y, int32_t state);
void draw_arrow_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state);
int32_t jwin_do_x_button_a5(int32_t x, int32_t y);
void dither_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2);
void jwin_draw_titlebar_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, bool draw_button, bool helpbtn);


void _handle_jwin_scrollable_scroll_click_a5(DIALOG *d, int32_t listsize, int32_t *offset, ALLEGRO_FONT *fnt);
void _jwin_draw_scrollable_frame_a5(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type);

int32_t gui_textout_ln_a5(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos);
int32_t gui_textout_ln_a5_dis(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos, ALLEGRO_COLOR dis_c);
int32_t gui_text_width_a5(ALLEGRO_FONT *f, const char *s);
int32_t gui_text_height_a5(ALLEGRO_FONT* f, const char *s);

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_tab_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_text_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_ctext_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_rtext_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t new_text_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_button_proc_a5(int32_t msg, DIALOG *d, int32_t);


#endif                                                      // _JWIN_H_

