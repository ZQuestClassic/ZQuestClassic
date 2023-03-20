#include "render.h"

RenderTreeItem rti_dialogs;

extern int32_t zq_screen_w, zq_screen_h;
unsigned char info_opacity = 255;
bool use_linear_bitmaps();

void set_bitmap_create_flags(bool preserve_texture)
{
	int flags = ALLEGRO_CONVERT_BITMAP;
	if(!preserve_texture)
		flags |= ALLEGRO_NO_PRESERVE_TEXTURE;
	if (use_linear_bitmaps())
		flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	al_set_new_bitmap_flags(flags);
}

void clear_a5_bmp(ALLEGRO_BITMAP* bmp)
{
	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	
	al_set_target_bitmap(bmp);
	
	al_clear_to_color(al_map_rgba(0,0,0,0));
	
	al_restore_state(&old_state);
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

RenderTreeItem* add_dlg_layer()
{
	if(!active_dlg_rti) return nullptr;
	set_bitmap_create_flags(true);
	
	RenderTreeItem* rti = new RenderTreeItem();
	rti->bitmap = al_create_bitmap(screen->w, screen->h);
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

