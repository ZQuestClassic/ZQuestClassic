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
// Snapshot the info layer and restore it later. The debugger uses these while paused
// so it can composite its transient sprite highlight without permanently erasing the
// walkability/hitbox cheat overlays drawn during the last game frame.
void save_info_bmp();
void restore_info_bmp();
void start_info_bmp();
void end_info_bmp();
void render_zc();

#endif
