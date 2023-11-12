#ifndef _RENDER_TOOLTIP_H_
#define _RENDER_TOOLTIP_H_

#include "base/zdefs.h"
#include <string>

extern int ttip_global_id;

int ttip_register_id();
void ttip_install(int id, std::string text, size_and_pos trigger_area, int tip_x = -1, int tip_y = -1);
void ttip_install(int id, std::string text, int x, int y, int w, int h, int tip_x = -1, int tip_y = -1, int fw = -1, int fh = -1);
void ttip_uninstall(int id);
void ttip_uninstall_all();
void ttip_set_highlight_thickness(int id, int thickness);
void ttip_set_z_index(int id, int z_index);
void ttip_clear_timer();

#endif
