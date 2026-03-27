#include "zalleg/zalleg.h"
#include "allegro/base.h"
#include "allegro/file.h"
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
		sprintf(sfxdat_sig,"SFX.Dat %s Build %d",VerStrFromHex(SFXDAT_VERSION), SFXDAT_BUILD);

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

	Z_message("Fonts.Dat...");
	char fontsdat_sig[52]={0};
	sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStrFromHex(FONTSDAT_VERSION), FONTSDAT_BUILD);

	size_t fontsdat_cnt = 0;
	if((fontsdata=load_datafile_count("modules/classic/classic_fonts.dat", fontsdat_cnt))==NULL)
	{
		Z_error_fatal("failed to load fonts");
	}
	if(fontsdat_cnt != FONTSDAT_CNT)
	{
		Z_error_fatal("failed: count error (found %d != exp %d)\n", fontsdat_cnt, FONTSDAT_CNT);
	}
	
	if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,24))
	{
		Z_error_fatal("\nIncompatible version of fonts.dat.\nPlease upgrade to %s Build %d",VerStrFromHex(FONTSDAT_VERSION), FONTSDAT_BUILD);
	}

	packfile_password(NULL);
}

void zalleg_create_window()
{
	// TODO: move window creation stuff to here.
	zapp_setup_icon();
	initFonts();
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
	// https://talk.automators.fm/t/getting-screen-dimensions-while-accounting-the-menu-bar-dock-and-multiple-displays/13639
	mh -= 38;
#endif
#ifdef _WIN32
	// Title bar.
	mh -= 23;
#endif
	// Small buffer, so the default window is never as big as the monitor.
	mw -= 50;
	mh -= 50;

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
