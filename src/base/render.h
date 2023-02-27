#ifndef _BASE_RENDER_TREE_H_
#define _BASE_RENDER_TREE_H_

#include "zdefs.h"
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

extern RenderTreeItem rti_dialogs;

void set_bitmap_create_flags(bool preserve_texture);
ALLEGRO_COLOR a5color(RGB c);
ALLEGRO_COLOR a5color(int index);
void clear_a5_bmp(ALLEGRO_BITMAP* bmp = nullptr, ALLEGRO_COLOR* c = nullptr);
void clear_a5_clip_rect(ALLEGRO_BITMAP* bmp = nullptr);
void render_tree_draw(RenderTreeItem* rti);

extern BITMAP* zqdialog_bg_bmp;
int get_zqdialog_a4_clear_color();
void clear_zqdialog_a4();
void popup_zqdialog_start(int x = 0, int y = 0, int w = -1, int h = -1, int transp = 0);
void popup_zqdialog_end();
void popup_zqdialog_blackout(int x = 0, int y = 0, int w = -1, int h = -1, int c = 0);
void popup_zqdialog_blackout_end();
void popup_zqdialog_start_a5(int x = 0, int y = 0, int w = -1, int h = -1);
void popup_zqdialog_end_a5();
void update_dialog_transform();
RenderTreeItem* add_dlg_layer();
void remove_dlg_layer(RenderTreeItem* rti);

#endif
