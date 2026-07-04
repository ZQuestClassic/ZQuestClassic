#ifndef ZC_RENDER_TREE_H_
#define ZC_RENDER_TREE_H_

#include "zalleg/render.h"

extern RenderTreeItem rti_root;
extern LegacyBitmapRTI rti_game;
extern RenderTreeItem rti_infolayer;
extern LegacyBitmapRTI rti_menu;
extern LegacyBitmapRTI rti_gui;
extern LegacyBitmapRTI rti_screen;

// False on the web build, where the info layer is disabled for performance.
// Anything drawing overlays between start_info_bmp/end_info_bmp must skip the
// whole block when this is false (see render.cpp).
bool info_bmp_enabled();
void clear_info_bmp();
void start_info_bmp();
void end_info_bmp();
void render_zc();

#endif
