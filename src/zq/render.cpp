#include "render.h"
#include "zconfig.h"
#include "base/gui.h"

static RenderTreeItem rti_root;
static RenderTreeItem rti_screen;
static RenderTreeItem rti_mmap;
static RenderTreeItem rti_tooltip;

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

	if (zc_get_config("zquest", "scaling_mode", 0) == 1)
		al_set_new_bitmap_flags(base_flags | ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR);
	else
		al_set_new_bitmap_flags(base_flags);
	rti_screen.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	rti_mmap.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_mmap.a4_bitmap = nullptr;
	rti_tooltip.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_tooltip.a4_bitmap = create_bitmap_ex(8, screen->w, screen->h);
	rti_tooltip.transparency_index = 0;
	clear_bitmap(rti_tooltip.a4_bitmap);

	rti_root.children.push_back(&rti_screen);
	rti_root.children.push_back(&rti_mmap);
	rti_root.children.push_back(&rti_tooltip);

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
		rti_screen.a4_bitmap = screen;
		
		rti_mmap.transform.x = (resx - w*scale) / 2 / scale;
		rti_mmap.transform.y = (resy - h*scale) / 2 / scale;
		rti_mmap.transform.scale = scale;
		rti_mmap.visible = !dialog_open();
		
		rti_tooltip.transform.x = (resx - w*scale) / 2 / scale;
		rti_tooltip.transform.y = (resy - h*scale) / 2 / scale;
		rti_tooltip.transform.scale = scale;
		rti_tooltip.visible = !dialog_open();
	}
}

ALLEGRO_BITMAP* get_minimap_bmp()
{
	return rti_mmap.bitmap;
}

BITMAP* get_tooltip_bmp()
{
	return rti_tooltip.a4_bitmap;
}

void render_zq()
{
	init_render_tree();
	configure_render_tree();

	al_set_target_backbuffer(all_get_display());
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	render_tree_draw(&rti_root);

	al_flip_display();
}

