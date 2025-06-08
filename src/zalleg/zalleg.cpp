#include "zalleg/zalleg.h"
#include "base/files.h"
#include "base/fonts.h"
#include "base/render.h"
#include "base/version.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "base/zc_alleg.h"
#include "base/zsys.h"
#include "sfx.h"
#include "zinfo.h"
#include <al5_img.h>
#include <loadpng.h>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

DATAFILE *fontsdata, *sfxdata;

void zalleg_setup_allegro(App id, int argc, char **argv)
{
	common_main_setup(id, argc, argv);

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

	Z_message("Initializing sound driver... ");
    bool sound = id == App::zelda || id == App::zquest;
	if (is_headless() || used_switch(argc,argv,"-s") || used_switch(argc,argv,"-nosound")) sound = false;
	if (id == App::zelda && zc_get_config("zeldadx","nosound",0)) sound = false;
	if (id == App::zquest && zc_get_config("zquest","nosound",0)) sound = false;
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
	set_always_use_native_file_dialog(zc_get_config("gui","native_file_dialog",0));

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
