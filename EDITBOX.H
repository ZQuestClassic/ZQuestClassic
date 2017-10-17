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

#ifndef _EDITBOX_H_
#define _EDITBOX_H_

#include "zc_alleg.h"
#include <string>
//#ifdef __cplusplus
//extern "C"
//{
//#endif

  enum {eb_wrap_none, eb_wrap_char, eb_wrap_word};
  enum {eb_crlf_n, eb_crlf_nr, eb_crlf_r, eb_crlf_rn, eb_crlf_any};
  enum {eb_scrollbar_optional, eb_scrollbar_on, eb_scrollbar_off};
  enum {eb_handle_vscroll, eb_handle_hscroll};

  int d_editbox_proc(int msg, DIALOG *d, int c);

  typedef struct editbox_data
  {
   // char **text;
	std::string text;
    int showcursor;
    int lines;
    int currtextline;
    int list_width;
    int currxpos;
    int fakexpos;
    int xofs;
    int yofs;
    int maxchars;
    int maxlines;
    int wrapping;
    int insertmode;
    int currchar;
    int tabdisplaystyle;
    int crlfdisplaystyle;
    int newcrlftype;
    int vsbarstyle;
    int hsbarstyle;
    FONT *font;
    //char *clipboard;
    //int clipboardsize;
	std::string clipboard;
    int defaulttabsize;
    int tabunits;
    int customtabs;
    int *customtabpos;
    int numchars;
    int selstart;
    int selend;
    int selfg;
    int selbg;
    int postpaste_dontmove;
  } editbox_data;

//#ifdef __cplusplus
//}
//#endif
#endif

/***  The End  ***/
 
