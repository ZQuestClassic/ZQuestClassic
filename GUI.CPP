//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  gui.c
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <dir.h>
#include <ctype.h>
#include "zc_alleg.h"

#ifdef ALLEGRO_DOS
#include <unistd.h>
#endif

#include "zdefs.h"
#include "zelda.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "zsys.h"
#include "qst.h"
#include "zc_sys.h"
#include "debug.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "gui.h"

/****************************/
/**********  GUI  ***********/
/****************************/

// make it global so the joystick button routine can set joy_on=TRUE
DIALOG_PLAYER *player;

int zc_do_dialog(DIALOG *d, int f)
{
  int ret=do_dialog(d,f);
  position_mouse_z(0);
  return ret;
}

int zc_popup_dialog(DIALOG *d, int f)
{
  int ret=popup_dialog(d,f);
  position_mouse_z(0);
  return ret;
}

int do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj)
{
  BITMAP* orig_screen = screen;
  screen = buffer;

  int ret=do_dialog(dialog, focus_obj);

  screen = orig_screen;
  blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);

  position_mouse_z(0);
  return ret;
}

int PopUp_dialog(DIALOG *d,int f)
{
  // uses the bitmap that's already allocated
  go();
  player = init_dialog(d,f);

  while (update_dialog(player));

  int ret = shutdown_dialog(player);
  comeback();
  position_mouse_z(0);
  return ret;
}

int popup_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj)
{
  BITMAP *bmp;
  int ret;

  bmp = create_bitmap_ex(bitmap_color_depth(screen),dialog->w+1, dialog->h+1);

  if (bmp)
  {
    scare_mouse();
    blit(screen, bmp, dialog->x, dialog->y, 0, 0, dialog->w+1, dialog->h+1);
    unscare_mouse();
  }
  else
    *allegro_errno = ENOMEM;

  ret = do_dialog(dialog, focus_obj);

  if (bmp)
  {
    scare_mouse();
    blit(bmp, screen, 0, 0, dialog->x, dialog->y, dialog->w+1, dialog->h+1);
    unscare_mouse();
    destroy_bitmap(bmp);
  }

  position_mouse_z(0);
  return ret;
}

int PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int f)
{
  // uses the bitmap that's already allocated
  go();
  player = init_dialog(d,f);

  while (update_dialog_through_bitmap(buffer,player));

  int ret = shutdown_dialog(player);
  comeback();
  position_mouse_z(0);
  return ret;
}

int update_dialog_through_bitmap (BITMAP* buffer, DIALOG_PLAYER *player)
{
  BITMAP* orig_screen = screen;
  int result;
  screen = buffer;
  result = update_dialog(player);
  screen = orig_screen;
  blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
  return result;
}

/*** end of gui.cpp ***/
