#ifndef GUI_H_
#define GUI_H_

#include "base/zdefs.h"
#include "base/render.h"

void zc_set_gui_bmp(BITMAP* bmp);
BITMAP* zc_get_gui_bmp();
int do_zqdialog(DIALOG *dialog, int focus_obj, bool checkexit = false);
int do_zq_subdialog(DIALOG *dialog, int focus_obj, bool checkexit = false);
int do_zqdialog_custom(DIALOG *dialog, int focus_obj, bool checkexit, std::function<bool(int)> proc);
void new_gui_popup_dialog(DIALOG* dialog, int focus_obj, bool& done, bool& running);

#endif

