#ifndef _ZQ_RENDER_TREE_H_
#define _ZQ_RENDER_TREE_H_

#include "base/render.h"

extern RenderTreeItem rti_overlay;
extern RenderTreeItem rti_tooltip_hl;
extern RenderTreeItem rti_scrborder;
extern RenderTreeItem rti_scrinfo;
extern RenderTreeItem rti_tooltip;

void zq_hide_screen(bool hidden);
void render_zq();


#endif
