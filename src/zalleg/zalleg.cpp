#include "zalleg/zalleg.h"
#include "allegro/base.h"
#include "allegro/file.h"
#include "allegro/inline/system.inl"
#include "allegro/palette.h"
#include "allegro5/events.h"
#include "zalleg/files.h"
#include "core/fonts.h"
#include "zalleg/render.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "zalleg/zsys.h"
#include "sfx.h"
#include "zsyssimple.h"
#include <al5_img.h>
#include <cstdint>
#include <fmt/format.h>
#include <loadpng.h>
#include <utility>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

#ifdef ALLEGRO_SDL
#include <SDL.h>
#endif

#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#include <windows.h>
#endif

#ifdef ALLEGRO_MACOSX
void zalleg_osx_bring_window_to_foreground(ALLEGRO_DISPLAY* display);
void zalleg_osx_get_main_screen_usable_size(int* w, int* h);
float zalleg_osx_get_main_screen_scale_factor(void);
#endif

#ifdef __linux__
#include <allegro5/allegro_x.h>
#endif

DATAFILE *fontsdata, *sfxdata;
bool sound_was_installed = false;

extern bool DragAspect;
extern double aspect_ratio;
extern int window_min_width, window_min_height;

namespace
{

//Fun fact: Allegro used to be in control of allegro.log. This caused
//problems, because it would hold on to a file handle. Even if we blank
//the contents of the log, it will still write to the end, causing
//lots of nulls.
//No more!

FILE * trace_file;

int32_t zc_trace_handler(const char * msg)
{
#ifndef _WIN32
	printf("%s", msg);
#endif

	if(trace_file == 0)
	{
		if (getenv("ALLEGRO_LEGACY_TRACE"))
			trace_file = fopen(getenv("ALLEGRO_LEGACY_TRACE"), "a+");
		else
			trace_file = fopen("allegro.log", "a+");
		
		if(0==trace_file)
		{
			return 0; // blargh.
		}
	}
	
	fprintf(trace_file, "%s", msg);
	fflush(trace_file);
	return 1;
}

void doAspectResize()
{
	if (!DragAspect || all_get_fullscreen_flag())
		return;

	static int prev_width = 0, prev_height = 0;
	
	if (prev_width == 0 || prev_height == 0)
	{
		prev_width = al_get_display_width(all_get_display());
		prev_height = al_get_display_height(all_get_display());
	}

	if (prev_width != al_get_display_width(all_get_display()) || prev_height != al_get_display_height(all_get_display()))
	{
		bool width_first = true;
		
		if (abs(prev_width - al_get_display_width(all_get_display())) < abs(prev_height - al_get_display_height(all_get_display()))) width_first = false;
		
		if (width_first)
		{
			al_resize_display(all_get_display(), al_get_display_width(all_get_display()), al_get_display_width(all_get_display())*aspect_ratio);
		}
		else
		{
			al_resize_display(all_get_display(), al_get_display_height(all_get_display())/aspect_ratio, al_get_display_height(all_get_display()));
		}
	}

	prev_width = al_get_display_width(all_get_display());
	prev_height = al_get_display_height(all_get_display());
}

void zc_do_minsize()
{
	if(all_get_fullscreen_flag()) return;
	if(!(window_min_width || window_min_height)) return;
	
	int wid = al_get_display_width(all_get_display());
	int hei = al_get_display_height(all_get_display());
	if(wid < window_min_width || hei < window_min_height)
	{
		if(wid < window_min_width) wid = window_min_width;
		if(hei < window_min_height) hei = window_min_height;
		al_resize_display(all_get_display(),wid,hei);
	}
}

void zapp_setup_icon()
{
	if (is_headless())
		return;

	// On Windows, the icon is embedded in the executable.
	// For Mac app bundle, only the entry point app will have the logo. If an executable is opened directly, or launcher via the launcher,
	// we must add the icon at runtime.
#ifndef _WIN32
	App app_id = get_app_id();
	std::string icon_path;
	// Allow for custom icon via `icon.png`.
	if (exists("icon.png"))
		icon_path = "icon.png";
	else if (app_id == App::launcher)
		icon_path = "assets/zc/ZC_Icon_Medium_Launcher.png";
	else if (app_id == App::zelda)
		icon_path = "assets/zc/ZC_Icon_Medium_Player.png";
	else if (app_id == App::zquest)
		icon_path = "assets/zc/ZC_Icon_Medium_Editor.png";
	ALLEGRO_BITMAP* icon_bitmap = al_load_bitmap(icon_path.c_str());
	if (icon_bitmap)
	{
		al_set_display_icon(all_get_display(), icon_bitmap);
		al_destroy_bitmap(icon_bitmap);
	}
#endif
}

void AllegroCheckFailedHandler(const char* file, int line, const char* func, const char* condition)
{
	std::string message = fmt::format("CHECK failed at {}:{} {}: {}", file, line, func, condition);
	al_trace("%s\n", message.c_str());
	zapp_reporting_add_breadcrumb("error", message.c_str());
}

} // end namespace

void zalleg_setup_allegro(App id, int argc, char **argv)
{
#ifdef ALLEGRO_SDL
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

	SetCheckFailedHandler(AllegroCheckFailedHandler);

	common_main_setup(id, argc, argv);

	zapp_set_crash_cb([](){
		if (all_get_display())
			al_save_bitmap(".sentry_native/screenshot.png", al_get_backbuffer(all_get_display()));
	});

	register_trace_handler(zc_trace_handler);
	al_register_trace_handler([](const char* str){zc_trace_handler(str);});
	all_disable_threaded_display();

	if (used_switch(argc, argv, "-headless") || std::getenv("ZC_HEADLESS") != nullptr)
	{
		set_headless_mode();
	}

	Z_message("Initializing Allegro... ");
	if (!al_init())
	{
		Z_error_fatal("Failed to init allegro: %s\n", "al_init");
	}
	if (allegro_init() != 0)
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "allegro_init", allegro_error);
	}

	// Merge old a4 config into a5 system config.
	ALLEGRO_CONFIG *tempcfg = al_load_config_file(get_config_file_name());
	if (tempcfg) {
		al_merge_config_into(al_get_system_config(), tempcfg);
		al_destroy_config(tempcfg);
	}

#ifdef __EMSCRIPTEN__
	em_mark_initializing_status();
	em_init_fs();
#endif

	if(!al_init_image_addon())
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "al_init_image_addon", allegro_error);
	}

	if(!al_init_font_addon())
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "al_init_font_addon", allegro_error);
	}

	if(!al_init_primitives_addon())
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "al_init_primitives_addon", allegro_error);
	}

	al5img_init();
	register_png_file_type();

    if(install_timer() < 0)
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "install_timer", allegro_error);
	}
	
	// Don't drive physical keyboard LEDs when headless: allegro's LED updates
	// go through X11 and crash without a usable display.
	if (is_headless())
		key_led_flag = FALSE;
	if(install_keyboard() < 0)
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "install_keyboard", allegro_error);
	}
	poll_keyboard();
	
	if(install_mouse() < 0)
	{
		Z_error_fatal("Failed to init allegro: %s\n%s\n", "install_mouse", allegro_error);
	}

	if (id == App::zelda || id == App::zquest)
	{
		if(install_joystick(JOY_TYPE_AUTODETECT) < 0)
		{
			Z_error_fatal("Failed to init allegro: %s\n%s\n", "install_joystick", allegro_error);
		}

		Z_message("SFX.Dat...");

		char sfxdat_sig[52];
		snprintf(sfxdat_sig, sizeof(sfxdat_sig), "SFX.Dat %s Build %d",VerStrFromHex(SFXDAT_VERSION), SFXDAT_BUILD);

		if((sfxdata=load_datafile("sfx.dat"))==NULL)
		{
			Z_error_fatal("failed to load sfx.dat");
		}
		if(strncmp((char*)sfxdata[0].dat,sfxdat_sig,22) || sfxdata[Z35].type != DAT_ID('S', 'A', 'M', 'P'))
			Z_error_fatal("\nIncompatible version of sfx.dat.\nPlease upgrade to %s Build %d",VerStrFromHex(SFXDAT_VERSION), SFXDAT_BUILD);

		Z_message("OK\n");
	}

	Z_message("Initializing sound driver... ");
    bool sound = id == App::zelda || id == App::zquest;
	if (is_headless() || used_switch(argc,argv,"-s") || used_switch(argc,argv,"-nosound")) sound = false;
	if (id == App::zelda && zc_get_config("zeldadx","nosound",0)) sound = false;
	if (id == App::zquest && zc_get_config("zquest","nosound",0)) sound = false;
	sound_was_installed = sound;
	if (!sound)
	{
		Z_message("skipped\n");
	}
	else
	{
		if(!al_install_audio())
		{
			// We can continue even with no audio.
			Z_error("Failed al_install_audio");
			sound_was_installed = false;
		}

		if(!al_init_acodec_addon())
		{
			Z_error("Failed al_init_acodec_addon");
		}

		if(install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL))
		{
			//      Z_error_fatal(allegro_error);
			Z_message("Sound driver not available.  Sound disabled.\n");
		}
		else
		{
			Z_message("OK\n");
		}
	}

	render_set_debug(zc_get_config("graphics","render_debug",0));
#ifndef __EMSCRIPTEN__
	set_always_use_native_file_dialog(zc_get_config("gui","native_file_dialog",1));
#endif

	packfile_password(datapwd);

	char fontsdat_sig[52]={0};
	snprintf(fontsdat_sig, sizeof(fontsdat_sig), "Fonts.Dat %s Build %d",VerStrFromHex(FONTSDAT_VERSION), FONTSDAT_BUILD);

	size_t fontsdat_cnt = 0;
	if((fontsdata=load_datafile_count("modules/classic/classic_fonts.dat", fontsdat_cnt))==NULL)
	{
		Z_error_fatal("failed to load fonts\n");
	}
	if(fontsdat_cnt != FONTSDAT_CNT)
	{
		Z_error_fatal("failed: count error (found %d != exp %d)\n", fontsdat_cnt, FONTSDAT_CNT);
	}
	
	if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,24))
	{
		Z_error_fatal("\nIncompatible version of fonts.dat.\nPlease upgrade to %s Build %d\n",VerStrFromHex(FONTSDAT_VERSION), FONTSDAT_BUILD);
	}

	packfile_password(NULL);
}

// (v_width, v_height): the size of the allegro 4 screen bitmap. It's the "base" resolution, as far as allegro 4 is concerned.
// (saved_window_width, saved_window_height): the size to make the display window. If -1, this function picks the best size based on the monitor.
void zalleg_create_window(const char* title, int gfx_mode, int v_width, int v_height, [[maybe_unused]] int saved_window_width, [[maybe_unused]] int saved_window_height, [[maybe_unused]] int max_scale)
{
	if (is_headless())
	{
		initFonts(); // Doesn't really belong here, but whatever.

		Z_message("gfx mode set: %s %dbpp %d x %d \n", "headless", get_color_depth(), v_width, v_height);
		return;
	}

#ifdef __EMSCRIPTEN__
	// For web, there's no point making the display size different from the virtual screen size,
	// because the canvas that SDL renders into is scaled up via CSS.
	int w = v_width;
	int h = v_height;
#else
	// Either get the saved window width/height (if not -1), or determine the largest width/height
	// for the primary monitor that fits the virtual screen's aspect ratio.
	auto [w, h] = zalleg_get_default_display_size(v_width, v_height, saved_window_width, saved_window_height, max_scale);
	if (w <= 0 || h <= 0)
	{
		w = v_width;
		h = v_height;
	}
#endif

	// This is an allegro 4 call to setup the graphics driver. It creates the `screen` bitmap using
	// (v_width, v_height), and creates an OS windows using (w, h).
	// This also sets up the allegro 5 display, via third_party/allegro_legacy/src/a5/a5_display.c
	if (set_gfx_mode(gfx_mode, w, h, v_width, v_height))
		Z_error_fatal("Failed to create window: %s\n", allegro_error);

	Z_message("gfx mode set: %d %dbpp %d x %d \n", gfx_mode, get_color_depth(), v_width, v_height);

	set_window_title(title);
	zapp_setup_icon();
	initFonts(); // Doesn't really belong here, but whatever.
}

void zalleg_bring_window_to_foreground(ALLEGRO_DISPLAY* display)
{
#ifdef _WIN32
	HWND hwnd = al_get_win_window_handle(display);
	SetForegroundWindow(hwnd);
#elif defined(ALLEGRO_MACOSX)
	zalleg_osx_bring_window_to_foreground(display);
#elif defined(__linux__)
	XID window_id = al_get_x_window_id(display);
	Display* x_display = XOpenDisplay(nullptr);
	if (x_display)
	{
		XEvent event = {};
		event.type = ClientMessage;
		event.xclient.display = x_display;
		event.xclient.window = window_id;
		event.xclient.message_type = XInternAtom(x_display, "_NET_ACTIVE_WINDOW", False);
		event.xclient.format = 32;
		event.xclient.data.l[0] = 1; // source: application
		event.xclient.data.l[1] = CurrentTime;
		event.xclient.data.l[2] = 0;
		XSendEvent(x_display, DefaultRootWindow(x_display), False,
		           SubstructureRedirectMask | SubstructureNotifyMask, &event);
		XFlush(x_display);
		XCloseDisplay(x_display);
	}
#endif
}

void zalleg_wait_for_all_keys_up()
{
	while (true)
	{
		bool found = false;
		for (int i = 0; i < KEY_MAX; i++)
			if (key[i]) found = true;
		if (!found) break;
		
		poll_keyboard();
		update_hw_screen();
	}
}

// If (saved_width, saved_height) is >0, ensures that fits in the primary monitor. If neither are true, fall
// back to default.
// Default will scale up (base_width, base_height) by an integer amount to fill up the primary monitor
// as much as possible, up to 3x.
std::pair<int, int> zalleg_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height, int max_scale)
{
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	int mw = info.x2 - info.x1;
	int mh = info.y2 - info.y1;
	if (saved_width > 0 && saved_height > 0 && saved_width <= mw && saved_height <= mh)
	{
		return {saved_width, saved_height};
	}

#ifdef ALLEGRO_MACOSX
	// Use the screen's usable area, which excludes the menu bar and dock
	// (al_get_monitor_info reports the full frame including the menu bar).
	int uw = 0, uh = 0;
	zalleg_osx_get_main_screen_usable_size(&uw, &uh);
	if (uw > 0 && uh > 0)
	{
		mw = uw;
		mh = uh;
	}
#endif
	// Reserve room for the window's title bar, plus a little breathing room so
	// the default window never fills the entire usable area.
#if defined(ALLEGRO_MACOSX)
	// visibleFrame already excludes the menu bar and dock, but not the title
	// bar, which scales with DPI.
	float scale = zalleg_osx_get_main_screen_scale_factor();
	mw -= (int)(24 * scale);
	mh -= (int)(48 * scale);
#elif defined(_WIN32)
	// Title bar.
	mw -= 50;
	mh -= 23 + 50;
#else
	mw -= 50;
	mh -= 50;
#endif

	int s = std::min(mh / base_height, mw / base_width);
	s = std::min(max_scale, s);
	int w = base_width * s;
	int h = base_height * s;
	return {w, h};
}

void zalleg_process_display_events()
{
	all_process_display_events();
	// TODO: should do this only in response to a resize event
	doAspectResize();
	zc_do_minsize();
}

PACKFILE* zalleg_pack_fopen_password(const char *filename, const char *mode, const char *password)
{
	packfile_password(password);
	PACKFILE* new_pf = pack_fopen(filename, mode);
	packfile_password("");
	return new_pf;
}

uint64_t zalleg_file_size_ex_password(const char *filename, const char *password)
{
	packfile_password(password);
	uint64_t new_pf = file_size_ex(filename);
	packfile_password("");
	return new_pf;
}

bool zalleg_alleg4_save_bitmap(BITMAP* source, int scale, const char* filename, AL_CONST RGB *pal)
{
	BITMAP* scaled = nullptr;
	if (scale != 1)
	{
		int w = source->w;
		int h = source->h;
		scaled = create_bitmap_ex(8, w*scale, h*scale);
		stretch_blit(source, scaled, 0, 0, w, h, 0, 0, w*scale, h*scale);
	}

	PALETTE default_pal;
	if (!pal)
		get_palette(default_pal);
	int result = save_bitmap(filename, scaled ? scaled : source, pal ? pal : default_pal);
	destroy_bitmap(scaled);
	return result == 0;
}

void zalleg_update_bmp_size(BITMAP** bmp_ptr, int w, int h)
{
	BITMAP* bmp = *bmp_ptr;
	if (bmp)
	{
		if (bmp->w == w && bmp->h == h)
			return;
		destroy_bitmap(bmp);
	}
	*bmp_ptr = create_bitmap_ex(8, w, h);
}

/* zalleg_save_midi:
*  Saves a standard MIDI file, returning 0 on success,
*  or non-zero on error.
*/
int32_t zalleg_save_midi(const char *filename, MIDI *midi)
{
	int32_t c;
	int32_t len;
	PACKFILE *fp;
	int32_t num_tracks = 0;
	
	if(!midi)
		return 1;
		
	fp = zalleg_pack_fopen_password(filename, F_WRITE,"");                       /* open the file */
	
	if(!fp)
		return 2;
		
	for(c=0; c<MIDI_TRACKS; c++)
		if(midi->track[c].len > 0)
			num_tracks++;
			
	pack_fwrite((void *) "MThd", 4, fp);                      /* write midi header */
	
	pack_mputl(6, fp);                                        /* header chunk length = 6 */
	
	pack_mputw((num_tracks==1) ? 0 : 1, fp);                  /* MIDI file type */
	
	pack_mputw(num_tracks, fp);                               /* number of tracks */
	
	pack_mputw(midi->divisions, fp);                          /* beat divisions (negatives?) */
	
	for(c=0; c<num_tracks; c++)                               /* write each track */
	{
		pack_fwrite((void *) "MTrk", 4, fp);                    /* write track header */
		
		len = midi->track[c].len;
		pack_mputl(len, fp);                                    /* length of track chunk */
		
		if(pack_fwrite(midi->track[c].data, len, fp) != len)
			goto err;
	}
	
	pack_fclose(fp);
	return 0;
	
	/* oh dear... */
err:
	pack_fclose(fp);
	delete_file(filename);
	return 3;
}

void zc_trace_clear()
{
	if(trace_file)
	{
		fclose(trace_file);
	}
	
	if (getenv("ALLEGRO_LEGACY_TRACE"))
		trace_file = fopen(getenv("ALLEGRO_LEGACY_TRACE"), "w");
	else
		trace_file = fopen("allegro.log", "w");
	ASSERT(trace_file);
}

void safe_al_trace(const char* str)
{
	zc_trace_handler(str);
}

void safe_al_trace(std::string const& str)
{
	zc_trace_handler(str.c_str());
}
