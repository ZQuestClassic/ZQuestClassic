#include "zalleg/zalleg.h"
#include "base/zdefs.h"
#include "launcher/launcher.h"
#include "base/module.h"
#include "base/fonts.h"
#include "base/render.h"
#include "dialog/alert.h"
#include "launcher/launcher_dialog.h"
#include "base/process_management.h"
#include "base/zapp.h"
#include "fontsdat.h"
#include "zinfo.h"
#include "zq/render_tooltip.h"

#define QUICK_EXIT 0


#define QUIT_LAUNCHER() \
do{ \
	return 1; \
} \
while(false)

PALETTE RAMpal;
COLOR_MAP trans_table, trans_table2;

volatile int32_t lastfps = 0;
volatile int32_t framecnt = 0;
int32_t joystick_index = 0;
int32_t readsize = 0, writesize = 0;
volatile int32_t myvsync=0;
int32_t zq_screen_w=LARGE_W;
int32_t zq_screen_h=LARGE_H;
BITMAP *tmp_scr;
BITMAP *mouse_bmp;
int32_t gui_colorset = 99;

char temppath[4096] = {0}, rootpath[4096] = {0};


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
void load_mouse()
{
	gui_mouse_focus=0;
	MouseSprite::set(-1);
	byte mousepx[16][16] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 242, 0, }, { 0, 241, 242, 243, 245, 245, 245, 245, 245, 245, 245, 245, 243, 241, 0, 0, }, { 0, 0, 241, 242, 243, 243, 245, 245, 245, 245, 245, 245, 243, 241, 0, 0, }, { 0, 0, 0, 241, 242, 243, 243, 243, 245, 245, 245, 243, 241, 0, 0, 0, }, { 0, 0, 0, 0, 241, 242, 243, 243, 243, 243, 245, 243, 241, 0, 0, 0, }, { 0, 0, 0, 0, 0, 241, 242, 243, 243, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 241, 242, 243, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 241, 242, 243, 243, 241, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 241, 242, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, } };
	for(auto x = 0; x < 16; ++x)
		for(auto y = 0; y < 16; ++y)
		{
			putpixel(mouse_bmp, x, y, cursorColor(mousepx[x][y]));
		}
	MouseSprite::assign(0, mouse_bmp);
	MouseSprite::set(0);
}

void hit_close_button()
{
    close_button_quit=true;
    return;
}

int32_t main(int32_t argc, char* argv[])
{
	zalleg_setup_allegro(App::launcher, argc, argv);

	if (used_switch(argc, argv, "-update"))
	{
		std::string output;
		bool success = run_and_get_output(ZUPDATER_FILE, {"-headless", "-cache"}, output);
		success &= output.find("Success!") != std::string::npos;
		printf("%s\n", output.c_str());
		exit(success ? 0 : 1);
	}

	zc_srand(time(0));

	if(install_timer() < 0
		|| install_keyboard() < 0
		|| install_mouse() < 0)
	{
		Z_error_fatal(allegro_error);
		QUIT_LAUNCHER();
	}
	
	LOCK_VARIABLE(lastfps);
	
	LOCK_VARIABLE(framecnt);
	LOCK_FUNCTION(fps_callback);
	
	if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
	{
		Z_error_fatal("couldn't allocate timer\n");
		QUIT_LAUNCHER();
	}
	LOCK_VARIABLE(myvsync);
	LOCK_FUNCTION(myvsync_callback);
	if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60)))
	{
		Z_error_fatal("couldn't allocate timer\n");
		QUIT_LAUNCHER();
	}
	
	LOCK_VARIABLE(dclick_status);
	LOCK_VARIABLE(dclick_time);
	lock_dclick_function();
	install_int(dclick_check, 20);
	
	set_gfx_mode(GFX_TEXT,80,50,0,0);
	
	set_color_depth(8);

	int window_width = zc_get_config("ZLAUNCH", "window_width", -1);
	int window_height = zc_get_config("ZLAUNCH", "window_height", -1);
	auto [w, h] = zc_get_default_display_size(zq_screen_w, zq_screen_h, window_width, window_height, 2);
	int32_t videofail = set_gfx_mode(GFX_AUTODETECT_WINDOWED,w,h,zq_screen_w, zq_screen_h);
	
	if(videofail)
	{
		Z_error_fatal(allegro_error);
		QUIT_LAUNCHER();
	}

	set_window_title("ZQuest Classic Launcher");
	zalleg_create_window();

	Z_message("Loading bitmaps..."); //{
	tmp_scr = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
	mouse_bmp = create_bitmap_ex(8,16,16);
	
	if(!(tmp_scr && mouse_bmp && screen))
	{
		Z_error_fatal("failed\n");
		QUIT_LAUNCHER();
	}
	Z_message("OK\n");
	//}
	
	// while(!key[KEY_SPACE]);
	
	Z_message("Loading configs...");
	gui_colorset = zc_get_config("ZLAUNCH","gui_colorset",99);
	Z_message("OK\n");
	
	Z_message("Initializing palette...");
	init_launcher_palette();
	Z_message("OK\n");
	
	
	Z_message("Initializing mouse...");
	//{ Mouse setup
	load_mouse();
	//}
	Z_message("OK\n");
	
	get_root_path(rootpath, 4096);
	
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
	return 0;
}
END_OF_MAIN()

//Things required to compile from shared files... le sigh -Em
bool is_compact = false;
bool DragAspect = true;
double aspect_ratio = LARGE_H / double(LARGE_W);
int window_min_width = 0, window_min_height = 0;
bool is_editor() //Used for sizing purposes
{
	return true;
}

bool is_reserved_key(int c)
{
	switch(c)
	{
		case KEY_ESC:
			return true;
	}
	return false;
}
bool is_reserved_keycombo(int c, int modflag)
{
	if(c==KEY_F4 && (modflag&KB_ALT_FLAG))
		return true;
	return false;
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
	blit(screen,tmp_scr,0,0,0,0,screen->w,screen->h);
}

void comeback()
{
	blit(tmp_scr,screen,0,0,0,0,screen->w,screen->h);
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
	
	zc_set_palette(RAMpal);
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

static RenderTreeItem rti_root("root");
static LegacyBitmapRTI rti_screen("screen");

static int zc_gui_mouse_x()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().first;
	return rti_screen.world_to_local(mouse_x, mouse_y).first;
}

static int zc_gui_mouse_y()
{
	if (rti_dialogs.has_children())
		return rti_dialogs.get_children().back()->rel_mouse().second;
	return rti_screen.world_to_local(mouse_x, mouse_y).second;
}

bool use_linear_bitmaps()
{
	static bool value = zc_get_config("ZLAUNCH", "scaling_mode", 0) == 1;
	return value;
}
static void init_render_tree()
{
	if (rti_root.has_children())
		return;
	
	if (use_linear_bitmaps())
		al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE | ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR);
	else
		al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE);
	rti_screen.bitmap = al_create_bitmap(screen->w, screen->h);
	rti_screen.a4_bitmap = screen;
	rti_screen.set_size(screen->w, screen->h);

	rti_root.add_child(&rti_screen);
	rti_root.add_child(&rti_dialogs);

	gui_mouse_x = zc_gui_mouse_x;
	gui_mouse_y = zc_gui_mouse_y;

	al_set_new_bitmap_flags(0);
	
	_init_render(al_get_bitmap_format(rti_screen.bitmap));
}

static void configure_render_tree()
{
	int resx = al_get_display_width(all_get_display());
	int resy = al_get_display_height(all_get_display());

	rti_root.visible = true;

	{
		static bool scaling_force_integer = zc_get_config("ZLAUNCH", "scaling_force_integer", 0) == 1;

		int w = al_get_bitmap_width(rti_screen.bitmap);
		int h = al_get_bitmap_height(rti_screen.bitmap);
		float xscale = (float)resx/w;
		float yscale = (float)resy/h;
		if (scaling_force_integer)
		{
			xscale = std::max((int) xscale, 1);
			yscale = std::max((int) yscale, 1);
		}
		rti_screen.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		rti_screen.visible = true;
		// TODO: it seems `rti_screen` is unnecessary, given this `rti_dialogs` also draws the main Window component.
		rti_dialogs.set_transform({
			.x = (int)(resx - w*xscale) / 2,
			.y = (int)(resy - h*yscale) / 2,
			.xscale = xscale,
			.yscale = yscale,
		});
		rti_dialogs.visible = true;
	}
	reload_dialog_tint();
}

static void render_launcher()
{
	ALLEGRO_STATE oldstate;
	al_store_state(&oldstate, ALLEGRO_STATE_TARGET_BITMAP);
	
	init_render_tree();
	configure_render_tree();

	al_set_target_backbuffer(all_get_display());
	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	render_tree_draw(&rti_root);

	al_flip_display();
	al_restore_state(&oldstate);
}

bool update_hw_pal = false;
void update_hw_screen(bool force)
{
	if(force || myvsync)
	{
		zc_process_display_events();
		if(update_hw_pal)
		{
			zc_set_palette(RAMpal);
			load_mouse();
		}
		update_hw_pal=false;
		if (force || myvsync)
			render_launcher();
	}
}

bool getname_nogo(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename)
{
    if(def!=temppath)
        strcpy(temppath,def);
        
    if(!usefilename)
    {
        int32_t i=(int32_t)strlen(temppath);
        
        while(i>=0 && temppath[i]!='\\' && temppath[i]!='/')
            temppath[i--]=0;
    }
    
    //  int32_t ret = file_select_ex(prompt,temppath,ext,255,-1,-1);
    int32_t ret=0;
    int32_t sel=0;
    
    if(list==NULL)
    {
        ret = jwin_file_select_ex(prompt,temppath,ext,2048,-1,-1,get_zc_font(font_lfont));
    }
    else
    {
        ret = jwin_file_browse_ex(prompt, temppath, list, &sel, 2048, -1, -1, get_zc_font(font_lfont));
    }
    
    return ret!=0;
}

bool getname(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename)
{
    go();
    int32_t ret=0;
    ret = getname_nogo(prompt,ext,list,def,usefilename);
    comeback();
    return ret != 0;
}

void clear_tooltip()
{
	
}
void ttip_uninstall_all()
{
	
}

// TODO: I experimented with making zcbase/zcgui shared/object libraries, and the followed was needed to compile zlauncher:
// The following is needed because of ~mapscr, which is in zcbase.
// void ffcdata::solid_update(bool push) {}
// bool ffcdata::setSolid(bool set) {return false;}
// void ffcdata::updateSolid() {}
// void ffcdata::doContactDamage(int32_t hdir) {}

// bool solid_object::setSolid(bool set) {return false;}
// bool solid_object::getSolid() const {return false;}
// void solid_object::updateSolid() {}
// bool solid_object::collide(solid_object const* other) const {return false;}
// bool solid_object::collide(zfix tx, zfix ty, zfix tw, zfix th) const {return false;}
// void solid_object::solid_update(bool push) {}
// void solid_object::solid_push(solid_object* pusher) {}

// // Needed for other things in zcbase to resolve.
// void replay_step_comment(std::string comment) {}
// bool fake_pack_writing=false;
// bool replay_is_active() {return false;}
// bool replay_version_check(int min, int max) {return false;}
