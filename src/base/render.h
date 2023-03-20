#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "zc_alleg.h"
#include <vector>
extern unsigned char info_opacity;

class RenderTreeItemProps
{
public:
	int x, y;
	float xscale = 1;
	float yscale = 1;
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
		return (x - computed.x) / computed.xscale;
	}
	int global_to_local_y(int y)
	{
		return (y - computed.y) / computed.yscale;
	}
	int local_to_global_x(int x)
	{
		return (x + computed.x) * computed.xscale;
	}
	int local_to_global_y(int y)
	{
		return (y + computed.y) * computed.yscale;
	}
};

namespace MouseSprite
{
	#define MAX_MOUSESPRITE 35
	void assign(int index, BITMAP* spr, int xf = 1, int yf = 1);
	void set(int index);
};

extern RenderTreeItem rti_dialogs;

void set_bitmap_create_flags(bool preserve_texture);
void clear_a5_bmp(ALLEGRO_BITMAP* bmp);
void render_tree_draw(RenderTreeItem* rti);

extern BITMAP* zqdialog_bg_bmp;
void popup_zqdialog_start();
void popup_zqdialog_end();
void popup_zqdialog_start_a5();
void popup_zqdialog_end_a5();
void update_dialog_transform();
RenderTreeItem* add_dlg_layer();
void remove_dlg_layer(RenderTreeItem* rti);

#endif
