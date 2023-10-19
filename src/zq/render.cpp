#include "zq/render.h"
#include "zconfig.h"
#include "base/gui.h"

extern int32_t prv_mode;
extern bool DragAspect;

static RenderTreeItem rti_root("root");
static RenderTreeItem rti_screen("screen");
static RenderTreeItem rti_mmap("mmap");
static RenderTreeItem rti_tooltip("tooltip");
static RenderTreeItem rti_tint("tint");

static int zc_gui_mouse_x()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().first;
	return rti_screen.rel_mouse().first;
}

static int zc_gui_mouse_y()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().second;
	return rti_screen.rel_mouse().second;
}

bool use_linear_bitmaps()
{
	return zc_get_config("zquest", "scaling_mode", 0) == 1;
}

static void init_render_tree()
{
	if (rti_root.has_children())
		return;
	
	set_bitmap_create_flags(false);
	rti_screen.bitmap = create_a5_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	rti_screen.visible = true;
	
	rti_tooltip.bitmap = create_a5_bitmap(screen->w, screen->h);
	rti_tooltip.a4_bitmap = create_bitmap_ex(8, screen->w, screen->h);
	rti_tooltip.transparency_index = 0;
	clear_bitmap(rti_tooltip.a4_bitmap);
	
	rti_tint.bitmap = create_a5_bitmap(screen->w, screen->h);
	rti_tint.visible = false;
	
	rti_screen.add_child(&rti_mmap);
	
	rti_root.add_child(&rti_screen);
	rti_root.add_child(&rti_tooltip);
	rti_root.add_child(&rti_tint);
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

	rti_root.visible = true;
	
	rti_mmap.visible = !prv_mode;
	
	{
		int w = al_get_bitmap_width(rti_screen.bitmap);
		int h = al_get_bitmap_height(rti_screen.bitmap);
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		if (scaling_force_integer)
		{
			xscale = std::max((int) xscale, 1);
			yscale = std::max((int) yscale, 1);
		}
		if(DragAspect)
			xscale = yscale = std::min(xscale,yscale);
		rti_screen.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		// TODO: don't recreate screen bitmap when alternating fullscreen mode.
		rti_screen.a4_bitmap = zqdialog_bg_bmp ? zqdialog_bg_bmp : screen;
		
		rti_tooltip.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		
		rti_dialogs.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		rti_tint.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
	}
	
	rti_dialogs.visible = rti_dialogs.has_children();
	rti_tint.visible = rti_dialogs.visible && !dlg_tint_paused();
	
	if(rti_dialogs.visible)
	{
		auto& tint = get_dlg_tint();
		if(!override_dlg_tint)
		{
			tint = al_premul_rgba(
				zc_get_config("ZQ_GUI","dlg_tint_r",0),
				zc_get_config("ZQ_GUI","dlg_tint_g",0),
				zc_get_config("ZQ_GUI","dlg_tint_b",0),
				zc_get_config("ZQ_GUI","dlg_tint_a",128)
			);
		}
		
		ALLEGRO_STATE oldstate;
		al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
		al_set_target_bitmap(rti_tint.bitmap);
		al_clear_to_color(tint);
		al_restore_state(&oldstate);
	}

	// Freeze dialogs that won't be changing.
	rti_screen.freeze = rti_dialogs.visible;
	rti_mmap.freeze = rti_dialogs.visible;
	int i = 0;
	while (i < rti_dialogs.get_children().size())
	{
		rti_dialogs.get_children()[i]->freeze = i != rti_dialogs.get_children().size() - 1;
		i++;
	}
	
	reload_dialog_tints();
}

RenderTreeItem* get_mmap_rti()
{
	return &rti_mmap;
}

RenderTreeItem* get_tooltip_rti()
{
	return &rti_tooltip;
}

void zq_hide_screen(bool hidden)
{
	rti_screen.visible = !hidden;
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
