//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_sys.cc
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

// to prevent <map> from generating errors
#define __GTHREAD_HIDE_WIN32API 1

#include "precompiled.h" //always first

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
#include <ctype.h>
#include "zc_alleg.h"
#include "gamedata.h"
#include "zc_init.h"
#include "zquest.h"
#include "init.h"

#ifdef ALLEGRO_DOS
#include <unistd.h>
#endif

#include "zdefs.h"
#include "zelda.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "zsys.h"
#include "qst.h"
#include "zc_sys.h"
#include "debug.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "gui.h"
#include "midi.h"
#include "subscr.h"
#include "maps.h"
#include "sprite.h"
#include "guys.h"
#include "link.h"
#include "title.h"
#include "particles.h"
#include "mem_debug.h"
#include "zconsole.h"

int sfx_voice[WAV_COUNT];
int d_stringloader(int msg,DIALOG *d,int c);

extern FONT *lfont;
extern LinkClass Link;
extern FFScript FFCore;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
extern int loadlast;
byte disable_direct_updating;
byte use_dwm_flush;
byte use_save_indicator;
byte midi_patch_fix;
bool midi_paused=false;

extern bool kb_typing_mode; //script only, for disbaling key presses affecting Link, etc. 

//extern movingblock mblock2; //mblock[4]?
//extern int db;

static const char *ZC_str = "Zelda Classic";
#ifdef ALLEGRO_DOS
static  const char *qst_dir_name = "dos_qst_dir";
#elif defined(ALLEGRO_WINDOWS)
static  const char *qst_dir_name = "win_qst_dir";
#elif defined(ALLEGRO_LINUX)
static  const char *qst_dir_name = "linux_qst_dir";
#elif defined(ALLEGRO_MACOSX)
static  const char *qst_dir_name = "macosx_qst_dir";
#endif
#ifdef ALLEGRO_LINUX
static  const char *samplepath = "samplesoundset/patches.dat";
#endif

#ifdef _MSC_VER
#define getcwd _getcwd
#endif

bool rF12();
bool rF5();
bool rF11();
bool rI();
bool rQ();
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
        int oldwidth = d[0].w;
        int oldheight = d[0].h;
        int oldx = d[0].x;
        int oldy = d[0].y;
        d[0].x -= int(d[0].w/RESIZE_AMT);
        d[0].y -= int(d[0].h/RESIZE_AMT);
        d[0].w = int(d[0].w*RESIZE_AMT);
        d[0].h = int(d[0].h*RESIZE_AMT);
        
        for(int i=1; d[i].proc !=NULL; i++)
        {
            // Place elements horizontally
            double xpc = ((double)(d[i].x - oldx) / (double)oldwidth);
            d[i].x = int(d[0].x + (xpc*d[0].w));
            
            if(d[i].proc != d_stringloader)
            {
                if(d[i].proc==d_bitmap_proc)
                {
                    d[i].w *= 2;
                }
                else d[i].w = int(d[i].w*RESIZE_AMT);
            }
            
            // Place elements vertically
            double ypc = ((double)(d[i].y - oldy) / (double)oldheight);
            d[i].y = int(d[0].y + (ypc*d[0].h));
            
            // Vertically resize elements
            if(d[i].proc == jwin_edit_proc || d[i].proc == jwin_check_proc || d[i].proc == jwin_checkfont_proc)
            {
                d[i].h = int((double)d[i].h*1.5);
            }
            else if(d[i].proc == jwin_droplist_proc)
            {
                d[i].y += int((double)d[i].h*0.25);
                d[i].h = int((double)d[i].h*1.25);
            }
            else if(d[i].proc==d_bitmap_proc)
            {
                d[i].h *= 2;
            }
            else d[i].h = int(d[i].h*RESIZE_AMT);
            
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
    
    for(int i=1; d[i].proc!=NULL; i++)
    {
        if(d[i].proc==jwin_slider_proc)
            continue;
            
        // Bigger font
        bool bigfontproc = (d[i].proc != jwin_initlist_proc && d[i].proc != d_midilist_proc && d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != jwin_list_proc);
        
        if(!d[i].dp2 && bigfontproc)
        {
            //d[i].dp2 = (d[i].proc == jwin_edit_proc) ? sfont3 : lfont_l;
            d[i].dp2 = lfont_l;
        }
        else if(!bigfontproc)
        {
//      ((ListData *)d[i].dp)->font = &sfont3;
            ((ListData *)d[i].dp)->font = &lfont_l;
        }
        
        // Make checkboxes work
        if(d[i].proc == jwin_check_proc)
            d[i].proc = jwin_checkfont_proc;
        else if(d[i].proc == jwin_radio_proc)
            d[i].proc = jwin_radiofont_proc;
    }
    
    jwin_center_dialog(d);
}


/**********************************/
/******** System functions ********/
/**********************************/

static char cfg_sect[] = "zeldadx";

int d_dummy_proc(int msg,DIALOG *d,int c)
{
    msg=msg;
    d=d;
    c=c; /*these are here to bypass compiler warnings about unused arguments*/ return D_O_K;
}

void load_game_configs()
{
    joystick_index = get_config_int(cfg_sect,"joystick_index",0);
    js_stick_1_x_stick = get_config_int(cfg_sect,"js_stick_1_x_stick",0);
    js_stick_1_x_axis = get_config_int(cfg_sect,"js_stick_1_x_axis",0);
    js_stick_1_x_offset = get_config_int(cfg_sect,"js_stick_1_x_offset",0) ? 128 : 0;
    js_stick_1_y_stick = get_config_int(cfg_sect,"js_stick_1_y_stick",0);
    js_stick_1_y_axis = get_config_int(cfg_sect,"js_stick_1_y_axis",1);
    js_stick_1_y_offset = get_config_int(cfg_sect,"js_stick_1_y_offset",0) ? 128 : 0;
    js_stick_2_x_stick = get_config_int(cfg_sect,"js_stick_2_x_stick",1);
    js_stick_2_x_axis = get_config_int(cfg_sect,"js_stick_2_x_axis",0);
    js_stick_2_x_offset = get_config_int(cfg_sect,"js_stick_2_x_offset",0) ? 128 : 0;
    js_stick_2_y_stick = get_config_int(cfg_sect,"js_stick_2_y_stick",1);
    js_stick_2_y_axis = get_config_int(cfg_sect,"js_stick_2_y_axis",1);
    js_stick_2_y_offset = get_config_int(cfg_sect,"js_stick_2_y_offset",0) ? 128 : 0;
    analog_movement = get_config_int(cfg_sect,"analog_movement",1);
    
    if((unsigned int)joystick_index >= MAX_JOYSTICKS)
        joystick_index = 0;
        
    Akey = get_config_int(cfg_sect,"key_a",KEY_ALT);
    Bkey = get_config_int(cfg_sect,"key_b",KEY_ZC_LCONTROL);
    Skey = get_config_int(cfg_sect,"key_s",KEY_ENTER);
    Lkey = get_config_int(cfg_sect,"key_l",KEY_Z);
    Rkey = get_config_int(cfg_sect,"key_r",KEY_X);
    Pkey = get_config_int(cfg_sect,"key_p",KEY_SPACE);
    Exkey1 = get_config_int(cfg_sect,"key_ex1",KEY_Q);
    Exkey2 = get_config_int(cfg_sect,"key_ex2",KEY_W);
    Exkey3 = get_config_int(cfg_sect,"key_ex3",KEY_A);
    Exkey4 = get_config_int(cfg_sect,"key_ex4",KEY_S);
    
    DUkey = get_config_int(cfg_sect,"key_up",   KEY_UP);
    DDkey = get_config_int(cfg_sect,"key_down", KEY_DOWN);
    DLkey = get_config_int(cfg_sect,"key_left", KEY_LEFT);
    DRkey = get_config_int(cfg_sect,"key_right",KEY_RIGHT);
    
    Abtn = get_config_int(cfg_sect,"btn_a",2);
    Bbtn = get_config_int(cfg_sect,"btn_b",1);
    Sbtn = get_config_int(cfg_sect,"btn_s",10);
    Mbtn = get_config_int(cfg_sect,"btn_m",9);
    Lbtn = get_config_int(cfg_sect,"btn_l",5);
    Rbtn = get_config_int(cfg_sect,"btn_r",6);
    Pbtn = get_config_int(cfg_sect,"btn_p",12);
    Exbtn1 = get_config_int(cfg_sect,"btn_ex1",7);
    Exbtn2 = get_config_int(cfg_sect,"btn_ex2",8);
    Exbtn3 = get_config_int(cfg_sect,"btn_ex3",4);
    Exbtn4 = get_config_int(cfg_sect,"btn_ex4",3);
    
    DUbtn = get_config_int(cfg_sect,"btn_up",13);
    DDbtn = get_config_int(cfg_sect,"btn_down",14);
    DLbtn = get_config_int(cfg_sect,"btn_left",15);
    DRbtn = get_config_int(cfg_sect,"btn_right",16);
    
    digi_volume = get_config_int(cfg_sect,"digi",248);
    midi_volume = get_config_int(cfg_sect,"midi",255);
    sfx_volume = get_config_int(cfg_sect,"sfx",248);
    emusic_volume = get_config_int(cfg_sect,"emusic",248);
    pan_style = get_config_int(cfg_sect,"pan",1);
    // 1 <= zcmusic_bufsz <= 128
    zcmusic_bufsz = vbound(get_config_int(cfg_sect,"zcmusic_bufsz",64),1,128);
    volkeys = get_config_int(cfg_sect,"volkeys",0)!=0;
    zc_vsync = get_config_int(cfg_sect,"vsync",0);
    Throttlefps = get_config_int(cfg_sect,"throttlefps",1)!=0;
    TransLayers = get_config_int(cfg_sect,"translayers",1)!=0;
    SnapshotFormat = get_config_int(cfg_sect,"snapshot_format",3);
    NameEntryMode = get_config_int(cfg_sect,"name_entry_mode",0);
    ShowFPS = get_config_int(cfg_sect,"showfps",0)!=0;
    NESquit = get_config_int(cfg_sect,"fastquit",0)!=0;
    ClickToFreeze = get_config_int(cfg_sect,"clicktofreeze",1)!=0;
    title_version = get_config_int(cfg_sect,"title",2);
    
    //default - scale x2, 640 x 480
    resx = get_config_int(cfg_sect,"resx",640);
    resy = get_config_int(cfg_sect,"resy",480);
    //screen_scale = get_config_int(cfg_sect,"screen_scale",2);
    
    scanlines = get_config_int(cfg_sect,"scanlines",0)!=0;
    loadlast = get_config_int(cfg_sect,"load_last",0);
    
// Fullscreen, page flipping may be problematic on newer windows systems.
#ifdef _WIN32
    fullscreen = get_config_int(cfg_sect,"fullscreen",0);
    disable_triplebuffer = (byte) get_config_int(cfg_sect,"doublebuffer",1);
    can_triplebuffer_in_windowed_mode = (byte) get_config_int(cfg_sect,"triplebuffer",0);
#else
    fullscreen = get_config_int(cfg_sect,"fullscreen",1);
    disable_triplebuffer = (byte) get_config_int(cfg_sect,"doublebuffer",0);
    can_triplebuffer_in_windowed_mode = (byte) get_config_int(cfg_sect,"triplebuffer",0);
#endif
    
    zc_color_depth = (byte) get_config_int(cfg_sect,"color_depth",8);
    
    //workaround for the 100% CPU bug. -Gleeok
#ifdef ALLEGRO_MACOSX //IIRC rest(0) was a mac issue fix.
    frame_rest_suggest = (byte) get_config_int(cfg_sect,"frame_rest_suggest",0);
#else
    frame_rest_suggest = (byte) get_config_int(cfg_sect,"frame_rest_suggest",1);
#endif
    frame_rest_suggest = zc_min(2, frame_rest_suggest);
    
    forceExit = (byte) get_config_int(cfg_sect,"force_exit",0);
    
#ifdef _WIN32
    use_debug_console = (byte) get_config_int(cfg_sect,"debug_console",0);
    //use_win7_keyboard_fix = (byte) get_config_int(cfg_sect,"use_win7_key_fix",0);
    use_win32_proc = (byte) get_config_int(cfg_sect,"zc_win_proc_fix",0); //buggy
    
    // This seems to fix some problems on Windows 7
    disable_direct_updating = (byte) get_config_int("graphics","disable_direct_updating",1);
    
    // This one's for Aero
    use_dwm_flush = (byte) get_config_int("zeldadx","use_dwm_flush",0);
	
	// And this one fixes patches unloading on some MIDI setups
	midi_patch_fix = (byte) get_config_int("zeldadx","midi_patch_fix",0);
#endif
    
#ifdef ALLEGRO_MACOSX
    const char *default_path="../../../";
#else
    const char *default_path="";
#endif
    strcpy(qstdir,get_config_string(cfg_sect,qst_dir_name,default_path));
    
    if(strlen(qstdir)==0)
    {
        getcwd(qstdir,2048);
        fix_filename_case(qstdir);
        fix_filename_slashes(qstdir);
        put_backslash(qstdir);
    }
    else
    {
        chop_path(qstdir);
    }
    
    strcpy(qstpath,qstdir); //qstpath is the local (for this run of ZC) quest path, qstdir is the universal quest dir.
	ss_enable = get_config_int(cfg_sect,"ss_enable",1) ? 1 : 0;
    ss_after = vbound(get_config_int(cfg_sect,"ss_after",14), 0, 14);
    ss_speed = vbound(get_config_int(cfg_sect,"ss_speed",2), 0, 6);
    ss_density = vbound(get_config_int(cfg_sect,"ss_density",3), 0, 6);
    heart_beep = get_config_int(cfg_sect,"heart_beep",1)!=0;
    gui_colorset = get_config_int(cfg_sect,"gui_colorset",0);
    sfxdat = get_config_int(cfg_sect,"use_sfx_dat",1);
    fullscreen = get_config_int(cfg_sect,"fullscreen",1);
    use_save_indicator = get_config_int(cfg_sect,"save_indicator",0);
}

void save_game_configs()
{
	packfile_password("");

    set_config_int(cfg_sect,"joystick_index",joystick_index);
    set_config_int(cfg_sect,"js_stick_1_x_stick",js_stick_1_x_stick);
    set_config_int(cfg_sect,"js_stick_1_x_axis",js_stick_1_x_axis);
    set_config_int(cfg_sect,"js_stick_1_x_offset",js_stick_1_x_offset ? 1 : 0);
    set_config_int(cfg_sect,"js_stick_1_y_stick",js_stick_1_y_stick);
    set_config_int(cfg_sect,"js_stick_1_y_axis",js_stick_1_y_axis);
    set_config_int(cfg_sect,"js_stick_1_y_offset",js_stick_1_y_offset ? 1 : 0);
    set_config_int(cfg_sect,"js_stick_2_x_stick",js_stick_2_x_stick);
    set_config_int(cfg_sect,"js_stick_2_x_axis",js_stick_2_x_axis);
    set_config_int(cfg_sect,"js_stick_2_x_offset",js_stick_2_x_offset ? 1 : 0);
    set_config_int(cfg_sect,"js_stick_2_y_stick",js_stick_2_y_stick);
    set_config_int(cfg_sect,"js_stick_2_y_axis",js_stick_2_y_axis);
    set_config_int(cfg_sect,"js_stick_2_y_offset",js_stick_2_y_offset ? 1 : 0);
    set_config_int(cfg_sect,"analog_movement",analog_movement);
    
    set_config_int(cfg_sect,"key_a",Akey);
    set_config_int(cfg_sect,"key_b",Bkey);
    set_config_int(cfg_sect,"key_s",Skey);
    set_config_int(cfg_sect,"key_l",Lkey);
    set_config_int(cfg_sect,"key_r",Rkey);
    set_config_int(cfg_sect,"key_p",Pkey);
    set_config_int(cfg_sect,"key_ex1",Exkey1);
    set_config_int(cfg_sect,"key_ex2",Exkey2);
    set_config_int(cfg_sect,"key_ex3",Exkey3);
    set_config_int(cfg_sect,"key_ex4",Exkey4);
    
    set_config_int(cfg_sect,"key_up",   DUkey);
    set_config_int(cfg_sect,"key_down", DDkey);
    set_config_int(cfg_sect,"key_left", DLkey);
    set_config_int(cfg_sect,"key_right",DRkey);
    
    set_config_int(cfg_sect,"btn_a",Abtn);
    set_config_int(cfg_sect,"btn_b",Bbtn);
    set_config_int(cfg_sect,"btn_s",Sbtn);
    set_config_int(cfg_sect,"btn_m",Mbtn);
    set_config_int(cfg_sect,"btn_l",Lbtn);
    set_config_int(cfg_sect,"btn_r",Rbtn);
    set_config_int(cfg_sect,"btn_p",Pbtn);
    set_config_int(cfg_sect,"btn_ex1",Exbtn1);
    set_config_int(cfg_sect,"btn_ex2",Exbtn2);
    set_config_int(cfg_sect,"btn_ex3",Exbtn3);
    set_config_int(cfg_sect,"btn_ex4",Exbtn4);
    
    set_config_int(cfg_sect,"btn_up",DUbtn);
    set_config_int(cfg_sect,"btn_down",DDbtn);
    set_config_int(cfg_sect,"btn_left",DLbtn);
    set_config_int(cfg_sect,"btn_right",DRbtn);
    
    set_config_int(cfg_sect,"digi",digi_volume);
    set_config_int(cfg_sect,"midi",midi_volume);
    set_config_int(cfg_sect,"sfx",sfx_volume);
    set_config_int(cfg_sect,"emusic",emusic_volume);
    set_config_int(cfg_sect,"pan",pan_style);
    set_config_int(cfg_sect,"zcmusic_bufsz",zcmusic_bufsz);
    set_config_int(cfg_sect,"volkeys",(int)volkeys);
    set_config_int(cfg_sect,"vsync",zc_vsync);
    set_config_int(cfg_sect,"throttlefps", (int)Throttlefps);
    set_config_int(cfg_sect,"translayers",(int)TransLayers);
    set_config_int(cfg_sect,"snapshot_format",SnapshotFormat);
    set_config_int(cfg_sect,"name_entry_mode",NameEntryMode);
    set_config_int(cfg_sect,"showfps",(int)ShowFPS);
    set_config_int(cfg_sect,"fastquit",(int)NESquit);
    set_config_int(cfg_sect,"clicktofreeze", (int)ClickToFreeze);
    set_config_int(cfg_sect,"title",title_version);
    
    set_config_int(cfg_sect,"resx",resx);
    set_config_int(cfg_sect,"resy",resy);
    
    //sbig depricated as of 2.5 RC3. handled exclusively by resx, resy now.
    //set_config_int(cfg_sect,"screen_scale",screen_scale);
    //set_config_int(cfg_sect,"sbig",sbig);
    //set_config_int(cfg_sect,"sbig2",sbig2);
    
    set_config_int(cfg_sect,"scanlines",scanlines);
    set_config_int(cfg_sect,"load_last",loadlast);
    chop_path(qstdir);
    set_config_string(cfg_sect,qst_dir_name,qstdir);
    set_config_int(cfg_sect,"ss_enable",ss_enable);
    set_config_int(cfg_sect,"ss_after",ss_after);
    set_config_int(cfg_sect,"ss_speed",ss_speed);
    set_config_int(cfg_sect,"ss_density",ss_density);
    set_config_int(cfg_sect,"heart_beep",heart_beep);
    set_config_int(cfg_sect,"gui_colorset",gui_colorset);
    set_config_int(cfg_sect,"use_sfx_dat",sfxdat);
    set_config_int(cfg_sect,"fullscreen",fullscreen);
    set_config_int(cfg_sect,"doublebuffer",disable_triplebuffer);
    set_config_int(cfg_sect,"triplebuffer",can_triplebuffer_in_windowed_mode);
    set_config_int(cfg_sect,"color_depth",zc_color_depth);
    set_config_int(cfg_sect,"frame_rest_suggest",frame_rest_suggest);
    set_config_int(cfg_sect,"force_exit",forceExit);
    
#ifdef _WIN32
    set_config_int(cfg_sect,"debug_console",use_debug_console);
    //set_config_int(cfg_sect,"use_win7_key_fix",use_win7_keyboard_fix);
    set_config_int(cfg_sect,"zc_win_proc_fix",use_win32_proc);
    set_config_int("graphics","disable_direct_updating",disable_direct_updating);
    set_config_int("zeldadx","use_dwm_flush",use_dwm_flush);
	set_config_int("zeldadx","midi_patch_fix",midi_patch_fix);
#endif
    
#ifdef ALLEGRO_LINUX
    set_config_string("sound","patches",samplepath); // set to sample sound path set for DIGMIDI driver in Linux ~ Takuya
#endif
    
    set_config_int(cfg_sect,"save_indicator",use_save_indicator);
    
    flush_config_file();
}

//----------------------------------------------------------------

// Timers

void fps_callback()
{
    lastfps=framecnt;
    dword tempsecs = fps_secs;
    ++tempsecs;
    //avgfps=((long double)avgfps*fps_secs+lastfps)/(++fps_secs); // DJGPP doesn't like this
    avgfps=((long double)avgfps*fps_secs+lastfps)/(tempsecs);
    ++fps_secs;
    framecnt=0;
}

END_OF_FUNCTION(fps_callback)

int Z_init_timers()
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

void go()
{
    scare_mouse();
    blit(screen,tmp_scr,scrx,scry,0,0,screen->w,screen->h);
    unscare_mouse();
}

void comeback()
{
    scare_mouse();
    blit(tmp_scr,screen,0,0,scrx,scry,screen->w,screen->h);
    unscare_mouse();
}

void dump_pal(BITMAP *dest)
{
    for(int i=0; i<256; i++)
        rectfill(dest,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

void show_paused(BITMAP *target)
{
    //  return;
    char buf[7] = "PAUSED";
    
    for(int i=0; buf[i]!=0; i++)
        buf[i]+=0x60;
        
    //  text_mode(-1);
    if(sbig)
    {
        int x = scrx+40-((screen_scale-1)*120);
        int y = scry+224+((screen_scale-1)*104);
        textout_ex(target,zfont,buf,x,y,-1,-1);
    }
    else
        textout_ex(target,zfont,buf,scrx+40,scry+224,-1,-1);
}

void show_fps(BITMAP *target)
{
    char buf[50];
    
    //  text_mode(-1);
    sprintf(buf,"%2d/60",lastfps);
    
    //  sprintf(buf,"%d/%u/%f/%u",lastfps,int(avgfps),avgfps,fps_secs);
    for(int i=0; buf[i]!=0; i++)
        if(buf[i]!=' ')
            buf[i]+=0x60;
            
    if(sbig)
    {
        int x = scrx+40-((screen_scale-1)*120);
        int y = scry+216+((screen_scale-1)*104);
        textout_ex(target,zfont,buf,x,y,-1,-1);
        // textout_ex(target,zfont,buf,scrx+40-120,scry+216+104,-1,-1);
    }
    else
    {
        textout_ex(target,zfont,buf,scrx+40,scry+216,-1,-1);
    }
}

void show_saving(BITMAP *target)
{
    if(!use_save_indicator)
        return;
    
    char buf[10] = "SAVING...";
    
    for(int i=0; buf[i]!=0; i++)
        buf[i]+=0x60;
        
    if(sbig)
    {
        int x = scrx+200+((screen_scale-1)*120);
        int y = scry+224+((screen_scale-1)*104);
        textout_ex(target,zfont,buf,x,y,-1,-1);
    }
    else
        textout_ex(target,zfont,buf,scrx+200,scry+224,-1,-1);
}

//----------------------------------------------------------------

// sets the video mode and initializes the palette and mouse sprite
bool game_vid_mode(int mode,int wait)
{
    if(set_gfx_mode(mode,resx,resy,0,0)!=0)
    {
        return false;
    }
    
    scrx = (resx-320)>>1;
    scry = (resy-240)>>1;
    
    set_mouse_sprite((BITMAP*)data[BMP_MOUSE].dat);
    
    for(int i=240; i<256; i++)
        RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
        
    set_palette(RAMpal);
    clear_to_color(screen,BLACK);
    
    rest(wait);
    return true;
}

void init_NES_mode()
{
    // qst.dat may not load correctly without this...
    QHeader.templatepath[0]='\0';
    
    if(!init_colordata(true, &QHeader, &QMisc))
    {
        return;
    }
    
    loadfullpal();
    init_tiles(false, &QHeader);
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

word screen_triangles[28][32];
/*
  qword triangles[4][16]= //[direction][value]
  {
  {
  0x00000000, 0x10000000, 0x21000000, 0x32100000, 0x43210000, 0x54321000, 0x65432100, 0x76543210, 0x87654321, 0x88765432, 0x88876543, 0x88887654, 0x88888765, 0x88888876, 0x88888887, 0x88888888
  },
  {
  0x00000000, 0xF0000000, 0xEF000000, 0xFDF00000, 0xCFDF0000, 0xBCFDF000, 0xABCFDF00, 0x9ABCFDF0, 0x89ABCFDF, 0x889ABCFD, 0x8889ABCD, 0x88889ABC, 0x888889AB, 0x8888889A, 0x88888889, 0x88888888
  },
  {
  0x00000000, 0x00000001, 0x00000012, 0x00000123, 0x00001234, 0x00012345, 0x00123456, 0x01234567, 0x12345678, 0x23456788, 0x34567888, 0x45678888, 0x56788888, 0x67888888, 0x78888888, 0x88888888
  },
  {
  0x00000000, 0x0000000F, 0x000000FE, 0x00000FED, 0x0000FEDC, 0x000FEDCB, 0x00FEDCBA, 0x0FEDCBA9, 0xFEDCBA98, 0xEDCBA988, 0xDCBA9888, 0xCBA98888, 0xBA988888, 0xA9888888, 0x98888888, 0x88888888
  }
  };
  */


/*
  byte triangles[4][16][8]= //[direction][value][line]
  {
  {
  {
  0,  0,  0,  0,  0,  0,  0,  0
  },
  {
  1,  0,  0,  0,  0,  0,  0,  0
  },
  {
  2,  1,  0,  0,  0,  0,  0,  0
  },
  {
  3,  2,  1,  0,  0,  0,  0,  0
  },
  {
  4,  3,  2,  1,  0,  0,  0,  0
  },
  {
  5,  4,  3,  2,  1,  0,  0,  0
  },
  {
  6,  5,  4,  3,  2,  1,  0,  0
  },
  {
  7,  6,  5,  4,  3,  2,  1,  0
  },
  {
  8,  7,  6,  5,  4,  3,  2,  1
  },
  {
  8,  8,  7,  6,  5,  4,  3,  2
  },
  {
  8,  8,  8,  7,  6,  5,  4,  3
  },
  {
  8,  8,  8,  8,  7,  6,  5,  4
  },
  {
  8,  8,  8,  8,  8,  7,  6,  5
  },
  {
  8,  8,  8,  8,  8,  8,  7,  6
  },
  {
  8,  8,  8,  8,  8,  8,  8,  7
  },
  {
  8,  8,  8,  8,  8,  8,  8,  8
  }
  },
  {
  {
  0,  0,  0,  0,  0,  0,  0,  0
  },
  {
  15,  0,  0,  0,  0,  0,  0,  0
  },
  {
  14, 15,  0,  0,  0,  0,  0,  0
  },
  {
  13, 14, 15,  0,  0,  0,  0,  0
  },
  {
  12, 13, 14, 15,  0,  0,  0,  0
  },
  {
  11, 12, 13, 14, 15,  0,  0,  0
  },
  {
  10, 11, 12, 13, 14, 15,  0,  0
  },
  {
  9, 10, 11, 12, 13, 14, 15,  0
  },
  {
  8,  9, 10, 11, 12, 13, 14, 15
  },
  {
  8,  8,  9, 10, 11, 12, 13, 14
  },
  {
  8,  8,  8,  9, 10, 11, 12, 13
  },
  {
  8,  8,  8,  8,  9, 10, 11, 12
  },
  {
  8,  8,  8,  8,  8,  9, 10, 11
  },
  {
  8,  8,  8,  8,  8,  8,  9, 10
  },
  {
  8,  8,  8,  8,  8,  8,  8,  9
  },
  {
  8,  8,  8,  8,  8,  8,  8,  8
  }
  },
  {
  {
  0,  0,  0,  0,  0,  0,  0,  0
  },
  {
  0,  0,  0,  0,  0,  0,  0,  1
  },
  {
  0,  0,  0,  0,  0,  0,  1,  2
  },
  {
  0,  0,  0,  0,  0,  1,  2,  3
  },
  {
  0,  0,  0,  0,  1,  2,  3,  4
  },
  {
  0,  0,  0,  1,  2,  3,  4,  5
  },
  {
  0,  0,  1,  2,  3,  4,  5,  6
  },
  {
  0,  1,  2,  3,  4,  5,  6,  7
  },
  {
  1,  2,  3,  4,  5,  6,  7,  8
  },
  {
  2,  3,  4,  5,  6,  7,  8,  8
  },
  {
  3,  4,  5,  6,  7,  8,  8,  8
  },
  {
  4,  5,  6,  7,  8,  8,  8,  8
  },
  {
  5,  6,  7,  8,  8,  8,  8,  8
  },
  {
  6,  7,  8,  8,  8,  8,  8,  8
  },
  {
  7,  8,  8,  8,  8,  8,  8,  8
  },
  {
  8,  8,  8,  8,  8,  8,  8,  8
  }
  },
  {
  {
  0,  0,  0,  0,  0,  0,  0,  0
  },
  {
  0,  0,  0,  0,  0,  0,  0, 15
  },
  {
  0,  0,  0,  0,  0,  0, 15, 14
  },
  {
  0,  0,  0,  0,  0, 15, 14, 13
  },
  {
  0,  0,  0,  0, 15, 14, 13, 12
  },
  {
  0,  0,  0, 15, 14, 13, 12, 11
  },
  {
  0,  0, 15, 14, 13, 12, 11, 10
  },
  {
  0, 15, 14, 13, 12, 11, 10,  9
  },
  {
  15, 14, 13, 12, 11, 10,  9,  8
  },
  {
  14, 13, 12, 11, 10,  9,  8,  8
  },
  {
  13, 12, 11, 10,  9,  8,  8,  8
  },
  {
  12, 11, 10,  9,  8,  8,  8,  8
  },
  {
  11, 10,  9,  8,  8,  8,  8,  8
  },
  {
  10,  9,  8,  8,  8,  8,  8,  8
  },
  {
  9,  8,  8,  8,  8,  8,  8,  8
  },
  {
  8,  8,  8,  8,  8,  8,  8,  8
  }
  }
  };
  */



/*
  for (int blockrow=0; blockrow<30; ++i)
  {
  for (int linerow=0; linerow<8; ++i)
  {
  qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
  for (int blockcolumn=0; blockcolumn<40; ++i)
  {
  triangleline=triangles[0][screen_triangles[blockrow][blockcolumn]][linerow];
  ++triangleline;
  }
  }
  }
  */

// the ULL suffixes are to prevent this warning:
// warning: integer constant is too large for "long" type

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

int black_opening_count=0;
int black_opening_x,black_opening_y;
int black_opening_shape;

int choose_opening_shape()
{
    // First, count how many bits are set
    int numBits=0;
    int bitCounter;
    
    for(int i=0; i<4; i++)
    {
        if(COOLSCROLL&(1<<i))
            numBits++;
    }
    
    // Shouldn't happen...
    if(numBits==0)
        return bosCIRCLE;
        
    // Pick a bit
    bitCounter=rand()%numBits+1;
    
    for(int i=0; i<4; i++)
    {
        // If this bit is set, decrement the bit counter
        if(COOLSCROLL&(1<<i))
            bitCounter--;
            
        // When the counter hits 0, return a value based on
        // which bit it stopped on.
        // Reminder: enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosMAX};
        if(bitCounter==0)
            return i;
    }
    
    // Shouldn't be necessary, but the compiler might complain, at least
    return bosCIRCLE;
}

void close_black_opening(int x, int y, bool wait)
{
    black_opening_shape=choose_opening_shape();
    
    int w=256, h=224;
    int blockrows=28, blockcolumns=32;
    int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
    
    for(int blockrow=0; blockrow<blockrows; ++blockrow)  //30
    {
        for(int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
        {
            screen_triangles[blockrow][blockcolumn]=zc_max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
        }
    }
    
    black_opening_count = 66;
    black_opening_x = x;
    black_opening_y = y;
    lensclk = 0;
    //black_opening_shape=(black_opening_shape+1)%bosMAX;
    
    if(wait)
    {
        for(int i=0; i<66; i++)
        {
            draw_screen(tmpscr);
            put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
            syskeys();
            advanceframe(true);
            
            if(Quit)
            {
                break;
            }
        }
    }
}

void open_black_opening(int x, int y, bool wait)
{
    black_opening_shape=choose_opening_shape();
    
    int w=256, h=224;
    int blockrows=28, blockcolumns=32;
    int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
    
    for(int blockrow=0; blockrow<blockrows; ++blockrow)  //30
    {
        for(int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
        {
            screen_triangles[blockrow][blockcolumn]=zc_max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
        }
    }
    
    black_opening_count = -66;
    black_opening_x = x;
    black_opening_y = y;
    lensclk = 0;
    
    if(wait)
    {
        for(int i=0; i<66; i++)
        {
            draw_screen(tmpscr);
            put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
            syskeys();
            advanceframe(true);
            
            if(Quit)
            {
                break;
            }
        }
    }
}

void black_opening(BITMAP *dest,int x,int y,int a,int max_a)
{
    clear_to_color(tmp_scr,BLACK);
    int w=256, h=224;
    
    switch(black_opening_shape)
    {
    case bosOVAL:
    {
        double new_w=(w/2)+abs(w/2-x);
        double new_h=(h/2)+abs(h/2-y);
        double b=sqrt(((new_w*new_w)/4)+(new_h*new_h));
        ellipsefill(tmp_scr,x,y,int(2*a*b/max_a)/8*8,int(a*b/max_a)/8*8,0);
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
        triangle(tmp_scr, x+int(cos(a0)*r), y-int(sin(a0)*r),
                 x+int(cos(a2)*r), y-int(sin(a2)*r),
                 x+int(cos(a4)*r), y-int(sin(a4)*r),
                 0);
        break;
    }
    
    case bosSMAS:
    {
        int distance=zc_max(abs(w/2-x),abs(h/2-y))/8;
        
        for(int blockrow=0; blockrow<28; ++blockrow)  //30
        {
            for(int linerow=0; linerow<8; ++linerow)
            {
                qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
                
                for(int blockcolumn=0; blockcolumn<32; ++blockcolumn)  //40
                {
                    *triangleline=triangles[(screen_triangles[blockrow][blockcolumn]&0xC000)>>14]
                                  [zc_min(zc_max((((31+distance)*(max_a-a)/max_a)+((screen_triangles[blockrow][blockcolumn]&0x0FFF)-0x0100)-(15+distance)),0),15)]
                                  [linerow];
                    ++triangleline;
                    
                    if(linerow==0)
                    {
                    }
                }
            }
        }
        
        break;
    }
    
    case bosCIRCLE:
    default:
    {
        double new_w=(w/2)+abs(w/2-x);
        double new_h=(h/2)+abs(h/2-y);
        int r=int(sqrt((new_w*new_w)+(new_h*new_h))*a/max_a);
        //circlefill(tmp_scr,x,y,a<<3,0);
        circlefill(tmp_scr,x,y,r,0);
        break;
    }
    }
    
    masked_blit(tmp_scr,dest,0,0,0,0,320,240);
}

//----------------------------------------------------------------

bool item_disabled(int item)                 //is this item disabled?
{
    return (item>=0 && game->items_off[item] != 0);
}

bool can_use_item(int item_type, int item)                  //can Link use this item?
{
    if(current_item(item_type, true) >=item)
    {
        return true;
    }
    
    return false;
}

bool has_item(int item_type, int it)                        //does Link possess this item?
{
    switch(item_type)
    {
    case itype_bomb:
    case itype_sbomb:
    {
        int itemid = getItemID(itemsbuf, item_type, it);
        
        if(itemid == -1)
            return false;
            
        return (game->get_item(itemid));
    }
    
    case itype_clock:
        return Link.getClock()?1:0;
        
    case itype_key:
        return (game->get_keys()>0);
        
    case itype_magiccontainer:
        return (game->get_maxmagic()>=MAGICPERBLOCK);
        
    case itype_triforcepiece:                               //it: -2=any, -1=current level, other=that level
    {
        switch(it)
        {
        case -2:
        {
            for(int i=0; i<MAXLEVELS; i++)
            {
                if(game->lvlitems[i]&liTRIFORCE)
                {
                    return true;
                }
            }
            
            return false;
            break;
        }
        
        case -1:
            return (game->lvlitems[dlevel]&liTRIFORCE);
            break;
            
        default:
            if(it>=0&&it<MAXLEVELS)
            {
                return (game->lvlitems[it]&liTRIFORCE);
            }
            
            break;
        }
        
        return 0;
    }
    
    case itype_map:                                         //it: -2=any, -1=current level, other=that level
    {
        switch(it)
        {
        case -2:
        {
            for(int i=0; i<MAXLEVELS; i++)
            {
                if(game->lvlitems[i]&liMAP)
                {
                    return true;
                }
            }
            
            return false;
        }
        break;
        
        case -1:
            return (game->lvlitems[dlevel]&liMAP)!=0;
            break;
            
        default:
            if(it>=0&&it<MAXLEVELS)
            {
                return (game->lvlitems[it]&liMAP)!=0;
            }
            
            break;
        }
        
        return 0;
    }
    
    case itype_compass:                                     //it: -2=any, -1=current level, other=that level
    {
        switch(it)
        {
        case -2:
        {
            for(int i=0; i<MAXLEVELS; i++)
            {
                if(game->lvlitems[i]&liCOMPASS)
                {
                    return true;
                }
            }
            
            return false;
            break;
        }
        
        case -1:
            return (game->lvlitems[dlevel]&liCOMPASS)!=0;
            break;
            
        default:
            if(it>=0&&it<MAXLEVELS)
            {
                return (game->lvlitems[it]&liCOMPASS)!=0;
            }
            
            break;
        }
        
        return 0;
    }
    
    case itype_bosskey:                                     //it: -2=any, -1=current level, other=that level
    {
        switch(it)
        {
        case -2:
        {
            for(int i=0; i<MAXLEVELS; i++)
            {
                if(game->lvlitems[i]&liBOSSKEY)
                {
                    return true;
                }
            }
            
            return false;
            break;
        }
        
        case -1:
            return (game->lvlitems[dlevel]&liBOSSKEY)?1:0;
            break;
            
        default:
            if(it>=0&&it<MAXLEVELS)
            {
                return (game->lvlitems[it]&liBOSSKEY)?1:0;
            }
            
            break;
        }
        
        return 0;
    }
    
    default:
        //it=(1<<(it-1));
        /*if (item_type>=itype_max)
        {
          system_pal();
          jwin_alert("Error","has_item exception",NULL,NULL,"O&K",NULL,'k',0,lfont);
          game_pal();
        
          return false;
        }*/
        int itemid = getItemID(itemsbuf, item_type, it);
        
        if(itemid == -1)
            return false;
            
        return game->get_item(itemid);
        break;
    }
}


int current_item(int item_type, bool checkenabled)           //item currently being used
{
    switch(item_type)
    {
    case itype_clock:
        return has_item(itype_clock,1) ? 1 : 0;
        break;
        
    case itype_key:
        return game->get_keys();
        
    case itype_lkey:
        return game->lvlkeys[get_dlevel()];
        
    case itype_magiccontainer:
        return game->get_maxmagic()/MAGICPERBLOCK;
        
    case itype_triforcepiece:
    {
        int count=0;
        
        for(int i=0; i<MAXLEVELS; i++)
        {
            count+=(game->lvlitems[i]&liTRIFORCE)?1:0;
        }
        
        return count;
        break;
    }
    
    case itype_map:
    {
        int count=0;
        
        for(int i=0; i<MAXLEVELS; i++)
        {
            count+=(game->lvlitems[i]&liMAP)?1:0;
        }
        
        return count;
        break;
    }
    
    case itype_compass:
    {
        int count=0;
        
        for(int i=0; i<MAXLEVELS; i++)
        {
            count+=(game->lvlitems[i]&liCOMPASS)?1:0;
        }
        
        return count;
        break;
    }
    
    case itype_bosskey:
    {
        int count=0;
        
        for(int i=0; i<MAXLEVELS; i++)
        {
            count+=(game->lvlitems[i]&liBOSSKEY)?1:0;
        }
        
        return count;
        break;
    }
    
    default:
        int maxid = getHighestLevelOfFamily(game, itemsbuf, item_type, checkenabled);
        
        if(maxid == -1)
            return 0;
            
        return itemsbuf[maxid].fam_type;
        break;
    }
}

int current_item(int item_type)           //item currently being used
{
    return current_item(item_type, true);
}

std::map<int, int> itemcache;

// Not actually used by anything at the moment...
void removeFromItemCache(int itemid)
{
    itemcache.erase(itemid);
}

void flushItemCache()
{
    itemcache.clear();
    
    //also fix the active subscreen if items were deleted -DD
    if(game != NULL)
    {
        verifyBothWeapons();
        load_Sitems(&QMisc);
    }
}

// This is used often, so it should be as direct as possible.
int current_item_id(int itemtype, bool checkmagic)
{
    if(itemtype!=itype_ring)  // Rings must always be checked.
    {
        std::map<int,int>::iterator res = itemcache.find(itemtype);
        
        if(res != itemcache.end())
            return res->second;
    }
    
    int result = -1;
    int highestlevel = -1;
    
    for(int i=0; i<MAXITEMS; i++)
    {
        if(game->get_item(i) && itemsbuf[i].family==itemtype && !item_disabled(i))
        {
            if((checkmagic || itemtype == itype_ring) && itemtype != itype_magicring)
            {
                //printf("Checkmagic for %d: %d (%d %d)\n",i,checkmagiccost(i),itemsbuf[i].magic*game->get_magicdrainrate(),game->get_magic());
                if(!checkmagiccost(i))
                {
                    continue;
                }
            }
            
            if(itemsbuf[i].fam_type >= highestlevel)
            {
                highestlevel = itemsbuf[i].fam_type;
                result=i;
            }
        }
    }
    
    itemcache[itemtype] = result;
    return result;
}

int current_item_power(int itemtype)
{
    int result = current_item_id(itemtype,true);
    return (result<0) ? 0 : itemsbuf[result].power;
}

int item_tile_mod(bool)
{
    long tile=0;
    int ret=0;
    
    ret=game->get_bombs();
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        if(current_item_id(itype_bomb,false)>=0)
            ret=itemsbuf[current_item_id(itype_bomb,false)].ltm;
        else
            ret=0;
            
        break;
        
    }
    
    tile+=ret;
    
    ret=game->get_sbombs();
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        if(current_item_id(itype_sbomb,false)>=0)
            ret=itemsbuf[current_item_id(itype_sbomb,false)].ltm;
        else
            ret=0;
            
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_clock);
    
    switch(ret)
    {
    case 1:
        ret=itemsbuf[iClock].ltm;
        break;
        
    default:
        ret=0;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_key);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iKey].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_lkey);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iLevelKey].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_map);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iMap].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_compass);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iCompass].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_bosskey);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iBossKey].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_magiccontainer);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iMagicC].ltm;
        break;
    }
    
    tile+=ret;
    
    ret=current_item(itype_triforcepiece);
    
    switch(ret)
    {
    case 0:
        ret=0;
        break;
        
    default:
        ret=itemsbuf[iTriforce].ltm;
        break;
    }
    
    tile+=ret;
    
    for(int i=0; i<itype_max; i++)
    {
        ret=current_item_id(i,false);
        
        if(ret >= 0)
            tile+=itemsbuf[ret].ltm;
    }
    
    return tile;
}

int dmap_tile_mod()
{
    return 0;
}

// Hints are drawn on a separate layer to combo reveals.
void draw_lens_under(BITMAP *dest, bool layer)
{
	//Lens flag 1: Replacement for qr_LENSHINTS; if set, lens will show hints. Does nothing if flag 2 is set.
	//Lens flag 2: Disable "hints", prevent rendering of Secret Combos
	//Lens flag 3: Don't show armos/chest/dive items
	//Lens flag 4: Show Raft Paths
	//Lens flag 5: Show Invisible Enemies
	bool hints = (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2) ? false : (layer && (get_bit(quest_rules,qr_LENSHINTS) || itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG1));
	
    int strike_hint_table[11]=
    {
        mfARROW, mfBOMB, mfBRANG, mfWANDMAGIC,
        mfSWORD, mfREFMAGIC, mfHOOKSHOT,
        mfREFFIREBALL, mfHAMMER, mfSWORDBEAM, mfWAND
    };
    
    //  int page = tmpscr->cpage;
    {
        int blink_rate=1;
        //    int temptimer=0;
        int tempitem, tempweapon=0;
        strike_hint=strike_hint_table[strike_hint_counter];
        
        if(strike_hint_timer>32)
        {
            strike_hint_timer=0;
            strike_hint_counter=((strike_hint_counter+1)%11);
        }
        
        ++strike_hint_timer;
        
        for(int i=0; i<176; i++)
        {
            int x = (i & 15) << 4;
            int y = (i & 0xF0) + playing_field_offset;
            int tempitemx=-16, tempitemy=-16;
            int tempweaponx=-16, tempweapony=-16;
            
            for(int iter=0; iter<2; ++iter)
            {
                int checkflag=0;
                
                if(iter==0)
                {
                    checkflag=combobuf[tmpscr->data[i]].flag;
                }
                else
                {
                    checkflag=tmpscr->sflag[i];
                }
                
                if(checkflag==mfSTRIKE)
                {
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSTRIKE],tmpscr->secretcset[sSTRIKE]);
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
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:
		case 120:
		case 121:
		case 122:
		case 123:
		case 124:
		case 125:
		case 126:
		case 127:
		case 128:
		case 129:
		case 130:
		case 131:
		case 132:
		case 133:
		case 134:
		case 135:
		case 136:
		case 137:
		case 138:
		case 139:
		case 140:
		case 141:
		case 142:
		case 143:
		case 144:
		case 145:
		case 146:
		case 147:
		case 148:
		case 149:
		case 150:
		case 151:
		case 152:
		case 153:
		case 154:
		case 155:
		case 156:
		case 157:
		case 158:
		case 159:
		case 160:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 166:
		case 167:
		case 168:
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:
		case 174:
		case 175:
		case 176:
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:
		case 182:
		case 183:
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:
		case 190:
		case 191:
		case 192:
		case 193:
		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
		case 215:
		case 216:
		case 217:
		case 218:
		case 219:
		case 220:
		case 221:
		case 222:
		case 223:
		case 224:
		case 225:
		case 226:
		case 227:
		case 228:
		case 229:
		case 230:
		case 231:
		case 232:
		case 233:
		case 234:
		case 235:
		case 236:
		case 237:
		case 238:
		case 239:
		case 240:
		case 241:
		case 242:
		case 243:
		case 244:
		case 245:
		case 246:
		case 247:
		case 248:
		case 249:
		case 250:
		case 251:
		case 252:
		case 253:
		case 254:
		case 255:
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
                    if(!hints && ((!(get_debug() && key[KEY_N]) && (lensclk&16))
                                  || ((get_debug() && key[KEY_N]) && (frame&16))))
                    {
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->undercombo,tmpscr->undercset);
                    }
                    
                    if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                            || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
                    {
                        if(hints)
                        {
                            switch(combobuf[tmpscr->data[i]].type)
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
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
                        {
                            tempitemx=x;
                            tempitemy=y;
                        }
                        
                        putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                    }
                    
                    break;
                    
                case mfBCANDLE:
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sBCANDLE],tmpscr->secretcset[sBCANDLE]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_candle,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
                        {
                            tempitemx=x;
                            tempitemy=y;
                        }
                        
                        putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                    }
                    
                    break;
                    
                case mfRCANDLE:
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sRCANDLE],tmpscr->secretcset[sRCANDLE]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_candle,2);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
                        {
                            tempitemx=x;
                            tempitemy=y;
                        }
                        
                        putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                    }
                    
                    break;
                    
                case mfWANDFIRE:
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sWANDFIRE],tmpscr->secretcset[sWANDFIRE]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_wand,1);
                        
                        if(tempitem<0) break;
                        
                        tempweapon=wFire;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                    
                case mfDINSFIRE:
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sDINSFIRE],tmpscr->secretcset[sDINSFIRE]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_dinsfire,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sARROW],tmpscr->secretcset[sARROW]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_arrow,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSARROW],tmpscr->secretcset[sSARROW]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_arrow,2);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sGARROW],tmpscr->secretcset[sGARROW]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_arrow,3);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sBOMB],tmpscr->secretcset[sBOMB]);
                    }
					else
                    {
                        //tempitem=getItemID(itemsbuf,itype_bomb,1);
                        tempweapon = wLitBomb;
                        
                        //if (tempitem<0) break;
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSBOMB],tmpscr->secretcset[sSBOMB]);
                    }
					else
                    {
                        //tempitem=getItemID(itemsbuf,itype_sbomb,1);
                        //if (tempitem<0) break;
                        tempweapon = wLitSBomb;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSTAIRS],tmpscr->secretcset[sSTAIRS]);
                    }    
                    break;
                    
                case mfBRANG:
                    if(!hints)
					{
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sBRANG],tmpscr->secretcset[sBRANG]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_brang,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sMBRANG],tmpscr->secretcset[sMBRANG]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_brang,2);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sFBRANG],tmpscr->secretcset[sFBRANG]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_brang,3);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sWANDMAGIC],tmpscr->secretcset[sWANDMAGIC]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_wand,1);
                        
                        if(tempitem<0) break;
                        
                        tempweapon=itemsbuf[tempitem].wpn3;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sREFMAGIC],tmpscr->secretcset[sREFMAGIC]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_shield,3);
                        
                        if(tempitem<0) break;
                        
                        tempweapon=ewMagic;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sREFFIREBALL],tmpscr->secretcset[sREFFIREBALL]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_shield,3);
                        
                        if(tempitem<0) break;
                        
                        tempweapon=ewFireball;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSWORD],tmpscr->secretcset[sSWORD]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sWSWORD],tmpscr->secretcset[sWSWORD]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,2);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sMSWORD],tmpscr->secretcset[sMSWORD]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,3);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sXSWORD],tmpscr->secretcset[sXSWORD]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,4);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sSWORDBEAM],tmpscr->secretcset[sSWORDBEAM]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sWSWORDBEAM],tmpscr->secretcset[sWSWORDBEAM]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,2);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sMSWORDBEAM],tmpscr->secretcset[sMSWORDBEAM]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,3);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sXSWORDBEAM],tmpscr->secretcset[sXSWORDBEAM]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_sword,4);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
						if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sHOOKSHOT],tmpscr->secretcset[sHOOKSHOT]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_hookshot,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sWAND],tmpscr->secretcset[sWAND]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_wand,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
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
			if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[sHAMMER],tmpscr->secretcset[sHAMMER]);
                    }
					else
                    {
                        tempitem=getItemID(itemsbuf,itype_hammer,1);
                        
                        if(tempitem<0) break;
                        
                        if((!(get_debug() && key[KEY_N]) && (lensclk&blink_rate))
                                || ((get_debug() && key[KEY_N]) && (frame&blink_rate)))
                        {
                            tempitemx=x;
                            tempitemy=y;
                        }
                        
                        putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                    }
                    
                    break;
                    
                case mfARMOS_ITEM:
                case mfDIVE_ITEM:
                    if(!getmapflag() && !(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG3))
		    {
                        putitem2(dest,x,y,tmpscr->catchall, lens_hint_item[tmpscr->catchall][0], lens_hint_item[tmpscr->catchall][1], 0);
		    }
                    break;
                    
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
                        if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,x,y,tmpscr->secretcombo[checkflag-16+4],
                                 tmpscr->secretcset[checkflag-16+4]);
                                 
                    break;
				
				case mfSTRIKE:
					if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))
					{
						goto special;
					}
					else
					{
						break;
					}
                    
                default: goto special;
				
				special:
                    if(layer && ((checkflag!=mfRAFT && checkflag!=mfRAFT_BRANCH&& checkflag!=mfRAFT_BOUNCE) || get_bit(quest_rules,qr_RAFTLENS) || (itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG4)))
                    {
                        if((!(get_debug() && key[KEY_N]) && (lensclk&1)) || ((get_debug() && key[KEY_N]) && (frame&1)))
                        {
                            rectfill(dest,x,y,x+15,y+15,WHITE);
                        }
                    }
                    
                    break;
                }
            }
        }
        
        if(layer)
        {
            if(tmpscr->door[0]==dWALK)
                rectfill(dest, 120, 16+playing_field_offset, 135, 31+playing_field_offset, WHITE);
                
            if(tmpscr->door[1]==dWALK)
                rectfill(dest, 120, 144+playing_field_offset, 135, 159+playing_field_offset, WHITE);
                
            if(tmpscr->door[2]==dWALK)
                rectfill(dest, 16, 80+playing_field_offset, 31, 95+playing_field_offset, WHITE);
                
            if(tmpscr->door[3]==dWALK)
                rectfill(dest, 224, 80+playing_field_offset, 239, 95+playing_field_offset, WHITE);
                
            if(tmpscr->door[0]==dBOMB)
            {
                showbombeddoor(dest, 0);
            }
            
            if(tmpscr->door[1]==dBOMB)
            {
                showbombeddoor(dest, 1);
            }
            
            if(tmpscr->door[2]==dBOMB)
            {
                showbombeddoor(dest, 2);
            }
            
            if(tmpscr->door[3]==dBOMB)
            {
                showbombeddoor(dest, 3);
            }
        }
        
        if(tmpscr->stairx + tmpscr->stairy)
        {
            if(!hints)
                if(!(itemsbuf[Link.getLastLensID()].flags & ITEM_FLAG2))putcombo(dest,tmpscr->stairx,tmpscr->stairy+playing_field_offset,tmpscr->secretcombo[sSTAIRS],tmpscr->secretcset[sSTAIRS]);
            else
            {
                if(tmpscr->flags&fWHISTLE)
                {
                    tempitem=getItemID(itemsbuf,itype_whistle,1);
                    int tempitemx=-16;
                    int tempitemy=-16;
                    
                    if((!(get_debug() && key[KEY_N]) && (lensclk&(blink_rate/4)))
                            || ((get_debug() && key[KEY_N]) && (frame&(blink_rate/4))))
                    {
                        tempitemx=tmpscr->stairx;
                        tempitemy=tmpscr->stairy+playing_field_offset;
                    }
                    
                    putitem2(dest,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                }
            }
        }
    }
}

BITMAP *lens_scr_d; // The "d" is for "destructible"!

void draw_lens_over()
{
    // Oh, what the heck.
    static BITMAP *lens_scr = NULL;
    static int last_width = -1;
    int width = itemsbuf[current_item_id(itype_lens,true)].misc1;
    
    // Only redraw the circle if the size has changed
    if(width != last_width)
    {
        if(lens_scr == NULL)
        {
            lens_scr = create_bitmap_ex(8,2*288,2*(240-playing_field_offset));
        }
        
        clear_to_color(lens_scr, BLACK);
        circlefill(lens_scr, 288, 240-playing_field_offset, width, 0);
        circle(lens_scr, 288, 240-playing_field_offset, width+2, 0);
        circle(lens_scr, 288, 240-playing_field_offset, width+5, 0);
        last_width=width;
    }
    
    masked_blit(lens_scr, framebuf, 288-(LinkX()+8), 240-playing_field_offset-(LinkY()+8), 0, playing_field_offset, 256, 168);
}

//----------------------------------------------------------------

void draw_wavy(BITMAP *source, BITMAP *target, int amplitude, bool interpol)
{
    //recreating a big bitmap every frame is highly sluggish.
    static BITMAP *wavebuf = create_bitmap_ex(8,288,240-original_playing_field_offset);
    
    clear_to_color(wavebuf, BLACK);
    blit(source,wavebuf,0,original_playing_field_offset,16,0,256,224-original_playing_field_offset);
    
    int ofs;
    //  int amplitude=8;
    //  int wavelength=4;
    amplitude = zc_min(2048,amplitude); // some arbitrary limit to prevent crashing
    int amp2=168;
    int i=frame%amp2;
    
    for(int j=0; j<168; j++)
    {
        if(j&1 && interpol)
        {
            // Add 288*2048 to ensure it's never negative. It'll get modded out.
            ofs=288*2048+int(sin((double(i+j)*2*PI/amp2))*amplitude);
        }
        else
        {
            ofs=288*2048-int(sin((double(i+j)*2*PI/amp2))*amplitude);
        }
        
        if(ofs)
        {
            for(int k=0; k<256; k++)
            {
                target->line[j+original_playing_field_offset][k]=wavebuf->line[j][(k+ofs+16)%288];
            }
        }
    }
}

void draw_fuzzy(int fuzz)
// draws from right half of scrollbuf to framebuf
{
    int firstx, firsty, xstep, ystep, i, y, dx, dy;
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
    
    for(y=0; y<224;)
    {
        start = &(scrollbuf->line[y][256]);
        
        for(dy=0; dy<ystep && dy+y<224; dy++)
        {
            si = start;
            di = &(framebuf->line[y+dy][0]);
            i = xstep;
            firstx = 1;
            
            for(dx=0; dx<256; dx++)
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
    static BITMAP *wavybuf = create_bitmap_ex(8,256,224);
    static BITMAP *panorama = create_bitmap_ex(8,256,224);
        
    if(toogam)
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
    
    if(refreshpal)
    {
        refreshpal=false;
        RAMpal[253] = _RGB(0,0,0);
        RAMpal[254] = _RGB(63,63,63);
        set_palette_range(RAMpal,0,255,false);
        
        create_rgb_table(&rgb_table, RAMpal, NULL);
        create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
        memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
        
        for(int q=0; q<PAL_SIZE; q++)
        {
            trans_table2.data[0][q] = q;
            trans_table2.data[q][q] = q;
        }
    }
    
    if(details)
        show_details();
    if(show_ff_scripts)
        show_ffscript_names();
    
    bool clearwavy = (wavy <= 0);
    
    if(wavy <= 0)
    {
        // So far one thing can alter wavy apart from scripts: Wavy DMaps.
        wavy = (DMaps[currdmap].flags&dmfWAVY ? 4 : 0);
    }
    
    blit(framebuf, wavybuf, 0, 0, 0, 0, 256, 224);
    
    if(wavy && Playing && allowwavy)
    {
        draw_wavy(framebuf, wavybuf, wavy,false);
    }
    
    if(clearwavy)
        wavy = 0; // Wavy was set by a DMap flag. Clear it.
    else if(Playing && !Paused)
        wavy--; // Wavy was set by a script. Decrement it.
        
    if(!(msgdisplaybuf->clip) && Playing && msgpos && !screenscrolling)
    {
        masked_blit(msgdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
    }
    
    /*
    if(!(msgdisplaybuf->clip) && Playing && msgpos && !screenscrolling)
    {
	    BITMAP* subBmp = 0;
	    masked_blit(msgdisplaybuf,subBmp,0,0,0,playing_field_offset,256,168);
	    // masked_blit(msgdisplaybuf,subBmp,0,playing_field_offset,256,168);
	     draw_trans_sprite(framebuf, subBmp, 0, playing_field_offset);
	    destroy_bitmap(subBmp);
	    //void draw_sprite_ex(BITMAP *bmp, BITMAP *sprite, int x, int y, int mode, int flip);
       // masked_blit(msgdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
	    //void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
    }
    */
    
    bool nosubscr = (tmpscr->flags3&fNOSUBSCR && !(tmpscr->flags3&fNOSUBSCROFFSET));
    
    if(nosubscr)
    {
        rectfill(panorama,0,0,255,passive_subscreen_height/2,0);
        rectfill(panorama,0,168+passive_subscreen_height/2,255,168+passive_subscreen_height-1,0);
        blit(wavybuf,panorama,0,playing_field_offset,0,passive_subscreen_height/2,256,224-passive_subscreen_height);
    }
    
    //TODO: Optimize blit 'overcalls' -Gleeok
    BITMAP *source = nosubscr ? panorama : wavybuf;
    BITMAP *target = NULL;
    
    bool dontusetb = triplebuffer_not_available ||
                     !(Throttlefps ^ (true && key[KEY_TILDE]));
                     
    if(dontusetb)
        target=screen;
    else
        target=tb_page[curr_tb_page];
        
//  static BITMAP *tempscreen=NULL;
    static BITMAP *scanlinesbmp=NULL;
    
    if(resx != SCREEN_W || resy != SCREEN_H)
    {
        Z_message("Conflicting variables warning: screen_scale %i, resx %i, resy %i, w %i, h %i\n", screen_scale, resx, resy, SCREEN_W, SCREEN_H);
        resx = SCREEN_W;
        resy = SCREEN_H;
        screen_scale = zc_max(zc_min(resx / 320, resy / 240), 1);
    }
    
    if(!sbig && screen_scale > 1)
        sbig = true;
        
    const int sx = 256 * screen_scale;
    const int sy = 224 * screen_scale;
    const int scale_mul = screen_scale - 1;
    const int mx = scale_mul * 128;
    const int my = scale_mul * 112;
    
    if(sbig)
    {
        if(scanlines)
        {
            if(!scanlinesbmp)
                scanlinesbmp = create_bitmap_ex(8, sx, sy);
                
            stretch_blit(source, scanlinesbmp, 0, 0, 256, 224, 0, 0, sx, sy);
            
            for(int i=0; i<224; ++i)
                _allegro_hline(scanlinesbmp, 0, (i*screen_scale)+1, sx, BLACK);
                
            blit(scanlinesbmp, target, 0, 0, scrx+32-mx, scry+8-my, sx, sy);
        }
        else
        {
            stretch_blit(source, target, 0, 0, 256, 224, scrx+32-mx, scry+8-my, sx, sy);
        }
        
        if(quakeclk>0)
            rectfill(target, // I don't know if these are right...
                     scrx+32 - mx, //x1
                     scry+8 - my + sy, //y1
                     scrx+32 - mx + sx, //x2
                     scry+8 - my + sy + (16 * scale_mul), //y2
                     BLACK);
                     
        //stretch_blit(nosubscr?panorama:wavybuf,target,0,0,256,224,scrx+32-128,scry+8-112,512,448);
        //if(quakeclk>0) rectfill(target,scrx+32-128,scry+8-112+448,scrx+32-128+512,scry+8-112+456,0);
    }
    else
    {
        blit(source,target,0,0,scrx+32,scry+8,256,224);
        
        if(quakeclk>0) rectfill(target,scrx+32,scry+8+224,scrx+32+256,scry+8+232,BLACK);
    }
    
    if(ShowFPS)
        show_fps(target);
        
    if(Paused)
        show_paused(target);
        
    if(details)
    {
        textprintf_ex(target,font,0,SCREEN_H-8,254,BLACK,"%-6d (%s)", idle_count, time_str_long(idle_count));
    }
    
    if(!dontusetb)
    {
        if(!poll_scroll())
        {
            request_video_bitmap(tb_page[curr_tb_page]);
            curr_tb_page=(curr_tb_page+1)%3;
            clear_to_color(tb_page[curr_tb_page],BLACK);
        }
    }
    
    //if(panorama!=NULL) destroy_bitmap(panorama);
    
    ++framecnt;
}

//----------------------------------------------------------------

PALETTE sys_pal;

int onGUISnapshot()
{
    char buf[20];
    int num=0;
    
    do
    {
#ifdef ALLEGRO_MACOSX
        sprintf(buf, "../../../zelda%03d.%s", ++num, snapshotformat_str[SnapshotFormat][1]);
#else
        sprintf(buf, "zelda%03d.%s", ++num, snapshotformat_str[SnapshotFormat][1]);
#endif
    }
    while(num<999 && exists(buf));
    
    BITMAP *b = create_bitmap_ex(8,resx,resy);
    
    if(b)
    {
        blit(screen,b,0,0,0,0,resx,resy);
        save_bmp(buf,b,sys_pal);
        destroy_bitmap(b);
    }
    
    return D_O_K;
}

int onNonGUISnapshot()
{
    PALETTE temppal;
    get_palette(temppal);
    bool realpal=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
    
    char buf[20];
    int num=0;
    
    do
    {
        sprintf(buf, "zelda%03d.%s", ++num, snapshotformat_str[SnapshotFormat][1]);
    }
    while(num<999 && exists(buf));
    
    BITMAP *panorama = create_bitmap_ex(8,256,168);
    
    if(tmpscr->flags3&fNOSUBSCR)
    {
        clear_to_color(panorama,0);
        blit(framebuf,panorama,0,playing_field_offset,0,0,256,168);
        save_bitmap(buf,panorama,realpal?temppal:RAMpal);
    }
    else
    {
        save_bitmap(buf,framebuf,realpal?temppal:RAMpal);
    }
    
    destroy_bitmap(panorama);
    return D_O_K;
}

int onSnapshot()
{
    if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
    {
        onGUISnapshot();
    }
    else
    {
        onNonGUISnapshot();
    }
    
    return D_O_K;
}

int onSaveMapPic()
{
    BITMAP* mappic = NULL;
    int mapres2 = 0;
    char buf[20];
    int num=0;
    set_clip_state(temp_buf,1);
    set_clip_rect(temp_buf,0,0,temp_buf->w, temp_buf->h);
    
    do
    {
#ifdef ALLEGRO_MACOSX
        sprintf(buf, "../../../zelda%03d.png", ++num);
#else
        sprintf(buf, "zelda%03d.png", ++num);
#endif
    }
    while(num<999 && exists(buf));
    
    //  if(!mappic) {
    mappic = create_bitmap_ex(8,(256*16)>>mapres2,(176*8)>>mapres2);
    
    if(!mappic)
    {
        system_pal();
        jwin_alert("Save Map Picture","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
        game_pal();
        return D_O_K;
    }
    
    //  }
    
    int layermap, layerscreen;
    int x2=0;
    
    // draw the map
    for(int y=0; y<8; y++)
    {
        for(int x=0; x<16; x++)
        {
            int s = (y<<4) + x;
            
            if(!displayOnMap(x, y))
            {
                rectfill(temp_buf, 0, 0, 255, 223, WHITE);
            }
            else
            {
                loadscr(1,currdmap,s,-1,false);
                putscr(temp_buf, 0, 0, tmpscr+1);
                
                for(int k=0; k<4; k++)
                {
                    if(k==2)
                    {
                        putscrdoors(temp_buf, 0, 0, tmpscr+1);
                    }
                    
                    layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
                    
                    if(layermap>-1)
                    {
                        layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
                        
                        if(TheMaps[currmap*MAPSCRS+s].layeropacity[k]==255)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(temp_buf,((i&15)<<4)+x2,(i&0xF0),TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(temp_buf,((i&15)<<4)+x2,(i&0xF0),TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],TheMaps[currmap*MAPSCRS+s].layeropacity[k]);
                            }
                        }
                    }
                }
                
                for(int i=0; i<176; i++)
                {
//          if (COMBOTYPE((i&15)<<4,i&0xF0)==cOLD_OVERHEAD)
                    if(combo_class_buf[COMBOTYPE((i&15)<<4,i&0xF0)].overhead)
                    {
                        overcombo(temp_buf,((i&15)<<4)+x2,(i&0xF0),MAPCOMBO((i&15)<<4,i&0xF0),MAPCSET((i&15)<<4,i&0xF0));
                    }
                }
                
                for(int k=4; k<6; k++)
                {
                    layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
                    
                    if(layermap>-1)
                    {
                        layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
                        
                        if(TheMaps[currmap*MAPSCRS+s].layeropacity[k]==255)
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombo(temp_buf,((i&15)<<4)+x2,(i&0xF0),TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                            }
                        }
                        else
                        {
                            for(int i=0; i<176; i++)
                            {
                                overcombotranslucent(temp_buf,((i&15)<<4)+x2,(i&0xF0),TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],TheMaps[currmap*MAPSCRS+s].layeropacity[k]);
                            }
                        }
                    }
                }
            }
            
            stretch_blit(temp_buf, mappic, 0, 0, 256, 176,
                         x<<(8-mapres2), (y*176)>>mapres2, 256>>mapres2, 176>>mapres2);
        }
        
    }
    
    save_bitmap(buf,mappic,RAMpal);
    destroy_bitmap(mappic);
    return D_O_K;
}

void f_Quit(int type)
{
    if(type==qQUIT && !Playing)
        return;
        
    music_pause();
    pause_all_sfx();
    system_pal();
    clear_keybuf();
    
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
        clear_to_color(screen,BLACK);
    }
    else
    {
        game_pal();
        music_resume();
        resume_all_sfx();
    }
    
    eat_buttons();
    
    if(key[KEY_ESC])
        key[KEY_ESC]=0;
        
    if(key[KEY_ENTER])
        key[KEY_ENTER]=0;
}

//----------------------------------------------------------------

int onNoWalls()
{
    toogam = !toogam;
    
    if(toogam)
    {
        cheat_superman=true;
        setClock(true);
    }
    
    return D_O_K;
}

int onIgnoreSideview()
{
    ignoreSideview = !ignoreSideview;
    return D_O_K;
}

int input_idle(bool checkmouse)
{
    static int mx, my, mz, mb;
    
    if(keypressed() || zc_key_pressed() ||
       (checkmouse && (mx != gui_mouse_x() || my != gui_mouse_y() || mz != gui_mouse_z() || mb != gui_mouse_b())))
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
    
    mx = gui_mouse_x();
    my = gui_mouse_y();
    mz = gui_mouse_z();
    mb = gui_mouse_b();
    
    return idle_count;
}

int onGoFast()
{
    gofast=gofast?false:true;
    return D_O_K;
}

int onKillCheat()
{
    for(int i=0; i<guys.Count(); i++)
    {
        if(!(((enemy*)guys.spr(i))->flags & guy_doesntcount))((enemy*)guys.spr(i))->kickbucket();
    }
    
    return D_O_K;
}

int onShowLayer0()
{
    show_layer_0 = !show_layer_0;
    return D_O_K;
}
int onShowLayer1()
{
    show_layer_1 = !show_layer_1;
    return D_O_K;
}
int onShowLayer2()
{
    show_layer_2 = !show_layer_2;
    return D_O_K;
}
int onShowLayer3()
{
    show_layer_3 = !show_layer_3;
    return D_O_K;
}
int onShowLayer4()
{
    show_layer_4 = !show_layer_4;
    return D_O_K;
}
int onShowLayer5()
{
    show_layer_5 = !show_layer_5;
    return D_O_K;
}
int onShowLayer6()
{
    show_layer_6 = !show_layer_6;
    return D_O_K;
}
int onShowLayerO()
{
    show_layer_over=!show_layer_over;
    return D_O_K;
}
int onShowLayerP()
{
    show_layer_push=!show_layer_push;
    return D_O_K;
}
int onShowLayerS()
{
    show_sprites=!show_sprites;
    return D_O_K;
}
int onShowLayerF()
{
    show_ffcs=!show_ffcs;
    return D_O_K;
}
int onShowLayerW()
{
    show_walkflags=!show_walkflags;
    return D_O_K;
}
int onShowFFScripts()
{
    show_ff_scripts=!show_ff_scripts;
    return D_O_K;
}
int onShowHitboxes()
{
    show_hitboxes=!show_hitboxes;
    return D_O_K;
}

int onLightSwitch()
{
    do_cheat_light=true;
    return D_O_K;
}

int onGoTo();
int onGoToComplete();

// Used in syskeys() to prevent keys from being read as both game and system input
static int storedInput[14];
static void backupAndClearInput()
{
    storedInput[0]=key[DUkey];
    key[DUkey]=false;
    storedInput[1]=key[DDkey];
    key[DDkey]=false;
    storedInput[2]=key[DLkey];
    key[DLkey]=false;
    storedInput[3]=key[DRkey];
    key[DRkey]=false;
    storedInput[4]=key[Akey];
    key[Akey]=false;
    storedInput[5]=key[Bkey];
    key[Bkey]=false;
    storedInput[6]=key[Skey];
    key[Skey]=false;
    storedInput[7]=key[Lkey];
    key[Lkey]=false;
    storedInput[8]=key[Rkey];
    key[Rkey]=false;
    storedInput[9]=key[Pkey];
    key[Pkey]=false;
    storedInput[10]=key[Exkey1];
    key[Exkey1]=false;
    storedInput[11]=key[Exkey2];
    key[Exkey2]=false;
    storedInput[12]=key[Exkey3];
    key[Exkey3]=false;
    storedInput[13]=key[Exkey4];
    key[Exkey4]=false;
}

static void restoreInput()
{
    key[DUkey]=storedInput[0];
    key[DDkey]=storedInput[1];
    key[DLkey]=storedInput[2];
    key[DRkey]=storedInput[3];
    key[Akey]=storedInput[4];
    key[Bkey]=storedInput[5];
    key[Skey]=storedInput[6];
    key[Lkey]=storedInput[7];
    key[Rkey]=storedInput[8];
    key[Pkey]=storedInput[9];
    key[Exkey1]=storedInput[10];
    key[Exkey2]=storedInput[11];
    key[Exkey3]=storedInput[12];
    key[Exkey4]=storedInput[13];
}

void syskeys()
{
	  //Saffith's method of separating system and game key bindings. Can't do this!!
    //backupAndClearInput(); //This caused input to become randomly 'stuck'. -Z
    
    int oldtitle_version;
    
    if(close_button_quit)
    {
        close_button_quit=false;
        f_Quit(qEXIT);
    }
    
    poll_joystick();
    
    if(rMbtn() || (gui_mouse_b() && !mouse_down && ClickToFreeze &&!disableClickToFreeze))
    {
        oldtitle_version=title_version;
        System();
    }
    
    mouse_down=gui_mouse_b();
    
    if(ReadKey(KEY_F1))
    {
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
            halt=!halt;
            //zinit.subscreen=(zinit.subscreen+1)%ssdtMAX;
        }
        else
        {
            Throttlefps=!Throttlefps;
            logic_counter=0;
        }
    }
    
    //  if(ReadKey(KEY_F1))    Vsync=!Vsync;
    /*
      if(ReadKey(KEY_F1))    set_bit(QHeader.rules4,qr4_NEWENEMYTILES,
      1-((get_bit(QHeader.rules4,qr4_NEWENEMYTILES))));
      */
    
    if(ReadKey(KEY_OPENBRACE))    if(frame_rest_suggest > 0) frame_rest_suggest--;
    
    if(ReadKey(KEY_CLOSEBRACE))    if(frame_rest_suggest <= 2) frame_rest_suggest++;
    
    if(ReadKey(KEY_F2))    ShowFPS=!ShowFPS;
    
    if(ReadKey(KEY_F3) && Playing)    Paused=!Paused;
    
    if(ReadKey(KEY_F4) && Playing)
    {
        Paused=true;
        Advance=true;
    }
    
    if(ReadKey(KEY_F6))    if(!get_bit(quest_rules, qr_NOCONTINUE)) f_Quit(qQUIT);
    
#ifndef ALLEGRO_MACOSX
    if(ReadKey(KEY_F9))    f_Quit(qRESET);
    
    if(ReadKey(KEY_F10))   f_Quit(qEXIT);
#else
    if(ReadKey(KEY_F7))    f_Quit(qRESET);
    
    if(ReadKey(KEY_F8))   f_Quit(qEXIT);
#endif
    if(rF5()&&(Playing && currscr<128 && DMaps[currdmap].flags&dmfVIEWMAP))    onSaveMapPic();
    
    if(rF12())
    {
        onSnapshot();
    }
    
    if(debug_enabled && ReadKey(KEY_TAB))
        set_debug(!get_debug());
        
    if(get_debug() || cheat>=1)
    {
	if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
	{
		if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
		{
			if(ReadKey(KEY_ASTERISK) || ReadKey(KEY_H))   game->set_life(game->get_maxlife());
			
			if(ReadKey(KEY_SLASH_PAD) || ReadKey(KEY_M))  game->set_magic(game->get_maxmagic());
			
			if(ReadKey(KEY_R))          game->set_drupy(999);
			
			if(ReadKey(KEY_B))
			{
			    onCheatBombs();
			}
			
			if(ReadKey(KEY_A))
			{
			    onCheatArrows();
			}
		}
	}
    }
    
    if(get_debug() || cheat>=2)
    {
	if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
	{
		if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
		{
			if(rI())
			{
			    setClock(!getClock());
			    cheat_superman=getClock();
			}
		}
	}
    }
    
    if(get_debug() || cheat>=4)
    {
	if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
	{
		if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
		{
			if(rF11())
			{
			    onNoWalls();
			}
			
			if(rQ())
			{
			    onGoFast();
			}
			
			if(ReadKey(KEY_F))
			{
			    if(Link.getAction()==freeze)
			    {
				Link.unfreeze();
			    }
			    else
			    {
				Link.Freeze();
			    }
			}
			
			if(ReadKey(KEY_G))   onGoToComplete();
			
			if(ReadKey(KEY_0))   onShowLayer0();
			
			if(ReadKey(KEY_1))   onShowLayer1();
			
			if(ReadKey(KEY_2))   onShowLayer2();
			
			if(ReadKey(KEY_3))   onShowLayer3();
			
			if(ReadKey(KEY_4))   onShowLayer4();
			
			if(ReadKey(KEY_5))   onShowLayer5();
			
			if(ReadKey(KEY_6))   onShowLayer6();
			
			//if(ReadKey(KEY_7))   onShowLayerO();
			if(ReadKey(KEY_7))   onShowLayerF();
			
			if(ReadKey(KEY_8))   onShowLayerS();
			
			if(ReadKey(KEY_W))   onShowLayerW();
			
			if(ReadKey(KEY_L))   onLightSwitch();
			
			if(ReadKey(KEY_V))   onIgnoreSideview();
		}
	}
    }
    
    if(volkeys)
    {
        if(ReadKey(KEY_PGUP)) master_volume(-1,midi_volume+8);
        
        if(ReadKey(KEY_PGDN)) master_volume(-1,midi_volume==255?248:midi_volume-8);
        
        if(ReadKey(KEY_HOME)) master_volume(digi_volume+8,-1);
        
        if(ReadKey(KEY_END))  master_volume(digi_volume==255?248:digi_volume-8,-1);
    }
    
    if(!get_debug() || !SystemKeys)
        goto bottom;
        
    if(ReadKey(KEY_D))
    {
        details = !details;
        rectfill(screen,0,0,319,7,BLACK);
        rectfill(screen,0,8,31,239,BLACK);
        rectfill(screen,288,8,319,239,BLACK);
        rectfill(screen,32,232,287,239,BLACK);
    }
    
    if(ReadKey(KEY_P))   Paused=!Paused;
    
    //if(ReadKey(KEY_P))   centerLink();
    if(ReadKey(KEY_A))
    {
        Paused=true;
        Advance=true;
    }
    
    if(ReadKey(KEY_G))   db=(db==999)?0:999;
#ifndef ALLEGRO_MACOSX
    if(ReadKey(KEY_F8))  Showpal=!Showpal;
    
    if(ReadKey(KEY_F7))
    {
        Matrix(ss_speed, ss_density, 0);
        game_pal();
    }
#else
    // The reason these are different on Mac in the first place is that
    // the OS doesn't let us use F9 and F10...
    if(ReadKey(KEY_F10))  Showpal=!Showpal;
    
    if(ReadKey(KEY_F9))
    {
        Matrix(ss_speed, ss_density, 0);
        game_pal();
    }
#endif
    if(ReadKey(KEY_PLUS_PAD) || ReadKey(KEY_EQUALS))
    {
        //change containers
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
            //magic containers
            if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
            {
                game->set_maxmagic(zc_min(game->get_maxmagic()+MAGICPERBLOCK,MAGICPERBLOCK*8));
            }
            else
            {
                game->set_maxlife(zc_min(game->get_maxlife()+HP_PER_HEART,HP_PER_HEART*24));
            }
        }
        else
        {
            if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
            {
                game->set_magic(zc_min(game->get_magic()+1,game->get_maxmagic()));
            }
            else
            {
                game->set_life(zc_min(game->get_life()+1,game->get_maxlife()));
            }
        }
    }
    
    if(ReadKey(KEY_MINUS_PAD) || ReadKey(KEY_MINUS))
    {
        //change containers
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
            //magic containers
            if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
            {
                game->set_maxmagic(zc_max(game->get_maxmagic()-MAGICPERBLOCK,0));
                game->set_magic(zc_min(game->get_maxmagic(), game->get_magic()));
                //heart containers
            }
            else
            {
                game->set_maxlife(zc_max(game->get_maxlife()-HP_PER_HEART,HP_PER_HEART));
                game->set_life(zc_min(game->get_maxlife(), game->get_life()));
            }
        }
        else
        {
            if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
            {
                game->set_magic(zc_max(game->get_magic()-1,0));
            }
            else
            {
                game->set_life(zc_max(game->get_life()-1,0));
            }
        }
    }
    
    if(ReadKey(KEY_COMMA))  jukebox(currmidi-1);
    
    if(ReadKey(KEY_STOP))   jukebox(currmidi+1);
    
    /*
      if(ReadKey(KEY_TILDE)) {
      wavyout();
      zinit.subscreen=(zinit.subscreen+1)%3;
      wavyin();
      }
      */
    
    verifyBothWeapons();
    
bottom:

    if(input_idle(true) > after_time())
    {
        Matrix(ss_speed, ss_density, 0);
        game_pal();
    }
    //Saffith's method of separating system and game key bindings. Can't do this!!
    //restoreInput(); //This caused input to become randomly 'stuck'. -Z
    
    //while(Playing && keypressed())
    //readkey();
    // What's the Playing check for?
    clear_keybuf();
}

void checkQuitKeys()
{
#ifndef ALLEGRO_MACOSX
    if(ReadKey(KEY_F9))    f_Quit(qRESET);
    
    if(ReadKey(KEY_F10))   f_Quit(qEXIT);
#else
    if(ReadKey(KEY_F7))    f_Quit(qRESET);
    
    if(ReadKey(KEY_F8))   f_Quit(qEXIT);
#endif
}

// 99*360 + 59*60
#define MAXTIME  21405240

void advanceframe(bool allowwavy, bool sfxcleanup)
{
    if(zcmusic!=NULL)
    {
        zcmusic_poll();
    }
    
    while(Paused && !Advance && !Quit)
    {
        // have to call this, otherwise we'll get an infinite loop
        syskeys();
        // to keep fps constant
        updatescr(allowwavy);
        throttleFPS();
        
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
    }
    
    if(Quit)
        return;
        
    if(Playing && game->get_time()<MAXTIME)
        game->change_time(1);
        
    Advance=false;
    ++frame;
    
    syskeys();
    // Someday... maybe install a Turbo button here?
    updatescr(allowwavy);
    throttleFPS();
    
#ifdef _WIN32
    
    if(use_dwm_flush)
    {
        do_DwmFlush();
    }
    
#endif
    
    //textprintf_ex(screen,font,0,72,254,BLACK,"%d %d", lastentrance, lastentrance_dmap);
    if(sfxcleanup)
        sfx_cleanup();
}

void zapout()
{
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
    blit(framebuf,scrollbuf,0,0,256,0,256,224);
    
    script_drawing_commands.Clear();
    
    // zap out
    for(int i=1; i<=24; i++)
    {
        draw_fuzzy(i);
        syskeys();
        advanceframe(true);
        
        if(Quit)
        {
            break;
        }
    }
}

void zapin()
{
    draw_screen(tmpscr);
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
    put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
    blit(framebuf,scrollbuf,0,0,256,0,256,224);
    
    // zap out
    for(int i=24; i>=1; i--)
    {
        draw_fuzzy(i);
        syskeys();
        advanceframe(true);
        
        if(Quit)
        {
            break;
        }
    }
}


void wavyout(bool showlink)
{
    draw_screen(tmpscr, showlink);
    put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
    
    BITMAP *wavebuf = create_bitmap_ex(8,288,224);
    clear_to_color(wavebuf,0);
    blit(framebuf,wavebuf,0,0,16,0,256,224);
    
    PALETTE wavepal;
    
    int ofs;
    int amplitude=8;
    
    int wavelength=4;
    double palpos=0, palstep=4, palstop=126;
    
    for(int i=0; i<168; i+=wavelength)
    {
        for(int l=0; l<256; l++)
        {
            wavepal[l].r=vbound(int(RAMpal[l].r+((palpos/palstop)*(63-RAMpal[l].r))),0,63);
            wavepal[l].g=vbound(int(RAMpal[l].g+((palpos/palstop)*(63-RAMpal[l].g))),0,63);
            wavepal[l].b=vbound(int(RAMpal[l].b+((palpos/palstop)*(63-RAMpal[l].b))),0,63);
        }
        
        palpos+=palstep;
        
        if(palpos>=0)
        {
            set_palette(wavepal);
        }
        else
        {
            set_palette(RAMpal);
        }
        
        for(int j=0; j+playing_field_offset<224; j++)
        {
            for(int k=0; k<256; k++)
            {
                ofs=0;
                
                if((j<i)&&(j&1))
                {
                    ofs=int(sin((double(i+j)*2*PI/168.0))*amplitude);
                }
                
                framebuf->line[j+playing_field_offset][k]=wavebuf->line[j+playing_field_offset][k+ofs+16];
            }
        }
        
        syskeys();
        advanceframe(true);
        
        //    animate_combos();
        if(Quit)
            break;
    }
    
    destroy_bitmap(wavebuf);
}

void wavyin()
{
    draw_screen(tmpscr);
    put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
    
    BITMAP *wavebuf = create_bitmap_ex(8,288,224);
    clear_to_color(wavebuf,0);
    blit(framebuf,wavebuf,0,0,16,0,256,224);
    
    PALETTE wavepal;
    
    //Breaks dark rooms.
    //In any case I don't think we need this, since palette is already loaded in doWarp() (famous last words...) -DD
    /*
      loadfullpal();
      loadlvlpal(DMaps[currdmap].color);
      ringcolor(false);
    */
    refreshpal=false;
    int ofs;
    int amplitude=8;
    int wavelength=4;
    double palpos=168, palstep=4, palstop=126;
    
    for(int i=0; i<168; i+=wavelength)
    {
        for(int l=0; l<256; l++)
        {
            wavepal[l].r=vbound(int(RAMpal[l].r+((palpos/palstop)*(63-RAMpal[l].r))),0,63);
            wavepal[l].g=vbound(int(RAMpal[l].g+((palpos/palstop)*(63-RAMpal[l].g))),0,63);
            wavepal[l].b=vbound(int(RAMpal[l].b+((palpos/palstop)*(63-RAMpal[l].b))),0,63);
        }
        
        palpos-=palstep;
        
        if(palpos>=0)
        {
            set_palette(wavepal);
        }
        else
        {
            set_palette(RAMpal);
        }
        
        for(int j=0; j+playing_field_offset<224; j++)
        {
            for(int k=0; k<256; k++)
            {
                ofs=0;
                
                if((j<(167-i))&&(j&1))
                {
                    ofs=int(sin((double(i+j)*2*PI/168.0))*amplitude);
                }
                
                framebuf->line[j+playing_field_offset][k]=wavebuf->line[j+playing_field_offset][k+ofs+16];
            }
        }
        
        syskeys();
        advanceframe(true);
        //    animate_combos();
        
        if(Quit)
            break;
    }
    
    destroy_bitmap(wavebuf);
}

void blackscr(int fcnt,bool showsubscr)
{
    reset_pal_cycling();
    script_drawing_commands.Clear();
    
    while(fcnt>0)
    {
        clear_bitmap(framebuf);
        
        if(showsubscr)
        {
            put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
        }
        
        syskeys();
        advanceframe(true);
        
        if(Quit)
            break;
            
        --fcnt;
    }
}

void openscreen()
{
    reset_pal_cycling();
    black_opening_count=0;
    
    if(COOLSCROLL)
    {
        open_black_opening(LinkX()+8, (LinkY()-LinkZ())+8+playing_field_offset, true);
        return;
    }
    else
    {
        Link.setDontDraw(true);
        show_subscreen_dmap_dots=false;
        show_subscreen_numbers=false;
        //    show_subscreen_items=false;
        show_subscreen_life=false;
    }
    
    int x=128;
    
    for(int i=0; i<80; i++)
    {
        draw_screen(tmpscr);
        //? draw_screen already draws the subscreen -DD
        //put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
        x=128-(((i*128/80)/8)*8);
        
        if(x>0)
        {
            rectfill(framebuf,0,playing_field_offset,x,167+playing_field_offset,0);
            rectfill(framebuf,256-x,playing_field_offset,255,167+playing_field_offset,0);
        }
        
        //    x=((80-i)/2)*4;
        /*
          --x;
          switch(++c)
          {
          case 5: c=0;
          case 0:
          case 2:
          case 3: --x; break;
          }
          */
        syskeys();
        advanceframe(true);
        
        if(Quit)
        {
            break;
        }
    }
    
    Link.setDontDraw(false);
    show_subscreen_items=true;
    show_subscreen_dmap_dots=true;
}

int TriforceCount()
{
    int c=0;
    
    for(int i=1; i<=8; i++)
        if(game->lvlitems[i]&liTRIFORCE)
            ++c;
            
    return c;
}

int onCustomGame()
{
    int file =  getsaveslot();
    
    if(file < 0)
        return D_O_K;
        
    bool ret = (custom_game(file)!=0);
    return ret ? D_CLOSE : D_O_K;
}

int onContinue()
{
    return D_CLOSE;
}

int onEsc() // Unused?? -L
{
    return key[KEY_ESC]?D_CLOSE:D_O_K;
}

int onVsync()
{
    Throttlefps = !Throttlefps;
    return D_O_K;
}

int onClickToFreeze()
{
    ClickToFreeze = !ClickToFreeze;
    return D_O_K;
}

int OnSaveZCConfig()
{
	if(jwin_alert3(
			"Save Configuration", 
			"Are you sure that you wish to save your present configuration settings?", 
			"This will overwrite your prior settings!",
			NULL,
		 "&Yes", 
		"&No", 
		NULL, 
		'y', 
		'n', 
		0, 
		lfont) == 1)	
	{
		save_game_configs();
		return D_O_K;
	}
	else return D_O_K;
}

int onDebugConsole()
{
	if ( !zconsole ) {
		if(jwin_alert3(
			"WARNING: Closing the Debug Console", 
			"WARNING: Closing the console window by using the close window widget will TERMINATE ZC!", 
			"To SAFELY close the debug console, use the SHOW DEBUG CONSOLE menu uption again!",
			"Are you seure that you wish to open the debug console?",
		 "&Yes", 
		"&No", 
		NULL, 
		'y', 
		'n', 
		0, 
		lfont) == 1)
		{
			DebugConsole::Open();

			zconsole = true;
			return D_O_K;
		}
		else return D_O_K;
	}
	else { 
		
		zconsole = false;
		DebugConsole::Close();
		return D_O_K;
	}
}


int onFrameSkip()
{
    FrameSkip = !FrameSkip;
    return D_O_K;
}

int onTransLayers()
{
    TransLayers = !TransLayers;
    return D_O_K;
}

int onNESquit()
{
    NESquit = !NESquit;
    return D_O_K;
}

int onVolKeys()
{
    volkeys = !volkeys;
    return D_O_K;
}

int onShowFPS()
{
    ShowFPS = !ShowFPS;
    scare_mouse();
    
    if(ShowFPS)
        show_fps(screen);
        
    if(sbig)
        stretch_blit(fps_undo,screen,0,0,64,16,scrx+40-120,scry+216+96,128,32);
    else
        blit(fps_undo,screen,0,0,scrx+40,scry+216,64,16);
        
    if(Paused)
        show_paused(screen);
        
    unscare_mouse();
    return D_O_K;
}

bool is_Fkey(int k)
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

void kb_getkey(DIALOG *d)
{
    d->flags|=D_SELECTED;
    
    scare_mouse();
    jwin_button_proc(MSG_DRAW,d,0);
    jwin_draw_win(screen, (resx-160)/2, (resy-48)/2, 160, 48, FR_WIN);
    //  text_mode(vc(11));
    textout_centre_ex(screen, font, "Press a key", resx/2, resy/2 - 8, jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_centre_ex(screen, font, "ESC to cancel", resx/2, resy/2, jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    unscare_mouse();
    
    clear_keybuf();
    int k = next_press_key();
    clear_keybuf();
    
    //shnarf
    //47=f1
    //59=esc
    if(k>0 && k<123 && !((k>46)&&(k<60)))
        *((int*)d->dp3) = k;
        
        
    d->flags&=~D_SELECTED;
}

int d_kbutton_proc(int msg,DIALOG *d,int c)
{
    switch(msg)
    {
    case MSG_KEY:
    case MSG_CLICK:
    
        kb_getkey(d);
        
        while(gui_mouse_b())
            clear_keybuf();
            
        return D_REDRAW;
    }
    
    return jwin_button_proc(msg,d,c);
}

void j_getbtn(DIALOG *d)
{
    d->flags|=D_SELECTED;
    scare_mouse();
    jwin_button_proc(MSG_DRAW,d,0);
    jwin_draw_win(screen, (resx-160)/2, (resy-48)/2, 160, 48, FR_WIN);
    //  text_mode(vc(11));
    int y = resy/2 - 12;
    textout_centre_ex(screen, font, "Press a button", resx/2, y, jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_centre_ex(screen, font, "ESC to cancel", resx/2, y+8, jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_centre_ex(screen, font, "SPACE to disable", resx/2, y+16, jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    unscare_mouse();
    
    int b = next_press_btn();
    
    if(b>=0)
        *((int*)d->dp3) = b;
        
    d->flags&=~D_SELECTED;
    
    if(!player) //safety first...
        player = init_dialog(d,-1);
        
    player->joy_on = TRUE;
}

int d_jbutton_proc(int msg,DIALOG *d,int c)
{
    switch(msg)
    {
    case MSG_KEY:
    case MSG_CLICK:
    
        j_getbtn(d);
        
        while(gui_mouse_b())
            clear_keybuf();
            
        return D_REDRAW;
    }
    
    return jwin_button_proc(msg,d,c);
}

//shnarf
const char *key_str[] =
{
    "(none)",         "a",              "b",              "c",
    "d",              "e",              "f",              "g",
    "h",              "i",              "j",              "k",
    "l",              "m",              "n",              "o",
    "p",              "q",              "r",              "s",
    "t",              "u",              "v",              "w",
    "x",              "y",              "z",              "0",
    "1",              "2",              "3",              "4",
    "5",              "6",              "7",              "8",
    "9",              "num 0",          "num 1",          "num 2",
    "num 3",          "num 4",          "num 5",          "num 6",
    "num 7",          "num 8",          "num 9",          "f1",
    "f2",             "f3",             "f4",             "f5",
    "f6",             "f7",             "f8",             "f9",
    "f10",            "f11",            "f12",            "esc",
    "~",              "-",              "=",              "backspace",
    "tab",            "{",              "}",              "enter",
    ":",              "quote",          "\\",             "\\ (2)",
    ",",              ".",              "/",              "space",
    "insert",         "delete",         "home",           "end",
    "page up",        "page down",      "left",           "right",
    "up",             "down",           "num /",          "num *",
    "num -",          "num +",          "num delete",     "num enter",
    "print screen",   "pause",          "abnt c1",        "yen",
    "kana",           "convert",        "no convert",     "at",
    "circumflex",     ": (2)",          "kanji",          "num =",
    "back quote",     ";",              "command",        "unknown (0)",
    "unknown (1)",    "unknown (2)",    "unknown (3)",    "unknown (4)",
    "unknown (5)",    "unknown (6)",    "unknown (7)",    "left shift",
    "right shift",    "left control",   "right control",  "alt",
    "alt gr",         "left win",       "right win",      "menu",
    "scroll lock",    "number lock",    "caps lock",      "MAX"
};

const char *pan_str[4] = { "MONO", " 1/2", " 3/4", "FULL" };
//extern int zcmusic_bufsz;

static char str_a[80],str_b[80],str_s[80],str_m[16],str_l[16],str_r[16],str_p[16],str_ex1[16],str_ex2[16],str_ex3[16],str_ex4[16];

int d_stringloader(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    if(msg==MSG_DRAW)
    {
        switch(d->w)
        {
        case 0:
            sprintf(str_a,"%d\n%s",Akey,key_str[Akey]);
            sprintf(str_b,"%d\n%s",Bkey,key_str[Bkey]);
            sprintf(str_s,"%d\n%s",Skey,key_str[Skey]);
            sprintf(str_l,"%d\n%s",Lkey,key_str[Lkey]);
            sprintf(str_r,"%d\n%s",Rkey,key_str[Rkey]);
            sprintf(str_p,"%d\n%s",Pkey,key_str[Pkey]);
            sprintf(str_ex1,"%d\n%s",Exkey1,key_str[Exkey1]);
            sprintf(str_ex2,"%d\n%s",Exkey2,key_str[Exkey2]);
            sprintf(str_ex3,"%d\n%s",Exkey3,key_str[Exkey3]);
            sprintf(str_ex4,"%d\n%s",Exkey4,key_str[Exkey4]);
            break;
            
        case 1:
            sprintf(str_a,"%d\n%s",DUkey,key_str[DUkey]);
            sprintf(str_b,"%d\n%s",DDkey,key_str[DDkey]);
            sprintf(str_l,"%d\n%s",DLkey,key_str[DLkey]);
            sprintf(str_r,"%d\n%s",DRkey,key_str[DRkey]);
            break;
            
        case 2:
            sprintf(str_a,"%d",Abtn);
            sprintf(str_b,"%d",Bbtn);
            sprintf(str_s,"%d",Sbtn);
            sprintf(str_l,"%d",Lbtn);
            sprintf(str_r,"%d",Rbtn);
            sprintf(str_m,"%d",Mbtn);
            sprintf(str_p,"%d",Pbtn);
            sprintf(str_ex1,"%d",Exbtn1);
            sprintf(str_ex2,"%d",Exbtn2);
            sprintf(str_ex3,"%d",Exbtn3);
            sprintf(str_ex4,"%d",Exbtn4);
            break;
            
        case 3:
            sprintf(str_a,"%3d",midi_volume);
            sprintf(str_b,"%3d",digi_volume);
            sprintf(str_l,"%3d",emusic_volume);
            sprintf(str_m,"%3dKB",zcmusic_bufsz);
            sprintf(str_r,"%3d",sfx_volume);
            strcpy(str_s,pan_str[pan_style]);
            break;
            
        case 4:
            sprintf(str_a,"%d",DUbtn);
            sprintf(str_b,"%d",DDbtn);
            sprintf(str_l,"%d",DLbtn);
            sprintf(str_r,"%d",DRbtn);
        }
    }
    
    return D_O_K;
}

int set_vol(void *dp3, int d2)
{
    switch(((int*)dp3)[0])
    {
    case 0:
        midi_volume   = zc_min(d2<<3,255);
        break;
        
    case 1:
        digi_volume   = zc_min(d2<<3,255);
        break;
        
    case 2:
        emusic_volume = zc_min(d2<<3,255);
        break;
        
    case 3:
        sfx_volume    = zc_min(d2<<3,255);
        break;
    }
    
    scare_mouse();
    // text_mode(vc(11));
    textprintf_right_ex(screen,is_large ? lfont_l : font, ((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%3d",zc_min(d2<<3,255));
    unscare_mouse();
    return D_O_K;
}

int set_pan(void *dp3, int d2)
{
    pan_style = vbound(d2,0,3);
    scare_mouse();
    // text_mode(vc(11));
    textout_right_ex(screen,is_large ? lfont_l : font, pan_str[pan_style],((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    unscare_mouse();
    return D_O_K;
}

int set_buf(void *dp3, int d2)
{
    scare_mouse();
    // text_mode(vc(11));
    zcmusic_bufsz = d2 + 1;
    textprintf_right_ex(screen,is_large ? lfont_l : font, ((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%3dKB",zcmusic_bufsz);
    unscare_mouse();
    return D_O_K;
}

static DIALOG key_dlg[] =
{
    // (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3)
    { jwin_win_proc,       8,    44,   304,  228,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Keyboard Buttons", NULL,  NULL },
    { d_stringloader,      0,    0,    0,    0,    0,       0,       0,       0,         0,        0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     14,   70,   148,  105,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     158,  70,   148,  105,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     14,  171,   292,  67,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_text_proc,         30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Standard", NULL,  NULL },
    { jwin_text_proc,         175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Extended", NULL,  NULL },
    
    { jwin_text_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a, NULL,  NULL },
    { jwin_text_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b, NULL,  NULL },
    { jwin_text_proc,      92,   148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s, NULL,  NULL },
    { jwin_text_proc,      92,   180,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex1, NULL,  NULL },
    { jwin_text_proc,      92,   212,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex3, NULL,  NULL },
    { jwin_text_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l, NULL,  NULL },
    { jwin_text_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r, NULL,  NULL },
    { jwin_text_proc,      237,  148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_p, NULL,  NULL },
    { jwin_text_proc,      237,  180,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex2, NULL,  NULL },
    { jwin_text_proc,      237,  212,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex4, NULL,  NULL },
    
    { d_kbutton_proc,      22,   90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "A",     NULL, &Akey},
    { d_kbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "B",     NULL, &Bkey},
    { d_kbutton_proc,      22,   146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Start", NULL, &Skey},
    { d_kbutton_proc,      22,   178,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX1",     NULL, &Exkey1},
    { d_kbutton_proc,      22,   210,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX3", NULL, &Exkey3},
    { d_kbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "L",     NULL, &Lkey},
    { d_kbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "R",     NULL, &Rkey},
    { d_kbutton_proc,      167,  146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Map",   NULL, &Pkey},
    { d_kbutton_proc,      167,  178,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX2",     NULL, &Exkey2},
    { d_kbutton_proc,      167,  210,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX4",   NULL, &Exkey4},
    
    { jwin_button_proc,    90,   240,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    170,  240,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};
static DIALOG keydir_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       8,    44,   304,  172,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Keyboard Directions", NULL,  NULL },
    { d_stringloader,      0,    0,    1,    0,    0,       0,       0,       0,         0,        0,       NULL, NULL, NULL },
    { jwin_frame_proc,     14,   70,   147,  80,   0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     159,  70,   147,  80,   0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_text_proc,         30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Vertical", NULL,  NULL },
    { jwin_text_proc,         175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Horizontal", NULL,  NULL },
    
    { jwin_text_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a, NULL,  NULL },
    { jwin_text_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b, NULL,  NULL },
    { jwin_text_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l, NULL,  NULL },
    { jwin_text_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r, NULL,  NULL },
    
    { d_kbutton_proc,      22,   90,   61,   21,   vc(14),  vc(11),  0,       0,         0,        0, (void *) "Up",     NULL, &DUkey},
    { d_kbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Down",   NULL, &DDkey},
    { d_kbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Left",   NULL, &DLkey},
    { d_kbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Right",  NULL, &DRkey},
    // 14
    { jwin_button_proc,    90,   184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    170,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG btn_dlg[] =
{
    // (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3)
    { jwin_win_proc,       8,    44,   304,  228,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Joystick Buttons", NULL,  NULL },
    { d_stringloader,      0,    0,    2,    0,    0,       0,       0,       0,         0,        0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     14,   70,   148,  105,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     158,  70,   148,  105,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     14,  171,   292,  67,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_text_proc,      30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Standard", NULL,  NULL },
    { jwin_text_proc,      175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Extended", NULL,  NULL },
    
    { jwin_ctext_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a, NULL,  NULL },
    { jwin_ctext_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b, NULL,  NULL },
    { jwin_ctext_proc,      92,   148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s, NULL,  NULL },
    { jwin_ctext_proc,      92,   180,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex1, NULL,  NULL },
    { jwin_ctext_proc,      92,   212,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex3, NULL,  NULL },
    { jwin_ctext_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l, NULL,  NULL },
    { jwin_ctext_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r, NULL,  NULL },
    { jwin_ctext_proc,      237,  148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_p, NULL,  NULL },
    { jwin_ctext_proc,      237,  180,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex2, NULL,  NULL },
    { jwin_ctext_proc,      237,  212,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_ex4, NULL,  NULL },
    
    { d_jbutton_proc,      22,   90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "A",     NULL, &Abtn},
    { d_jbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "B",     NULL, &Bbtn},
    { d_jbutton_proc,      22,   146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Start", NULL, &Sbtn},
    { d_jbutton_proc,      22,   178,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX1",     NULL, &Exbtn1},
    { d_jbutton_proc,      22,   210,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX3", NULL, &Exbtn3},
    { d_jbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "L",     NULL, &Lbtn},
    { d_jbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "R",     NULL, &Rbtn},
    { d_jbutton_proc,      167,  146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Map",   NULL, &Pbtn},
    { d_jbutton_proc,      167,  178,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX2",     NULL, &Exbtn2},
    { d_jbutton_proc,      167,  210,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "EX4",   NULL, &Exbtn4},
    
    { jwin_button_proc,    90,   240,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    170,  240,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG btndir_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       8,    44,   304,  172,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Joystick Directions", NULL,  NULL },
    { d_stringloader,      0,    0,    4,    0,    0,       0,       0,       0,         0,        0,       NULL, NULL, NULL },
    { jwin_frame_proc,     14,   70,   147,  80,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_frame_proc,     159,  70,   147,  80,  0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_text_proc,         30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Vertical", NULL,  NULL },
    { jwin_text_proc,         175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Horizontal", NULL,  NULL },
    
    { jwin_text_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a, NULL,  NULL },
    { jwin_text_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b, NULL,  NULL },
    { jwin_text_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l, NULL,  NULL },
    { jwin_text_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r, NULL,  NULL },
    
    { d_jbutton_proc,      22,   90,   61,   21,   vc(14),  vc(11),  0,       0,         0,        0, (void *) "Up",     NULL, &DUbtn },
    { d_jbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Down",   NULL, &DDbtn },
    { d_jbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Left",   NULL, &DLbtn },
    { d_jbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0, (void *) "Right",  NULL, &DRbtn },
    { jwin_check_proc,     22,   154,  147,  8,    vc(14),  vc(1),   0,       0,         1,        0, (void *) "Use Analog Stick/DPad", NULL, NULL },
    
    // 16
    { jwin_button_proc,    90,   184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    170,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

/*
int midi_dp[3] = {0,147,104};
int digi_dp[3] = {1,147,120};
int pan_dp[3]  = {0,147,136};
int buf_dp[3]  = {0,147,152};
*/
int midi_dp[3] = {0,0,0};
int digi_dp[3] = {1,0,0};
int emus_dp[3] = {2,0,0};
int buf_dp[3]  = {0,0,0};
int sfx_dp[3]  = {3,0,0};
int pan_dp[3]  = {0,0,0};

static DIALOG sound_dlg[] =
{
    //(dialog proc)          (x)     (y)    (w)     (h)    (fg)       (bg)                 (key) (flags)       (d1)           (d2) (dp)                           (dp2)               (dp3)
    { jwin_win_proc,          0,      0,    320,    178,    0,         0,                   0,    D_EXIT,       0,             0, (void *) "Sound Settings",      NULL,               NULL     },
    { d_stringloader,         0,      0,      3,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { jwin_button_proc,      58,    148,     61,     21,    0,         0,                   0,    D_EXIT,       0,             0, (void *) "OK",                  NULL,               NULL     },
    { jwin_button_proc,     138,    148,     61,     21,    0,         0,                   0,    D_EXIT,       0,             0, (void *) "Cancel",              NULL,               NULL     },
    { d_timer_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { jwin_frame_proc,       10,     28,    300,    112,    0,         0,                   0,    0,            FR_ETCHED,     0,  NULL,                           NULL,               NULL     },
    { jwin_rtext_proc,      190,     40,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_a,                 NULL,               NULL     },
    { jwin_rtext_proc,      190,     56,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_b,                 NULL,               NULL     },
    { jwin_rtext_proc,      190,     72,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_l,                 NULL,               NULL     },
    { jwin_rtext_proc,      190,     88,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_m,                 NULL,               NULL     },
    // 10
    { jwin_rtext_proc,      190,    104,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_r,                 NULL,               NULL     },
    { jwin_rtext_proc,      190,    120,     40,      8,    vc(7),     vc(11),              0,    0,            0,             0, (void *) str_s,                 NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { jwin_slider_proc,     196,     40,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,           32,             0,  NULL, (void *) set_vol,   midi_dp  },
    { jwin_slider_proc,     196,     56,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,           32,             0,  NULL, (void *) set_vol,   digi_dp  },
    { jwin_slider_proc,     196,     72,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,           32,             0,  NULL, (void *) set_vol,   emus_dp  },
    { jwin_slider_proc,     196,     88,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,          127,             0,  NULL, (void *) set_buf,   buf_dp   },
    { jwin_slider_proc,     196,    104,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,           32,             0,  NULL, (void *) set_vol,   sfx_dp   },
    //20
    { jwin_slider_proc,     196,    120,     96,      8,    vc(0),     jwin_pal[jcBOX],     0,    0,            3,             0,  NULL, (void *) set_pan,   pan_dp   },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { jwin_text_proc,        17,     40,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "Master MIDI Volume",  NULL,               NULL     },
    { jwin_text_proc,        17,     56,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "Master Digi Volume",  NULL,               NULL     },
    { jwin_text_proc,        17,     72,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "Enhanced Music Volume",     NULL,               NULL     },
    { jwin_text_proc,        17,     88,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "Enhanced Music Buffer",     NULL,               NULL     },
    { jwin_text_proc,        17,    104,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "SFX Volume",          NULL,               NULL     },
    { jwin_text_proc,        17,    120,     48,      8,    vc(0),     vc(11),              0,    0,            0,             0, (void *) "SFX Pan",             NULL,               NULL     },
    //30
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { d_dummy_proc,           0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
    { NULL,                   0,      0,      0,      0,    0,         0,                   0,    0,            0,             0,  NULL,                           NULL,               NULL     },
};

static DIALOG about_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       68,   52,   184,  154,  0,       0,       0,       D_EXIT,    0,        0, (void *) "About", NULL,  NULL },
    { jwin_button_proc,    140,  176,  41,   21,   vc(14),  0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_ctext_proc,        160,  84,   0,    8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) ZELDA_ABOUT_STR, NULL,  NULL },
    { jwin_ctext_proc,        160,  92,   0,    8,    vc(0) ,  vc(11),  0,       0,         0,        0,       str_s, NULL,  NULL },
    { jwin_ctext_proc,        160,  100,  0,    8,    vc(0) ,  vc(11),  0,       0,         0,        0, (void *) DATE_STR, NULL,  NULL },
    { jwin_text_proc,         88,   124,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Coded by:", NULL,  NULL },
    { jwin_text_proc,         88,   132,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "  Phantom Menace", NULL,  NULL },
    { jwin_text_proc,         88,   144,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Produced by:", NULL,  NULL },
    { jwin_text_proc,         88,   152,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "  Armageddon Games", NULL,  NULL },
    { jwin_frame_proc,     80,   117,  160,  50,   0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


static DIALOG quest_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       68,   25,   184,  190,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Quest Info", NULL,  NULL },
    { jwin_edit_proc,      84,   54,   152,  16,   0,       0,       0,       D_READONLY, 100,     0,       NULL, NULL,  NULL },
    { jwin_text_proc,         89,   84,   141,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Number:", NULL,  NULL },
    { jwin_text_proc,         152,  84,   24,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a, NULL,  NULL },
    { jwin_text_proc,         89,   94,   141,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Version:", NULL,  NULL },
    { jwin_text_proc,         160,  94,   64,   8,    vc(7),   vc(11),  0,       0,         0,        0,       QHeader.version, NULL,  NULL },
    { jwin_text_proc,         89,   104,  141,  8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "ZQ Version:", NULL,  NULL },
    { jwin_text_proc,         184,  104,  64,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s, NULL,  NULL },
    { jwin_text_proc,         84,   126,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Title:", NULL,  NULL },
    { jwin_textbox_proc,   84,   136,  152,  24,   0,       0,       0,       0,         0,        0,       QHeader.title, NULL,  NULL },
    { jwin_text_proc,         84,   168,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Author:", NULL,  NULL },
    { jwin_textbox_proc,   84,   178,  152,  24,   0,       0,       0,       0,         0,        0,       QHeader.author, NULL,  NULL },
    { jwin_frame_proc,     84,   79,   152,  38,   0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG triforce_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,      72,    64,  177,  105,  vc(14),   vc(1),    0,  D_EXIT,           0,        0, (void *) "Triforce Pieces", NULL,  NULL },
    // 1
    { jwin_check_proc,   129,    94,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "1", NULL,  NULL },
    { jwin_check_proc,   129,   104,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "2", NULL,  NULL },
    { jwin_check_proc,   129,   114,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "3", NULL,  NULL },
    { jwin_check_proc,   129,   124,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "4", NULL,  NULL },
    { jwin_check_proc,   172,    94,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "5", NULL,  NULL },
    { jwin_check_proc,   172,   104,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "6", NULL,  NULL },
    { jwin_check_proc,   172,   114,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "7", NULL,  NULL },
    { jwin_check_proc,   172,   124,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0, (void *) "8", NULL,  NULL },
    // 9
    { jwin_button_proc,  90,    144,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0, (void *) "O&K", NULL,  NULL },
    { jwin_button_proc,  170,   144,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

/*static DIALOG equip_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      16,    18,  289,  215,  vc(14),   vc(1),    0,  D_EXIT,           0,        0,     (void *) "Equipment", NULL,  NULL },
  // 1
  { jwin_button_proc,   90,   206,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0,     (void *) "O&K", NULL,  NULL },
  { jwin_button_proc,  170,   206,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0,     (void *) "Cancel", NULL,  NULL },
  // 3
  { jwin_frame_proc,    25,    45,   77,   50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        29,    42,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Sword", NULL,  NULL },
  { jwin_check_proc,    33,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden", NULL,  NULL },
  { jwin_check_proc,    33,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "White", NULL,  NULL },
  { jwin_check_proc,    33,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic", NULL,  NULL },
  { jwin_check_proc,    33,    82,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Master", NULL,  NULL },
  // 9
  { jwin_frame_proc,    25,    99,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        29,    96,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Shield", NULL,  NULL },
  { jwin_check_proc,    33,   106,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small", NULL,  NULL },
  { jwin_check_proc,    33,   116,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic", NULL,  NULL },
  { jwin_check_proc,    33,   126,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Mirror", NULL,  NULL },
  // 14
  { jwin_frame_proc,    25,   143,   61,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        29,   140,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Ring", NULL,  NULL },
  { jwin_check_proc,    33,   150,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue", NULL,  NULL },
  { jwin_check_proc,    33,   160,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red", NULL,  NULL },
  { jwin_check_proc,    33,   170,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Gold", NULL,  NULL },
  // 19
  { jwin_frame_proc,   110,    45,   85,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       114,    42,   64,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bracelet", NULL,  NULL },
  { jwin_check_proc,   118,    52,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1", NULL,  NULL },
  { jwin_check_proc,   118,    62,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2", NULL,  NULL },
  // 23
  { jwin_frame_proc,   110,    79,   85,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       114,    76,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Amulet", NULL,  NULL },
  { jwin_check_proc,   118,    86,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1", NULL,  NULL },
  { jwin_check_proc,   118,    96,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2", NULL,  NULL },
  // 27
  { jwin_frame_proc,   110,   113,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       114,   110,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Wallet", NULL,  NULL },
  { jwin_check_proc,   118,   120,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small", NULL,  NULL },
  { jwin_check_proc,   118,   130,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large", NULL,  NULL },
  // 31
  { jwin_frame_proc,   110,   147,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       114,   144,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bow", NULL,  NULL },
  { jwin_check_proc,   118,   154,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small", NULL,  NULL },
  { jwin_check_proc,   118,   164,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large", NULL,  NULL },
  // 35
  { jwin_frame_proc,   203,    45,   93,   70,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       207,    42,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Other", NULL,  NULL },
  { jwin_check_proc,   211,    52,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Raft", NULL,  NULL },
  { jwin_check_proc,   211,    62,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Ladder", NULL,  NULL },
  { jwin_check_proc,   211,    72,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Book", NULL,  NULL },
  { jwin_check_proc,   211,    82,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic Key", NULL,  NULL },
  { jwin_check_proc,   211,    92,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Flippers", NULL,  NULL },
  { jwin_check_proc,   211,   102,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Boots", NULL,  NULL },
  { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG items_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      16,    18,  289,  215,  vc(14),   vc(1),    0,  D_EXIT,           0,        0,     (void *) "Items", NULL,  NULL },
  //1
  { jwin_button_proc,   90,   206,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0,     (void *) "O&K", NULL,  NULL },
  { jwin_button_proc,  170,   206,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0,     (void *) "Cancel", NULL,  NULL },
  // 3
  { jwin_frame_proc,    27,    45,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        31,    42,   64,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Boomerang", NULL,  NULL },
  { jwin_check_proc,    35,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden", NULL,  NULL },
  { jwin_check_proc,    35,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic", NULL,  NULL },
  { jwin_check_proc,    35,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Fire", NULL,  NULL },
  // 8
  { jwin_frame_proc,    27,    89,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        31,    86,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Arrow", NULL,  NULL },
  { jwin_check_proc,    35,    96,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden", NULL,  NULL },
  { jwin_check_proc,    35,   106,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Silver", NULL,  NULL },
  { jwin_check_proc,    35,   116,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Golden", NULL,  NULL },
  // 13
  { jwin_frame_proc,    27,   133,   63,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,        31,   130,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Potion", NULL,  NULL },
  { jwin_radio_proc,    35,   140,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None", NULL,  NULL },
  { jwin_radio_proc,    35,   150,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue", NULL,  NULL },
  { jwin_radio_proc,    35,   160,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red", NULL,  NULL },
  // 18
  { jwin_frame_proc,   114,    45,   93,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       118,    42,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Whistle", NULL,  NULL },
  { jwin_check_proc,   122,    52,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Recorder", NULL,  NULL },
  // 21
  { jwin_frame_proc,   114,    69,   86,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       118,    66,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Hammer", NULL,  NULL },
  { jwin_check_proc,   122,    76,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1", NULL,  NULL },
  // 24
  { jwin_frame_proc,   114,    93,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       118,    90,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Hookshot", NULL,  NULL },
  { jwin_check_proc,   122,   100,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Short", NULL,  NULL },
  { jwin_check_proc,   122,   110,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Long", NULL,  NULL },
  // 28
  { jwin_frame_proc,   114,   127,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       118,   124,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Candle", NULL,  NULL },
  { jwin_check_proc,   122,   134,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue", NULL,  NULL },
  { jwin_check_proc,   122,   144,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red", NULL,  NULL },
  // 32
  { jwin_frame_proc,   217,    45,   77,  138,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       221,    42,   80,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Other", NULL,  NULL },
  { jwin_check_proc,   225,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Bait", NULL,  NULL },
  { jwin_check_proc,   225,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Letter", NULL,  NULL },
  { jwin_check_proc,   225,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wand", NULL,  NULL },
  { jwin_check_proc,   225,    82,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Lens", NULL,  NULL },
  { jwin_check_proc,   225,    92,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Din's Fire", NULL,  NULL },
  { jwin_check_proc,   225,   102,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Farore's Wind", NULL,  NULL },
  { jwin_check_proc,   225,   112,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Nayru's Love", NULL,  NULL },
  { jwin_text_proc,       225,   132,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bombs:", NULL,  NULL },
  { jwin_edit_proc,    229,   142,   40,   16,       0,       0,    0,       0,           6,        0,     NULL, NULL,  NULL },
  { jwin_text_proc,       225,   162,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "S-Bombs:", NULL,  NULL },
  { jwin_edit_proc,    229,   162,   40,   16,       0,       0,    0,       0,           6,        0,     NULL, NULL,  NULL },
  { jwin_check_proc,   225,   122,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Cane of Byrna", NULL,  NULL },
  //45
  { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};*/

static DIALOG credits_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       40,   38,   241,  173,  vc(14),  vc(1),   0,       D_EXIT,    0,        0, (void *) "Zelda Classic Credits", NULL,  NULL },
    { jwin_frame_proc,     47,   65,   227,  115,  vc(15),  vc(1),   0,       0,         FR_DEEP,  0,       NULL, NULL,  NULL },
    { d_bitmap_proc,       49,   67,   222,  110,  vc(15),  vc(1),   0,       0,         0,        0,       NULL, NULL,  NULL },
    { jwin_button_proc,    140,  184,  41,   21,   vc(14),  vc(1),   0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static ListData dmap_list(dmaplist, &font);

static DIALOG goto_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)                     (dp2) (dp3) */
    { jwin_win_proc,       48,   25,   205,  100,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Goto Location", NULL,  NULL },
    { jwin_button_proc,    90,   176-78,  61,   21,   vc(14),  0,       13,     D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    170,  176-78,  61,   21,   vc(14),  0,       27,     D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { jwin_text_proc,      55,   129-75,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "DMap:", NULL,  NULL },
    { jwin_droplist_proc,      88,  126-75,  160,  16,   0,       0,       0,       0,         0,        0, (void *) &dmap_list, NULL,  NULL },
    { jwin_text_proc,      55,   149-75,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Screen:", NULL,  NULL },
    { jwin_edit_proc,      132,  146-75,  91,   16,   0,       0,       0,       0,         2,        0,       NULL, NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int xtoi(char *hexstr)
{
    int val=0;
    
    while(isxdigit(*hexstr))
    {
        val<<=4;
        
        if(*hexstr<='9')
            val += *hexstr-'0';
        else val+= ((*hexstr)|0x20)-'a'+10;
        
        ++hexstr;
    }
    
    return val;
}

int onGoTo()
{
    bool music = false;
    music = music;
    sprintf(cheat_goto_screen_str,"%X",cheat_goto_screen);
    
    goto_dlg[0].dp2=lfont;
    goto_dlg[4].d2=cheat_goto_dmap;
    goto_dlg[6].dp=cheat_goto_screen_str;
    
    clear_keybuf();
    
    if(is_large)
        large_dialog(goto_dlg);
        
    if(zc_popup_dialog(goto_dlg,4)==1)
    {
        cheat_goto_dmap=goto_dlg[4].d2;
        cheat_goto_screen=zc_min(xtoi(cheat_goto_screen_str),0x7F);
        do_cheat_goto=true;
    };
    
    return D_O_K;
}

int onGoToComplete()
{
    if(!Playing)
    {
        return D_O_K;
    }
    
    system_pal();
    music_pause();
    pause_all_sfx();
    show_mouse(screen);
    onGoTo();
    eat_buttons();
    
    if(key[KEY_ESC])
        key[KEY_ESC]=0;
        
    show_mouse(NULL);
    game_pal();
    music_resume();
    resume_all_sfx();
    return D_O_K;
}

int onCredits()
{
    go();
    
    BITMAP *win = create_bitmap_ex(8,222,110);
    
    if(!win)
        return D_O_K;
        
    int c=0;
    int l=0;
    int ol=-1;
    RLE_SPRITE *rle = (RLE_SPRITE*)(data[RLE_CREDITS].dat);
    RGB *pal = (RGB*)(data[PAL_CREDITS].dat);
    PALETTE tmppal;
    
    clear_bitmap(win);
    draw_rle_sprite(win,rle,0,0);
    credits_dlg[0].dp2=lfont;
    credits_dlg[1].fg = gui_mg_color;
    credits_dlg[2].dp = win;
    set_palette_range(black_palette,0,127,false);
    
    DIALOG_PLAYER *p = init_dialog(credits_dlg,3);
    
    while(update_dialog(p))
    {
        throttleFPS();
        ++c;
        l = zc_max((c>>1)-30,0);
        
        if(l > rle->h)
            l = c = 0;
            
        if(l > rle->h - 112)
            l = rle->h - 112;
            
        clear_bitmap(win);
        draw_rle_sprite(win,rle,0,0-l);
        
        if(c<=64)
            fade_interpolate(black_palette,pal,tmppal,c,0,127);
            
        set_palette_range(tmppal,0,127,false);
        
        if(l!=ol)
        {
            scare_mouse();
            d_bitmap_proc(MSG_DRAW,credits_dlg+2,0);
            unscare_mouse();
            SCRFIX();
            ol=l;
        }
    }
    
    shutdown_dialog(p);
    destroy_bitmap(win);
    comeback();
    return D_O_K;
}

const char *midilist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size=0;
        
        for(int i=0; i<MAXMIDIS; i++)
            if(tunes[i].data)
                ++(*list_size);
                
        return NULL;
    }
    
    int i=0,m=0;
    
    while(m<=index && i<=MAXMIDIS)
    {
        if(tunes[i].data)
            ++m;
            
        ++i;
    }
    
    --i;
    
    if(i==MAXMIDIS && m<index)
        return "(null)";
        
    return tunes[i].title;
}

/*  ------- MIDI info stuff -------- */

char *text;
midi_info *zmi;
bool dialog_running;
bool listening;

void get_info(int index);

int d_midilist_proc(int msg,DIALOG *d,int c)
{
    int d2 = d->d2;
    int ret = jwin_droplist_proc(msg,d,c);
    
    if(d2!=d->d2)
    {
        get_info(d->d2);
    }
    
    return ret;
}

int d_listen_proc(int msg,DIALOG *d,int c)
{
    /* 'd->d1' is offset from 'd' in DIALOG array to midilist proc */
    
    int ret = jwin_button_proc(msg,d,c);
    
    if(ret == D_CLOSE)
    {
        // get current midi index
        int index = (d+(d->d1))->d2;
        int i=0, m=0;
        
        while(m<=index && i<=MAXMIDIS)
        {
            if(tunes[i].data)
                ++m;
                
            ++i;
        }
        
        --i;
        jukebox(i);
        listening = true;
        ret = D_O_K;
    }
    
    return ret;
}

int d_savemidi_proc(int msg,DIALOG *d,int c)
{
    /* 'd->d1' is offset from 'd' in DIALOG array to midilist proc */
    
    int ret = jwin_button_proc(msg,d,c);
    
    if(ret == D_CLOSE)
    {
        // get current midi index
        int index = (d+(d->d1))->d2;
        int i=0, m=0;
        
        while(m<=index && i<=MAXMIDIS)
        {
            if(tunes[i].data)
                ++m;
                
            ++i;
        }
        
        --i;
        
        // get file name
        
        int  sel=0;
        //struct ffblk f;
        char title[40] = "Save MIDI: ";
        static char fname[2048] = "";
        static EXT_LIST list[] =
        {
            { (char *)"MIDI files (*.mid)", (char *)"mid" },
            { (char *)"HTML files (*.html, *.html)", (char *)"htm html" },
            { NULL,                                  NULL }
        };
        
        strcpy(title+11, tunes[i].title);
        
        if(jwin_file_browse_ex(title, fname, list, &sel, 2048, -1, -1, lfont)==0)
            goto done;
            
        if(exists(fname))
        {
            if(jwin_alert(title, fname, "already exists.", "Overwrite it?", "&Yes","&No",'y','n',lfont)==2)
                goto done;
        }
        
        // save midi i
        
        if(save_midi(fname, (MIDI*)tunes[i].data) != 0)
            jwin_alert(title, "Error saving MIDI to", fname, NULL, "Darn", NULL,13,27,lfont);
            
done:
        chop_path(fname);
        ret = D_REDRAW;
    }
    
    return ret;
}

static ListData midi_list(midilist, &font);

static DIALOG midi_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
    { jwin_win_proc,       8,    28,   304,  184,  0,       0,        0,       D_EXIT,    0,        0, (void *) "MIDI Info", NULL,  NULL },
    { jwin_text_proc,         32,   60,   40,   8,    vc(0),   vc(11),   0,       0,         0,        0, (void *) "Tune:", NULL,  NULL },
    { d_midilist_proc,     80,   56,   192,  16,   0,       0,        0,       0,         0,        0, (void *) &midi_list, NULL,  NULL },
    { jwin_textbox_proc,   15,   80,   290,  96,   0,       0,        0,       0,         0,        0,       NULL, NULL,  NULL },
    { d_listen_proc,       24,   183,  72,   21,   0,       0,        'l',     D_EXIT,    -2,       0, (void *) "&Listen", NULL,  NULL },
    { d_savemidi_proc,     108,  183,  72,   21,   0,       0,        's',     D_EXIT,    -3,       0, (void *) "&Save", NULL,  NULL },
    { jwin_button_proc,    236,  183,  61,   21,   0,       0,        'k',     D_EXIT,    0,        0, (void *) "O&K", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void get_info(int index)
{
    int i=0, m=0;
    
    while(m<=index && i<=MAXMIDIS)
    {
        if(tunes[i].data)
            ++m;
            
        ++i;
    }
    
    --i;
    
    if(i==MAXMIDIS && m<index)
        strcpy(text,"(null)");
    else
    {
        get_midi_info((MIDI*)tunes[i].data,zmi);
        get_midi_text((MIDI*)tunes[i].data,zmi,text);
    }
    
    midi_dlg[0].dp2=lfont;
    midi_dlg[3].dp = text;
    midi_dlg[3].d1 = midi_dlg[3].d2 = 0;
    midi_dlg[5].flags = (tunes[i].flags&tfDISABLESAVE) ? D_DISABLED : D_EXIT;
    
    if(dialog_running)
    {
        scare_mouse();
        jwin_textbox_proc(MSG_DRAW,midi_dlg+3,0);
        d_savemidi_proc(MSG_DRAW,midi_dlg+5,0);
        unscare_mouse();
    }
}

int onMIDICredits()
{
    text = (char*)zc_malloc(4096);
    zmi = (midi_info*)zc_malloc(sizeof(midi_info));
    
    if(!text || !zmi)
    {
        jwin_alert(NULL,"Not enough memory",NULL,NULL,"OK",NULL,13,27,lfont);
        return D_O_K;
    }
    
    midi_dlg[0].dp2=lfont;
    midi_dlg[2].d1 = 0;
    midi_dlg[2].d2 = 0;
    midi_dlg[4].flags = (midi_pos >= 0) ? D_DISABLED : D_EXIT;
    midi_dlg[5].flags = (tunes[midi_dlg[2].d1].flags&tfDISABLESAVE) ? D_DISABLED : D_EXIT;
    
    listening = false;
    dialog_running=false;
    get_info(0);
    
    dialog_running=true;
    
    if(is_large)
        large_dialog(midi_dlg);
        
    zc_popup_dialog(midi_dlg,0);
    dialog_running=false;
    
    if(listening)
        music_stop();
        
    zc_free(text);
    zc_free(zmi);
    return D_O_K;
}

int onAbout()
{
    switch(IS_BETA)
    {
    case 1:
        sprintf(str_s,"(%s Beta Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
        
    case -1:
        sprintf(str_s,"(%s Alpha Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
        
    case 0:
    default:
        sprintf(str_s,"(%s Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
    }
    
    //  sprintf(str_s,"%s",VerStr(ZELDA_VERSION));
    about_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(about_dlg);
        
    zc_popup_dialog(about_dlg,1);
    return D_O_K;
}

int onQuest()
{
    char fname[100];
    strcpy(fname, get_filename(qstpath));
    quest_dlg[0].dp2=lfont;
    quest_dlg[1].dp = fname;
    
    if(QHeader.quest_number==0)
        sprintf(str_a,"Custom");
    else
        sprintf(str_a,"%d",QHeader.quest_number);
        
    sprintf(str_s,"%s",VerStr(QHeader.zelda_version));
    
    quest_dlg[11].d1 = quest_dlg[9].d1 = 0;
    quest_dlg[11].d2 = quest_dlg[9].d2 = 0;
    
    if(is_large)
        large_dialog(quest_dlg);
        
    zc_popup_dialog(quest_dlg, 0);
    return D_O_K;
}

int onVidMode()
{
    int VidMode=gfx_driver->id;
#ifdef ALLEGRO_DOS
    
    switch(VidMode)
    {
    case GFX_MODEX:
        sprintf(str_a,"VGA Mode X");
        break;
        
    case GFX_VESA1:
        sprintf(str_a,"VESA 1.x");
        break;
        
    case GFX_VESA2B:
        sprintf(str_a,"VESA2 Banked");
        break;
        
    case GFX_VESA2L:
        sprintf(str_a,"VESA2 Linear");
        break;
        
    case GFX_VESA3:
        sprintf(str_a,"VESA3");
        break;
        
    default:
        sprintf(str_a,"Unknown... ?");
        break;
    }
    
#elif defined(ALLEGRO_WINDOWS)
    
    switch(VidMode)
    {
    case GFX_DIRECTX:
        sprintf(str_a,"DirectX Hardware Accelerated");
        break;
    
    case GFX_DIRECTX_SOFT:
        sprintf(str_a,"DirectX Software Accelerated");
        break;
    
    case GFX_DIRECTX_SAFE:
        sprintf(str_a,"DirectX Safe");
        break;
    
    case GFX_DIRECTX_WIN:
        sprintf(str_a,"DirectX Windowed");
        break;
    
    case GFX_GDI:
        sprintf(str_a,"GDI");
        break;
    
    default:
        sprintf(str_a,"Unknown... ?");
        break;
    }
    
#elif defined(ALLEGRO_MACOSX)
    
    switch(VidMode)
    {
    case GFX_SAFE:
        sprintf(str_a,"MacOS X Safe");
        break;
    
    case GFX_QUARTZ_FULLSCREEN:
        sprintf(str_a,"MacOS X Fullscreen Quartz");
        break;
    
    case GFX_QUARTZ_WINDOW:
        sprintf(str_a,"MacOS X Windowed Quartz");
        break;
    
    default:
        sprintf(str_a,"Unknown... ?");
        break;
    }
    
#elif defined(ALLEGRO_LINUX)
    
    switch(VidMode)
    {
    case GFX_AUTODETECT_WINDOWED:
        sprintf(str_a,"Autodetect Windowed");
        break;
    
    default:
        sprintf(str_a,"Unknown... ?");
        break;
    }
    
#endif
    
    sprintf(str_b,"%dx%d 8-bit",resx,resy);
    jwin_alert("Video Mode",str_a,str_b,NULL,"OK",NULL,13,27,lfont);
    return D_O_K;
}

#define addToHash(c,b,h) if(h->find(c ## key) == h->end()) \
{(*h)[c ## key]=true;} else {b = false;}

int onKeyboard()
{
    int a = Akey;
    int b = Bkey;
    int s = Skey;
    int l = Lkey;
    int r = Rkey;
    int p = Pkey;
    int ex1 = Exkey1;
    int ex2 = Exkey2;
    int ex3 = Exkey3;
    int ex4 = Exkey4;
    bool done=false;
    int ret;
    
    key_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(key_dlg);
        
    while(!done)
    {
        ret = zc_popup_dialog(key_dlg,27);
        
        if(ret==27) // OK
        {
            std::map<int,bool> *keyhash = new std::map<int,bool>();
            bool unique = true;
            addToHash(A,unique,keyhash);
            addToHash(B,unique,keyhash);
            addToHash(S,unique,keyhash);
            addToHash(L,unique,keyhash);
            addToHash(R,unique,keyhash);
            addToHash(P,unique,keyhash);
            addToHash(DU,unique,keyhash);
            addToHash(DD,unique,keyhash);
            addToHash(DL,unique,keyhash);
            addToHash(DR,unique,keyhash);
            
            if(keyhash->find(Exkey1) == keyhash->end())
            {
                (*keyhash)[Exkey1]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey2) == keyhash->end())
            {
                (*keyhash)[Exkey2]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey3) == keyhash->end())
            {
                (*keyhash)[Exkey3]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey4) == keyhash->end())
            {
                (*keyhash)[Exkey4]=true;
            }
            else
            {
                unique = false;
            }
            
            delete keyhash;
            
            if(unique)
                done=true;
            else
                jwin_alert("Error", "Key bindings must be unique!", "", "", "OK",NULL,'o',0,lfont);
        }
        else // Cancel
        {
            Akey = a;
            Bkey = b;
            Skey = s;
            Lkey = l;
            Rkey = r;
            Pkey = p;
            Exkey1 = ex1;
            Exkey2 = ex2;
            Exkey3 = ex3;
            Exkey4 = ex4;
            done=true;
        }
    }
    
    save_game_configs();
    return D_O_K;
}

int onKeyboardDir()
{
    int u = DUkey;
    int d = DDkey;
    int l = DLkey;
    int r = DRkey;
    
    keydir_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(keydir_dlg);
        
    bool done=false;
    int ret;
    
    key_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(key_dlg);
        
    while(!done)
    {
        ret = zc_popup_dialog(keydir_dlg,14);
        
        if(ret==14) // OK
        {
            std::map<int,bool> *keyhash = new std::map<int,bool>();
            bool unique = true;
            addToHash(A,unique,keyhash);
            addToHash(B,unique,keyhash);
            addToHash(S,unique,keyhash);
            addToHash(L,unique,keyhash);
            addToHash(R,unique,keyhash);
            addToHash(P,unique,keyhash);
            addToHash(DU,unique,keyhash);
            addToHash(DD,unique,keyhash);
            addToHash(DL,unique,keyhash);
            addToHash(DR,unique,keyhash);
            
            if(keyhash->find(Exkey1) == keyhash->end())
            {
                (*keyhash)[Exkey1]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey2) == keyhash->end())
            {
                (*keyhash)[Exkey2]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey3) == keyhash->end())
            {
                (*keyhash)[Exkey3]=true;
            }
            else
            {
                unique = false;
            }
            
            if(keyhash->find(Exkey4) == keyhash->end())
            {
                (*keyhash)[Exkey4]=true;
            }
            else
            {
                unique = false;
            }
            
            delete keyhash;
            
            if(unique)
                done=true;
            else
                jwin_alert("Error", "Key bindings must be unique!", "", "", "OK",NULL,'o',0,lfont);
        }
        else // Cancel
        {
            DUkey = u;
            DDkey = d;
            DLkey = l;
            DRkey = r;
            done=true;
        }
    }
    
    save_game_configs();
    return D_O_K;
}

int onJoystick()
{
    int a = Abtn;
    int b = Bbtn;
    int s = Sbtn;
    int l = Lbtn;
    int r = Rbtn;
    int m = Mbtn;
    int p = Pbtn;
    int ex1 = Exbtn1;
    int ex2 = Exbtn2;
    int ex3 = Exbtn3;
    int ex4 = Exbtn4;
    
    btn_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(btn_dlg);
        
    int ret = zc_popup_dialog(btn_dlg,27);
    
    // not OK'd
    if(ret != 27)
    {
        Abtn = a;
        Bbtn = b;
        Sbtn = s;
        Lbtn = l;
        Rbtn = r;
        Mbtn = m;
        Pbtn = p;
        Exbtn1 = ex1;
        Exbtn2 = ex2;
        Exbtn3 = ex3;
        Exbtn4 = ex4;
    }
    
    save_game_configs();
    return D_O_K;
}

int onJoystickDir()
{
    int up = DUbtn;
    int down = DDbtn;
    int left = DLbtn;
    int right = DRbtn;
    
    btndir_dlg[0].dp2=lfont;
    if(analog_movement)
        btndir_dlg[14].flags|=D_SELECTED;
    else
        btndir_dlg[14].flags&=~D_SELECTED;
    
    if(is_large)
        large_dialog(btndir_dlg);
        
    int ret = zc_popup_dialog(btndir_dlg,15);
    
    if(ret==15) // OK
        analog_movement = btndir_dlg[14].flags&D_SELECTED;
    else // Cancel
    {
        DUbtn = up;
        DDbtn = down;
        DLbtn = left;
        DRbtn = right;
    }
    
    save_game_configs();
    return D_O_K;
}

int onSound()
{
	//if out of beta, we cmight want to clear the settings from scripts:
	#ifndef IS_BETA
	
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
		master_volume(-1,((long)FFCore.usr_midi_volume));
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
		master_volume((long)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
		emusic_volume = (long)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
		sfx_volume = (long)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (long)FFCore.usr_panstyle;
	}
    #endif
    int m = midi_volume;
    int d = digi_volume;
    int e = emusic_volume;
    int b = zcmusic_bufsz;
    int s = sfx_volume;
    int p = pan_style;
    pan_style = vbound(pan_style,0,3);
    
    sound_dlg[0].dp2=lfont;
    
    if(is_large)
        large_dialog(sound_dlg);
        
    midi_dp[1] = sound_dlg[6].x;
    midi_dp[2] = sound_dlg[6].y;
    digi_dp[1] = sound_dlg[7].x;
    digi_dp[2] = sound_dlg[7].y;
    emus_dp[1] = sound_dlg[8].x;
    emus_dp[2] = sound_dlg[8].y;
    buf_dp[1]  = sound_dlg[9].x;
    buf_dp[2]  = sound_dlg[9].y;
    sfx_dp[1]  = sound_dlg[10].x;
    sfx_dp[2]  = sound_dlg[10].y;
    pan_dp[1]  = sound_dlg[11].x;
    pan_dp[2]  = sound_dlg[11].y;
    sound_dlg[15].d2 = (midi_volume==255) ? 32 : midi_volume>>3;
    sound_dlg[16].d2 = (digi_volume==255) ? 32 : digi_volume>>3;
    sound_dlg[17].d2 = (emusic_volume==255) ? 32 : emusic_volume>>3;
    sound_dlg[18].d2 = zcmusic_bufsz;
    sound_dlg[19].d2 = (sfx_volume==255) ? 32 : sfx_volume>>3;
    sound_dlg[20].d2 = pan_style;
    
    int ret = zc_popup_dialog(sound_dlg,1);
    
    if(ret==2)
    {
        master_volume(digi_volume,midi_volume);
        
        for(int i=0; i<WAV_COUNT; ++i)
        {
            //allegro assertion fails when passing in -1 as voice -DD
            if(sfx_voice[i] > 0)
                voice_set_volume(sfx_voice[i], sfx_volume);
        }
    }
    else
    {
        midi_volume   = m;
        digi_volume   = d;
        emusic_volume = e;
        zcmusic_bufsz = b;
        sfx_volume    = s;
        pan_style     = p;
    }
    
    return D_O_K;
}

int queding(char const* s1, char const* s2, char const* s3)
{
    return jwin_alert(ZC_str,s1,s2,s3,"&Yes","&No",'y','n',lfont);
}

int onQuit()
{
    if(Playing)
    {
        int ret=0;
        
        if(get_bit(quest_rules, qr_NOCONTINUE))
        {
            if(standalone_mode)
            {
                ret=queding("End current game?",
                            "The continue screen is disabled; the game",
                            "will be reloaded from the last save.");
            }
            else
            {
                ret=queding("End current game?",
                            "The continue screen is disabled. You will",
                            "be returned to the file select screen.");
            }
        }
        else
            ret=queding("End current game?",NULL,NULL);
            
        if(ret==1)
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

int onReset()
{
    if(queding("  Reset system?  ",NULL,NULL)==1)
    {
        disableClickToFreeze=false;
        Quit=qRESET;
        return D_CLOSE;
    }
    
    return D_O_K;
}

int onExit()
{
    if(queding(" Quit Zelda Classic? ",NULL,NULL)==1)
    {
        Quit=qEXIT;
        return D_CLOSE;
    }
    
    return D_O_K;
}

int onTitle_NES()
{
    title_version=0;
    return D_O_K;
}
int onTitle_DX()
{
    title_version=1;
    return D_O_K;
}
int onTitle_25()
{
    title_version=2;
    return D_O_K;
}

int onDebug()
{
    if(debug_enabled)
        set_debug(!get_debug());
        
    return D_O_K;
}

int onHeartBeep()
{
    heart_beep=!heart_beep;
    return D_O_K;
}

int onSaveIndicator()
{
    use_save_indicator=!use_save_indicator;
    return D_O_K;
}

int onTriforce()
{
    for(int i=0; i<MAXINITTABS; ++i)
    {
        init_tabs[i].flags&=~D_SELECTED;
    }
    
    init_tabs[3].flags=D_SELECTED;
    return onCheatConsole();
    /*triforce_dlg[0].dp2=lfont;
    for(int i=1; i<=8; i++)
      triforce_dlg[i].flags = (game->lvlitems[i] & liTRIFORCE) ? D_SELECTED : 0;
    
    if(zc_popup_dialog (triforce_dlg,-1)==9)
    {
      for(int i=1; i<=8; i++)
      {
        game->lvlitems[i] &= ~liTRIFORCE;
        game->lvlitems[i] |= (triforce_dlg[i].flags & D_SELECTED) ? liTRIFORCE : 0;
      }
    }
    return D_O_K;*/
}

bool rc = false;
/*
int onEquipment()
{
  for (int i=0; i<MAXINITTABS; ++i)
  {
    init_tabs[i].flags&=~D_SELECTED;
  }
  init_tabs[0].flags=D_SELECTED;
  return onCheatConsole();
}
*/

int onItems()
{
    for(int i=0; i<MAXINITTABS; ++i)
    {
        init_tabs[i].flags&=~D_SELECTED;
    }
    
    init_tabs[1].flags=D_SELECTED;
    return onCheatConsole();
}

static DIALOG getnum_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        80,   80,     160,    72,   vc(0),  vc(11),  0,       D_EXIT,     0,             0,       NULL, NULL,  NULL },
    { jwin_text_proc,          104,  104+4,  48,     8,    vc(0),  vc(11),  0,       0,          0,             0, (void *) "Number:", NULL,  NULL },
    { jwin_edit_proc,       168,  104,    48,     16,    0,     0,       0,       0,          6,             0,       NULL, NULL,  NULL },
    { jwin_button_proc,     90,   126,    61,     21,   vc(0),  vc(11),  13,      D_EXIT,     0,             0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,     170,  126,    61,     21,   vc(0),  vc(11),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int getnumber(const char *prompt,int initialval)
{
    char buf[20];
    sprintf(buf,"%d",initialval);
    getnum_dlg[0].dp=(void *)prompt;
    getnum_dlg[0].dp2=lfont;
    getnum_dlg[2].dp=buf;
    
    if(is_large)
        large_dialog(getnum_dlg);
        
    if(zc_popup_dialog(getnum_dlg,2)==3)
        return atoi(buf);
        
    return initialval;
}

int onLife()
{
    //this used to be a 0 instead of a 1.
    game->set_life(vbound(getnumber("Life",game->get_life()),1,game->get_maxlife()));
    return D_O_K;
}

int onHeartC()
{
    game->set_maxlife(vbound(getnumber("Heart Containers",game->get_maxlife()/HP_PER_HEART),1,4095) * HP_PER_HEART);
    game->set_life(vbound(getnumber("Life",game->get_life()/HP_PER_HEART),1,game->get_maxlife()/HP_PER_HEART)*HP_PER_HEART);
    return D_O_K;
}

int onMagicC()
{
    game->set_maxmagic(vbound(getnumber("Magic Containers",game->get_maxmagic()/MAGICPERBLOCK),0,2047) * MAGICPERBLOCK);
    game->set_magic(vbound(getnumber("Magic",game->get_magic()/MAGICPERBLOCK),0,game->get_maxmagic()/MAGICPERBLOCK)*MAGICPERBLOCK);
    return D_O_K;
}

int onRupies()
{
    game->set_rupies(vbound(getnumber("Rupees",game->get_rupies()),0,game->get_maxcounter(1)));
    return D_O_K;
}

int onMaxBombs()
{
    game->set_maxbombs(vbound(getnumber("Max Bombs",game->get_maxbombs()),0,0xFFFF));
    game->set_bombs(game->get_maxbombs());
    return D_O_K;
}

int onRefillLife()
{
    game->set_life(game->get_maxlife());
    return D_O_K;
}
int onRefillMagic()
{
    game->set_magic(game->get_maxmagic());
    return D_O_K;
}
int onClock()
{
    setClock(!getClock());
    return D_O_K;
}

int onQstPath()
{
    char path[2048];
    
    chop_path(qstdir);
    strcpy(path,qstdir);
    
    go();
    
    if(jwin_dfile_select_ex("Quest File Directory", path, "qst", 2048, -1, -1, lfont))
    {
        chop_path(path);
        fix_filename_case(path);
        fix_filename_slashes(path);
        strcpy(qstdir,path);
        strcpy(qstpath,qstdir);
    }
    
    comeback();
    return D_O_K;
}

static DIALOG cheat_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp) */
    { jwin_win_proc,       72,   72,   176,  96,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Enter cheat code", NULL,  NULL },
    { jwin_edit_proc,      88,   104,  144,  16,   0,       0,       0,       0,         40,       0,       NULL, NULL,  NULL },
    { jwin_button_proc,    130,  136,  61,   21,   0,       0,       13,      D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int onCheat()
{
    if(!zcheats.flags && !get_debug())
        return D_O_K;
        
    str_a[0]=0;
    cheat_dlg[0].dp2=lfont;
    cheat_dlg[1].dp=str_a;
    
    if(is_large)
        large_dialog(cheat_dlg);
        
    int ret=zc_popup_dialog(cheat_dlg,1);
    
    if((ret==2) && strlen(str_a))
    {
        char str[80];
        
        for(int i=0; i<4; i++)
        {
            if(!strcmp(str_a, zcheats.codes[i]))
            {
                cheat   = i + 1;
                game->set_cheat(game->get_cheat() | cheat);
                sprintf(str, "Cheat level %d enabled", i+1);
                jwin_alert("Cheats enabled",NULL,str,NULL,"OK",NULL,13,27,lfont);
                goto done;
            }
        }
        
        jwin_alert("ZQuest",NULL,"Invalid cheat code",NULL,"OK",NULL,13,27,lfont);
    }
    
done:
    return D_O_K;
}

int onCheatRupies()
{
    game->set_drupy(game->get_maxcounter(1));
    return D_O_K;
}

int onCheatArrows()
{
    game->set_arrows(game->get_maxarrows());
    return D_O_K;
}

int onCheatBombs()
{
    //getitem(iBombs,true);
    for(int i=0; i<MAXITEMS; i++)
    {
        if(itemsbuf[i].family == itype_bomb
                || itemsbuf[i].family == itype_sbomb)
            getitem(i, true);
    }
    
    game->set_bombs(game->get_maxbombs());
    game->set_sbombs(game->get_maxcounter(6));
    return D_O_K;
}

// *** screen saver

int after_time()
{
	if(ss_enable == 0)
		return INT_MAX;

    if(ss_after <= 0)
        return 5 * 60;
        
    if(ss_after <= 3)
        return ss_after * 15 * 60;
        
    if(ss_after <= 13)
        return (ss_after - 3) * 60 * 60;
        
    return MAX_IDLE + 1;
}

static const char *after_str[15] =
{
    " 5 sec", "15 sec", "30 sec", "45 sec", " 1 min", " 2 min", " 3 min",
    " 4 min", " 5 min", " 6 min", " 7 min", " 8 min", " 9 min", "10 min",
    "Never"
};

const char *after_list(int index, int *list_size)
{
    if(index < 0)
    {
        *list_size = 15;
        return NULL;
    }
    
    return after_str[index];
}

static ListData after__list(after_list, &font);

static DIALOG scrsaver_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)  (dp2)    (dp3) */
    { jwin_win_proc,       32,   64,   256,  136,  0,       0,       0,       D_EXIT,    0,        0, (void *) "Screen Saver Settings", NULL,  NULL },
    { jwin_frame_proc,     42,   92,   236,  70,   0,       0,       0,       0,         FR_ETCHED,0,       NULL, NULL,  NULL },
    { jwin_text_proc,         60,   104,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Run After", NULL,  NULL },
    { jwin_text_proc,         60,   128,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Speed", NULL,  NULL },
    { jwin_text_proc,         60,   144,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0, (void *) "Density", NULL,  NULL },
    { jwin_droplist_proc,  144,  100,  96,   16,   0,       0,       0,       0,         0,        0, (void *) &after__list, NULL,  NULL },
    { jwin_slider_proc,    144,  128,  116,  8,    vc(0),   jwin_pal[jcBOX],  0,       0,         6,        0,       NULL, NULL,  NULL },
    { jwin_slider_proc,    144,  144,  116,  8,    vc(0),   jwin_pal[jcBOX],  0,       0,         6,        0,       NULL, NULL,  NULL },
    { jwin_button_proc,    42,   170,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "OK", NULL,  NULL },
    { jwin_button_proc,    124,  170,  72,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Preview", NULL,  NULL },
    { jwin_button_proc,    218,  170,  61,   21,   0,       0,       0,       D_EXIT,    0,        0, (void *) "Cancel", NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int onScreenSaver()
{
    scrsaver_dlg[0].dp2=lfont;
    scrsaver_dlg[5].d1 = scrsaver_dlg[5].d2 = ss_after;
    scrsaver_dlg[6].d2 = ss_speed;
    scrsaver_dlg[7].d2 = ss_density;
    
    if(is_large)
        large_dialog(scrsaver_dlg);
        
    int ret = zc_popup_dialog(scrsaver_dlg,-1);
    
    if(ret == 8 || ret == 9)
    {
        ss_after   = scrsaver_dlg[5].d1;
        ss_speed   = scrsaver_dlg[6].d2;
        ss_density = scrsaver_dlg[7].d2;
    }
    
    if(ret == 9)
        // preview Screen Saver
    {
        clear_keybuf();
        scare_mouse();
        Matrix(ss_speed, ss_density, 30);
        system_pal();
        unscare_mouse();
    }
    
    return D_O_K;
}

/*****  Menus  *****/

static MENU game_menu[] =
{
    { (char *)"&Continue\tESC",            onContinue,               NULL,                      0, NULL },
    { (char *)"",                          NULL,                     NULL,                      0, NULL },
    { (char *)"L&oad Quest...",            onCustomGame,             NULL,                      0, NULL },
    { (char *)"&End Game\tF6",             onQuit,                   NULL,                      0, NULL },
    { (char *)"",                          NULL,                     NULL,                      0, NULL },
#ifndef ALLEGRO_MACOSX
    { (char *)"&Reset\tF9",                onReset,                  NULL,                      0, NULL },
    { (char *)"&Quit\tF10",                onExit,                   NULL,                      0, NULL },
#else
    { (char *)"&Reset\tF7",                onReset,                  NULL,                      0, NULL },
    { (char *)"&Quit\tF8",                onExit,                   NULL,                      0, NULL },
#endif
    { NULL,                                NULL,                     NULL,                      0, NULL }
};

static MENU title_menu[] =
{
    { (char *)"&Original",                 onTitle_NES,              NULL,                      0, NULL },
    { (char *)"&Zelda Classic",            onTitle_DX,               NULL,                      0, NULL },
    { (char *)"Zelda Classic &2.50",       onTitle_25,               NULL,                      0, NULL },
    { NULL,                                NULL,                     NULL,                      0, NULL }
};

static MENU snapshot_format_menu[] =
{
    { (char *)"&BMP",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { (char *)"&GIF",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { (char *)"&JPG",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { (char *)"&PNG",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { (char *)"PC&X",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { (char *)"&TGA",                      onSetSnapshotFormat,      NULL,                      0, NULL },
    { NULL,                                NULL,                     NULL,                      0, NULL }
};

static MENU controls_menu[] =
{
    { (char *)"Key &Buttons...",            onKeyboard,              NULL,                      0, NULL },
    { (char *)"Key &Directions...",         onKeyboardDir,           NULL,                      0, NULL },
    { (char *)"&Joystick Buttons...",       onJoystick,              NULL,                      0, NULL },
    { (char *)"Joy&stick Directions...",    onJoystickDir,           NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

static MENU name_entry_mode_menu[] =
{
    { (char *)"&Keyboard",                  onKeyboardEntry,         NULL,                      0, NULL },
    { (char *)"&Letter Grid",               onLetterGridEntry,       NULL,                      0, NULL },
    { (char *)"&Extended Letter Grid",      onExtLetterGridEntry,    NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

static MENU settings_menu[] =
{
    { (char *)"C&ontrols",                  NULL,                    controls_menu,             0, NULL },
    { (char *)"&Sound...",                  onSound,                 NULL,                      0, NULL },
    { (char *)"&Title Screen",              NULL,                    title_menu,                0, NULL },
    { (char *)"Name &Entry Mode",           NULL,                    name_entry_mode_menu,      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"&Cap FPS\tF1",               onVsync,                 NULL,                      0, NULL },
    { (char *)"Show &FPS\tF2",              onShowFPS,               NULL,                      0, NULL },
    { (char *)"Show Trans. &Layers",        onTransLayers,           NULL,                      0, NULL },
    { (char *)"Up+A+B To &Quit",            onNESquit,               NULL,                      0, NULL },
    { (char *)"Click to Freeze",            onClickToFreeze,         NULL,                      0, NULL },
    { (char *)"Volume &Keys",               onVolKeys,               NULL,                      0, NULL },
    { (char *)"Cont. &Heart Beep",          onHeartBeep,             NULL,                      0, NULL },
    { (char *)"Sa&ve Indicator",            onSaveIndicator,         NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"S&napshot Format",           NULL,                    snapshot_format_menu,      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"Debu&g",                     onDebug,                 NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

static MENU misc_menu[] =
{
    { (char *)"&About...",                  onAbout,                 NULL,                      0, NULL },
    { (char *)"&Credits...",                onCredits,               NULL,                      0, NULL },
    { (char *)"&Fullscreen",                onFullscreenMenu,        NULL,                      0, NULL },
    { (char *)"&Video Mode...",             onVidMode,               NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"&Quest Info...",             onQuest,                 NULL,                      0, NULL },
    { (char *)"Quest &MIDI Info...",        onMIDICredits,           NULL,                      0, NULL },
    { (char *)"Quest &Directory...",        onQstPath,               NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"Take &Snapshot\tF12",        onSnapshot,              NULL,                      0, NULL },
    { (char *)"Sc&reen Saver...",           onScreenSaver,           NULL,                      0, NULL },
    { (char *)"Save ZC Configuration",           OnSaveZCConfig,           NULL,                      0, NULL },
     { (char *)"Show Debug Console",           onDebugConsole,           NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

static MENU refill_menu[] =
{
    { (char *)"&Life\t*, H",                onRefillLife,            NULL,                      0, NULL },
    { (char *)"&Magic\t/, M",               onRefillMagic,           NULL,                      0, NULL },
    { (char *)"&Bombs\tb",                  onCheatBombs,            NULL,                      0, NULL },
    { (char *)"&Rupees\tr",                 onCheatRupies,           NULL,                      0, NULL },
    { (char *)"&Arrows\ta",                 onCheatArrows,           NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

static MENU show_menu[] =
{
    { (char *)"Combos\t0",             onShowLayer0,                 NULL,                      0, NULL },
    { (char *)"Layer 1\t1",             onShowLayer1,                 NULL,                      0, NULL },
    { (char *)"Layer 2\t2",             onShowLayer2,                 NULL,                      0, NULL },
    { (char *)"Layer 3\t3",             onShowLayer3,                 NULL,                      0, NULL },
    { (char *)"Layer 4\t4",             onShowLayer4,                 NULL,                      0, NULL },
    { (char *)"Layer 5\t5",             onShowLayer5,                 NULL,                      0, NULL },
    { (char *)"Layer 6\t6",             onShowLayer6,                 NULL,                      0, NULL },
    { (char *)"Overhead Combos",        onShowLayerO,                 NULL,                      0, NULL },
    { (char *)"Push Blocks",            onShowLayerP,                 NULL,                      0, NULL },
    { (char *)"Freeform Combos\t7",	  onShowLayerF,                 NULL,                      0, NULL },
    { (char *)"Sprites\t8",			  onShowLayerS,                 NULL,                      0, NULL },
    { (char *)"",                           NULL,                     NULL,                      0, NULL },
    { (char *)"Walkability\tw",		  onShowLayerW,                 NULL,                      0, NULL },
    { (char *)"Current FFC Scripts",	  onShowFFScripts,              NULL,                      0, NULL },
    { (char *)"Hitboxes",				  onShowHitboxes,               NULL,                      0, NULL },
    { NULL,                                 NULL,                     NULL,                      0, NULL }
};

static MENU cheat_menu[] =
{
    { (char *)"&Enter code...",             onCheat,                 NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"Re&fill",                    NULL,                    refill_menu,               0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"&Clock\ti",                  onClock,                 NULL,                      0, NULL },
    { (char *)"Ma&x Bombs...",              onMaxBombs,              NULL,                      0, NULL },
    { (char *)"&Heart Containers...",       onHeartC,                NULL,                      0, NULL },
    { (char *)"&Magic Containers...",       onMagicC,                NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"&Link Data...",              onCheatConsole,          NULL,                      0, NULL },
    { (char *)"",                           NULL,                    NULL,                      0, NULL },
    { (char *)"Walk Through &Walls\tF11",   onNoWalls,               NULL,                      0, NULL },
    { (char *)"Link Ignores Side&view\tv",  onIgnoreSideview,        NULL,                      0, NULL },
    { (char *)"&Quick Movement\tq",         onGoFast,                NULL,                      0, NULL },
    { (char *)"&Kill All Enemies",          onKillCheat,             NULL,                      0, NULL },
    { (char *)"Show/Hide Layer",            NULL,                    show_menu,                 0, NULL },
    { (char *)"Toggle Light\tl",            onLightSwitch,           NULL,                      0, NULL },
    { (char *)"&Goto Location...\tg",       onGoTo,                  NULL,                      0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};


MENU the_menu[] =
{
    { (char *)"&Game",                      NULL,                    game_menu,                 0, NULL },
    { (char *)"&Settings",                  NULL,                    settings_menu,             0, NULL },
    { (char *)"&Cheat",                     NULL,                    cheat_menu,                0, NULL },
    { (char *)"&Misc",                      NULL,                    misc_menu,                 0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

MENU the_menu2[] =
{
    { (char *)"&Game",                      NULL,                    game_menu,                 0, NULL },
    { (char *)"&Settings",                  NULL,                    settings_menu,             0, NULL },
    { (char *)"&Misc",                      NULL,                    misc_menu,                 0, NULL },
    { NULL,                                 NULL,                    NULL,                      0, NULL }
};

int onKeyboardEntry()
{
    NameEntryMode=0;
    return D_O_K;
}

int onLetterGridEntry()
{
    NameEntryMode=1;
    return D_O_K;
}

int onExtLetterGridEntry()
{
    NameEntryMode=2;
    return D_O_K;
}

int onFullscreenMenu()
{
    onFullscreen();
    misc_menu[2].flags =(isFullScreen()==1)?D_SELECTED:0;
    return D_O_K;
}

void fix_menu()
{
    if(!debug_enabled)
        settings_menu[15].text = NULL;
}

static DIALOG system_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
    { jwin_menu_proc,    0,    0,    0,    0,    0,    0,    0,       D_USER,  0,        0, (void *) the_menu, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0, (void *) onVsync, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0, (void *) onShowFPS, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0, (void *) onQuit, NULL,  NULL },
#ifndef ALLEGRO_MACOSX
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F9,   0, (void *) onReset, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F10,  0, (void *) onExit, NULL,  NULL },
#else
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0, (void *) onReset, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0, (void *) onExit, NULL,  NULL },
#endif
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0, (void *) onSnapshot, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TAB,  0, (void *) onDebug, NULL,  NULL },
    { d_timer_proc,      0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       NULL,             NULL, NULL },
    { NULL,              0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       NULL,                           NULL,  NULL }
};

static DIALOG system_dlg2[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
    { jwin_menu_proc,    0,    0,    0,    0,    0,    0,    0,       D_USER,  0,        0, (void *) the_menu2, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0, (void *) onVsync, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0, (void *) onShowFPS, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0, (void *) onQuit, NULL,  NULL },
#ifndef ALLEGRO_MACOSX
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F9,   0, (void *) onReset, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F10,  0, (void *) onExit, NULL,  NULL },
#else
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0, (void *) onReset, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0, (void *) onExit, NULL,  NULL },
#endif
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0, (void *) onSnapshot, NULL,  NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TAB,  0, (void *) onDebug, NULL,  NULL },
    { d_timer_proc,      0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       NULL,             NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void reset_snapshot_format_menu()
{
    for(int i=0; i<ssfmtMAX; ++i)
    {
        snapshot_format_menu[i].flags=0;
    }
}

int onSetSnapshotFormat()
{
    switch(active_menu->text[1])
    {
    case 'B': //"&BMP"
        SnapshotFormat=0;
        break;
        
    case 'G': //"&GIF"
        SnapshotFormat=1;
        break;
        
    case 'J': //"&JPG"
        SnapshotFormat=2;
        break;
        
    case 'P': //"&PNG"
        SnapshotFormat=3;
        break;
        
    case 'C': //"PC&X"
        SnapshotFormat=4;
        break;
        
    case 'T': //"&TGA"
        SnapshotFormat=5;
        break;
        
    case 'L': //"&LBM"
        SnapshotFormat=6;
        break;
    }
    
    snapshot_format_menu[SnapshotFormat].flags=D_SELECTED;
    return D_O_K;
}


void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int from,int to)
{
    PALETTE tmp;
    
    for(int i=0; i<256; i++)
    {
        tmp[i].r=r;
        tmp[i].g=g;
        tmp[i].b=b;
    }
    
    fade_interpolate(src,tmp,dest,pos,from,to);
}


void system_pal()
{
    PALETTE pal;
    copy_pal((RGB*)data[PAL_GUI].dat, pal);
    
    // set up the grayscale palette
    for(int i=128; i<192; i++)
    {
        pal[i].r = i-128;
        pal[i].g = i-128;
        pal[i].b = i-128;
    }
    
    
    switch(gui_colorset)
    {
        /*
          enum
          {
          jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
          jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
          jcMAX
          };
          */
    case 1:  //Windows 98
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
        pal[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
        
        byte palrstart=  0*63/255, palrend=166*63/255,
             palgstart=  0*63/255, palgend=202*63/255,
             palbstart=128*63/255, palbend=240*63/255,
             paldivs=7;
             
        for(int i=0; i<paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(3);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(2);
        jwin_pal[jcDARK]   =dvc(1);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
    
    case 2:  //Windows 99
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  64*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
        pal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(9)] = _RGB(0*63/255,   0*63/255,  80*63/255);
        
        byte palrstart=  0*63/255, palrend=166*63/255,
             palgstart=  0*63/255, palgend=202*63/255,
             
             palbstart=128*63/255, palbend=240*63/255,
             paldivs=6;
             
        for(int i=0; i<paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(6);
        jwin_pal[jcMEDLT]  =dvc(5);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(10);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(8);
        jwin_pal[jcTEXTBG] =dvc(6);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(9);
        jwin_pal[jcSELFG]  =dvc(7);
    }
    break;
    
    case 3:  //Windows 2000 Blue
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(16*63/255,  15*63/255, 116*63/255);
        pal[dvc(3)] = _RGB(82*63/255,  80*63/255, 182*63/255);
        pal[dvc(4)] = _RGB(162*63/255, 158*63/255, 250*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 127*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255,  63*63/255);
        pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
        
        byte palrstart=  0*63/255, palrend=162*63/255,
             palgstart=  0*63/255, palgend=158*63/255,
             palbstart= 80*63/255, palbend=250*63/255,
             paldivs=7;
             
        for(int i=0; i<paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(4);
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
    }
    break;
    
    case 687:  //Windows 2000 Gold (6-87 was the North American release date of LoZ)
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  43*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255,  85*63/255);
        pal[dvc(4)] = _RGB(223*63/255, 200*63/255, 128*63/255); // Old Gold
        pal[dvc(5)] = _RGB(223*63/255, 223*63/255, 149*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(9)] = _RGB(80*63/255,  80*63/255,   0*63/255);
        
        byte palrstart=128*63/255, palrend=240*63/255,
             palgstart=128*63/255, palgend=202*63/255,
             palbstart=  0*63/255, palbend=166*63/255,
             paldivs=6;
             
        for(int i=0; i<paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(6);
        jwin_pal[jcMEDLT]  =dvc(5);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(10);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(8);
        jwin_pal[jcTEXTBG] =dvc(6);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(9);
        jwin_pal[jcSELFG]  =dvc(7);
    }
    break;
    
    case 4104:  //Windows 2000 Easter (4-1-04 is April Fools Day, the date of this release)
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  64*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(4)] = _RGB(252*63/255, 186*63/255, 188*63/255);
        pal[dvc(5)] = _RGB(254*63/255, 238*63/255, 238*63/255);
        pal[dvc(6)] = _RGB(244*63/255, 243*63/255, 161*63/255);
        pal[dvc(7)] = _RGB(120*63/255, 173*63/255, 189*63/255);
        pal[dvc(8)] = _RGB(220*63/255, 183*63/255, 227*63/255);
        
        byte palrstart=244*63/255, palrend=220*63/255,
             palgstart=243*63/255, palgend=183*63/255,
             palbstart=161*63/255, palbend=227*63/255,
             paldivs=7;
             
        for(int i=0; i < paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(7);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(7);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
    
    default:  //Windows 2000
    {
        pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(66*63/255,  65*63/255,  66*63/255);
        pal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
        pal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
        
        byte palrstart= 10*63/255, palrend=166*63/255,
             palgstart= 36*63/255, palgend=202*63/255,
             palbstart=106*63/255, palbend=240*63/255,
             paldivs=7;
             
        for(int i=0; i<paldivs; i++)
        {
            pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
            pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }
        
        jwin_pal[jcBOX]    =dvc(4);
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
    }
    break;
    }
    
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    
    jwin_set_colors(jwin_pal);
    
    color_layer(pal, pal, 24,16,16, 28, 128,191);
    
    for(int i=0; i<256; i+=2)
    {
        int v = (i>>3)+2;
        int c = (i>>3)+192;
        pal[c] = _RGB(v,v,v+(v>>1));
        /*
          if(i<240)
          {
          _allegro_hline(tmp_scr,0,i,319,c);
          _allegro_hline(tmp_scr,0,i+1,319,c);
          }
          */
    }
    
    // draw the vertical screen gradient
    for(int i=0; i<240; ++i)
    {
        _allegro_hline(tmp_scr,0,i,319,192+(i*31/239));
    }
    
    /*
      palrstart= 10*63/255; palrend=166*63/255;
      palgstart= 36*63/255; palgend=202*63/255;
      palbstart=106*63/255; palbend=240*63/255;
      paldivs=32;
      for(int i=0; i<paldivs; i++)
      {
      pal[223-paldivs+1+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
      pal[223-paldivs+1+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
      pal[223-paldivs+1+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }
      */
    BITMAP *panorama = create_bitmap_ex(8,256,224);
    int ts_height, ts_start;
    
    if(tmpscr->flags3&fNOSUBSCR && !(tmpscr->flags3&fNOSUBSCROFFSET))
    {
        clear_to_color(panorama,0);
        blit(framebuf,panorama,0,playing_field_offset,0,28,256,224-passive_subscreen_height);
        ts_height=224-passive_subscreen_height;
        ts_start=28;
    }
    else
    {
        blit(framebuf,panorama,0,0,0,0,256,224);
        ts_height=224;
        ts_start=0;
    }
    
    // gray scale the current frame
    for(int y=0; y<ts_height; y++)
    {
        for(int x=0; x<256; x++)
        {
            int c = panorama->line[y+ts_start][x];
            int gray = zc_min((RAMpal[c].r*42 + RAMpal[c].g*75 + RAMpal[c].b*14) >> 7, 63);
            tmp_scr->line[y+8+ts_start][x+32] = gray+128;
        }
    }
    
    destroy_bitmap(panorama);
    
    // save the fps_undo section
    blit(tmp_scr,fps_undo,40,216,0,0,64,16);
    
    // display everything
    vsync();
    set_palette_range(pal,0,255,false);
    
    if(sbig)
        stretch_blit(tmp_scr,screen,0,0,320,240,scrx-(160*(screen_scale-1)),scry-(120*(screen_scale-1)),screen_scale*320,screen_scale*240);
    else
        blit(tmp_scr,screen,0,0,scrx,scry,320,240);
        
    if(ShowFPS)
        show_fps(screen);
        
    if(Paused)
        show_paused(screen);
        
    //  sys_pal = pal;
    memcpy(sys_pal,pal,sizeof(pal));
}


void system_pal2()
{
    PALETTE RAMpal2;
    copy_pal((RGB*)data[PAL_GUI].dat, RAMpal2);
    
    /* Windows 2000 colors
      RAMpal2[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal2[dvc(2)] = _RGB( 66*63/255,  65*63/255,  66*63/255);
      RAMpal2[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
      RAMpal2[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
      RAMpal2[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal2[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal2[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal2[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);
    
      byte palrstart= 10*63/255, palrend=166*63/255,
      palgstart= 36*63/255, palgend=202*63/255,
      palbstart=106*63/255, palbend=240*63/255,
      paldivs=7;
      for(int i=0; i<paldivs; i++)
      {
      RAMpal2[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }
      */
    
    /* Windows 98 colors
      RAMpal2[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal2[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
      RAMpal2[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
      RAMpal2[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
      RAMpal2[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal2[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal2[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal2[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);
    
      byte palrstart=  0*63/255, palrend=166*63/255,
      palgstart=  0*63/255, palgend=202*63/255,
      palbstart=128*63/255, palbend=240*63/255,
      paldivs=7;
      for(int i=0; i<paldivs; i++)
      {
      RAMpal2[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }
      */
    
    /* Windows 99 colors
      RAMpal2[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal2[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
      RAMpal2[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
      RAMpal2[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
      RAMpal2[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
      RAMpal2[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal2[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal2[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal2[dvc(9)] = _RGB(  0*63/255,   0*63/255,  80*63/255);
    
      byte palrstart=  0*63/255, palrend=166*63/255,
      palgstart=  0*63/255, palgend=202*63/255,
    
      palbstart=128*63/255, palbend=240*63/255,
      paldivs=6;
      for(int i=0; i<paldivs; i++)
      {
      RAMpal2[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
      RAMpal2[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }
      */
    
    
    
    RAMpal2[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
    RAMpal2[dvc(2)] = _RGB(64*63/255,  64*63/255,  64*63/255);
    RAMpal2[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
    RAMpal2[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
    RAMpal2[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
    RAMpal2[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
    RAMpal2[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
    RAMpal2[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
    RAMpal2[dvc(9)] = _RGB(0*63/255,   0*63/255,  80*63/255);
    
    byte palrstart=  0*63/255, palrend=166*63/255,
         palgstart=  0*63/255, palgend=202*63/255,
         palbstart=128*63/255, palbend=240*63/255,
         paldivs=6;
         
    for(int i=0; i<paldivs; i++)
    {
        RAMpal2[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal2[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal2[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
    }
    
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    
    jwin_set_colors(jwin_pal);
    
    
    // set up the new palette
    for(int i=128; i<192; i++)
    {
        RAMpal2[i].r = i-128;
        RAMpal2[i].g = i-128;
        RAMpal2[i].b = i-128;
    }
    
    /*
      for(int i=0; i<64; i++)
      {
      RAMpal2[128+i] = _RGB(i,i,i)1));
      }
      */
    
    /*
    
      pal[vc(1)]  = _RGB(0x00,0x00,0x14);
      pal[vc(4)]  = _RGB(0x36,0x36,0x36);
      pal[vc(6)]  = _RGB(0x10,0x10,0x10);
      pal[vc(7)]  = _RGB(0x20,0x20,0x20);
      pal[vc(9)]  = _RGB(0x20,0x20,0x24);
      pal[vc(11)] = _RGB(0x30,0x30,0x30);
      pal[vc(14)] = _RGB(0x3F,0x38,0x28);
    
      gui_fg_color=vc(14);
      gui_bg_color=vc(1);
      gui_mg_color=vc(9);
    
      jwin_set_colors(jwin_pal);
      */
    
    //  color_layer(RAMpal2, RAMpal2, 24,16,16, 28, 128,191);
    
    // set up the colors for the vertical screen gradient
    for(int i=0; i<256; i+=2)
    {
        int v = (i>>3)+2;
        int c = (i>>3)+192;
        RAMpal2[c] = _RGB(v,v,v+(v>>1));
        
        /*
          if(i<240)
          {
          _allegro_hline(tmp_scr,0,i,319,c);
          _allegro_hline(tmp_scr,0,i+1,319,c);
          }
          */
    }
    
    set_palette(RAMpal2);
    
    for(int i=0; i<240; ++i)
    {
        _allegro_hline(tmp_scr,0,i,319,192+(i*31/239));
    }
    
    /*
      byte palrstart= 10*63/255, palrend=166*63/255,
      palgstart= 36*63/255, palgend=202*63/255,
      palbstart=106*63/255, palbend=240*63/255,
      paldivs=32;
      for(int i=0; i<paldivs; i++)
      {
      pal[223-paldivs+1+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
      pal[223-paldivs+1+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
      pal[223-paldivs+1+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }
      */
    BITMAP *panorama = create_bitmap_ex(8,256,224);
    int ts_height, ts_start;
    
    if(tmpscr->flags3&fNOSUBSCR && !(tmpscr->flags3&fNOSUBSCROFFSET))
    {
        clear_to_color(panorama,0);
        blit(framebuf,panorama,0,playing_field_offset,0,28,256,224-passive_subscreen_height);
        ts_height=224-passive_subscreen_height;
        ts_start=28;
    }
    else
    {
        blit(framebuf,panorama,0,0,0,0,256,224);
        ts_height=224;
        ts_start=0;
    }
    
    // gray scale the current frame
    for(int y=0; y<ts_height; y++)
    {
        for(int x=0; x<256; x++)
        {
            int c = panorama->line[y+ts_start][x];
            int gray = zc_min((RAMpal2[c].r*42 + RAMpal2[c].g*75 + RAMpal2[c].b*14) >> 7, 63);
            tmp_scr->line[y+8+ts_start][x+32] = gray+128;
        }
    }
    
    destroy_bitmap(panorama);
    
    // save the fps_undo section
    blit(tmp_scr,fps_undo,40,216,0,0,64,16);
    
    // display everything
    vsync();
    set_palette_range(RAMpal2,0,255,false);
    
    if(sbig)
        //stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
        stretch_blit(tmp_scr,screen,0,0,320,240,scrx-(160*(screen_scale-1)),scry-(120*(screen_scale-1)),screen_scale*320,screen_scale*240);
    else
        blit(tmp_scr,screen,0,0,scrx,scry,320,240);
        
    if(ShowFPS)
        show_fps(screen);
        
    if(Paused)
        show_paused(screen);
        
    //  sys_pal = pal;
    memcpy(sys_pal,RAMpal2,sizeof(RAMpal2));
}

#ifdef _WIN32
void switch_out_callback()
{
	if(midi_patch_fix==0 || currmidi==0)
        return;
        
    bool was_paused=midi_paused;
    long pos=midi_pos;
    int digi_vol, midi_vol;
	
	get_volume(&digi_vol, &midi_vol);
    stop_midi();
    jukebox(currmidi);
	set_volume(digi_vol, midi_vol);
    midi_seek(pos);
    
    if(was_paused)
    {
        midi_pause();
        midi_paused=true;
    }
}

void switch_in_callback()
{
	if(midi_patch_fix==0 || currmidi==0)
        return;
        
    bool was_paused=midi_paused;
    long pos=midi_pos;
    int digi_vol, midi_vol;
	
	get_volume(&digi_vol, &midi_vol);
    stop_midi();
    jukebox(currmidi);
	set_volume(digi_vol, midi_vol);
    midi_seek(pos);
    
    if(was_paused)
    {
        midi_pause();
        midi_paused=true;
    }
}
#else // Not Windows
void switch_out_callback()
{
}

void switch_in_callback()
{
}
#endif

void game_pal()
{
    clear_to_color(screen,BLACK);
    set_palette_range(RAMpal,0,255,false);
}

static char bar_str[] = "";

void music_pause()
{
    //al_pause_duh(tmplayer);
    zcmusic_pause(zcmusic, ZCM_PAUSE);
    midi_pause();
    midi_paused=true;
}

void music_resume()
{
    //al_resume_duh(tmplayer);
    zcmusic_pause(zcmusic, ZCM_RESUME);
    midi_resume();
    midi_paused=false;
}

void music_stop()
{
    //al_stop_duh(tmplayer);
    //unload_duh(tmusic);
    //tmusic=NULL;
    //tmplayer=NULL;
    zcmusic_stop(zcmusic);
    zcmusic_unload_file(zcmusic);
    stop_midi();
    midi_paused=false;
    currmidi=0;
}

void System()
{
    mouse_down=gui_mouse_b();
    music_pause();
    pause_all_sfx();
    
    system_pal();
    //  FONT *oldfont=font;
    //  font=tfont;
    
    misc_menu[2].flags =(isFullScreen()==1)?D_SELECTED:0;
    
    game_menu[2].flags = getsaveslot() > -1 ? 0 : D_DISABLED;
    game_menu[3].flags =
        misc_menu[5].flags = Playing ? 0 : D_DISABLED;
    misc_menu[7].flags = !Playing ? 0 : D_DISABLED;
    
    clear_keybuf();
    show_mouse(screen);
    
    DIALOG_PLAYER *p;
    
    if(!Playing || (!zcheats.flags && !get_debug()))
    {
        p = init_dialog(system_dlg2,-1);
    }
    else
    {
        p = init_dialog(system_dlg,-1);
    }
    
    // drop the menu on startup if menu button pressed
    if(joybtn(Mbtn)||key[KEY_ESC])
        simulate_keypress(KEY_G << 8);
        
    do
    {
        rest(17);
        
        if(mouse_down && !gui_mouse_b())
            mouse_down=0;
            
        title_menu[0].flags = (title_version==0) ? D_SELECTED : 0;
        title_menu[1].flags = (title_version==1) ? D_SELECTED : 0;
        title_menu[2].flags = (title_version==2) ? D_SELECTED : 0;
        
        settings_menu[5].flags = Throttlefps?D_SELECTED:0;
        settings_menu[6].flags = ShowFPS?D_SELECTED:0;
        settings_menu[7].flags = TransLayers?D_SELECTED:0;
        settings_menu[8].flags = NESquit?D_SELECTED:0;
        settings_menu[9].flags = ClickToFreeze?D_SELECTED:0;
        settings_menu[10].flags = volkeys?D_SELECTED:0;
        
        name_entry_mode_menu[0].flags = (NameEntryMode==0)?D_SELECTED:0;
        name_entry_mode_menu[1].flags = (NameEntryMode==1)?D_SELECTED:0;
        name_entry_mode_menu[2].flags = (NameEntryMode==2)?D_SELECTED:0;
        
        /*
          if(!Playing || (!zcheats.flags && !debug))
          {
          cheat_menu[0].flags = D_DISABLED;
          cheat_menu[1].text  = NULL;
          }
          else */
        {
            cheat_menu[0].flags = 0;
            refill_menu[4].flags = get_bit(quest_rules, qr_TRUEARROWS) ? 0 : D_DISABLED;
            cheat_menu[1].text  = (cheat >= 1) || get_debug() ? bar_str : NULL;
            cheat_menu[3].text  = (cheat >= 2) || get_debug() ? bar_str : NULL;
            cheat_menu[8].text  = (cheat >= 3) || get_debug() ? bar_str : NULL;
            cheat_menu[10].text = (cheat >= 4) || get_debug() ? bar_str : NULL;
            cheat_menu[4].flags = getClock() ? D_SELECTED : 0;
            cheat_menu[11].flags = toogam ? D_SELECTED : 0;
            cheat_menu[12].flags = ignoreSideview ? D_SELECTED : 0;
            cheat_menu[13].flags = gofast ? D_SELECTED : 0;
            
            show_menu[0].flags = show_layer_0 ? D_SELECTED : 0;
            show_menu[1].flags = show_layer_1 ? D_SELECTED : 0;
            show_menu[2].flags = show_layer_2 ? D_SELECTED : 0;
            show_menu[3].flags = show_layer_3 ? D_SELECTED : 0;
            show_menu[4].flags = show_layer_4 ? D_SELECTED : 0;
            show_menu[5].flags = show_layer_5 ? D_SELECTED : 0;
            show_menu[6].flags = show_layer_6 ? D_SELECTED : 0;
            show_menu[7].flags = show_layer_over ? D_SELECTED : 0;
            show_menu[8].flags = show_layer_push ? D_SELECTED : 0;
            show_menu[9].flags = show_sprites ? D_SELECTED : 0;
            show_menu[10].flags = show_ffcs ? D_SELECTED : 0;
            show_menu[12].flags = show_walkflags ? D_SELECTED : 0;
            show_menu[13].flags = show_ff_scripts ? D_SELECTED : 0;
            show_menu[14].flags = show_hitboxes ? D_SELECTED : 0;
        }
        
        settings_menu[11].flags = heart_beep ? D_SELECTED : 0;
        settings_menu[12].flags = use_save_indicator ? D_SELECTED : 0;
        reset_snapshot_format_menu();
        snapshot_format_menu[SnapshotFormat].flags = D_SELECTED;
        
        if(debug_enabled)
        {
            settings_menu[16].flags = get_debug() ? D_SELECTED : 0;
        }
        
        if(gui_mouse_b() && !mouse_down)
            break;
            
        // press menu to drop the menu
        if(rMbtn())
            simulate_keypress(KEY_G << 8);
            
        {
            if(input_idle(true) > after_time())
                // run Screeen Saver
            {
				// Screen saver enabled for now.
				clear_keybuf();
                scare_mouse();
                Matrix(ss_speed, ss_density, 0);
                system_pal();
                unscare_mouse();
                broadcast_dialog_message(MSG_DRAW, 0);
            }
        }
        
    }
    while(update_dialog(p));
    
    //  font=oldfont;
    mouse_down=gui_mouse_b();
    shutdown_dialog(p);
    show_mouse(NULL);
    
    if(Quit)
    {
        kill_sfx();
        music_stop();
        clear_to_color(screen,BLACK);
    }
    else
    {
        game_pal();
        music_resume();
        resume_all_sfx();
        
        if(rc)
            ringcolor(false);
    }
    
    eat_buttons();
    
    rc=false;
    clear_keybuf();
    //  text_mode(0);
}

void fix_dialog(DIALOG *d)
{
    for(; d->proc != NULL; d++)
    {
        d->x += scrx;
        d->y += scry;
    }
}

void fix_dialogs()
{
    /*
      int x = scrx-(sbig?160:0);
      int y = scry-(sbig?120:0);
      if(x>0) x+=3;
      if(y>0) y+=3;
      if(x<0) x=0;
      if(y<0) y=0;
    
      system_dlg[0].x = x;
      system_dlg[0].y = y;
      system_dlg2[0].x = x;
      system_dlg2[0].y = y;
    */
    
    jwin_center_dialog(about_dlg);
    jwin_center_dialog(btn_dlg);
    jwin_center_dialog(btndir_dlg);
    jwin_center_dialog(cheat_dlg);
    jwin_center_dialog(credits_dlg);
    jwin_center_dialog(gamemode_dlg);
    jwin_center_dialog(getnum_dlg);
    jwin_center_dialog(goto_dlg);
    center_zc_init_dialog();
    jwin_center_dialog(key_dlg);
    jwin_center_dialog(keydir_dlg);
    jwin_center_dialog(midi_dlg);
    jwin_center_dialog(quest_dlg);
    jwin_center_dialog(scrsaver_dlg);
    jwin_center_dialog(sound_dlg);
    jwin_center_dialog(triforce_dlg);
    
    digi_dp[1] += scrx;
    digi_dp[2] += scry;
    midi_dp[1] += scrx;
    midi_dp[2] += scry;
    pan_dp[1]  += scrx;
    pan_dp[2]  += scry;
    emus_dp[1]  += scrx;
    emus_dp[2]  += scry;
    buf_dp[1]  += scrx;
    buf_dp[2]  += scry;
    sfx_dp[1]  += scrx;
    sfx_dp[2]  += scry;
}

/*****************************/
/**** Custom Sound System ****/
/*****************************/

INLINE int mixvol(int v1,int v2)
{
    return (zc_min(v1,255)*zc_min(v2,255)) >> 8;
}

// Run an NSF, or a MIDI if the NSF is missing somehow.
bool try_zcmusic(char *filename, int track, int midi)
{
    ZCMUSIC *newzcmusic = NULL;
    
    // Try the ZC directory first
    {
        char exepath[2048];
        char musicpath[2048];
        get_executable_name(exepath, 2048);
        replace_filename(musicpath, exepath, filename, 2048);
        newzcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
    }
    
    // Not in ZC directory, try the quest directory
    if(newzcmusic==NULL)
    {
        char musicpath[2048];
        replace_filename(musicpath, qstpath, filename, 2048);
        newzcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
    }
    
    // Found it
    if(newzcmusic!=NULL)
    {
        zcmusic_stop(zcmusic);
        zcmusic_unload_file(zcmusic);
        stop_midi();
        
        zcmusic=newzcmusic;
        zcmusic_play(zcmusic, emusic_volume);
        
        if(track>0)
            zcmusic_change_track(zcmusic,track);
            
        return true;
    }
    
    // Not found, play MIDI - unless this was called by a script (yay, magic numbers)
    else if(midi>-1000)
        jukebox(midi);
        
    return false;
}

void jukebox(int index,int loop)
{
    music_stop();
    
    if(index<0)         index=MAXMIDIS-1;
    
    if(index>=MAXMIDIS) index=0;
    
    music_stop();
    
    // Allegro's DIGMID driver (the one normally used on on Linux) gets
    // stuck notes when a song stops. This fixes it.
    if(strcmp(midi_driver->name, "DIGMID")==0)
        set_volume(0, 0);
        
    set_volume(-1, mixvol(tunes[index].volume,midi_volume>>1));
    play_midi((MIDI*)tunes[index].data,loop);
    
    if(tunes[index].start>0)
        midi_seek(tunes[index].start);
        
    midi_loop_end = tunes[index].loop_end;
    midi_loop_start = tunes[index].loop_start;
    
    currmidi=index;
    master_volume(digi_volume,midi_volume);
    midi_paused=false;
}

void jukebox(int index)
{
    if(index<0)         index=MAXMIDIS-1;
    
    if(index>=MAXMIDIS) index=0;
    
    // do nothing if it's already playing
    if(index==currmidi && midi_pos>=0)
    {
        midi_paused=false;
        return;
    }
    
    jukebox(index,tunes[index].loop);
}

void play_DmapMusic()
{
    static char tfile[2048];
    static int ttrack=0;
    bool domidi=false;
    
    // Seems like this ought to call try_zcmusic()...
    
    if(DMaps[currdmap].tmusic[0]!=0)
    {
        if(zcmusic==NULL ||
           strcmp(zcmusic->filename,DMaps[currdmap].tmusic)!=0 ||
           (zcmusic->type==ZCMF_GME && zcmusic->track != DMaps[currdmap].tmusictrack))
        {
            if(zcmusic != NULL)
            {
                zcmusic_stop(zcmusic);
                zcmusic_unload_file(zcmusic);
                zcmusic = NULL;
            }
            
            // Try the ZC directory first
            {
                char exepath[2048];
                char musicpath[2048];
                get_executable_name(exepath, 2048);
                replace_filename(musicpath, exepath, DMaps[currdmap].tmusic, 2048);
                zcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
            }
            
            // Not in ZC directory, try the quest directory
            if(zcmusic==NULL)
            {
                char musicpath[2048];
                replace_filename(musicpath, qstpath, DMaps[currdmap].tmusic, 2048);
                zcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
            }
            
            if(zcmusic!=NULL)
            {
                stop_midi();
                strcpy(tfile,DMaps[currdmap].tmusic);
                zcmusic_play(zcmusic, emusic_volume);
                int temptracks=0;
                temptracks=zcmusic_get_tracks(zcmusic);
                temptracks=(temptracks<2)?1:temptracks;
                ttrack = vbound(DMaps[currdmap].tmusictrack,0,temptracks-1);
                zcmusic_change_track(zcmusic,ttrack);
            }
            else
            {
                tfile[0] = 0;
                domidi=true;
            }
        }
    }
    else
    {
        domidi=true;
    }
    
    if(domidi)
    {
        int m=DMaps[currdmap].midi;
        
        switch(m)
        {
        case 1:
            jukebox(ZC_MIDI_OVERWORLD);
            break;
            
        case 2:
            jukebox(ZC_MIDI_DUNGEON);
            break;
            
        case 3:
            jukebox(ZC_MIDI_LEVEL9);
            break;
            
        default:
            if(m>=4 && m<4+MAXCUSTOMMIDIS)
                jukebox(m-4+ZC_MIDI_COUNT);
            else
                music_stop();
        }
    }
}

void playLevelMusic()
{
    int m=tmpscr->screen_midi;
    
    switch(m)
    {
    case -2:
        music_stop();
        break;
        
    case -1:
        play_DmapMusic();
        break;
        
    case 1:
        jukebox(ZC_MIDI_OVERWORLD);
        break;
        
    case 2:
        jukebox(ZC_MIDI_DUNGEON);
        break;
        
    case 3:
        jukebox(ZC_MIDI_LEVEL9);
        break;
        
    default:
        if(m>=4 && m<4+MAXCUSTOMMIDIS)
            jukebox(m-4+ZC_MIDI_COUNT);
        else
            music_stop();
    }
}

void master_volume(int dv,int mv)
{
    if(dv>=0) digi_volume=zc_max(zc_min(dv,255),0);
    
    if(mv>=0) midi_volume=zc_max(zc_min(mv,255),0);
    
    int i = zc_min(zc_max(currmidi,0),MAXMIDIS-1);
    set_volume(digi_volume,mixvol(tunes[i].volume,midi_volume));
}

/*****************/
/*****  SFX  *****/
/*****************/

// array of voices, one for each sfx sample in the data file
// 0+ = voice #
// -1 = voice not allocated
void Z_init_sound()
{
    for(int i=0; i<WAV_COUNT; i++)
        sfx_voice[i]=-1;
        
    for(int i=0; i<ZC_MIDI_COUNT; i++)
        tunes[i].data = (MIDI*)mididata[i].dat;
        
    for(int j=0; j<MAXCUSTOMMIDIS; j++)
        tunes[ZC_MIDI_COUNT+j].data=NULL;
        
    master_volume(digi_volume,midi_volume);
}

// returns number of voices currently allocated
int sfx_count()
{
    int c=0;
    
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
            ++c;
            
    return c;
}

// clean up finished samples
void sfx_cleanup()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1 && voice_get_position(sfx_voice[i])<0)
        {
            deallocate_voice(sfx_voice[i]);
            sfx_voice[i]=-1;
        }
}

// allocates a voice for the sample "wav_index" (index into zelda.dat)
// if a voice is already allocated (and/or playing), then it just returns true
// Returns true:  voice is allocated
//         false: unsuccessful
bool sfx_init(int index)
{
    // check index
    if(index<=0 || index>=WAV_COUNT)
        return false;
        
    if(sfx_voice[index]==-1)
    {
        if(sfxdat)
        {
            if(index<Z35)
            {
                sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[index].dat);
            }
            else
            {
                sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[Z35].dat);
            }
        }
        else
        {
            sfx_voice[index]=allocate_voice(&customsfxdata[index]);
        }
        
        voice_set_volume(sfx_voice[index], sfx_volume);
    }
    
    return sfx_voice[index] != -1;
}

// plays an sfx sample
void sfx(int index,int pan,bool loop, bool restart)
{
    if(!sfx_init(index))
        return;
        
    voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
    voice_set_pan(sfx_voice[index],pan);
    
    int pos = voice_get_position(sfx_voice[index]);
    
    if(restart) voice_set_position(sfx_voice[index],0);
    
    if(pos<=0)
        voice_start(sfx_voice[index]);
}

// true if sfx is allocated
bool sfx_allocated(int index)
{
    return (index>0 && index<WAV_COUNT && sfx_voice[index]!=-1);
}

// start it (in loop mode) if it's not already playing,
// otherwise adjust it to play in loop mode -DD
void cont_sfx(int index)
{
    if(!sfx_init(index))
    {
        return;
    }
    
    if(voice_get_position(sfx_voice[index])<=0)
    {
        voice_set_position(sfx_voice[index],0);
        voice_set_playmode(sfx_voice[index],PLAYMODE_LOOP);
        voice_start(sfx_voice[index]);
    }
    else
    {
        adjust_sfx(index, 128, true);
    }
}

// adjust parameters while playing
void adjust_sfx(int index,int pan,bool loop)
{
    if(index<=0 || index>=WAV_COUNT || sfx_voice[index]==-1)
        return;
        
    voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
    voice_set_pan(sfx_voice[index],pan);
}

// pauses a voice
void pause_sfx(int index)
{
    if(index>0 && index<WAV_COUNT && sfx_voice[index]!=-1)
        voice_stop(sfx_voice[index]);
}

// resumes a voice
void resume_sfx(int index)
{
    if(index>0 && index<WAV_COUNT && sfx_voice[index]!=-1)
        voice_start(sfx_voice[index]);
}

// pauses all active voices
void pause_all_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
            voice_stop(sfx_voice[i]);
}

// resumes all paused voices
void resume_all_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
            voice_start(sfx_voice[i]);
}

// stops an sfx and deallocates the voice
void stop_sfx(int index)
{
    if(index<=0 || index>=WAV_COUNT)
        return;
        
    if(sfx_voice[index]!=-1)
    {
        deallocate_voice(sfx_voice[index]);
        sfx_voice[index]=-1;
    }
}

// Stops SFX played by Link's item of the given family
void stop_item_sfx(int family)
{
    int id=current_item_id(family);
    
    if(id<0)
        return;
        
    stop_sfx(itemsbuf[id].usesound);
}

void kill_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
        {
            deallocate_voice(sfx_voice[i]);
            sfx_voice[i]=-1;
        }
}

int pan(int x)
{
    switch(pan_style)
    {
    case 0:
        return 128;
        
    case 1:
        return vbound((x>>1)+68,0,255);
        
    case 2:
        return vbound(((x*3)>>2)+36,0,255);
    }
    
    return vbound(x,0,255);
}

/*******************************/
/******* Input Handlers ********/
/*******************************/

bool joybtn(int b)
{
    if(b == 0)
        return false;
        
    return joy[joystick_index].button[b-1].b !=0;
}

int next_press_key()
{
    char k[128];
    
    for(int i=0; i<128; i++)
        k[i]=key[i];
        
    for(;;)
    {
        for(int i=0; i<128; i++)
            if(key[i]!=k[i])
                return i;
    }
    
    //	return (readkey()>>8);
}

int next_press_btn()
{
    clear_keybuf();
    /*bool b[joy[joystick_index].num_buttons+1];
    
    for(int i=1; i<=joy[joystick_index].num_buttons; i++)
        b[i]=joybtn(i);*/
        
    //first, we need to wait until they're pressing no buttons
    for(;;)
    {
        if(keypressed())
        {
            switch(readkey()>>8)
            {
            case KEY_ESC:
                return -1;
                
            case KEY_SPACE:
                return 0;
            }
        }
        
        poll_joystick();
        bool done = true;
        
        for(int i=1; i<=joy[joystick_index].num_buttons; i++)
        {
            if(joybtn(i)) done = false;
        }
        
        if(done) break;
    }
    
    //now, we need to wait for them to press any button
    for(;;)
    {
        if(keypressed())
        {
            switch(readkey()>>8)
            {
            case KEY_ESC:
                return -1;
                
            case KEY_SPACE:
                return 0;
            }
        }
        
        poll_joystick();
        
        for(int i=1; i<=joy[joystick_index].num_buttons; i++)
        {
            if(joybtn(i)) return i;
        }
    }
}

static bool rButton(bool(proc)(),bool &flag)
{
    if(!proc())
    {
        flag=false;
    }
    else if(!flag)
    {
        flag=true;
        return true;
    }
    
    return false;
}

bool control_state[18]=
{
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
};

bool drunk_toggle_state[11]=
{
    false, false, false, false, false, false, false, false, false, false, false
};

bool button_press[18] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
bool button_hold[18] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

#define STICK_1_X joy[joystick_index].stick[js_stick_1_x_stick].axis[js_stick_1_x_axis]
#define STICK_1_Y joy[joystick_index].stick[js_stick_1_y_stick].axis[js_stick_1_y_axis]
#define STICK_2_X joy[joystick_index].stick[js_stick_2_x_stick].axis[js_stick_2_x_axis]
#define STICK_2_Y joy[joystick_index].stick[js_stick_2_y_stick].axis[js_stick_2_y_axis]
#define STICK_PRECISION   56 //define your own sensitivity

void load_control_state()
{
    control_state[0]=key[DUkey]||(analog_movement ? STICK_1_Y.d1 || STICK_1_Y.pos - js_stick_1_y_offset < -STICK_PRECISION : joybtn(DUbtn));
    control_state[1]=key[DDkey]||(analog_movement ? STICK_1_Y.d2 || STICK_1_Y.pos - js_stick_1_y_offset > STICK_PRECISION : joybtn(DDbtn));
    control_state[2]=key[DLkey]||(analog_movement ? STICK_1_X.d1 || STICK_1_X.pos - js_stick_1_x_offset < -STICK_PRECISION : joybtn(DLbtn));
    control_state[3]=key[DRkey]||(analog_movement ? STICK_1_X.d2 || STICK_1_X.pos - js_stick_1_x_offset > STICK_PRECISION : joybtn(DRbtn));
    control_state[4]=key[Akey]||joybtn(Abtn);
    control_state[5]=key[Bkey]||joybtn(Bbtn);
    control_state[6]=key[Skey]||joybtn(Sbtn);
    control_state[7]=key[Lkey]||joybtn(Lbtn);
    control_state[8]=key[Rkey]||joybtn(Rbtn);
    control_state[9]=key[Pkey]||joybtn(Pbtn);
    control_state[10]=key[Exkey1]||joybtn(Exbtn1);
    control_state[11]=key[Exkey2]||joybtn(Exbtn2);
    control_state[12]=key[Exkey3]||joybtn(Exbtn3);
    control_state[13]=key[Exkey4]||joybtn(Exbtn4);
    
    if(num_joysticks != 0)
    {
        control_state[14]= STICK_2_Y.pos - js_stick_2_y_offset < -STICK_PRECISION;
        control_state[15]= STICK_2_Y.pos - js_stick_2_y_offset > STICK_PRECISION;
        control_state[16]= STICK_2_X.pos - js_stick_2_x_offset < -STICK_PRECISION;
        control_state[17]= STICK_2_X.pos - js_stick_2_x_offset > STICK_PRECISION;
    }
    
    button_press[0]=rButton(Up,button_hold[0]);
    button_press[1]=rButton(Down,button_hold[1]);
    button_press[2]=rButton(Left,button_hold[2]);
    button_press[3]=rButton(Right,button_hold[3]);
    button_press[4]=rButton(cAbtn,button_hold[4]);
    button_press[5]=rButton(cBbtn,button_hold[5]);
    button_press[6]=rButton(cSbtn,button_hold[6]);
    button_press[7]=rButton(cLbtn,button_hold[7]);
    button_press[8]=rButton(cRbtn,button_hold[8]);
    button_press[9]=rButton(cPbtn,button_hold[9]);
    button_press[10]=rButton(cEx1btn,button_hold[10]);
    button_press[11]=rButton(cEx2btn,button_hold[11]);
    button_press[12]=rButton(cEx3btn,button_hold[12]);
    button_press[13]=rButton(cEx4btn,button_hold[13]);
    button_press[14]=rButton(AxisUp,button_hold[14]);
    button_press[15]=rButton(AxisDown,button_hold[15]);
    button_press[16]=rButton(AxisLeft,button_hold[16]);
    button_press[17]=rButton(AxisRight,button_hold[17]);
}

// Returns true if any game key is pressed. This is needed because keypressed()
// doesn't detect modifier keys and control_state[] can be modified by scripts.
bool zc_key_pressed()
{
    if((key[DUkey]||(analog_movement ? STICK_1_Y.d1 || STICK_1_Y.pos - js_stick_1_y_offset< -STICK_PRECISION : joybtn(DUbtn))) ||
       (key[DDkey]||(analog_movement ? STICK_1_Y.d2 || STICK_1_Y.pos - js_stick_1_y_offset > STICK_PRECISION : joybtn(DDbtn))) ||
       (key[DLkey]||(analog_movement ? STICK_1_X.d1 || STICK_1_X.pos - js_stick_1_x_offset < -STICK_PRECISION : joybtn(DLbtn))) ||
       (key[DRkey]||(analog_movement ? STICK_1_X.d2 || STICK_1_X.pos - js_stick_1_x_offset > STICK_PRECISION : joybtn(DRbtn))) ||
       (key[Akey]||joybtn(Abtn)) ||
       (key[Bkey]||joybtn(Bbtn)) ||
       (key[Skey]||joybtn(Sbtn)) ||
       (key[Lkey]||joybtn(Lbtn)) ||
       (key[Rkey]||joybtn(Rbtn)) ||
       (key[Pkey]||joybtn(Pbtn)) ||
       (key[Exkey1]||joybtn(Exbtn1)) ||
       (key[Exkey2]||joybtn(Exbtn2)) ||
       (key[Exkey3]||joybtn(Exbtn3)) ||
       (key[Exkey4]||joybtn(Exbtn4))) // Skipping joystick axes
        return true;
    
    return false;
}

bool Up()
{
    return (control_state[0] && !(FFCore.kb_typing_mode));
}
bool Down()
{
    return (control_state[1] && !(FFCore.kb_typing_mode));
}
bool Left()
{
    return (control_state[2] && !(FFCore.kb_typing_mode));
}
bool Right()
{
    return (control_state[3] && !(FFCore.kb_typing_mode));
}
bool cAbtn()
{
    return (control_state[4] && !(FFCore.kb_typing_mode));
}
bool cBbtn()
{
    return (control_state[5] && !(FFCore.kb_typing_mode));
}
bool cSbtn()
{
    return (control_state[6] && !(FFCore.kb_typing_mode));
}
bool cLbtn()
{
    return (control_state[7] && !(FFCore.kb_typing_mode));
}
bool cRbtn()
{
    return (control_state[8] && !(FFCore.kb_typing_mode));
}
bool cPbtn()
{
    return (control_state[9] && !(FFCore.kb_typing_mode));
}
bool cEx1btn()
{
    return (control_state[10] && !(FFCore.kb_typing_mode));
}
bool cEx2btn()
{
    return (control_state[11] && !(FFCore.kb_typing_mode));
}
bool cEx3btn()
{
    return (control_state[12] && !(FFCore.kb_typing_mode));
}
bool cEx4btn()
{
    return (control_state[13] && !(FFCore.kb_typing_mode));
}
bool AxisUp()
{
    return (control_state[14] && !(FFCore.kb_typing_mode));
}
bool AxisDown()
{
    return (control_state[15] && !(FFCore.kb_typing_mode));
}
bool AxisLeft()
{
    return (control_state[16] && !(FFCore.kb_typing_mode));
}
bool AxisRight()
{
    return (control_state[17] && !(FFCore.kb_typing_mode));
}

bool cMbtn()
{
    return ((key[KEY_ESC]||joybtn(Mbtn)) && !(FFCore.kb_typing_mode));
}
bool cF12()
{
    return ((key[KEY_F12] != 0) && !(FFCore.kb_typing_mode));
}
bool cF11()
{
    return ((key[KEY_F11] != 0) && !(FFCore.kb_typing_mode));
}
bool cF5()
{
    return ((key[KEY_F5]  != 0 ) && !(FFCore.kb_typing_mode));
}
bool cQ()
{
    return ((key[KEY_Q]   != 0) && !(FFCore.kb_typing_mode));
}
bool cI()
{
    return ((key[KEY_I]   != 0) && !(FFCore.kb_typing_mode));
}

bool rUp()
{
    return ((rButton(Up,Udown)) && !(FFCore.kb_typing_mode));
}
bool rDown()
{
    return ((rButton(Down,Ddown)) && !(FFCore.kb_typing_mode));
}
bool rLeft()
{
    return ((rButton(Left,Ldown)) && !(FFCore.kb_typing_mode));
}
bool rRight()
{
    return ((rButton(Right,Rdown)) && !(FFCore.kb_typing_mode));
}
bool rAbtn()
{
    return ((rButton(cAbtn,Adown)) && !(FFCore.kb_typing_mode));
}
bool rBbtn()
{
    return ((rButton(cBbtn,Bdown)) && !(FFCore.kb_typing_mode));
}
bool rSbtn()
{
    return ((rButton(cSbtn,Sdown)) && !(FFCore.kb_typing_mode));
}
bool rMbtn()
{
    return ((rButton(cMbtn,Mdown)) && !(FFCore.kb_typing_mode));
}
bool rLbtn()
{
    return ((rButton(cLbtn,LBdown)) && !(FFCore.kb_typing_mode));
}
bool rRbtn()
{
    return ((rButton(cRbtn,RBdown)) && !(FFCore.kb_typing_mode));
}
bool rPbtn()
{
    return ((rButton(cPbtn,Pdown)) && !(FFCore.kb_typing_mode));
}
bool rEx1btn()
{
    return ((rButton(cEx1btn,Ex1down)) && !(FFCore.kb_typing_mode));
}
bool rEx2btn()
{
    return ((rButton(cEx2btn,Ex2down)) && !(FFCore.kb_typing_mode));
}
bool rEx3btn()
{
    return ((rButton(cEx3btn,Ex3down)) && !(FFCore.kb_typing_mode));
}
bool rEx4btn()
{
    return ((rButton(cEx4btn,Ex4down)) && !(FFCore.kb_typing_mode));
}
bool rAxisUp()
{
    return ((rButton(AxisUp,AUdown)) && !(FFCore.kb_typing_mode));
}
bool rAxisDown()
{
    return ((rButton(AxisDown,ADdown)) && !(FFCore.kb_typing_mode));
}
bool rAxisLeft()
{
    return ((rButton(AxisLeft,ALdown)) && !(FFCore.kb_typing_mode));
}
bool rAxisRight()
{
    return ((rButton(AxisRight,ARdown)) && !(FFCore.kb_typing_mode));
}

bool rF12()
{
    return rButton(cF12, F12);
}
bool rF11()
{
    return rButton(cF11, F11);
}
bool rF5()
{
    return rButton(cF5, F5);
}
bool rQ()
{
    return rButton(cQ,  keyQ);
}
bool rI()
{
    return rButton(cI,  keyI);
}

/*No longer in use -V
bool drunk()
{
    return ((!(frame%((rand()%100)+1)))&&(rand()%MAXDRUNKCLOCK<Link.DrunkClock()));
}*/

bool DrunkUp()
{
    return !drunk_toggle_state[0] != !Up();
}
bool DrunkDown()
{
    return !drunk_toggle_state[1] != !Down();
}
bool DrunkLeft()
{
    return !drunk_toggle_state[2] != !Left();
}
bool DrunkRight()
{
    return !drunk_toggle_state[3] != !Right();
}
bool DrunkcAbtn()
{
    return !drunk_toggle_state[4] != !cAbtn();
}
bool DrunkcBbtn()
{
    return !drunk_toggle_state[5] != !cBbtn();
}
bool DrunkcSbtn()
{
    return !drunk_toggle_state[6] != !cSbtn();
}
bool DrunkcMbtn()
{
    return !drunk_toggle_state[10] != !cMbtn();
}
bool DrunkcLbtn()
{
    return !drunk_toggle_state[7] != !cLbtn();
}
bool DrunkcRbtn()
{
    return !drunk_toggle_state[8] != !cRbtn();
}
bool DrunkcPbtn()
{
    return !drunk_toggle_state[9] != !cPbtn();
}

bool DrunkrUp()
{
    return !drunk_toggle_state[0] != !rUp();
}
bool DrunkrDown()
{
    return !drunk_toggle_state[1] != !rDown();
}
bool DrunkrLeft()
{
    return !drunk_toggle_state[2] != !rLeft();
}
bool DrunkrRight()
{
    return !drunk_toggle_state[3] != !rRight();
}
bool DrunkrAbtn()
{
    return !drunk_toggle_state[4] != !rAbtn();
}
bool DrunkrBbtn()
{
    return !drunk_toggle_state[5] != !rBbtn();
}
bool DrunkrSbtn()
{
    return !drunk_toggle_state[6] != !rSbtn();
}
bool DrunkrMbtn()
{
    return !drunk_toggle_state[10] != !rMbtn();
}
bool DrunkrLbtn()
{
    return !drunk_toggle_state[7] != !rLbtn();
}
bool DrunkrRbtn()
{
    return !drunk_toggle_state[8] != !rRbtn();
}
bool DrunkrPbtn()
{
    return !drunk_toggle_state[9] != !rPbtn();
}

void eat_buttons()
{
    rAbtn();
    rBbtn();
    rSbtn();
    rMbtn();
    rLbtn();
    rRbtn();
    rPbtn();
    rEx1btn();
    rEx2btn();
    rEx3btn();
    rEx4btn();
}

bool ReadKey(int k)
{
    if(key[k])
    {
        key[k]=0;
        return true;
    }
    
    return false;
}

void zc_putpixel(int layer, int x, int y, int cset, int color, int timer)
{
    timer=timer;
    particles.add(new particle(fix(x), fix(y), layer, cset, color));
}

// these are here so that copy_dialog won't choke when compiling zelda
int d_alltriggerbutton_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_comboa_radio_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_comboabutton_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssdn_btn_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssdn_btn2_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssdn_btn3_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssdn_btn4_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_sslt_btn_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_sslt_btn2_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_sslt_btn3_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_sslt_btn4_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssrt_btn_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssrt_btn2_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssrt_btn3_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssrt_btn4_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssup_btn_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssup_btn2_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssup_btn3_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_ssup_btn4_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_tri_edit_proc(int, DIALOG*, int)
{
    return D_O_K;
}

int d_triggerbutton_proc(int, DIALOG*, int)
{
    return D_O_K;
}

/*** end of zc_sys.cc ***/

