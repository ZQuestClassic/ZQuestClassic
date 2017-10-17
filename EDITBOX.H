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
#ifdef __cplusplus
extern "C"
{
  #endif

  int d_editbox_proc(int msg, DIALOG *d, int c);

  typedef struct textline
  {
    textline *prev;
    char     *text;
    textline *next;
    void     *extra;
  } textline;

  typedef struct textline_extra
  {
    int maxlen;
  } textline_extra;

  typedef struct editbox_data
  {
    textline *text_data;
    int       showcursor;
    int       lines;
    int       currtextline;
    int       list_width;
    int       currxpos;
    int       fakexpos;
    int       xofs;
    int       yofs;
    int       maxchars;
    int       maxlines;
    int       wrapping;
    int       insertmode;
    int       currchar;
    int       tabdisplaystyle;
    int       crlfdisplaystyle;
    int       newcrlftype;
    int       vsbarstyle;
    int       hsbarstyle;
    FONT      *font;
    char      *clipboard;
    int       clipboardsize;
    int       defaulttabsize;
    int       tabunits;
    int       customtabs;
    int      *customtabpos;
    int       numchars;
    int       selstart;
    int       selend;
    int       selfg;
    int       selbg;
    int       postpaste_dontmove;
  } editbox_data;

  #ifdef __cplusplus
}
#endif
#endif

/***  The End  ***/
