//jwin for a5

#ifndef _JWIN_A5_H_
#define _JWIN_A5_H_

#include "jwin.h"

extern int InvalidStatic;

extern ALLEGRO_COLOR AL5_INVIS, AL5_BLACK, AL5_WHITE, AL5_YELLOW, AL5_PINK,
	AL5_VLGRAY, AL5_LGRAY, AL5_DGRAY, AL5_BLUE, AL5_LRED, AL5_DRED;
extern ALLEGRO_COLOR AL5_COL_SOLIDITY, AL5_COL_EFFECT, AL5_COL_CS2;
extern ALLEGRO_COLOR jwin_a5_colors[9];
extern ALLEGRO_COLOR db_a5_colors[9];
ALLEGRO_COLOR jwin_a5_pal(int jc);

struct cliprect
{
	int x=0, y=0, w=0, h=0;
	void getclip();
	void setclip() const;
};

void jwin_reset_a5_colors();
void jwin_set_a5_colors(ALLEGRO_COLOR* colors, bool setmain = false);
void jwin_get_a5_colors(ALLEGRO_COLOR* colors, bool getmain = false);

ALLEGRO_COLOR& a5_rainbow(int ind);

void start_db_proc();
void end_db_proc();

bool do_over_area(int x, int y, int w, int h, bool sel);

void al_draw_hline(float x1, float y1, float x2, ALLEGRO_COLOR c);
void al_draw_vline(float x1, float y1, float y2, ALLEGRO_COLOR c);
void al_draw_x(float x1, float y1, float x2, float y2, ALLEGRO_COLOR c, float thickness);

void draw_static(int x, int y, int w, int h);
void al5_invalid(int x, int y, int w, int h, bool sides = true);

void jwin_draw_frame_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t style);
void jwin_draw_frag_frame_a5(int x1, int y1, int w, int h, int fw, int fh, int style);
void jwin_draw_minimap_frame_a5(int x,int y,int w,int h,int scrsz,int style);
void jwin_draw_win_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t frame);
void dotted_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR fg, ALLEGRO_COLOR bg);
void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str);
void jwin_textout_a5(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc);
void jwin_textout_a5_dis(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR dis_c);
void jwin_textout_a5_shd(ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR shd_c, int shdty);
void jwin_textout_a5_scl(float scl, ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc);
void jwin_textout_a5_scl_dis(float scl, ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR dis_c);
void jwin_textout_a5_scl_shd(float scl, ALLEGRO_FONT* f, ALLEGRO_COLOR tc, float x, float y, int flag, char const* str, ALLEGRO_COLOR bgc, ALLEGRO_COLOR shd_c, int shdty);
void jwin_draw_button_a5(int32_t x,int32_t y,int32_t w,int32_t h,int32_t state,int32_t type);
void jwin_draw_text_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, int32_t flags, bool show_dotted_rect = false);
bool do_text_button_a5(int32_t x,int32_t y,int32_t w,int32_t h,const char *text);
bool do_text_button_reset_a5(int32_t x,int32_t y,int32_t w,int32_t h,const char *text);
void draw_question_button_a5(int32_t x, int32_t y, int32_t state);
void draw_x_button_a5(int32_t x, int32_t y, int32_t state);
void draw_checkbox_a5(int32_t x,int32_t y,int32_t sz,bool value);
void draw_checkbox_a5(int32_t x,int32_t y,int32_t wid,int32_t hei,bool value);
void draw_arrow_a5(ALLEGRO_COLOR c, int32_t x, int32_t y, int32_t h, bool up, bool center);
void draw_arrow_horz_a5(ALLEGRO_COLOR c, int32_t x, int32_t y, int32_t w, bool left, bool center);
void draw_arrow_button_a5(int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state);
int32_t jwin_do_x_button_dlg_a5(int32_t x, int32_t y);
bool jwin_do_x_button_a5(int32_t x, int32_t y);
bool jwin_do_question_button_a5(int32_t x, int32_t y);
bool do_checkbox_a5(int32_t x,int32_t y,int32_t sz,int32_t &value);
bool do_checkbox_a5(int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t &value);
void dither_rect_a5(int32_t x1, int32_t y1, int32_t x2, int32_t y2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2);
void jwin_draw_titlebar_a5(int32_t x, int32_t y, int32_t w, int32_t h, const char *str, bool draw_button, bool helpbtn = false);


void _handle_jwin_scrollable_scroll_click_a5(DIALOG *d, int32_t listsize, int32_t *offset, ALLEGRO_FONT *fnt);
void _handle_jwin_scrollable_scroll_a5(DIALOG *d, int32_t listsize, int32_t *index, int32_t *offset, ALLEGRO_FONT *fnt);
bool _handle_jwin_listbox_click_a5(DIALOG *d);
void _jwin_draw_scrollable_frame_a5(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type);
void _jwin_draw_listbox_a5(DIALOG *d,bool abc);

int32_t gui_textout_ln_a5(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos);
int32_t gui_textout_ln_a5_dis(ALLEGRO_FONT *f, const char *s, int32_t x, int32_t y, ALLEGRO_COLOR color, ALLEGRO_COLOR bg, int32_t pos, ALLEGRO_COLOR dis_c);
int32_t gui_text_width_a5(ALLEGRO_FONT *f, const char *s);
int32_t gui_text_height_a5(ALLEGRO_FONT* f, const char *s);

int32_t jwin_selcolor_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_color_swatch_a5(int32_t msg, DIALOG *d, int32_t c);

int getnumber(const char *prompt, int initialval, bool* cancelled = nullptr);
int getnumber_hex(const char *prompt, int initialval, bool* cancelled = nullptr);

int32_t jwin_win_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_tab_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_text_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_ctext_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_rtext_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t new_text_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_button_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t new_check_proc_a5(int32_t msg, DIALOG *d, int32_t);
int32_t jwin_list_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_abclist_proc_a5(int32_t msg,DIALOG *d,int32_t c);
int32_t jwin_droplist_proc_a5(int32_t msg,DIALOG *d,int32_t c);
int32_t jwin_frame_proc_a5(int32_t msg, DIALOG *d, int32_t);

int32_t jwin_swapbtn_proc_a5(int32_t msg, DIALOG* d, int32_t c);
int32_t jwin_numedit_swap_byte_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_numedit_swap_sshort_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_numedit_swap_zsint_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_numedit_swap_zsint_nodec_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_numedit_swap_zsint2_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_vedit_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_edit_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_hexedit_proc_a5(int32_t msg,DIALOG *d,int32_t c);
int32_t jwin_numedit_proc_a5(int32_t msg,DIALOG *d,int32_t c);

int32_t jwin_radio_proc_a5(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_radiofont_proc_a5(int32_t msg, DIALOG *d, int32_t c);

#endif                                                      // _JWIN_H_

