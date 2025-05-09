#include "zq/render.h"
#include "zq/render_tooltip.h"
#include "base/render.h"
#include "zconfig.h"
#include "base/gui.h"

extern int32_t prv_mode;
extern bool DragAspect;

static auto rti_root = RenderTreeItem("root");
static auto rti_screen = LegacyBitmapRTI("screen");
static bool screen_never_freeze;
static bool center_root_rti = true;

RenderTreeItem& gui_mouse_target()
{
	if (rti_dialogs.has_children())
		return *rti_dialogs.get_children().back();
	return rti_screen;
}

static int zc_gui_mouse_x()
{
	return gui_mouse_target().rel_mouse().first;
}

static int zc_gui_mouse_y()
{
	return gui_mouse_target().rel_mouse().second;
}

int window_mouse_x()
{
	return rti_screen.rel_mouse().first;
}
int window_mouse_y()
{
	return rti_screen.rel_mouse().second;
}

bool use_linear_bitmaps()
{
	static bool value = zc_get_config("zquest", "scaling_mode", 0) == 1;
	return value;
}

static void init_render_tree()
{
	if (rti_root.has_children())
		return;

	set_bitmap_create_flags(true);
	rti_screen.set_size(screen->w, screen->h);
	rti_screen.bitmap = create_a5_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	rti_screen.visible = true;

	rti_root.add_child(&rti_screen);
	rti_root.add_child(&rti_dialogs);

	gui_mouse_x = zc_gui_mouse_x;
	gui_mouse_y = zc_gui_mouse_y;

	al_set_new_bitmap_flags(0);

	_init_render(al_get_bitmap_format(rti_screen.bitmap));
}

static void configure_render_tree()
{
	static bool scaling_force_integer = zc_get_config("zquest", "scaling_force_integer", 0) != 0;

	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());

	{
		int w = rti_screen.width;
		int h = rti_screen.height;
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		if (scaling_force_integer)
		{
			xscale = std::max((int) xscale, 1);
			yscale = std::max((int) yscale, 1);
		}
		if(DragAspect)
			xscale = yscale = std::min(xscale,yscale);
		rti_root.set_transform({
			.x = center_root_rti ? (int)(resx - w*xscale) / 2 : 0,
			.y = center_root_rti ? (int)(resy - h*yscale) / 2 : 0,
			.xscale = xscale,
			.yscale = yscale,
		});

		// TODO: don't recreate screen bitmap when alternating fullscreen mode.
		rti_screen.a4_bitmap = zqdialog_bg_bmp ? zqdialog_bg_bmp : screen;
	}

	// Not necessary, but a few things use `rti_dialogs.visible` for convenience.
	rti_dialogs.visible = rti_dialogs.has_children();

	// Freeze dialogs that won't be changing.
	if (screen_never_freeze)
		rti_screen.freeze = false;
	else
		rti_screen.freeze = rti_dialogs.has_children();
	int i = 0;
	while (i < rti_dialogs.get_children().size())
	{
		rti_dialogs.get_children()[i]->freeze = i != rti_dialogs.get_children().size() - 1;
		i++;
	}

	reload_dialog_tint();
}

RenderTreeItem* get_root_rti()
{
	return &rti_root;
}

void set_center_root_rti(bool center)
{
	center_root_rti = center;
}

RenderTreeItem* get_screen_rti()
{
	return &rti_screen;
}

void zq_hide_screen(bool hidden)
{
	rti_screen.visible = !hidden;
}

void zq_set_screen_never_freeze(bool value)
{
	screen_never_freeze = value;
}

void render_zq()
{
	if (is_headless())
		return;

	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	
	BITMAP* tmp = screen;
	if(zqdialog_bg_bmp)
		screen = zqdialog_bg_bmp;
	
	init_render_tree();
	configure_render_tree();
	
	al_set_target_backbuffer(all_get_display());
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	
	render_tree_draw(&rti_root);
	if (render_get_debug())
		render_tree_draw_debug(&rti_root);

	al_flip_display();
	
	screen = tmp;
	al_restore_state(&oldstate);
}

void clear_tooltip()
{
	ttip_uninstall_all();
}

