#include "zc/render.h"
#include "zc/crt_filter.h"
#include "zalleg/render.h"
#include "zc/debugger/debugger.h"
#include "zc/zelda.h"
#include "zc/maps.h"
#include "sprite.h"
#include "zc/guys.h"
#include "iter.h"
#include "core/qst.h"
#include "zalleg/gui.h"
#include "base/zapp.h"
#include "core/mapscr.h"
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

RenderTreeItem& gui_mouse_target()
{
	if (rti_dialogs.has_children())
		return *rti_dialogs.get_children().back();
	if (rti_dialogs.visible || rti_gui.visible)
		return rti_gui;
	if (rti_menu.visible)
		return rti_menu;
	return rti_game;
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
	// A child of the game layer (identity transform, same size), so the CRT filters apply
	// to the cheat/debug overlays too. As a fullres_overlay child it is composited by its
	// own live shader pass rather than baked, keeping it updating while the game layer is
	// frozen (the debugger draws its sprite highlight while paused under a dialog).
	rti_infolayer.fullres_overlay = true;

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
	rti_game.add_child(&rti_infolayer);
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
	rti_root.set_size(resx, resy);
	
	int w = rti_game.width;
	int h = rti_game.height;
	float xscale = (float)resx/w;
	float yscale = (float)resy/(h+6);
	bool keep_aspect_ratio = !stretchGame;
	if (keep_aspect_ratio)
		xscale = yscale = std::min(xscale, yscale);
	if (scaleForceInteger)
	{
		xscale = intscale(xscale);
		yscale = intscale(yscale);
	}

	rti_game.set_transform({
		.x = (float)((int)(resx - w*xscale) / 2),
		.y = (float)((int)(resy - h*yscale) / 2),
		.xscale = xscale,
		.yscale = yscale,
	});
	rti_game.visible = true;

	// The CRT filter applies only to the game layer, so menus and dialogs stay crisp.
	rti_game.shader = crt_filter_shader();
	if (rti_game.shader)
	{
		rti_game.shader_prepare = [](RenderTreeItem* rti, int out_w, int out_h) {
			crt_filter_set_uniforms(rti->width, rti->height, out_w, out_h);
		};
		// Full-resolution children (the title logo) composite through the overlay variant.
		rti_game.overlay_shader = crt_filter_overlay_shader();
		rti_game.overlay_prepare = [](RenderTreeItem* rti, RenderTreeItem* child, int out_w, int out_h) {
			auto& t = child->get_transform();
			crt_filter_set_overlay_uniforms(rti->width, rti->height, out_w, out_h,
				(float)t.x / rti->width,
				(float)t.y / rti->height,
				al_get_bitmap_width(child->bitmap) * t.xscale / rti->width,
				al_get_bitmap_height(child->bitmap) * t.yscale / rti->height);
		};
		rti_game.uv_warp = crt_filter_warps_mouse() ?
			[](double u, double v) { return crt_filter_warp_uv(u, v); } :
			std::function<std::pair<double, double>(double, double)>();
	}
	else
	{
		rti_game.shader_prepare = nullptr;
		rti_game.overlay_shader = nullptr;
		rti_game.overlay_prepare = nullptr;
		rti_game.uv_warp = nullptr;
	}

	// Identity: as a child of rti_game it inherits the transform above. Hidden while empty
	// so the layer costs nothing (a fullscreen blend, or a whole extra shader pass under a
	// CRT filter) when no cheat/debug overlay is being drawn - which is nearly always.
	rti_infolayer.set_transform({});
	rti_infolayer.visible = info_bmp_has_content();
	
	rti_dialogs.visible = rti_dialogs.has_children();
	rti_gui.visible = (dialog_count >= 1 && !active_dialog) || dialog_count >= 2 || screen == gui_bmp;

	float gui_xscale, gui_yscale;
	{
		int w = rti_gui.width;
		int h = rti_gui.height;
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		gui_xscale = gui_yscale = std::min(xscale, yscale);
	}
	
	if (rti_dialogs.visible || rti_gui.visible)
	{
		int w = rti_gui.width;
		int h = rti_gui.height;
		float xscale = gui_xscale;
		float yscale = gui_yscale;
		rti_gui.set_transform({
			.x = (float)((int)(resx - w*xscale) / 2),
			.y = (float)((int)(resy - h*yscale) / 2),
			.xscale = xscale,
			.yscale = yscale,
		});
		
		rti_dialogs.set_transform({
			.x = 0,
			.y = 0,
			.xscale = xscale,
			.yscale = yscale,
		});
	}
	
	bool has_zqdialog = false;
	auto& dlgs = rti_dialogs.get_children();
	for(auto it = dlgs.rbegin(); it != dlgs.rend(); ++it)
	{
		auto rti = *it;
		if (rti->type == RTI_TY_DIALOG_A4 || rti->type == RTI_TY_DIALOG_A5)
		{
			int dialogs_w = rti_gui.width;
			int dialogs_h = rti_gui.height;
			auto t = rti->get_transform();
			t.x = resx / gui_xscale / 2 - dialogs_w / 2;
			t.y = resy / gui_yscale / 2 - dialogs_h / 2;
			rti->set_transform(t);
			has_zqdialog = true;
		}
		else if(rti->type == RTI_TY_POPUP_MENU)
			rti->visible = !has_zqdialog;
	}
	rti_menu.visible = MenuOpen && !has_zqdialog;
	
	if (rti_menu.visible)
	{
		float xscale = gui_xscale;
		float yscale = gui_yscale;
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
			.x = (float)((int)(resx - w*xscale) / 2),
			.y = (float)((int)(resy - h*yscale) / 2),
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

	// The tint dims whatever is behind the active dialog, so it must cover the entire
	// display. It lives inside rti_dialogs, which only spans the GUI area (letterboxed
	// to keep the GUI's aspect ratio), so undo that scaling here.
	auto& rti_tint = *get_dlg_tint_rti();
	if (rti_tint.width > 0 && rti_tint.height > 0)
	{
		rti_tint.set_transform({
			.x = 0,
			.y = 0,
			.xscale = resx / (gui_xscale * rti_tint.width),
			.yscale = resy / (gui_yscale * rti_tint.height),
		});
	}
}

// This is disabled in the web build because the high cost of swapping textures on the GPU.
// This is just for displaying a debug layer, so it's fine to disable.
// The current setup has this bitmap cleared every frame in draw_screen and conditionally drawn to if
// some debug data must be drawn, so the cost was being paid to swap textures even when not used.

// Whether anything has drawn into the info layer since it was last cleared. Every drawer
// goes through start_info_bmp, so this is exact - it drives the layer's visibility.
static bool info_bmp_written;

bool info_bmp_has_content()
{
	return info_bmp_written;
}

void clear_info_bmp()
{
	info_bmp_written = false;
#ifndef __EMSCRIPTEN__
	clear_a5_bmp(rti_infolayer.bitmap);
#endif
}

// Overlay drawers (walkflags/effectflags cheats, hitboxes, the debugger's
// sprite highlight) must check this and skip their draws entirely when false:
// start_info_bmp is a no-op on the web build, so their allegro5 primitives
// would otherwise land on whatever target happens to be current - or abort in
// allegro's no-current-display assert (this crashed web replays that toggled
// the walkflags cheat).
bool info_bmp_enabled()
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	// The infolayer bitmap is never created in headless mode.
	return rti_infolayer.bitmap != nullptr;
#endif
}

// Backing store for save_info_bmp/restore_info_bmp, and whether the layer had content when
// it was captured - so restoring an empty backup doesn't re-enable the layer's draw.
static ALLEGRO_BITMAP* infobmp_backup = nullptr;
static bool infobmp_backup_written;

void save_info_bmp()
{
#ifndef __EMSCRIPTEN__
	ALLEGRO_BITMAP* src = rti_infolayer.bitmap;
	if (!src)
		return;

	int w = al_get_bitmap_width(src);
	int h = al_get_bitmap_height(src);
	if (infobmp_backup && (al_get_bitmap_width(infobmp_backup) != w || al_get_bitmap_height(infobmp_backup) != h))
	{
		al_destroy_bitmap(infobmp_backup);
		infobmp_backup = nullptr;
	}
	if (!infobmp_backup)
		infobmp_backup = create_a5_bitmap(w, h);

	infobmp_backup_written = info_bmp_written;

	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(infobmp_backup);
	al_clear_to_color(AL5_INVIS);
	// Straight copy: overwrite the destination pixels rather than alpha-blend.
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_draw_bitmap(src, 0, 0, 0);
	al_restore_state(&old_state);
#endif
}

void restore_info_bmp()
{
#ifndef __EMSCRIPTEN__
	if (!infobmp_backup)
	{
		clear_info_bmp();
		return;
	}

	info_bmp_written = infobmp_backup_written;

	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(rti_infolayer.bitmap);
	al_clear_to_color(AL5_INVIS);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_draw_bitmap(infobmp_backup, 0, 0, 0);
	al_restore_state(&old_state);
#endif
}

static ALLEGRO_STATE infobmp_old_state;
void start_info_bmp()
{
#ifndef __EMSCRIPTEN__
	info_bmp_written = true;
	al_store_state(&infobmp_old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rti_infolayer.bitmap);
	al_set_clipping_rectangle(0, playing_field_offset, al_get_bitmap_width(rti_infolayer.bitmap), al_get_bitmap_height(rti_infolayer.bitmap)-playing_field_offset);
#endif
}
void end_info_bmp()
{
#ifndef __EMSCRIPTEN__
	al_set_clipping_rectangle(0, 0, al_get_bitmap_width(rti_infolayer.bitmap), al_get_bitmap_height(rti_infolayer.bitmap));
	al_restore_state(&infobmp_old_state);
#endif
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

	ALLEGRO_FONT* a5font = get_zc_font_a5(font_gboraclepfont);
	// Match the overlay text to the game's scale rather than using a fixed scale. The text is
	// stamped into the backbuffer, so a fixed scale means its apparent size tracks the
	// backbuffer resolution - most visible on web, where the backbuffer is the 640x480 virtual
	// screen without a CRT filter (CSS then magnifies it, text included) but the full canvas
	// resolution with one.
	int font_scale = std::max(1, (int)std::round(rti_game.get_transform().yscale));

	std::vector<std::string> lines_left;
	std::vector<std::string> lines_right;

	if (ShowGameTime && game && Playing)
	{
		if (MenuOpen || Paused)
			lines_left.push_back(fmt::format("{} ({})", time_str_long(game->get_time()), game->get_time()));
		else
			lines_left.push_back(fmt::format("{}", ShowGameTime == 2 ? time_str_long(game->get_time()) : time_str_med(game->get_time())));
	}
	// TODO calculate fps without using a timer thread.
	if (ShowFPS)
		lines_left.push_back(fmt::format("FPS: {}", (int)lastfps));
	if (replay_is_replaying())
		lines_left.push_back(replay_get_buttons_string().c_str());
	else if (replay_is_recording() && MenuOpen)
		lines_left.push_back(fmt::format("Recording, frame {}", replay_get_frame()));
	if (auto debugger = zscript_debugger_get_if_open(); debugger && debugger->state == Debugger::State::Paused)
		lines_right.push_back("PAUSED (DEBUGGER)");
	else if (Paused)
		lines_right.push_back("PAUSED");
	if (Saving && use_save_indicator)
		lines_right.push_back("SAVING ...");
	if (show_ff_scripts)
	{
		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if (ffc_handle.ffc->scrconfig.script)
				lines_right.push_back(ffcmap[ffc_handle.ffc->scrconfig.script-1].scriptname);
		});
	}

	ALLEGRO_BITMAP* bitmap = al_get_backbuffer(all_get_display());
	render_text_lines(bitmap, a5font, lines_left, TextJustify::left, TextAlignment::bottom, font_scale);
	render_text_lines(bitmap, a5font, lines_right, TextJustify::right, TextAlignment::bottom, font_scale);

	if (render_get_debug())
		render_tree_draw_debug(&rti_root);

    al_flip_display();
	
	screen = tmp;
	al_restore_state(&oldstate);
}

void zq_push_unfrozen_dialogs(size_t){}
void zq_pop_unfrozen_dialogs(){}

void clear_tooltip()
{
	
}
void ttip_uninstall_all()
{
	
}

