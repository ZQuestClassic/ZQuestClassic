#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "zc_alleg.h"
#include <vector>

class RenderTreeItemProps
{
public:
	int x, y;
	float scale;
};

class RenderTreeItem
{
public:
	RenderTreeItemProps transform;
	RenderTreeItemProps computed;
	bool visible;
	// -1 for no transparency.
	int transparency_index = -1;
	ALLEGRO_BITMAP* bitmap;
	BITMAP* a4_bitmap;
	std::vector<ALLEGRO_BITMAP*> overlays;
	bool freeze_a4_bitmap_render;
	ALLEGRO_COLOR* tint;
	std::vector<RenderTreeItem*> children;

	int global_to_local_x(int x)
	{
		return (x - computed.x) / computed.scale;
	}
	int global_to_local_y(int y)
	{
		return (y - computed.y) / computed.scale;
	}
	int local_to_global_x(int x)
	{
		return (x + computed.x) * computed.scale;
	}
	int local_to_global_y(int y)
	{
		return (y + computed.y) * computed.scale;
	}
};

void clear_a5_bmp(ALLEGRO_BITMAP* bmp);
void render_tree_draw(RenderTreeItem* rti);

#endif
