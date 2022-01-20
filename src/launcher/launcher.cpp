// Launcher executable main file
#include "zdefs.h"
#include "launcher.h"
#include "module.h"
#include "zsys.h"
#include "fonts.h"
#include "dialog/alert.h"
#include "launcher_dialog.h"

#define QUICK_EXIT 0


#define QUIT_LAUNCHER() \
do{ \
	allegro_exit(); \
	return 1; \
} \
while(false)

DATAFILE *fontsdata;
PALETTE RAMpal;

ZModule zcm;
zcmodule moduledata;

volatile int32_t lastfps = 0;
volatile int32_t framecnt = 0;
int32_t joystick_index = 0;
int32_t readsize = 0, writesize = 0;
volatile int32_t myvsync=0;
int32_t zqwin_scale = 1;
int32_t zq_screen_w=800;
int32_t zq_screen_h=600;
BITMAP *tmp_scr;
BITMAP *hw_screen;
BITMAP *mouse_bmp;
int32_t gui_colorset = 99;
volatile bool close_button_quit = false;

void init_launcher_palette();
void fps_callback();
void myvsync_callback();

int32_t cursorColor(int32_t col)
{
	switch(col)
	{
		case dvc(1):
		case dvc(4):
			return jwin_pal[jcCURSORMISC];
		case dvc(2):
			return jwin_pal[jcCURSOROUTLINE];
		case dvc(3):
			return jwin_pal[jcCURSORLIGHT];
		case dvc(5):
			return jwin_pal[jcCURSORDARK];
	}
	return col;
}

void hit_close_button()
{
    close_button_quit=true;
    return;
}

int32_t main(int32_t argc, char* argv[])
{
	set_uformat(U_ASCII);
	zc_srand(time(0));
	
	
	Z_message("Initializing Allegro... "); //{
	allegro_init();
	set_config_standard();
	// register_bitmap_file_type("GIF",  load_gif, save_gif);
	// jpgalleg_init();
	// loadpng_init();
	if(install_timer() < 0
		|| install_keyboard() < 0
		|| install_mouse() < 0)
	{
		Z_error(allegro_error);
		QUIT_LAUNCHER();
	}
	
	enable_hardware_cursor();
	LOCK_VARIABLE(lastfps);
	
	LOCK_VARIABLE(framecnt);
	LOCK_FUNCTION(fps_callback);
	
	if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
	{
		Z_error("couldn't allocate timer\n");
		QUIT_LAUNCHER();
	}
	LOCK_VARIABLE(myvsync);
	LOCK_FUNCTION(myvsync_callback);
	if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60)))
	{
		Z_error("couldn't allocate timer\n");
		QUIT_LAUNCHER();
	}
	
	LOCK_VARIABLE(dclick_status);
	LOCK_VARIABLE(dclick_time);
	lock_dclick_function();
	install_int(dclick_check, 20);
	
	
	set_gfx_mode(GFX_TEXT,80,50,0,0);
	Z_message("OK\n");
	//} end Initializing Allegro...OK
	
	Z_message("Initializing module...");
	zcm.init(true);
	Z_message("OK\n");
	
	Z_message("Loading data files:\n"); //{
	
	resolve_password(datapwd);
	packfile_password(datapwd);
	
	Z_message("....Fonts.Dat..."); //{
	if((fontsdata=load_datafile(moduledata.datafiles[fonts_dat]))==NULL)
	{
		Z_error("failed: load error\n");
		QUIT_LAUNCHER();
	}
	char fontsdat_sig[52]={0};
	sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);
	if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,24))
	{
		Z_error("failed: version error\n");
		QUIT_LAUNCHER();
	}
	initFonts();
	Z_message("OK\n");
	//} end Fonts.Dat...OK
	packfile_password("");
	
	Z_message("....OK\n");
	//} end Loading data files:
	
	set_color_depth(8);
	int32_t videofail = set_gfx_mode(GFX_AUTODETECT_WINDOWED,zq_screen_w,zq_screen_h,0,0);
	
	if(videofail)
	{
		Z_error(allegro_error);
		QUIT_LAUNCHER();
	}
	
	Z_message("Loading bitmaps..."); //{
	tmp_scr = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
	mouse_bmp = create_bitmap_ex(8,16,16);
	//{ Screen setup
	hw_screen = screen;
	screen = create_bitmap_ex(8, zq_screen_w, zq_screen_h);
	//}
	
	if(!(tmp_scr && mouse_bmp && screen))
	{
		Z_error("failed\n");
		QUIT_LAUNCHER();
	}
	Z_message("OK\n");
	//}
	
	while(!key[KEY_SPACE]);
	
	Z_message("Loading configs...");
	gui_colorset = zc_get_config("ZLAUNCH","gui_colorset",99);
	Z_message("OK\n");
	
	Z_message("Initializing palette...");
	init_launcher_palette();
	Z_message("OK\n");
	
	
	Z_message("Initializing mouse...");
	//{ Mouse setup
	scare_mouse();
	set_mouse_sprite(NULL);
	byte mousepx[16][16] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 0, }, { 0, 241, 242, 243, 245, 245, 245, 245, 245, 245, 245, 245, 243, 241, 0, 0, }, { 0, 0, 241, 242, 243, 243, 245, 245, 245, 245, 245, 245, 243, 241, 0, 0, }, { 0, 0, 0, 241, 242, 243, 243, 243, 245, 245, 245, 243, 241, 0, 0, 0, }, { 0, 0, 0, 0, 241, 242, 243, 243, 243, 243, 245, 243, 241, 0, 0, 0, }, { 0, 0, 0, 0, 0, 241, 242, 243, 243, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 241, 242, 243, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 241, 242, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 241, 242, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, } };
	for(auto x = 0; x < 16; ++x)
		for(auto y = 0; y < 16; ++y)
		{
			putpixel(mouse_bmp, x, y, cursorColor(mousepx[x][y]));
		}
	set_mouse_sprite(mouse_bmp);
	unscare_mouse();
	show_mouse(screen);
	//}
	Z_message("OK\n");
	
	set_window_title("ZQuest Launcher");
	set_close_button_callback((void (*)()) hit_close_button);
	//
	Z_message("Launcher opened successfully.\n");
	#if QUICK_EXIT > 0
	goto exit;
	#endif
	
	LauncherDialog().show();

#if QUICK_EXIT > 0
exit:
#endif
	Z_message("Exiting launcher...\n");
	//
	
	flush_config_file();
	allegro_exit();
	return 0;
}
END_OF_MAIN()

//Things required to compile from shared files... le sigh -Em
bool is_large = true; //scaling
bool is_zquest() //Used for sizing purposes, romview and zquest both also return true.
{
	return true;
}

//{ Nil funcs

int32_t d_alltriggerbutton_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_comboa_radio_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_comboabutton_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssdn_btn_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssdn_btn2_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssdn_btn3_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssdn_btn4_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_sslt_btn_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_sslt_btn2_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_sslt_btn3_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_sslt_btn4_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssrt_btn_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssrt_btn2_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssrt_btn3_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssrt_btn4_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssup_btn_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssup_btn2_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssup_btn3_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_ssup_btn4_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_tri_edit_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_triggerbutton_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t dcs_auto_button_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_jbutton_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_kbutton_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_listen_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_savemidi_proc(int32_t, DIALOG*, int32_t)
{
	return D_O_K;
}

int32_t d_dummy_proc(int32_t,DIALOG *,int32_t)
{
	return D_O_K;
}

int32_t d_timer_proc(int32_t, DIALOG *, int32_t)
{
	return D_O_K;
}

void go()
{
	scare_mouse();
	blit(screen,tmp_scr,0,0,0,0,screen->w,screen->h);
	unscare_mouse();
}

void comeback()
{
	scare_mouse();
	blit(tmp_scr,screen,0,0,0,0,screen->w,screen->h);
	unscare_mouse();
}

void large_dialog(DIALOG *)
{
}

//}

void init_launcher_palette()
{
	get_palette(RAMpal);
	//{ Hardcoded instead of datafile
	for(auto q = 0; q < 224; ++q)
	{
		RAMpal[q].r = 16; RAMpal[q].g =  0; RAMpal[q].b = 16;
	}
	RAMpal[224].r =  0; RAMpal[224].g =  0; RAMpal[224].b =  0;
	RAMpal[225].r =  0; RAMpal[225].g =  0; RAMpal[225].b = 42;
	RAMpal[226].r =  0; RAMpal[226].g = 39; RAMpal[226].b =  0;
	RAMpal[227].r =  0; RAMpal[227].g = 42; RAMpal[227].b = 42;
	RAMpal[228].r = 44; RAMpal[228].g =  9; RAMpal[228].b =  9;
	RAMpal[229].r = 42; RAMpal[229].g =  0; RAMpal[229].b = 42;
	RAMpal[230].r = 41; RAMpal[230].g = 26; RAMpal[230].b =  2;
	RAMpal[231].r = 42; RAMpal[231].g = 42; RAMpal[231].b = 42;
	RAMpal[232].r = 21; RAMpal[232].g = 21; RAMpal[232].b = 21;
	RAMpal[233].r = 21; RAMpal[233].g = 21; RAMpal[233].b = 63;
	RAMpal[234].r = 21; RAMpal[234].g = 63; RAMpal[234].b = 21;
	RAMpal[235].r = 21; RAMpal[235].g = 63; RAMpal[235].b = 63;
	RAMpal[236].r = 63; RAMpal[236].g = 21; RAMpal[236].b = 21;
	RAMpal[237].r = 63; RAMpal[237].g = 21; RAMpal[237].b = 63;
	RAMpal[238].r = 63; RAMpal[238].g = 63; RAMpal[238].b = 21;
	RAMpal[239].r = 63; RAMpal[239].g = 63; RAMpal[239].b = 63;
	RAMpal[240].r =  0; RAMpal[240].g =  0; RAMpal[240].b = 32;
	RAMpal[241].r =  0; RAMpal[241].g =  0; RAMpal[241].b =  0;
	RAMpal[242].r = 16; RAMpal[242].g = 16; RAMpal[242].b = 16;
	RAMpal[243].r = 32; RAMpal[243].g = 32; RAMpal[243].b = 32;
	RAMpal[244].r = 52; RAMpal[244].g = 51; RAMpal[244].b = 49;
	RAMpal[245].r = 63; RAMpal[245].g = 63; RAMpal[245].b = 63;
	RAMpal[246].r = 63; RAMpal[246].g = 63; RAMpal[246].b = 55;
	RAMpal[247].r = 63; RAMpal[247].g = 55; RAMpal[247].b = 39;
	RAMpal[248].r =  0; RAMpal[248].g =  0; RAMpal[248].b = 19;
	RAMpal[249].r =  2; RAMpal[249].g =  8; RAMpal[249].b = 26;
	RAMpal[250].r =  8; RAMpal[250].g = 14; RAMpal[250].b = 31;
	RAMpal[251].r = 15; RAMpal[251].g = 21; RAMpal[251].b = 37;
	RAMpal[252].r = 21; RAMpal[252].g = 28; RAMpal[252].b = 42;
	RAMpal[253].r = 28; RAMpal[253].g = 35; RAMpal[253].b = 48;
	RAMpal[254].r = 34; RAMpal[254].g = 42; RAMpal[254].b = 53;
	RAMpal[255].r = 41; RAMpal[255].g = 49; RAMpal[255].b = 59;
	//}
	
	load_colorset(gui_colorset);
	
	set_palette(RAMpal);
	clear_to_color(screen,vc(0));
}

void fps_callback()
{
	lastfps=framecnt;
	framecnt=0;
}
END_OF_FUNCTION(fps_callback)

void myvsync_callback()
{
    ++myvsync;
}
END_OF_FUNCTION(myvsync_callback)

bool update_hw_pal = false;
void update_hw_screen()
{
	if(myvsync)
	{
		blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
		if(update_hw_pal) set_palette(RAMpal);
		update_hw_pal=false;
		myvsync=0;
	}
}


