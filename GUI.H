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

#include "zdefs.h"

extern DIALOG_PLAYER *player;

int  zc_do_dialog(DIALOG *d, int f);
int  zc_popup_dialog(DIALOG *d, int f);
int  PopUp_dialog(DIALOG *d,int f);
int  update_dialog_through_bitmap (BITMAP* buffer, DIALOG_PLAYER *player);
int  do_dialog_through_bitmap(BITMAP *buffer, DIALOG *dialog, int focus_obj);
int  popup_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int f);
int  zc_popup_dialog_dbuf(DIALOG* , int);
int  PopUp_dialog_through_bitmap(BITMAP *buffer,DIALOG *d,int f);
//int  popup_zqdialog(DIALOG *dialog, int focus_obj);
//int  do_zqdialog(DIALOG *dialog, int focus_obj);
#endif                                                      // _GUI_H_
 
