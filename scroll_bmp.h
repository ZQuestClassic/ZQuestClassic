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

#ifndef _EB_INTERN_H_
#define _EB_INTERN_H_

#ifdef __cplusplus
extern "C"
{
  #endif

  #include "zc_alleg.h"
  #include <allegro/internal/aintern.h>

  enum {scrollbar_optional, scrollbar_on, scrollbar_off};
  enum {handle_vscroll, handle_hscroll};

  int d_scroll_bmp_proc(int msg, DIALOG *d, int c);
}
#endif
#endif

/***  The End  ***/
