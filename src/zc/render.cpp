#include "render.h"
#include "zelda.h"
#include "maps.h"
#include "sprite.h"
#include "guys.h"
#include "qst.h"
#include "base/gui.h"
#include "base/zapp.h"
#include <fmt/format.h>

extern sprite_list guys;

RenderTreeItem rti_root;
RenderTreeItem rti_game;
RenderTreeItem rti_menu;
RenderTreeItem rti_gui;
RenderTreeItem rti_screen;

static int zc_gui_mouse_x()
{
	if (rti_gui.visible)
	{
		return rti_gui.global_to_local_x(mouse_x);
	}
	else if (rti_menu.visible)
	{
		return rti_menu.global_to_local_x(mouse_x);
	}
	else
	{
		return rti_game.global_to_local_x(mouse_x);
	}
}

static int zc_gui_mouse_y()
{
	if (rti_gui.visible)
	{
		return rti_gui.global_to_local_y(mouse_y);
	}
	else if (rti_menu.visible)
	{
		return rti_menu.global_to_local_y(mouse_y);
	}
	else
	{
		return rti_game.global_to_local_y(mouse_y);
	}
}

static void init_render_tree()
{
	static const int base_flags_preserve_texture = ALLEGRO_CONVERT_BITMAP;
	static const int base_flags = ALLEGRO_NO_PRESERVE_TEXTURE | base_flags_preserve_texture;

	if (!rti_root.children.empty())
		return;

	// ALLEGRO_NO_PRESERVE_TEXTURE is not included for rti_game because on Windows that results in
	// the bitmap being cleared when losing focus. Since we sometimes don't always draw to this
	// every frame (when it is frozen under a pause menu), we need to pay the cost to keep the texture
	// backed up.
	if (zc_get_config("zeldadx", "scaling_mode", 0) == 1)
		al_set_new_bitmap_flags(base_flags_preserve_texture | ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR);
	else
		al_set_new_bitmap_flags(base_flags_preserve_texture);
	rti_game.bitmap = al_create_bitmap(framebuf->w, framebuf->h);
	rti_game.a4_bitmap = framebuf;

	al_set_new_bitmap_flags(base_flags);
	rti_menu.bitmap = al_create_bitmap(menu_bmp->w, menu_bmp->h);
	rti_menu.a4_bitmap = menu_bmp;
	rti_menu.transparency_index = 0;

	gui_bmp = create_bitmap_ex(8, 640, 480);
	zc_set_gui_bmp(gui_bmp);
	al_set_new_bitmap_flags(base_flags);
	rti_gui.bitmap = al_create_bitmap(gui_bmp->w, gui_bmp->h);
	rti_gui.a4_bitmap = gui_bmp;
	rti_gui.transparency_index = 0;

	al_set_new_bitmap_flags(base_flags);
	rti_screen.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	rti_screen.transparency_index = 0;

	rti_root.children.push_back(&rti_game);
	rti_root.children.push_back(&rti_menu);
	rti_root.children.push_back(&rti_gui);
	rti_root.children.push_back(&rti_screen);

	gui_mouse_x = zc_gui_mouse_x;
	gui_mouse_y = zc_gui_mouse_y;

	al_set_new_bitmap_flags(0);
}

static void configure_render_tree()
{
	static bool scaling_force_integer = zc_get_config("zeldadx", "scaling_force_integer", 1);

	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());

	rti_root.transform.x = 0;
	rti_root.transform.y = 0;
	rti_root.transform.scale = 1;
	rti_root.visible = true;

	{
		int w = al_get_bitmap_width(rti_game.bitmap);
		int h = al_get_bitmap_height(rti_game.bitmap);
		float scale = std::min((float)resx/w, (float)resy/h);
		if (scaling_force_integer)
			scale = std::max((int) scale, 1);
		rti_game.transform.x = (resx - w*scale) / 2 / scale;
		rti_game.transform.y = (resy - h*scale) / 2 / scale;
		rti_game.transform.scale = scale;
		rti_game.visible = true;
	}

	if (rti_menu.visible = MenuOpen)
	{
		int w = al_get_bitmap_width(rti_menu.bitmap);
		int h = al_get_bitmap_height(rti_menu.bitmap);
		float scale = std::min((float)resx / w, (float)resy / h);
		if (scaling_force_integer)
			scale = std::max((int) scale, 1);
		rti_menu.transform.x = 0;
		rti_menu.transform.y = 0;
		rti_menu.transform.scale = scale;
	}

	if (rti_gui.visible = (dialog_count >= 1 && !active_dialog) || dialog_count >= 2 || screen == gui_bmp)
	{
		int w = al_get_bitmap_width(rti_gui.bitmap);
		int h = al_get_bitmap_height(rti_gui.bitmap);
		float scale = std::min((float)resx/w, (float)resy/h);
		if (scaling_force_integer)
			scale = std::max((int) scale, 1);
		rti_gui.transform.x = (resx - w*scale) / 2 / scale;
		rti_gui.transform.y = (resy - h*scale) / 2 / scale;
		rti_gui.transform.scale = scale;
		if (rti_gui.visible)
			rti_menu.visible = false;
	}

	if (rti_screen.visible)
	{
		int w = al_get_bitmap_width(rti_screen.bitmap);
		int h = al_get_bitmap_height(rti_screen.bitmap);
		float scale = std::min((float)resx/w, (float)resy/h);
		if (scaling_force_integer)
			scale = std::max((int) scale, 1);
		rti_screen.transform.x = (resx - w*scale) / 2 / scale;
		rti_screen.transform.y = (resy - h*scale) / 2 / scale;
		rti_screen.transform.scale = scale;
		// TODO: don't recreate screen bitmap when alternating fullscreen mode.
		rti_screen.a4_bitmap = screen;
	}

	rti_game.freeze_a4_bitmap_render = rti_menu.visible || rti_gui.visible || Saving;
	if (rti_game.freeze_a4_bitmap_render)
	{
		static ALLEGRO_COLOR tint = al_premul_rgba_f(0.4, 0.4, 0.8, 0.8);
		rti_game.tint = &tint;
	}
	else
	{
		rti_game.tint = nullptr;
	}
}

static void render_debug_text(ALLEGRO_FONT* font, std::string text, int x, int y, int scale)
{
	int resx = al_get_display_width(all_get_display());
	int w = al_get_text_width(font, text.c_str());
	int h = al_get_font_line_height(font);

	static ALLEGRO_BITMAP* text_bitmap;
	if (text_bitmap == nullptr || resx != al_get_bitmap_width(text_bitmap))
	{
		if (text_bitmap)
			al_destroy_bitmap(text_bitmap);
		al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE);
		text_bitmap = al_create_bitmap(resx, 8);
	}

	al_set_target_bitmap(text_bitmap);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_draw_filled_rectangle(0, 0, w, h, al_map_rgba_f(0, 0, 0, 0.6));
	al_draw_text(font, al_map_rgb_f(1,1,1), 0, 0, 0, text.c_str());

	al_set_target_backbuffer(all_get_display());
	al_draw_scaled_bitmap(text_bitmap,
		0, 0,
		al_get_bitmap_width(text_bitmap), al_get_bitmap_height(text_bitmap),
		x, y,
		al_get_bitmap_width(text_bitmap) * scale, al_get_bitmap_height(text_bitmap) * scale,
		0
	);
}

enum class TextJustify {
	left,
	right,
};
static void render_text_lines(ALLEGRO_FONT* font, std::vector<std::string> lines, TextJustify justify, int scale)
{
	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());
	int font_height = al_get_font_line_height(font);
	int debug_text_y = resy - scale*font_height - 5;
	for (std::string line : lines)
	{
		int x = justify == TextJustify::left ?
			5 :
			resx - al_get_text_width(font, line.c_str())*scale - 5;
		render_debug_text(font, line.c_str(), x, debug_text_y, scale);
		debug_text_y -= scale*font_height + 3;
	}
}

void render_zc()
{
	init_render_tree();
	configure_render_tree();

	al_set_target_backbuffer(all_get_display());
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	render_tree_draw(&rti_root);

	static ALLEGRO_FONT* font = al_create_builtin_font();
	static int font_scale = 3;

	std::vector<std::string> lines_left;
	std::vector<std::string> lines_right;

	// TODO calculate fps without using a timer thread.
	if (ShowFPS)
		lines_left.push_back(fmt::format("fps: {}", (int)lastfps));
	if (replay_is_replaying())
		lines_left.push_back(replay_get_buttons_string().c_str());
	if (Paused)
		lines_right.push_back("PAUSED");
	if (Saving)
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
		for (int i = 0; i < tmpscr.numFFC(); i++)
		{
			// TODO z3 ffc
			if (tmpscr.ffcs[i].script)
				lines_right.push_back(ffcmap[tmpscr.ffcs[i].script-1].scriptname);
		}
	}

	render_text_lines(font, lines_left, TextJustify::left, font_scale);
	render_text_lines(font, lines_right, TextJustify::right, font_scale);

    al_flip_display();
}
