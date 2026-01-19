#include "zc/zc_sys.h"

#include "allegro/gfx.h"
#include "allegro/gui.h"
#include "allegro/inline/draw.inl"
#include "allegro5/joystick.h"
#include "base/files.h"
#include "base/render.h"
#include "base/zdefs.h"
#include "zalleg/zalleg.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include <functional>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>
#include <map>
#include <filesystem>
#include <ctype.h>
#include <sstream>
#include "base/version.h"
#include "base/zc_alleg.h"
#include "gamedata.h"
#include "zc/frame_timings.h"
#include "zc/replay_upload.h"
#include "zc/zasm_pipeline.h"
#include "zc/zc_init.h"
#include "init.h"
#include "zc/replay.h"
#include "zc/cheats.h"
#include "zc/render.h"
#include "base/zc_math.h"
#include "base/zapp.h"
#include "dialog/control_binding.h"
#include "metadata/metadata.h"
#include "zc/zelda.h"
#include "zc/saves.h"
#include "tiles.h"
#include "base/colors.h"
#include "pal.h"
#include "base/zsys.h"
#include "base/qst.h"
#include "zc/zc_sys.h"
#include "play_midi.h"
#include "gui/jwin_a5.h"
#include "base/jwinfsel.h"
#include "base/gui.h"
#include "midi.h"
#include "subscr.h"
#include "zc/maps.h"
#include "sprite.h"
#include "zc/guys.h"
#include "zc/hero.h"
#include "zc/title.h"
#include "particles.h"
#include "sound/zcmusic.h"
#include "zc/ffscript.h"
#include "dialog/info.h"
#include "zc/combos.h"
#include "zc/jit.h"
#include "zc/zc_subscr.h"
#include <fmt/format.h>
#include "zconsole/ConsoleLogger.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include "music_playback.h"
#include "base/new_menu.h"
#include "base/files.h"
#include "iter.h"
#include "dialog/externs.h"
#include "advanced_music.h"

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

using namespace std::chrono_literals;

extern bool Playing;

extern byte monochrome_console;

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;
extern std::string loadlast;
byte use_dwm_flush;
byte use_save_indicator;
int32_t paused_midi_pos = 0;
byte midi_suspended = 0;
byte zc_192b163_warp_compatibility;
bool epilepsyFlashReduction;
signed char pause_in_background_menu_init = 0;
byte pause_in_background = 0;
bool is_sys_pal = false;
static bool load_control_called_this_frame;
extern PALETTE* hw_palette;
extern bool update_hw_pal;

extern bool kb_typing_mode; //script only, for disbaling key presses affecting Hero, etc. 

#ifdef ALLEGRO_LINUX
static  const char *samplepath = "samplesoundset/patches.dat";
#endif
char qst_files_path[2048];

extern TopMenu the_player_menu;
#ifdef _MSC_VER
#define getcwd _getcwd
#endif

bool zc_key_pressed();

#ifdef _WIN32

// This should only be necessary for MinGW, since it doesn't have a dwmapi.h. Add another #ifdef if you like.
extern "C"
{
	typedef HRESULT(WINAPI *t_DwmFlush)();
	typedef HRESULT(WINAPI *t_DwmIsCompositionEnabled)(BOOL *pfEnabled);
}

void do_DwmFlush()
{
	static HMODULE shell = LoadLibrary("dwmapi.dll");
	
	if(!shell)
		return;
		
	static t_DwmFlush flush=reinterpret_cast<t_DwmFlush>(GetProcAddress(shell, "DwmFlush"));
	static t_DwmIsCompositionEnabled isEnabled=reinterpret_cast<t_DwmIsCompositionEnabled>(GetProcAddress(shell, "DwmIsCompositionEnabled"));
	
	BOOL enabled;
	isEnabled(&enabled);
	
	if(isEnabled)
		flush();
}

#endif // _WIN32

void zc_exit(int code)
{
	extern CConsoleLoggerEx zscript_coloured_console;

	set_is_exiting();

	if (replay_get_mode() == ReplayMode::Record) replay_save();
	replay_stop();
	music_stop();
	kill_sfx();

	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		//restore user volume settings
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			master_volume(-1, ((int32_t)FFCore.usr_midi_volume));
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_DIGI_VOLUME)
		{
			master_volume((int32_t)(FFCore.usr_digi_volume), 1);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME)
		{
			emusic_volume = (int32_t)FFCore.usr_music_volume;
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_SFX_VOLUME)
		{
			sfx_volume = (int32_t)FFCore.usr_sfx_volume;
		}
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}
	cleanup_control_schemes();
	save_game_configs();

	zscript_coloured_console.kill();
	zasm_pipeline_shutdown();
	frame_timings_end();
	quit_game();

	Z_message("ZQuest Classic website: https://zquestclassic.com\n");
	Z_message("ZQuest Classic docs: https://docs.zquestclassic.com\n");

	allegro_exit();
	exit(code);
}

bool flash_reduction_enabled(bool check_qr)
{
	return (check_qr && get_qr(qr_EPILEPSY)) || epilepsyFlashReduction || replay_is_debug();
}

// Dialogue largening
void large_dialog(DIALOG *d)
{
	large_dialog(d, 1.5);
}

void large_dialog(DIALOG *d, float RESIZE_AMT)
{
	if(!d[0].d1)
	{
		d[0].d1 = 1;
		int32_t oldwidth = d[0].w;
		int32_t oldheight = d[0].h;
		int32_t oldx = d[0].x;
		int32_t oldy = d[0].y;
		d[0].x -= int32_t(d[0].w/RESIZE_AMT);
		d[0].y -= int32_t(d[0].h/RESIZE_AMT);
		d[0].w = int32_t(d[0].w*RESIZE_AMT);
		d[0].h = int32_t(d[0].h*RESIZE_AMT);
		
		for(int32_t i=1; d[i].proc !=NULL; i++)
		{
			// Place elements horizontally
			double xpc = ((double)(d[i].x - oldx) / (double)oldwidth);
			d[i].x = int32_t(d[0].x + (xpc*d[0].w));
			
			if(d[i].proc==d_bitmap_proc)
			{
				d[i].w *= 2;
			}
			else d[i].w = int32_t(d[i].w*RESIZE_AMT);
			
			// Place elements vertically
			double ypc = ((double)(d[i].y - oldy) / (double)oldheight);
			d[i].y = int32_t(d[0].y + (ypc*d[0].h));
			
			// Vertically resize elements
			if(d[i].proc == jwin_edit_proc || d[i].proc == jwin_check_proc || d[i].proc == jwin_checkfont_proc)
			{
				d[i].h = int32_t((double)d[i].h*1.5);
			}
			else if(d[i].proc == jwin_droplist_proc)
			{
				d[i].y += int32_t((double)d[i].h*0.25);
				d[i].h = int32_t((double)d[i].h*1.25);
			}
			else if(d[i].proc==d_bitmap_proc)
			{
				d[i].h *= 2;
			}
			else d[i].h = int32_t(d[i].h*RESIZE_AMT);
			
			// Fix frames
			if(d[i].proc == jwin_frame_proc)
			{
				d[i].x++;
				d[i].y++;
				d[i].w-=4;
				d[i].h-=4;
			}
		}
	}
	
	for(int32_t i=1; d[i].proc!=NULL; i++)
	{
		if(d[i].proc==jwin_slider_proc)
			continue;
			
		// Bigger font
		bool bigfontproc = (d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != jwin_list_proc);
		
		if(!d[i].dp2 && bigfontproc)
		{
			d[i].dp2 = get_zc_font(font_lfont_l);
		}
		else if(!bigfontproc)
		{
			((ListData *)d[i].dp)->font = &a4fonts[font_lfont_l];
		}
		
		// Make checkboxes work
		if(d[i].proc == jwin_check_proc)
			d[i].proc = jwin_checkfont_proc;
		else if(d[i].proc == jwin_radio_proc)
			d[i].proc = jwin_radiofont_proc;
	}
	
	jwin_center_dialog(d);
}

char cfg_sect[] = "zeldadx"; //We need to rename this.
char sfx_sect[] = "Volume";

int32_t d_dummy_proc(int32_t,DIALOG *,int32_t)
{
	return D_O_K;
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
bool checkcheat(Cheat cheat)
{
	if(active_control_scheme->cheatkeys[cheat][0] && zc_readkey(active_control_scheme->cheatkeys[cheat][0]))
		return true; //Main key pressed
	if(active_control_scheme->cheatkeys[cheat][1] && zc_readkey(active_control_scheme->cheatkeys[cheat][1]))
		return true; //Alt key pressed
	return false;
}

static bool loaded_game_configs;

void load_game_configs()
{
	loaded_game_configs = true;
	
	epilepsyFlashReduction = zc_get_config(cfg_sect,"epilepsy_flash_reduction",0);
	
	midi_volume = zc_get_config(sfx_sect,"midi",255);
	sfx_volume = zc_get_config(sfx_sect,"sfx",255);
	emusic_volume = zc_get_config(sfx_sect,"emusic",255);
	pan_style = zc_get_config(sfx_sect,"pan",1);
	volkeys = zc_get_config(sfx_sect,"volkeys",0)!=0;
	zc_vsync = zc_get_config(cfg_sect,"vsync",0);
	Throttlefps = zc_get_config(cfg_sect,"throttlefps",1)!=0;
	Maxfps = zc_get_config(cfg_sect,"maxfps",0);
	TransLayers = zc_get_config(cfg_sect,"translayers",1)!=0;
	SnapshotFormat = zc_get_config(cfg_sect,"snapshot_format",3);
	ShowBottomPixels = zc_get_config(cfg_sect,"bottom_8_px",0);
	SnapshotScale = zc_get_config(cfg_sect,"snapshot_scale",2);
	NameEntryMode = zc_get_config(cfg_sect,"name_entry_mode",0);
#ifdef __EMSCRIPTEN__
	if (em_is_mobile()) NameEntryMode = 2;
#endif
	ShowFPS = zc_get_config(cfg_sect,"showfps",0)!=0;
	ShowGameTime = zc_get_config(cfg_sect,"showtime",0);
	NESquit = zc_get_config(cfg_sect,"fastquit",0)!=0;
	ClickToFreeze = zc_get_config(cfg_sect,"clicktofreeze",1)!=0;
	abc_patternmatch = zc_get_config(cfg_sect, "lister_pattern_matching", 1);
	pause_in_background = zc_get_config(cfg_sect, "pause_in_background", 0);
	
	window_width = resx = zc_get_config(cfg_sect,"window_width",-1);
	window_height = resy = zc_get_config(cfg_sect,"window_height",-1);
	SaveDragResize = zc_get_config(cfg_sect,"save_drag_resize",0)!=0;
	DragAspect = zc_get_config(cfg_sect,"drag_aspect",0)!=0;
	SaveWinPos = zc_get_config(cfg_sect,"save_window_position",0)!=0;
	scaleForceInteger = zc_get_config("zeldadx","scaling_force_integer",1)!=0;
	stretchGame = zc_get_config("zeldadx","stretch_game_area",0)!=0;
	
	loadlast = zc_get_config(cfg_sect,"load_last_path","");
	
	fullscreen = zc_get_config(cfg_sect,"fullscreen",0);
	
	info_opacity = zc_get_config("zc","debug_info_opacity",255);
#ifdef _WIN32
	console_enabled = (byte) zc_get_config("CONSOLE", "enabled", 0);
	//use_win7_keyboard_fix = (byte) zc_get_config(cfg_sect,"use_win7_key_fix",0);
	use_win32_proc = (byte) zc_get_config(cfg_sect,"zc_win_proc_fix",0); //buggy
   
	// This one's for Aero
	use_dwm_flush = (byte) zc_get_config("zeldadx","use_dwm_flush",0);
   
	monochrome_console = (byte) zc_get_config("CONSOLE","monochrome_debuggers",0);
#else //UNIX
	console_enabled = (byte) zc_get_config("CONSOLE", "enabled", 0);
	monochrome_console = (byte) zc_get_config("CONSOLE","monochrome_debuggers",0);
#endif
	clearConsoleOnLoad = zc_get_config("CONSOLE","clear_console_on_load",1)!=0;
	clearConsoleOnReload = zc_get_config("CONSOLE","clear_console_on_reload",0)!=0;

	strcpy(qstdir,zc_get_config(cfg_sect,"quest_dir","quests"));
	strcpy(qstpath,qstdir); //qstpath is the local (for this run of ZC) quest path, qstdir is the universal quest dir.
	ss_enable = zc_get_config(cfg_sect, "ss_enable", 0) != 0;
	ss_seconds = zc_get_config(cfg_sect,"ss_seconds", 5 * 60);
	
	if (int old_ss_time = zc_get_config(cfg_sect, "ss_after", -99); old_ss_time != -99)
	{
		if(old_ss_time <= 0)
			ss_seconds = 5;
		else if(old_ss_time <= 3)
			ss_seconds = old_ss_time * 15;
		else if(old_ss_time <= 13)
			ss_seconds = (old_ss_time - 3) * 60;
		else if (ss_enable)
		{
			ss_enable = false;
			zc_set_config(cfg_sect, "ss_enable", 0);
		}
		zc_set_config(cfg_sect, "ss_after", nullptr);
		zc_set_config(cfg_sect, "ss_seconds", ss_seconds);
	}
	
	ss_speed = vbound(zc_get_config(cfg_sect,"ss_speed",2), 0, 6);
	ss_density = vbound(zc_get_config(cfg_sect,"ss_density",3), 0, 6);
	heart_beep = zc_get_config(cfg_sect,"heart_beep",0)!=0;
	//gui_colorset = zc_get_config(cfg_sect,"gui_colorset",0);
	fullscreen = zc_get_config(cfg_sect,"fullscreen",0);
	use_save_indicator = zc_get_config(cfg_sect,"save_indicator",0);
	zc_192b163_warp_compatibility = zc_get_config(cfg_sect,"zc_192b163_warp_compatibility",0);
	SkipTitle = zc_get_config(cfg_sect,"skip_title",0);
}

void save_game_configs()
{
	if (!loaded_game_configs) return;

	packfile_password("");
	
	if (all_get_display() && !all_get_fullscreen_flag()&& SaveWinPos)
	{
		int o_window_x, o_window_y;
		al_get_window_position(all_get_display(), &o_window_x, &o_window_y);
		zc_set_config(cfg_sect,"window_x",o_window_x);
		zc_set_config(cfg_sect,"window_y",o_window_y);
	}
	
	if (all_get_display() && !all_get_fullscreen_flag() && SaveDragResize)
	{
		window_width = al_get_display_width(all_get_display());
		window_height = al_get_display_height(all_get_display());
		zc_set_config(cfg_sect,"window_width",window_width);
		zc_set_config(cfg_sect,"window_height",window_height);
	}
	
	zc_set_config(cfg_sect,"load_last_path",loadlast.c_str());
	
	flush_config_file();
#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif
}

//----------------------------------------------------------------

// Timers

void fps_callback()
{
	lastfps=framecnt;
	framecnt=0;
}

END_OF_FUNCTION(fps_callback)

int32_t Z_init_timers()
{
	static bool didit = false;
	const static char *err_str = "Couldn't allocate timer";
	err_str = err_str; //Unused variable warning
	
	if(didit)
		return 1;
		
	didit = true;
	
	LOCK_VARIABLE(lastfps);
	LOCK_VARIABLE(framecnt);
	LOCK_FUNCTION(fps_callback);
	
	if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
		return 0;
		
	return 1;
}

void Z_remove_timers()
{
	remove_int(fps_callback);
}

//----------------------------------------------------------------

void dump_pal(BITMAP *dest)
{
	for(int32_t i=0; i<256; i++)
		rectfill(dest,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

//----------------------------------------------------------------

int game_mouse_index = ZCM_BLANK;
static bool system_mouse = false;
bool sys_mouse()
{
	system_mouse = true;
	return MouseSprite::set(ZCM_NORMAL);
}
bool game_mouse()
{
	system_mouse = false;
	return MouseSprite::set(game_mouse_index);
}
void custom_mouse(BITMAP* bmp, int fx, int fy, bool sys_recolor, bool user_scale)
{
	if(!bmp)
		return;
	float scale = vbound(zc_get_config("zeldadx","cursor_scale_large",1.5),1.0,5.0);
	int scaledw = bmp->w*scale, scaledh = bmp->h*scale;
	if(bmp->w == scaledw && bmp->h == scaledh)
		user_scale = false;
	if(user_scale || sys_recolor)
	{
		if(!user_scale) scale = 1;
		BITMAP* tmpbmp = create_bitmap_ex(8,bmp->w*scale,bmp->h*scale);
		if(user_scale)
			stretch_blit(bmp, tmpbmp, 0, 0, bmp->w, bmp->h, 0, 0, tmpbmp->w, tmpbmp->h);
		else
			blit(bmp, tmpbmp, 0, 0, 0, 0, bmp->w, bmp->h);
		if(sys_recolor)
			recolor_mouse(tmpbmp);
		MouseSprite::assign(ZCM_CUSTOM, tmpbmp, fx*scale, fy*scale);
		destroy_bitmap(tmpbmp);
	}
	else
	{
		MouseSprite::assign(ZCM_CUSTOM, bmp, fx, fy);
	}
}

void refresh_pal_mouse()
{
	if (is_sys_pal)
	{
		system_pal(true);
		sys_mouse();
	}
	else
	{
		game_pal();
		game_mouse();
	}
}

//Handles converting the mouse sprite from the .dat file
void recolor_mouse(BITMAP* bmp)
{
	for(int32_t x = 0; x < bmp->w; ++x)
	{
		for(int32_t y = 0; y < bmp->h; ++y)
		{
			int32_t color = getpixel(bmp, x, y);
			switch(color)
			{
				case dvc(1):
					color = jwin_pal[jcCURSORMISC];
					break;
				case dvc(2):
					color = jwin_pal[jcCURSOROUTLINE];
					break;
				case dvc(3):
					color = jwin_pal[jcCURSORLIGHT];
					break;
				case dvc(5):
					color = jwin_pal[jcCURSORDARK];
					break;
				default:
					continue;
			}
			putpixel(bmp, x, y, color);
		}
	}
}
void load_mouse()
{
	PALETTE pal;
	BITMAP* cursor_bitmap = load_bitmap("assets/cursor.bmp", pal);
	if (!cursor_bitmap)
		Z_error_fatal("Missing required file %s\n", "assets/cursor.bmp");

	enter_sys_pal();
	MouseSprite::set(-1);
	float scale = vbound(zc_get_config("zeldadx","cursor_scale_large",1.5),1.0,5.0);
	int32_t sz = 16*scale;
	for(int32_t j = 0; j < 1; ++j)
	{
		BITMAP* tmpbmp = create_bitmap_ex(8,16,16);
		if(zcmouse[j])
			destroy_bitmap(zcmouse[j]);
		zcmouse[j] = create_bitmap_ex(8,sz,sz);
		clear_bitmap(zcmouse[j]);
		clear_bitmap(tmpbmp);
		blit(cursor_bitmap,tmpbmp,1,j*17+1,0,0,16,16);
		recolor_mouse(tmpbmp);
		if(sz!=16)
			stretch_blit(tmpbmp, zcmouse[j], 0, 0, 16, 16, 0, 0, sz, sz);
		else
			blit(tmpbmp, zcmouse[j], 0, 0, 0, 0, 16, 16);
		destroy_bitmap(tmpbmp);
	}
	if(!hw_palette) hw_palette = &RAMpal;
	zc_set_palette(*hw_palette);
	
	BITMAP* blankmouse = create_bitmap_ex(8,16,16);
	clear_bitmap(blankmouse);
	
	MouseSprite::assign(ZCM_NORMAL, zcmouse[0], 1*scale, 1*scale);
	MouseSprite::assign(ZCM_BLANK, blankmouse);
	//Don't assign ZCM_CUSTOM. That'll be handled by scripts.
	
	//Reload the mouse
	if(system_mouse)
		sys_mouse();
	else game_mouse();
	
	destroy_bitmap(blankmouse);
	destroy_bitmap(cursor_bitmap);
	exit_sys_pal();
}

// sets the video mode and initializes the palette and mouse sprite
bool game_vid_mode(int32_t mode,int32_t wait)
{
	if (is_headless())
		return true;

	extern int zq_screen_w, zq_screen_h;
	if(set_gfx_mode(mode,resx,resy,zq_screen_w,zq_screen_h)!=0)
	{
		return false;
	}
	
	scrx = (resx-320)>>1;
	scry = (resy-240)>>1;
	for(int32_t q = 0; q < NUM_ZCMOUSE; ++q)
		zcmouse[q] = NULL;
	load_mouse();
	
	for(int32_t i=240; i<256; i++)
		RAMpal[i]=pal_gui[i];
		
	zc_set_palette(RAMpal);
	clear_to_color(screen,BLACK);
	
	rest(wait);
	return true;
}

void null_quest()
{
	std::string title_assets_path = "modules/classic/title_gfx.dat";
	if (get_last_loaded_qstpath() == title_assets_path)
		return;

	byte skip_flags[4];
	for (int i = 0; i < skip_max; i++)
		set_bit(skip_flags, i, 1);
	set_bit(skip_flags, skip_tiles, 0);
	set_bit(skip_flags, skip_csets, 0);
	set_bit(skip_flags, skip_misc, 0); // needed for miscsfx (skip this and `tests/replays/demons_inferno/demons_inferno_1_of_2.zplay` fails).
	loadquest(title_assets_path.c_str(), &QHeader, &QMisc, tunes+ZC_MIDI_COUNT, false, skip_flags, 0, false);
	if (!sfxdat)
	{
		sfxdat = 1;
		setupsfx(); // reload default sfx from sfxdat
	}
	// TODO: sfx.dat is ~1.2 MB. Could be better to break that up into individual files and load on demand / not at startup.
	// TODO: can we cache the tiles/colordata so we don't have to read title_gfx.dat more than once?
	//       colordata is tiny, but tilebuf is huge, so limit that to just what the title screen needs.
	//       Another option: embed this data into the binary (`xxd -i resources/modules/classic/title_gfx.dat > title_gfx.h`)
}

void init_NES_mode()
{
	null_quest();
}

//----------------------------------------------------------------

qword trianglelines[16]=
{
	0x0000000000000000ULL,
	0xFD00000000000000ULL,
	0xFDFD000000000000ULL,
	0xFDFDFD0000000000ULL,
	0xFDFDFDFD00000000ULL,
	0xFDFDFDFDFD000000ULL,
	0xFDFDFDFDFDFD0000ULL,
	0xFDFDFDFDFDFDFD00ULL,
	0xFDFDFDFDFDFDFDFDULL,
	0x00FDFDFDFDFDFDFDULL,
	0x0000FDFDFDFDFDFDULL,
	0x000000FDFDFDFDFDULL,
	0x00000000FDFDFDFDULL,
	0x0000000000FDFDFDULL,
	0x000000000000FDFDULL,
	0x00000000000000FDULL,
};

word screen_triangles[29][32];

// the ULL suffixes are to prevent this warning:
// warning: integer constant is too large for "int32_t" type

qword triangles[4][16][8]= //[direction][value][line]
{
	{
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL,
			0xFD00000000000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFD000000000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFD0000000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFD00000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFD000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFD0000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFD00ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		}
	},
	{
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL,
			0x0000000000000000ULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL,
			0x00000000000000FDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL,
			0x000000000000FDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x0000000000FDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x00000000FDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x000000FDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		}
	},
	{
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL
		},
		{
			0x0000000000000000ULL,
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL
		},
		{
			0xFD00000000000000ULL,
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFD000000000000ULL,
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFD0000000000ULL,
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFD00000000ULL,
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFD000000ULL,
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFD0000ULL,
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFD00ULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		}
	},
	{
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL
		},
		{
			0x0000000000000000ULL,
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL
		},
		{
			0x0000000000000000ULL,
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL
		},
		{
			0x00000000000000FDULL,
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x000000000000FDFDULL,
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x0000000000FDFDFDULL,
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x00000000FDFDFDFDULL,
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x000000FDFDFDFDFDULL,
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x0000FDFDFDFDFDFDULL,
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0x00FDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		},
		{
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL,
			0xFDFDFDFDFDFDFDFDULL
		}
	}
};

static bool is_opening_screen;
int32_t black_opening_count=0;
int32_t black_opening_x,black_opening_y;
int32_t black_opening_shape;

int32_t choose_opening_shape()
{
	// First, count how many bits are set
	int32_t numBits=0;
	int32_t bitCounter;
	
	for(int32_t i=0; i<bosMAX; i++)
	{
		if(COOLSCROLL&(1<<i))
			numBits++;
	}
	
	// Shouldn't happen...
	if(numBits==0)
		return bosCIRCLE;
		
	// Pick a bit
	bitCounter=zc_rand()%numBits+1;
	
	for(int32_t i=0; i<bosMAX; i++)
	{
		// If this bit is set, decrement the bit counter
		if(COOLSCROLL&(1<<i))
			bitCounter--;
			
		// When the counter hits 0, return a value based on
		// which bit it stopped on.
		// Reminder: enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosFADEBLACK, bosMAX};
		if(bitCounter==0)
			return i;
	}
	
	// Shouldn't be necessary, but the compiler might complain, at least
	return bosCIRCLE;
}

void close_black_opening(int32_t x, int32_t y, bool wait, int32_t shape)
{
	x -= viewport.x;
	y -= viewport.y;
	black_opening_shape= (shape>-1 ? shape : choose_opening_shape());
	
	int32_t w=framebuf->w, h=framebuf->h;
	int32_t blockrows=h/8, blockcolumns=32;
	int32_t xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
	
	for(int32_t blockrow=0; blockrow<blockrows; ++blockrow)  //30
	{
		for(int32_t blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
		{
			screen_triangles[blockrow][blockcolumn]=zc_max(abs(int32_t(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int32_t(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
		}
	}
	
	black_opening_count = 66;
	black_opening_x = x;
    black_opening_y = y;
	lensclk = 0;
	//black_opening_shape=(black_opening_shape+1)%bosMAX;
	
	
	if(black_opening_shape == bosFADEBLACK)
	{
		refreshTints();
		memcpy(tempblackpal, RAMpal, sizeof(RAMpal)); //Store palette in temp palette for fade effect
	}
	if(wait)
	{
		FFCore.warpScriptCheck();
		for(int32_t i=0; i<66; i++)
		{
			draw_screen();
			advanceframe(true);
			
			if(Quit)
			{
				break;
			}
		}
    }
}

void open_black_opening(int32_t x, int32_t y, bool wait, int32_t shape)
{
	x -= viewport.x;
	y -= viewport.y;
	black_opening_shape= (shape>-1 ? shape : choose_opening_shape());
	
	int32_t w=framebuf->w, h=framebuf->h;
	int32_t blockrows=h/8, blockcolumns=32;
	int32_t xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
	
	for(int32_t blockrow=0; blockrow<blockrows; ++blockrow)  //30
	{
		for(int32_t blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
		{
			screen_triangles[blockrow][blockcolumn]=zc_max(abs(int32_t(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int32_t(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
		}
	}
	
	black_opening_count = -66;
	black_opening_x = x;
    black_opening_y = y;
	lensclk = 0;
	if(black_opening_shape == bosFADEBLACK)
	{
		refreshTints();
		memcpy(tempblackpal, RAMpal, sizeof(RAMpal)); //Store palette in temp palette for fade effect
	}
	if(wait)
	{
		FFCore.warpScriptCheck();
		for(int32_t i=0; i<66; i++)
		{
			draw_screen();
			advanceframe(true);
			
			if(Quit)
			{
				break;
			}
		}
	}
}

void black_opening(BITMAP *dest,int32_t x,int32_t y,int32_t a,int32_t max_a)
{
	clear_to_color(tmp_scr,BLACK);
	int32_t w=dest->w, h=dest->h;
	
	switch(black_opening_shape)
	{
	case bosOVAL:
	{
		double new_w=(w/2)+abs(w/2-x);
		double new_h=(h/2)+abs(h/2-y);
		double b=sqrt(((new_w*new_w)/4)+(new_h*new_h));
		ellipsefill(tmp_scr,x,y,int32_t(2*a*b/max_a)/8*8,int32_t(a*b/max_a)/8*8,0);
		break;
	}
	
	case bosTRIANGLE:
	{
		double new_w=(w/2)+abs(w/2-x);
		double new_h=(h/2)+abs(h/2-y);
		double r=a*(new_w*sqrt((double)3)+new_h)/max_a;
		double P2= (PI/2);
		double P23=(2*PI/3);
		double P43=(4*PI/3);
		double Pa= (-4*PI*a/(3*max_a));
		double angle=P2+Pa;
		double a0=angle;
		double a2=angle+P23;
		double a4=angle+P43;
		triangle(tmp_scr, x+int32_t(zc::math::Cos(a0)*r), y-int32_t(zc::math::Sin(a0)*r),
				 x+int32_t(zc::math::Cos(a2)*r), y-int32_t(zc::math::Sin(a2)*r),
				 x+int32_t(zc::math::Cos(a4)*r), y-int32_t(zc::math::Sin(a4)*r),
				 0);
		break;
	}
	
	case bosSMAS:
	{
		int32_t distance=zc_max(abs(w/2-x),abs(h/2-y))/8;
		
		for(int32_t blockrow=0; blockrow<h/8; ++blockrow)  //30
		{
			for(int32_t linerow=0; linerow<8; ++linerow)
			{
				qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
				
				for(int32_t blockcolumn=0; blockcolumn<32; ++blockcolumn)  //40
				{
					*triangleline=triangles[(screen_triangles[blockrow][blockcolumn]&0xC000)>>14]
								  [zc_min(zc_max((((31+distance)*(max_a-a)/max_a)+((screen_triangles[blockrow][blockcolumn]&0x0FFF)-0x0100)-(15+distance)),0),15)]
								  [linerow];
					++triangleline;
				}
			}
		}
		
		break;
	}
	
	case bosFADEBLACK:
	{
		if(black_opening_count<0)
		{
			black_fade(zc_min(-black_opening_count,63));
		}
		else if(black_opening_count>0)
		{
			black_fade(63-zc_max(black_opening_count-3,0));
		}
		else black_fade(0);
		return; //no blitting from tmp_scr!
	}
	
	case bosCIRCLE:
	default:
	{
		double new_w=(w/2)+abs(w/2-x);
		double new_h=(h/2)+abs(h/2-y);
		int32_t r=int32_t(sqrt((new_w*new_w)+(new_h*new_h))*a/max_a);
		//circlefill(tmp_scr,x,y,a<<3,0);
		circlefill(tmp_scr,x,y,r,0);
		break;
	}
	}
	
	masked_blit(tmp_scr,dest,0,0,0,0,320,240);
}

// fadeamnt is 0-63
void black_fade(int32_t fadeamnt)
{
	fadeamnt = _rgb_scale_6[fadeamnt];
	for(int32_t i=0; i < 0xEF; i++)
	{
		RAMpal[i].r = vbound(tempblackpal[i].r-fadeamnt,0,255);
		RAMpal[i].g = vbound(tempblackpal[i].g-fadeamnt,0,255);
		RAMpal[i].b = vbound(tempblackpal[i].b-fadeamnt,0,255);
	}
	
	refreshpal = true;
}

//----------------------------------------------------------------

bool item_disabled(int32_t item)				 //is this item disabled?
{
	return (unsigned(item) < MAXITEMS && game->items_off[item] != 0);
}

bool can_use_item(int32_t item_type, int32_t item)				  //can Hero use this item?
{
	if(current_item(item_type, true) >=item)
	{
		return true;
	}
	
	return false;
}

bool has_item(int32_t item_type, int32_t it)						//does Hero possess this item?
{
	switch(item_type)
	{
		case itype_bomb:
		case itype_sbomb:
		{
			int32_t itemid = getItemID(itemsbuf, item_type, it);
			
			if(itemid == -1)
				return false;
				
			return (game->get_item(itemid));
		}
		
		case itype_clock:
		{
			int32_t itemid = getItemID(itemsbuf, item_type, it);
			
			if(itemid != -1 && (itemsbuf[itemid].flags & item_flag1)) //Active clock
				return (game->get_item(itemid));
			return Hero.getClock()?1:0;
		}
			
		case itype_key:
			return (game->get_keys()>0);
			
		case itype_magiccontainer:
			return (game->get_maxmagic()>=game->get_mp_per_block());
			
		case itype_triforcepiece:							   //it: -2=any, -1=current level, other=that level
		{
			switch(it)
			{
				case -2:
				{
					for(int32_t i=0; i<MAXLEVELS; i++)
					{
						if(game->lvlitems[i]&(1 << li_mcguffin))
						{
							return true;
						}
					}
					
					return false;
				}
				
				case -1:
					return (game->lvlitems[dlevel]&(1 << li_mcguffin));
					
				default:
					if(it>=0&&it<MAXLEVELS)
					{
						return (game->lvlitems[it]&(1 << li_mcguffin));
					}
					
					break;
			}
			
			return 0;
		}
		
		case itype_map:										 //it: -2=any, -1=current level, other=that level
		{
			switch(it)
			{
				case -2:
				{
					for(int32_t i=0; i<MAXLEVELS; i++)
					{
						if(game->lvlitems[i]&(1 << li_map))
						{
							return true;
						}
					}
					
					return false;
				}
				
				case -1:
					return (game->lvlitems[dlevel]&(1 << li_map))!=0;
					
				default:
					if(it>=0&&it<MAXLEVELS)
					{
						return (game->lvlitems[it]&(1 << li_map))!=0;
					}
					
					break;
			}
			
			return 0;
		}
		
		case itype_compass:									 //it: -2=any, -1=current level, other=that level
		{
			switch(it)
			{
				case -2:
				{
					for(int32_t i=0; i<MAXLEVELS; i++)
					{
						if(game->lvlitems[i]&(1 << li_compass))
						{
							return true;
						}
					}
					
					return false;
				}
				
				case -1:
					return (game->lvlitems[dlevel]&(1 << li_compass))!=0;
					
				default:
					if(it>=0&&it<MAXLEVELS)
					{
						return (game->lvlitems[it]&(1 << li_compass))!=0;
					}
					
					break;
			}
			return 0;
		}
		
		case itype_bosskey:									 //it: -2=any, -1=current level, other=that level
		{
			switch(it)
			{
				case -2:
				{
					for(int32_t i=0; i<MAXLEVELS; i++)
					{
						if(game->lvlitems[i]&(1 << li_boss_key))
						{
							return true;
						}
					}
					
					return false;
				}
				
				case -1:
					return (game->lvlitems[dlevel]&(1 << li_boss_key))?1:0;
					
				default:
					if(it>=0&&it<MAXLEVELS)
					{
						return (game->lvlitems[it]&(1 << li_boss_key))?1:0;
					}
					break;
			}
			return 0;
		}
		
		default:
			int32_t itemid = getItemID(itemsbuf, item_type, it);
			
			if(itemid == -1)
				return false;
				
			return game->get_item(itemid);
	}
}

int current_item(int item_type, bool checkmagic, bool jinx_check, bool check_bunny)
{
	switch(item_type)
	{
		case itype_clock:
		{
			int maxid = current_item_id(item_type, checkmagic, jinx_check, check_bunny);
			
			if(maxid != -1 && (itemsbuf[maxid].flags & item_flag1)) //Active clock
				return itemsbuf[maxid].level;
			
			return has_item(itype_clock,1) ? 1 : 0;
		}
			
		case itype_key:
			return game->get_keys();
			
		case itype_lkey:
			return game->lvlkeys[get_dlevel()];
			
		case itype_magiccontainer:
			return game->get_maxmagic()/game->get_mp_per_block();
			
		case itype_triforcepiece:
		{
			int count=0;
			
			for(int i=0; i<MAXLEVELS; i++)
			{
				count+=(game->lvlitems[i]&(1 << li_mcguffin))?1:0;
			}
			
			return count;
		}
		
		case itype_map:
		{
			int count=0;
			
			for(int i=0; i<MAXLEVELS; i++)
			{
				count+=(game->lvlitems[i]&(1 << li_map))?1:0;
			}
			
			return count;
		}
		
		case itype_compass:
		{
			int count=0;
			
			for(int i=0; i<MAXLEVELS; i++)
			{
				count+=(game->lvlitems[i]&(1 << li_compass))?1:0;
			}
			
			return count;
		}
		
		case itype_bosskey:
		{
			int count=0;
			
			for(int i=0; i<MAXLEVELS; i++)
			{
				count+=(game->lvlitems[i]&(1 << li_boss_key))?1:0;
			}
			
			return count;
		}
		
		default:
			int maxid = current_item_id(item_type, checkmagic, jinx_check, check_bunny);
			
			if(maxid == -1)
				return 0;
				
			return itemsbuf[maxid].level;
	}
}

std::map<int32_t, int32_t> itemcache;
std::map<int32_t, int32_t> itemcache_cost;

void removeFromItemCache(int32_t itemclass)
{
	itemcache.erase(itemclass);
	itemcache_cost.erase(itemclass);
	cache_tile_mod_clear();
}

void flushItemCache(bool justcost)
{
	itemcache_cost.clear();
	if(!justcost)
		itemcache.clear();
	else if(replay_version_check(0,19))
		return;

	cache_tile_mod_clear();
	
	//also fix the active subscreen if items were deleted -DD
	if(game != NULL)
	{
		verifyBothWeapons();
		refresh_subscr_items();
	}
}

// This is used often, so it should be as direct as possible.
int _c_item_id_internal(int itemtype, bool checkmagic, bool jinx_check, bool check_bunny)
{
	bool use_cost_cache = replay_version_check(19);
	if(jinx_check)
	{
		//special case for shields...
		if (itemtype == itype_shield && !HeroShieldClk())
			jinx_check = false;
		else if(!(HeroSwordClk() || HeroItemClk()))
			jinx_check = false; //not jinxed
	}
	if(!Hero.BunnyClock() || itemtype == itype_pearl) // bunny_check does not apply
		check_bunny = false;
	if(itemtype == itype_ring) checkmagic = true;
	if (!jinx_check && !check_bunny
		&& (use_cost_cache || itemtype != itype_ring))
	{
		auto& cache = checkmagic && use_cost_cache ? itemcache_cost : itemcache;
		auto res = cache.find(itemtype);
		
		if(res != cache.end())
			return res->second;
	}
	
	int result = -1;
	int highestlevel = -1;
	
	for(int i=0; i<MAXITEMS; i++)
	{
		if(game->get_item(i) && itemsbuf[i].type==itemtype && !item_disabled(i))
		{
			if(checkmagic && itemtype != itype_magicring)
				if(!checkmagiccost(i))
					continue;
			if(jinx_check && (usesSwordJinx(i) ? HeroSwordClk() : HeroItemClk()))
				if(!(itemsbuf[i].flags & item_jinx_immune))
					continue;
			if(check_bunny && !checkbunny(i))
				continue;
			
			if(itemsbuf[i].level >= highestlevel)
			{
				highestlevel = itemsbuf[i].level;
				result=i;
			}
		}
	}
	
	if(!(jinx_check || check_bunny)) //Can't cache jinx_check/check_bunny
	{
		if (use_cost_cache)
		{
			if (!checkmagic)
				itemcache[itemtype] = result;
			if (checkmagic || result < 0 || checkmagiccost(result))
				itemcache_cost[itemtype] = result;
		}
		else
		{
			itemcache[itemtype] = result;
		}
	}
	return result;
}

// 'jinx_check' indicates that the highest level item *immune to jinxes* should be returned.
int current_item_id(int itype, bool checkmagic, bool jinx_check, bool check_bunny)
{
	if(itype < 0 || itype >= itype_max) return -1;
	if(game->OverrideItems[itype] > -2)
	{
		auto ovid = game->OverrideItems[itype];
		if(ovid < 0 || ovid >= MAXITEMS)
			return -1;
		if(itemsbuf[ovid].type == itype)
		{
			if(itype == itype_magicring)
				checkmagic = false;
			else if(itype == itype_ring)
				checkmagic = true;
			
			if(checkmagic && !checkmagiccost(ovid))
				return -1;

			if (jinx_check && !checkitem_jinx(ovid))
			{
				return -1;
			}
			return ovid;
		}
	}
	auto ret = _c_item_id_internal(itype,checkmagic,jinx_check,check_bunny);
	if(!jinx_check) //If not already a jinx-immune-only check...
	{
		//And the player IS jinxed...
		if(HeroIsJinxed())
		{
			//Then do a jinx-immune-only check here
			auto ret2 = _c_item_id_internal(itype,checkmagic,true,check_bunny);
			//And *IF IT FINDS A VALID ITEM*, return that one instead! -Em
			//Should NOT need a compat rule, as this should always return -1 in old quests.
			if(ret2 > -1) return ret2;
		}
	}
	return ret;
}

int current_item_power(int itemtype, bool checkmagic, bool jinx_check, bool check_bunny)
{
	int result = current_item_id(itemtype, checkmagic, jinx_check, check_bunny);
	return (result<0) ? 0 : itemsbuf[result].power;
}

int32_t heart_container_id()
{
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(itemsbuf[i].type == itype_heartcontainer)
		{
			return i;
		}
	}
	return -1;
}

struct tilemod_cache_state_t
{
	bool operator==(const tilemod_cache_state_t&) const = default;

	bool valid;
	bool bunny_clock;
	bool superman;
	int shield;
};
tilemod_cache_state_t tilemod_cache_state;
int32_t tilemod_cache_value;

void cache_tile_mod_clear()
{
	tilemod_cache_state = {false};
}

int32_t item_tile_mod()
{
	tilemod_cache_state_t state = {
		.valid = true,
		.bunny_clock = Hero.BunnyClock() != 0,
		.superman = Hero.superman,
		.shield = Hero.active_shield_id,
	};
	if (tilemod_cache_state == state)
		return tilemod_cache_value;

	int32_t tile=0;
	bool check_bombcost = !get_qr(qr_BROKEN_BOMB_AMMO_COSTS);
	if(check_bombcost || game->get_bombs())
	{
		int32_t itemid = current_item_id(itype_bomb,check_bombcost);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(check_bombcost || game->get_sbombs())
	{
		int32_t itemid = current_item_id(itype_sbomb,check_bombcost);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_clock))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iClock
				: getHighestLevelEvenUnowned(itemsbuf, itype_clock);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_key))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iKey
				: getHighestLevelEvenUnowned(itemsbuf, itype_key);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_lkey))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iLevelKey
				: getHighestLevelEvenUnowned(itemsbuf, itype_lkey);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_map))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iMap
				: getHighestLevelEvenUnowned(itemsbuf, itype_map);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_compass))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iCompass
				: getHighestLevelEvenUnowned(itemsbuf, itype_compass);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_bosskey))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iBossKey
				: getHighestLevelEvenUnowned(itemsbuf, itype_bosskey);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_magiccontainer))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iMagicC
				: getHighestLevelEvenUnowned(itemsbuf, itype_magiccontainer);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	if(current_item(itype_triforcepiece))
	{
		int32_t itemid =
			get_qr(qr_HARDCODED_LITEM_LTMS)
				? iTriforce
				: getHighestLevelEvenUnowned(itemsbuf, itype_triforcepiece);
		if(itemid > -1 && checkbunny(itemid))
			tile+=itemsbuf[itemid].ltm;
	}
	
	for(int32_t i=0; i<itype_max; i++)
	{
		if(!get_qr(qr_HARDCODED_LITEM_LTMS))
		{
			switch(i)
			{
				case itype_bomb:
				case itype_sbomb:
				case itype_clock:
				case itype_key:
				case itype_lkey:
				case itype_map:
				case itype_compass:
				case itype_bosskey:
				case itype_magiccontainer:
				case itype_triforcepiece:
					continue; //already handled
			}
		}
		int32_t itemid = current_item_id(i,false);
		if(i == itype_shield)
			itemid = getCurrentShield(false);
		
		if(itemid < 0 || !checkbunny(itemid))
			continue;
		
		itemdata const& itm = itemsbuf[itemid];
		
		switch(itm.type)
		{
			case itype_shield:
				if(itm.flags & item_flag9) //active shield
				{
					if(!usingActiveShield(itemid))
					{
						tile+=itm.misc6; //'Inactive PTM'
						continue;
					}
				}
				break;
		}
		
		tile+=itm.ltm;
	}
	
	tilemod_cache_value = tile;
	tilemod_cache_state = state;
	return tile;
}

int32_t bunny_tile_mod()
{
	if(Hero.BunnyClock())
	{
		return game->get_bunny_ltm();
	}
	return 0;
}

// Hints are drawn on a separate layer to combo reveals.
// TODO: move out of zc_sys.cpp, weird place for this code.
void draw_lens_under(BITMAP *dest, bool layer)
{
	//Lens flag 1: Replacement for qr_LENSHINTS; if set, lens will show hints. Does nothing if flag 2 is set.
	//Lens flag 2: Disable "hints", prevent rendering of Secret Combos
	//Lens flag 3: Don't show armos/chest/dive items
	//Lens flag 4: Show Raft Paths
	//Lens flag 5: Show Invisible Enemies
	bool hints = (itemsbuf[Hero.getLastLensID()].flags & item_flag2) ? false : (layer && (itemsbuf[Hero.getLastLensID()].flags & item_flag1));
	
	int32_t strike_hint_table[11]=
	{
		mfARROW, mfBOMB, mfBRANG, mfWANDMAGIC,
		mfSWORD, mfREFMAGIC, mfHOOKSHOT,
		mfREFFIREBALL, mfHAMMER, mfSWORDBEAM, mfWAND
	};
	
	{
		int32_t blink_rate=flash_reduction_enabled()?6:1;
		int32_t tempitem, tempweapon=0;
		strike_hint=strike_hint_table[strike_hint_counter];
		
		if(strike_hint_timer>32)
		{
			strike_hint_timer=0;
			strike_hint_counter=((strike_hint_counter+1)%11);
		}
		
		++strike_hint_timer;

		for_every_visible_rpos_layer0([&](const rpos_handle_t& rpos_handle) {
			mapscr* scr = rpos_handle.scr;
			auto [x, y] = rpos_handle.xy();
			y += playing_field_offset;

			int32_t tempitemx=-16, tempitemy=-16;
			int32_t tempweaponx=-16, tempweapony=-16;
			
			for(int32_t iter=0; iter<2; ++iter)
			{
				int32_t checkflag=0;
				
				if(iter==0)
				{
					checkflag = rpos_handle.cflag();
				}
				else
				{
					checkflag = rpos_handle.sflag();
				}
				
				if(checkflag==mfSTRIKE)
				{
					if(!hints)
					{
						if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sSTRIKE],scr->secretcset[sSTRIKE]);
					}
					else
					{
						checkflag = strike_hint;
					}
				}
				
				switch(checkflag)
				{
					case 0:
					case mfZELDA:
					case mfPUSHED:
					case mfENEMY0:
					case mfENEMY1:
					case mfENEMY2:
					case mfENEMY3:
					case mfENEMY4:
					case mfENEMY5:
					case mfENEMY6:
					case mfENEMY7:
					case mfENEMY8:
					case mfENEMY9:
					case mfSINGLE:
					case mfSINGLE16:
					case mfNOENEMY:
					case mfTRAP_H:
					case mfTRAP_V:
					case mfTRAP_4:
					case mfTRAP_LR:
					case mfTRAP_UD:
					case mfNOGROUNDENEMY:
					case mfNOBLOCKS:
					case mfSCRIPT1:
					case mfSCRIPT2:
					case mfSCRIPT3:
					case mfSCRIPT4:
					case mfSCRIPT5:
					case mfSCRIPT6:
					case mfSCRIPT7:
					case mfSCRIPT8:
					case mfSCRIPT9:
					case mfSCRIPT10:
					case mfSCRIPT11:
					case mfSCRIPT12:
					case mfSCRIPT13:
					case mfSCRIPT14:
					case mfSCRIPT15:
					case mfSCRIPT16:
					case mfSCRIPT17:
					case mfSCRIPT18:
					case mfSCRIPT19:
					case mfSCRIPT20:
					case mfPITHOLE:
					case mfPITFALLFLOOR:
					case mfLAVA:
					case mfICE:
					case mfICEDAMAGE:
					case mfDAMAGE1:
					case mfDAMAGE2:
					case mfDAMAGE4:
					case mfDAMAGE8:
					case mfDAMAGE16:
					case mfDAMAGE32:
					case mfFREEZEALL:
					case mfFREZEALLANSFFCS:
					case mfFREEZEFFCSOLY:
					case mfSCRITPTW1TRIG:
					case mfSCRITPTW2TRIG:
					case mfSCRITPTW3TRIG:
					case mfSCRITPTW4TRIG:
					case mfSCRITPTW5TRIG:
					case mfSCRITPTW6TRIG:
					case mfSCRITPTW7TRIG:
					case mfSCRITPTW8TRIG:
					case mfSCRITPTW9TRIG:
					case mfSCRITPTW10TRIG:
					case mfTROWEL:
					case mfTROWELNEXT:
					case mfTROWELSPECIALITEM:
					case mfSLASHPOT:
					case mfLIFTPOT:
					case mfLIFTORSLASH:
					case mfLIFTROCK:
					case mfLIFTROCKHEAVY:
					case mfDROPITEM:
					case mfSPECIALITEM:
					case mfDROPKEY:
					case mfDROPLKEY:
					case mfDROPCOMPASS:
					case mfDROPMAP:
					case mfDROPBOSSKEY:
					case mfSPAWNNPC:
					case mfSWITCHHOOK:
					case mfSIDEVIEWLADDER:
					case mfSIDEVIEWPLATFORM:
					case mfNOENEMYSPAWN:
					case mfENEMYALL:
					case mfNOMIRROR:
					case mfUNSAFEGROUND:
					case mf168:
					case mf169:
					case mf170:
					case mf171:
					case mf172:
					case mf173:
					case mf174:
					case mf175:
					case mf176:
					case mf177:
					case mf178:
					case mf179:
					case mf180:
					case mf181:
					case mf182:
					case mf183:
					case mf184:
					case mf185:
					case mf186:
					case mf187:
					case mf188:
					case mf189:
					case mf190:
					case mf191:
					case mf192:
					case mf193:
					case mf194:
					case mf195:
					case mf196:
					case mf197:
					case mf198:
					case mf199:
					case mf200:
					case mf201:
					case mf202:
					case mf203:
					case mf204:
					case mf205:
					case mf206:
					case mf207:
					case mf208:
					case mf209:
					case mf210:
					case mf211:
					case mf212:
					case mf213:
					case mf214:
					case mf215:
					case mf216:
					case mf217:
					case mf218:
					case mf219:
					case mf220:
					case mf221:
					case mf222:
					case mf223:
					case mf224:
					case mf225:
					case mf226:
					case mf227:
					case mf228:
					case mf229:
					case mf230:
					case mf231:
					case mf232:
					case mf233:
					case mf234:
					case mf235:
					case mf236:
					case mf237:
					case mf238:
					case mf239:
					case mf240:
					case mf241:
					case mf242:
					case mf243:
					case mf244:
					case mf245:
					case mf246:
					case mf247:
					case mf248:
					case mf249:
					case mf250:
					case mf251:
					case mf252:
					case mf253:
					case mf254:
					case mfEXTENDED:
						break;
						
					case mfPUSHUD:
					case mfPUSHLR:
					case mfPUSH4:
					case mfPUSHU:
					case mfPUSHD:
					case mfPUSHL:
					case mfPUSHR:
					case mfPUSHUDNS:
					case mfPUSHLRNS:
					case mfPUSH4NS:
					case mfPUSHUNS:
					case mfPUSHDNS:
					case mfPUSHLNS:
					case mfPUSHRNS:
					case mfPUSHUDINS:
					case mfPUSHLRINS:
					case mfPUSH4INS:
					case mfPUSHUINS:
					case mfPUSHDINS:
					case mfPUSHLINS:
					case mfPUSHRINS:
						if(!hints && ((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&16))
									  || ((get_debug() && zc_getkey(KEY_N)) && (frame&16))))
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->undercombo,scr->undercset);
						}
						
						if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
								|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
						{
							if(hints)
							{
								switch (rpos_handle.ctype())
								{
								case cPUSH_HEAVY:
								case cPUSH_HW:
									tempitem=getItemIDPower(itemsbuf,itype_bracelet,1);
									tempitemx=x, tempitemy=y;
									
									if(tempitem>-1)
										putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
										
									break;
									
								case cPUSH_HEAVY2:
								case cPUSH_HW2:
									tempitem=getItemIDPower(itemsbuf,itype_bracelet,2);
									tempitemx=x, tempitemy=y;
									
									if(tempitem>-1)
										putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
										
									break;
								}
							}
						}
						
						break;
						
					case mfWHISTLE:
						if(hints)
						{
							tempitem=getItemID(itemsbuf,itype_whistle,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
						//Why is this here?
					case mfFAIRY:
					case mfMAGICFAIRY:
					case mfALLFAIRY:
						if(hints)
						{
							tempitem=getItemID(itemsbuf, itype_fairy,1);//iFairyMoving;
							
							if(tempitem < 0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfANYFIRE:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sBCANDLE],scr->secretcset[sBCANDLE]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_candle,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfSTRONGFIRE:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sRCANDLE],scr->secretcset[sRCANDLE]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_candle,2);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfMAGICFIRE:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sWANDFIRE],scr->secretcset[sWANDFIRE]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_wand,1);
							
							if(tempitem<0) break;
							
							tempweapon=wFire;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							else
							{
								tempweaponx=x;
								tempweapony=y;
							}
							
							putweapon(dest,tempweaponx,tempweapony,tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfDIVINEFIRE:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sDIVINEFIRE],scr->secretcset[sDIVINEFIRE]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_divinefire,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfARROW:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sARROW],scr->secretcset[sARROW]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_arrow,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfSARROW:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sSARROW],scr->secretcset[sSARROW]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_arrow,2);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfGARROW:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sGARROW],scr->secretcset[sGARROW]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_arrow,3);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfBOMB:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sBOMB],scr->secretcset[sBOMB]);
						}
						else
						{
							//tempitem=getItemID(itemsbuf,itype_bomb,1);
							tempweapon = wLitBomb;
							
							//if (tempitem<0) break;
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempweaponx=x;
								tempweapony=y;
							}
							
							putweapon(dest,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
						}
						
						break;
						
					case mfSBOMB:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sSBOMB],scr->secretcset[sSBOMB]);
						}
						else
						{
							//tempitem=getItemID(itemsbuf,itype_sbomb,1);
							//if (tempitem<0) break;
							tempweapon = wLitSBomb;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempweaponx=x;
								tempweapony=y;
							}
							
							putweapon(dest,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
						}
						
						break;
						
					case mfARMOS_SECRET:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
								putcombo(dest,x,y,scr->secretcombo[sSTAIRS],scr->secretcset[sSTAIRS]);
						}
						break;
						
					case mfBRANG:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
								putcombo(dest,x,y,scr->secretcombo[sBRANG],scr->secretcset[sBRANG]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_brang,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfMBRANG:
						if(!hints)
				{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sMBRANG],scr->secretcset[sMBRANG]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_brang,2);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfFBRANG:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sFBRANG],scr->secretcset[sFBRANG]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_brang,3);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfWANDMAGIC:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sWANDMAGIC],scr->secretcset[sWANDMAGIC]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_wand,1);
							
							if(tempitem<0) break;
							
							tempweapon=itemsbuf[tempitem].wpn3;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							else
							{
								tempweaponx=x;
								tempweapony=y;
								--lens_hint_weapon[wMagic][4];
								
								if(lens_hint_weapon[wMagic][4]<-8)
								{
									lens_hint_weapon[wMagic][4]=8;
								}
							}
							
							putweapon(dest,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfREFMAGIC:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sREFMAGIC],scr->secretcset[sREFMAGIC]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_shield,3);
							
							if(tempitem<0) break;
							
							tempweapon=ewMagic;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							else
							{
								tempweaponx=x;
								tempweapony=y;
								
								if(lens_hint_weapon[ewMagic][2]==up)
								{
									--lens_hint_weapon[ewMagic][4];
								}
								else
								{
									++lens_hint_weapon[ewMagic][4];
								}
								
								if(lens_hint_weapon[ewMagic][4]>8)
								{
									lens_hint_weapon[ewMagic][2]=up;
								}
								
								if(lens_hint_weapon[ewMagic][4]<=0)
								{
									lens_hint_weapon[ewMagic][2]=down;
								}
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
							putweapon(dest,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, lens_hint_weapon[ewMagic][2], lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
						}
						
						break;
						
					case mfREFFIREBALL:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sREFFIREBALL],scr->secretcset[sREFFIREBALL]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_shield,3);
							
							if(tempitem<0) break;
							
							tempweapon=ewFireball;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
								tempweaponx=x;
								tempweapony=y;
								++lens_hint_weapon[ewFireball][3];
								
								if(lens_hint_weapon[ewFireball][3]>8)
								{
									lens_hint_weapon[ewFireball][3]=-8;
									lens_hint_weapon[ewFireball][4]=8;
								}
								
								if(lens_hint_weapon[ewFireball][3]>0)
								{
									++lens_hint_weapon[ewFireball][4];
								}
								else
								{
									--lens_hint_weapon[ewFireball][4];
								}
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
							putweapon(dest,tempweaponx+lens_hint_weapon[tempweapon][3],tempweapony+lens_hint_weapon[ewFireball][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1],-1);
						}
						
						break;
						
					case mfSWORD:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sSWORD],scr->secretcset[sSWORD]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfWSWORD:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sWSWORD],scr->secretcset[sWSWORD]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,2);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfMSWORD:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sMSWORD],scr->secretcset[sMSWORD]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,3);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfXSWORD:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sXSWORD],scr->secretcset[sXSWORD]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,4);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfSWORDBEAM:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sSWORDBEAM],scr->secretcset[sSWORDBEAM]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 1);
						}
						
						break;
						
					case mfWSWORDBEAM:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sWSWORDBEAM],scr->secretcset[sWSWORDBEAM]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,2);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 2);
						}
						
						break;
						
					case mfMSWORDBEAM:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sMSWORDBEAM],scr->secretcset[sMSWORDBEAM]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,3);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 3);
						}
						
						break;
						
					case mfXSWORDBEAM:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sXSWORDBEAM],scr->secretcset[sXSWORDBEAM]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_sword,4);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 4);
						}
						
						break;
						
					case mfHOOKSHOT:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sHOOKSHOT],scr->secretcset[sHOOKSHOT]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_hookshot,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfWAND:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sWAND],scr->secretcset[sWAND]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_wand,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfHAMMER:
						if(!hints)
						{
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))putcombo(dest,x,y,scr->secretcombo[sHAMMER],scr->secretcset[sHAMMER]);
						}
						else
						{
							tempitem=getItemID(itemsbuf,itype_hammer,1);
							
							if(tempitem<0) break;
							
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate))
									|| ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								tempitemx=x;
								tempitemy=y;
							}
							
							putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
						}
						
						break;
						
					case mfARMOS_ITEM:
					case mfDIVE_ITEM:
					{
						int flag = (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM;
						if((!getmapflag(scr, flag) || (scr->flags9&fBELOWRETURN)) && !(itemsbuf[Hero.getLastLensID()].flags & item_flag3))
						{
							putitem2(dest,x,y,scr->catchall, lens_hint_item[scr->catchall][0], lens_hint_item[scr->catchall][1], 0);
						}
						break;
					}
						
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					case 25:
					case 26:
					case 27:
					case 28:
					case 29:
					case 30:
					case 31:
						if(!hints)
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
								putcombo(dest,x,y,scr->secretcombo[checkflag-16+4],scr->secretcset[checkflag-16+4]);
									 
						break;
					case mfSECRETSNEXT:
						if(!hints)
							if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
								putcombo(dest,x,y,rpos_handle.data()+1,rpos_handle.cset());
									 
						break;
					
					case mfSTRIKE:
						if(!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
						{
							goto special;
						}
						else
						{
							break;
						}
						
					default: goto special;
					
					special:
						if(layer && ((checkflag!=mfRAFT && checkflag!=mfRAFT_BRANCH&& checkflag!=mfRAFT_BOUNCE) ||(itemsbuf[Hero.getLastLensID()].flags & item_flag4)))
						{
							if((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&blink_rate)) || ((get_debug() && zc_getkey(KEY_N)) && (frame&blink_rate)))
							{
								rectfill(dest,x,y,x+15,y+15,WHITE);
							}
						}
						
						break;
				}
			}
		});

		for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
			auto [offx, offy] = translate_screen_coordinates_to_world(scr->screen);

			offx -= viewport.x;
			offy -= viewport.y;
			offy += playing_field_offset;

			if (layer)
			{
				if (scr->door[0]==dWALK)
					rectfill(dest, 120+offx, 16+offy, 135+offx, 31+offy, WHITE);
					
				if (scr->door[1]==dWALK)
					rectfill(dest, 120+offx, 144+offy, 135+offx, 159+offy, WHITE);
					
				if (scr->door[2]==dWALK)
					rectfill(dest, 16+offx, 80+offy, 31+offx, 95+offy, WHITE);
					
				if (scr->door[3]==dWALK)
					rectfill(dest, 224+offx, 80+offy, 239+offx, 95+offy, WHITE);
					
				if (scr->door[0]==dBOMB)
				{
					showbombeddoor(scr, dest, 0, offx, offy);
				}
				
				if (scr->door[1]==dBOMB)
				{
					showbombeddoor(scr, dest, 1, offx, offy);
				}
				
				if (scr->door[2]==dBOMB)
				{
					showbombeddoor(scr, dest, 2, offx, offy);
				}
				
				if (scr->door[3]==dBOMB)
				{
					showbombeddoor(scr, dest, 3, offx, offy);
				}
			}

			if (scr->stairx || scr->stairy)
			{
				if (!hints)
				{
					if (!(itemsbuf[Hero.getLastLensID()].flags & item_flag2))
						putcombo(dest,scr->stairx+offx,scr->stairy+offy,scr->secretcombo[sSTAIRS],scr->secretcset[sSTAIRS]);
				}
				else
				{
					if(scr->flags&fWHISTLE)
					{
						tempitem=getItemID(itemsbuf,itype_whistle,1);
						int32_t tempitemx=-16+offx;
						int32_t tempitemy=-16+offy-playing_field_offset;
						
						if ((!(get_debug() && zc_getkey(KEY_N)) && (lensclk&(blink_rate/4)))
								|| ((get_debug() && zc_getkey(KEY_N)) && (frame&(blink_rate/4))))
						{
							tempitemx=scr->stairx+offx;
							tempitemy=scr->stairy+offy;
						}
						
						putitem2(dest, tempitemx, tempitemy, tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
					}
				}
			}
		});
	}
}

void draw_lens_over(BITMAP *dest)
{
	int w = 288;
	int h = 240;

	static BITMAP *lens_scr = create_bitmap_ex(8,2*w,2*h);
	static int32_t last_width = -1;
	int32_t width = itemsbuf[current_item_id(itype_lens,true)].misc1;
	
	// Only redraw the circle if the size has changed
	if (width != last_width)
	{
		clear_to_color(lens_scr, BLACK);
		circlefill(lens_scr, w, h, width, 0);
		circle(lens_scr, w, h, width+2, 0);
		circle(lens_scr, w, h, width+5, 0);
		last_width=width;
	}
	
	masked_blit(lens_scr, dest, w-(HeroX()+8)+viewport.x, h-(HeroY()+8)+viewport.y, 0, playing_field_offset, 256, viewport.h);
	do_primitives(dest, SPLAYER_LENS_OVER);
}

static void update_bmp_size(BITMAP** bmp_ptr, int w, int h)
{
	BITMAP* bmp = *bmp_ptr;
	if (bmp->w == w && bmp->h == h)
		return;

	int depth = bitmap_color_depth(bmp);
	destroy_bitmap(bmp);
	*bmp_ptr = create_bitmap_ex(depth, w, h);
}

void draw_wavy(BITMAP *source, BITMAP *target, int32_t amplitude, bool interpol)
{
	static BITMAP *wavebuf = create_bitmap_ex(8,288,240-original_playing_field_offset);
	update_bmp_size(&wavebuf, 288, 240 - original_playing_field_offset);

	clear_to_color(wavebuf, BLACK);
	blit(source,wavebuf,0,original_playing_field_offset,16,0,256,framebuf->h-original_playing_field_offset);
	
	int32_t ofs;
	amplitude = zc_min(2048,amplitude); // some arbitrary limit to prevent crashing
	if(flash_reduction_enabled() && !get_qr(qr_WAVY_NO_EPILEPSY)) amplitude = zc_min(16,amplitude);
	int32_t amp2 = viewport.visible_height(show_bottom_8px);
	if(flash_reduction_enabled() && !get_qr(qr_WAVY_NO_EPILEPSY_2)) amp2*=2;
	int32_t i=frame%amp2;
	
	for(int32_t j=0; j<viewport.visible_height(show_bottom_8px); j++)
	{
		if(j&1 && interpol)
		{
			// Add 288*2048 to ensure it's never negative. It'll get modded out.
			ofs=288*2048+int32_t(zc::math::Sin((double(i+j)*2*PI/amp2))*amplitude);
		}
		else
		{
			ofs=288*2048-int32_t(zc::math::Sin((double(i+j)*2*PI/amp2))*amplitude);
		}
		
		if(ofs)
		{
			for(int32_t k=0; k<256; k++)
			{
				target->line[j+original_playing_field_offset][k]=wavebuf->line[j][(k+ofs+16)%288];
			}
		}
	}
}

void draw_fuzzy(int32_t fuzz)
// draws from right half of scrollbuf to framebuf
{
	int32_t firstx, firsty, xstep, ystep, i, y, dx, dy;
	byte *start, *si, *di;
	
	if(fuzz<1)
		fuzz = 1;
		
	xstep = 128%fuzz;
	
	if(xstep > 0)
		xstep = fuzz-xstep;
		
	ystep = 112%fuzz;
	
	if(ystep > 0)
		ystep = fuzz-ystep;
		
	firsty = 1;
	
	for(y=0; y<framebuf->h;)
	{
		start = &(scrollbuf_old->line[y][256]);
		
		for(dy=0; dy<ystep && dy+y<framebuf->h; dy++)
		{
			si = start;
			di = &(framebuf->line[y+dy][0]);
			i = xstep;
			firstx = 1;
			
			for(dx=0; dx<framebuf->w; dx++)
			{
				*(di++) = *si;
				
				if(++i >= fuzz)
				{
					if(!firstx)
						si += fuzz;
					else
					{
						si += fuzz-xstep;
						firstx = 0;
					}
					
					i = 0;
				}
			}
		}
		
		if(!firsty)
			y += fuzz;
		else
		{
			y += ystep;
			ystep = fuzz;
			firsty = 0;
		}
	}
}

void updatescr(bool allowwavy)
{
	static BITMAP *wavybuf = create_bitmap_ex(8, framebuf->w, framebuf->h);
	static BITMAP *panorama = create_bitmap_ex(8, framebuf->w, framebuf->h);
	update_bmp_size(&wavybuf, framebuf->w, framebuf->h);
	update_bmp_size(&panorama, framebuf->w, framebuf->h);

	if(walk_through_walls)
	{
		textout_ex(framebuf,font,"no walls",8,216,1,-1);
	}
	
	if(Showpal)
		dump_pal(framebuf);
		
	if(!Playing)
		black_opening_count=0;
		
	if(black_opening_count<0) //shape is opening up
	{
		black_opening(framebuf,black_opening_x,black_opening_y,(66+black_opening_count),66);
		
		if(Advance||(!Paused))
		{
			++black_opening_count;
		}
	}
	else if(black_opening_count>0) //shape is closing
	{
		black_opening(framebuf,black_opening_x,black_opening_y,black_opening_count,66);
		
		if(Advance||(!Paused))
		{
			--black_opening_count;
		}
	}

	if(black_opening_count==0&&black_opening_shape==bosFADEBLACK)
	{
		black_opening_shape = bosCIRCLE;
		memcpy(RAMpal, tempblackpal, PAL_SIZE*sizeof(RGB));
		refreshTints();
		refreshpal=true;
	}
	
	if(refreshpal)
	{
		refreshpal=false;
		RAMpal[253] = _RGB(0,0,0);
		RAMpal[254] = _RGB(255,255,255);
		hw_palette = &RAMpal;
		update_hw_pal = true;
		refresh_rgb_tables();
	}
	
	bool clearwavy = (wavy <= 0);
	
	if(wavy <= 0)
	{
		// So far one thing can alter wavy apart from scripts: Wavy DMaps.
		wavy = (DMaps[cur_dmap].flags&dmfWAVY ? 4 : 0);
	}
	
	blit(framebuf, wavybuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
	
	if(wavy && Playing && allowwavy)
	{
		draw_wavy(framebuf, wavybuf, wavy,false);
	}
	
	if(clearwavy)
		wavy = 0; // Wavy was set by a DMap flag. Clear it.
	else if(Playing && !Paused)
		wavy--; // Wavy was set by a script. Decrement it.
	
	if(Playing && !Paused)
		++light_wave_clk;
	
	if(Playing && msg_active && !screenscrolling)
	{
		if(!(msg_bg_display_buf->clip))
			blit_msgstr_bg(framebuf,0,0,0,playing_field_offset,256,176);
		if(!(msg_portrait_display_buf->clip))
			blit_msgstr_prt(framebuf,0,0,0,playing_field_offset,256,176);
		if(!(msg_txt_display_buf->clip))
			blit_msgstr_fg(framebuf,0,0,0,playing_field_offset,256,176);
	}
	
	bool nosubscr = GameLoaded && no_subscreen() && !(hero_scr->flags3&fNOSUBSCROFFSET);
	
	if(nosubscr)
	{
		clear_to_color(panorama, 0);
		blit(wavybuf,panorama,0,playing_field_offset,0,playing_field_offset/2,256,framebuf->h-playing_field_offset);
	}
	
	//TODO: Optimize blit 'overcalls' -Gleeok
	BITMAP *source = nosubscr ? panorama : wavybuf;
	blit(source, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);

	update_hw_screen();
}

//----------------------------------------------------------------

static PALETTE syspal;
int32_t onGUISnapshot()
{
	char buf[200];
	int32_t num=0;
	do
	{
		sprintf(buf, "%szc_screen%05d.%s", get_snap_str(), ++num, snapshotformat_str[SnapshotFormat][1]);
	}
	while(num<99999 && exists(buf));
	
	if (!al_save_bitmap(buf, al_get_backbuffer(all_get_display())))
		InfoDialog("Error", "Failed to save snapshot").show();
	
	return D_O_K;
}

int32_t onNonGUISnapshot()
{
	PALETTE temppal;
	get_palette(temppal);
	bool realpal=(zc_getkey(KEY_ZC_LCONTROL, true) || zc_getkey(KEY_ZC_RCONTROL, true));
	
	char buf[200];
	int32_t num=0;
	
	do
	{
		sprintf(buf, "%szc_screen%05d.%s", get_snap_str(), ++num, snapshotformat_str[SnapshotFormat][1]);
	}
	while(num<99999 && exists(buf));

	if (hero_scr && no_subscreen() && !(hero_scr->flags3&fNOSUBSCROFFSET) && !key[KEY_ALT])
	{
		BITMAP *b = create_bitmap_ex(8, 256, viewport.visible_height(show_bottom_8px));
		clear_to_color(b,0);
		blit(framebuf,b,0,playing_field_offset/2,0,0,b->w,b->h);
		alleg4_save_bitmap(b, SnapshotScale, buf, realpal ? temppal : RAMpal);
		destroy_bitmap(b);
	}
	else
	{
		alleg4_save_bitmap(framebuf, SnapshotScale, buf, realpal?temppal:RAMpal);
	}
	
	return D_O_K;
}

int32_t onSnapshot()
{
	if(zc_getkey(KEY_LSHIFT, true)||zc_getkey(KEY_RSHIFT, true))
	{
		onGUISnapshot();
	}
	else
	{
		onNonGUISnapshot();
	}
	
	return D_O_K;
}

int32_t onSaveMapPic()
{
	char buf[200];
	int32_t num=0;
	BITMAP* _screen_draw_buffer = NULL;
	_screen_draw_buffer = create_bitmap_ex(8,256,176);
	
	do
	{
		sprintf(buf, "%szc_screen%05d.png", get_snap_str(), ++num);
	}
	while(num<99999 && exists(buf));
	
	BITMAP* mappic = create_bitmap_ex(8,(256*16),(176*8));
	clear_to_color(mappic, BLACK);
	
	if(!mappic)
	{
		enter_sys_pal();
		displayinfo("View Map","Not enough memory.");
		exit_sys_pal();
		return D_O_K;;
	}

	clear_to_color(_screen_draw_buffer, BLACK);

	auto prev_viewport = viewport;
	viewport.x = 0;
	viewport.y = 0;
	
	// draw the map
	
	bool classic_draw = get_qr(qr_CLASSIC_DRAWING_ORDER);
	for(int32_t y=0; y<8; y++)
	{
		for(int32_t x=0; x<16; x++)
		{
			if (!displayOnMap(x, y))
				continue;

			int screen = map_scr_xy_to_index(x, y);
			auto scrs = loadscr2(screen);
			mapscr* scr = &scrs[0];
			if (!scr->is_valid())
				continue;

			screen_handles_t screen_handles;
			for (int i = 0; i <= 6; i++)
				screen_handles[i] = {scr, scrs[i].is_valid() ? &scrs[i] : nullptr, screen, i};

			int xx = 0;
			int yy = -playing_field_offset;
			
			if (!classic_draw)
				for (int layer = -7; layer <= -4; ++layer)
					do_ffc_layer(_screen_draw_buffer, layer, screen_handles[0], xx, yy);

			if(classic_draw)
			{
				if(XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
					do_layer(_screen_draw_buffer, 0, screen_handles[2], xx, yy);
				do_ffc_layer(_screen_draw_buffer, -2, screen_handles[0], xx, yy);
			}

			if(XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
				do_layer(_screen_draw_buffer, 0, screen_handles[3], xx, yy);
			do_ffc_layer(_screen_draw_buffer, -3, screen_handles[0], xx, yy);

			if(!classic_draw)
			{
				if(XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
					do_layer(_screen_draw_buffer, 0, screen_handles[2], xx, yy);
				do_ffc_layer(_screen_draw_buffer, -2, screen_handles[0], xx, yy);
				do_ffc_layer(_screen_draw_buffer, -1, screen_handles[0], xx, yy);
			}

			if(lenscheck(scr,0))
				putscr(scr, _screen_draw_buffer, 0, 0);
			do_ffc_layer(_screen_draw_buffer, 0, screen_handles[0], xx, yy);
			do_layer(_screen_draw_buffer, 0, screen_handles[1], xx, yy);
			do_ffc_layer(_screen_draw_buffer, 1, screen_handles[0], xx, yy);
			
			if(!XOR(scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG))
			{
				do_layer(_screen_draw_buffer, 0, screen_handles[2], xx, yy);
				do_ffc_layer(_screen_draw_buffer, 2, screen_handles[0], xx, yy);
			}
			
			putscrdoors(scr, _screen_draw_buffer, xx, yy);
			if(get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			{
				do_layer(_screen_draw_buffer, -2, screen_handles[0], xx, yy);
				if(get_qr(qr_PUSHBLOCK_LAYER_1_2))
				{
					do_layer(_screen_draw_buffer, -2, screen_handles[1], xx, yy);
					do_layer(_screen_draw_buffer, -2, screen_handles[2], xx, yy);
				}
			}
			
			if(!XOR(scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG))
			{
				do_layer(_screen_draw_buffer, 0, screen_handles[3], xx, yy);
				do_ffc_layer(_screen_draw_buffer, 3, screen_handles[0], xx, yy);
			}
			
			do_layer(_screen_draw_buffer, 0, screen_handles[4], xx, yy);
			do_ffc_layer(_screen_draw_buffer, 4, screen_handles[0], xx, yy);
			do_layer(_screen_draw_buffer, -1, screen_handles[0], xx, yy);
			if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
			{
				do_layer(_screen_draw_buffer, -1, screen_handles[1], xx, yy);
				do_layer(_screen_draw_buffer, -1, screen_handles[2], xx, yy);
			}
			do_layer(_screen_draw_buffer, 0, screen_handles[5], xx, yy);
			do_ffc_layer(_screen_draw_buffer, 5, screen_handles[0], xx, yy);
			if(replay_version_check(40))
				do_ffc_layer(_screen_draw_buffer, -1000, screen_handles[0], xx, yy);
			do_layer(_screen_draw_buffer, 0, screen_handles[6], xx, yy);
			do_ffc_layer(_screen_draw_buffer, 6, screen_handles[0], xx, yy);
			do_ffc_layer(_screen_draw_buffer, 7, screen_handles[0], xx, yy);

			blit(_screen_draw_buffer, mappic, 0, 0, x*256, y*176, 256, 176);
		}
	}

	viewport = prev_viewport;
	save_bitmap(buf,mappic,RAMpal);
	destroy_bitmap(mappic);
	destroy_bitmap(_screen_draw_buffer);
	return D_O_K;
}

void f_Quit(int32_t type)
{
	if(type==qQUIT && !Playing)
		return;
	
	bool from_menu = is_sys_pal;
	
	if(!from_menu)
	{
		music_pause();
		pause_all_sfx();
		sys_mouse();
	}
	enter_sys_pal();
	clear_keybuf();
	
	if (replay_version_check(0, 10))
		replay_poll();
	if (replay_is_replaying())
		replay_peek_quit();

	if (!replay_is_replaying())
		switch(type)
		{
		case qQUIT:
			onQuit();
			break;
			
		case qRESET:
			onReset();
			break;
			
		case qEXIT:
			onExit();
			break;
		}
	
	if(Quit)
	{
		kill_sfx();
		music_stop();
		exit_sys_pal();
		update_hw_screen();
	}
	else
	{
		exit_sys_pal();
		if(!from_menu)
		{
			music_resume();
			resume_all_sfx();
		}
	}
	
	if(!from_menu)
		game_mouse();
	eat_buttons();
	
	zc_readrawkey(KEY_ESC);
		
	zc_readrawkey(KEY_ENTER);
}

//----------------------------------------------------------------

int32_t onNoWalls()
{
	cheats_enqueue(Cheat::Walls);
	return D_O_K;
}

int32_t onIgnoreSideview()
{
	cheats_enqueue(Cheat::IgnoreSideView);
	return D_O_K;
}

int32_t input_idle()
{
	static int32_t mz, mb;
	
	if(keypressed() || button_pressed() || zc_key_pressed() || mz != mouse_z || mb != mouse_b)
	{
		idle_count = 0;
		
		if(active_count < MAX_ACTIVE)
		{
			++active_count;
		}
	}
	else if(idle_count < MAX_IDLE)
	{
		++idle_count;
		active_count = 0;
	}
	
	mz = mouse_z;
	mb = mouse_b;
	
	return idle_count;
}

int32_t onGoFast()
{
	cheats_enqueue(Cheat::Fast);
	return D_O_K;
}

int32_t onKillCheat()
{
	cheats_enqueue(Cheat::Kill);
	return D_O_K;
}

int32_t onSecretsCheat()
{
	cheats_enqueue(Cheat::TrigSecrets);
	return D_O_K;
}
int32_t onSecretsCheatPerm()
{
	cheats_enqueue(Cheat::TrigSecretsPerm);
	return D_O_K;
}

int32_t onShowLayer0()
{
	show_layers[0] = !show_layers[0];
	return D_O_K;
}
int32_t onShowLayer1()
{
	show_layers[1] = !show_layers[1];
	return D_O_K;
}
int32_t onShowLayer2()
{
	show_layers[2] = !show_layers[2];
	return D_O_K;
}
int32_t onShowLayer3()
{
	show_layers[3] = !show_layers[3];
	return D_O_K;
}
int32_t onShowLayer4()
{
	show_layers[4] = !show_layers[4];
	return D_O_K;
}
int32_t onShowLayer5()
{
	show_layers[5] = !show_layers[5];
	return D_O_K;
}
int32_t onShowLayer6()
{
	show_layers[6] = !show_layers[6];
	return D_O_K;
}
int32_t onShowLayerO()
{
	show_layer_over=!show_layer_over;
	return D_O_K;
}
int32_t onShowLayerP()
{
	show_layer_push=!show_layer_push;
	return D_O_K;
}
int32_t onShowLayerS()
{
	show_sprites=!show_sprites;
	return D_O_K;
}
int32_t onShowLayerF()
{
	show_ffcs=!show_ffcs;
	return D_O_K;
}
int32_t onShowLayerW()
{
	show_walkflags=!show_walkflags;
	if(show_walkflags)
		show_effectflags = false;
	return D_O_K;
}
int32_t onShowLayerE()
{
	show_effectflags=!show_effectflags;
	if(show_effectflags)
		show_walkflags = false;
	return D_O_K;
}
int32_t onShowFFScripts()
{
	show_ff_scripts=!show_ff_scripts;
	return D_O_K;
}
int32_t onShowHitboxes()
{
	show_hitboxes=!show_hitboxes;
	return D_O_K;
}
int32_t onShowInfoOpacity()
{
	if (auto num = call_get_num("Debug Info Opacity", info_opacity, 255, 0))
	{
		info_opacity = *num;
		zc_set_config("zc", "debug_info_opacity", info_opacity);
	}
	return D_O_K;
}

int32_t onLightSwitch()
{
	cheats_enqueue(Cheat::Light);
	return D_O_K;
}

int32_t onGoTo();
int32_t onGoToComplete();

bool handle_close_btn_quit()
{
	if(close_button_quit)
	{
		close_button_quit=false;
		f_Quit(qEXIT);
	}
	return (exiting_program = Quit==qEXIT);
}

void syskeys()
{
	poll_keyboard();
	poll_joystick();
	update_system_keys();
	
	handle_close_btn_quit();
	if(Quit == qEXIT) return;
	
	if(menu_pressed(true) || (gui_mouse_b() && !mouse_down && ClickToFreeze &&!disableClickToFreeze))
	{
		System();
	}
	
	mouse_down = gui_mouse_b();
	
	if(zc_read_system_key(KEY_F1))
	{
		if(zc_get_system_key(KEY_ZC_LCONTROL) || zc_get_system_key(KEY_ZC_RCONTROL))
		{
			halt=!halt;
			//zinit.subscreen=(zinit.subscreen+1)%ssdtMAX;
		}
		else
		{
			Throttlefps=!Throttlefps;
			zc_set_config(cfg_sect,"throttlefps", (int32_t)Throttlefps);
		}
	}
	
	if(zc_read_system_key(KEY_F2))
	{
		ShowFPS=!ShowFPS;
		zc_set_config(cfg_sect,"showfps",(int32_t)ShowFPS);
	}
	
	if(zc_read_system_key(KEY_F3) && Playing)	Paused=!Paused;
	
	if(zc_read_system_key(KEY_F4) && Playing)
	{
		Paused=true;
		Advance=true;
	}
	
	if(zc_read_system_key(KEY_F6)) onTryQuit();
	
#ifndef ALLEGRO_MACOSX
	if(zc_read_system_key(KEY_F9))	f_Quit(qRESET);
	
	if(zc_read_system_key(KEY_F10))   f_Quit(qEXIT);
#else
	if(zc_read_system_key(KEY_F7))	f_Quit(qRESET);
	
	if(zc_read_system_key(KEY_F8))   f_Quit(qEXIT);
#endif
	if(zc_read_system_key(KEY_F5)&&(Playing && cur_screen<128 && DMaps[cur_dmap].flags&dmfVIEWMAP))	onSaveMapPic();
	
	if (zc_read_system_key(KEY_F12))
	{
		onSnapshot();
	}
	
	if(debug_enabled && zc_read_system_key(KEY_TAB))
		set_debug(!get_debug());
	
	if(CheatModifierKeys())
	{
		for(Cheat c = (Cheat)1; c < Cheat::Last; c = (Cheat)(c+1))
		{
			if(!bindable_cheat(c))
				continue;
			if(get_debug() || cheat >= cheat_lvl(c))
			{
				if(checkcheat(c))
					cheats_hit_bind(c);
			}
		}
	}
	
	if(volkeys)
	{
		if(zc_read_system_key(KEY_PGUP)) master_volume(-1,midi_volume+8);
		
		if(zc_read_system_key(KEY_PGDN)) master_volume(-1,midi_volume==255?248:midi_volume-8);
		
		if(zc_read_system_key(KEY_HOME)) master_volume(digi_volume+8,-1);
		
		if(zc_read_system_key(KEY_END))  master_volume(digi_volume==255?248:digi_volume-8,-1);
	}
	
	if(!get_debug() || !SystemKeys || replay_is_replaying())
		goto bottom;
	
	if(zc_readkey(KEY_P))   Paused=!Paused;
	
	if(zc_readkey(KEY_A))
	{
		Paused=true;
		Advance=true;
	}
	
	if(zc_readkey(KEY_G))   db=(db==999)?0:999;
#ifndef ALLEGRO_MACOSX
	if(zc_readkey(KEY_F8))  Showpal=!Showpal;
	
	if(zc_readkey(KEY_F7))
		screen_saver(ss_speed, ss_density);
#else
	// The reason these are different on Mac in the first place is that
	// the OS doesn't let us use F9 and F10...
	if(zc_readkey(KEY_F10))  Showpal=!Showpal;
	
	if(zc_readkey(KEY_F9))
		screen_saver(ss_speed, ss_density);
#endif
	if(zc_readkey(KEY_PLUS_PAD) || zc_readkey(KEY_EQUALS))
	{
		//change containers
		if(zc_getkey(KEY_ZC_LCONTROL) || zc_getkey(KEY_ZC_RCONTROL))
		{
			//magic containers
			if(zc_getkey(KEY_LSHIFT) || zc_getkey(KEY_RSHIFT))
			{
				game->set_maxmagic(zc_min(game->get_maxmagic()+game->get_mp_per_block(),game->get_mp_per_block()*8));
			}
			else
			{
				game->set_maxlife(zc_min(game->get_maxlife()+game->get_hp_per_heart(),game->get_hp_per_heart()*24));
			}
		}
		else
		{
			if(zc_getkey(KEY_LSHIFT) || zc_getkey(KEY_RSHIFT))
			{
				game->set_magic(zc_min(game->get_magic()+1,game->get_maxmagic()));
			}
			else
			{
				game->set_life(zc_min(game->get_life()+1,game->get_maxlife()));
			}
		}
	}
	
	if(zc_readkey(KEY_MINUS_PAD) || zc_readkey(KEY_MINUS))
	{
		//change containers
		if(zc_getkey(KEY_ZC_LCONTROL) || zc_getkey(KEY_ZC_RCONTROL))
		{
			//magic containers
			if(zc_getkey(KEY_LSHIFT) || zc_getkey(KEY_RSHIFT))
			{
				game->set_maxmagic(zc_max(game->get_maxmagic()-game->get_mp_per_block(),0));
				game->set_magic(zc_min(game->get_maxmagic(), game->get_magic()));
				//heart containers
			}
			else
			{
				game->set_maxlife(zc_max(game->get_maxlife()-game->get_hp_per_heart(),game->get_hp_per_heart()));
				game->set_life(zc_min(game->get_maxlife(), game->get_life()));
			}
		}
		else
		{
			if(zc_getkey(KEY_LSHIFT) || zc_getkey(KEY_RSHIFT))
			{
				game->set_magic(zc_max(game->get_magic()-1,0));
			}
			else
			{
				game->set_life(zc_max(game->get_life()-1,0));
			}
		}
	}
	
	if(zc_readkey(KEY_COMMA))  jukebox(wrap(currmidi-1, 0, MAXMIDIS-1));
	
	if(zc_readkey(KEY_STOP))   jukebox(wrap(currmidi+1, 0, MAXMIDIS-1));
	
	verifyBothWeapons();
	
bottom:

	if(ss_enable && input_idle() > ss_seconds * 60)
		screen_saver(ss_speed, ss_density);
	
	mouse_down = gui_mouse_b();
}

void checkQuitKeys()
{
#ifndef ALLEGRO_MACOSX
	if(key[KEY_F9])	f_Quit(qRESET);
	
	if(key[KEY_F10])   f_Quit(qEXIT);
#else
	if(key[KEY_F7])	f_Quit(qRESET);
	
	if(key[KEY_F8])   f_Quit(qEXIT);
#endif
}

bool CheatModifierKeys()
{
	// Cheats are replayed via the X cheat step, no need to check for keyboard input
	// to trigger cheats.
	if (replay_is_replaying())
		return false;

	if ( ( active_control_scheme->cheat_modifier_keys[0] > 0 && key[active_control_scheme->cheat_modifier_keys[0]] ) ||
		( active_control_scheme->cheat_modifier_keys[1] > 0 && key[active_control_scheme->cheat_modifier_keys[1]] ) ||
		(active_control_scheme->cheat_modifier_keys[0] <= 0 && active_control_scheme->cheat_modifier_keys[1] <= 0))
	{
		if ( ( active_control_scheme->cheat_modifier_keys[2] <= 0 || key[active_control_scheme->cheat_modifier_keys[2]] ) ||
			( active_control_scheme->cheat_modifier_keys[3] > 0 && key[active_control_scheme->cheat_modifier_keys[3]] )  ||
			(active_control_scheme->cheat_modifier_keys[2] <= 0 && active_control_scheme->cheat_modifier_keys[3] <= 0))
		{
			return true;
		}
	}
	return false;
}

//99:05:54, for some reason?
#define OLDMAXTIME  21405240
//9000:00:00, the highest even-thousand hour fitting within 32b signed. This is 375 *DAYS*.
#define MAXTIME	 1944000000

// (qr, value)
static std::queue<std::pair<int, bool>> change_qr_queue;

void enqueue_qr_change(int qr, bool value)
{
	change_qr_queue.push({qr, value});
}

// During regular play, QR changes issued through `syskey` / `System` and enqueued
// and soon executed here.
// During playing back a replay file, the replay system adds to the same queue and
// is executed here too.
// This is currently only used to allow users to configure qr_HIDE_BOTTOM_8_PIXELS, but
// could be later extended to all QRs (perhaps as a cheat).
void process_enqueued_qr_changes()
{
	if (replay_is_replaying())
		replay_do_qrs();

	while (!change_qr_queue.empty())
	{
		auto [qr, value] = change_qr_queue.front();
		change_qr_queue.pop();

		// Don't modify `quest_rules`, as that is used to store the canonical QR value which can be reset to
		// via system menus. Changing the unpacked array is enough to modify the engine's behavior.
		_qrs_unpacked[qr] = value;
		apply_qr_rule(qr);

		if (replay_is_recording())
			replay_step_qr(qr, value);
	}
}

void advanceframe(bool allowwavy, bool sfxcleanup, bool allowF6Script)
{
	if(zcmusic!=NULL)
	{
		zcmusic_poll();
	}
	zcmixer_update(zcmixer, emusic_volume, FFCore.usr_music_volume, get_qr(qr_OLD_SCRIPT_VOLUME));

	updatescr(allowwavy);

	Advance=false;
	while(Paused && !Advance && !Quit)
	{
		// have to call this, otherwise we'll get an infinite loop
		syskeys();
		if(allowF6Script)
		{
			FFCore.runF6Engine();
		}
		
#ifdef _WIN32
		
		if(use_dwm_flush)
		{
			do_DwmFlush();
		}
		
#endif
		
		// to keep music playing
		if(zcmusic!=NULL)
		{
			zcmusic_poll();
		}

		update_hw_screen();
	}
	
	if(Quit)
		return;
	
	if(Playing && game->get_time()<unsigned(get_qr(qr_GREATER_MAX_TIME) ? MAXTIME : OLDMAXTIME))
		game->change_time(1);
	
	// Many mistakes have been make re: inputs, and we are stuck with many replays relying on those mistakes.

	bool should_reset_down_state = !get_qr(qr_BROKEN_INPUT_DOWN_STATE);
	if (replay_version_check(0, 16))
		should_reset_down_state = replay_version_check(11, 16);
	if (should_reset_down_state)
	{
		for (int i = 0; i < controls::btnLast; i++)
			down_control_states[i] = raw_control_state[i];
	}
	
	if (replay_is_active())
	{
		if (replay_version_check(3))
			replay_poll();
		
		if (replay_version_check(11) || replay_version_check(6, 8))
			replay_peek_input();
	}

	process_enqueued_qr_changes();

	load_control_called_this_frame = false;
	
	poll_keyboard();
	update_keys();
	
	++frame;
	
	if (replay_is_replaying())
		replay_do_cheats();
	syskeys();
	
	// The mouse variables can change from the mouse thread at anytime during a frame,
	// so save the result at the start so that replaying is consistent.
	script_mouse_x = gui_mouse_x();
	script_mouse_y = gui_mouse_y();
	script_mouse_z = mouse_z;
	script_mouse_b = mouse_b;
	
	// Cheats used via the System menu (called by syskeys) will call cheats_enqueue. syskeys
	// is called just above, and in the paused loop above, so the queue-and-defer-slightly
	// approach here means it doesn't matter which call adds the cheat.
	cheats_execute_queued();

	if (replay_is_replaying())
		replay_peek_quit();
	if (GameFlags & GAMEFLAG_TRYQUIT)
		replay_step_quit(0);
	if(allowF6Script)
		FFCore.runF6Engine();
	if (Quit)
		replay_step_quit(Quit);
	
#ifdef _WIN32
	
	if(use_dwm_flush)
	{
		do_DwmFlush();
	}
	
#endif
	
	if(sfxcleanup)
		sfx_cleanup();

	frame_timings_poll();

#ifdef __EMSCRIPTEN__
	// Yield the main thread back to the browser occasionally.
	if (is_headless())
	{
		static int rate = 10000;
		static int force_yield = rate;
		if (force_yield++ >= rate)
		{
			force_yield = 0;
			emscripten_sleep(0);
		}
	}
#endif

	if (zqtesting_mode && test_mode_auto_restart)
	{
		static auto last_write_time = fs::last_write_time(qstpath);
		static auto last_check = std::chrono::system_clock::now();

		if (std::chrono::system_clock::now() - last_check > 200ms)
		{
			last_check = std::chrono::system_clock::now();
			auto write_time = fs::last_write_time(qstpath);
			if (last_write_time != write_time)
			{
				last_write_time = write_time;
				disableClickToFreeze = false;
				Quit = qRESET;
				replay_quit();
			}
		}
	}
}

void zapout()
{
	set_clip_rect(scrollbuf_old, 0, 0, scrollbuf_old->w, scrollbuf_old->h);
	blit(framebuf,scrollbuf_old,0,0,256,0,256,framebuf->h);
	
	FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
	script_drawing_commands.Clear();
	
	// zap out
	for(int32_t i=1; i<=24; i++)
	{
		draw_fuzzy(i);
		advanceframe(true);
		
		if(Quit)
		{
			break;
		}
	}
}

void zapin()
{
	FFCore.warpScriptCheck();
	draw_screen();
	set_clip_rect(scrollbuf_old, 0, 0, scrollbuf_old->w, scrollbuf_old->h);
	blit(framebuf,scrollbuf_old,0,0,256,0,256,framebuf->h);
	
	// zap out
	FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
	for(int32_t i=24; i>=1; i--)
	{
		draw_fuzzy(i);
		advanceframe(true);
		
		if(Quit)
		{
			break;
		}
	}
}


void wavyout(bool showhero)
{
	draw_screen(showhero);
	
	BITMAP *wavebuf = create_bitmap_ex(8,288,framebuf->h);
	clear_to_color(wavebuf,0);
	blit(framebuf,wavebuf,0,0,16,0,framebuf->w,framebuf->h);
	
	static PALETTE wavepal;
	
	int32_t ofs;
	int32_t amplitude=8;
	
	int32_t wavelength=4;
	int height = viewport.visible_height(show_bottom_8px);
	double palpos=0, palstep=4, palstop=126;

	FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
	for(int32_t i=0; i<height; i+=wavelength)
	{
		for(int32_t l=0; l<256; l++)
		{
			wavepal[l].r=vbound(int32_t(RAMpal[l].r+((palpos/palstop)*(255-RAMpal[l].r))),0,255);
			wavepal[l].g=vbound(int32_t(RAMpal[l].g+((palpos/palstop)*(255-RAMpal[l].g))),0,255);
			wavepal[l].b=vbound(int32_t(RAMpal[l].b+((palpos/palstop)*(255-RAMpal[l].b))),0,255);
		}
		
		palpos+=palstep;
		
		if(palpos>=0)
		{
			hw_palette = &wavepal;
			update_hw_pal = true;
		}
		else
		{
			hw_palette = &RAMpal;
			update_hw_pal = true;
		}
		
		for(int32_t j=0; j+playing_field_offset<framebuf->h; j++)
		{
			for(int32_t k=0; k<256; k++)
			{
				ofs=0;
				
				if((j<i)&&(j&1))
				{
					ofs=int32_t(zc::math::Sin((double(i+j)*2*PI/height))*amplitude);
				}
				
				framebuf->line[j+playing_field_offset][k]=wavebuf->line[j+playing_field_offset][k+ofs+16];
			}
		}
		
		advanceframe(true);

		if(Quit)
			break;
	}
	
	destroy_bitmap(wavebuf);

	hw_palette = &RAMpal;
	update_hw_pal = true;
}

void wavyin()
{
	draw_screen();
	
	BITMAP *wavebuf = create_bitmap_ex(8,288,framebuf->h);
	clear_to_color(wavebuf,0);
	blit(framebuf,wavebuf,0,0,16,0,framebuf->w,framebuf->h);
	
	static PALETTE wavepal;

	refreshpal=false;
	int32_t ofs;
	int32_t amplitude=8;
	int32_t wavelength=4;
	int height = viewport.visible_height(show_bottom_8px);
	double palpos=height, palstep=4, palstop=126;
	
	FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
	for(int32_t i=0; i<height; i+=wavelength)
	{
		for(int32_t l=0; l<256; l++)
		{
			wavepal[l].r=vbound(int32_t(RAMpal[l].r+((palpos/palstop)*(255-RAMpal[l].r))),0,255);
			wavepal[l].g=vbound(int32_t(RAMpal[l].g+((palpos/palstop)*(255-RAMpal[l].g))),0,255);
			wavepal[l].b=vbound(int32_t(RAMpal[l].b+((palpos/palstop)*(255-RAMpal[l].b))),0,255);
		}
		
		palpos-=palstep;
		
		if(palpos>=0)
		{
			hw_palette = &wavepal;
			update_hw_pal = true;
		}
		else
		{
			hw_palette = &RAMpal;
			update_hw_pal = true;
		}
		
		for(int32_t j=0; j+playing_field_offset<framebuf->h; j++)
		{
			for(int32_t k=0; k<256; k++)
			{
				ofs=0;
				
				if((j<(height-1-i))&&(j&1))
				{
					ofs=int32_t(zc::math::Sin((double(i+j)*2*PI/height))*amplitude);
				}
				
				framebuf->line[j+playing_field_offset][k]=wavebuf->line[j+playing_field_offset][k+ofs+16];
			}
		}
		
		advanceframe(true);
		
		if(Quit)
			break;
	}
	
	destroy_bitmap(wavebuf);

	hw_palette = &RAMpal;
	update_hw_pal = true;
}

void blackscr(int32_t fcnt,bool showsubscr)
{
	reset_pal_cycling();
	script_drawing_commands.Clear();
	
	FFCore.warpScriptCheck();
	bool showtime = game->should_show_time();
	while(fcnt>0)
	{
		clear_bitmap(framebuf);
		
		if(showsubscr)
		{
			put_passive_subscr(framebuf,0,0,showtime,sspUP);
			if(get_qr(qr_SCRIPTDRAWSINWARPS) || (get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN)))
			{
				do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
			}
		}
		
		advanceframe(true);
		
		if(Quit)
			break;
			
		--fcnt;
	}
}

void openscreen(int32_t shape)
{
	update_viewport();
	is_opening_screen = true;
	reset_pal_cycling();
	black_opening_count=0;
	
	if(COOLSCROLL || shape>-1)
	{
		open_black_opening(HeroX()+8, (HeroY()-HeroZ()-HeroFakeZ())+8+playing_field_offset, true, shape);
		return;
	}
	else
	{
		Hero.setDontDraw(true);
		show_subscreen_dmap_dots=false;
		show_subscreen_numbers=false;
		show_subscreen_life=false;
	}
	
	int32_t x=128;
	
	FFCore.warpScriptCheck();
	for(int32_t i=0; i<80; i++)
	{
		draw_screen();
		x=128-(((i*128/80)/8)*8);
		
		if(x>0)
		{
			rectfill(framebuf,0,playing_field_offset,x,(viewport.h - 1)+playing_field_offset,0);
			rectfill(framebuf,viewport.w-x,playing_field_offset,255,(viewport.h - 1)+playing_field_offset,0);
		}
		
		advanceframe(true);
		
		if(Quit)
		{
			break;
		}
	}
	
	Hero.setDontDraw(false);
	show_subscreen_items=true;
	show_subscreen_dmap_dots=true;
	show_subscreen_numbers=true;
	show_subscreen_life=true;
}

void closescreen(int32_t shape)
{
	is_opening_screen = false;
	reset_pal_cycling();
	black_opening_count=0;
	
	if(COOLSCROLL || shape>-1)
	{
		close_black_opening(HeroX()+8, (HeroY()-HeroZ()-HeroFakeZ())+8+playing_field_offset, true, shape);
		return;
	}
	else
	{
		Hero.setDontDraw(true);
		show_subscreen_dmap_dots=false;
		show_subscreen_numbers=false;
		//	show_subscreen_items=false;
		show_subscreen_life=false;
	}
	
	int32_t x=128;
	
	FFCore.warpScriptCheck();
	for(int32_t i=79; i>=0; --i)
	{
		draw_screen();
		x=128-(((i*128/80)/8)*8);
		
		if(x>0)
		{
			rectfill(framebuf,0,playing_field_offset,x,(viewport.h - 1)+playing_field_offset,0);
			rectfill(framebuf,viewport.w-x,playing_field_offset,255,(viewport.h - 1)+playing_field_offset,0);
		}
		
		advanceframe(true);
		
		if(Quit)
		{
			break;
		}
	}
	
	Hero.setDontDraw(false);
	show_subscreen_items=true;
	show_subscreen_dmap_dots=true;
}

int32_t TriforceCount()
{
	int32_t c=0;
	
	for(int32_t i=1; i<=8; i++)
		if(game->lvlitems[i]&(1 << li_mcguffin))
			++c;
			
	return c;
}

int32_t onCustomGame()
{
	auto save = get_unset_save_slot();
	if (!save)
		return D_CLOSE;

	if (prompt_for_quest_path(save->header->qstpath))
	{
		save->header->qstpath = qstpath;
		return D_O_K;
	}

	return D_CLOSE;
}

int32_t onContinue()
{
	return D_CLOSE;
}

int32_t onThrottleFPS()
{
	Throttlefps = !Throttlefps;
	zc_set_config(cfg_sect,"throttlefps", (int32_t)Throttlefps);
	return D_O_K;
}

int32_t onWinPosSave()
{
	SaveWinPos = !SaveWinPos;
	zc_set_config(cfg_sect,"save_window_position",(int32_t)SaveWinPos);
	return D_O_K;
}
int32_t onIntegerScaling()
{
	scaleForceInteger = !scaleForceInteger;
	zc_set_config("zeldadx","scaling_force_integer",(int)scaleForceInteger);
	return D_O_K;
}
int32_t onStretchGame()
{
	stretchGame = !stretchGame;
	zc_set_config("zeldadx","stretch_game_area",stretchGame?1:0);
	return D_O_K;
}

int32_t onClickToFreeze()
{
	ClickToFreeze = !ClickToFreeze;
	zc_set_config(cfg_sect,"clicktofreeze", (int32_t)ClickToFreeze);
	return D_O_K;
}

int32_t OnSaveZCConfig()
{
	if (alert_confirm("Save Configuration",
		"Are you sure that you wish to save your present configuration settings?"
		"\nThis will overwrite your prior settings!"))
	{
		save_game_configs();
		return D_O_K;
	}
	else return D_O_K;
}

int32_t OnnClearQuestDir()
{
	auto current_path = fs::current_path() / "quests";
	if (alert_confirm("Clear Current Directory Cache",
		fmt::format("Are you sure that you wish to reset where ZC Player looks for quests?"
			"The new directory will be: {}", current_path.string())))
	{
		zc_set_config("zeldadx","quest_dir","quests");
		flush_config_file();
		strcpy(qstdir,"quests");
#ifdef __EMSCRIPTEN__
		em_sync_fs();
#endif
		return D_O_K;
	}
	else return D_O_K;
}

int32_t onConsole()
{
	if ( !console_enabled )
	{
		if (alert_confirm("ZC Console",
			"Open the ZC Console?"
			"\nThis will display any messages logged by scripts,"
			" including errors."))
		{
			FFCore.ZScriptConsole(true);
		}
		return D_O_K;
	}
	else
	{
		FFCore.ZScriptConsole(false);
		return D_O_K;
	}
}

int32_t onClrConsoleOnReload()
{
	clearConsoleOnReload = !clearConsoleOnReload;
	zc_set_config("CONSOLE","clear_console_on_reload",clearConsoleOnReload?1:0);
	return D_O_K;
}
int32_t onClrConsoleOnLoad()
{
	clearConsoleOnLoad = !clearConsoleOnLoad;
	zc_set_config("CONSOLE","clear_console_on_load",clearConsoleOnLoad?1:0);
	return D_O_K;
}


int32_t onFrameSkip()
{
	FrameSkip = !FrameSkip;
	return D_O_K;
}

int32_t onSaveDragResize()
{
	SaveDragResize = !SaveDragResize;
	zc_set_config(cfg_sect,"save_drag_resize",(int32_t)SaveDragResize);
	return D_O_K;
}

int32_t onDragAspect()
{
	DragAspect = !DragAspect;
	zc_set_config(cfg_sect,"drag_aspect",(int32_t)DragAspect);
	return D_O_K;
}

int32_t onTransLayers()
{
	TransLayers = !TransLayers;
	zc_set_config(cfg_sect,"translayers",(int32_t)TransLayers);
	return D_O_K;
}

int32_t onNESquit()
{
	NESquit = !NESquit;
	zc_set_config(cfg_sect,"fastquit",(int32_t)NESquit);
	return D_O_K;
}

int32_t onVolKeys()
{
	volkeys = !volkeys;
	zc_set_config(sfx_sect,"volkeys",(int32_t)volkeys);
	return D_O_K;
}

int32_t onShowFPS()
{
	ShowFPS = !ShowFPS;
	zc_set_config(cfg_sect,"showfps",(int32_t)ShowFPS);
	return D_O_K;
}

int32_t onShowTime()
{
	ShowGameTime = !ShowGameTime;
	zc_set_config(cfg_sect,"showtime",ShowGameTime);
	return D_O_K;
}

bool is_Fkey(int32_t k)
{
	switch(k)
	{
	case KEY_F1:
	case KEY_F2:
	case KEY_F3:
	case KEY_F4:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
		return true;
	}
	
	return false;
}

int32_t onToggleRecordingNewSaves()
{
	if (zc_get_config("zeldadx", "replay_new_saves", false))
	{
		zc_set_config("zeldadx", "replay_new_saves", false);
	}
	else
	{
		zc_set_config("zeldadx", "replay_new_saves", true);
		displayinfo("Recording", "Newly created saves will be recorded and written to a replay file.");
	}
	return D_O_K;
}

#ifdef HAS_CURL
int32_t onToggleAutoUploadReplays()
{
	if (zc_get_config("zeldadx", "replay_upload", false))
	{
		zc_set_config("zeldadx", "replay_upload", false);
	}
	else
	{
		zc_set_config("zeldadx", "replay_upload", true);
		displayinfo("Replays", "Replays will be automatically uploaded. This helps development by"
			" preventing bugs and simplifying bug reports."
			"\nUpload will happen no more than once a week when closing ZC");

		if (!zc_get_config("zeldadx", "replay_new_saves", false))
			onToggleRecordingNewSaves();
	}
	return D_O_K;
}

int32_t onUploadReplays()
{
	if (alert_confirm("Upload replays",
		"Upload your replays now to assist in development?"))
	{
		int num_uploaded = replay_upload();
		displayinfo("Upload replays", fmt::format("Uploaded {} replays", num_uploaded));
	}
	return D_O_K;
}

int32_t onClearUploadCache()
{
	if (alert_confirm("Upload replays",
		"Clear the upload cache?\nThis simply deletes replays/state.json. There's no harm in doing this, but likely is not necessary."))
	{
		replay_upload_clear_cache();
	}
	return D_O_K;
}
#endif

int32_t onToggleSnapshotAllFrames()
{
	replay_set_snapshot_all_frames(!replay_is_snapshot_all_frames());
	return D_O_K;
}

int32_t onStopReplayOrRecord()
{
	if (replay_is_replaying())
	{
		replay_quit();
	}
	else if (replay_get_mode() == ReplayMode::Record)
	{
		if (!replay_get_meta_bool("test_mode"))
		{
			displayinfo("Recording", "You cannot stop recording a save file.");
			return D_CLOSE;
		}

		if (!alert_confirm("Stop Recording", "Save replay to disk and stop recording?"
			"\nThis will stop the recording."))
			return D_CLOSE;

		replay_save();
		replay_stop();
	}
	return D_O_K;
}

static int32_t handle_on_load_replay(ReplayMode mode)
{
	bool ctrl = CHECK_CTRL_CMD;
	if (Playing)
	{
		if (!alert_confirm("Replay - Warning!",
			"Loading a replay will exit the current game."
			"\nAll unsaved progress will be lost."
			"\nDo you wish to continue?"))
			return D_CLOSE;
	}

	std::string mode_string = replay_mode_to_string(mode);
	mode_string[0] = std::toupper(mode_string[0]);

	std::string msg = "Select a replay file to play back."
		"\nYou won't be able to save, and it won't effect existing saves."
		"\nYou can stop the replay and take over manually any time.";
	if (mode == ReplayMode::Update)
	{
		msg = "Select a replay file to update."
			"\nWARNING: be sure to back up the zplay file"
			"\nand verify that the updated replay works as expected!";
	}

	if (alert_confirm(mode_string, msg, "OK","Nevermind"))
	{
		std::string replay_path = "replays/";
		if(ctrl && devpwd())
			replay_path = "../../tests/replays/";
		std::string prompt = fmt::format("Load Replay ({})", REPLAY_EXTENSION);
		if (auto result = prompt_for_existing_file(prompt, REPLAY_EXTENSION, nullptr, replay_path))
			replay_path = *result;
		else
			return D_CLOSE;

		replay_quit();
		load_replay_file_deferred(mode, replay_path);
		Quit = qRESET;
		return D_CLOSE;
	}
	return D_O_K;
}

int32_t onLoadReplay()
{
	return handle_on_load_replay(ReplayMode::Replay);
}

int32_t onLoadReplayAssert()
{
	return handle_on_load_replay(ReplayMode::Assert);
}

int32_t onLoadReplayUpdate()
{
	return handle_on_load_replay(ReplayMode::Update);
}

int32_t onSaveReplay()
{
	if (replay_get_mode() == ReplayMode::Record)
	{
		if (!replay_get_meta_bool("test_mode"))
		{
			if (!alert_confirm("Save Replay",
				"This will save a copy of the replay up to this point."
				"\nThe official replay file will be untouched."
				"\nDo you wish to continue?"))
			{
				return D_CLOSE;
			}

			std::string replay_path = replay_get_replay_path().string();
			std::string prompt = fmt::format("Save Replay ({})", REPLAY_EXTENSION);
			if (auto result = prompt_for_new_file(prompt, REPLAY_EXTENSION, nullptr, replay_path))
				replay_path = *result;
			else
				return D_CLOSE;

			if (fileexists(replay_path.c_str()))
			{
				displayinfo("Save Replay", "You cannot overwrite an existing file.");
				return D_CLOSE;
			}

			replay_save(replay_path);
		}
		else
		{
			replay_save();
		}
	}
	return D_O_K;
}

enum
{
	MENUID_REPLAY_RECORDNEW,
	MENUID_REPLAY_STOP,
	MENUID_REPLAY_SAVE,
	MENUID_REPLAY_SNAP_ALL,
	MENUID_REPLAY_AUTOUPLOAD,
	MENUID_REPLAY_UPLOAD,
	MENUID_REPLAY_CLEARUPLOADCACHE,
};
static NewMenu replay_menu
{
	{ "Record new saves", onToggleRecordingNewSaves, MENUID_REPLAY_RECORDNEW },
	{},
	{ "Stop replay", onStopReplayOrRecord, MENUID_REPLAY_STOP },
	{ "Load replay", onLoadReplay },
	{ "Load replay (assert)", onLoadReplayAssert },
	{ "Load replay (update)", onLoadReplayUpdate },
	{ "Save replay", onSaveReplay, MENUID_REPLAY_SAVE },
	{ "Snapshot all frames", onToggleSnapshotAllFrames, MENUID_REPLAY_SNAP_ALL },
#ifdef HAS_CURL
	{},
	{ "Auto upload replays", onToggleAutoUploadReplays, MENUID_REPLAY_AUTOUPLOAD },
	{ "Upload replays", onUploadReplays, MENUID_REPLAY_UPLOAD },
	{ "Clear upload cache", onClearUploadCache, MENUID_REPLAY_CLEARUPLOADCACHE },
#endif
};

void call_warp_cheat_picker();
int32_t onGoTo()
{
	call_warp_cheat_picker();
	return D_O_K;
}

int32_t onGoToComplete()
{
	if(!Playing)
	{
		return D_O_K;
	}
	
	enter_sys_pal();
	music_pause();
	pause_all_sfx();
	onGoTo();
	eat_buttons();
	
	zc_readrawkey(KEY_ESC);
	
	exit_sys_pal();
	music_resume();
	resume_all_sfx();
	return D_O_K;
}

int32_t onCredits()
{
	return D_O_K;
}

void call_zc_midi_dlg();
int32_t onMIDICredits()
{
	call_zc_midi_dlg();
	return D_O_K;
}

int32_t onAbout()
{
	char buf1[80]={0};
	std::ostringstream oss;
	sprintf(buf1,"ZQuest Classic Player");
	oss << buf1 << '\n';
	sprintf(buf1,"Version: %s", getVersionString());
	oss << buf1 << '\n';
	sprintf(buf1,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(), (char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
	oss << buf1 << '\n';
	
	InfoDialog("About ZC", oss.str()).show();
	return D_O_K;
}

int32_t onQuest()
{
	string title_str = QHeader.title[0] ? fmt::format("\n--- Title ---\n{}\n", QHeader.title) : "Untitled Quest\n";
	string author_str = QHeader.author[0] ? fmt::format("\n--- Author ---\n{}\n", QHeader.author) : "No Author Listed\n";
	string qst_ver_str = QHeader.version[0] ? fmt::format("Quest Version: {}\n", QHeader.version) : "";
	InfoDialog(get_filename(qstpath), fmt::format("{}\n{}ZQ Version: {}\n{}{}",
		relativize_path(string(qstpath)), qst_ver_str, QHeader.getVerStr(), title_str, author_str)).show();
	return D_O_K;
}

void call_vidmode_dlg();
int32_t onVidMode()
{
	call_vidmode_dlg();
	return D_O_K;
}

#define addToHash(c,b,h) if(h->find(c ## key) == h->end()) \
{(*h)[c ## key]=true;} else { if ( c ## key != 0 ) b = false;}
//Added an extra statement, so that if the key is cleared to 0, the cleared
//keybinding status need not be unique. -Z ( 1st April, 2019 )

bool edit_controls_dialog();
int32_t onControls()
{
	edit_controls_dialog();
	return D_O_K;
}

void call_volume_dialog();
int32_t onSound()
{
	call_volume_dialog();
	return D_O_K;
}

int32_t onQuit()
{
	if(Playing)
	{
		string s = "End current game?";
		
		if(get_qr(qr_NOCONTINUE))
		{
			if(standalone_mode)
			{
				s += "\nThe continue screen is disabled; the game"
					"\nwill be reloaded from the last save.";
			}
			else
			{
				s += "\nThe continue screen is disabled. You will"
					"\nbe returned to the file select screen.";
			}
		}
		
		if(alert_confirm("End Current Game?", s))
		{
			disableClickToFreeze=false;
			Quit=qQUIT;
			
			// Trying to evade a door repair charge?
			if(repaircharge)
			{
				game->change_drupy(-repaircharge);
				repaircharge=0;
			}
			
			return D_CLOSE;
		}
	}
	
	return D_O_K;
}

int32_t onTryQuitMenu()
{
	return onTryQuit(true);
}

int32_t onTryQuit(bool inMenu)
{
	if(Playing && !(GameFlags & GAMEFLAG_NO_F6))
	{
		if(active_cutscene.can_f6())
		{
			if(get_qr(qr_OLD_F6))
			{
				if(inMenu) onQuit();
				else /*if(!get_qr(qr_NOCONTINUE))*/ f_Quit(qQUIT);
			}
			else
			{
				disableClickToFreeze=false;
				GameFlags |= GAMEFLAG_TRYQUIT;
			}
			return D_CLOSE;
		}
		else active_cutscene.error();
	}
	
	return D_O_K;
}

int32_t onReset()
{
	if(alert_confirm("Reset system?", "Reset system?"))
	{
		disableClickToFreeze=false;
		Quit=qRESET;
		replay_quit();
		return D_CLOSE;
	}
	
	return D_O_K;
}

int32_t onExit()
{
	if(alert_confirm("Quit ZQuest Classic?", "Quit ZQuest Classic?"))
	{
		Quit=qEXIT;
		return D_CLOSE;
	}
	
	return D_O_K;
}

int32_t onDebug()
{
	if(debug_enabled)
		set_debug(!get_debug());
	return D_O_K;
}

int32_t onHeartBeep()
{
	heart_beep=!heart_beep;
	zc_set_config(cfg_sect,"heart_beep",heart_beep);
	return D_O_K;
}

int32_t onSaveIndicator()
{
	use_save_indicator = use_save_indicator ? 0 : 1;
	zc_set_config(cfg_sect,"save_indicator",use_save_indicator);
	return D_O_K;
}

int32_t onEpilepsy()
{
	if (alert_confirm("Epilepsy Flash Reduction",
		"Enabling this will reduce the intensity of flashing and screen wave effects."
		"\nDisabling this will restore standard flash and wavy behaviour.\nProceed?"))
	{
		epilepsyFlashReduction = epilepsyFlashReduction ? 0 : 1;
		zc_set_config(cfg_sect,"epilepsy_flash_reduction",epilepsyFlashReduction);
	}
	return D_O_K;
}

bool rc = false;

int32_t onLife()
{
	if (auto num = call_get_num("Life", game->get_life()/game->get_hp_per_heart(), game->get_maxlife()/game->get_hp_per_heart(), 1))
		cheats_enqueue(Cheat::Life, *num * game->get_hp_per_heart());
	return D_O_K;
}

int32_t onHeartC()
{
	if (auto num = call_get_num("Heart Containers", game->get_maxlife()/game->get_hp_per_heart(), 65535/game->get_hp_per_heart(), 1))
		cheats_enqueue(Cheat::MaxLife, *num * game->get_hp_per_heart());
	onLife();
	return D_O_K;
}

int32_t onMagicC()
{
	if (auto num = call_get_num("Magic Containers", game->get_maxmagic()/game->get_mp_per_block(), 65535/game->get_mp_per_block(), 0))
		cheats_enqueue(Cheat::MaxMagic, *num * game->get_mp_per_block());
	if (auto num = call_get_num("Magic", game->get_magic()/game->get_mp_per_block(), game->get_maxmagic()/game->get_mp_per_block(), 0))
		cheats_enqueue(Cheat::Magic, *num * game->get_mp_per_block());
	return D_O_K;
}

int32_t onRupies()
{
	if (auto num = call_get_num("Rupees", game->get_rupies(), game->get_maxcounter(1), 0))
		cheats_enqueue(Cheat::Rupies, *num);
	return D_O_K;
}

int32_t onMaxBombs()
{
	if (auto num = call_get_num("Max Bombs", game->get_maxbombs(), 0xFFFF, 0))
	{
		cheats_enqueue(Cheat::MaxBombs, *num);
		cheats_enqueue(Cheat::Bombs, *num);
	}
	return D_O_K;
}

int32_t onRefillLife()
{
	cheats_enqueue(Cheat::Life, game->get_maxlife());
	return D_O_K;
}
int32_t onRefillMagic()
{
	cheats_enqueue(Cheat::Magic, game->get_maxmagic());
	return D_O_K;
}
int32_t onClock()
{
	cheats_enqueue(Cheat::Clock);
	return D_O_K;
}

int32_t onQstPath()
{
	char initial_path[2048];
	chop_path(qstdir);
	strcpy(initial_path, qstdir);
	
	if (auto result = prompt_for_existing_folder("Quest File Directory", initial_path, "qst"))
	{
		char* path = result->data();
		chop_path(path);
		fix_filename_case(path);
		fix_filename_slashes(path);
		strcpy(qstdir,path);
		strcpy(qstpath,qstdir);
		zc_set_config("zeldadx","quest_dir",qstdir);
		flush_config_file();
	}

	return D_O_K;
}

#include "dialog/cheat_dialog.h"
int32_t onCheat()
{
	call_setcheat_dialog();
	game->set_cheat(maxcheat);
	if(cheat) game->did_cheat(true);
	return D_O_K;
}

int32_t onCheatRupies()
{
	cheats_enqueue(Cheat::Rupies, game->get_maxcounter(1));
	return D_O_K;
}

int32_t onCheatArrows()
{
	cheats_enqueue(Cheat::Arrows, game->get_maxarrows());
	return D_O_K;
}

int32_t onCheatBombs()
{
	cheats_enqueue(Cheat::Bombs, game->get_maxbombs(), game->get_maxcounter(6));
	return D_O_K;
}

void call_screen_saver_settings();
int32_t onScreenSaver()
{
	call_screen_saver_settings();
	return D_O_K;
}

/*****  Menus  *****/

enum
{
	MENUID_GAME_LOADQUEST,
	MENUID_GAME_ENDGAME,
};
static NewMenu game_menu
{
	{ "&Continue","ESC", onContinue },
	{},
	{ "L&oad Quest...", onCustomGame, MENUID_GAME_LOADQUEST },
	{ "&End Game","F6", onTryQuitMenu, MENUID_GAME_ENDGAME },
	{},
#ifdef __EMSCRIPTEN__
	{ "&Reset","F7", onReset },
#elif defined(ALLEGRO_MACOSX)
	{ "&Reset","F7", onReset },
	{ "&Quit","F8", onExit },
#else
	{ "&Reset","F9", onReset },
	{ "&Quit","F10", onExit },
#endif
};

static NewMenu snapshot_format_menu
{
	{ "&BMP", std::bind(onSetSnapshotFormat, ssfmtBMP) },
	{ "&GIF", std::bind(onSetSnapshotFormat, ssfmtGIF) },
	{ "&JPG", std::bind(onSetSnapshotFormat, ssfmtJPG) },
	{ "&PNG", std::bind(onSetSnapshotFormat, ssfmtPNG) },
	{ "PC&X", std::bind(onSetSnapshotFormat, ssfmtPCX) },
	{ "&TGA", std::bind(onSetSnapshotFormat, ssfmtTGA) },
};

static NewMenu bottom_8_pixels_menu
{
	{ "&Default (qst)", std::bind(onSetBottom8Pixels, 0) },
	{ "&On", std::bind(onSetBottom8Pixels, 1) },
	{ "&Off", std::bind(onSetBottom8Pixels, 2) },
};  

static NewMenu name_entry_mode_menu
{
	{ "&Keyboard", onKeyboardEntry },
	{ "&Letter Grid", onLetterGridEntry },
	{ "&Extended Letter Grid", onExtLetterGridEntry },
};

enum
{
	MENUID_WINDOW_LOCK_ASPECT,
	MENUID_WINDOW_LOCK_INTSCALE,
	MENUID_WINDOW_SAVE_SIZE,
	MENUID_WINDOW_SAVE_POS,
	MENUID_WINDOW_STRETCH,
};
static NewMenu window_menu
{
	{ "Lock Aspect Ratio", onDragAspect, MENUID_WINDOW_LOCK_ASPECT },
	{ "Lock Integer Scale", onIntegerScaling, MENUID_WINDOW_LOCK_INTSCALE },
	{ "Save Size Changes", onSaveDragResize, MENUID_WINDOW_SAVE_SIZE },
	{ "Save Position Changes", onWinPosSave, MENUID_WINDOW_SAVE_POS },
	{ "Stretch Game Area", onStretchGame, MENUID_WINDOW_STRETCH },
};
void call_zc_options_dlg();
enum
{
	MENUID_OPTIONS_PAUSE_BG,
	MENUID_OPTIONS_EPILEPSYPROT,
	MENUID_OPTIONS_SHOWBOTTOMPIXELS,
};
static NewMenu options_menu
{
	{ "Name &Entry Mode", &name_entry_mode_menu },
	{ "S&napshot Format", &snapshot_format_menu },
	{ "&Window Settings", &window_menu },
	{ "Epilepsy Flash Reduction", onEpilepsy, MENUID_OPTIONS_EPILEPSYPROT },
	{ "Pause In Background", onPauseInBackground, MENUID_OPTIONS_PAUSE_BG },
	{ "Show Bottom 8 Pixels", &bottom_8_pixels_menu, MENUID_OPTIONS_SHOWBOTTOMPIXELS },
	{ "More Options", call_zc_options_dlg },
};
enum
{
	MENUID_SETTINGS_CONTROLS,
	MENUID_SETTINGS_CAPFPS,
	MENUID_SETTINGS_SHOWFPS,
	MENUID_SETTINGS_SHOWTIME,
	MENUID_SETTINGS_CLICK_FREEZE,
	MENUID_SETTINGS_TRANSLAYERS,
	MENUID_SETTINGS_NESQUIT,
	MENUID_SETTINGS_VOLKEYS,
	MENUID_SETTINGS_HEARTBEEP,
	MENUID_SETTINGS_SAVEINDICATOR,
	MENUID_SETTINGS_DEBUG,
};
static NewMenu settings_menu
{
	{ "&Sound...", onSound },
	{ "C&ontrols", onControls, MENUID_SETTINGS_CONTROLS },
	{},
	{ "Options", &options_menu },
	{},
	{ "&Cap FPS","F1", onThrottleFPS, MENUID_SETTINGS_CAPFPS },
	{ "Show &FPS","F2", onShowFPS, MENUID_SETTINGS_SHOWFPS },
	{ "Show &Time", onShowTime, MENUID_SETTINGS_SHOWTIME },
	{ "Click to Freeze", onClickToFreeze, MENUID_SETTINGS_CLICK_FREEZE },
	{ "Cont. &Heart Beep", onHeartBeep, MENUID_SETTINGS_HEARTBEEP },
	{ "Show Trans. &Layers", onTransLayers, MENUID_SETTINGS_TRANSLAYERS },
	{ "Up+A+B To &Quit", onNESquit, MENUID_SETTINGS_NESQUIT },
	{ "Volume &Keys", onVolKeys, MENUID_SETTINGS_VOLKEYS },
	{ "Sa&ve Indicator", onSaveIndicator, MENUID_SETTINGS_SAVEINDICATOR },
	{},
	{ "Debu&g", onDebug, MENUID_SETTINGS_DEBUG },
};

enum
{
	MENUID_MISC_FULLSCREEN,
	MENUID_MISC_VIDMODE,
	MENUID_MISC_QUEST_INFO,
	MENUID_MISC_QUEST_DIR,
	MENUID_MISC_CONSOLE,
	MENUID_MISC_CLEAR_CONSOLE_ON_LOAD,
};
static NewMenu misc_menu
{
	{ "&About...", onAbout },
	// TODO: re-enable, but: 1) do not use a bitmap thing that is hard to update 2) update names and 3) don't use the Z-word.
	// { "&Credits...", onCredits },
	{ "&Fullscreen", onFullscreenMenu, MENUID_MISC_FULLSCREEN },
	{ "&Video Mode...", onVidMode, MENUID_MISC_VIDMODE },
	{},
	{ "&Quest Info...", onQuest, MENUID_MISC_QUEST_INFO },
	{ "Quest &MIDI Info...", onMIDICredits },
	{ "Quest &Directory...", onQstPath, MENUID_MISC_QUEST_DIR },
	{},
	{ "Take &Snapshot F12", onSnapshot },
	{ "Sc&reen Saver...", onScreenSaver },
	{ "Save ZC Configuration", OnSaveZCConfig },
	{ "Show Console", onConsole, MENUID_MISC_CONSOLE },
	{ "Clear Console on Qst Load", onClrConsoleOnLoad, MENUID_MISC_CLEAR_CONSOLE_ON_LOAD },
	{ "Clear Directory Cache", OnnClearQuestDir },
};

enum
{
	MENUID_REFILL_ARROWS,
};
static NewMenu refill_menu
{
	{ "&Life", onRefillLife },
	{ "&Magic", onRefillMagic },
	{ "&Bombs", onCheatBombs },
	{ "&Rupees", onCheatRupies },
	{ "&Arrows", onCheatArrows, MENUID_REFILL_ARROWS },
};

enum
{
	MENUID_SHOW_L0,
	MENUID_SHOW_L1,
	MENUID_SHOW_L2,
	MENUID_SHOW_L3,
	MENUID_SHOW_L4,
	MENUID_SHOW_L5,
	MENUID_SHOW_L6,
	MENUID_SHOW_OVER,
	MENUID_SHOW_PUSH,
	MENUID_SHOW_FFC,
	MENUID_SHOW_SPR,
	MENUID_SHOW_SCRIPTNAME,
	MENUID_SHOW_SOLIDITY,
	MENUID_SHOW_HITBOX,
	MENUID_SHOW_EFFECT,
};
static NewMenu show_menu
{
	{ "Combos", onShowLayer0, MENUID_SHOW_L0 },
	{ "Layer 1", onShowLayer1, MENUID_SHOW_L1 },
	{ "Layer 2", onShowLayer2, MENUID_SHOW_L2 },
	{ "Layer 3", onShowLayer3, MENUID_SHOW_L3 },
	{ "Layer 4", onShowLayer4, MENUID_SHOW_L4 },
	{ "Layer 5", onShowLayer5, MENUID_SHOW_L5 },
	{ "Layer 6", onShowLayer6, MENUID_SHOW_L6 },
	{ "Overhead Combos", onShowLayerO, MENUID_SHOW_OVER },
	{ "Push Blocks", onShowLayerP, MENUID_SHOW_PUSH },
	{ "Freeform Combos", onShowLayerF, MENUID_SHOW_FFC },
	{ "Sprites", onShowLayerS, MENUID_SHOW_SPR },
	{},
	{ "Current FFC Scripts", onShowFFScripts, MENUID_SHOW_SCRIPTNAME },
	{},
	{ "Walkability", onShowLayerW, MENUID_SHOW_SOLIDITY },
	{ "Hitboxes", onShowHitboxes, MENUID_SHOW_HITBOX },
	{ "Effects", onShowLayerE, MENUID_SHOW_EFFECT },
	{ "Info Opacity", onShowInfoOpacity },
};

enum
{
	MENUID_CHEAT_CHOP_L1,
	MENUID_CHEAT_CHOP_L2,
	MENUID_CHEAT_CHOP_L3,
	MENUID_CHEAT_CHOP_L4,
	MENUID_CHEAT_INVULN,
	MENUID_CHEAT_NOCLIP,
	MENUID_CHEAT_IGNORESV,
	MENUID_CHEAT_GOFAST,
};
static NewMenu cheat_menu
{
	{ "Set &Cheat", onCheat },
	{ MENUID_CHEAT_CHOP_L1 },
	{ "Re&fill", &refill_menu },
	{ MENUID_CHEAT_CHOP_L2 },
	{ "&Invincible", onClock, MENUID_CHEAT_INVULN },
	{ "Ma&x Bombs...", onMaxBombs },
	{ "&Heart Containers...", onHeartC },
	{ "&Magic Containers...", onMagicC },
	{ MENUID_CHEAT_CHOP_L3 },
	{ "&Hero Data...", onCheatConsole },
	{ MENUID_CHEAT_CHOP_L4 },
	{ "Walk Through &Walls", onNoWalls, MENUID_CHEAT_NOCLIP },
	{ "Hero Ignores Side&view", onIgnoreSideview, MENUID_CHEAT_IGNORESV },
	{ "&Quick Movement", onGoFast, MENUID_CHEAT_GOFAST },
	{ "&Kill All Enemies", onKillCheat },
	{ "Trigger &Secrets", onSecretsCheat },
	{ "Trigger Secrets Perm", onSecretsCheatPerm },
	{ "Show/Hide Layer", &show_menu },
	{ "Toggle &Light", onLightSwitch },
	{ "&Goto Location...", onGoTo },
};

#if DEVLEVEL > 0
int32_t devLogging();
int32_t devDebug();
int32_t devTimestmp();
#if DEVLEVEL > 1
int32_t setCheat();
#endif //DEVLEVEL > 1
enum
{
	MENUID_DEV_LOGGING,
	MENUID_DEV_DEBUG,
	MENUID_DEV_TIMESTAMP,
	MENUID_DEV_SETCHEAT,
};
static NewMenu dev_menu
{
	{ "&Force Error Log", devLogging, MENUID_DEV_LOGGING },
	// { "&Extra Debug Log", devDebug, MENUID_DEV_DEBUG },
	{ "&Timestamp Log", devTimestmp, MENUID_DEV_TIMESTAMP },
	#if DEVLEVEL > 1
	{},
	{ "Set &Cheat", setCheat, MENUID_DEV_SETCHEAT },
	#endif //DEVLEVEL > 1
};
int32_t devLogging()
{
	dev_logging = !dev_logging;
	dev_menu.select_uid(MENUID_DEV_LOGGING, dev_logging);
	return D_O_K;
}
// int32_t devDebug()
// {
	// dev_debug = !dev_debug;
	// dev_menu.select_uid(MENUID_DEV_DEBUG, dev_debug);
	// dev_menu[dv_dbg].flags = dev_debug ? D_SELECTED : 0;
	// return D_O_K;
// }
int32_t devTimestmp()
{
	dev_timestmp = !dev_timestmp;
	dev_menu.select_uid(MENUID_DEV_TIMESTAMP, dev_timestmp);
	return D_O_K;
}
#if DEVLEVEL > 1
int32_t setCheat()
{
	if (auto num = call_get_num("Cheat Level", cheat, 4, 0))
		cheat = *num;
	return D_O_K;
}
#endif //DEVLEVEL > 1
#endif //DEVLEVEL > 0

enum
{
	MENUID_PLAYER_CHEAT,
};
TopMenu the_player_menu
{
	{ "&Game", &game_menu },
	{ "&Settings", &settings_menu },
	{ "&Cheat", &cheat_menu, MENUID_PLAYER_CHEAT, true },
	{ "&Replay", &replay_menu },
	{ "&ZC", &misc_menu },
	#if DEVLEVEL > 0
	{ "&Dev", &dev_menu },
	#endif
};

int32_t onPauseInBackground()
{
	if (alert_confirm("Toggle Pause In Background",
		"This action will change whether ZC Player pauses when the window loses focus.\nProceed?"))
	{
		pause_in_background = pause_in_background ? 0 : 1;
		zc_set_config("zeldadx","pause_in_background", pause_in_background);
		int switch_type = pause_in_background ? SWITCH_PAUSE : SWITCH_BACKGROUND;
		set_display_switch_mode(fullscreen?SWITCH_BACKAMNESIA:switch_type);
		set_display_switch_callback(SWITCH_OUT, switch_out_callback);
		set_display_switch_callback(SWITCH_IN, switch_in_callback);
	}
	options_menu.select_uid(MENUID_OPTIONS_PAUSE_BG, pause_in_background);
	return D_O_K;
}

int32_t onKeyboardEntry()
{
	NameEntryMode=0;
	zc_set_config(cfg_sect,"name_entry_mode",NameEntryMode);
	return D_O_K;
}

int32_t onLetterGridEntry()
{
	NameEntryMode=1;
	zc_set_config(cfg_sect,"name_entry_mode",NameEntryMode);
	return D_O_K;
}

int32_t onExtLetterGridEntry()
{
	NameEntryMode=2;
	zc_set_config(cfg_sect,"name_entry_mode",NameEntryMode);
	return D_O_K;
}

static BITMAP* oldscreen;
int32_t onFullscreenMenu()
{
	PALETTE oldpal;
	get_palette(oldpal);

	fullscreen = !fullscreen;
	all_toggle_fullscreen(fullscreen);
	zc_set_config("zeldadx","fullscreen",fullscreen);

	zc_set_palette(oldpal);
	gui_mouse_focus=0;
	extern int32_t switch_type;
	switch_type = pause_in_background ? SWITCH_PAUSE : SWITCH_BACKGROUND;
	set_display_switch_mode(fullscreen?SWITCH_BACKAMNESIA:switch_type);
	set_display_switch_callback(SWITCH_OUT, switch_out_callback);
	set_display_switch_callback(SWITCH_IN, switch_in_callback);
	misc_menu.select_uid(MENUID_MISC_FULLSCREEN, isFullScreen());
	misc_menu.select_uid(MENUID_MISC_VIDMODE, isFullScreen());

	return D_O_K;
}

void fix_menu()
{
	if(!debug_enabled)
		settings_menu.chop_index = 13;
}

int32_t onSetSnapshotFormat(SnapshotType format)
{
	SnapshotFormat = format;
	zc_set_config("zeldadx", "snapshot_format", format);
	snapshot_format_menu.select_only_index(format);
	return D_O_K;
}

int32_t onSetBottom8Pixels(int option)
{
	ShowBottomPixels = option;
	zc_set_config("zeldadx", "bottom_8_px", option);
	bottom_8_pixels_menu.select_only_index(option);

	int qr = qr_HIDE_BOTTOM_8_PIXELS;
	bool value = false;
	if (option == 0)
		value = get_bit(quest_rules, qr) != 0; // This is the original value, as set in the qst file (or via scripting).
	else if (option == 1)
		value = false;
	else if (option == 2)
		value = true;
	enqueue_qr_change(qr, value);

	return D_O_K;
}

void updateShowBottomPixels()
{
	// It's too tricky the allow modifying the screen height between opening and closing the
	// active subscreen.
	if (subscreen_open)
		return;

	if (!GameLoaded)
		show_bottom_8px = false;
	else
		show_bottom_8px = !get_qr(qr_HIDE_BOTTOM_8_PIXELS);

	int target_bitmap_height = show_bottom_8px ? 232 : 224;
	if (framebuf->h != target_bitmap_height)
	{
		BITMAP* new_framebuf = create_bitmap_ex(8, 256, target_bitmap_height);
		clear_bitmap(new_framebuf);
		blit(framebuf, new_framebuf, 0, 0, 0, 0, new_framebuf->w, new_framebuf->h);

		BITMAP* new_framebuf_active_subscreen = create_bitmap_ex(8, 256, target_bitmap_height);
		clear_bitmap(new_framebuf_active_subscreen);
		if (framebuf_no_passive_subscreen)
			blit(framebuf_no_passive_subscreen, new_framebuf_active_subscreen, 0, 0, 0, 0, new_framebuf_active_subscreen->w, new_framebuf_active_subscreen->h);

		destroy_bitmap(framebuf);
		destroy_bitmap(framebuf_no_passive_subscreen);
		destroy_bitmap(script_menu_buf);
		destroy_bitmap(f6_menu_buf);
		destroy_bitmap(darkscr_bmp);
		destroy_bitmap(darkscr_bmp_trans);

		framebuf = new_framebuf;
		framebuf_no_passive_subscreen = new_framebuf_active_subscreen;
		script_menu_buf = create_bitmap_ex(8, 256, target_bitmap_height);
		f6_menu_buf = create_bitmap_ex(8, 256, target_bitmap_height);
		darkscr_bmp = create_bitmap_ex(8, 256, target_bitmap_height);
		darkscr_bmp_trans = create_bitmap_ex(8, 256, target_bitmap_height);

		rti_game.a4_bitmap = framebuf;
		rti_game.set_size(framebuf->w, framebuf->h);
		al_set_new_bitmap_flags(ALLEGRO_CONVERT_BITMAP);
		al_destroy_bitmap(rti_game.bitmap);
		rti_game.bitmap = create_a5_bitmap(framebuf->w, framebuf->h);
		al_destroy_bitmap(rti_infolayer.bitmap);
		rti_infolayer.bitmap = create_a5_bitmap(framebuf->w, framebuf->h);
		rti_infolayer.set_size(framebuf->w, framebuf->h);
	}
}

void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int32_t from,int32_t to)
{
	PALETTE tmp;
	
	for(int32_t i=0; i<256; i++)
	{
		tmp[i].r=r;
		tmp[i].g=g;
		tmp[i].b=b;
	}
	
	fade_interpolate(src,tmp,dest,pos,from,to);
}

void system_pal(bool force)
{
	if(is_sys_pal && !force) return;
	is_sys_pal = true;
	load_colorset(gui_colorset, syspal, jwin_a5_colors);
	hw_palette = &syspal;
	update_hw_pal = true;
}

static uint32_t entered_sys_pal = 0;
void enter_sys_pal()
{
	if(is_sys_pal)
	{
		if(entered_sys_pal)
			++entered_sys_pal;
		return;
	}
	sys_mouse();
	system_pal(true);
	++entered_sys_pal;
}
void exit_sys_pal()
{
	if(entered_sys_pal)
	{
		if(!--entered_sys_pal)
		{
			game_pal();
			game_mouse();
		}
	}
}

void switch_out_callback()
{
	if (pause_in_background && !MenuOpen)
	{
		System();
	}
}

void switch_in_callback()
{
}

void game_pal()
{
	is_sys_pal = false;
	entered_sys_pal = 0;
	hw_palette = &RAMpal;
	update_hw_pal = true;
}

static char bar_str[] = "";

void music_pause()
{
	//al_pause_duh(tmplayer);
	zcmusic_pause(zcmusic, ZCM_PAUSE);
	if(zcmixer->oldtrack)
		zcmusic_pause(zcmixer->oldtrack, ZCM_PAUSE);
	zc_midi_pause();
}

void music_resume()
{
	//al_resume_duh(tmplayer);
	zcmusic_pause(zcmusic, ZCM_RESUME);
	if (zcmixer->oldtrack)
		zcmusic_pause(zcmixer->oldtrack, ZCM_RESUME);
	zc_midi_resume();
}

bool reload_fonts = false;
void System()
{
	mouse_down = gui_mouse_b();
	music_pause();
	pause_all_sfx();
	MenuOpen = true;
	enter_sys_pal();
	//  FONT *oldfont=font;
	//  font=tfont;
	
	misc_menu.select_uid(MENUID_MISC_FULLSCREEN, isFullScreen());
	misc_menu.disable_uid(MENUID_MISC_VIDMODE, isFullScreen());
	
	#if DEVLEVEL > 1
	dev_menu.disable_uid(MENUID_DEV_SETCHEAT, !Playing);
	#endif
	game_menu.disable_uid(MENUID_GAME_LOADQUEST, get_unset_save_slot());
	game_menu.disable_uid(MENUID_GAME_ENDGAME, !Playing);
	misc_menu.disable_uid(MENUID_MISC_QUEST_INFO, !Playing);
	misc_menu.disable_uid(MENUID_MISC_QUEST_DIR, Playing);
	clear_keybuf();

	clear_bitmap(menu_bmp);
	oldscreen = screen;
	screen = menu_bmp;
	
	the_player_menu.reset_state();
	the_player_menu.position(0, 0);
	
	bool running = true;
	// if pressing ESC on keyboard, pop open menu.
	// Ignore gamepad 'Menu' button, as gamepad can't navigate the popped-open menu, and would get stuck!
	bool autopop = menu_buttons(false).first;
	menu_pressed(true); // eat menu input
	do
	{
		if(reload_fonts)
		{
			init_custom_fonts();
			clear_bitmap(menu_bmp);
			broadcast_dialog_message(MSG_DRAW, 0);
			reload_fonts = false;
		}
		if(handle_close_btn_quit())
			break;
		
		//update submenus
		{
			settings_menu.disable_uid(MENUID_SETTINGS_CONTROLS, replay_is_replaying());
			settings_menu.select_uid(MENUID_SETTINGS_CAPFPS, Throttlefps);
			settings_menu.select_uid(MENUID_SETTINGS_SHOWFPS, ShowFPS);
			settings_menu.select_uid(MENUID_SETTINGS_SHOWTIME, ShowGameTime);
			settings_menu.select_uid(MENUID_SETTINGS_CLICK_FREEZE, ClickToFreeze);
			settings_menu.select_uid(MENUID_SETTINGS_TRANSLAYERS, TransLayers);
			settings_menu.select_uid(MENUID_SETTINGS_NESQUIT, NESquit);
			settings_menu.select_uid(MENUID_SETTINGS_VOLKEYS, volkeys);

			window_menu.select_uid(MENUID_WINDOW_LOCK_ASPECT, DragAspect);
			window_menu.select_uid(MENUID_WINDOW_LOCK_INTSCALE, scaleForceInteger);
			window_menu.select_uid(MENUID_WINDOW_SAVE_SIZE, SaveDragResize);
			window_menu.select_uid(MENUID_WINDOW_SAVE_POS, SaveWinPos);
			window_menu.select_uid(MENUID_WINDOW_STRETCH, stretchGame);
			
			options_menu.select_uid(MENUID_OPTIONS_EPILEPSYPROT, epilepsyFlashReduction);
			options_menu.select_uid(MENUID_OPTIONS_PAUSE_BG, pause_in_background);
			options_menu.disable_uid(MENUID_OPTIONS_SHOWBOTTOMPIXELS, replay_is_replaying());
			
			name_entry_mode_menu.select_only_index(NameEntryMode);
			
			misc_menu.select_uid(MENUID_MISC_CONSOLE, console_enabled);
			misc_menu.select_uid(MENUID_MISC_CLEAR_CONSOLE_ON_LOAD, clearConsoleOnLoad);
			
			bool nocheat = (replay_is_replaying() || !Playing
				|| (!maxcheat && !zcheats.flags && !get_debug() && DEVLEVEL < 2 && !zqtesting_mode && !devpwd()));
			the_player_menu.disable_uid(MENUID_PLAYER_CHEAT, nocheat);
			refill_menu.disable_uid(MENUID_REFILL_ARROWS, !get_qr(qr_TRUEARROWS));
			cheat_menu.chop_index.reset();
			if(cheat < 4)
				cheat_menu.chop_index = cheat_menu.ind_at(MENUID_CHEAT_CHOP_L1+cheat);
			cheat_menu.select_uid(MENUID_CHEAT_INVULN, getClock());
			cheat_menu.select_uid(MENUID_CHEAT_NOCLIP, walk_through_walls);
			cheat_menu.select_uid(MENUID_CHEAT_IGNORESV, ignoreSideview);
			cheat_menu.select_uid(MENUID_CHEAT_GOFAST, gofast);
			
			show_menu.select_uid(MENUID_SHOW_L0, show_layers[0]);
			show_menu.select_uid(MENUID_SHOW_L1, show_layers[1]);
			show_menu.select_uid(MENUID_SHOW_L2, show_layers[2]);
			show_menu.select_uid(MENUID_SHOW_L3, show_layers[3]);
			show_menu.select_uid(MENUID_SHOW_L4, show_layers[4]);
			show_menu.select_uid(MENUID_SHOW_L5, show_layers[5]);
			show_menu.select_uid(MENUID_SHOW_L6, show_layers[6]);
			show_menu.select_uid(MENUID_SHOW_OVER, show_layer_over);
			show_menu.select_uid(MENUID_SHOW_PUSH, show_layer_push);
			show_menu.select_uid(MENUID_SHOW_SPR, show_sprites);
			show_menu.select_uid(MENUID_SHOW_FFC, show_ffcs);
			show_menu.select_uid(MENUID_SHOW_SOLIDITY, show_walkflags);
			show_menu.select_uid(MENUID_SHOW_SCRIPTNAME, show_ff_scripts);
			show_menu.select_uid(MENUID_SHOW_HITBOX, show_hitboxes);
			show_menu.select_uid(MENUID_SHOW_EFFECT, show_effectflags);
			
			settings_menu.select_uid(MENUID_SETTINGS_HEARTBEEP, heart_beep);
			settings_menu.select_uid(MENUID_SETTINGS_SAVEINDICATOR, use_save_indicator);
			
			replay_menu.by_uid(MENUID_REPLAY_STOP)->text = fmt::format("Stop {}",
				replay_get_mode() == ReplayMode::Record ? "recording" : "replaying");
			
			replay_menu.select_uid(MENUID_REPLAY_RECORDNEW, zc_get_config("zeldadx", "replay_new_saves", false));
#ifdef HAS_CURL
			replay_menu.select_uid(MENUID_REPLAY_AUTOUPLOAD, replay_upload_auto_enabled());
#endif
			replay_menu.disable_uid(MENUID_REPLAY_STOP, !replay_is_active());
			replay_menu.disable_uid(MENUID_REPLAY_SAVE, replay_get_mode() != ReplayMode::Record);
			replay_menu.select_uid(MENUID_REPLAY_SNAP_ALL, replay_is_snapshot_all_frames());
			
			snapshot_format_menu.select_only_index(SnapshotFormat);
			bottom_8_pixels_menu.select_only_index(ShowBottomPixels);
		}
		
		if(debug_enabled)
			settings_menu.select_uid(MENUID_SETTINGS_DEBUG, get_debug());
		
		if(autopop)
			clear_keybuf();
		the_player_menu.run(true);
		if(autopop)
		{
			the_player_menu.pop_sub(0, &the_player_menu);
			the_player_menu.draw(screen, the_player_menu.hovered_ind());
			autopop = false;
			update_hw_screen();
		}
		
		update_hw_screen();
		
		auto mb = gui_mouse_b();
		if(XOR(mb, mouse_down))
		{
			if(!the_player_menu.has_mouse())
				if(mb)
					break;
			mouse_down = mb;
		}
		
		if(ss_enable && input_idle() > ss_seconds * 60)
			screen_saver(ss_speed, ss_density);
		
		poll_keyboard();
		poll_joystick();
		if (menu_pressed(true))
			running = false;
		
		if(running && keypressed() && !CHECK_ALT) //System hotkeys
		{
			auto c = peekkey();
			bool eatkey = true;
			switch(c>>8)
			{
				//Spare keys used by the menu
				case KEY_UP:
				case KEY_DOWN:
				case KEY_LEFT:
				case KEY_RIGHT:
					eatkey = false;
					break;
				case KEY_F1:
					onThrottleFPS();
					break;
				case KEY_F2:
					onShowFPS();
					break;
				case KEY_F6:
					onTryQuitMenu();
					break;
				#ifndef ALLEGRO_MACOSX
				case KEY_F9:
					onReset();
					break;
				case KEY_F10:
					onExit();
					break;
				#else
				case KEY_F7:
					onReset();
					break;
				case KEY_F8:
					onExit();
					break;
				#endif
				case KEY_F12:
					onSnapshot();
					break;
				case KEY_TAB:
					onDebug();
					break;
				case KEY_ESC:
					// is handled by menu button check
					break;
			}
			if(eatkey)
				readkey();
		}
		if(Quit || (GameFlags & GAMEFLAG_TRYQUIT))
			break;
	}
	while(running);

	screen = oldscreen;
	
	mouse_down=gui_mouse_b();
	MenuOpen = false;
	if(Quit)
	{
		kill_sfx();
		music_stop();
		update_hw_screen();
	}
	else
	{
		music_resume();
		resume_all_sfx();
		
		if(rc)
			ringcolor(false);
	}
	exit_sys_pal();
	
	eat_buttons();
	
	rc=false;
	clear_keybuf();

	zc_init_apply_cheat_delta();
}

void fix_dialogs()
{
	jwin_center_dialog(gamemode_dlg);
}

INLINE int32_t mixvol(int32_t v1,int32_t v2)
{
	return (zc_min(v1,255)*zc_min(v2,255)) >> 8;
}

int32_t get_emusic_volume()
{
	int32_t temp_volume = emusic_volume;
	if (GameLoaded && !get_qr(qr_OLD_SCRIPT_VOLUME))
		temp_volume = (emusic_volume * FFCore.usr_music_volume) / 10000 / 100;
	if (!zcmusic)
		return temp_volume;
	return (temp_volume * zcmusic->fadevolume) / 10000;
}

int32_t get_zcmusicpos()
{
	int32_t debugtracething = zcmusic_get_curpos(zcmusic);
	return debugtracething;
	return 0;
}

void set_zcmusicpos(int32_t position)
{
	zcmusic_set_curpos(zcmusic, position);
}

void set_zcmusicspeed(int32_t speed)
{
	zcmusic_set_speed(zcmusic, speed);
}

int32_t get_zcmusiclen()
{
	return zcmusic_get_length(zcmusic);
}

void set_zcmusicloop(double start, double end)
{
	zcmusic_set_loop(zcmusic, start, end);
}

void jukebox(int index, int loop)
{
	if (is_headless())
		return;

	music_stop();
	
	if (index < 0 || index >= MAXMIDIS)
		return;
	
	// Allegro's DIGMID driver (the one normally used on on Linux) gets
	// stuck notes when a song stops. This fixes it.
	if(strcmp(midi_driver->name, "DIGMID")==0)
		zc_set_volume(0, 0);
	
	zc_set_volume(-1, mixvol(tunes[index].volume, midi_volume >>1));
	zc_play_midi(tunes[index].data,loop);
	
	if(tunes[index].start>0)
		zc_midi_seek(tunes[index].start);
		
	midi_loop_start = tunes[index].loop_start;
	midi_loop_end = tunes[index].loop_end;
	
	currmidi=index;
	master_volume(digi_volume, midi_volume);
	//midi_paused=false;
}

void jukebox(int index)
{
	// do nothing if it's already playing
	if (index == currmidi && midi_pos >= 0)
		return;
	int loop = 0;
	if (unsigned(index) < MAXMIDIS)
		loop = tunes[index].loop;
	
	jukebox(index, loop);
}

void playLevelMusic()
{
	if (is_headless())
		return;

	int32_t m = hero_scr->music;

	switch(m)
	{
	case -2:
		music_stop();
		return;
		
	case -1:
		m = DMaps[cur_dmap].music;
	[[fallthrough]];
	default:
		if (m > 0 && m <= quest_music.size())
		{
			auto& amus = quest_music[m-1];
			if (!amus.is_playing()) // avoid resetting the song
				amus.play();
		}
		else
			music_stop();
	}
	engine_music_active = true;
}

void master_volume(int32_t dv,int32_t mv)
{
	if(dv>=0) digi_volume=zc_max(zc_min(dv,255),0);
	
	if(mv>=0) midi_volume=zc_max(zc_min(mv,255),0);
	
	int32_t i = vbound(currmidi, MAXMIDIS-1, 0);
	int32_t temp_vol = midi_volume;
	if (!get_qr(qr_OLD_SCRIPT_VOLUME))
		temp_vol = (midi_volume * FFCore.usr_music_volume) / 10000 / 100;
	zc_set_volume(digi_volume,mixvol(tunes[i].volume, temp_vol));
}

void Z_init_sound()
{
	const char* midis[ZC_MIDI_COUNT] = {
		"assets/dungeon.mid",
		"assets/ending.mid",
		"assets/gameover.mid",
		"assets/level9.mid",
		"assets/overworld.mid",
		"assets/title.mid",
		"assets/triforce.mid",
	};
	for(uint q = 0; q < ZC_MIDI_COUNT; ++q)
	{
		tunes[q].data = load_midi(midis[q]);
		if (!tunes[q].data)
			Z_error_fatal("Missing required file %s\n", midis[q]);
	}

	for(uint q = 0; q < MAXCUSTOMMIDIS; ++q)
		tunes[ZC_MIDI_COUNT + q].data = nullptr;
		
	master_volume(digi_volume, midi_volume);
}

// Stops SFX played by Hero's item of the given family
void stop_item_sfx(int32_t family)
{
	int32_t id=current_item_id(family);
	
	if(id<0)
		return;
		
	stop_sfx(itemsbuf[id].usesound);
}

// TODO: when far out of bounds, sounds should dampen. currently we only pan.
int32_t pan(int32_t x)
{
	switch(pan_style)
	{
	// MONO
	case 0:
		return 128;

	// 1/2
	case 1:
		x -= viewport.x;
		return vbound((x>>1)+68,0,255);

	// 3/4
	case 2:
		x -= viewport.x;
		return vbound(((x*3)>>2)+36,0,255);

	// FULL
	case 3:
	default:
		x -= viewport.x;
		return vbound(x,0,255);
	}
}

int32_t button_pressed()
{
	if (active_control_scheme->joystick_index >= MAX_JOYSTICKS)
		return 0;

	for(int32_t i=1; i<=joy[active_control_scheme->joystick_index].num_buttons; i++)
	{
		if(joybtn(active_control_scheme->joystick_index, i))
			return i;
	}

	return 0;
}

static bool rButton(bool &btn, bool &flag, bool rawbtn)
{
	bool ret = btn && !flag;
	flag = rawbtn;
	
	return ret;
}
static bool rButton(bool &btn, bool &flag)
{
	bool ret = btn && !flag;
	flag = btn;
	
	return ret;
}
static bool rButtonPeek(bool btn, bool flag)
{
	if(!btn)
	{
		return false;
	}
	else if(!flag)
	{
		return true;
	}
	
	return false;
}

// Updated only by keyboard/gamepad.
// If in replay mode, this is set directly by the replay system.
// This should never be read from directly - use control_state instead.
bool raw_control_state[controls::btnLast];

// Every call to load_control_state (pretty much every frame) resets this to be equal to raw_control_state.
// This state can drift from raw_control_state if button states are "eaten" or overriden by a script. But that only
// lasts until the next call to load_control_state.
bool control_state[controls::btnLast];
bool disable_control[controls::btnLast];
bool drunk_toggle_state[11];
bool disabledKeys[127];
bool KeyInput[127];
bool KeyPress[127];

bool key_current_frame[127];
bool key_previous_frame[127];

static bool key_system[127];
static bool key_system_previous[127];
static bool key_system_press[127];

bool button_press[controls::btnLast];
bool button_hold[controls::btnLast];

#define STICK_1_X joy[active_control_scheme->joystick_index].stick[active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_x][control_scheme::data_stick]].axis[active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_x][control_scheme::data_axis]]
#define STICK_1_Y joy[active_control_scheme->joystick_index].stick[active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_y][control_scheme::data_stick]].axis[active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_y][control_scheme::data_axis]]
#define STICK_1_X_OFFSET active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_x][control_scheme::data_offset]
#define STICK_1_Y_OFFSET active_control_scheme->stick_data[control_scheme::stick_1][control_scheme::axis_y][control_scheme::data_offset]

#define STICK_2_X joy[active_control_scheme->joystick_index].stick[active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_x][control_scheme::data_stick]].axis[active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_x][control_scheme::data_axis]]
#define STICK_2_Y joy[active_control_scheme->joystick_index].stick[active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_y][control_scheme::data_stick]].axis[active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_y][control_scheme::data_axis]]
#define STICK_2_X_OFFSET active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_x][control_scheme::data_offset]
#define STICK_2_Y_OFFSET active_control_scheme->stick_data[control_scheme::stick_2][control_scheme::axis_y][control_scheme::data_offset]
#define STICK_PRECISION   56 //define your own sensitivity

void load_control_state()
{
	load_control_called_this_frame = true;

	if (replay_version_check(8, 11))
	{
		for (int i = 0; i < controls::btnLast; i++)
			down_control_states[i] = raw_control_state[i];
	}

	if (!replay_is_replaying())
	{
		raw_control_state[0]=zc_getrawkey(active_control_scheme->keys[btnUp], true)||(active_control_scheme->analog_movement ? STICK_1_Y.d1 || STICK_1_Y.pos - STICK_1_Y_OFFSET < -STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnUp]));
		raw_control_state[1]=zc_getrawkey(active_control_scheme->keys[btnDown], true)||(active_control_scheme->analog_movement ? STICK_1_Y.d2 || STICK_1_Y.pos - STICK_1_Y_OFFSET > STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnDown]));
		raw_control_state[2]=zc_getrawkey(active_control_scheme->keys[btnLeft], true)||(active_control_scheme->analog_movement ? STICK_1_X.d1 || STICK_1_X.pos - STICK_1_X_OFFSET < -STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnLeft]));
		raw_control_state[3]=zc_getrawkey(active_control_scheme->keys[btnRight], true)||(active_control_scheme->analog_movement ? STICK_1_X.d2 || STICK_1_X.pos - STICK_1_X_OFFSET > STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnRight]));
		raw_control_state[4]=zc_getrawkey(active_control_scheme->keys[btnA], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnA]);
		raw_control_state[5]=zc_getrawkey(active_control_scheme->keys[btnB], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnB]);
		raw_control_state[6]=zc_getrawkey(active_control_scheme->keys[btnS], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnS]);
		raw_control_state[7]=zc_getrawkey(active_control_scheme->keys[btnL], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnL]);
		raw_control_state[8]=zc_getrawkey(active_control_scheme->keys[btnR], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnR]);
		raw_control_state[9]=zc_getrawkey(active_control_scheme->keys[btnP], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnP]);
		raw_control_state[10]=zc_getrawkey(active_control_scheme->keys[btnEx1], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx1]);
		raw_control_state[11]=zc_getrawkey(active_control_scheme->keys[btnEx2], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx2]);
		raw_control_state[12]=zc_getrawkey(active_control_scheme->keys[btnEx3], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx3]);
		raw_control_state[13]=zc_getrawkey(active_control_scheme->keys[btnEx4], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx4]);
		
		if(num_joysticks != 0)
		{
			raw_control_state[14] = STICK_2_Y.pos - STICK_2_Y_OFFSET < -STICK_PRECISION;
			raw_control_state[15] = STICK_2_Y.pos - STICK_2_Y_OFFSET > STICK_PRECISION;
			raw_control_state[16] = STICK_2_X.pos - STICK_2_X_OFFSET < -STICK_PRECISION;
			raw_control_state[17] = STICK_2_X.pos - STICK_2_X_OFFSET > STICK_PRECISION;
		}
		else
		{
			raw_control_state[14] = false;
			raw_control_state[15] = false;
			raw_control_state[16] = false;
			raw_control_state[17] = false;
		}
	}
	if (replay_is_active())
	{
		if (replay_get_version() < 3)
			replay_poll();
		else if (replay_is_replaying() && replay_get_version() < 6)
			replay_peek_input();
		else if (replay_is_replaying() && replay_version_check(8, 11))
			replay_peek_input();
		if (replay_get_version() == 8)
			update_keys();
	}

	// Some test replay files were made before a serious input bug was fixed, so instead
	// of re-doing them or tossing them out, just check for that zplay version.
	bool botched_input = replay_is_active() && replay_get_version() != 1 && replay_get_version() < 8;
	for (int i = 0; i < controls::btnLast; i++)
	{
		control_state[i] = raw_control_state[i];
		if (botched_input && !control_state[i])
			down_control_states[i] = false;
	}
	bool did_bad_cutscene_btn = false;
	for(int q = 0; q < 18; ++q)
		if(control_state[q] && !active_cutscene.can_button(q))
		{
			control_state[q] = false;
			did_bad_cutscene_btn = true;
		}
	if(did_bad_cutscene_btn)
		active_cutscene.error();
	
	button_press[0]=rButton(control_state[0],button_hold[0]);
	button_press[1]=rButton(control_state[1],button_hold[1]);
	button_press[2]=rButton(control_state[2],button_hold[2]);
	button_press[3]=rButton(control_state[3],button_hold[3]);
	button_press[4]=rButton(control_state[4],button_hold[4]);
	button_press[5]=rButton(control_state[5],button_hold[5]);
	button_press[6]=rButton(control_state[6],button_hold[6]);
	button_press[7]=rButton(control_state[7],button_hold[7]);
	button_press[8]=rButton(control_state[8],button_hold[8]);
	button_press[9]=rButton(control_state[9],button_hold[9]);
	button_press[10]=rButton(control_state[10],button_hold[10]);
	button_press[11]=rButton(control_state[11],button_hold[11]);
	button_press[12]=rButton(control_state[12],button_hold[12]);
	button_press[13]=rButton(control_state[13],button_hold[13]);
	button_press[14]=rButton(control_state[14],button_hold[14]);
	button_press[15]=rButton(control_state[15],button_hold[15]);
	button_press[16]=rButton(control_state[16],button_hold[16]);
	button_press[17]=rButton(control_state[17],button_hold[17]);
}

// Returns true if any game key is pressed. This is needed because keypressed()
// doesn't detect modifier keys and control_state[] can be modified by scripts.
bool zc_key_pressed()
//may also need to use zc_getrawkey
{
	if((zc_getrawkey(active_control_scheme->keys[btnUp], true)||(active_control_scheme->analog_movement ? STICK_1_Y.d1 || STICK_1_Y.pos - STICK_1_Y_OFFSET< -STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnUp]))) ||
	   (zc_getrawkey(active_control_scheme->keys[btnDown], true)||(active_control_scheme->analog_movement ? STICK_1_Y.d2 || STICK_1_Y.pos - STICK_1_Y_OFFSET > STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnDown]))) ||
	   (zc_getrawkey(active_control_scheme->keys[btnLeft], true)||(active_control_scheme->analog_movement ? STICK_1_X.d1 || STICK_1_X.pos - STICK_1_X_OFFSET < -STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnLeft]))) ||
	   (zc_getrawkey(active_control_scheme->keys[btnRight], true)||(active_control_scheme->analog_movement ? STICK_1_X.d2 || STICK_1_X.pos - STICK_1_X_OFFSET > STICK_PRECISION : joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnRight]))) ||
	   (zc_getrawkey(active_control_scheme->keys[btnA], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnA])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnB], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnB])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnS], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnS])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnL], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnL])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnR], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnR])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnP], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnP])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnEx1], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx1])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnEx2], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx2])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnEx3], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx3])) ||
	   (zc_getrawkey(active_control_scheme->keys[btnEx4], true)||joybtn(active_control_scheme->joystick_index, active_control_scheme->btns[btnEx4]))) // Skipping joystick axes
		return true;
	
	return false;
}

bool getInput(int32_t btn, int input_flags)
{
	if(FFCore.kb_typing_mode) return false;
	if((input_flags & INPUT_HERO_ACTION) && Hero.no_control())
		return false;
	
	bool press = input_flags & INPUT_PRESS;
	bool drunk = input_flags & INPUT_DRUNK;
	bool ignoreDisable = input_flags & INPUT_IGNORE_DISABLE;
	bool eatEntirely = input_flags & INPUT_EAT_ENTIRELY;
	bool peek = input_flags & INPUT_PEEK;
	
	bool drunkstate = btn < 11 && drunk_toggle_state[btn];
	if(!ignoreDisable && get_qr(qr_FIXDRUNKINPUTS) && disable_control[btn])
		drunkstate = false;
	
	bool* flag = &down_control_states[btn];
	bool ret = control_state[btn] && (ignoreDisable || !disable_control[btn]);
	bool rawret = raw_control_state[btn];
	
	assert(flag);
	if(press)
	{
		if(peek)
			ret = rButtonPeek(ret, *flag);
		else if(get_qr(qr_BROKEN_INPUT_DOWN_STATE)) ret = rButton(ret, *flag);
		else ret = rButton(ret, *flag, rawret);
	}
	if(eatEntirely && ret)
		control_state[btn] = false;
	if(drunk && drunkstate)
		ret = !ret;
	return ret;
}

std::pair<bool, bool> menu_buttons(bool just_pressed)
{
	static bool menu_pressed_state = false;
	bool menu_kb = key[KEY_ESC];
	bool menu_joy = joybtn(active_control_scheme->joystick_index, active_control_scheme->btn_menu);
	if (just_pressed)
	{
		if (menu_pressed_state)
		{
			if (menu_joy || menu_kb)
			{
				menu_joy = false;
				menu_kb = false;
			}
			else menu_pressed_state = false;
		}
		else menu_pressed_state = menu_joy || menu_kb;
	}
	return {menu_kb, menu_joy};
}
bool menu_pressed(bool just_pressed)
{
	auto [kb, joy] = menu_buttons(just_pressed);
	return kb || joy;
}

byte getIntBtnInput(byte intbtn, int input_flags)
{
	byte ret = 0;
	if(intbtn & INT_BTN_A) ret |= getInput(btnA, input_flags) ? INT_BTN_A : 0;
	if(intbtn & INT_BTN_B) ret |= getInput(btnB, input_flags) ? INT_BTN_B : 0;
	if(intbtn & INT_BTN_L) ret |= getInput(btnL, input_flags) ? INT_BTN_L : 0;
	if(intbtn & INT_BTN_R) ret |= getInput(btnR, input_flags) ? INT_BTN_R : 0;
	if(intbtn & INT_BTN_EX1) ret |= getInput(btnEx1, input_flags) ? INT_BTN_EX1 : 0;
	if(intbtn & INT_BTN_EX2) ret |= getInput(btnEx2, input_flags) ? INT_BTN_EX2 : 0;
	if(intbtn & INT_BTN_EX3) ret |= getInput(btnEx3, input_flags) ? INT_BTN_EX3 : 0;
	if(intbtn & INT_BTN_EX4) ret |= getInput(btnEx4, input_flags) ? INT_BTN_EX4 : 0;
	return ret; //No early return, to make sure all button presses are eaten that should be! -Em
}

byte checkIntBtnVal(byte intbtn, byte vals)
{
	return intbtn&vals;
}

void eat_buttons()
{
	getInput(btnA, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnB, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnS, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnL, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnR, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnP, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnEx1, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnEx2, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnEx3, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	getInput(btnEx4, INPUT_PRESS | INPUT_IGNORE_DISABLE);
}

// Is true for the _first frame_ of a key press.
// But! it is possible that a script manually sets the value of KeyPress,
// in which case it will be restored to the "true" value based on `key_current_frame`
// and `key_previous_frame` on the next frame.
bool zc_readkey(int32_t k, bool ignoreDisable)
{
	if(ignoreDisable) return KeyPress[k];
	switch(k)
	{
		case KEY_F7:
		case KEY_F8:
		case KEY_F9:
			return KeyPress[k];
			
		default:
			return KeyPress[k] && !disabledKeys[k];
	}
}

// Is true for _every frame_ a key is held down.
// But! it is possible that a script manually sets the value of KeyInput,
// in which case it will be restored to the "true" value based on `key_current_frame`
// on the next frame.
bool zc_getkey(int32_t k, bool ignoreDisable)
{
	if(ignoreDisable) return KeyInput[k];
	switch(k)
	{
		case KEY_F7:
		case KEY_F8:
		case KEY_F9:
			return KeyInput[k];
			
		default:
			return KeyInput[k] && !disabledKeys[k];
	}
}

// Reads (and then clears) the current frame key state directly.
// Scripts can also modify `key_current_frame`.
bool zc_readrawkey(int32_t k, bool ignoreDisable)
{
	if(zc_getrawkey(k, ignoreDisable))
	{
		_key[k]=key[k]=key_current_frame[k]=0;
		return true;
	}
	_key[k]=key[k]=key_current_frame[k]=0;
	return false;
}

// Reads the current frame key state directly.
// Scripts can also modify `key_current_frame`.
bool zc_getrawkey(int32_t k, bool ignoreDisable)
{
	if(ignoreDisable) return key_current_frame[k];
	switch(k)
	{
		case KEY_F7:
		case KEY_F8:
		case KEY_F9:
			return key_current_frame[k];
			
		default:
			return key_current_frame[k] && !disabledKeys[k];
	}
}

// Only used for a handful of keys, like tilde and Function keys.
// This state is never read within the game.
// It exists so that all keyboard input still functions during replay,
// without inadvertently doing things like toggling throttling if the player
// presses ~
bool zc_get_system_key(int32_t k)
{
	return key_system[k];
}

// True for the _first_ frame of a key press.
bool zc_read_system_key(int32_t k)
{
	return key_system_press[k];
}

bool is_system_key(int32_t k)
{
	switch (k)
	{
		case KEY_BACKQUOTE:
		case KEY_CLOSEBRACE:
		case KEY_END:
		case KEY_HOME:
		case KEY_OPENBRACE:
		case KEY_PGDN:
		case KEY_PGUP:
		case KEY_TAB:
		case KEY_TILDE:
			return true;
	}
	return is_Fkey(k);
}

void update_system_keys()
{
	for (int32_t q = 0; q < 127; ++q)
	{
		if (!is_system_key(q))
			continue;

		key_system[q] = key[q];
		key_system_press[q] = key_system[q] && !key_system_previous[q];
		key_system_previous[q] = key_system[q];
	}
}

void update_keys()
{
	for (int32_t q = 0; q < 127; ++q)
	{
		// When replaying, replay.cpp takes care of updating `key_current_frame`.
		if (!replay_is_replaying())
			key_current_frame[q] = key[q];

		KeyPress[q] = key_current_frame[q] && !key_previous_frame[q];
		KeyInput[q] = key_current_frame[q];
		key_previous_frame[q] = key_current_frame[q];
	}
}

