//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.cc
//
//  Main code for Zelda Classic. Originally written in
//  SPHINX C--, now rewritten in DJGPP with Allegro.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <sdl/sdl.h>
#include <string>
#include <map>
#include <vector>

#include "zc_alleg.h"

#include <png.h>
#include <pngconf.h>
#include <stdlib.h>

#include <loadpng.h>
#include <jpgalleg.h>

#include "zc_malloc.h"
#include "mem_debug.h"
#include "zscriptversion.h"
#include "zcmusic.h"
#include "zdefs.h"
#include "zelda.h"

zcmodule moduledata;
ZModule zcm;
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "aglogo.h"
#include "zsys.h"
#include "qst.h"
#include "matrix.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "save_gif.h"
#include "load_gif.h" // not really needed; we're just saving GIF files in ZC.
#include "fontsdat.h"
#include "particles.h"
#include "gamedata.h"
#include "ffscript.h"
#include "ConsoleLogger.h"
extern FFScript FFCore;

#ifdef _WIN32
extern CConsoleLoggerEx zscript_coloured_console;
#endif

#include "init.h"
#include <assert.h>
#include "zc_array.h"
#include "rendertarget.h"
#include "zconsole.h"
#include "win32.h"
#include "vectorset.h"
#include "single_instance.h"

#include "metadata/sigs/devsig.h.sig"
#include "metadata/versionsig.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define stricmp _stricmp
#endif

// MSVC fix
#if _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif

extern char zc_builddate[80];
extern char zc_aboutstr[80];

ZCMUSIC *zcmusic = NULL;
zinitdata zinit;
int colordepth;
int db=0;
//zinitdata  zinit;
int detail_int[10];                                         //temporary holder for things you want to detail
int lens_hint_item[MAXITEMS][2];                            //aclk, aframe
int lens_hint_weapon[MAXWPNS][5];                           //aclk, aframe, dir, x, y
int cheat_modifier_keys[4]; //two options each, default either control and either shift
int strike_hint_counter=0;
int strike_hint_timer=0;
int strike_hint;
int slot_arg, slot_arg2;
char save_file_name[1024] = "zc.sav";
//char *SAVE_FILE = (char *)"zc.sav";
char *SAVE_FILE = NULL;

CScriptDrawingCommands script_drawing_commands;

using std::string;
using std::pair;
extern std::map<int, pair<string,string> > ffcmap;

int zq_screen_w, zq_screen_h;
int passive_subscreen_height=56;
int original_playing_field_offset=56;
int playing_field_offset=original_playing_field_offset;
int passive_subscreen_offset=0;
extern word quest_header_zelda_version; //2.53 ONLY. In 2.55, we have an array for this in FFCore! -Z
extern word quest_header_zelda_build; //2.53 ONLY. In 2.55, we have an array for this in FFCore! -Z
extern int directItem;
extern int directItemA;
extern int directItemB;
extern byte emulation_patches[emuLAST];
extern int hangcount;
bool is_large=false;
char __isZQuest = 0; //Shared functionscan reference this. -Z

bool standalone_mode=false;
char *standalone_quest=NULL;
bool skip_title=false;

int favorite_combos[MAXFAVORITECOMBOS];
int favorite_comboaliases[MAXFAVORITECOMBOALIASES];

void playLevelMusic();

//Prevent restarting during ending sequence from creating a rect clip
int draw_screen_clip_rect_x1=0;
int draw_screen_clip_rect_x2=255;
int draw_screen_clip_rect_y1=0;
int draw_screen_clip_rect_y2=223;

volatile int logic_counter=0;
bool trip=false;
void update_logic_counter()
{
    ++logic_counter;
}
END_OF_FUNCTION(update_logic_counter)

#ifdef _SCRIPT_COUNTER
volatile int script_counter=0;
void update_script_counter()
{
    ++script_counter;
}
END_OF_FUNCTION(update_script_counter)
#endif

void throttleFPS()
{
#ifdef _WIN32           // TEMPORARY!! -Trying to narrow down a win10 bug that affects performance.
    timeBeginPeriod(1); // Basically, jist is that other programs can affect the FPS of ZC in weird ways. (making it better for example... go figure)
#endif

    if(Throttlefps ^ (key[KEY_TILDE]!=0))
    {
        if(zc_vsync == FALSE)
        {
			int t = 0;
            while(logic_counter < 1)
			{
				// bugfix: win xp/7/8 have incompatible timers.
				// preserve 60 fps and CPU based on user settings. -Gleeok
				int ms = t >= 16 ? 0 : frame_rest_suggest;
                rest(ms);
				t += frame_rest_suggest;
			}
        }
        else
        {
            vsync();
        }
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif

    logic_counter = 0;
}

int onHelp()
{
    //  restore_mouse();
    //  doHelp(vc(15),vc(0));
    return D_O_K;
}
int d_dropdmaplist_proc(int ,DIALOG *,int)
{
    return D_O_K;
}

static char dmap_str_buf[37];
int dmap_list_size=MAXDMAPS;
bool dmap_list_zero=true;

const char *dmaplist(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,dmap_list_size-1);
        sprintf(dmap_str_buf,"%3d-%s",index+(dmap_list_zero?0:1), DMaps[index].name);
        return dmap_str_buf;
    }
    
    *list_size=dmap_list_size;
    return NULL;
}

int startdmapxy[6] = {0,0,0,0,0,0};

/**********************************/
/******** Global Variables ********/
/**********************************/

int curr_tb_page=0;
bool triplebuffer_not_available=false;

RGB_MAP rgb_table;
COLOR_MAP trans_table, trans_table2;

BITMAP     *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *screen2, *fps_undo, *msgdisplaybuf, *pricesdisplaybuf, *tb_page[3], *real_screen, *temp_buf, *prim_bmp;
BITMAP *zcmouse[4];
DATAFILE   *data, *sfxdata, *fontsdata, *mididata;
FONT       *nfont, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
           *goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont;
PALETTE    RAMpal;
byte       *colordata, *trashbuf;
//byte       *tilebuf;
itemdata   *itemsbuf;
wpndata    *wpnsbuf;
comboclass *combo_class_buf;
guydata    *guysbuf;
item_drop_object    item_drop_sets[MAXITEMDROPSETS];
ZCHEATS    zcheats;
byte       use_cheats;
byte       use_tiles;
char       palnames[MAXLEVELS][PALNAMESIZE];
/*
tiledata   *newtilebuf, *grabtilebuf;
newcombo   *combobuf;
word animated_combo_table[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table4[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos;
word animated_combo_table2[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table24[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos2;
bool blank_tile_table[NEWMAXTILES];                         //keeps track of blank tiles
bool blank_tile_quarters_table[NEWMAXTILES*4];              //keeps track of blank tiles
*/
bool ewind_restart=false;


word     msgclk, msgstr,
         msgpos,	// screen position of the next character.
         msgptr,	// position within the string of the next character. <MSGSIZE.
         msgcolour,	// colour to use for the displayed text.
         msgspeed,	// delay between each character (5 = default).
         msg_w,
         msg_h,
         msg_count,
         cursor_x,
         cursor_y,
         msg_xpos=0,
         msg_ypos=0,
         msgorig=0;
bool msg_onscreen = false, msg_active = false, msgspace = false;
BITMAP   *msgbmpbuf = NULL;
FONT	 *msgfont;
word     door_combo_set_count;
word     introclk, intropos, dmapmsgclk, linkedmsgclk;
short    lensclk;
int     lensid; // Lens's item id. -1 if lens is off.
int    Bpos;
byte screengrid[22];
byte ffcgrid[4];
bool halt=false;
bool screenscrolling=false;
bool close_button_quit=false;
PALETTE tempbombpal;
bool usebombpal;

int readsize, writesize;
bool fake_pack_writing=false;
combo_alias combo_aliases[MAXCOMBOALIASES];  //Temporarily here so ZC can compile. All memory from this is freed after loading the quest file.

SAMPLE customsfxdata[WAV_COUNT];
unsigned char customsfxflag[WAV_COUNT>>3];
int sfxdat=1;
BITMAP *hw_screen;
int zqwin_scale;

int jwin_pal[jcMAX];
int gui_colorset=0;
int fullscreen;
byte frame_rest_suggest=0,forceExit=0,zc_vsync=0;
byte disable_triplebuffer=0,can_triplebuffer_in_windowed_mode=0;
byte zc_color_depth=8;
byte use_debug_console=0, use_win32_proc=1, zscript_debugger = 0; //windows-build configs
int homescr,currscr,frame=0,currmap=0,dlevel,warpscr,worldscr;
int newscr_clk=0,opendoors=0,currdmap=0,fadeclk=-1,currgame=0,listpos=0;
int lastentrance=0,lastentrance_dmap=0,prices[3]={0},loadside, Bwpn = 0, Awpn = 0;
int digi_volume,midi_volume,sfx_volume,emusic_volume,currmidi,hasitem,whistleclk,pan_style;
bool analog_movement=true;
int joystick_index=0,Akey,Bkey,Skey,Lkey,Rkey,Pkey,Exkey1,Exkey2,Exkey3,Exkey4,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Exbtn1,Exbtn2,Exbtn3,Exbtn4,Quit=0;
int js_stick_1_x_stick, js_stick_1_x_axis, js_stick_1_x_offset;
int js_stick_1_y_stick, js_stick_1_y_axis, js_stick_1_y_offset;
int js_stick_2_x_stick, js_stick_2_x_axis, js_stick_2_x_offset;
int js_stick_2_y_stick, js_stick_2_y_axis, js_stick_2_y_offset;
int DUkey, DDkey, DLkey, DRkey, DUbtn, DDbtn, DLbtn, DRbtn, ss_after, ss_speed, ss_density, ss_enable;
int hs_startx, hs_starty, hs_xdist, hs_ydist, clockclk, clock_zoras[eMAXGUYS];
int cheat_goto_dmap=0, cheat_goto_screen=0, currcset;
int gfc, gfc2, pitx, pity, refill_what, refill_why, heart_beep_timer=0, new_enemy_tile_start=1580;
int nets=1580, magicitem=-1,nayruitem=-1, title_version, magiccastclk, quakeclk=0, wavy=0, castx, casty, df_x, df_y, nl1_x, nl1_y, nl2_x, nl2_y;
int magicdrainclk=0, conveyclk=3, memrequested=0;
float avgfps=0;
dword fps_secs=0;
bool do_cheat_goto=false, do_cheat_light=false;
int checkx, checky;
int loadlast=0;
int skipcont=0;
int skipicon=0;

bool show_layer_0=true, show_layer_1=true, show_layer_2=true, show_layer_3=true, show_layer_4=true, show_layer_5=true, show_layer_6=true,
//oveheard combos     //pushblocks
     show_layer_over=true, show_layer_push=true, show_sprites=true, show_ffcs=true, show_hitboxes=false, show_walkflags=false, show_ff_scripts=false;


bool Throttlefps, ClickToFreeze=false, Paused=false, Advance=false, ShowFPS, Showpal=false, disableClickToFreeze=false;
bool Playing, FrameSkip=false, TransLayers;
bool __debug=false,debug_enabled;
bool refreshpal,blockpath,loaded_guys,freeze_guys,
     loaded_enemies,drawguys,details=false,watch;
bool darkroom=false,naturaldark=false,BSZ;                         //,NEWSUBSCR;
bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown,Ex1down,Ex2down,Ex3down,Ex4down,AUdown,ADdown,ALdown,ARdown,F12,F11, F5,keyI, keyQ,
     SystemKeys=true,NESquit,volkeys,useCD=false,boughtsomething=false,
     fixed_door=false, hookshot_used=false, hookshot_frozen=false,
     pull_link=false, add_chainlink=false, del_chainlink=false, hs_fix=false,
     cheat_superman=false, gofast=false, checklink=true, didpit=false, heart_beep=true,
     pausenow=false, castnext=false, add_df1asparkle, add_df1bsparkle, add_nl1asparkle, add_nl1bsparkle, add_nl2asparkle, add_nl2bsparkle,
     is_on_conveyor, activated_timed_warp=false;

byte COOLSCROLL;

int  add_asparkle=0, add_bsparkle=0;
int SnapshotFormat, NameEntryMode=0;

char   zeldadat_sig[52];
char   sfxdat_sig[52];
char   fontsdat_sig[52];
char   cheat_goto_dmap_str[4];
char   cheat_goto_screen_str[3];
short  visited[6];
byte   guygrid[176];
mapscr tmpscr[2];
mapscr tmpscr2[6];
mapscr tmpscr3[6];
gamedata *game=NULL;
ffscript *ffscripts[NUMSCRIPTFFC];
ffscript *itemscripts[NUMSCRIPTITEM];
ffscript *globalscripts[NUMSCRIPTGLOBAL];

//If only...
ffscript *guyscripts[NUMSCRIPTGUYS];
ffscript *wpnscripts[NUMSCRIPTWEAPONS];
ffscript *linkscripts[NUMSCRIPTLINK];
ffscript *screenscripts[NUMSCRIPTSCREEN];

extern refInfo globalScriptData;
extern word g_doscript;
extern bool global_wait;

//ZScript array storage
std::vector<ZScriptArray> globalRAM;
ZScriptArray localRAM[MAX_ZCARRAY_SIZE];
byte arrayOwner[MAX_ZCARRAY_SIZE];

//script bitmap drawing
ZScriptDrawingRenderTarget* zscriptDrawingRenderTarget;

DebugConsole DebugConsole::singleton = DebugConsole();


void setZScriptVersion(int s_version)
{
    ZScriptVersion::setVersion(s_version);
}

void initZScriptArrayRAM(bool firstplay)
{
    for(word i = 0; i < MAX_ZCARRAY_SIZE; i++)
    {
        localRAM[i].Clear();
        arrayOwner[i]=255;
    }
    
    if(game->globalRAM.size() != 0)
        game->globalRAM.clear();
        
    if(firstplay)
    {
        //leave to global script ~Init to allocate global memory first time round
        game->globalRAM.resize(getNumGlobalArrays());
    }
    else
    {
        //allocate from save file
        game->globalRAM.resize(saves[currgame].globalRAM.size());
        
        for(dword i = 0; i < game->globalRAM.size(); i++)
        {
#ifdef _DEBUGARRAYALLOC
            al_trace("Global Array: %i\n",i);
#endif
            ZScriptArray &from = saves[currgame].globalRAM[i];
            ZScriptArray &to = game->globalRAM[i];
            to.Resize(from.Size());
            
            for(dword j = 0; j < from.Size(); j++)
            {
#ifdef _DEBUGARRAYALLOC
                al_trace("Element: %i\nInit: %i, From save file: %i\n", j, to[j], from[j]);
#endif
                to[j] = from[j];
            }
        }
    }
}

void initZScriptGlobalRAM()
{
    g_doscript = 1;
    globalScriptData.Clear();
    clear_global_stack();
}

dword getNumGlobalArrays()
{
    word scommand, pc = 0, ret = 0;
    
    do
    {
        scommand = globalscripts[GLOBAL_SCRIPT_INIT][pc].command;
        
        if(scommand == ALLOCATEGMEMV || scommand == ALLOCATEGMEMR)
            ret++;
            
        pc++;
    }
    while(scommand != 0xFFFF);
    
    return ret;
}

//movingblock mblock2; //mblock[4]?
//LinkClass   Link;

int resx,resy,scrx,scry;
bool sbig;                                                  // big screen
bool sbig2;													// bigger screen
int screen_scale = 2; //default = 2 (640x480)
bool scanlines;                                             //do scanlines if sbig==1
bool toogam=false;
bool ignoreSideview=false;

int cheat = (DEVLEVEL > 1) ? 4 : 0;                            // 0 = none; 1,2,3,4 = cheat level


int mouse_down;                                             // used to hold the last reading of 'gui_mouse_b()' status
int idle_count, active_count;


// quest file data
zquestheader QHeader;
byte                quest_rules[QUESTRULES_SIZE];
byte                extra_rules[EXTRARULES_SIZE];
byte                midi_flags[MIDIFLAGS_SIZE];
byte                music_flags[MUSICFLAGS_SIZE];
word                map_count;
MsgStr              *MsgStrings;
int					msg_strings_size;
DoorComboSet        *DoorComboSets;
dmap                *DMaps;
miscQdata           QMisc;
std::vector<mapscr> TheMaps;
zcmap               *ZCMaps;
byte                *quest_file;
dword               quest_map_pos[MAPSCRS*MAXMAPS2];

char     *qstpath=NULL;
char     *qstdir=NULL;
gamedata *saves=NULL;

volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync=0;

/**********************************/
/*********** Misc Data ************/
/**********************************/

const char startguy[8] = {-13,-13,-13,-14,-15,-18,-21,-40};
const char gambledat[12*6] =
{
    20,-10,-10, 20,-10,-10, 20,-40,-10, 20,-10,-40,
    50,-10,-10, 50,-10,-10, 50,-40,-10, 50,-10,-40,
    -10,20,-10, -10,20,-10, -40,20,-10, -10,20,-40,
    -10,50,-10, -10,50,-10, -40,50,-10, -10,50,-40,
    -10,-10,20, -10,-10,20, -10,-40,20, -40,-10,20,
    -10,-10,50, -10,-10,50, -10,-40,50, -40,-10,50
};
const byte stx[4][9] =
{
    { 48, 80, 80, 96,112,144,160,160,192},
    { 48, 80, 80, 96,128,144,160,160,192},
    { 80, 80,128,128,160,160,192,192,208},
    { 32, 48, 48, 80, 80,112,112,160,160}
};
const byte sty[4][9] =
{
    {112, 64,128, 96, 80, 96, 64,128,112},
    { 48, 32, 96, 64, 80, 64, 32, 96, 48},
    { 32,128, 64, 96, 64, 96, 48,112, 80},
    { 80, 48,112, 64, 96, 64, 96, 32,128}
};

const byte ten_rupies_x[10] = {120,112,128,96,112,128,144,112,128,120};
const byte ten_rupies_y[10] = {49,65,65,81,81,81,81,97,97,113};

zctune tunes[MAXMIDIS] =
{
    // (title)                              (s) (ls) (le) (l) (vol) (midi)(fmt)
    zctune((char *)"Zelda - Dungeon",     0,  -1,  -1,  1,  176,  NULL, 0),
    zctune((char *)"Zelda - Ending",      0, 129, 225,  1,  160,  NULL, 0),
    zctune((char *)"Zelda - Game Over",   0,  -1,  -1,  1,  224,  NULL, 0),
    zctune((char *)"Zelda - Level 9",     0,  -1,  -1,  1,  255,  NULL, 0),
    zctune((char *)"Zelda - Overworld",   0,  17,  -1,  1,  208,  NULL, 0),
    zctune((char *)"Zelda - Title",       0,  -1,  -1,  0,  168,  NULL, 0),
    zctune((char *)"Zelda - Triforce",    0,  -1,  -1,  0,  168,  NULL, 0)
};

// emusic enhancedMusic[MAXMUSIC];

FONT *setmsgfont()
{
    switch(MsgStrings[msgstr].font)
    {
    default:
        return zfont;
        
    case font_z3font:
        return z3font;
        
    case font_z3smallfont:
        return z3smallfont;
        
    case font_deffont:
        return deffont;
        
    case font_lfont:
        return lfont;
        
    case font_lfont_l:
        return lfont_l;
        
    case font_pfont:
        return pfont;
        
    case font_mfont:
        return mfont;
        
    case font_ztfont:
        return ztfont;
        
    case font_sfont:
        return sfont;
        
    case font_sfont2:
        return sfont2;
        
    case font_spfont:
        return spfont;
        
    case font_ssfont1:
        return ssfont1;
        
    case font_ssfont2:
        return ssfont2;
        
    case font_ssfont3:
        return ssfont3;
        
    case font_ssfont4:
        return ssfont4;
        
    case font_gblafont:
        return gblafont;
        
    case font_goronfont:
        return goronfont;
        
    case font_zoranfont:
        return zoranfont;
        
    case font_hylian1font:
        return hylian1font;
        
    case font_hylian2font:
        return hylian2font;
        
    case font_hylian3font:
        return hylian3font;
        
    case font_hylian4font:
        return hylian4font;
        
    case font_gboraclefont:
        return gboraclefont;
        
    case font_gboraclepfont:
        return gboraclepfont;
        
    case font_dsphantomfont:
        return dsphantomfont;
        
    case font_dsphantompfont:
        return dsphantompfont;
    }
}

void donewmsg(int str)
{
    //al_trace("donewmsg %d\n",str);
    if(msg_onscreen || msg_active)
        dismissmsg();
        
    linkedmsgclk=0;
    msg_active = true;
    // Don't set msg_onscreen - not onscreen just yet
    msgstr = str;
    msgorig = msgstr;
    msgfont = setmsgfont();
    msgcolour=QMisc.colors.msgtext;
    msgspeed=zinit.msg_speed;
    
    if(introclk==0 || (introclk>=72 && dmapmsgclk==0))
        clear_bitmap(msgdisplaybuf);
        
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(msgbmpbuf);
    
    if(MsgStrings[msgstr].tile!=0)
    {
        frame2x2(msgbmpbuf,&QMisc,0,0,MsgStrings[msgstr].tile,MsgStrings[msgstr].cset,
                 (MsgStrings[msgstr].w>>3)+2,(MsgStrings[msgstr].h>>3)+2,0,true,0);
    }
    
    msgclk=msgpos=msgptr=0;
    msgspace=true;
    msg_w=MsgStrings[msgstr].w;
    msg_h=MsgStrings[msgstr].h;
    msg_xpos=MsgStrings[msgstr].x;
    msg_ypos=MsgStrings[msgstr].y;
    cursor_x=0;
    cursor_y=0;
}

// Called to make a message disappear
void dismissmsg()
{
    linkedmsgclk=0;
    msgstr = msgclk=msgpos=msgptr=0;
    cursor_x=0;
    cursor_y=0;
    msg_onscreen = msg_active = false;
    //Link.finishedmsg(); //Not possible?
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
}

void dointro()
{
    if(game->visited[currdmap]!=1 || (DMaps[currdmap].flags&dmfALWAYSMSG)!=0 || emulation_patches[emu250DMAPINTOREPEAT])
    {
        dmapmsgclk=0;
        game->visited[currdmap]=1;
        introclk=intropos=0;
    }
    else
    {
        introclk=intropos=72;
    }
}

bool bad_version(int version)
{
    // minimum zquest version allowed for any quest file
    if(version < 0x183)
        return true;
        
    return false;
}

extern char *weapon_string[];
extern char *item_string[];
extern char *sfx_string[];
extern char *guy_string[];


/**********************************/
/******* Other Source Files *******/
/**********************************/

bool get_debug()
{
    //return false; //remove later
    return __debug;
}

void set_debug(bool d)
{
    __debug=d;
    return;
}

void hit_close_button()
{
    close_button_quit=true;
    return;
}

void zprint(const char * const format,...)
{
    if(get_bit(quest_rules,qr_SCRIPTERRLOG) || DEVLEVEL > 0)
    {
        
        char buf[2048];
        
        va_list ap;
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        al_trace("%s",buf);
        
        if(zconsole)
	{
            printf("%s",buf);
	}
	if ( zscript_debugger ) 
	{
		#ifdef _WIN32
		zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"%s",buf);
		#endif
	}
	
    }
}

//Always prints
void zprint2(const char * const format,...)
{
    {
        
        char buf[2048];
        
        va_list ap;
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        al_trace("%s",buf);
        
        if(zconsole)
	{
            printf("%s",buf);
	}
	if ( zscript_debugger ) 
	{
		#ifdef _WIN32
		zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"%s",buf);
		#endif
	}
	
    }
}

void Z_eventlog(const char *format,...)
{
    if(get_bit(quest_rules,qr_LOG) || DEVLEVEL > 0 )
    {
        char buf[2048];
        
        va_list ap;
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        al_trace("%s",buf);
        
        if(zconsole)
            printf("%s",buf);
	//Add event console here. -Z 
	#ifdef _WIN32
		if ( zscript_debugger ) {zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"%s",buf); }
		#endif
    }
}

// Yay, more extern globals.
extern byte curScriptType;
extern word curScriptNum;
extern std::map<int, std::pair<std::string, std::string> > ffcmap;
extern std::map<int, std::pair<std::string, std::string> > globalmap;
extern std::map<int, std::pair<std::string, std::string> > itemmap;

void Z_scripterrlog(const char * const format,...)
{
    if(get_bit(quest_rules,qr_SCRIPTERRLOG) || DEVLEVEL > 0)
    {
       
        switch(curScriptType)
        {
		case SCRIPT_GLOBAL:
		    al_trace("Global script %u (%s): ", curScriptNum+1, globalmap[curScriptNum].second.c_str());
			#ifdef _WIN32
			if ( zscript_debugger ) {zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | 
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"Global script %u (%s): \n", 
				curScriptNum+1, globalmap[curScriptNum].second.c_str()); }
			#endif
		    break;
		    
		case SCRIPT_FFC:
		    al_trace("FFC script %u (%s): ", curScriptNum, ffcmap[curScriptNum-1].second.c_str());
		    
			#ifdef _WIN32
			if ( zscript_debugger ) {zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | 
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"FFC script %u (%s): ", curScriptNum, ffcmap[curScriptNum-1].second.c_str());}
			#endif
		break;
		    
		case SCRIPT_ITEM:
		    al_trace("Item script %u (%s): ", curScriptNum, itemmap[curScriptNum-1].second.c_str());
			#ifdef _WIN32
			if ( zscript_debugger ) {zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | 
				CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"Item script %u (%s): ", curScriptNum, itemmap[curScriptNum-1].second.c_str());}
			#endif
		break;
		default: break;
	
        }
        
        char buf[2048];
        
        va_list ap;
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        al_trace("%s",buf);
        
	if(zconsole)
	{
            printf("%s",buf);
	}
	if ( zscript_debugger ) 
	{
		#ifdef _WIN32
		zscript_coloured_console.cprintf((CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | 
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK),"%s",buf);
		#endif
	}
    }
}


bool blockmoving;
#include "sprite.h"
movingblock mblock2;                                        //mblock[4]?

sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;

#include "zc_custom.h"
#include "link.h"
LinkClass   Link;

#include "maps.h"
#include "subscr.h"
#include "guys.h"

#include "title.h"
#include "ending.h"

#include "zc_sys.h"

// Wait... this is only used by ffscript.cpp!?
void addLwpn(int x,int y,int z,int id,int type,int power,int dir, int parentid)
{
    Lwpns.add(new weapon((fix)x,(fix)y,(fix)z,id,type,power,dir,-1,parentid));
}

void ALLOFF(bool messagesToo, bool decorationsToo)
{
    if(messagesToo)
    {
        clear_bitmap(msgdisplaybuf);
        set_clip_state(msgdisplaybuf, 1);
    }
    
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    
    if(items.idCount(iPile))
    {
        loadlvlpal(DMaps[currdmap].color);
    }
    
    items.clear();
    guys.clear();
    Lwpns.clear();
    Ewpns.clear();
    chainlinks.clear();
    clearScriptHelperData();
    
    if(decorationsToo)
        decorations.clear();
        
    particles.clear();
    
    if(Link.getNayrusLoveShieldClk())
    {
        Link.setNayrusLoveShieldClk(Link.getNayrusLoveShieldClk());
    }
    
    Link.resetflags(false);
    Link.reset_hookshot();
    linkedmsgclk=0;
    add_asparkle=0;
    add_bsparkle=0;
    add_df1asparkle=false;
    add_df1bsparkle=false;
    add_nl1asparkle=false;
    add_nl1bsparkle=false;
    add_nl2asparkle=false;
    add_nl2bsparkle=false;
    //  for(int i=0; i<1; i++)
    mblock2.clk=0;
    dismissmsg();
    fadeclk=-1;
    introclk=intropos=72;
    
    lensclk = 0;
    lensid=-1;
    drawguys=Udown=Ddown=Ldown=Rdown=Adown=Bdown=Sdown=true;
    
    if(watch && !cheat_superman)
    {
        Link.setClock(false);
    }
    
    //  if(watch)
    //    Link.setClock(false);
    watch=freeze_guys=loaded_guys=loaded_enemies=blockpath=false;
    stop_sfx(WAV_BRANG);
    
    for(int i=0; i<176; i++)
        guygrid[i]=0;
        
    sle_clk=0;
    blockmoving=false;
    fairy_cnt=0;
    
    if(usebombpal)
    {
        memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
        refreshpal=true;
        usebombpal=false;
    }
}
void centerLink()
{
    Link.setX(120);
    Link.setY(80);
}
fix  LinkX()
{
    return Link.getX();
}
fix  LinkY()
{
    return Link.getY();
}
fix  LinkZ()
{
    return Link.getZ();
}
int  LinkHClk()
{
    return Link.getHClk();
}
int  LinkAction()
{
    return Link.getAction();
}
int  LinkCharged()
{
    return Link.isCharged();
}
int  LinkNayrusLoveShieldClk()
{
    return Link.getNayrusLoveShieldClk();
}
int  LinkHoverClk()
{
    return Link.getHoverClk();
}
int  LinkSwordClk()
{
    return Link.getSwordClk();
}
int  LinkItemClk()
{
    return Link.getItemClk();
}
void setSwordClk(int newclk)
{
    Link.setSwordClk(newclk);
}
void setItemClk(int newclk)
{
    Link.setItemClk(newclk);
}
int  LinkLStep()
{
    return Link.getLStep();
}
void LinkCheckItems()
{
    Link.checkitems();
}
bool LinkGetDontDraw()
{
    return Link.getDontDraw();
}
fix  GuyX(int j)
{
    return guys.getX(j);
}
fix  GuyY(int j)
{
    return guys.getY(j);
}
int  GuyID(int j)
{
    return guys.getID(j);
}
int  GuyMisc(int j)
{
    return guys.getMisc(j);
}
bool  GuySuperman(int j)
{
    if((j>=guys.Count())||(j<0))
    {
        return true;
    }
    
    return ((enemy*)guys.spr(j))->superman !=0;
}

int  GuyCount()
{
    return guys.Count();
}
void StunGuy(int j,int stun)
{
    if(stun<=0) return;
    
    if(((enemy*)guys.spr(j))->z==0 && canfall(((enemy*)guys.spr(j))->id))
    {
        ((enemy*)guys.spr(j))->stunclk=zc_min(360,stun*4);
        ((enemy*)guys.spr(j))->fall=-zc_min(FEATHERJUMP,(stun*8)+rand()%5);
    }
}

fix LinkModifiedX()
{
    return Link.getModifiedX();
}
fix LinkModifiedY()
{
    return Link.getModifiedY();
}
int LinkDir()
{
    return Link.getDir();
}
void add_grenade(int wx, int wy, int wz, int size, int parentid)
{
    if(size)
    {
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wSBomb,0,16*DAMAGE_MULTIPLIER,LinkDir(),
                             -1, parentid));
        Lwpns.spr(Lwpns.Count()-1)->id=wSBomb;
    }
    else
    {
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wBomb,0,4*DAMAGE_MULTIPLIER,LinkDir(),
                             -1, parentid));
        Lwpns.spr(Lwpns.Count()-1)->id=wBomb;
    }
    
    Lwpns.spr(Lwpns.Count()-1)->clk=48;
    Lwpns.spr(Lwpns.Count()-1)->misc=50;
}

fix distance(int x1, int y1, int x2, int y2)

{
    return (fix)sqrt(pow((double)abs(x1-x2),2)+pow((double)abs(y1-y2),2));
}

bool getClock()
{
    return Link.getClock();
}
void setClock(bool state)
{
    Link.setClock(state);
}
void CatchBrang()
{
    Link.Catch();
}

/**************************/
/***** Main Game Code *****/
/**************************/

int load_quest(gamedata *g, bool report)
{
    chop_path(qstpath);
    char *tempdir=(char *)"";
#ifndef ALLEGRO_MACOSX
    tempdir=qstdir;
#endif
    
    if(g->get_quest()<255)
    {
        // Check the ZC directory first for 1st-4th quests; check qstdir if they're not there
        sprintf(qstpath, "%s.qst", ordinal(g->get_quest()));
        
        if(!exists(qstpath))
        {
            sprintf(qstpath,"%s%s.qst",tempdir,ordinal(g->get_quest()));
        }
    }
    else
    {
        if(is_relative_filename(g->qstpath))
        {
            sprintf(qstpath,"%s%s",qstdir,g->qstpath);
        }
        else
        {
            sprintf(qstpath,"%s", g->qstpath);
        }

		// ZC paths are retarded.
		// This is just an awful hack, and generally some of the worst code ever written, but it only ever gets run
		// when there would be a "file not found error"... (it's easier than dealing with everything else. Sue me.)
		//
		// *side note*
		// It does have a few perks though: You can now move around entire folders and sub-folders from one ZC directory to the next,
		// and we can find them! You can even put all the ZC quests into different sub-directories and share save files, etc..
		// ~Gleeok
		if(!exists(qstpath))
		{
			al_trace("File not found \"%s\". Searching...\n", qstpath);

			if(exists(g->qstpath)) //not found? -try this place first:
			{
				sprintf(qstpath,"%s", g->qstpath);
				al_trace("Set quest path to \"%s\".\n", qstpath);
			}
			else // Howsabout in here?
			{
				std::string gQstPath = g->qstpath;
				size_t bs1 = 0;
				size_t bs2 = std::string::npos;

				// Keep searching through every parent directory as if it was the current one.
				while(bs1 != std::string::npos || bs2 != std::string::npos)
				{
					bs1 = gQstPath.find_first_of('/');
					if(bs1 != std::string::npos)
					{
						gQstPath = gQstPath.substr(bs1 + 1, std::string::npos);
					}
					else
					{
						bs2 = gQstPath.find_first_of('\\');
						if(bs2 != std::string::npos)
							gQstPath = gQstPath.substr(bs2 + 1, std::string::npos);
					}

					if(exists(gQstPath.c_str())) //Quick! Try it now!
					{
						sprintf(qstpath,"%s", gQstPath.c_str());
						al_trace("Set quest path to \"%s\".\n", qstpath);
						break;
					}
					else //Still no dice eh?
					{
						char cwdbuf[260];
						memset(cwdbuf,0,260*sizeof(char));
#ifdef _ALLEGRO_WINDOWS
						_getcwd(cwdbuf, 260);
#else
                        getcwd(cwdbuf, 260);
#endif

						std::string path = cwdbuf;
						std::string fn;

						if(path.size() != 0 && 
							!(*(path.end()-1)=='/' || *(path.end()-1)=='\\')
							)
						{
							path += '/';
						}

						fn = path + gQstPath;

						if(exists(fn.c_str())) //Last chance for hookers and blackjack truck stop
						{
							sprintf(qstpath,"%s", fn.c_str());
							al_trace("Set quest path to \"%s\".\n", qstpath);
							break;
						}
					}
				} //while

			}
		}//end hack

    }
    
    //setPackfilePassword(datapwd);
    byte skip_flags[4];
    
    for(int i=0; i<4; ++i)
    {
        skip_flags[i]=0;
    }
    
    int ret = loadquest(qstpath,&QHeader,&QMisc,tunes+ZC_MIDI_COUNT,false,true,true,true,skip_flags);
    //setPackfilePassword(NULL);
    
    if(!g->title[0] || g->get_hasplayed() == 0)
    {
        strcpy(g->version,QHeader.version);
        strcpy(g->title,QHeader.title);
    }
    else
    {
        if(!ret && strcmp(g->title,QHeader.title))
        {
            ret = qe_match;
        }
    }
    
    if(QHeader.minver[0])
    {
        if(strcmp(g->version,QHeader.minver) < 0)
            ret = qe_minver;
    }
    
    if(ret && report)
    {
        system_pal();
        char buf1[80],buf2[80];
        sprintf(buf1,"Error loading %s:",get_filename(qstpath));
        sprintf(buf2,"%s",qst_error[ret]);
        jwin_alert("File error",buf1,buf2,qstpath,"OK",NULL,13,27,lfont);
        
        if(standalone_mode)
        {
            exit(1);
        }
        
        game_pal();
    }
    
    return ret;
}

void init_dmap()
{
    // readjust disabled items; could also do dmap-specific scripts here
    for(int i=0; i<MAXITEMS; i++)
    {
        game->items_off[i] &= (~1); // disable last bit - this one is set by dmap
        game->items_off[i] |= DMaps[currdmap].disableditems[i]; // and reset if required
    }
    
    flushItemCache();
    // also update subscreens
    update_subscreens();
    verifyBothWeapons();
    
    return;
}

int init_game()
{
    srand(time(0));
    //introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
	draw_screen_clip_rect_x1=0; //Prevent the ending sequence from carrying over through 'Reset System' -Z
	draw_screen_clip_rect_x2=255;
	draw_screen_clip_rect_y1=0;
	draw_screen_clip_rect_y2=223;	
	
//Some initialising globals
    didpit=false;
    Link.unfreeze();
    Link.reset_hookshot();
    Link.reset_ladder();
    linkedmsgclk=0;
    blockmoving=false;
    add_asparkle=0;
    add_bsparkle=0;
    add_df1asparkle=false;
    add_df1bsparkle=false;
    add_nl1asparkle=false;
    add_nl1bsparkle=false;
    add_nl2asparkle=false;
    add_nl2bsparkle=false;
    gofast=false;
    //al_trace("Clearing old RenderTarget\n");
    if ( zscriptDrawingRenderTarget ) zscriptDrawingRenderTarget->SetCurrentRenderTarget(-1); //clear the last set Rendertarget between games
    //zscriptDrawingRenderTarget = new ZScriptDrawingRenderTarget();
    
    
    cheat=0;
    wavy=quakeclk=0;
    show_layer_0=show_layer_1=show_layer_2=show_layer_3=show_layer_4=show_layer_5=show_layer_6=true;
    show_layer_over=show_layer_push=show_sprites=show_ffcs=true;
    cheat_superman=do_cheat_light=do_cheat_goto=show_walkflags=show_ff_scripts=show_hitboxes=false;
    
    for(int x = 0; x < MAXITEMS; x++)
    {
        lens_hint_item[x][0]=0;
        lens_hint_item[x][1]=0;
    }
    
    for(int x = 0; x < MAXWPNS; x++)
    {
        lens_hint_weapon[x][0]=0;
        lens_hint_weapon[x][1]=0;
    }
    
    /* Disabling to see if this is causing virus scanner redflags. -Z
//Confuse the cheaters by moving the game data to a random location
    if(game != NULL)
        delete game;
        
    char *dummy = (char *) zc_malloc((rand()%(RAND_MAX/2))+32);
    game = new gamedata;
    game->Clear();
    
    zc_free(dummy);
    */
//Copy saved data to RAM data (but not global arrays)
    game->Copy(saves[currgame]);
    flushItemCache();
    
//Load the quest
    //setPackfilePassword(datapwd);
    int ret = load_quest(game);
    
    if(ret != qe_OK)
    {
        Quit = qERROR;
        //setPackfilePassword(NULL);
        return 1;
    }
    
    //setPackfilePassword(NULL);
    
    char keyfilename[2048]; //master key .key
    char keyfilename2[2048]; //zquest key .zpwd
    char keyfilename3[2048]; //zc cheats only key, .zcheat
    replace_extension(keyfilename, qstpath, "key", 2047);
    replace_extension(keyfilename2, qstpath, "zpwd", 2047);
    replace_extension(keyfilename3, qstpath, "zcheat", 2047);
    bool gotfromkey=false;
    bool gotfrompwdfile=false;
    bool gotfromcheatfile=false;
    
    if(exists(keyfilename))
    {
	    al_trace("Found a Quest Master Key, filename: %s\n", keyfilename);
        char password[32], pwd[32];
        PACKFILE *fp = pack_fopen_password(keyfilename, F_READ,"");
        char msg[80];
        memset(msg,0,80);
        pfread(msg, 80, fp,true);
        
        if(strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
        {
		//al_trace("Found Quest Master Key\n");
            short ver;
            byte  bld;
            p_igetw(&ver,fp,true);
            p_getc(&bld,fp,true);
            memset(password,0,32);
            pfread(password, 30, fp,true);
            /*
            get-questpwd(&QHeader, pwd);
            if (strcmp(pwd,password)==0)
            {
            	gotfromkey=true;
            }
            */
            gotfromkey=check_questpwd(&QHeader, password);
            memset(password,0,32);
            memset(pwd,0,32);
        }
        
        pack_fclose(fp);
	goto skip_keychecks;
    }
    
    if(exists(keyfilename2)) //zquest key...superfluous?
    {
	    al_trace("Found a ZQuest Password Key, filename: %s\n", keyfilename2);
        char password[32], pwd[32];
        PACKFILE *fp = pack_fopen_password(keyfilename2, F_READ,"");
        char msg[80];
        memset(msg,0,80);
        pfread(msg, 80, fp,true);
        
        if(strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
        {
            short ver;
            byte  bld;
            p_igetw(&ver,fp,true);
            p_getc(&bld,fp,true);
            memset(password,0,32);
            pfread(password, 30, fp,true);
            /*
            get-questpwd(&QHeader, pwd);
            if (strcmp(pwd,password)==0)
            {
            	gotfromkey=true;
            }
            */
            gotfrompwdfile=check_questpwd(&QHeader, password);
            memset(password,0,32);
            memset(pwd,0,32);
        }
        
        pack_fclose(fp);
    }
    
    if(exists(keyfilename3)) //zc cheat key
    {
	    al_trace("Found a ZC Cheat Key, filename: %s\n", keyfilename3);
	    
        char password[32], pwd[32];
        PACKFILE *fp = pack_fopen_password(keyfilename3, F_READ,"");
        char msg[80];
        memset(msg,0,80);
        pfread(msg, 80, fp,true);
        
        if(strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
        {
		//al_trace("checking .zcheat file header %s","\n");
            short ver;
            byte  bld;
            p_igetw(&ver,fp,true);
            p_getc(&bld,fp,true);
		//al_trace("about to read password\n");
            memset(password,0,32);
		pfread(password, 30, fp,true);
            //pfread(password, 30, fp,true);
		
		//al_trace("making space for hash\n");
		char unhashed_pw[32];
		memset(unhashed_pw,0,32);
		
            
		
		char hashmap = 'Z';
		hashmap += 'Q';
		hashmap += 'U';
		hashmap += 'E';
		hashmap += 'S';
		hashmap += 'T';
		
		//al_trace("applying reverse hash\n");
		for ( int q = 0; q < 32; q++ ) 
		{
			unhashed_pw[q] = password[q] - hashmap;
		}
		//al_trace("hashed password is: %s\n", password);
		//al_trace("un-hashed password is: %s\n", unhashed_pw);
		
		
		
            
            //get-questpwd(&QHeader, pwd);
            //if (strcmp(pwd,password)==0)
            //{
            //	gotfromkey=true;
            //}
            
           // gotfromcheatfile=check_questpwd(&QHeader, unhashed_pw);
            memset(password,0,32);
            memset(unhashed_pw,0,32);
            memset(pwd,0,32);
	    cheat=4;
        }
        
        pack_fclose(fp);
    }
    else goto skip_keycheats;
    
    skip_keychecks:
    
    if(gotfromkey)
    {
        cheat=4;
    }
    
    skip_keycheats:
    
    bool firstplay = (game->get_hasplayed() == 0);
    
    
    
    BSZ = get_bit(quest_rules,qr_BSZELDA)!=0;
    //setuplinktiles(zinit.linkanimationstyle);
    
    COOLSCROLL = (get_bit(quest_rules,qr_COOLSCROLL)!=0 ? 1 : 0) |
                 (get_bit(quest_rules,qr_OVALWIPE)!=0 ? 2 : 0) |
                 (get_bit(quest_rules,qr_TRIANGLEWIPE)!=0 ? 4 : 0) |
                 (get_bit(quest_rules,qr_SMASWIPE)!=0 ? 8 : 0);
    identifyCFEnemies();
                 
    //  NEWSUBSCR = get_bit(quest_rules,qr_NEWSUBSCR);
    
    //  homescr = currscr = DMaps[0].cont;
    //  currdmap = warpscr = worldscr=0;
    if(firstplay)
    {
        game->set_continue_dmap(zinit.start_dmap);
        resetItems(game,&zinit,true);
	if ( quest_header_zelda_version < 0x190 ) { game->set_maxbombs(8); al_trace("Starting bombs set to %d for a quest made in ZC %x\n", game->get_maxbombs(), quest_header_zelda_version); }
    }
    
    currdmap = warpscr = worldscr=game->get_continue_dmap();
    init_dmap();
    
    if(game->get_continue_scrn() >= 0x80)
    {
        //if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW || QHeader.zelda_version <= 0x190)
        if((DMaps[currdmap].type&dmfTYPE)==dmOVERW)
        {
            homescr = currscr = DMaps[currdmap].cont;
        }
        else
        {
            homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
        }
    }
    else
    {
        homescr = currscr = game->get_continue_scrn();
    }
    
    lastentrance = currscr;
    game->set_continue_scrn(lastentrance);
    lastentrance_dmap = currdmap;
    currmap = DMaps[currdmap].map;
    dlevel = DMaps[currdmap].level;
    sle_x=sle_y=newscr_clk=opendoors=Bwpn=Bpos=0;
    fadeclk=-1;
    
    if(DMaps[currdmap].flags&dmfVIEWMAP)
    {
        game->maps[(currmap*MAPSCRSNORMAL)+currscr] |= mVISITED;              // mark as visited
    }
    
    for(int i=0; i<6; i++)
    {
        visited[i]=-1;
    }
    
    game->lvlitems[9] &= ~liBOSS;
    
    ALLOFF();
    whistleclk=-1;
    clockclk=0;
    currcset=DMaps[currdmap].color;
    darkroom=naturaldark=false;
    
    tmpscr[0].zero_memory();
    tmpscr[1].zero_memory();
    
    //Script-related nonsense
    script_drawing_commands.Clear();
    
    initZScriptArrayRAM(firstplay);
    initZScriptGlobalRAM();
    global_wait=false;
    
    //loadscr(0,currscr,up);
    loadscr(0,currdmap,currscr,-1,false);
    putscr(scrollbuf,0,0,&tmpscr[0]);
    putscrdoors(scrollbuf,0,0,&tmpscr[0]);
    
    //preloaded freeform combos
    //ffscript_engine(true); Can't do this here! Global arrays haven't been allocated yet... ~Joe
    
    Link.init();
    Link.resetflags(true);
    Link.setEntryPoints(LinkX(),LinkY());
    
    copy_pal((RGB*)data[PAL_GUI].dat,RAMpal);
    loadfullpal();
    ringcolor(false);
    loadlvlpal(DMaps[currdmap].color);
    lighting(false,true);
    wavy=quakeclk=0;
    
    if(firstplay)
    {
        game->set_life(zinit.start_heart*HP_PER_HEART);
    }
    else
    {
        if(game->get_cont_percent())
        {
            if(game->get_maxlife()%HP_PER_HEART==0)
                game->set_life(((game->get_maxlife()*game->get_cont_hearts()/100)/HP_PER_HEART)*HP_PER_HEART);
            else
                game->set_life(game->get_maxlife()*game->get_cont_hearts()/100);
        }
        else
        {
            game->set_life(game->get_cont_hearts()*HP_PER_HEART);
        }
    }
    
    if(firstplay)
        game->set_hasplayed(1);
        
    if(firstplay)
    {
	game->awpn=0;
	game->bwpn=0;
	game->forced_awpn = -1; 
	game->forced_bwpn = -1;    
    }
    
    update_subscreens();
    
    load_Sitems(&QMisc);
    
//load the previous weapons -DD
    bool usesaved = (game->get_quest() == 0xFF); //What was wrong with firstplay?
    int apos = 0;
    int bpos = 0;
    
    if(!get_bit(quest_rules,qr_SELECTAWPN))
    {
        Awpn = selectSword();
        apos = -1;
        bpos = selectWpn_new(SEL_VERIFY_RIGHT, usesaved ? game->bwpn : 0xFF, -1);
    }
    else
    {
        apos = selectWpn_new(SEL_VERIFY_RIGHT, usesaved ? game->awpn : 0xFF);
        bpos = selectWpn_new(SEL_VERIFY_RIGHT, usesaved ? game->bwpn : 0xFF, apos);
        
        if(bpos==0xFF)
        {
            bpos=apos;
            apos=-1;
        }
        
        Awpn = Bweapon(apos);
        directItemA = directItem;
    }
    
    game->awpn = apos;
    game->bwpn = bpos;
    Bwpn = Bweapon(bpos);
    directItemB = directItem;
    directItemA = directItem;
    update_subscr_items();
    
    reset_subscr_items();
    
    Link.setDontDraw(false);
    show_subscreen_dmap_dots=true;
    show_subscreen_items=true;
    show_subscreen_numbers=true;
    show_subscreen_life=true;
    
    Playing=true;
    
    map_bkgsfx(true);
    openscreen();
    show_subscreen_numbers=true;
    show_subscreen_life=true;
    dointro();
    loadguys();
    
    activated_timed_warp=false;
    newscr_clk = frame;
    
    if(isdungeon() && currdmap>0) // currdmap>0 is weird, but at least one quest (Mario's Insane Rampage) depends on it
    {
        Link.stepforward(get_bit(quest_rules,qr_LTTPWALK) ? 11: 12, false);
    }
    
    if(!Quit)
        playLevelMusic();
        
    if(firstplay)
    {
        memset(game->screen_d, 0, MAXDMAPS * 64 * 8 * sizeof(long));
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_INIT);
    }
    else
    {
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_CONTINUE); //Do this after global arrays have been loaded
    }
    
    initZScriptGlobalRAM(); //Call again so we're set up for GLOBAL_SCRIPT_GAME
    ffscript_engine(true);  //Here is a much safer place...
    
    return 0;
}

int cont_game()
{
    //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
    didpit=false;
    Link.unfreeze();
    Link.reset_hookshot();
    Link.reset_ladder();
    linkedmsgclk=0;
    blockmoving=0;
    add_asparkle=0;
    add_bsparkle=0;
    add_df1asparkle=false;
    add_df1bsparkle=false;
    add_nl1asparkle=false;
    add_nl1bsparkle=false;
    add_nl2asparkle=false;
    add_nl2bsparkle=false;
    /*
      if(DMaps[currdmap].cont >= 0x80)
      {
      homescr = currscr = DMaps[0].cont;
      currdmap = warpscr = worldscr=0;
      currmap = DMaps[0].map;
      dlevel = DMaps[0].level;
      }
      */
    currdmap = lastentrance_dmap;
    homescr = currscr = lastentrance;
    currmap = DMaps[currdmap].map;
    dlevel = DMaps[currdmap].level;
    init_dmap();
    
    for(int i=0; i<6; i++)
    {
        visited[i]=-1;
    }
    
    if(dlevel==0)
    {
        game->lvlitems[9]&=~liBOSS;
    }
    
    ALLOFF();
    whistleclk=-1;
    currcset=DMaps[currdmap].color;
    darkroom=naturaldark=false;
    tmpscr[0].zero_memory();
    tmpscr[1].zero_memory();
    
//loadscr(0,currscr,up);
    loadscr(0,currdmap,currscr,-1,false);
    putscr(scrollbuf,0,0,&tmpscr[0]);
    putscrdoors(scrollbuf,0,0,&tmpscr[0]);
    script_drawing_commands.Clear();
    
    //preloaded freeform combos
    ffscript_engine(true);
    
    loadfullpal();
    ringcolor(false);
    loadlvlpal(DMaps[currdmap].color);
    lighting(false,true);
    Link.init();
    wavy=quakeclk=0;
    
    //if(get_bit(zinit.misc,idM_CONTPERCENT))
    if(game->get_cont_percent())
    {
        if(game->get_maxlife()%HP_PER_HEART==0)
            game->set_life(((game->get_maxlife()*game->get_cont_hearts()/100)/HP_PER_HEART)*HP_PER_HEART);
        else
            game->set_life(game->get_maxlife()*game->get_cont_hearts()/100);
    }
    else
    {
        game->set_life(game->get_cont_hearts()*HP_PER_HEART);
    }
        
    /*
      else
      game->life=3*HP_PER_HEART;
      */
    
    //  for(int i=0; i<128; i++)
    //    key[i]=0;
    
    update_subscreens();
    Playing=true;
    map_bkgsfx(true);
    openscreen();
    show_subscreen_numbers=true;
    show_subscreen_life=true;
    dointro();
    loadguys();
    
    if(!Quit)
    {
        //play_DmapMusic();
        playLevelMusic();
        
        if(isdungeon())
            Link.stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
            
        newscr_clk=frame;
        activated_timed_warp=false;
    }
    
    initZScriptGlobalRAM();
    return 0;
}

void restart_level()
{
    blackscr(16,true);
    map_bkgsfx(false);
    
    if(dlevel)
    {
        currdmap = lastentrance_dmap;
        homescr = currscr = lastentrance;
        init_dmap();
    }
    else
    {
        if((DMaps[currdmap].type&dmfTYPE)==dmOVERW)
        {
            homescr = currscr = DMaps[currdmap].cont;
        }
        else
        {
            homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
        }
    }
    
    currmap = DMaps[currdmap].map;
    dlevel = DMaps[currdmap].level;
    
    for(int i=0; i<6; i++)
        visited[i]=-1;
        
    ALLOFF();
    whistleclk=-1;
    darkroom=naturaldark=false;
    tmpscr[0].zero_memory();
    tmpscr[1].zero_memory();
    
    loadscr(0,currdmap,currscr,-1,false);
    putscr(scrollbuf,0,0,&tmpscr[0]);
    putscrdoors(scrollbuf,0,0,&tmpscr[0]);
    //preloaded freeform combos
    ffscript_engine(true);
    
    loadfullpal();
    ringcolor(false);
    loadlvlpal(DMaps[currdmap].color);
    lighting(false,true);
    Link.init();
    currcset=DMaps[currdmap].color;
    openscreen();
    map_bkgsfx(true);
    Link.setEntryPoints(LinkX(),LinkY());
    show_subscreen_numbers=true;
    show_subscreen_life=true;
    loadguys();
    
    if(!Quit)
    {
        //play_DmapMusic();
        playLevelMusic();
        
        if(isdungeon())
            Link.stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
            
        newscr_clk=frame;
        activated_timed_warp=false;
    }
}


void putintro()
{
    if(!stricmp("                                                                        ", DMaps[currdmap].intro))
    {
        introclk=intropos=72;
        return;
    }
    
    if((cBbtn())&&(get_bit(quest_rules,qr_ALLOWMSGBYPASS)))
    {
        //finish writing out the string
        for(; intropos<72; ++intropos)
        {
            textprintf_ex(msgdisplaybuf,zfont,((intropos%24)<<3)+32,((intropos/24)<<3)+40,QMisc.colors.msgtext,-1,
                          "%c",DMaps[currdmap].intro[intropos]);
        }
    }
    
    if(intropos>=72)
    {
        if(dmapmsgclk >= 51)
            dmapmsgclk=50;
            
        return;
    }
    
    if(((introclk++)%6<5)&&((!cAbtn())||(!get_bit(quest_rules,qr_ALLOWFASTMSG))))
        return;
        
    dmapmsgclk=51;
    
    if(intropos == 0)
    {
        while(DMaps[currdmap].intro[intropos]==' ')
            ++intropos;
    }
    
    sfx(WAV_MSG);
    
    
    //using the clip value to indicate the bitmap is "dirty"
    //rather than add yet another global variable
    set_clip_state(msgdisplaybuf, 0);
    textprintf_ex(msgdisplaybuf,zfont,((intropos%24)<<3)+32,((intropos/24)<<3)+40,QMisc.colors.msgtext,-1,
                  "%c",DMaps[currdmap].intro[intropos]);
                  
    ++intropos;
    
    if(DMaps[currdmap].intro[intropos]==' ' && DMaps[currdmap].intro[intropos+1]==' ')
        while(DMaps[currdmap].intro[intropos]==' ')
            ++intropos;
}

//static char *dirstr[4] = {"Up","Down","Left","Right"};
//static char *dirstr[32] = {"U","D","L","R"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "};

//use detail_int[x] for global detail info
void show_details()
{
    //textprintf_ex(framebuf,font,-3,-5,WHITE,BLACK,"%-4d",whistleclk);
    textprintf_ex(framebuf,font,0,8,WHITE,BLACK,"dlvl:%-2d dngn:%d", dlevel, isdungeon());
    textprintf_ex(framebuf,font,0,176,WHITE,BLACK,"%ld %s",game->get_time(),time_str_long(game->get_time()));
    
//    textprintf_ex(framebuf,font,200,16,WHITE,BLACK,"%3d",Link.getPushing());
//    for(int i=0; i<Lwpns.Count(); i++)
//      textprintf_ex(framebuf,font,200,(i<<3)+16,WHITE,BLACK,"%3d",items.spr(i)->id);

//    for(int i=0; i<Ewpns.Count(); i++)
//      textprintf_ex(framebuf,font,90,(i<<3)+16,WHITE,BLACK,"%3d %3d %3d %3d %3d",((weapon*)Ewpns.spr(i))->id, ((weapon*)Ewpns.spr(i))->tile, ((weapon*)Ewpns.spr(i))->clk, ((weapon*)Ewpns.spr(i))->aframe, wpnsbuf[((weapon*)Ewpns.spr(i))->id].frames);

//    for(int i=0; i<items.Count(); i++)
//      textprintf_ex(framebuf,font,90,(i<<3)+16,WHITE,BLACK,"%3d %3d %3d",((weapon*)Lwpns.spr(i))->tile, ((weapon*)Lwpns.spr(i))->dir, ((weapon*)Lwpns.spr(i))->flip);

    for(int i=0; i<guys.Count(); i++)
        textprintf_ex(framebuf,font,90,(i<<3)+16,WHITE,BLACK,"%d",(int)((enemy*)guys.spr(i))->id);
        
//      textprintf_ex(framebuf,font,90,16,WHITE,BLACK,"%3d, %3d",int(LinkModifiedX()),int(LinkModifiedY()));
    //textprintf_ex(framebuf,font,90,24,WHITE,BLACK,"%3d, %3d",detail_int[0],detail_int[1]);
//      textprintf_ex(framebuf,font,200,16,WHITE,BLACK,"%3d",Link.getAction());

    /*
      for(int i=0; i<Ewpns.Count(); i++)
      {
      sprite *s=Ewpns.spr(i);
      textprintf_ex(framebuf,font,100,(i<<3)+16,WHITE,BLACK,"%3d>%3d %3d>%3d %3d<%3d %3d<%3d ",
      int(Link.getX()+0+16), int(s->x+s->hxofs),  int(Link.getY()+0+16), int(s->y+s->hyofs),
      int(Link.getX()+0), int(s->x+s->hxofs+s->hxsz), int(Link.getY()+0), int(s->y+s->hyofs+s->hysz));
      }
      */
//        textprintf_ex(framebuf,font,200,16,WHITE,BLACK,"gi=%3d",guycarryingitem);
}

void show_ffscript_names()
{
    int ypos = 8;
    
    for(int i=0; i< MAXFFCS; i++)
    {
        if(tmpscr->ffscript[i])
        {
            textout_shadowed_ex(framebuf,font, ffcmap[tmpscr->ffscript[i]-1].second.c_str(),2,ypos,WHITE,BLACK,-1);
            ypos+=12;
        }
    }
}

void do_magic_casting()
{
    static int tempx, tempy;
    static byte linktilebuf[256];
    int ltile=0;
    int lflip=0;
    bool shieldModify=true;
    
    if(magicitem==-1)
    {
        return;
    }
    
    switch(itemsbuf[magicitem].family)
    {
    case itype_dinsfire:
    {
        if(magiccastclk==0)
        {
            Lwpns.add(new weapon(LinkX(),LinkY(),LinkZ(),wPhantom,pDINSFIREROCKET,0,up, magicitem, Link.getUID()));
            weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            //          Link.tile=(BSZ)?32:29;
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            casty=Link.getY();
        }
        
        if(magiccastclk==64)
        {
            Lwpns.add(new weapon((fix)LinkX(),(fix)(-32),(fix)LinkZ(),wPhantom,pDINSFIREROCKETRETURN,0,down, magicitem, Link.getUID()));
            weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            //          Link.tile=29;
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            castnext=false;
        }
        
        if(castnext)
        {
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(itemsbuf[magicitem].usesound,pan(int(Link.getX())));
                
            int flamemax=itemsbuf[magicitem].misc1;
            
            for(int flamecounter=((-1)*(flamemax/2))+1; flamecounter<=((flamemax/2)+1); flamecounter++)
            {
                Lwpns.add(new weapon((fix)LinkX(),(fix)LinkY(),(fix)LinkZ(),wFire,3,itemsbuf[magicitem].power*DAMAGE_MULTIPLIER,
                                     (tmpscr->flags7&fSIDEVIEW) ? (flamecounter<flamemax ? left : right) : 0, magicitem, Link.getUID()));
                weapon *w = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
                w->step=(itemsbuf[magicitem].misc2/100.0);
                w->angular=true;
                w->angle=(flamecounter*PI/(flamemax/2.0));
            }
            
            castnext=false;
            magiccastclk=128;
        }
        
        if((magiccastclk++)>=226)
        {
            magicitem=-1;
            magiccastclk=0;
        }
    }
    break;
    
    case itype_faroreswind:
    {
        if(magiccastclk==0)
        {
            linktile(&ltile, &lflip, ls_stab, down, zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                ltile+=item_tile_mod(shieldModify);
            }
            
            unpack_tile(newtilebuf, ltile, lflip, true);
            memcpy(linktilebuf, unpackbuf, 256);
            tempx=Link.getX();
            tempy=Link.getY();
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_pound, down, zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
        }
        
        if(magiccastclk>=0&&magiccastclk<64)
        {
            Link.setX(tempx+((rand()%3)-1));
            Link.setY(tempy+((rand()%3)-1));
        }
        
        if(magiccastclk==64)
        {
            Link.setX(tempx);
            Link.setY(tempy);
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_stab, down, zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
        }
        
        if(magiccastclk==96)
        {
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(itemsbuf[magicitem].usesound,pan(int(Link.getX())));
                
            Link.setDontDraw(true);
            
            for(int i=0; i<16; ++i)
            {
                for(int j=0; j<16; ++j)
                {
                    if(linktilebuf[i*16+j])
                    {
                        if(itemsbuf[magicitem].misc1==1)  // Twilight
                        {
                            particles.add(new pTwilight(Link.getX()+j, Link.getY()-Link.getZ()+i, 5, 0, 0, (rand()%8)+i*4));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=3;
                        }
                        else if(itemsbuf[magicitem].misc1==2)  // Sands of Hours
                        {
                            particles.add(new pTwilight(Link.getX()+j, Link.getY()-Link.getZ()+i, 5, 1, 2, (rand()%16)+i*2));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=4;
                            
                            if(rand()%10 < 2)
                            {
                                p->color=1;
                                p->cset=0;
                            }
                        }
                        else
                        {
                            particles.add(new pFaroresWindDust(Link.getX()+j, Link.getY()-Link.getZ()+i, 5, 6, linktilebuf[i*16+j], rand()%96));
                            
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->angular=true;
                            p->angle=rand();
                            p->step=(((double)j)/8);
                            p->yofs=Link.getYOfs();
                        }
                    }
                }
            }
        }
        
        if((magiccastclk++)>=226)
        {
            //attackclk=0;
            int nayrutemp=nayruitem;
            restart_level();
            nayruitem=nayrutemp;
            //xofs=0;
            //action=none;
            magicitem=-1;
            magiccastclk=0;
            Link.setDontDraw(false);
        }
    }
    break;
    
    case itype_nayruslove:
    {
        // See also Link.cpp, LinkClass::checkhit().
        if(magiccastclk==0)
        {
            Lwpns.add(new weapon(LinkX(),LinkY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET1,0,left, magicitem, Link.getUID()));
            weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            Lwpns.add(new weapon(LinkX(),LinkY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET2,0,right, magicitem, Link.getUID()));
            w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            //          Link.tile=(BSZ)?32:29;
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            castx=Link.getX();
        }
        
        if(magiccastclk==64)
        {
            for(int i=0; i<Lwpns.Count(); i++)
            {
                weapon* w=static_cast<weapon*>(Lwpns.spr(i));
                if(w->id==wPhantom &&
                  w->type>=pNAYRUSLOVEROCKET1 && w->type<=pNAYRUSLOVEROCKETTRAILRETURN2)
                    Lwpns.del(i);
            }
            
            int d=zc_max(LinkX(),256-LinkX())+32;
            Lwpns.add(new weapon((fix)(LinkX()-d),(fix)LinkY(),(fix)LinkZ(),wPhantom,pNAYRUSLOVEROCKETRETURN1,0,right, magicitem,Link.getUID()));
            weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            Lwpns.add(new weapon((fix)(LinkX()+d),(fix)LinkY(),(fix)LinkZ(),wPhantom,pNAYRUSLOVEROCKETRETURN2,0,left, magicitem,Link.getUID()));
            w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w1->step=4;
            //          Link.tile=29;
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            castnext=false;
        }
        
        if(castnext)
        {
            //          Link.tile=4;
            linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
            {
                Link.tile+=item_tile_mod(shieldModify);
            }
            
            Link.setNayrusLoveShieldClk(itemsbuf[magicitem].misc1);
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                if(nayruitem != -1)
                {
                    stop_sfx(itemsbuf[nayruitem].usesound+1);
                    stop_sfx(itemsbuf[nayruitem].usesound);
                }
                
                cont_sfx(itemsbuf[magicitem].usesound);
            }
            
            castnext=false;
            magiccastclk=128;
            nayruitem = magicitem;
        }
        
        // Finish the final spell pose
        if((magiccastclk++)>=160)
        {
            for(int i=0; i<Lwpns.Count(); i++)
            {
                weapon* w=static_cast<weapon*>(Lwpns.spr(i));
                if(w->id==wPhantom &&
                  w->type>=pNAYRUSLOVEROCKET1 && w->type<=pNAYRUSLOVEROCKETTRAILRETURN2)
                    Lwpns.del(i);
            }
            
            magicitem=-1;
            magiccastclk=0;
        }
    }
    break;
    
    default:
        magiccastclk=0;
        break;
    }
}

void update_hookshot()
{
    int hs_x, hs_y, hs_z,hs_dx, hs_dy;
    bool check_hs=false;
    int dist_bx, dist_by, hs_w;
    chainlinks.animate();
    //  char tempbuf[80];
    //  char tempbuf2[80];
    
    //find out where the head is and make it
    //easy to reference
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        check_hs=true;
    }
    
    if(check_hs)
    {
        int parentitem = ((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->parentitem;
        hs_x=Lwpns.spr(Lwpns.idFirst(wHookshot))->x;
        hs_y=Lwpns.spr(Lwpns.idFirst(wHookshot))->y;
        hs_z=Lwpns.spr(Lwpns.idFirst(wHookshot))->z;
        hs_dx=hs_x-hs_startx;
        hs_dy=hs_y-hs_starty;
        
        //extending
        if(((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==0)
        {
            int maxchainlinks=itemsbuf[parentitem].misc2;
            
            if(chainlinks.Count()<maxchainlinks)           //extending chain
            {
                if(abs(hs_dx)>=hs_xdist+8)
                {
                    hs_xdist=abs(hs_x-hs_startx);
                    chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,Link.getDir(), parentitem,Link.getUID()));
                }
                else if(abs(hs_dy)>=hs_ydist+8)
                {
                    hs_ydist=abs(hs_y-hs_starty);
                    chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,Link.getDir(), parentitem,Link.getUID()));
                }
            }                                                     //stretching chain
            else
            {
                dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
                dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
                hs_w=8;
                
                if(hs_dx<0)
                {
                    dist_bx=0-dist_bx;
                    hs_w=-8;
                }
                
                if(hs_dy<0)
                {
                    dist_by=0-dist_by;
                    hs_w=-8;
                }
                
                for(int counter=0; counter<chainlinks.Count(); counter++)
                {
                    if(Link.getDir()>down)                            //chain is moving horizontally
                    {
                        chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
                    }
                    else
                    {
                        chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
                    }
                }
            }
        }                                                       //retracting
        else if(((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==1)
        {
            dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
            dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
            hs_w=8;
            
            if(hs_dx<0)
            {
                dist_bx=0-dist_bx;
                hs_w=-8;
            }
            
            if(hs_dy<0)
            {
                dist_by=0-dist_by;
                hs_w=-8;
            }
            
            /* With ZScript modification, chains can conceivably move diagonally.*/
            //if (Link.getDir()>down)                               //chain is moving horizontally
            {
                if(abs(hs_dx)-(8*chainlinks.Count())>0)             //chain is stretched
                {
                    for(int counter=0; counter<chainlinks.Count(); counter++)
                    {
                        chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
                    }
                }
                else
                {
                    if(abs(hs_x-hs_startx)<=hs_xdist-8)
                    {
                        hs_xdist=abs(hs_x-hs_startx);
                        
                        if(pull_link==false)
                        {
                            chainlinks.del(chainlinks.idLast(wHSChain));
                        }
                        else
                        {
                            chainlinks.del(chainlinks.idFirst(wHSChain));
                        }
                    }
                }
            }                                                     //chain is moving vertically
            //else
            {
                if(abs(hs_dy)-(8*chainlinks.Count())>0)             //chain is stretched
                {
                    for(int counter=0; counter<chainlinks.Count(); counter++)
                    {
                        chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
                    }
                }
                else
                {
                    if(abs(hs_y-hs_starty)<=hs_ydist-8)
                    {
                        hs_ydist=abs(hs_y-hs_starty);
                        
                        if(pull_link==false)
                        {
                            chainlinks.del(chainlinks.idLast(wHSChain));
                        }
                        else
                        {
                            chainlinks.del(chainlinks.idFirst(wHSChain));
                        }
                    }
                }
            }
        }
    }
}

void do_dcounters()
{
    static bool sfxon = false;
    
    for(int i=0; i<32; i++)
    {
        if(game->get_dcounter(i)==0)
        {
            sfxon = false;
            continue;
        }
        
        if(frame&1)
        {
            sfxon = true;
            
            if(game->get_dcounter(i)>0)
            {
                int drain = (i==4 ? MAGICPERBLOCK/4 : 1);
                drain = zc_min(game->get_dcounter(i),drain);
                
                if(game->get_counter(i) < game->get_maxcounter(i))
                {
                    game->change_counter(drain, i);
                    game->change_dcounter(-drain, i);
                    
                    if(game->get_dcounter(i) < 0)  // Better safe
                        game->set_dcounter(0, i);
                }
                else
                {
                    game->set_dcounter(0, i);
                    game->set_counter(game->get_maxcounter(i), i);
                }
            }
            else
            {
                if(i!=1)   // Only rupee drain is sounded
                    sfxon = false;
                    
                int drain = (i==4 ? 2*game->get_magicdrainrate() : 1);
                
                if(game->get_counter(i)>0)
                {
                    game->change_counter(-drain, i);
                    game->change_dcounter(drain, i);
                    
                    if(game->get_dcounter(i) > 0)  // Better safe
                        game->set_dcounter(0, i);
                }
                else
                {
                    game->set_dcounter(0, i);
                    game->set_counter(0, i);
                }
            }
        }
        
        if((sfxon || i==1) && !lensclk && (i<2 || i==4)) // Life, Rupees and Magic
            sfx(WAV_MSG);
    }
}

void game_loop()
{

    //  walkflagx=0; walkflagy=0;
    if(fadeclk>=0)
    {
        if(fadeclk==0 && currscr<128)
            blockpath=false;
            
        --fadeclk;
    }
    
    // Three kinds of freezes: freeze, freezemsg, freezeff
    
    // freezemsg if message is being printed && qr_MSGFREEZE is on,
    // or if a message is being prepared && qr_MSGDISAPPEAR is on.
    bool freezemsg = ((msg_active || (intropos && intropos<72) || (linkedmsgclk && get_bit(quest_rules,qr_MSGDISAPPEAR)))
                      && get_bit(quest_rules,qr_MSGFREEZE));
                      
    // Messages also freeze FF combos.
    bool freezeff = freezemsg;
    
    bool freeze = false;
    
    for(int i=0; i<32; i++)
    {
        if(combobuf[tmpscr->ffdata[i]].type==cSCREENFREEZE) freeze=true;
        
        if(combobuf[tmpscr->ffdata[i]].type==cSCREENFREEZEFF) freezeff=true;
    }
    
    for(int i=0; i<176; i++)
    {
        if(combobuf[tmpscr->data[i]].type == cSCREENFREEZE) freeze=true;
        
        if(combobuf[tmpscr->data[i]].type == cSCREENFREEZEFF) freezeff=true;
        
        if(guygrid[i]>0)
        {
            --guygrid[i];
        }
    }
    
    animate_combos();
    load_control_state();
    
    if(!freezemsg)
    {
        script_drawing_commands.Clear();
    }
    
    if(!freezeff)
    {
        update_freeform_combos();
    }
    
    // Arbitrary Rule 637: neither 'freeze' nor 'freezeff' freeze the global script.
    if(!freezemsg && g_doscript)
    {
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_GAME);
    }
    
    if(!freeze && !freezemsg)
    {
        mblock2.animate(0);
        items.animate();
        items.check_conveyor();
        guys.animate();
        roaming_item();
        dragging_item();
        Ewpns.animate();
        checklink = true;
        
        for(int i = 0; i < (gofast ? 8 : 1); i++)
        {
            if(Link.animate(0))
            {
                if(!Quit)
                {
                    Quit = qGAMEOVER;
                }
                
                return;
            }
            
            checklink=false;
        }
	
	Link.cleanupByrna();
        
        do_magic_casting();
        Lwpns.animate();
        decorations.animate();
        particles.animate();
        update_hookshot();
        
        if(conveyclk<=0)
        {
            conveyclk=3;
        }
        
        --conveyclk;
        check_collisions();
        dryuplake();
        cycle_palette();
    }
    else if(freezemsg)
    {
        for(int i=0; i<guys.Count(); i++)
            if(((enemy*)guys.spr(i))->ignore_msg_freeze())
                guys.spr(i)->animate(i);
    }
    
    if(global_wait)
    {
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_GAME);
        global_wait=false;
    }
    
    draw_screen(tmpscr);
    
    if(linkedmsgclk==1)
    {
        if(wpnsbuf[iwMore].tile!=0)
        {
            putweapon(framebuf,zinit.msg_more_x, message_more_y(), wPhantom, 4, up, lens_hint_weapon[wPhantom][0], lens_hint_weapon[wPhantom][1],-1);
        }
    }
    
    if(!freeze)
    {
        putintro();
    }
    
    if(dmapmsgclk>0)
    {
        Link.Freeze();
        
        if(dmapmsgclk<=50)
        {
            --dmapmsgclk;
        }
    }
    
    if(dmapmsgclk==1)
    {
        Link.finishedmsg();
        dmapmsgclk=0;
        introclk=72;
        clear_bitmap(msgdisplaybuf);
        set_clip_state(msgdisplaybuf, 1);
        //    clear_bitmap(pricesdisplaybuf);
    }
    
    if(!freeze)
    {
        if(introclk==0 || (introclk>=72 && dmapmsgclk==0))
        {
            putmsg();
            
            if(msgstr)
            {
                set_clip_state(msgdisplaybuf, 0);
                blit(msgbmpbuf, msgdisplaybuf, 0, 0, msg_xpos, msg_ypos, msg_w+16, msg_h+16);
            }
        }
        
        do_dcounters();
        
        if(!freezemsg && current_item(itype_heartring))
        {
            int itemid = current_item_id(itype_heartring);
            int fskip = itemsbuf[itemid].misc2;
            
            if(fskip == 0 || frame % fskip == 0)
                game->set_life(zc_min(game->get_life() + itemsbuf[itemid].misc1, game->get_maxlife()));
        }
        
        if(!freezemsg && current_item(itype_magicring))
        {
            int itemid = current_item_id(itype_magicring);
            int fskip = itemsbuf[itemid].misc2;
            
            if(fskip == 0 || frame % fskip == 0)
            {
                game->set_magic(zc_min(game->get_magic() + itemsbuf[itemid].misc1, game->get_maxmagic()));
            }
        }
        
        if(!freezemsg && current_item(itype_wallet))
        {
            int itemid = current_item_id(itype_wallet);
            int fskip = itemsbuf[itemid].misc2;
            
            if(fskip == 0 || frame % fskip == 0)
            {
                game->set_rupies(zc_min(game->get_rupies() + itemsbuf[itemid].misc1, game->get_maxcounter(1)));
            }
        }
        
        if(!freezemsg && current_item(itype_bombbag))
        {
            int itemid = current_item_id(itype_bombbag);
            
            if(itemsbuf[itemid].misc1)
            {
                int fskip = itemsbuf[itemid].misc2;
                
                if(fskip == 0 || frame % fskip == 0)
                {
                    game->set_bombs(zc_min(game->get_bombs() + itemsbuf[itemid].misc1, game->get_maxbombs()));
                }
                
                if(itemsbuf[itemid].flags & ITEM_FLAG1)
                {
                    int ratio = zinit.bomb_ratio;
                    
                    fskip = itemsbuf[itemid].misc2 * ratio;
                    
                    if(fskip == 0 || frame % fskip == 0)
                    {
                        game->set_sbombs(zc_min(game->get_sbombs() + zc_max(itemsbuf[itemid].misc1 / ratio, 1), game->get_maxbombs() / ratio));
                    }
                }
            }
        }
        
        if(!freezemsg && current_item(itype_quiver) && game->get_arrows() != game->get_maxarrows())
        {
            int itemid = current_item_id(itype_quiver);
            int fskip = itemsbuf[itemid].misc2;
            
            if(fskip == 0 || frame % fskip == 0)
            {
                game->set_arrows(zc_min(game->get_arrows() + itemsbuf[itemid].misc1, game->get_maxarrows()));
            }
        }
        
        if(lensclk)
        {
            draw_lens_over();
            --lensclk;
        }
        
        // Earthquake!
        if(quakeclk>0)
        {
            playing_field_offset=56+((int)(sin((double)(--quakeclk*2-frame))*4));
        }
        else
        {
            playing_field_offset=56;
        }
        
        // Other effects in zc_sys.cpp
    }
    
    //  putpixel(framebuf, walkflagx, walkflagy+playing_field_offset, vc(int(rand()%16)));
}

int get_currdmap()
{
    return currdmap;
}

int get_dlevel()
{
    return dlevel;
}

int get_currscr()
{
    return currscr;
}

int get_homescr()
{
    return homescr;
}

int get_bmaps(int si)
{
    return game->bmaps[si];
}

bool no_subscreen()
{
    return (tmpscr->flags3&fNOSUBSCR)!=0;
}

/**************************/
/********** Main **********/
/**************************/

void PopulateInitDialog();

bool is_zquest()
{
    return false;
}

bool screenIsScrolling()
{
    return screenscrolling;
}

int isFullScreen()
{
    return !is_windowed_mode();
}

class TB_Handler //Dear Santa: please kill Easter bunny. I've been a good boy.
{
public:

    TB_Handler() {}
    ~TB_Handler() {}
    
    bool CanEnable() const
    {
        if(is_windowed_mode() && can_triplebuffer_in_windowed_mode == FALSE)
        {
            triplebuffer_not_available = true;
            return false;
        }
        
        return (disable_triplebuffer == FALSE);
    }
    bool GFX_can_triple_buffer() const
    {
        if(!CanEnable())
        {
            triplebuffer_not_available = true;
            return false;
        }
        
        triplebuffer_not_available = false;
        
        if(!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)) enable_triple_buffer();
        
        if(!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)) triplebuffer_not_available = true;
        
        return !triplebuffer_not_available;
    }
    void Destroy() const
    {
        if(disable_triplebuffer != FALSE || triplebuffer_not_available) return;
        
        for(int i=0; i<3; i++)
            if(tb_page[i])
                destroy_bitmap(tb_page[i]);
    }
    void Create() const
    {
        if(!CanEnable())
        {
            triplebuffer_not_available = true;
            return;
        }
        
        for(int i=0; i<3; ++i)
        {
            tb_page[i]=create_video_bitmap(SCREEN_W, SCREEN_H);
            
            if(!tb_page[i])
            {
                triplebuffer_not_available = true;
                break;
            }
        }
        
        Clear();
    }
    void Clear() const
    {
        for(int i=0; i<3; i++)
            clear_bitmap(tb_page[i]);
    }
}
static Triplebuffer;

bool setGraphicsMode(bool windowed)
{
    int type=windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN;
    return set_gfx_mode(type, resx, resy, 0, 0)==0;
}

int onFullscreen()
{
    PALETTE oldpal;
    get_palette(oldpal);
    
    show_mouse(NULL);
    bool windowed=is_windowed_mode()!=0;
    
    // these will become ultra corrupted no matter what.
    Triplebuffer.Destroy();
    
    bool success=setGraphicsMode(!windowed);
    if(success)
        fullscreen=!fullscreen;
    else
    {
        // Try to restore the previous mode, then...
        success=setGraphicsMode(windowed);
        if(!success)
        {
            Z_message("Failed to set video mode.\n");
            Z_message(allegro_error);
            exit(1);
        }
    }
    
    /* ZC will crash going from fullscreen to windowed mode if triple buffer is left unchecked. -Gleeok  */
    if(Triplebuffer.GFX_can_triple_buffer())
    {
        Triplebuffer.Create();
        Z_message("Triplebuffer enabled \n");
    }
    else
        Z_message("Triplebuffer disabled \n");
    
    //Everything set?
    Z_message("gfx mode set at -%d %dbpp %d x %d \n", is_windowed_mode(), get_color_depth(), resx, resy);
    
    set_palette(oldpal);
    gui_mouse_focus=0;
    show_mouse(screen);
    set_display_switch_mode(fullscreen?SWITCH_BACKAMNESIA:SWITCH_BACKGROUND);
//	set_display_switch_callback(SWITCH_OUT, switch_out_callback);/
//	set_display_switch_callback(SWITCH_IN,switch_in_callback);

    return D_REDRAW;
}

static const char months[13][13] =
{ 
	"Nonetober", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

static std::string dayextension(int dy)
{ 
	char temp[6]; 
	switch(dy)
	{
		
		
		//st
		case 1:
		case 21:
		case 31:
			sprintf(temp,"%d%s",dy,"st"); 
			break;
		//nd
		case 2:
		case 22:
			sprintf(temp,"%d%s",dy,"nd"); 
			break;
		//rd
		case 3:
		case 23:
			sprintf(temp,"%d%s",dy,"rd"); 
			break;
		//th
		default:
			sprintf(temp,"%d%s",dy,"th");
			break;
	}
	
	return std::string(temp); 
} 

int main(int argc, char* argv[])
{
    bool onlyInstance=true;
	
    memset(zc_builddate,0,80);
    memset(zc_aboutstr,0,80);

    sprintf(zc_builddate,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(), (char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
    sprintf(zc_aboutstr,"%s (%s), Version %s", ZC_PLAYER_NAME, PROJECT_NAME, ZC_PLAYER_V);
    
	
    if ( V_ZC_ALPHA )
	{
		Z_title("%s, v.%s Alpha %d",ZC_PLAYER_NAME, ZC_PLAYER_V, V_ZC_ALPHA);
	}
		
	else if ( V_ZC_BETA )
	{
		Z_title("%s, v.%s Beta %d",ZC_PLAYER_NAME, ZC_PLAYER_V, V_ZC_BETA);
	}
	else if ( V_ZC_GAMMA )
	{
		Z_title("%s, v.%s Gamma %d",ZC_PLAYER_NAME, ZC_PLAYER_V, V_ZC_GAMMA);
	}
	else /*( V_ZC_RELEASE )*/
	{
		Z_title("%s, v.%s Release %d",ZC_PLAYER_NAME, ZC_PLAYER_V, V_ZC_RELEASE);
	}
	/*
	    switch(IS_BETA)
	    {
	    
	    case -1:
	    {
		Z_title("Zelda Classic %s Alpha (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
		//Print the current time to allegro.log as a test.
		
		//for (int q = 0; q < curTimeLAST; q++) 
		//{
		//    int t_time_v = FFCore.getTime(q);
		//}
		    
		break;
	    }
		
	    case 1:
		Z_title("Zelda Classic %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
		break;
		
	    case 0:
		Z_title("Zelda Classic %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
	    }
	*/
    
    if(used_switch(argc, argv, "-standalone"))
    {
        standalone_mode=true;
        
        int arg=used_switch(argc, argv, "-standalone");
        
        if(arg==argc-1)
        {
            Z_error("-standalone requires a quest file, e.g.\n" \
                    "  -standalone MyQuest.qst\n" \
                    "  -standalone \"Name with spaces.qst\"");
            exit(1);
        }
        
        standalone_quest=argv[arg+1];
        
	for ( int q = 0; q < 10; q++ )
        if(!stricmp(standalone_quest, moduledata.quests[q]))
        {
            Z_error("Standalone mode can only be used with custom quests.");
            exit(1);
        }
        
        int len=strlen(standalone_quest);
        
        for(int i=0; i<len; i++)
        {
#ifdef _ALLEGRO_WINDOWS
        
            if(standalone_quest[i]=='/')
            {
                standalone_quest[i]='\\';
            }
            
#else
            
            if(standalone_quest[i]=='\\')
            {
                standalone_quest[i]='/';
            }
            
#endif
        }
    }
    
    //turn on MSVC memory checks
    //this should be interesting...
    
//  InitCrtDebug();

    // Before anything else, let's register our custom trace handler:
    register_trace_handler(zc_trace_handler);
    
    //  Z_title("Zelda Classic %s",VerStr(ZELDA_VERSION));
    
    // allocate quest data buffers
    
#ifdef _WIN32
    
    if(used_switch(argc, argv, "-console") || used_switch(argc, argv, "-con"))
    {
        DebugConsole::Open();
        zconsole = true;
    }
    
#endif
    
    
    
    PopulateInitDialog();
    
    memrequested += 4096;
    Z_message("Allocating quest path buffers (%s)...", byte_conversion2(4096,memrequested,-1,-1));
    qstdir = (char*)zc_malloc(2048);
    qstpath = (char*)zc_malloc(2048);
    
    if(!qstdir || !qstpath)
    {
        Z_error("Allocation error");
        quit_game();
    }
    
    qstdir[0] = 0;
    qstpath[0] = 0;
    
#ifdef ALLEGRO_MACOSX
    sprintf(qstdir, "../../../");
    sprintf(qstpath, "../../../");
#endif
    
    Z_message("OK\n");
    
    if(!get_qst_buffers())
    {
        Z_error("Error");
        quit_game();
    }
    
    // initialize Allegro
    
    Z_message("Initializing Allegro... ");
    
    if(allegro_init() != 0)
    {
        Z_error("Failed Init!");
        quit_game();
    }
    
    three_finger_flag=false;
    //atexit(&dumb_exit);
    //dumb_register_stdfiles();
    
    register_bitmap_file_type("GIF",  load_gif, save_gif);
    jpgalleg_init();
    loadpng_init();
    
    // set and load game configurations
    set_config_file("zc.cfg");
    
    if(exists("zc.cfg") != 0)
    {
        load_game_configs();
    }
    else
    {
        load_game_configs();
        save_game_configs();
    }
    zcm.init();
        
#ifndef ALLEGRO_MACOSX // Should be done on Mac, too, but I haven't gotten that working
    if(!is_only_instance("zc.lck"))
    {
        if(used_switch(argc, argv, "-multiple") || get_config_int("zeldadx","multiple_instances",0) )
            onlyInstance=false;
        else
            exit(1);
    }
#endif
    
#ifdef _WIN32
    
    //launch debug console if requested.
    if(use_debug_console != FALSE)
    {
        DebugConsole::Open();
        zconsole = true;
    }
    if ( zscript_debugger )
    {
	FFCore.ZScriptConsole(true);
    }
    
#endif
    
    if(install_timer() < 0)
    {
        Z_error(allegro_error);
        quit_game();
    }
    
    if(install_keyboard() < 0)
    {
        Z_error(allegro_error);
        quit_game();
    }
    
    if(install_mouse() < 0)
    {
        Z_error(allegro_error);
        quit_game();
    }
    
    if(install_joystick(JOY_TYPE_AUTODETECT) < 0)
    {
        Z_error(allegro_error);
        quit_game();
    }
    
    //set_keyboard_rate(1000,160);
    
    LOCK_VARIABLE(logic_counter);
    LOCK_FUNCTION(update_logic_counter);
    install_int_ex(update_logic_counter, BPS_TO_TIMER(60));
    
#ifdef _SCRIPT_COUNTER
    LOCK_VARIABLE(script_counter);
    LOCK_FUNCTION(update_script_counter);
    install_int_ex(update_script_counter, 1);
#endif
    
    if(!Z_init_timers())
    {
        Z_error("Couldn't Allocate Timers");
        quit_game();
    }
    
    Z_message("OK\n");
    
    // check for the included quest files
    if(!standalone_mode)
    {
        Z_message("Checking Files... ");
        
        char path[2048];
        
        append_filename(path, qstdir, moduledata.quests[0], 2048);
        
        if(!exists(moduledata.quests[0]) && !exists(path))
        {
            Z_error("\"1st.qst\" not found.");
            quit_game();
        }
        
        append_filename(path, qstdir, moduledata.quests[1], 2048);
        
        if(!exists(moduledata.quests[1]) && !exists(path))
        {
            Z_error("\"2nd.qst\" not found.");
            quit_game();
        }
        
        append_filename(path, qstdir, moduledata.quests[2], 2048);
        
        if(!exists(moduledata.quests[2]) && !exists(path))
        {
            Z_error("\"3rd.qst\" not found.");
            quit_game();
        }
        
        append_filename(path, qstdir, moduledata.quests[3], 2048);
        
        if(!exists(moduledata.quests[3]) && !exists(path))
        {
            Z_error("\"4th.qst\" not found.");
            quit_game();
        }
        
        append_filename(path, qstdir, moduledata.quests[4], 2048);
        
        if(!exists(moduledata.quests[4]) && !exists(path))
        {
            Z_error("\"5th.qst\" not found.");
            quit_game();
        }
	
	append_filename(path, qstdir, moduledata.quests[5], 2048);
        
        if(!exists(moduledata.quests[5]) && !exists(path))
        {
            al_trace("\"6th.qst\" not found.");
            //quit_game();
        }
        
        Z_message("OK\n");
    }
    
    // allocate bitmap buffers
    Z_message("Allocating bitmap buffers... ");
    
    //Turns out color depth can be critical. -Gleeok
    if(used_switch(argc,argv,"-0bit")) set_color_depth(desktop_color_depth());
    else if(used_switch(argc,argv,"-15bit")) set_color_depth(15);
    else if(used_switch(argc,argv,"-16bit")) set_color_depth(16);
    else if(used_switch(argc,argv,"-24bit")) set_color_depth(24);
    else if(used_switch(argc,argv,"-32bit")) set_color_depth(32);
    else
    {
        //command-line switches takes priority
        switch(zc_color_depth)
        {
        case 0:
            set_color_depth(desktop_color_depth());
            break;
            
        case 8:
            set_color_depth(8);
            break;
            
        case 15:
            set_color_depth(15);
            break;
            
        case 16:
            set_color_depth(16);
            break;
            
        case 24:
            set_color_depth(24);
            break;
            
        case 32:
            set_color_depth(32);
            break;
            
        default:
            zc_color_depth = 8; //invalid configuration, set to default in config file.
            set_color_depth(8);
            break;
        }
    }
    
    
    framebuf  = create_bitmap_ex(8,256,224);
    temp_buf  = create_bitmap_ex(8,256,224);
    scrollbuf = create_bitmap_ex(8,512,406);
    screen2   = create_bitmap_ex(8,320,240);
    tmp_scr   = create_bitmap_ex(8,320,240);
    tmp_bmp   = create_bitmap_ex(8,32,32);
    fps_undo  = create_bitmap_ex(8,64,16);
    prim_bmp  = create_bitmap_ex(8,512,512);
    msgdisplaybuf = create_bitmap_ex(8,256, 176);
    msgbmpbuf = create_bitmap_ex(8, 512+16, 512+16);
    pricesdisplaybuf = create_bitmap_ex(8,256, 176);
    
    if(!framebuf || !scrollbuf || !tmp_bmp || !fps_undo || !tmp_scr
            || !screen2 || !msgdisplaybuf || !pricesdisplaybuf)
    {
        Z_error("Error");
        quit_game();
    }
    
    clear_bitmap(scrollbuf);
    clear_bitmap(framebuf);
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    Z_message("OK\n");
    
    
    zcmusic_init();
    
    //  int mode = VidMode;                                       // from config file
    int tempmode=GFX_AUTODETECT;
    int res_arg = used_switch(argc,argv,"-res");
    
    if(used_switch(argc,argv,"-v0")) Throttlefps=false;
    
    if(used_switch(argc,argv,"-v1")) Throttlefps=true;
    
    
    resolve_password(zeldapwd);
    debug_enabled = used_switch(argc,argv,"-d") && !strcmp(get_config_string("zeldadx","debug",""),zeldapwd);
    set_debug(debug_enabled);
    
    skipicon = standalone_mode || used_switch(argc,argv,"-quickload");
    
    int load_save=0;
    
    load_save = used_switch(argc,argv,"-load");
    load_save = load_save?(argc>load_save+1)?atoi(argv[load_save+1]):0:0;
    
    if(!(used_switch(argc,argv,"-load")))
        if(used_switch(argc,argv,"-loadlast"))
            load_save = loadlast;
            
    slot_arg = used_switch(argc,argv,"-slot");
    
    if(argc <= (slot_arg+1))
    {
        slot_arg = 0;
    }
    else
    {
        slot_arg2 = vbound(atoi(argv[slot_arg+1]), 1, MAXSAVES);
    }
    
    if(standalone_mode)
    {
        load_save=1;
        slot_arg=1;
        slot_arg2=1;
    }
    
    int fast_start = debug_enabled || used_switch(argc,argv,"-fast") || get_config_int("zeldadx","skiplogo",0) || (!standalone_mode && (load_save || (slot_arg && (argc>(slot_arg+1)))));
    skip_title = used_switch(argc, argv, "-notitle") > 0;
    int save_arg = used_switch(argc,argv,"-savefile");
    
    //if ( !strcmp(get_config_string("zeldadx","debug",""),"") )
    //{
	//for ( int q = 0; q < 1024; ++q ) { save_file_name[q] = 0; }
        //strcpy(save_file_name,"zc.sav");
	//SAVE_FILE = (char *)save_file_name;  
	    
    //}
    /*else*/ //if ( strcmp(get_config_string("zeldadx","debug","")) )
    {	    
	for ( int q = 0; q < 1024; ++q ) { save_file_name[q] = 0; }
        strcpy(save_file_name,get_config_string("SAVEFILE","save_filename","zc.sav"));
	SAVE_FILE = (char *)save_file_name;
    }
    //al_trace("Current save file is: %s\n", save_file_name);
    if(save_arg && (argc>(save_arg+1)))
    {
        SAVE_FILE = (char *)zc_malloc(2048);
        sprintf(SAVE_FILE, "%s", argv[save_arg+1]);
        
        int len=strlen(SAVE_FILE);
        
        for(int i=0; i<len; i++)
        {
#ifdef _ALLEGRO_WINDOWS
        
            if(SAVE_FILE[i]=='/')
            {
                SAVE_FILE[i]='\\';
            }
            
#else
            
            if(SAVE_FILE[i]=='\\')
            {
                SAVE_FILE[i]='/';
            }
            
#endif
        }
    }
    
    
    
    // load the data files
    resolve_password(datapwd);
//  setPackfilePassword(datapwd);
    packfile_password(datapwd);
    
    
    Z_message("Loading data files:\n");
    set_color_conversion(COLORCONV_NONE);
    
    sprintf(zeldadat_sig,"Zelda.Dat %s Build %d",VerStr(ZELDADAT_VERSION), ZELDADAT_BUILD);
    sprintf(sfxdat_sig,"SFX.Dat %s Build %d",VerStr(SFXDAT_VERSION), SFXDAT_BUILD);
    sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);
    
    packfile_password(NULL); // Temporary measure. -L
    Z_message("Zelda.Dat...");
    
    //if((data=load_datafile("zelda.dat"))==NULL)
    if((data=load_datafile(moduledata.datafiles[zelda_dat]))==NULL)
    {
        Z_error("failed");
        quit_game();
    }
    
    if(strncmp((char*)data[0].dat,zeldadat_sig,24))
    {
        Z_error("\nIncompatible version of zelda.dat.\nPlease upgrade to %s Build %d",VerStr(ZELDADAT_VERSION), ZELDADAT_BUILD);
        quit_game();
    }
    
    Z_message("OK\n");
    packfile_password(datapwd); // Temporary measure. -L
    
    Z_message("Fonts.Dat...");
    
    //if((fontsdata=load_datafile("fonts.dat"))==NULL)
    if((fontsdata=load_datafile(moduledata.datafiles[fonts_dat]))==NULL)
    {
        Z_error("failed");
        quit_game();
    }
    
    if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,24))
    {
        Z_error("\nIncompatible version of fonts.dat.\nPlease upgrade to %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);
        quit_game();
    }
    
    Z_message("OK\n");
    
//  setPackfilePassword(NULL);
    packfile_password(NULL);
    
    Z_message("SFX.Dat...");
    
    //if((sfxdata=load_datafile("sfx.dat"))==NULL)
    if((sfxdata=load_datafile(moduledata.datafiles[sfx_dat]))==NULL)
    {
        Z_error("failed");
        quit_game();
    }
    
    if(strncmp((char*)sfxdata[0].dat,sfxdat_sig,22) || sfxdata[Z35].type != DAT_ID('S', 'A', 'M', 'P'))
    {
        Z_error("\nIncompatible version of sfx.dat.\nPlease upgrade to %s Build %d",VerStr(SFXDAT_VERSION), SFXDAT_BUILD);
        quit_game();
    }
    
    Z_message("OK\n");
    
    mididata = (DATAFILE*)data[ZC_MIDI].dat;
    
    set_uformat(U_ASCII);
    deffont = font;
    nfont = (FONT*)fontsdata[FONT_GUI_PROP].dat;
    font = nfont;
    pfont = (FONT*)fontsdata[FONT_8xPROP_THIN].dat;
    lfont = (FONT*)fontsdata[FONT_LARGEPROP].dat;
    lfont_l = (FONT*)fontsdata[FONT_LARGEPROP_L].dat;
    zfont = (FONT*)fontsdata[FONT_NES].dat;
    z3font = (FONT*)fontsdata[FONT_Z3].dat;
    z3smallfont = (FONT*)fontsdata[FONT_Z3SMALL].dat;
    mfont = (FONT*)fontsdata[FONT_MATRIX].dat;
    ztfont = (FONT*)fontsdata[FONT_ZTIME].dat;
    sfont = (FONT*)fontsdata[FONT_6x6].dat;
    sfont2 = (FONT*)fontsdata[FONT_6x4].dat;
    sfont3 = (FONT*)fontsdata[FONT_12x8].dat;
    spfont = (FONT*)fontsdata[FONT_6xPROP].dat;
    ssfont1 = (FONT*)fontsdata[FONT_SUBSCREEN1].dat;
    ssfont2 = (FONT*)fontsdata[FONT_SUBSCREEN2].dat;
    ssfont3 = (FONT*)fontsdata[FONT_SUBSCREEN3].dat;
    ssfont4 = (FONT*)fontsdata[FONT_SUBSCREEN4].dat;
    gblafont = (FONT*)fontsdata[FONT_GB_LA].dat;
    goronfont = (FONT*)fontsdata[FONT_GORON].dat;
    zoranfont = (FONT*)fontsdata[FONT_ZORAN].dat;
    hylian1font = (FONT*)fontsdata[FONT_HYLIAN1].dat;
    hylian2font = (FONT*)fontsdata[FONT_HYLIAN2].dat;
    hylian3font = (FONT*)fontsdata[FONT_HYLIAN3].dat;
    hylian4font = (FONT*)fontsdata[FONT_HYLIAN4].dat;
    gboraclefont = (FONT*)fontsdata[FONT_GB_ORACLE].dat;
    gboraclepfont = (FONT*)fontsdata[FONT_GB_ORACLE_P].dat;
    dsphantomfont = (FONT*)fontsdata[FONT_DS_PHANTOM].dat;
    dsphantompfont = (FONT*)fontsdata[FONT_DS_PHANTOM_P].dat;
    
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<MAXSUBSCREENITEMS; j++)
        {
            memset(&custom_subscreen[i].objects[j],0,sizeof(subscreen_object));
        }
    }
    
    for(int i=0; i<WAV_COUNT; i++)
    {
        customsfxdata[i].data=NULL;
        sfx_string[i] = new char[36];
    }
    
    for(int i=0; i<WAV_COUNT>>3; i++)
    {
        customsfxflag[i] = 0;
    }
    
    for(int i=0; i<WPNCNT; i++)
    {
        weapon_string[i] = new char[64];
    }
    
    for(int i=0; i<ITEMCNT; i++)
    {
        item_string[i] = new char[64];
    }
    
    for(int i=0; i<eMAXGUYS; i++)
    {
        guy_string[i] = new char[64];
    }
    
    for(int i=0; i<512; i++)
    {
        ffscripts[i] = new ffscript[1];
        ffscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<256; i++)
    {
        itemscripts[i] = new ffscript[1];
        itemscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<256; i++)
    {
        guyscripts[i] = new ffscript[1];
        guyscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<256; i++)
    {
        wpnscripts[i] = new ffscript[1];
        wpnscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<256; i++)
    {
        screenscripts[i] = new ffscript[1];
        screenscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<NUMSCRIPTGLOBAL; i++)
    {
        globalscripts[i] = new ffscript[1];
        globalscripts[i][0].command = 0xFFFF;
    }
    
    for(int i=0; i<3; i++)
    {
        linkscripts[i] = new ffscript[1];
        linkscripts[i][0].command = 0xFFFF;
    }
    
    //script drawing bitmap allocation
    zscriptDrawingRenderTarget = new ZScriptDrawingRenderTarget();
    
    
    // initialize sound driver
    
    Z_message("Initializing sound driver... ");
    
    if(used_switch(argc,argv,"-s") || used_switch(argc,argv,"-nosound"))
    {
        Z_message("skipped\n");
    }
    else
    {
        if(install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL))
        {
            //      Z_error(allegro_error);
            Z_message("Sound driver not available.  Sound disabled.\n");
        }
        else
        {
            Z_message("OK\n");
        }
    }
    
    Z_init_sound();
    
    
    // CD player
    
    /*
      if(used_switch(argc,argv,"-cd"))
      {
      printf("Initializing CD player... ");
      if(cd_init())
      Z_error("Error");
      printf("OK\n");
      useCD = true;
      }
      */
    
    //use only page flipping
    if(used_switch(argc,argv,"-doublebuffer"))
    {
        disable_triplebuffer = 1;
        Z_message("used switch: -doublebuffer\n");
    }
    
    //allow video bitmaps in windowed mode
    if(used_switch(argc,argv,"-triplebuffer"))
    {
        can_triplebuffer_in_windowed_mode = 1;
        Z_message("used switch: -triplebuffer\n");
    }
    
    const int wait_ms_on_set_graphics = 20; //formerly 250. -Gleeok
    
    // quick quit
    if(used_switch(argc,argv,"-q"))
    {
        printf("-q switch used, quitting program.\n");
        goto quick_quit;
    }
    
    // set video mode
    
    if(res_arg && (argc>(res_arg+2)))
    {
        resx = atoi(argv[res_arg+1]);
        resy = atoi(argv[res_arg+2]);
        bool old_sbig = (argc>(res_arg+3))? stricmp(argv[res_arg+3],"big")==0 : 0;
        bool old_sbig2 = (argc>(res_arg+3))? stricmp(argv[res_arg+3],"big2")==0 : 0;
        
//    mode = GFX_AUTODETECT;
    }
    
    if(resx>=640 && resy>=480)
    {
        is_large=true;
    }
    
    //request_refresh_rate(60);
    
    if(used_switch(argc,argv,"-fullscreen") ||
            (!used_switch(argc, argv, "-windowed") && get_config_int("zeldadx","fullscreen",1)==1))
    {
        al_trace("Used switch: -fullscreen\n");
        tempmode = GFX_AUTODETECT_FULLSCREEN;
    }
    else if(used_switch(argc,argv,"-windowed") || get_config_int("zeldadx","fullscreen",1)==0)
    {
        al_trace("Used switch: -windowed\n");
        tempmode=GFX_AUTODETECT_WINDOWED;
    }
    
    //set scale
    if(resx < 320) resx = 320;
    
    if(resy < 240) resy = 240;
    
    screen_scale = zc_max(zc_min(resx / 320, resy / 240), 1);
    
    if(!game_vid_mode(tempmode, wait_ms_on_set_graphics))
    {
        //what we need here is not rightousness but madness!!!
        
#define TRY_SET_VID_MODE(scale) \
	Z_message("Unable to set gfx mode at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy); \
	screen_scale=scale; \
	resx=320*scale; \
	resy=240*scale
        
        TRY_SET_VID_MODE(2);
        
        if(!game_vid_mode(tempmode, wait_ms_on_set_graphics))
        {
            TRY_SET_VID_MODE(1);
            
            if(!game_vid_mode(tempmode, wait_ms_on_set_graphics))
            {
                if(tempmode != GFX_AUTODETECT_WINDOWED)
                {
                    tempmode=GFX_AUTODETECT_WINDOWED;
                    al_trace("-fullscreen not supported by your video driver! setting -windowed switch\n");
                    TRY_SET_VID_MODE(2);
                    
                    if(!game_vid_mode(tempmode, wait_ms_on_set_graphics))
                    {
                        TRY_SET_VID_MODE(1);
                        
                        if(!game_vid_mode(tempmode, wait_ms_on_set_graphics))
                        {
                            Z_message("Unable to set gfx mode at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
                            al_trace("Fatal Error...Zelda Classic could not be initialized. Have a nice day :) \n");
                            Z_error(allegro_error);
                            quit_game();
                        }
                        else Z_message("set gfx mode succsessful at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
                    }
                    else Z_message("set gfx mode succsessful at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
                }
                else
                {
                    al_trace("Fatal Error: could not create a window for Zelda Classic.\n");
                    Z_error(allegro_error);
                    quit_game();
                }
            }
            else Z_message("set gfx mode succsessful at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
        }
        else Z_message("set gfx mode succsessful at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
    }
    else
    {
        Z_message("set gfx mode succsessful at -%d %dbpp %d x %d \n", tempmode, get_color_depth(), resx, resy);
    }
    
    sbig = (screen_scale > 1);
    set_display_switch_mode(is_windowed_mode()?SWITCH_BACKGROUND:SWITCH_BACKAMNESIA);
    zq_screen_w = resx;
    zq_screen_h = resy;
    
    real_screen = screen;
    
    if(Triplebuffer.GFX_can_triple_buffer())
    {
        Triplebuffer.Create();
    }
    
    Z_message("Triplebuffer %savailable\n", triplebuffer_not_available?"not ":"");
    
    set_close_button_callback((void (*)()) hit_close_button);
    set_window_title("Zelda Classic");
    
    fix_dialogs();
    gui_mouse_focus = FALSE;
    position_mouse(resx-16,resy-16);
    
    if(!onlyInstance)
    {
        clear_to_color(screen,BLACK);
        system_pal();
        int ret=jwin_alert3("Multiple Instances",
                            "Another instance of ZC is already running.",
                            "Running multiple instances may cause your",
                            "save file to be deleted. Continue anyway?",
                            "&No","&Yes", 0, 'n', 'y', 0, lfont);
        if(ret!=2)
        {
            if(forceExit)
                exit(0);
                
            allegro_exit();
            return 0;
        }
    }
    
// load saved games
    Z_message("Loading saved games... ");
    
    if(load_savedgames() != 0)
    {
        Z_error("Insufficient memory");
        quit_game();
    }
    
    Z_message("OK\n");
    
#ifdef _WIN32
    // Nothing for them to do on other platforms
    set_display_switch_callback(SWITCH_IN,switch_in_callback);
    set_display_switch_callback(SWITCH_OUT,switch_out_callback);
#endif
    
    // AG logo
    if(!fast_start)
    {
        set_volume(240,-1);
        aglogo(tmp_scr, scrollbuf, resx, resy);
        master_volume(digi_volume,midi_volume);
    }
    
    // play the game
    fix_menu();
    reset_items(true, &QHeader);
    
    clear_to_color(screen,BLACK);
    Quit = (fast_start||skip_title) ? qQUIT : qRESET;
    
    rgb_map = &rgb_table;
    
    // set up an initial game save slot (for the list_saves function)
    game = new gamedata;
    game->Clear();
    
    hangcount = get_config_int("ZSCRIPT","ZASM_Hangcount",1000);
    
#ifdef _WIN32
    
    if(use_win32_proc != FALSE)
    {
        al_trace("Config file warning: \"zc_win_proc_fix\" enabled switch found. This can cause crashes on some computers.\n");
        
        if(win32data.zcSetCustomCallbackProc(win_get_window()) != 0)
        {
            use_win32_proc = FALSE;
        }
    }
    
#endif
    
    while(Quit!=qEXIT)
    {
        // this is here to continually fix the keyboard repeat
        set_keyboard_rate(250,33);
        toogam = false;
        ignoreSideview=false;
        titlescreen(load_save);
        
        load_save=0;
        setup_combo_animations();
        setup_combo_animations2();
        
        while(!Quit)
        {
#ifdef _WIN32
        
            if(use_win32_proc != FALSE)
            {
                win32data.Update(0);
            }
            
#endif
            game_loop();
            advanceframe(true);
        }
        
        tmpscr->flags3=0;
        Playing=Paused=false;
        
        switch(Quit)
        {
        case qQUIT:
        case qGAMEOVER:
        {
            playing_field_offset=56; // Fixes an issue with Link being drawn wrong when quakeclk>0
            Link.setDontDraw(false);
            show_subscreen_dmap_dots=true;
            show_subscreen_numbers=true;
            show_subscreen_items=true;
            show_subscreen_life=true;
            show_ff_scripts=false;
            introclk=intropos=0;
            
            initZScriptGlobalRAM();
            ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_END);
            
            if(!skipcont&&!get_bit(quest_rules,qr_NOCONTINUE)) game_over(get_bit(quest_rules,qr_NOSAVE));
            
            skipcont = 0;
        }
        break;
        
        case qWON:
        {
            Link.setDontDraw(false);
		//Link.setCharging(0);//don't have the sword out during the ending. 
		//Link.setSwordClk(0);
            show_subscreen_dmap_dots=true;
            show_subscreen_numbers=true;
            show_subscreen_items=true;
            show_subscreen_life=true;
            
            initZScriptGlobalRAM();
            ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_END);
            ending();
        }
        break;
        
        }
        
        kill_sfx();
        music_stop();
        clear_to_color(screen,BLACK);
    }
    
    // clean up
    
    music_stop();
    kill_sfx();
    
quick_quit:
    show_saving(screen);
    save_savedgames();
    save_game_configs();
    Triplebuffer.Destroy();
    set_gfx_mode(GFX_TEXT,80,25,0,0);
    //rest(250); // ???
    //  if(useCD)
    //    cd_exit();
    quit_game();
    Z_message("Armageddon Games web site: http://www.armageddongames.com\n");
    Z_message("Zelda Classic web site: http://www.zeldaclassic.com\n");
    Z_message("Zelda Classic wiki: http://www.shardstorm.com/ZCwiki/\n");
    
    __zc_debug_malloc_free_print_memory_leaks(); //this won't do anything without debug_malloc_logging defined.
    
    if(forceExit) //fix for the allegro at_exit() hang.
        exit(0);
        
    allegro_exit();
    return 0;
}
END_OF_MAIN()


void remove_installed_timers()
{
    al_trace("Removing timers. \n");
    remove_int(update_logic_counter);
    Z_remove_timers();
#ifdef _SCRIPT_COUNTER
    remove_int(update_script_counter);
#endif
    
}


void delete_everything_else() //blarg.
{
    delete_combo_aliases();
    reset_subscr_items();
    delete_selectors();
    Sitems.clear();
    
}

void quit_game()
{
    script_drawing_commands.Dispose(); //for allegro bitmaps
    
    remove_installed_timers();
    delete_everything_else();
    
    al_trace("Freeing Data: \n");
    
    if(game) delete game;
    
    if(data) unload_datafile(data);
    
    if(fontsdata) unload_datafile(fontsdata);
    
    if(sfxdata) unload_datafile(sfxdata);
    
    //if(mididata) unload_datafile(mididata);
    //  if(mappic)
    //    destroy_bitmap(mappic);
    
    al_trace("Bitmaps... \n");
    destroy_bitmap(framebuf);
    destroy_bitmap(scrollbuf);
    destroy_bitmap(tmp_scr);
    destroy_bitmap(screen2);
    destroy_bitmap(tmp_bmp);
    destroy_bitmap(fps_undo);
    destroy_bitmap(prim_bmp);
    set_clip_state(msgdisplaybuf, 1);
    destroy_bitmap(msgdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    destroy_bitmap(pricesdisplaybuf);
	destroy_bitmap(zcmouse[0]);
	destroy_bitmap(zcmouse[1]);
	destroy_bitmap(zcmouse[2]);
	destroy_bitmap(zcmouse[3]);
    
    al_trace("Subscreens... \n");
    
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<MAXSUBSCREENITEMS; j++)
        {
            switch(custom_subscreen[i].objects[j].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                if(custom_subscreen[i].objects[j].dp1 != NULL) delete[](char *)custom_subscreen[i].objects[j].dp1;
            }
        }
    }
    
    al_trace("SFX... \n");
    zcmusic_exit();
    
    for(int i=0; i<WAV_COUNT; i++)
    {
        delete [] sfx_string[i];
        
        if(customsfxdata[i].data!=NULL)
        {
//      delete [] customsfxdata[i].data;
            zc_free(customsfxdata[i].data);
        }
    }
    
    al_trace("Misc... \n");
    
    for(int i=0; i<WPNCNT; i++)
    {
        delete [] weapon_string[i];
    }
    
    for(int i=0; i<ITEMCNT; i++)
    {
        delete [] item_string[i];
    }
    
    for(int i=0; i<eMAXGUYS; i++)
    {
        delete [] guy_string[i];
    }
    
    al_trace("Script buffers... \n");
    
    for(int i=0; i<512; i++)
    {
        if(ffscripts[i]!=NULL) delete [] ffscripts[i];
    }
    
    for(int i=0; i<256; i++)
    {
        if(itemscripts[i]!=NULL) delete [] itemscripts[i];
    }
    
    for(int i=0; i<256; i++)
    {
        if(guyscripts[i]!=NULL) delete [] guyscripts[i];
    }
    
    for(int i=0; i<256; i++)
    {
        if(wpnscripts[i]!=NULL) delete [] wpnscripts[i];
    }
    
    for(int i=0; i<256; i++)
    {
        if(screenscripts[i]!=NULL) delete [] screenscripts[i];
    }
    
    for(int i=0; i<NUMSCRIPTGLOBAL; i++)
    {
        if(globalscripts[i]!=NULL) delete [] globalscripts[i];
    }
    
    for(int i=0; i<3; i++)
    {
        if(linkscripts[i]!=NULL) delete [] linkscripts[i];
    }
    
    delete zscriptDrawingRenderTarget;
    
    //for(int i=0; i<map_count*MAPSCRS; i++)
    //{
    //if(TheMaps[i].data != NULL) delete [] TheMaps[i].data;
    //if(TheMaps[i].sflag != NULL) delete [] TheMaps[i].sflag;
    //if(TheMaps[i].cset != NULL) delete [] TheMaps[i].cset;
    //}
    al_trace("Screen Data... \n");
    
    al_trace("Deleting quest buffers... \n");
    del_qst_buffers();
    
    if(qstdir) zc_free(qstdir);
    
    if(qstpath) zc_free(qstpath);
    
    //if(TheMaps != NULL) zc_free(TheMaps);
    //if(ZCMaps != NULL) zc_free(ZCMaps);
    //  dumb_exit();
}


int d_timer_proc(int, DIALOG *, int)
{
    return D_O_K;
}






/////////////////////////////////////////////////
// zc_malloc
/////////////////////////////////////////////////

//Want Logging:
//Set this to 1 to allow zc_malloc/zc_free to track pointers and
//write logging data to allegro.log
#define ZC_DEBUG_MALLOC_WANT_LOGGING_INFO 0


#include "vectorset.h"

#if (defined(NDEBUG) || !defined(_DEBUG)) && (ZC_DEBUG_MALLOC_ENABLED) && (ZC_DEBUG_MALLOC_WANT_LOGGING_INFO) //this is not fun with debug
#define ZC_WANT_DETAILED_MALLOC_LOGGING 1
#endif


#if ZC_WANT_DETAILED_MALLOC_LOGGING
size_t totalBytesAllocated = 0;
typedef vectorset<void*> debug_malloc_pool_type;
debug_malloc_pool_type debug_zc_malloc_allocated_pool;
#endif

void* __zc_debug_malloc(size_t numBytes, const char* file, int line)
{
#if ZC_WANT_DETAILED_MALLOC_LOGGING
    static bool zcDbgMallocInit = false;
    
    if(!zcDbgMallocInit)
    {
        zcDbgMallocInit = true;
        debug_zc_malloc_allocated_pool.reserve(1 << 17);
        //yeah. completely ridiculous... there's no reason zc should ever need this many..
        //BUT it does... go figure
    }
    
    totalBytesAllocated += numBytes;
    
    //char buf[1024];
    //sprintf(buf, "%i : %s, line %i, %u bytes allocated.\n", 0, file, line, numBytes);
    //al_trace("%s", buf);
    
    al_trace("info: %i : %s, line %i, %u bytes, pool size %u, total %u,",
             0,
             file,
             line,
             numBytes,
             debug_zc_malloc_allocated_pool.size(),
             totalBytesAllocated / 1024
            );
#endif
            
    ZC_MALLOC_ALWAYS_ASSERT(numBytes != 0);
    void* p = malloc(numBytes);
    
#if ZC_WANT_DETAILED_MALLOC_LOGGING
    al_trace("at address %x\n", (int)p);
    
    if(!p)
        al_trace("____________ ERROR: __zc_debug_malloc: returned null. out of memory.\n");
        
    debug_malloc_pool_type::insert_iterator_type it = debug_zc_malloc_allocated_pool.insert(p);
    
    if(!it.second)
        al_trace("____________ ERROR: malloc returned identical address to one in use... No way Jose!\n");
        
#endif
        
    return p;
}


void __zc_debug_free(void* p, const char* file, int line)
{
    ZC_MALLOC_ALWAYS_ASSERT(p != 0);
    
#if ZC_WANT_DETAILED_MALLOC_LOGGING
    al_trace("alloc info: %i : %s line %i, freeing memory at address %x\n", 0, file, line, (int)p);
    
    size_t numErased = debug_zc_malloc_allocated_pool.erase(p);
    
    if(numErased == 0)
        al_trace("____________ ERROR: __zc_debug_free: no known ptr to memory exists. ..attempting to free it anyways.\n");
        
#endif
        
    free(p);
}


void __zc_debug_malloc_free_print_memory_leaks()
{
#if ZC_WANT_DETAILED_MALLOC_LOGGING
    al_trace("LOGGING INFO FROM debug_zc_malloc_allocated_pool:\n");
    
    for(debug_malloc_pool_type::iterator it = debug_zc_malloc_allocated_pool.begin();
            it != debug_zc_malloc_allocated_pool.end();
            ++it
       )
    {
        al_trace("block at address %x.\n", (int)*it);
    }
    
#endif
}


void __zc_always_assert(bool e, const char* expression, const char* file, int line)
{
    if(!e)
    {
        //for best results set a breakpoint in here.
        char buf[1024];
        sprintf("ASSERTION FAILED! : %s, %s line %i\n", expression, file, line);
        
        al_trace("%s", buf);
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("%s", buf);
        //exit(-1); //flashing lights are probably enough.
    }
}


/***************************/
/******  .zmod Light  ******/
/***************************/

void ZModule::init() //bool default
{
	
	
	memset(moduledata.module_name, 0, sizeof(moduledata.module_name));
	memset(moduledata.quests, 0, sizeof(moduledata.quests));
	memset(moduledata.skipnames, 0, sizeof(moduledata.skipnames));
	memset(moduledata.datafiles, 0, sizeof(moduledata.datafiles));
	
	memset(moduledata.startingdmap, 0, sizeof(moduledata.startingdmap));
	memset(moduledata.startingscreen, 0, sizeof(moduledata.startingscreen));
	
	moduledata.max_quest_files = 0;
	moduledata.old_quest_serial_flow = 0;
	
	//strcpy(moduledata.module_name,"default.zmod");
	//al_trace("Module name set to %s\n",moduledata.module_name);
	//We load the current module name from zc.cfg or zquest.cfg!
	//Otherwise, we don't know what file to access to load the module vars! 
	strcpy(moduledata.module_name,get_config_string("ZCMODULE","current_module","classic.zmod"));
	al_trace("\n\nThe Current ZC Player Module is: %s\n\n",moduledata.module_name); 
		
	//zcm path
	set_config_file(moduledata.module_name); //Switch to the module to load its config properties.
	//al_trace("Module name set to %s\n",moduledata.module_name);
	
	//quests
	moduledata.old_quest_serial_flow = get_config_int("QUESTS","quest_flow",1);
	
	moduledata.max_quest_files = get_config_int("QUESTS","num_quest_files",5);
	//al_trace("Module flow set to %d\n",moduledata.old_quest_serial_flow);
	//al_trace("Module number of serial quests set to %d\n",moduledata.max_quest_files);
	strcpy(moduledata.quests[0],get_config_string("QUESTS","first_qst","1st.qst"));
	//al_trace("Module quest 1 set to %s\n",moduledata.quests[0]);
	strcpy(moduledata.quests[1],get_config_string("QUESTS","second_qst","2nd.qst"));
	//al_trace("Module quest 2 set to %s\n",moduledata.quests[1]);
	strcpy(moduledata.quests[2],get_config_string("QUESTS","third_qst","3rd.qst"));
	//al_trace("Module quest 3 set to %s\n",moduledata.quests[2]);
	strcpy(moduledata.quests[3],get_config_string("QUESTS","fourth_qst","4th.qst"));
	//al_trace("Module quest 4 set to %s\n",moduledata.quests[3]);
	strcpy(moduledata.quests[4],get_config_string("QUESTS","fifth_qst","5th.qst"));
	strcpy(moduledata.quests[5],get_config_string("QUESTS","sixth_qst","6th.qst"));
	strcpy(moduledata.quests[6],get_config_string("QUESTS","seventh_qst","7th.qst"));
	strcpy(moduledata.quests[7],get_config_string("QUESTS","eighth_qst","8th.qst"));
	strcpy(moduledata.quests[8],get_config_string("QUESTS","ninth_qst","9th.qst"));
	strcpy(moduledata.quests[9],get_config_string("QUESTS","tenth_qst","10th.qst"));
	//al_trace("Module quest 5 set to %s\n",moduledata.quests[4]);
	
	//quest skip names
	strcpy(moduledata.skipnames[0],get_config_string("NAMEENTRY","first_qst_skip"," "));
	//al_trace("Module quest skip 1 set to %s\n",moduledata.skipnames[0]);
	strcpy(moduledata.skipnames[1],get_config_string("NAMEENTRY","second_qst_skip","ZELDA"));
	//al_trace("Module quest skip 2 set to %s\n",moduledata.skipnames[1]);
	strcpy(moduledata.skipnames[2],get_config_string("NAMEENTRY","third_qst_skip","ALPHA"));
	//al_trace("Module quest skip 3 set to %s\n",moduledata.skipnames[2]);
	strcpy(moduledata.skipnames[3],get_config_string("NAMEENTRY","fourth_qst_skip","GANON"));
	//al_trace("Module quest skip 4 set to %s\n",moduledata.skipnames[3]);
	strcpy(moduledata.skipnames[4],get_config_string("NAMEENTRY","fifth_qst_skip","JEAN"));
	strcpy(moduledata.skipnames[5],get_config_string("NAMEENTRY","sixth_qst_skip","OMNIUS"));
	strcpy(moduledata.skipnames[6],get_config_string("NAMEENTRY","seventh_qst_skip","MCCOY"));
	strcpy(moduledata.skipnames[7],get_config_string("NAMEENTRY","eighth_qst_skip","MCGANN"));
	strcpy(moduledata.skipnames[8],get_config_string("NAMEENTRY","ninth_qst_skip","HURT"));
	strcpy(moduledata.skipnames[9],get_config_string("NAMEENTRY","tenth_qst_skip","ECCLESTON"));
	//al_trace("Module quest skip 5 set to %s\n",moduledata.skipnames[4]);
	
	//Quest starting screens and DMaps
	
	//dmaps
	moduledata.startingdmap[0] = get_config_int("QUESTS","first_startdmap",0);
	moduledata.startingdmap[1] = get_config_int("QUESTS","second_startdmap",0);
	moduledata.startingdmap[2] = get_config_int("QUESTS","third_startdmap",0);
	moduledata.startingdmap[3] = get_config_int("QUESTS","fourth_startdmap",0);
	moduledata.startingdmap[4] = get_config_int("QUESTS","fifth_startdmap",0);
	moduledata.startingdmap[5] = get_config_int("QUESTS","sixth_startdmap",0);
	moduledata.startingdmap[6] = get_config_int("QUESTS","seventh_startdmap",0);
	moduledata.startingdmap[7] = get_config_int("QUESTS","eighth_startdmap",0);
	moduledata.startingdmap[8] = get_config_int("QUESTS","ninth_startdmap",0);
	moduledata.startingdmap[9] = get_config_int("QUESTS","tenth_startdmap",0);
	//screens
	moduledata.startingscreen[0] = get_config_int("QUESTS","first_startscreen",0x77);
	moduledata.startingscreen[1] = get_config_int("QUESTS","second_startscreen",0x77);
	moduledata.startingscreen[2] = get_config_int("QUESTS","third_startscreen",0x77);
	moduledata.startingscreen[3] = get_config_int("QUESTS","fourth_startscreen",0x77);
	moduledata.startingscreen[4] = get_config_int("QUESTS","fifth_startscreen",0x77);
	moduledata.startingscreen[5] = get_config_int("QUESTS","sixth_startscreen",0x77);
	moduledata.startingscreen[6] = get_config_int("QUESTS","seventh_startscreen",0x77);
	moduledata.startingscreen[7] = get_config_int("QUESTS","eighth_startscreen",0x77);
	moduledata.startingscreen[8] = get_config_int("QUESTS","ninth_startscreen",0x77);
	moduledata.startingscreen[9] = get_config_int("QUESTS","tenth_startscreen",0x77);
	//al_trace("startscreen base is: %x\n", moduledata.startingscreen[0]);
	
	//datafiles
	strcpy(moduledata.datafiles[zelda_dat],get_config_string("DATAFILES","zcplayer_datafile","zelda.dat"));
	al_trace("Module zelda_dat set to %s\n",moduledata.datafiles[zelda_dat]);
	strcpy(moduledata.datafiles[zquest_dat],get_config_string("DATAFILES","zquest_datafile","zquest.dat"));
	al_trace("Module zquest_dat set to %s\n",moduledata.datafiles[zquest_dat]);
	strcpy(moduledata.datafiles[fonts_dat],get_config_string("DATAFILES","fonts_datafile","fonts.dat"));
	al_trace("Module fonts_dat set to %s\n",moduledata.datafiles[fonts_dat]);
	strcpy(moduledata.datafiles[sfx_dat],get_config_string("DATAFILES","sounds_datafile","sfx.dat"));
	al_trace("Module sfx_dat set to %s\n",moduledata.datafiles[sfx_dat]);
	strcpy(moduledata.datafiles[qst_dat],get_config_string("DATAFILES","quest_template_datafile","qst.dat"));
	al_trace("Module qst_dat set to %s\n",moduledata.datafiles[qst_dat]);
	
	set_config_file("zc.cfg"); //shift back to the normal config file, when done
	
}


/*** end of zelda.cc ***/

