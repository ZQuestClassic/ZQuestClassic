/*                 __                  __
  *                /_/\  __  __  __    /_/\  ______
  *               _\_\/ / /\/ /\/ /\  _\_\/ / ____ \
  *              / /\  / / / / / / / / /\  / /\_ / /\
  *         __  / / / / /_/ /_/ / / / / / / / / / / /
  *        / /_/ / / /_________/ / /_/ / /_/ / /_/ /
  *        \____/ /  \_________\/  \_\/  \_\/  \_\/
  *         \___\/
  *
  *
  *
  *     jwin.h
  *
  *     Windows(R) style GUI for Allegro.
  *     by Jeremy Craner
  *
  *     Most routines are adaptations of Allegro code.
  *     Allegro is by Shawn Hargreaves, et al.
  *
  *     Version: 3/22/00
  *     Allegro version: 3.1x  (don't know if it works with WIP)
  *
  *     How to use:
  *
  *     - include jwin.c in your project
  *     - include this header in your code
  *
  *     - do the same for jwinfsel.* if you want to use the file selector
  *
  *     - call jwin_set_colors() at program start up
  *       (send jwin_colors for default colors or make your own array
  *       of colors and send them to jwin_set_colors()... see jwin.c)
  *
  *     - use the DIALOG procs the same way you use the default Allegro ones
  *
  */

/* This code is not fully tested */

#ifndef _JWIN_H_
#define _JWIN_H_

#include "zc_alleg.h"
#include "tab_ctl.h"

struct ListData
{
    constexpr ListData() noexcept:
        unownedFunc(nullptr), ownedFunc(nullptr), font(nullptr), owner(nullptr)
    {}

    ListData(const char *(*lf)(int, int*), FONT **f) noexcept:
        unownedFunc(lf), ownedFunc(nullptr), font(f), owner(nullptr)
    {}

    ListData(const char *(*lf)(int, int*, void*), FONT **f, void* o) noexcept:
        unownedFunc(nullptr), ownedFunc(lf), font(f), owner(o)
    {}

    const char* listFunc(int index, int* size) const
    {
        if(owner)
            return ownedFunc(index, size, owner);
        else
            return unownedFunc(index, size);
    }

    const char *(*unownedFunc)(int, int *);
    const char *(*ownedFunc)(int, int *, void *);

    FONT **font;
    void* owner;
};

#ifdef __cplusplus
extern "C"
{
#endif

/* read only flag for jwin_edit_proc */
#define D_READONLY      D_USER

/* Disable click to select combo for d_combo_proc */
#define D_NOCLICK       D_USER

#define D_RESIZED		(D_USER<<1)

/* This should be used for all DIALOGs in the new GUI system.
 * It indicates that there is a Dialog that owns this array
 * and should be prompted to handle some events.
 */
#define D_NEW_GUI (D_USER<<2)

/* Sent to newgui_dialog_proc to tell it to handle an event. */
#define MSG_GUI_EVENT MSG_USER

/* frame styles */
enum {
    FR_WIN, FR_BOX, FR_INV, FR_DEEP, FR_DARK, FR_ETCHED, FR_MEDDARK
};

/* todo: (maybe) put the colors into a struct */

/* color indices */
enum
{
	jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
	jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
	jcCURSORMISC, jcCURSOROUTLINE, jcCURSORLIGHT, jcCURSORDARK,
	jcMAX
};

extern int abc_patternmatch;

/* a copy of the default color scheme; do what you want with this */
extern int jwin_colors[jcMAX];

extern int mix_value(int c1,int c2,int pos,int max);

/* 1.5k lookup table for color matching */
extern unsigned int col_diff[3*128];
extern int last_droplist_sel;

/* Used to indicate the new GUI dialog root. */
extern char newGuiMarker;

/* All the events that may be handled by a new GUI widget.
 * These could possibly be pared down a bit, but it doesn't matter much.
 */
enum guiEvent
{
    geCLICK, geCHANGE_SELECTION, geCHANGE_VALUE, geCLOSE, geENTER, geTOGGLE
};

#define GUI_EVENT(dlg, event)                \
do                                           \
{                                            \
    if(dlg->flags&D_NEW_GUI)                 \
    {                                        \
        int ret=new_gui_event(dlg-1, event); \
        if(ret>=0)                           \
            return ret;                      \
    }                                        \
} while(false)

/* Triggers a message in the new GUI system. You should use the macro below
 * instead of calling this directly.
 */
int new_gui_event(DIALOG* d, guiEvent event);

int get_selected_tab(TABPANEL* panel);

/* you should call this before using the other procedures */
void jwin_set_colors(int *colors);

/* drawing routines */
void jwin_draw_frame(BITMAP *dest,int x,int y,int w,int h,int style);
void jwin_draw_minimap_frame(BITMAP *dest,int x,int y,int w,int h,int scrsz,int style);
void jwin_draw_win(BITMAP *dest,int x,int y,int w,int h,int frame);
void jwin_draw_button(BITMAP *dest,int x,int y,int w,int h,int state,int type);
void draw_x_button(BITMAP *dest, int x, int y, int state);
char *shorten_string(char *dest, char *src, FONT *usefont, int maxchars, int maxwidth);
void jwin_draw_titlebar(BITMAP *dest, int x, int y, int w, int h, const char *str, bool draw_button);
void jwin_draw_text_button(BITMAP *dest, int x, int y, int w, int h, const char *str, int flags, bool show_dotted_rect);
void jwin_draw_graphics_button(BITMAP *dest, int x, int y, int w, int h, BITMAP *bmp, BITMAP *bmp2, int flags, bool show_dotted_rect, bool overlay);

/* Allegro DIALOG procedures */
int jwin_win_proc(int msg, DIALOG *d, int c);
int jwin_frame_proc(int msg, DIALOG *d, int c);
int jwin_guitest_proc(int msg, DIALOG *d, int c);
int jwin_button_proc(int msg, DIALOG *d, int c);
int jwin_func_button_proc(int msg, DIALOG *d, int c);
int jwin_text_proc(int msg, DIALOG *d, int c);
int jwin_ctext_proc(int msg, DIALOG *d, int c);
int jwin_rtext_proc(int msg, DIALOG *d, int c);
int jwin_edit_proc(int msg, DIALOG *d, int c);
int jwin_hexedit_proc(int msg,DIALOG *d,int c); /**< Restricted only to hex. numbers */
int jwin_numedit_zscriptint_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers,  bound to ZScript int (no decimals) */
int jwin_numedit_byte_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers, bound to unsigned byte (8b) */
int jwin_numedit_sbyte_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers, bound to signed byte (8b) */
int jwin_numedit_short_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers, bound to unsigned short int (16b) */
int jwin_numedit_sshort_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers, bound to signed short int (16b) */
int jwin_numedit_proc(int msg,DIALOG *d,int c); /**< Restricted only to dec. numbers */
int jwin_list_proc(int msg, DIALOG *d, int c);
int jwin_textbox_proc(int msg, DIALOG *d, int c);
int jwin_slider_proc(int msg, DIALOG *d, int c);
int jwin_menu_proc(int msg, DIALOG *d, int c);
int jwin_droplist_proc(int msg, DIALOG *d, int c);
int jwin_abclist_proc(int msg, DIALOG *d, int c);
int jwin_check_proc(int msg, DIALOG *d, int c);
int jwin_checkfont_proc(int msg, DIALOG *d, int c);
int jwin_radio_proc(int msg, DIALOG *d, int c);
int jwin_radiofont_proc(int msg, DIALOG *d, int c);
int jwin_tab_proc(int msg, DIALOG *d, int c);
int jwin_hline_proc(int msg, DIALOG *d, int c);
int jwin_vline_proc(int msg, DIALOG *d, int c);
void _jwin_draw_abclistbox(DIALOG *d);
int jwin_do_abclist_proc(int msg, DIALOG *d, int c);
void wipe_abc_keypresses();

/* other GUI procedures */
void jwin_set_dialog_color(DIALOG *dialog);

int gui_textout_ln(BITMAP *bmp, FONT *f, unsigned char *s, int x, int y, int color, int bg, int pos);

int jwin_do_menu(MENU *menu, int x, int y);

int jwin_alert3(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, const char *b3, int c1, int c2, int c3, FONT *title_font);
int jwin_alert(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, int c1, int c2, FONT *title_font);

/* event handler that closes a dialog */
int close_dlg();
int mouse_in_rect(int x,int y,int w,int h);

void bestfit_init(void);
int bestfit_color_range(AL_CONST PALETTE pal, int r, int g, int b, unsigned char start, unsigned char end);
int makecol8_map(int r, int g, int b, RGB_MAP *table);
void create_rgb_table_range(RGB_MAP *table, AL_CONST PALETTE pal, unsigned char start, unsigned char end, void (*callback)(int pos));
int short_bmp_avg(BITMAP *bmp, int i);
void dither_rect(BITMAP *bmp, PALETTE *pal, int x1, int y1, int x2, int y2,
                 int src_color1, int src_color2, unsigned char dest_color1,
                 unsigned char dest_color2);
void jwin_center_dialog(DIALOG *dialog);
void jwin_ulalign_dialog(DIALOG *dialog);

void _calc_scroll_bar(int h, int height, int listsize, int offset,
                      int *bh, int *len, int *pos);
void draw_arrow_button(BITMAP *dest, int x, int y, int w, int h, int up, int state);
void draw_arrow_button_horiz(BITMAP *dest, int x, int y, int w, int h, int up, int state);


void dotted_rect(BITMAP *dest, int x1, int y1, int x2, int y2, int fg, int bg);
void _jwin_draw_scrollable_frame(DIALOG *d, int listsize, int offset, int height, int type);
void _handle_jwin_scrollable_scroll_click(DIALOG *d, int listsize, int *offset, FONT *fnt);

extern int  popup_zqdialog(DIALOG *dialog, int focus_obj);
extern int  do_zqdialog(DIALOG *dialog, int focus_obj);

int d_jslider_proc(int msg, DIALOG *d, int c);
int d_jwinbutton_proc(int msg, DIALOG *d, int c);

//Misc bitmap drawing
void draw_x(BITMAP* dest, int x1, int y1, int x2, int y2, int color);


#ifdef __cplusplus
}
#endif
#endif                                                      // _JWIN_H_
