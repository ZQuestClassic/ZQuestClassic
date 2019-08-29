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
  *     jwin.c
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
  */

#ifndef _EB_INTERN_H_
#define _EB_INTERN_H_

#include "zc_alleg.h"
#include "editbox.h"

#ifdef __cplusplus
extern "C"
{
#endif

  //extern FONT* font_vtable_color;

  /* RGB -> color mapping table. Not needed, but speeds things up */
  RGB_MAP eb_rgb_table;

  /* negative color mapping table */
  COLOR_MAP negative_table;

  PALETTE old_pal, current_pal;

#define vc(x)  ((x)+224)                                  // offset to 'VGA color' x

#ifndef _MSC_VER
#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))
#endif

  /*******************************/
  /******* d_editbox_prox ********/
  /*******************************/
  void _find_char_in_textbox_line(char *thetext, int wword, int tabsize,
                                  int x, int y, int w, int h, int *line,
                                  int *firstchar, int *lastchar,
                                  int findchar);
  int prevuchar(char* thetext, int nextchar);
  int nextuchar(char* thetext, int prevchar);
  int othernr(int currentnr);
  int is_nr(int testnr);
  void hl_textout_ex(BITMAP *bmp, const FONT *f, const char *s,
                     int x, int y, int fg, int bg, int selfg,
                     int selbg, int selstart, int selend, int currchar);
  void _focus_on_cursor(int ypos, int *yofs, int list_height, int frame_height,
                        int xpos, int *xofs, int list_width, int frame_width);
  void _set_cursor_visibility(int *showcur, int *flags, clock_t *t, int visible);
  void _draw_scrollable_frame_vh(DIALOG *d,
                                 int list_height, int y_offset, int frame_height,
                                 int list_width, int x_offset, int frame_width,
                                 int vscroll_bar_style, int hscroll_bar_style,
                                 int fg_color, int bg);
  void _handle_scrollable_scroll_click_vh(DIALOG *d,
                                          int list_height, int *y_offset, int frame_height,
                                          int list_width, int *x_offset, int frame_width,
                                          int vscroll_bar_style, int hscroll_bar_style,
                                          int handle_what);
  void _draw_editbox(char *thetext, int *listsize, int draw, int x_offset,
                     int y_offset, int wword, int tabsize, int x, int y,
                     int w, int h, int disabled, int fore, int deselect,
                     int disable, int *maxwidth, int selstart, int selend,
                     int selfg, int selbg, int printtab, int printcrlf);
  int _crlf_width(int type, int style);
  int _draw_crlf(int x, int y, int type, int style,
                 int fg, int bg, int selfg, int selbg, int selstart,
                 int selend, int currchar);
  void _draw_tab(int x, int y, int tabstop, int printtab, int fg, int bg,
                 int selfg, int selbg, int selstart, int selend, int currchar);

  INLINE int is_in_rect(int x,int y,int rx1,int ry1,int rx2,int ry2)
  {
    return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
  }

  int eb_delete(char *s, int index, int w);

  /*
    INLINE int is_color_font(const FONT *fnt)
    {
    return fnt->vtable == font_vtable_color;
    }
    */

#ifdef __cplusplus
}
#endif
#endif

/***  The End  ***/
