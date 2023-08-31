//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_sys.h
//
//  System functions, input handlers, GUI stuff, etc.
//  for ZQuest Classic.
//
//--------------------------------------------------------

#ifndef _GUI_H_
#define _GUI_H_

#include "base/zdefs.h"
#include "base/render.h"

extern DIALOG_PLAYER *player;

void     zc_set_gui_bmp(BITMAP* bmp);
BITMAP*  zc_get_gui_bmp();
int32_t do_zqdialog(DIALOG *dialog, int32_t focus_obj, bool checkexit = false);
int32_t do_zqdialog_custom(DIALOG *dialog, int32_t focus_obj, bool checkexit, std::function<bool(int)> proc);
void new_gui_popup_dialog(DIALOG* dialog, int32_t focus_obj, bool& done, bool& running);
#endif                                                      // _GUI_H_

