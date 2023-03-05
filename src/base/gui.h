//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_sys.h
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

#ifndef _GUI_H_
#define _GUI_H_

#include "base/zdefs.h"
#include "base/render.h"

extern DIALOG_PLAYER *player;

int new_popup_dlg(DIALOG* dialog, int32_t focus_obj);
int do_zqdialog(DIALOG *dialog, int focus_obj);
void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running);

int popup_menu(MENU *menu,int x,int y);
int popup_menu_abs(MENU *menu,int x,int y);
#endif                                                      // _GUI_H_

