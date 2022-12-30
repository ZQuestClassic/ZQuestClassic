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

extern DIALOG_PLAYER *player;

void     zc_set_gui_bmp(BITMAP* bmp);
BITMAP*  zc_get_gui_bmp();
int32_t  zc_do_dialog(DIALOG *d, int32_t f);
int32_t  zc_popup_dialog(DIALOG *d, int32_t f);
int32_t  PopUp_dialog(DIALOG *d,int32_t f);
int32_t  update_dialog_through_bitmap(BITMAP* buffer, DIALOG_PLAYER *player);
int32_t  do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int32_t focus_obj);
int32_t  popup_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int32_t f);
int32_t  zc_popup_dialog_dbuf(DIALOG* , int32_t);
int32_t  PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int32_t f);
void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running);

void popup_zqdialog_start();
void popup_zqdialog_draw();
void popup_zqdialog_end();
#endif                                                      // _GUI_H_

