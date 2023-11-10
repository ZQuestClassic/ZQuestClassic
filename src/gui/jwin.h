#ifndef _JWIN_H_
#define _JWIN_H_

#include "base/zdefs.h"
#include "base/zc_alleg.h"
#include "tab_ctl.h"
#include "base/gui.h"

struct ListData
{
    constexpr ListData() noexcept:
        unownedFunc(nullptr), ownedFunc(nullptr), font(nullptr), owner(nullptr)
    {}

    ListData(const char *(*lf)(int32_t, int32_t*), FONT **f) noexcept:
        unownedFunc(lf), ownedFunc(nullptr), font(f), owner(nullptr)
    {}

    ListData(const char *(*lf)(int32_t, int32_t*, void*), FONT **f, void* o) noexcept:
        unownedFunc(nullptr), ownedFunc(lf), font(f), owner(o)
    {}

    const char* listFunc(int32_t index, int32_t* size) const
    {
        if(owner)
            return ownedFunc(index, size, owner);
        else
            return unownedFunc(index, size);
    }

    const char *(*unownedFunc)(int32_t, int32_t *);
    const char *(*ownedFunc)(int32_t, int32_t *, void *);

    FONT **font;
    void* owner;
};

byte getHighlightColor(int32_t c);
byte getHighlightColor(RGB const& col);

#ifdef __cplusplus
extern "C"
{
#endif

/* read only flag for jwin_edit_proc */
#define D_READONLY      D_USER

/* Disable click to select combo for d_combo_proc */
#define D_NOCLICK       D_USER

#define D_RESIZED		(D_USER<<5)

/* This should be used for all DIALOGs in the new GUI system.
 * It indicates that there is a Dialog that owns this array
 * and should be prompted to handle some events.
 */
#define D_NEW_GUI (D_USER<<2)

/* Set on widgets inside a scrolling pane so that they know they're in one. */
#define D_SCROLLING (D_USER<<3)

/* Set on widgets in a scrolling pane to indicate that the screen bitmap
 * is clipped before redrawing.
 */
#define D_ISCLIPPED (D_USER<<4)

/* Sent to newgui_dialog_proc to tell it to handle an event. */
#define MSG_GUI_EVENT MSG_USER

/* Send to a scrolling pane to redraw a child with the screen bitmap clipped. */
#define MSG_DRAWCLIPPED (MSG_USER+1)

/* Sent to a scrolling pane to indicate that one of its children has
 * taken focus and it needs to scroll to show it.
 */
#define MSG_CHILDFOCUSED (MSG_USER+2)

/* Sent to a proc to check if it handles mouse wheel events. If not,
 * they can be handled by the containing scrolling pane. Any nonzero
 * return value is treated as true.
 */
#define MSG_WANTWHEEL (MSG_USER+3)

#define MSG_VSYNC (MSG_USER+4)

/* frame styles */
enum {
    FR_INVIS = -1, FR_WIN, FR_BOX, FR_INV, FR_DEEP, FR_DARK, FR_ETCHED, FR_MEDDARK, FR_RED, FR_GREEN
};

/* todo: (maybe) put the colors into a struct */

/* color indices */
enum
{
	jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
	jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
	jcCURSORMISC, jcCURSOROUTLINE, jcCURSORLIGHT, jcCURSORDARK,
	jcALT_TEXTBG, jcALT_TEXTFG,
	jcDISABLED_BG, jcDISABLED_FG,
	jcMAX
};

extern int32_t abc_patternmatch;

/* a copy of the default color scheme; do what you want with this */
extern int32_t jwin_colors[jcMAX];

extern int32_t jwin_pal[jcMAX];

extern int32_t scheme[jcMAX];

extern int32_t mix_value(int32_t c1,int32_t c2,int32_t pos,int32_t max);

/* 1.5k lookup table for color matching */
extern uint32_t col_diff[3*128];
extern int32_t last_droplist_sel;

/* Used to indicate the new GUI dialog root. */
extern char newGuiMarker;

/* All the events that may be handled by a new GUI widget.
 * These could possibly be pared down a bit, but it doesn't matter much.
 */
enum guiEvent
{
    geCLICK, geCHANGE_SELECTION, geCHANGE_VALUE, geCLOSE, geENTER, geTOGGLE, geUPDATE_SWAP,
	geRADIO, geRCLICK, geDCLICK
};

enum
{
	ONTICK_EXIT = -1, ONTICK_CONTINUE, ONTICK_CLOSE, ONTICK_REDRAW
};

enum
{
	BTNICON_ARROW_UP, //0x88
	BTNICON_ARROW_DOWN, //0x89
	BTNICON_ARROW_LEFT, //0x8A
	BTNICON_ARROW_RIGHT, //0x8B
	BTNICON_CONTRACT_VERT, //0x98
	BTNICON_EXPAND_VERT, //0x99
	BTNICON_CONTRACT_HORZ, //0x9A
	BTNICON_EXPAND_HORZ, //0x9B
	BTNICON_ARROW_RIGHT2, //0x8B 0x8B
	BTNICON_ARROW_RIGHT3, //0x8B 0x8B 0x8B
	BTNICON_STOPSQUARE, //0x8D
	BTNICON_MINUS,
	BTNICON_PLUS,
	BTNICON_ARROW_LEFT2,
	BTNICON_ARROW_LEFT3,
	BTNICON_MAX
};

#define GUI_EVENT(dlg, event)                  \
do                                             \
{                                              \
	if(dlg->flags&D_NEW_GUI)                   \
	{                                          \
		int32_t ret = new_gui_event(dlg-1, event); \
		if(ret >= 0)                           \
			return ret;                        \
	}                                          \
} while(false)

/* Triggers a message in the new GUI system. You should use the macro below
 * instead of calling this directly.
 */
int32_t new_gui_event(DIALOG* d, guiEvent event);

int32_t get_selected_tab(TABPANEL* panel);

/* you should call this before using the other procedures */
void jwin_set_colors(int32_t *colors);

/* drawing routines */
void jwin_draw_frame(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t style);
void jwin_draw_frag_frame(BITMAP* dest, int x1, int y1, int w, int h, int fw, int fh, int style);
void jwin_draw_minimap_frame(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t scrsz,int32_t style);
void jwin_draw_win(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t frame);
void jwin_draw_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int32_t state,int32_t type);
void draw_x_button(BITMAP *dest, int32_t x, int32_t y, int32_t state);
void draw_question_button(BITMAP *dest, int32_t x, int32_t y, int32_t state);
char *shorten_string(char *dest, char const* src, FONT *usefont, int32_t maxchars, int32_t maxwidth);
void jwin_draw_titlebar(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, const char *str, bool draw_button, bool helpbtn = false);
void jwin_draw_text_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, const char *str, int32_t flags, bool show_dotted_rect);
void jwin_draw_icon_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int icon, int32_t flags, bool show_dotted_rect);
void jwin_draw_graphics_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, BITMAP *bmp, BITMAP *bmp2, int32_t flags, bool show_dotted_rect, bool overlay);
bool do_text_button(int32_t x,int32_t y,int32_t w,int32_t h,const char *text);

/* Allegro DIALOG procedures */
int32_t jwin_win_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_frame_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_guitest_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_button_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_iconbutton_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_infobtn_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_func_button_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_text_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_ctext_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_rtext_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t d_ctext2_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t new_text_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_edit_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_hexedit_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to hex. numbers */
int32_t jwin_numedit_zscriptint_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers,  bound to ZScript int32_t (no decimals) */
int32_t jwin_numedit_byte_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers, bound to unsigned byte (8b) */
int32_t jwin_numedit_sbyte_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers, bound to signed byte (8b) */
int32_t jwin_numedit_short_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers, bound to uint16_t int32_t (16b) */
int32_t jwin_numedit_sshort_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers, bound to int16_t (16b) */
int32_t jwin_numedit_proc(int32_t msg,DIALOG *d,int32_t c); /**< Restricted only to dec. numbers */
//
int32_t jwin_swapbtn_proc(int32_t msg,DIALOG *d,int32_t c); //Button to swap numedit styles
int32_t jwin_numedit_swap_byte_proc(int32_t msg,DIALOG *d,int32_t c); //Bound to unsigned byte, dec and hex modes
int32_t jwin_numedit_swap_sshort_proc(int32_t msg,DIALOG *d,int32_t c); //Bound to int16_t, dec and hex modes
int32_t jwin_numedit_swap_zsint_proc(int32_t msg,DIALOG *d,int32_t c); //Bound to int32_t, dec and hex modes, 4 dec places, int32_t modes
int32_t jwin_numedit_swap_zsint_nodec_proc(int32_t msg,DIALOG *d,int32_t c); //Bound to int32_t, dec and hex modes, no dec places
int32_t jwin_numedit_swap_zsint2_proc(int32_t msg,DIALOG *d,int32_t c); //Bound to int32_t, dec and hex modes, 4 dec places, int32_t modes, bool mode
int32_t jwin_numedit_noswap_zsint_proc(int32_t msg,DIALOG *d,int32_t c); //Same as 'jwin_numedit_swap_zsint2_proc', but starts and stays in one mode.
//
int32_t jwin_list_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_textbox_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_slider_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_menu_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_droplist_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_abclist_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_check_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_checkfont_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t new_check_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_radio_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_radiofont_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_tab_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t new_tab_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_hline_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t jwin_vline_proc(int32_t msg, DIALOG *d, int32_t c);
void _jwin_draw_abclistbox(DIALOG *d);
int32_t jwin_do_abclist_proc(int32_t msg, DIALOG *d, int32_t c);
void wipe_abc_keypresses();

/* other GUI procedures */
void jwin_set_dialog_color(DIALOG *dialog);

int32_t gui_textout_ln(BITMAP *bmp, FONT *f, unsigned const char *s, int32_t x, int32_t y, int32_t color, int32_t bg, int32_t pos);
int32_t gui_text_width(FONT *f, const char *s);

int32_t jwin_do_menu(MENU *menu, int32_t x, int32_t y);

int32_t jwin_color_swatch(int32_t msg, DIALOG *d, int32_t c);

int32_t jwin_alert3(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, const char *b3, int32_t c1, int32_t c2, int32_t c3, FONT *title_font);
int32_t jwin_alert(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, int32_t c1, int32_t c2, FONT *title_font);
int32_t jwin_auto_alert3(const char *title, const char *s1, int32_t lenlim, int32_t vspace, const char *b1, const char *b2, const char *b3, int32_t c1, int32_t c2, int32_t c3, FONT *title_font);
int32_t jwin_auto_alert(const char *title, const char *s1, int32_t lenlim, int32_t vspace, const char *b1, const char *b2, int32_t c1, int32_t c2, FONT *title_font);

/* event handler that closes a dialog */
int32_t close_dlg();
int32_t mouse_in_rect(int32_t x,int32_t y,int32_t w,int32_t h);

void bestfit_init(void);
int32_t bestfit_color_range(AL_CONST PALETTE pal, int32_t r, int32_t g, int32_t b, uint8_t start, uint8_t end);
int32_t makecol8_map(int32_t r, int32_t g, int32_t b, RGB_MAP *table);
void create_rgb_table_range(RGB_MAP *table, AL_CONST PALETTE pal, uint8_t start, uint8_t end, void (*callback)(int32_t pos));
int32_t short_bmp_avg(BITMAP *bmp, int32_t i);
void lfsrInit();
uint16_t lfsrNext();
void dither_rect(BITMAP *bmp, PALETTE *pal, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                 int32_t src_color1, int32_t src_color2, uint8_t dest_color1,
                 uint8_t dest_color2);
bool do_text_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text);
bool do_icon_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,int icon);
void jwin_center_dialog(DIALOG *dialog);
void jwin_ulalign_dialog(DIALOG *dialog);

void _calc_scroll_bar(int32_t h, int32_t height, int32_t listsize, int32_t offset,
                      int32_t *bh, int32_t *len, int32_t *pos);
void draw_arrow_button(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state);
void draw_arrow_button_horiz(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t up, int32_t state);


void dotted_rect(BITMAP *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t fg, int32_t bg);
void _jwin_draw_scrollable_frame(DIALOG *d, int32_t listsize, int32_t offset, int32_t height, int32_t type);
void _handle_jwin_scrollable_scroll_click(DIALOG *d, int32_t listsize, int32_t *offset, FONT *fnt);

int32_t d_jslider_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t d_jwinbutton_proc(int32_t msg, DIALOG *d, int32_t c);

//Misc bitmap drawing
void draw_x(BITMAP* dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color);
void draw_checkerboard(BITMAP* dest, int basex, int basey, int x, int y, int sz, optional<int> cb_sz = nullopt);

int32_t d_vsync_proc(int32_t msg,DIALOG *,int32_t c);

//box_out stuff
void box_out(const char *msg);
void box_out_nl(const char *msg);
void box_start(int32_t style, const char *title, FONT *title_font, FONT *message_font, bool log, int32_t w = -1, int32_t h = -1, uint8_t scale = 1);
void box_end(bool pause);
void box_eol();

#ifdef __cplusplus
}
#endif

void jwin_draw_icon(BITMAP *dest, int x, int y, int col, int icon, int asz, bool center);
void jwin_draw_icon(BITMAP *dest, int x, int y, int col, int icon, int aw, int ah, bool center);
int icon_proportion(int icon,int s1,int s2);

void draw_dis_checkbox(BITMAP *dest,int x,int y,int wid,int hei,bool value);
void draw_checkbox(BITMAP *dest,int x,int y,int wid,int hei,bool value);
void draw_checkbox(BITMAP *dest,int x,int y,int sz,bool value);
bool do_checkbox(BITMAP *dest,int x,int y,int wid,int hei,int &value, int xoffs = 0, int yoffs = 0);
bool do_checkbox_tx(BITMAP *dest,int x,int y,int wid,int hei,int &value, int txtoffs, int xoffs = 0, int yoffs = 0);
bool do_scheckbox(BITMAP *dest,int x,int y,int sz,int &value, int xoffs = 0, int yoffs = 0);

#endif                                                      // _JWIN_H_
