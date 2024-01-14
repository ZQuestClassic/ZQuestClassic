#include "zc/render.h"
#include "base/render.h"
#include "zc/zelda.h"
#include "zc/maps.h"
#include "sprite.h"
#include "zc/guys.h"
#include "qst.h"
#include "iter.h"
#include "base/gui.h"
#include "base/zapp.h"
#include "base/mapscr.h"
#include <fmt/format.h>

extern sprite_list guys;
extern double aspect_ratio;
extern byte use_save_indicator;

RenderTreeItem rti_root("root");
LegacyBitmapRTI rti_game("game");
RenderTreeItem rti_infolayer("info");
LegacyBitmapRTI rti_menu("menu");
LegacyBitmapRTI rti_gui("gui");
LegacyBitmapRTI rti_screen("screen");

bool use_linear_bitmaps()
{
	static bool value = zc_get_config("zeldadx", "scaling_mode", 0) == 1;
	return value;
}

static int zc_gui_mouse_x()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().first;
	if (rti_dialogs.visible || rti_gui.visible)
		return rti_gui.rel_mouse().first;
	if (rti_menu.visible)
		return rti_menu.rel_mouse().first;
	return rti_game.rel_mouse().first;
}

static int zc_gui_mouse_y()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().second;
	if (rti_dialogs.visible || rti_gui.visible)
		return rti_gui.rel_mouse().second;
	if (rti_menu.visible)
		return rti_menu.rel_mouse().second;
	return rti_game.rel_mouse().second;
}

int window_mouse_x()
{
	return rti_game.rel_mouse().first;
}
int window_mouse_y()
{
	return rti_game.rel_mouse().second;
}

static void init_render_tree()
{
	static const int base_flags_preserve_texture = ALLEGRO_CONVERT_BITMAP;
	static const int base_flags = ALLEGRO_NO_PRESERVE_TEXTURE | base_flags_preserve_texture;

	if (!rti_root.get_children().empty())
		return;

	// ALLEGRO_NO_PRESERVE_TEXTURE is not included for rti_game because on Windows that results in
	// the bitmap being cleared when losing focus. Since we sometimes don't always draw to this
	// every frame (when it is frozen under a pause menu), we need to pay the cost to keep the texture
	// backed up.
	al_set_new_bitmap_flags(base_flags_preserve_texture);
	rti_game.bitmap = create_a5_bitmap(framebuf->w, framebuf->h);
	rti_game.set_size(framebuf->w, framebuf->h);
	rti_game.a4_bitmap = framebuf;
	rti_infolayer.bitmap = create_a5_bitmap(framebuf->w, framebuf->h);
	rti_infolayer.set_size(framebuf->w, framebuf->h);

	al_set_new_bitmap_flags(base_flags);
	rti_menu.bitmap = create_a5_bitmap(menu_bmp->w, menu_bmp->h);
	rti_menu.set_size(menu_bmp->w, menu_bmp->h);
	rti_menu.a4_bitmap = menu_bmp;
	rti_menu.transparency_index = 0;

	gui_bmp = create_bitmap_ex(8, 640, 480);
	clear_bitmap(gui_bmp);
	zc_set_gui_bmp(gui_bmp);
	al_set_new_bitmap_flags(base_flags);
	rti_gui.bitmap = create_a5_bitmap(gui_bmp->w, gui_bmp->h);
	rti_gui.set_size(gui_bmp->w, gui_bmp->h);
	rti_gui.a4_bitmap = gui_bmp;
	rti_gui.transparency_index = 0;

	al_set_new_bitmap_flags(base_flags);
	rti_screen.bitmap = create_a5_bitmap(screen->w, screen->h);
	rti_screen.set_size(screen->w, screen->h);
	rti_screen.a4_bitmap = zqdialog_bg_bmp ? zqdialog_bg_bmp : screen;
	rti_screen.transparency_index = 0;
	
	rti_root.add_child(&rti_game);
	rti_root.add_child(&rti_infolayer);
	rti_root.add_child(&rti_menu);
	rti_root.add_child(&rti_gui);
	rti_root.add_child(&rti_screen);
	rti_root.add_child(&rti_dialogs);

	gui_mouse_x = zc_gui_mouse_x;
	gui_mouse_y = zc_gui_mouse_y;

	al_set_new_bitmap_flags(0);
	
	_init_render(al_get_bitmap_format(rti_screen.bitmap));
}

float intscale(float scale)
{
	return std::max(1,int(scale));
}
static void configure_render_tree()
{
	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());
	
	int w = rti_game.width;
	int h = rti_game.height;
	float xscale = (float)resx/w;
	float yscale = (float)resy/(h+12);
	float scale = std::min(xscale, yscale);
	if (scaleForceInteger)
		scale = intscale(scale);

	rti_game.set_transform({
		.x = (int)(resx - w*scale) / 2,
		.y = (int)(resy - h*scale) / 2,
		.xscale = scale,
		.yscale = scale,
	});
	rti_game.visible = true;

	rti_infolayer.set_transform({
		.x = (int)(resx - w*scale) / 2,
		.y = (int)(resy - h*scale) / 2,
		.xscale = scale,
		.yscale = scale,
	});
	rti_infolayer.visible = true;
	
	rti_dialogs.visible = rti_dialogs.has_children();
	rti_gui.visible = (dialog_count >= 1 && !active_dialog) || dialog_count >= 2 || screen == gui_bmp;
	
	if (rti_dialogs.visible || rti_gui.visible)
	{
		int w = rti_gui.width;
		int h = rti_gui.height;
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		rti_gui.set_transform({
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
	}
	
	bool has_zqdialog = false;
	auto& dlgs = rti_dialogs.get_children();
	for(auto it = dlgs.rbegin(); it != dlgs.rend(); ++it)
	{
		auto rti = *it;
		if(rti->type == RTI_TY_DIALOG_A4 || rti->type == RTI_TY_DIALOG_A5)
			has_zqdialog = true;
		else if(rti->type == RTI_TY_POPUP_MENU)
			rti->visible = !has_zqdialog;
	}
	rti_menu.visible = MenuOpen && !has_zqdialog;
	
	if (rti_menu.visible)
	{
		int w = rti_menu.width;
		int h = rti_menu.height;
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		rti_menu.set_transform({
			.x = 0,
			.y = 0,
			.xscale = xscale,
			.yscale = yscale,
		});
	}
	
	rti_screen.visible = false;

	if (rti_screen.visible)
	{
		int w = rti_screen.width;
		int h = rti_screen.height;
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		if (scaleForceInteger)
		{
			xscale = intscale(xscale);
			yscale = intscale(yscale);
		}

		rti_screen.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		// TODO: don't recreate screen bitmap when alternating fullscreen mode.
		rti_screen.a4_bitmap = zqdialog_bg_bmp ? zqdialog_bg_bmp : screen;
	}

	rti_game.freeze = rti_menu.visible || rti_gui.visible || rti_dialogs.visible || is_sys_pal;
	if (rti_game.freeze)
	{
		static ALLEGRO_COLOR tint = al_premul_rgba_f(0.4, 0.4, 0.8, 0.8);
		rti_game.tint = &tint;
		rti_infolayer.tint = &tint;
		// TODO: renderer should tint children somehow.
		for (auto it : rti_game.get_children())
			it->tint = &tint;
		for (auto it : rti_infolayer.get_children())
			it->tint = &tint;
	}
	else
	{
		rti_game.tint = nullptr;
		rti_infolayer.tint = nullptr;
		for (auto it : rti_game.get_children())
			it->tint = nullptr;
		for (auto it : rti_infolayer.get_children())
			it->tint = nullptr;
	}
	reload_dialog_tint();
}

static ALLEGRO_STATE infobmp_old_state;
void start_info_bmp()
{
	al_store_state(&infobmp_old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rti_infolayer.bitmap);
	al_set_clipping_rectangle(0, playing_field_offset, al_get_bitmap_width(rti_infolayer.bitmap), al_get_bitmap_height(rti_infolayer.bitmap)-playing_field_offset);
}
void end_info_bmp()
{
	al_set_clipping_rectangle(0, 0, al_get_bitmap_width(rti_infolayer.bitmap), al_get_bitmap_height(rti_infolayer.bitmap));
	al_restore_state(&infobmp_old_state);
}

void render_zc()
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

	ALLEGRO_FONT* a5font = get_zc_font_a5(font_lfont_l);//al_create_builtin_font();
	static int font_scale = 3;

	std::vector<std::string> lines_left;
	std::vector<std::string> lines_right;

	// TODO calculate fps without using a timer thread.
	if (ShowFPS)
		lines_left.push_back(fmt::format("FPS: {}", (int)lastfps));
	if (replay_is_replaying())
		lines_left.push_back(replay_get_buttons_string().c_str());
	else if (replay_is_recording() && MenuOpen)
		lines_left.push_back(fmt::format("Recording, frame {}", replay_get_frame()));
	if (Paused)
		lines_right.push_back("PAUSED");
	if (Saving && use_save_indicator)
		lines_right.push_back("SAVING ...");
	if (details && game)
	{
		lines_right.push_back(fmt::format("dlvl:{:2} dngn:{}", dlevel, isdungeon()));
		lines_right.push_back(time_str_long(game->get_time()));
		for (int i = 0; i < guys.Count(); i++)
			lines_right.push_back(fmt::format("{}", (int)((enemy*)guys.spr(i))->id));
	}
	if (show_ff_scripts)
	{
		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			if (ffc_handle.ffc->script)
				lines_right.push_back(ffcmap[ffc_handle.ffc->script-1].scriptname);
		});
	}

	ALLEGRO_BITMAP* bitmap = al_get_backbuffer(all_get_display());
	render_text_lines(bitmap, a5font, lines_left, TextJustify::left, TextAlign::bottom, font_scale);
	render_text_lines(bitmap, a5font, lines_right, TextJustify::right, TextAlign::bottom, font_scale);

	if (render_get_debug())
		render_tree_draw_debug(&rti_root);

    al_flip_display();
	
	screen = tmp;
	al_restore_state(&oldstate);
}

void clear_tooltip()
{
	
}
void ttip_uninstall_all()
{
	
}

