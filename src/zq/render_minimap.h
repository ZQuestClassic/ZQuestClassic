#pragma once

#include "zalleg/render.h"

void mmap_mark_active();
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
