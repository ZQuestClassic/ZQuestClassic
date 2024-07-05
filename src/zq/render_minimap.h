#ifndef RENDER_MINIMAP_H_
#define RENDER_MINIMAP_H_

#include "base/render.h"

void mmap_mark_dirty();
void mmap_mark_dirty_delayed();
void mmap_set_zoom(bool zoomed);
void mmap_init();

class MiniMapRTI : public RenderTreeItem
{
public:
	MiniMapRTI();

private:
	void prepare();
	void render(bool bitmap_resized);
};

#endif
