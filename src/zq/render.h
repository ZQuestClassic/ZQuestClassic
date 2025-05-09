#ifndef ZQ_RENDER_TREE_H_
#define ZQ_RENDER_TREE_H_

#include "base/render.h"

RenderTreeItem* get_root_rti();
void set_center_root_rti(bool center);
RenderTreeItem* get_screen_rti();
void zq_hide_screen(bool hidden);
void zq_set_screen_never_freeze(bool value);
void render_zq();


#endif
