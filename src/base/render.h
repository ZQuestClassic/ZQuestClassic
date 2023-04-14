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
	bool set(int index);
	void clear(int index);
};

extern RenderTreeItem rti_dialogs;
extern ALLEGRO_COLOR AL5_INVIS,AL5_BLACK,AL5_WHITE,AL5_YELLOW,
	AL5_PINK,AL5_DGRAY,AL5_LGRAY,AL5_BLUE,AL5_LRED,AL5_DRED,
	AL5_LGREEN,AL5_LAQUA;
void set_bitmap_create_flags(bool preserve_texture);
void clear_a5_bmp(ALLEGRO_BITMAP* bmp = nullptr);
void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp = nullptr);
ALLEGRO_BITMAP* create_a5_bitmap(int w, int h);
void render_tree_draw(RenderTreeItem* rti);

void _init_render(int fmt);
uint32_t get_backend_a5_col(RGB const& c);
uint32_t repl_a5_backend_alpha(uint32_t back_col, unsigned char a);
void load_palette(uint32_t* backpal, ALLEGRO_COLOR* backcols, PALETTE pal, int start = 0, int end = 255);
void zc_set_palette(PALETTE pal);
void zc_set_palette_range(PALETTE pal, int start, int end, bool=false);
void render_a4_a5(BITMAP* src,int sx,int sy,int dx,int dy,int w,int h,int maskind = 0,uint32_t* backpal = nullptr);

extern BITMAP* zqdialog_bg_bmp;
void popup_zqdialog_start();
void popup_zqdialog_end();
void popup_zqdialog_start_a5();
void popup_zqdialog_end_a5();
void update_dialog_transform();
RenderTreeItem* add_dlg_layer(int x = 0, int y = 0, int w = -1, int h = -1);
void remove_dlg_layer(RenderTreeItem* rti);

#endif
