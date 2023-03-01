#include "render.h"
#include "util.h"
#include <fmt/format.h>

RenderTreeItem rti_dialogs;

extern int32_t zq_screen_w, zq_screen_h;
bool use_linear_bitmaps();
unsigned char info_opacity = 255;
static int freezecount = 0;
void freeze_render()
{
	++freezecount;
}
void unfreeze_render()
{
	if(freezecount > 0)
		--freezecount;
}
bool render_frozen()
{
	return freezecount > 0;
}


void set_bitmap_create_flags(bool preserve_texture)
{
	int flags = ALLEGRO_CONVERT_BITMAP;
	if(!preserve_texture)
		flags |= ALLEGRO_NO_PRESERVE_TEXTURE;
	if (use_linear_bitmaps())
		flags |= ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	al_set_new_bitmap_flags(flags);
}

ALLEGRO_COLOR a5color(RGB c, unsigned char alpha)
{
	return al_map_rgba(c.r*4,c.g*4,c.b*4,alpha);
}
ALLEGRO_COLOR a5color(int index, unsigned char alpha)
{
	RGB tmp;
	get_color(index,&tmp);
	return a5color(tmp,alpha);
}

void clear_a5_bmp(ALLEGRO_COLOR col, ALLEGRO_BITMAP* bmp)
{
	if(bmp && bmp != al_get_target_bitmap())
	{
		ALLEGRO_STATE old_state;
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		
		al_set_target_bitmap(bmp);
		
		al_clear_to_color(col);
		
		al_restore_state(&old_state);
	}
	else
	{
		al_clear_to_color(col);
	}
}

void collide_clip_rect(int& x, int& y, int& w, int& h)
{
	int ox,oy,ow,oh;
	al_get_clipping_rectangle(&ox,&oy,&ow,&oh);
	if(x >= ox+ow || y >= oy+oh || x+w <= ox || y+h <= oy)
	{
		x=y=w=h=0;
		return;
	}
	if(x < ox)
	{
		w -= ox-x;
		x = ox;
	}
	if(y < oy)
	{
		h -= oy-y;
		y = oy;
	}
	if(y+h > oy+oh)
		h = (oy+oh)-y;
	if(x+w > ox+ow)
		w = (ox+ow)-x;
	
	if(w <= 0 || h <= 0)
		x=y=w=h=0;
}

void clear_a5_clip_rect(ALLEGRO_BITMAP* bmp)
{
	if(bmp && bmp != al_get_target_bitmap())
	{
		ALLEGRO_STATE old_state;
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		
		al_set_target_bitmap(bmp);
		
		al_set_clipping_rectangle(0, 0, al_get_bitmap_width(bmp)-1, al_get_bitmap_height(bmp)-1);
		
		al_restore_state(&old_state);
	}
	else
	{
		bmp = al_get_target_bitmap();
		al_set_clipping_rectangle(0, 0, al_get_bitmap_width(bmp)-1, al_get_bitmap_height(bmp)-1);
	}
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


BITMAP* zqdialog_bg_bmp = nullptr;
static RenderTreeItem* active_dlg_rti = nullptr;
static RenderTreeItem* active_a5_dlg_rti = nullptr;
static RenderTreeItem* active_a4_dlg_rti = nullptr;

void save_debug_bitmaps(char const* pref)
{
	std::string path = fmt::format("tmp/{}",pref?pref:"");
	util::create_path(path.c_str());
	PALETTE tpal;
	get_palette(tpal);
	if(active_a4_dlg_rti)
	{
		save_bitmap(fmt::format("{}/a4.bmp",path).c_str(),active_a4_dlg_rti->a4_bitmap,tpal);
		al_save_bitmap(fmt::format("{}/a4_render.bmp",path).c_str(),active_a4_dlg_rti->bitmap);
	}
	if(active_a5_dlg_rti)
	{
		al_save_bitmap(fmt::format("{}/a5.bmp",path).c_str(),active_a5_dlg_rti->bitmap);
	}
	if(active_dlg_rti&&active_dlg_rti->children.size())
	{
		int q = 1;
		for(RenderTreeItem* rti : active_dlg_rti->children)
		{
			if(rti->a4_bitmap)
			{
				save_bitmap(fmt::format("{}/layer_{}_a4.bmp",path,q).c_str(),rti->a4_bitmap,tpal);
				al_save_bitmap(fmt::format("{}/layer_{}_a4_render.bmp",path,q).c_str(),rti->bitmap);
			}
			else if(rti->bitmap)
			{
				al_save_bitmap(fmt::format("{}/layer_{}_a5.bmp",path,q).c_str(),rti->bitmap);
			}
			++q;
		}
	}
}

static void pop_active_rti()
{
	if(active_dlg_rti == active_a4_dlg_rti)
		active_a4_dlg_rti = nullptr;
	if(active_dlg_rti == active_a5_dlg_rti)
		active_a5_dlg_rti = nullptr;
	active_dlg_rti = nullptr;
	rti_dialogs.children.pop_back();
	
	if(rti_dialogs.children.size())
	{
		active_dlg_rti = rti_dialogs.children.back();
		if(active_dlg_rti->a4_bitmap)
			active_a4_dlg_rti = active_dlg_rti;
		else active_a5_dlg_rti = active_dlg_rti;
		
		if(!active_a5_dlg_rti && rti_dialogs.children.size() > 1)
		{
			auto* rti = rti_dialogs.children[rti_dialogs.children.size()-2];
			if(!rti->a4_bitmap)
				active_a5_dlg_rti = rti;
		}
	}
}
int get_zqdialog_a4_clear_color()
{
	if(active_a4_dlg_rti)
		return active_dlg_rti->clear_color();
	return 0;
}
void clear_zqdialog_a4()
{
	if(active_a4_dlg_rti)
		clear_to_color(active_a4_dlg_rti->a4_bitmap, active_a4_dlg_rti->clear_color());
}

void get_zqdialog_offset(int&x, int&y, int&w, int&h)
{
	if(active_dlg_rti)
	{
		x = active_dlg_rti->transform.x;
		y = active_dlg_rti->transform.y;
		if(active_dlg_rti->bitmap)
		{
			w = al_get_bitmap_width(active_dlg_rti->bitmap);
			h = al_get_bitmap_height(active_dlg_rti->bitmap);
		}
	}
	else
	{
		x=0;
		y=0;
		w=zq_screen_w;
		h=zq_screen_h;
	}
}

void popup_zqdialog_start(int x, int y, int w, int h, int transp)
{
	if(w < 0) w = zq_screen_w;
	if(h < 0) h = zq_screen_h;
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	BITMAP* tmp_bmp = create_bitmap_ex(8, w, h);
	
	if(tmp_bmp)
	{
		if(transp > 0)
			clear_to_color(tmp_bmp, transp);
		else clear_bitmap(tmp_bmp);
		show_mouse(tmp_bmp);
		screen = tmp_bmp;
		
		RenderTreeItem* rti = new RenderTreeItem();
		set_bitmap_create_flags(false);
		rti->bitmap = al_create_bitmap(w, h);
		rti->a4_bitmap = tmp_bmp;
		rti->owned = true;
		rti->transparency_index = transp;
		rti->transform.x = x;
		rti->transform.y = y;
		rti_dialogs.children.push_back(rti);
		rti_dialogs.visible = true;
		active_dlg_rti = active_a4_dlg_rti = rti;
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
		show_mouse(NULL);
		
		RenderTreeItem* to_del = active_dlg_rti;
		
		pop_active_rti();
		
		screen = active_a4_dlg_rti ? active_a4_dlg_rti->a4_bitmap : zqdialog_bg_bmp;
		if(rti_dialogs.children.empty())
			zqdialog_bg_bmp = nullptr;
		
		show_mouse(screen);
		delete to_del;
	}
	position_mouse_z(0);
}

void popup_zqdialog_blackout(int x, int y, int w, int h, int c)
{
	if(w < 0) w = zq_screen_w;
	if(h < 0) h = zq_screen_h;
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	
	RenderTreeItem* rti = new RenderTreeItem();
	set_bitmap_create_flags(true);
	rti->bitmap = al_create_bitmap(w, h);
	rti->owned = true;
	rti->transform.x = x;
	rti->transform.y = y;
	rti_dialogs.children.push_back(rti);
	rti_dialogs.visible = true;
	al_set_new_bitmap_flags(0);
	
	clear_a5_bmp(a5color(c), rti->bitmap);
	
	popup_zqdialog_start(x,y,w,h);
}

void popup_zqdialog_blackout_end()
{
	if (rti_dialogs.children.size() >= 2)
	{
		show_mouse(NULL);
		
		RenderTreeItem* to_del = active_dlg_rti;
		pop_active_rti();
		RenderTreeItem* to_del_2 = active_dlg_rti;
		pop_active_rti();
		
		screen = active_a4_dlg_rti ? active_a4_dlg_rti->a4_bitmap : zqdialog_bg_bmp;
		if(rti_dialogs.children.empty())
			zqdialog_bg_bmp = nullptr;
		
		show_mouse(screen);
		delete to_del;
		delete to_del_2;
	}
	position_mouse_z(0);
}

static std::vector<ALLEGRO_STATE> old_a5_states;
void popup_zqdialog_start_a5(int x, int y, int w, int h)
{
	if(w < 0) w = zq_screen_w;
	if(h < 0) h = zq_screen_h;
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	
	RenderTreeItem* rti = new RenderTreeItem();
	set_bitmap_create_flags(true);
	rti->bitmap = al_create_bitmap(w, h);
	rti->owned = true;
	rti->transform.x = x;
	rti->transform.y = y;
	rti_dialogs.children.push_back(rti);
	rti_dialogs.visible = true;
	active_dlg_rti = active_a5_dlg_rti = rti;
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
		
		pop_active_rti();
		
		screen = active_a4_dlg_rti ? active_a4_dlg_rti->a4_bitmap : zqdialog_bg_bmp;
		if(rti_dialogs.children.empty())
			zqdialog_bg_bmp = nullptr;
		
		ALLEGRO_STATE& oldstate = old_a5_states.back();
		al_restore_state(&oldstate);
		old_a5_states.pop_back();

		delete to_del;
	}
	position_mouse_z(0);
}

RenderTreeItem* popup_zqdialog_a5_child(int x, int y, int w, int h)
{
	if(!active_a5_dlg_rti) return nullptr;
	
	RenderTreeItem* rti = new RenderTreeItem();
	set_bitmap_create_flags(true);
	rti->bitmap = al_create_bitmap(w, h);
	rti->owned = true;
	rti->transform.x = x;
	rti->transform.y = y;
	active_a5_dlg_rti->children.push_back(rti);
	al_set_new_bitmap_flags(0);
	
	//Set the bitmap's transformation
	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	
	al_set_target_bitmap(rti->bitmap);
	
	ALLEGRO_TRANSFORM T;
	al_identity_transform(&T);
	al_translate_transform(&T, -x, -y);
	al_use_transform(&T);
	
	al_restore_state(&old_state);
	
	return rti;
}

bool a4_bmp_active()
{
	return active_dlg_rti && active_dlg_rti->a4_bitmap;
}

RenderTreeItem* add_dlg_layer()
{
	if(!active_dlg_rti) return nullptr;
	set_bitmap_create_flags(true);
	
	RenderTreeItem* rti = new RenderTreeItem();
	rti->bitmap = al_create_bitmap(screen->w, screen->h);
	rti->a4_bitmap = nullptr;
	rti->owned = true;
	active_dlg_rti->children.push_back(rti);

	al_set_new_bitmap_flags(0);
	return rti;
}
RenderTreeItem* add_dlg_layer_a4(int transp)
{
	if(!active_dlg_rti) return nullptr;
	set_bitmap_create_flags(true);
	
	RenderTreeItem* rti = new RenderTreeItem();
	rti->bitmap = al_create_bitmap(screen->w, screen->h);
	rti->a4_bitmap = create_bitmap_ex(8,screen->w,screen->h);
	rti->transparency_index = transp;
	if(transp > 0)
		clear_to_color(rti->a4_bitmap,transp);
	else clear_bitmap(rti->a4_bitmap);
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

