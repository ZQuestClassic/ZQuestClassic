#include "render.h"
#include "zdefs.h"
#include "jwin_a5.h"

RenderTreeItem rti_dialogs;

extern int32_t zq_screen_w, zq_screen_h;
unsigned char info_opacity = 255;
bool use_linear_bitmaps();
ALLEGRO_COLOR AL5_INVIS = al_map_rgba(0,0,0,0),
	AL5_BLACK = al_map_rgb(0,0,0),
	AL5_WHITE = al_map_rgb(255,255,255),
	AL5_YELLOW = al_map_rgb(255,255,0),
	AL5_PINK = al_map_rgb(255,0,255),
	AL5_DGRAY = al_map_rgb(85,85,85),
	AL5_LGRAY = al_map_rgb(170,170,170),
	AL5_BLUE = al_map_rgb(85,85,255),
	AL5_LRED = al_map_rgb(255,85,85),
	AL5_DRED = al_map_rgb(178,36,36),
	AL5_LGREEN = al_map_rgb(85,255,85),
	AL5_LAQUA = al_map_rgb(85,255,255);
void set_bitmap_create_flags(bool preserve_texture)
{
	int flags = ALLEGRO_CONVERT_BITMAP;
	if(!preserve_texture)
		flags |= ALLEGRO_NO_PRESERVE_TEXTURE;
	if (use_linear_bitmaps())
		flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	al_set_new_bitmap_flags(flags);
}

void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp)
{
	if(bmp)
	{
		ALLEGRO_STATE old_state;
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		
		al_set_target_bitmap(bmp);
		
		al_clear_to_color(col);
		
		al_restore_state(&old_state);
	}
	else al_clear_to_color(col);
}
void clear_a5_bmp(ALLEGRO_BITMAP* bmp)
{
	clear_a5_bmp(AL5_INVIS,bmp);
}

RenderTreeItem::~RenderTreeItem()
{
	if(owned)
	{
		if(bitmap)
			al_destroy_bitmap(bitmap);
		if(a4_bitmap)
			destroy_bitmap(a4_bitmap);
		if(tint)
			delete tint;
	}
	for(RenderTreeItem* child : children)
	{
		if(owned || child->owned)
		{
			delete child;
		}
	}
}

ALLEGRO_COLOR a5colors[256];
uint32_t zc_backend_palette[256];
static int backend_fmt = ALLEGRO_PIXEL_FORMAT_ABGR_8888;
void _init_render(int fmt)
{
	backend_fmt = fmt;
}
uint32_t get_backend_a5_col(RGB const& c)
{
	unsigned char r = c.r*4, g = c.g*4, b = c.b*4, a = 255;
	switch(backend_fmt)
	{
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888: default:
			return r | (g << 8) | (b << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
			return r | (g << 8) | (b << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_ARGB_8888:
			return b | (g << 8) | (r << 16) | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_RGBA_8888:
			return a | (b << 8) | (g << 16) | (r << 24);
	}
}
uint32_t repl_a5_backend_alpha(uint32_t back_col, unsigned char a)
{
	switch(backend_fmt)
	{
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888: default:
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
		case ALLEGRO_PIXEL_FORMAT_ARGB_8888:
			back_col &= ~(0xFF << 24);
			return back_col | (a << 24);
		case ALLEGRO_PIXEL_FORMAT_RGBA_8888:
			back_col &= ~(0xFF);
			return a | back_col;
	}
}
void load_palette(uint32_t* backpal, ALLEGRO_COLOR* backcols, PALETTE pal, int start, int end)
{
	if(start>end) zc_swap(start,end);
	for(int q = start; q <= end; ++q)
	{
		if(backcols) backcols[q] = a5color(pal[q]);
		if(backpal) backpal[q] = get_backend_a5_col(pal[q]);
	}
}

void zc_set_palette(PALETTE pal)
{
	load_palette(zc_backend_palette, a5colors, pal);
	set_palette(pal);
}
void zc_set_palette_range(PALETTE pal, int start, int end, bool)
{
	load_palette(zc_backend_palette, a5colors, pal, start, end);
	set_palette_range(pal,start,end,false);
}

void render_a4_a5(BITMAP* src,int sx,int sy,int dx,int dy,int w,int h,int maskind,uint32_t* backpal)
{
	if(!backpal) backpal = zc_backend_palette;
	ALLEGRO_BITMAP* buf = al_create_bitmap(w,h);
	ALLEGRO_LOCKED_REGION * lr;
	uint8_t * line_8;
	uint32_t * line_32;
	int x, y;

	lr = al_lock_bitmap(buf, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	if(lr)
	{
		line_8 = (uint8_t*)lr->data;
		line_32 = (uint32_t*)lr->data;
		for(y = 0; y < h; y++)
		{
			if(y+sy >= src->h) break;
			for(x = 0; x < w; x++)
			{
				if(x+sx >= src->w) break;
				int index = src->line[y+sy][x+sx];
				if (index == maskind)
					line_32[x] = 0;
				else
					line_32[x] = backpal[index];
			}
			line_8 += lr->pitch;
			line_32 = (uint32_t *)line_8;
		}
		al_unlock_bitmap(buf);
	}
	else
	{
		al_destroy_bitmap(buf);
		return;
	}
	
	al_draw_bitmap(buf, dx, dy, 0);
}

static void render_tree_layout(RenderTreeItem* rti, RenderTreeItem* rti_parent)
{
	if (!rti_parent)
	{
		rti->computed.xscale = rti->transform.xscale;
		rti->computed.yscale = rti->transform.yscale;
		rti->computed.x = rti->transform.xscale * rti->transform.x;
		rti->computed.y = rti->transform.yscale * rti->transform.y;
	}
	else
	{
		rti->computed.xscale = rti->transform.xscale * rti_parent->computed.xscale;
		rti->computed.yscale = rti->transform.yscale * rti_parent->computed.yscale;
		rti->computed.x = rti->computed.xscale * rti->transform.x + rti_parent->transform.x;
		rti->computed.y = rti->computed.yscale * rti->transform.y + rti_parent->transform.y;
	}
	
	for (auto rti_child : rti->children)
	{
		render_tree_layout(rti_child, rti);
	}
}

static void render_tree_draw_item(RenderTreeItem* rti)
{
	if (!rti->visible)
		return;

	if (rti->bitmap)
	{
		if (rti->a4_bitmap && !rti->freeze_a4_bitmap_render)
		{
			all_set_transparent_palette_index(rti->transparency_index);
			all_render_a5_bitmap(rti->a4_bitmap, rti->bitmap);
		}

		int w = al_get_bitmap_width(rti->bitmap);
		int h = al_get_bitmap_height(rti->bitmap);
		
		if (rti->tint)
		{
			al_draw_tinted_scaled_bitmap(rti->bitmap, *rti->tint, 0, 0, w, h, rti->computed.x, rti->computed.y, w*rti->computed.xscale, h*rti->computed.yscale, 0);
		}
		else
		{
			al_draw_scaled_bitmap(rti->bitmap, 0, 0, w, h, rti->computed.x, rti->computed.y, w*rti->computed.xscale, h*rti->computed.yscale, 0);
		}
	}
		
	for (auto rti_child : rti->children)
	{
		render_tree_draw_item(rti_child);
	}
}

void render_tree_draw(RenderTreeItem* rti)
{
	render_tree_layout(rti, nullptr);
	render_tree_draw_item(rti);
}

namespace MouseSprite
{
	static int active_mouse_sprite = -1;
	ALLEGRO_MOUSE_CURSOR* zc_mouse_sprites[MAX_MOUSESPRITE] = {nullptr};
	ALLEGRO_MOUSE_CURSOR* nullmouse = nullptr;
	static void set_nullmouse()
	{
		if(!nullmouse)
		{
			static char mouse_arrow_data[16*16] =
			{
				2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				2, 1, 2, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 2, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0
			};
			ALLEGRO_COLOR white = al_map_rgb(255,255,255);
			ALLEGRO_COLOR black = al_map_rgb(0,0,0);
			ALLEGRO_BITMAP* bmp = al_create_bitmap(16,16);
			auto* lock = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
			for(int y = 0; y < 16; ++y)
			{
				for(int x = 0; x < 16; ++x)
				{
					ALLEGRO_COLOR* col = nullptr;
					switch(mouse_arrow_data[x+y*16])
					{
						case 1: col = &white; break;
						case 2: col = &black; break;
					}
					if(col)
						al_put_pixel(x, y, *col);
				}
			}
			al_unlock_bitmap(bmp);
			nullmouse = al_create_mouse_cursor(bmp, 1, 1);
			al_destroy_bitmap(bmp);
		}
		active_mouse_sprite = -1;
		al_show_mouse_cursor(all_get_display());
		al_set_mouse_cursor(all_get_display(), nullmouse);
	}
	void assign(int index, BITMAP* spr, int xf, int yf)
	{
		ALLEGRO_MOUSE_CURSOR* old_cursor = zc_mouse_sprites[index];
		all_set_transparent_palette_index(0);
		ALLEGRO_BITMAP* a5_mouse_sprite = all_get_a5_bitmap(spr);
		zc_mouse_sprites[index] = al_create_mouse_cursor(a5_mouse_sprite, xf, yf);
		al_destroy_bitmap(a5_mouse_sprite);
		
		if(index == active_mouse_sprite)
			set(index);
		
		if(old_cursor)
			al_destroy_mouse_cursor(old_cursor);
	}
	void set(int index)
	{
		if(index < 0 || index >= MAX_MOUSESPRITE)
		{
			active_mouse_sprite = -1;
			set_nullmouse();
		}
		if(index == active_mouse_sprite) return;
		if(zc_mouse_sprites[index])
		{
			active_mouse_sprite = index;
			al_show_mouse_cursor(all_get_display());
			al_set_mouse_cursor(all_get_display(), zc_mouse_sprites[index]);
		}
	}
}

BITMAP* zqdialog_bg_bmp = nullptr;
static RenderTreeItem* active_dlg_rti = nullptr;
void popup_zqdialog_start()
{
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	BITMAP* tmp_bmp = create_bitmap_ex(8, zq_screen_w, zq_screen_h);
	
	if(tmp_bmp)
	{
		clear_bitmap(tmp_bmp);
		screen = tmp_bmp;
		
		RenderTreeItem* rti = new RenderTreeItem();
		set_bitmap_create_flags(false);
		rti->bitmap = al_create_bitmap(zq_screen_w, zq_screen_h);
		rti->a4_bitmap = tmp_bmp;
		rti->transparency_index = 0xFF;
		clear_to_color(tmp_bmp,0xFF);
		rti->visible = true;
		rti->owned = true;
		rti_dialogs.children.push_back(rti);
		rti_dialogs.visible = true;
		active_dlg_rti = rti;
		al_set_new_bitmap_flags(0);
	}
	else
	{
		*allegro_errno = ENOMEM;
	}
}

void popup_zqdialog_end()
{
	if (active_dlg_rti)
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.children.pop_back();
		if(rti_dialogs.children.size())
		{
			active_dlg_rti = rti_dialogs.children.back();
			screen = active_dlg_rti->a4_bitmap;
		}
		else
		{
			active_dlg_rti = nullptr;
			screen = zqdialog_bg_bmp;
			zqdialog_bg_bmp = nullptr;
		}
		delete to_del;
	}
	position_mouse_z(0);
}

static std::vector<ALLEGRO_STATE> old_a5_states;
void popup_zqdialog_start_a5()
{
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	
	RenderTreeItem* rti = new RenderTreeItem();
	set_bitmap_create_flags(true);
	rti->bitmap = al_create_bitmap(zq_screen_w, zq_screen_h);
	rti->visible = true;
	rti->owned = true;
	rti_dialogs.children.push_back(rti);
	rti_dialogs.visible = true;
	active_dlg_rti = rti;
	al_set_new_bitmap_flags(0);
	
	old_a5_states.emplace_back();
	ALLEGRO_STATE& oldstate = old_a5_states.back();
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rti->bitmap);
}

void popup_zqdialog_end_a5()
{
	if (active_dlg_rti && old_a5_states.size())
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.children.pop_back();
		if(rti_dialogs.children.size())
			active_dlg_rti = rti_dialogs.children.back();
		else
		{
			active_dlg_rti = nullptr;
			zqdialog_bg_bmp = nullptr;
		}
		ALLEGRO_STATE& oldstate = old_a5_states.back();
		al_restore_state(&oldstate);
		old_a5_states.pop_back();

		delete to_del;
	}
	position_mouse_z(0);
}

void update_dialog_transform(){}

RenderTreeItem* add_dlg_layer(int x, int y, int w, int h)
{
	if(!active_dlg_rti) return nullptr;
	if(w<0) w = screen->w-x;
	if(h<0) h = screen->h-y;
	set_bitmap_create_flags(true);
	
	RenderTreeItem* rti = new RenderTreeItem();
	rti->bitmap = al_create_bitmap(w,h);
	rti->transform.x = x;
	rti->transform.y = y;
	rti->a4_bitmap = nullptr;
	rti->visible = true;
	rti->owned = true;
	active_dlg_rti->children.push_back(rti);

	al_set_new_bitmap_flags(0);
	return rti;
}
void remove_dlg_layer(RenderTreeItem* rti)
{
	if(active_dlg_rti) //Remove from children vector
	{
		auto& vec = active_dlg_rti->children;
		for(auto it = vec.begin(); it != vec.end();)
		{
			RenderTreeItem* child = *it;
			if(child == rti)
			{
				it = vec.erase(it);
			}
			else ++it;
		}
	}
	delete rti;
}

