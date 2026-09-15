#include "render.h"
#include "a5alleg.h"
#include "allegro5/bitmap.h"
#include "allegro5/display.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "core/fonts.h"
#include <fmt/format.h>
#include "gui/jwin_a5.h"
#include <chrono>
#include <map>

using namespace std::chrono_literals;

RenderTreeItem rti_dialogs("dialogs");
static auto rti_tint = RenderTreeItem("tint");

extern int32_t zq_screen_w, zq_screen_h;

BITMAP* zqdialog_bg_bmp = nullptr;
static RenderTreeItem* active_dlg_rti = nullptr;
void zqdialog_set_skiptint(bool skipTint)
{
	if(active_dlg_rti)
		active_dlg_rti->skip_tint = skipTint;
}
void zqdialog_name(string const& name)
{
	if(active_dlg_rti)
		active_dlg_rti->name = name;
}
void zqdialog_freeze(bool frozen)
{
	if(active_dlg_rti)
		active_dlg_rti->freeze = frozen;
}
void get_zqdialog_xy(int& x, int& y)
{
	if(active_dlg_rti)
	{
		x = active_dlg_rti->get_transform().x;
		y = active_dlg_rti->get_transform().y;
	}
	else x = y = 0;
}

static vector<std::function<void()>> on_close_procs;
void on_zqdialog_close(std::function<void()>&& proc)
{
	on_close_procs.emplace_back(std::move(proc));
}
void on_zqdialog_close()
{
	vector<std::function<void()>> procs;
	procs.swap(on_close_procs);
	for(auto& proc : procs)
		proc();
}

static RenderTreeItem* get_active_dialog(bool forTint = false)
{
	auto& children = rti_dialogs.get_children();
	for (auto it = children.rbegin(); it != children.rend(); it++)
	{
		auto child = *it;
		if(forTint && child->skip_tint)
			continue;
		if(child != &rti_tint)
			return child;
	}
	return nullptr;
}
void popup_zqdialog_start(string name, RenderTreeItemType type, int x, int y, int w, int h, int transp)
{
	if (is_headless())
		return;

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
		screen = tmp_bmp;
		
		LegacyBitmapRTI* rti = new LegacyBitmapRTI(name);
		rti->type = type;
		rti->set_size(w, h);
		set_bitmap_create_flags(true);
		rti->bitmap = create_a5_bitmap(w, h);
		al_set_new_bitmap_flags(0);
		rti->a4_bitmap = tmp_bmp;
		rti->transparency_index = transp;
		rti->set_transform({(float)x, (float)y});
		rti->owned = true;
		rti_dialogs.add_child(rti);
		rti_dialogs.visible = true;
		active_dlg_rti = rti;
		
		clear_tooltip();
	}
	else
	{
		*allegro_errno = ENOMEM;
	}
}
void popup_zqdialog_start(int x, int y, int w, int h, int transp)
{
	popup_zqdialog_start("zqdialog", RenderTreeItemType::dialog_a4, x, y, w, h, transp);
}
void popup_zqdialog_end()
{
	if (is_headless())
		return;

	bool closed = false;
	if (active_dlg_rti)
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.remove_child(to_del);
		active_dlg_rti = get_active_dialog();
		if (active_dlg_rti)
		{
			auto rti = dynamic_cast<LegacyBitmapRTI*>(active_dlg_rti);
			screen = rti ? rti->a4_bitmap : nullptr;
		}
		else
		{
			screen = zqdialog_bg_bmp;
			zqdialog_bg_bmp = nullptr;
			closed = true;
		}
		delete to_del;
	}
	position_mouse_z(0);
	clear_tooltip();
	if(closed)
		on_zqdialog_close();
}

static std::vector<ALLEGRO_STATE> old_a5_states;
void popup_zqdialog_start_a5()
{
	if(!zqdialog_bg_bmp)
		zqdialog_bg_bmp = screen;
	
	auto rti = new RenderTreeItem("zqdialog_a5");
	rti->type = RenderTreeItemType::dialog_a5;
	rti->set_size(zq_screen_w, zq_screen_h);
	set_bitmap_create_flags(true);
	rti->bitmap = create_a5_bitmap(zq_screen_w, zq_screen_h);
	al_set_new_bitmap_flags(0);
	rti->owned = true;
	rti_dialogs.add_child(rti);
	rti_dialogs.visible = true;
	active_dlg_rti = rti;
	
	clear_tooltip();
	
	old_a5_states.emplace_back();
	ALLEGRO_STATE& oldstate = old_a5_states.back();
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rti->bitmap);
}

void popup_zqdialog_end_a5()
{
	bool closed = false;
	if (active_dlg_rti && old_a5_states.size())
	{
		RenderTreeItem* to_del = active_dlg_rti;
		rti_dialogs.remove_child(to_del);
		active_dlg_rti = get_active_dialog();
		if (!active_dlg_rti)
		{
			zqdialog_bg_bmp = nullptr;
			closed = true;
		}
		ALLEGRO_STATE& oldstate = old_a5_states.back();
		al_restore_state(&oldstate);
		old_a5_states.pop_back();

		delete to_del;
	}
	position_mouse_z(0);
	clear_tooltip();
	if(closed)
		on_zqdialog_close();
}

RenderTreeItem* add_dlg_layer(int x, int y, int w, int h)
{
	if(w<0) w = (screen ? screen->w : zq_screen_w)-x;
	if(h<0) h = (screen ? screen->h : zq_screen_h)-y;
	set_bitmap_create_flags(true);

	RenderTreeItem* rti = new RenderTreeItem("dlg");
	rti->bitmap = al_create_bitmap(w,h);
	rti->set_size(w, h);
	clear_a5_bmp(rti->bitmap);
	rti->set_transform({.x = (float)x, .y = (float)y});
	rti->visible = true;
	rti->owned = true;
	// Not dirty: the draw pass clears and re-renders a dirty item's bitmap, which would
	// erase content that imperative callers have already drawn into it. render_cb users
	// mark dirty themselves whenever their source data changes.
	rti->dirty = false;
	if(active_dlg_rti)
		active_dlg_rti->add_child(rti);

	al_set_new_bitmap_flags(0);
	return rti;
}
void remove_dlg_layer(RenderTreeItem* rti)
{
	if(!rti) return;

	rti->remove();
	delete rti;
}

ALLEGRO_COLOR dialog_tint = al_premul_rgba(0, 0, 0, 64);
ALLEGRO_COLOR* override_dlg_tint = nullptr;
static size_t dlg_tint_pause = 0;

// Place a tinted bitmap before the active dialog render item.
void reload_dialog_tint()
{
	auto& children = rti_dialogs.get_children();
	if (children.empty())
		return;

	auto& tint = get_dlg_tint();
	if (!override_dlg_tint)
	{
		// Nothing changes these config values at runtime (runtime tint changes go
		// through override_dlg_tint), and this runs every frame - so read them once.
		static ALLEGRO_COLOR config_tint = al_premul_rgba(
			zc_get_config("ZQ_GUI","dlg_tint_r",0),
			zc_get_config("ZQ_GUI","dlg_tint_g",0),
			zc_get_config("ZQ_GUI","dlg_tint_b",0),
			zc_get_config("ZQ_GUI","dlg_tint_a",128)
		);
		tint = config_tint;
	}
	rti_tint.tint = &tint;
	if (!rti_tint.bitmap)
	{
		rti_tint.set_size(screen->w, screen->h);
		set_bitmap_create_flags(true);
		rti_tint.bitmap = create_a5_bitmap(screen->w, screen->h);
		ALLEGRO_STATE oldstate;
		al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
		al_set_target_bitmap(rti_tint.bitmap);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		al_restore_state(&oldstate);
		rti_tint.freeze = true;
		rti_tint.dirty = false;
	}

	auto next_dialog_rti = get_active_dialog(true);
	if (next_dialog_rti)
		rti_dialogs.add_child_before(&rti_tint, next_dialog_rti);
	else
		rti_tint.remove();

	rti_tint.visible = !dlg_tint_paused();
}
RenderTreeItem* get_dlg_tint_rti()
{
	return &rti_tint;
}
ALLEGRO_COLOR& get_dlg_tint()
{
	return override_dlg_tint ? *override_dlg_tint : dialog_tint;
}
void pause_dlg_tint(bool pause)
{
	if(pause)
		++dlg_tint_pause;
	else if(dlg_tint_pause)
		--dlg_tint_pause;
}
bool dlg_tint_paused()
{
	return dlg_tint_pause;
}
