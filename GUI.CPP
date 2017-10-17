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
#include "zquest.h"
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
#include "trapper_keeper.h" 

/****************************/
/**********  GUI  ***********/
/****************************/


// make it global so the joystick button routine can set joy_on=TRUE
DIALOG_PLAYER *player = NULL;

int zc_do_dialog(DIALOG *d, int f)
{
  int ret=do_zqdialog(d,f);
  position_mouse_z(0);
  return ret;
}

int zc_popup_dialog(DIALOG *d, int f)
{
  int ret=popup_zqdialog(d,f);
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

int zc_popup_dialog_dbuf(DIALOG *dialog, int focus_obj)
{
  BITMAP* buffer = create_bitmap_ex(get_color_depth(),SCREEN_H,SCREEN_W);
  blit(screen, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

  gui_set_screen(buffer);
  int ret=popup_dialog(dialog, focus_obj);
  gui_set_screen(NULL);

  blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
  position_mouse_z(0);
  return ret;
}

int PopUp_dialog(DIALOG *d,int f)
{
  // uses the bitmap that's already allocated
  go();
  player = init_dialog(d,f);

  while (update_dialog(player)) { /* do nothing */ }

  int ret = shutdown_dialog(player);
  comeback();
  position_mouse_z(0);
  return ret;
}

int popup_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj)
{
  //these are here to bypass compiler warnings about unused arguments
  buffer=buffer;

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

  ret = do_zqdialog(dialog, focus_obj);

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

  while (update_dialog_through_bitmap(buffer,player)) { /* do nothing */ }

  int ret = shutdown_dialog(player);
  comeback();
  position_mouse_z(0);
  return ret;
}

int update_dialog_through_bitmap (BITMAP* buffer, DIALOG_PLAYER *the_player)
{
  BITMAP* orig_screen = screen;
  int result;
  screen = buffer;
  result = update_dialog(the_player);
  screen = orig_screen;
  blit(buffer, screen, 0, 0, 0, 0, screen->w, screen->h);
  position_mouse_z(0);
  return result;
}

extern int zqwin_scale;

int do_zqdialog(DIALOG *dialog, int focus_obj)
{
  BITMAP *mouse_screen = _mouse_screen;
  BITMAP *gui_bmp = screen;
  int screen_count = _gfx_mode_set_count;
  DIALOG_PLAYER *player2;
  ASSERT(dialog);

  if (!is_same_bitmap(_mouse_screen, gui_bmp) && !(gfx_capabilities&GFX_HW_CURSOR))
  {
    show_mouse(gui_bmp);
  }

  player2 = init_dialog(dialog, focus_obj);

  while (update_dialog(player2))
  {
  /* If a menu is active, we yield here, since the dialog
  * engine is shut down so no user code can be running.
  */
    if(myvsync)
    {
      if(zqwin_scale > 1)
      {
        stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
      }
      else
      {
        blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
      }
      myvsync=0;
    }
    //if (active_menu_player2)
    //rest(1);
  }

  if (_gfx_mode_set_count == screen_count && !(gfx_capabilities&GFX_HW_CURSOR))
  {
    show_mouse(mouse_screen);
  }

  return shutdown_dialog(player2);
}



/* popup_dialog:
 *  Like do_dialog(), but it stores the data on the screen before drawing
 *  the dialog and restores it when the dialog is closed. The screen area
 *  to be stored is calculated from the dimensions of the first object in
 *  the dialog, so all the other objects should lie within this one.
 */
int popup_zqdialog(DIALOG *dialog, int focus_obj)
{
  BITMAP *bmp;
  BITMAP *gui_bmp;
  int ret;
  ASSERT(dialog);

  bmp = create_bitmap(dialog->w, dialog->h);
  gui_bmp = screen;
  if (bmp)
  {
    scare_mouse_area(dialog->x, dialog->y, dialog->w, dialog->h);
    blit(gui_bmp, bmp, dialog->x, dialog->y, 0, 0, dialog->w, dialog->h);
    unscare_mouse();
  }
  else
  {
    *allegro_errno = ENOMEM;
  }

  ret = do_zqdialog(dialog, focus_obj);

  if (bmp)
  {
    scare_mouse_area(dialog->x, dialog->y, dialog->w, dialog->h);
    blit(bmp, gui_bmp, 0, 0, dialog->x, dialog->y, dialog->w, dialog->h);
    unscare_mouse();
    destroy_bitmap(bmp);
  }

  return ret;
}

/*** end of gui.cpp ***/

