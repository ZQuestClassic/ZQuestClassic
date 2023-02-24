#include "render.h"
#include "zconfig.h"
#include "base/gui.h"

static RenderTreeItem rti_root;
static RenderTreeItem rti_screen;
static RenderTreeItem rti_tooltip;
static RenderTreeItem rti_dialogs;

static int zc_gui_mouse_x()
{
	return rti_screen.global_to_local_x(mouse_x);
}

static int zc_gui_mouse_y()
{
	return rti_screen.global_to_local_y(mouse_y);
}

static void init_render_tree()
{
	static const int base_flags = ALLEGRO_NO_PRESERVE_TEXTURE | ALLEGRO_CONVERT_BITMAP;

	if (!rti_root.children.empty())
		return;
	
	int lin_flags = 0;
	if (zc_get_config("zquest", "scaling_mode", 0) == 1)
		lin_flags = ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	
	al_set_new_bitmap_flags(base_flags | lin_flags);
	rti_screen.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	
	rti_tooltip.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_tooltip.a4_bitmap = create_bitmap_ex(8, screen->w, screen->h);
	rti_tooltip.transparency_index = 0;
	clear_bitmap(rti_tooltip.a4_bitmap);
	
	rti_dialogs.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_dialogs.a4_bitmap = create_bitmap_ex(8, screen->w, screen->h);
	rti_dialogs.transparency_index = 0;
	clear_bitmap(rti_dialogs.a4_bitmap);
	
	al_set_new_bitmap_flags(ALLEGRO_CONVERT_BITMAP | lin_flags);
	rti_screen.overlays.push_back(al_create_bitmap(screen->w, screen->h));
	
	rti_root.children.push_back(&rti_screen);
	rti_root.children.push_back(&rti_tooltip);
	rti_root.children.push_back(&rti_dialogs);

	gui_mouse_x = zc_gui_mouse_x;
	gui_mouse_y = zc_gui_mouse_y;

	al_set_new_bitmap_flags(0);
}

static void configure_render_tree()
{
	static bool scaling_force_integer = zc_get_config("zquest", "scaling_force_integer", 0) != 0;

	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());

	rti_root.transform.x = 0;
	rti_root.transform.y = 0;
	rti_root.transform.scale = 1;
	rti_root.visible = true;

	{
		int w = al_get_bitmap_width(rti_screen.bitmap);
		int h = al_get_bitmap_height(rti_screen.bitmap);
		float scale = std::min((float)resx/w, (float)resy/h);
		if (scaling_force_integer)
			scale = std::max((int) scale, 1);
		rti_screen.transform.x = (resx - w*scale) / 2 / scale;
		rti_screen.transform.y = (resy - h*scale) / 2 / scale;
		rti_screen.transform.scale = scale;
		rti_screen.visible = true;
		// TODO: don't recreate screen bitmap when alternating fullscreen mode.
		rti_screen.a4_bitmap = zqdialog_bg_bmp ? zqdialog_bg_bmp : screen;
		
		rti_tooltip.transform.x = (resx - w*scale) / 2 / scale;
		rti_tooltip.transform.y = (resy - h*scale) / 2 / scale;
		rti_tooltip.transform.scale = scale;
		rti_tooltip.visible = zqdialog_tmp_bmps.empty();
		
		rti_dialogs.transform.x = (resx - w*scale) / 2 / scale;
		rti_dialogs.transform.y = (resy - h*scale) / 2 / scale;
		rti_dialogs.transform.scale = scale;
		rti_dialogs.visible = true;
	}
}

ALLEGRO_BITMAP* get_overlay_bmp()
{
	return rti_screen.overlays.empty() ? nullptr : rti_screen.overlays[0];
}
ALLEGRO_BITMAP* add_dlg_overlay()
{
	int lin_flags = 0;
	if (zc_get_config("zquest", "scaling_mode", 0) == 1)
		lin_flags = ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR;
	al_set_new_bitmap_flags(ALLEGRO_CONVERT_BITMAP | lin_flags);
	
	ALLEGRO_BITMAP* targ = al_create_bitmap(screen->w, screen->h);
	rti_dialogs.overlays.push_back(targ);
	return targ;
}
void remove_dlg_overlay(ALLEGRO_BITMAP* bmp)
{
	auto& vec = rti_dialogs.overlays;
	for(auto it = vec.begin(); it != vec.end();)
	{
		if(*it == bmp)
		{
			it = vec.erase(it);
		}
		else ++it;
	}
	al_destroy_bitmap(bmp);
}

BITMAP* get_tooltip_bmp()
{
	return rti_tooltip.a4_bitmap;
}

void render_zq()
{
	BITMAP* tmp = screen;
	if(zqdialog_bg_bmp)
		screen = zqdialog_bg_bmp;
	
	init_render_tree();
	configure_render_tree();
	
	zqdialog_render(rti_dialogs.a4_bitmap);
	
	al_set_target_backbuffer(all_get_display());
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	
	render_tree_draw(&rti_root);

	al_flip_display();
	
	screen = tmp;
}

