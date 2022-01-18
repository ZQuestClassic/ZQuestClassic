// Launcher executable main file
#include "zdefs.h"
#include "launcher.h"
#include "module.h"
#include "zsys.h"
#include "fonts.h"
#include "dialog/alert.h"

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
BITMAP *mouse_bmp;
volatile bool close_button_quit = false;

void init_launcher_palette();
void fps_callback();

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
	zc_srand(time(0));
	
	
	set_uformat(U_ASCII);
	Z_message("Initializing Allegro... "); //{
	allegro_init();
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
		Z_error("couldn't allocate timer");
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
	
	Z_message("Fonts.Dat..."); //{
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
	
	//} end Loading data files:
	
	set_color_depth(8);
	int32_t videofail = set_gfx_mode(GFX_AUTODETECT_WINDOWED,zq_screen_w,zq_screen_h,0,0);
	
	if(videofail)
	{
		Z_error(allegro_error);
		QUIT_LAUNCHER();
	}
	
	Z_message("Loading bitmaps...");
	tmp_scr = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
	mouse_bmp = create_bitmap_ex(8,16,16);
	
	if(!(tmp_scr && mouse_bmp))
	{
		Z_error("failed\n");
		QUIT_LAUNCHER();
	}
	Z_message("OK\n");
	
	init_launcher_palette();
	
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
	
	set_window_title("ZQuest Launcher");
	set_close_button_callback((void (*)()) hit_close_button);
	//
	
	while(true)
	{
		//InfoDialog("Info","Hello! This is a popup!!!").show();
		if(close_button_quit)
		{
			bool r = false;
			AlertDialog("Exit",
				"Are you sure?",
				[&](bool ret)
				{
					r = ret;
				}).show();
			close_button_quit = false;
			if(r)
				break;
		}
	}
	
	//
	
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

RGB _RGB(byte *si)
{
	RGB x;
	x.r = si[0];
	x.g = si[1];
	x.b = si[2];
	x.filler=0;
	return x;
}

RGB _RGB(int32_t r,int32_t g,int32_t b)
{
	RGB x;
	x.r = r;
	x.g = g;
	x.b = b;
	x.filler=0;
	return x;
}

RGB invRGB(RGB s)
{
	RGB x;
	x.r = 63-s.r;
	x.g = 63-s.g;
	x.b = 63-s.b;
	x.filler=0;
	return x;
}

RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio)
{
	RGB x;
	x.r = (r1*(64-ratio) + r2*ratio) >> 6;
	x.g = (g1*(64-ratio) + g2*ratio) >> 6;
	x.b = (b1*(64-ratio) + b2*ratio) >> 6;
	x.filler=0;
	return x;
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
	//{Theme stuff
	RAMpal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
	RAMpal[dvc(2)] = _RGB(66*63/255,  65*63/255,  66*63/255);
	RAMpal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
	RAMpal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
	RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
	RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
	RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
	RAMpal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
	
	byte palrstart= 10*63/255, palrend=166*63/255,
		 palgstart= 36*63/255, palgend=202*63/255,
		 palbstart=106*63/255, palbend=240*63/255,
		 paldivs=7;
		 
	for(int32_t i=0; i<paldivs; i++)
	{
		RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
		RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
		RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
	}
	
	jwin_pal[jcBOX]	=dvc(4);
	jwin_pal[jcLIGHT]  =dvc(5);
	jwin_pal[jcMEDLT]  =dvc(4);
	jwin_pal[jcMEDDARK]=dvc(3);
	jwin_pal[jcDARK]   =dvc(2);
	jwin_pal[jcBOXFG]  =dvc(1);
	jwin_pal[jcTITLEL] =dvc(9);
	jwin_pal[jcTITLER] =dvc(15);
	jwin_pal[jcTITLEFG]=dvc(7);
	jwin_pal[jcTEXTBG] =dvc(5);
	jwin_pal[jcTEXTFG] =dvc(1);
	jwin_pal[jcSELBG]  =dvc(8);
	jwin_pal[jcSELFG]  =dvc(6);
	jwin_pal[jcCURSORMISC] = dvc(1);
	jwin_pal[jcCURSOROUTLINE] = dvc(2);
	jwin_pal[jcCURSORLIGHT] = dvc(3);
	jwin_pal[jcCURSORDARK] = dvc(5);
	
	gui_bg_color=jwin_pal[jcBOX];
	gui_fg_color=jwin_pal[jcBOXFG];
	gui_mg_color=jwin_pal[jcMEDDARK];
	jwin_set_colors(jwin_pal);
	//}
	set_palette(RAMpal);
	clear_to_color(screen,vc(0));
}

void fps_callback()
{
	lastfps=framecnt;
	framecnt=0;
}
END_OF_FUNCTION(fps_callback)




