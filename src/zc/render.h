#ifndef _ZC_RENDER_TREE_H_
#define _ZC_RENDER_TREE_H_

#include "base/render.h"

extern RenderTreeItem rti_root;
extern LegacyBitmapRTI rti_game;
extern RenderTreeItem rti_infolayer;
extern LegacyBitmapRTI rti_menu;
extern LegacyBitmapRTI rti_gui;
extern LegacyBitmapRTI rti_screen;

void start_info_bmp();
void end_info_bmp();
void render_zc();

#endif
