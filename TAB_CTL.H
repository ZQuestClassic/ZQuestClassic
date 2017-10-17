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

/* This code is not fully tested */

#ifndef _TAB_INTERN_H_
#define _TAB_INTERN_H_

#include "zc_alleg.h"


#ifdef __cplusplus
extern "C"
{
#endif
  int vc2(int x);
  /* a tab panel */
  typedef struct TABPANEL
  {
    char *text;                   /* menu item text */
                                  // AL_METHOD(int, proc, (void)); /* callback function */
                                  // struct MENU *child;           /* to allow nested menus */
    int flags;                    /* flags about the menu state */
    int *dialog;                  /* pointer to group of dialog objects that this tab contains */
    int objects;                  /* count of number of objects this tab handles; auto-calculated */
    int *xy;                      /* x and y coords of those dialog objects. */
  } TABPANEL;

  bool uses_tab_arrows(TABPANEL *panel, int maxwidth);
  int tab_count(TABPANEL *panel);
  int last_visible_tab(TABPANEL *panel, int first_tab, int maxwidth);
  int d_tab_proc(int msg, DIALOG *d, int c);
#ifdef __cplusplus
}
#endif
#endif

/***  The End  ***/
 
