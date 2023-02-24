#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "zc_alleg.h"
#include <vector>

class RenderTreeItemProps
{
public:
	int x, y;
	float scale = 1;
};

class RenderTreeItem
{
public:
	RenderTreeItemProps transform;
	RenderTreeItemProps computed;
	bool visible = true;
	// -1 for no transparency.
	int transparency_index = -1;
	ALLEGRO_BITMAP* bitmap = nullptr;
	BITMAP* a4_bitmap = nullptr;
	bool freeze_a4_bitmap_render = false;
	ALLEGRO_COLOR* tint = nullptr;
	std::vector<RenderTreeItem*> children;
	bool owned = false;
	
	~RenderTreeItem();
	
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

extern RenderTreeItem rti_dialogs;

void set_bitmap_create_flags(bool preserve_texture);
void clear_a5_bmp(ALLEGRO_BITMAP* bmp);
void render_tree_draw(RenderTreeItem* rti);

extern BITMAP* zqdialog_bg_bmp;
void popup_zqdialog_start(bool transp = true);
void popup_zqdialog_end();
ALLEGRO_BITMAP* add_dlg_layer();
void remove_dlg_layer(ALLEGRO_BITMAP* bmp);

#endif
