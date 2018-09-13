//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.h
//
//  Definitions, function prototypes, etc. for zelda.cc
//
//--------------------------------------------------------

#ifndef _ZELDA_H_
#define _ZELDA_H_

/*********************************/
/********** Definitions **********/
/*********************************/

#include <vector>
#include "zdefs.h"
#include "zc_array.h"
#include "zc_sys.h"
#include "zeldadat.h"
#include "sfx.h"
#include "zcmusic.h"
#include "jwin.h"
#include "gamedata.h"
#include "zsys.h"
#include "script_drawing.h"

int isFullScreen();
int onFullscreen();

#define  MAXMIDIS     ZC_MIDI_COUNT+MAXCUSTOMTUNES

#define MAX_IDLE      72000                                 // 20 minutes
#define MAX_ACTIVE    72000                                 // 20 minutes

// saved games
#define MAXSAVES      (standalone_mode?1:15) // It's constant enough... :p

// game.maps[] flags
#define mSECRET             8192                                 // only overworld and caves use this
#define mVISITED            16384                                // only overworld uses this

#define mDOOR_UP            1                                 // only dungeons use this
#define mDOOR_DOWN          2                                 //        ''
#define mDOOR_LEFT          4                                 //        ''
#define mDOOR_RIGHT         8                                 //        ''

#define mITEM              16                                 // item (main screen)
#define mBELOW             32                                 // special item (underground)
#define mNEVERRET          64                                 // enemy never returns
#define mTMPNORET         128                                 // enemies don't return until you leave the dungeon

#define mLOCKBLOCK        256                                 // if the lockblock on the screen has been triggered
#define mBOSSLOCKBLOCK    512                                 // if the bosslockblock on the screen has been triggered

#define mCHEST           1024                                 // if the unlocked check on this screen has been opened
#define mLOCKEDCHEST     2048                                 // if the locked chest on this screen has been opened
#define mBOSSCHEST       4096                                 // if the boss chest on this screen has been opened
#define mOTHER3         32768                                 // overwrite this value, use for expansion

/*********************************/
/******** Enums & Structs ********/
/*********************************/

enum { qQUIT=1, qRESET, qEXIT, qGAMEOVER, qCONT, qWON, qERROR };

// "special" walk flags
enum
{
    spw_none, spw_door, spw_clipright, spw_floater, spw_trap, spw_halfstep,
    spw_water, spw_wizzrobe, spw_clipbottomright
};

// death & fade defs
enum
{
    fade_none, fade_flicker, fade_invisible, fade_flash_die,
    fade_blue_poof
};

/*********************************/
/*********** Procedures **********/
/*********************************/

/*

  // aglogo.cc
  int  aglogo(BITMAP* frame);


  // title.cc
  void update_game_icons();

  // zc_sys.cc
  void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int from,int to);
  void go();
  void comeback();
  void waitvsync(bool fast);
  int  input_idle(bool checkmouse);
  int  after_time();
  void hit_close_button();
  */

void port250QuestRules();

bool get_debug();
void set_debug(bool d);

void Z_eventlog(const char *format, ...);
void Z_scripterrlog(const char * const format, ...);

// zelda.cc
void addLwpn(int x,int y,int z,int id,int type,int power,int dir, int parentid);
void addLwpnEx(int x,int y,int z,int id,int type,int power,int dir, int parentitem, int parentid, byte script_gen);
void ALLOFF(bool messagesToo = true, bool decorationsToo = true);
void centerLink();
fix  LinkX();
fix  LinkY();
fix  LinkZ();
int  LinkHClk();
int  LinkNayrusLoveShieldClk();
int  LinkHoverClk();
int  LinkSwordClk();
int  LinkItemClk();
int  LinkAction();
int  LinkCharged();
byte LinkGetDontDraw();
void setSwordClk(int newclk);
void setItemClk(int newclk);
int  LinkLStep();
void LinkCheckItems();
fix  LinkModifiedX();
fix  LinkModifiedY();
fix  GuyX(int j);
fix  GuyY(int j);
int  GuyID(int j);
int  GuyMisc(int j);
void StunGuy(int j,int stun);
bool  GuySuperman(int j);
int  GuyCount();
int  LinkDir();
void add_grenade(int wx, int wy, int wz, int size, int parentid);
fix distance(int x1, int y1, int x2, int y2);
bool getClock();
void setClock(bool state);
void CatchBrang();;
int LinkAction();

void do_dcounters();

void clearmsgnext(int str);
void donewmsg(int str);
int donew_shop_msg(int itmstr, int shopstr);
void dismissmsg();
void dointro();
void init_dmap();
int  init_game();
int  cont_game();
void restart_level();
int  load_quest(gamedata *g, bool report=true);
void show_details();
void show_ffscript_names();
//int  init_palnames();

int get_currdmap();
int get_dlevel();
int get_currscr();
int get_homescr();
int get_bmaps(int si);
bool no_subscreen();
bool is_zquest();
bool screenIsScrolling();
//void quit_game();
int d_timer_proc(int msg, DIALOG *d, int c);

//2.54
void setMonochrome(bool state); //GFX are monochrome. 
bool isMonochrome();

void runDrunkRNG();

//Save screen settings. 
enum { 	SAVESC_BACKGROUND, 		SAVESC_TEXT, 			SAVESC_USETILE, 	
	SAVESC_CURSOR_CSET, 		SAVESC_CUR_SOUND,  		SAVESC_TEXT_CONTINUE_COLOUR, 
	SAVESC_TEXT_SAVE_COLOUR, 	SAVESC_TEXT_RETRY_COLOUR, 	SAVESC_TEXT_CONTINUE_FLASH, 
	SAVESC_TEXT_SAVE_FLASH, 	SAVESC_TEXT_RETRY_FLASH,	SAVESC_MIDI,
	SAVESC_CUR_FLIP, 		SAVSC_TEXT_DONTSAVE_COLOUR, 	SAVESC_TEXT_SAVEQUIT_COLOUR, 
	SAVESC_TEXT_SAVE2_COLOUR, 	SAVESC_TEXT_QUIT_COLOUR, 	SAVSC_TEXT_DONTSAVE_FLASH,
	SAVESC_TEXT_SAVEQUIT_FLASH,	SAVESC_TEXT_SAVE2_FLASH, 	SAVESC_TEXT_QUIT_FLASH,
	SAVESC_EXTRA1, 			SAVESC_EXTRA2,			SAVESC_EXTRA3,			
	SAVESC_LAST	};

extern long SaveScreenSettings[24]; //BG, Text, Cursor CSet, MIDI
//Save Screen text. 
enum { SAVESC_CONTINUE, SAVESC_SAVE, SAVESC_RETRY, SAVESC_STRING_MISC1, SAVESC_STRING_MISC2, SAVESC_STRING_MISC3 };
extern char SaveScreenText[7][32]; //(char *) "CONTINUE", (char *) "SAVE", (char*) "RETRY" , 
					//DON'T SAVE, SAVE AND QUIT, SAVE, QUIT
extern void SetSaveScreenSetting(int indx, int value);
extern void ChangeSubscreenText(int index, const char *f);
INLINE void sfx(int index)
{
    sfx(index,128,false);
}
INLINE void sfx(int index,int pan)
{
    sfx(index,vbound(pan, 0, 255) ,false);
}

//INLINE void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }

/**********************************/
/******** Global Variables ********/
/**********************************/


extern ZCMUSIC *zcmusic;

extern int colordepth;
extern int db;
extern int detail_int[10];                                  //temporary holder for things you want to detail
extern zinitdata  zinit;
extern int lens_hint_item[MAXITEMS][2];                     //aclk, aframe
extern int lens_hint_weapon[MAXWPNS][5];                    //aclk, aframe, dir, x, y
extern int strike_hint_counter;
extern int strike_hint_timer;
extern int strike_hint;

extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table, trans_table2;
extern BITMAP     *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *screen2, *fps_undo, *msgbmpbuf, *msgdisplaybuf, *pricesdisplaybuf, *tb_page[3], *real_screen, *temp_buf, *temp_buf2, *prim_bmp;
extern DATAFILE *data, *sfxdata, *fontsdata, *mididata;
extern SAMPLE   wav_refill;
extern FONT  *nfont, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
       *goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont,
       
		//New fonts for 2.54+
		*atari800font, *acornfont, *adosfont, *baseallegrofont, *apple2font, *apple280colfont, *apple2gsfont,
		*aquariusfont, *atari400font, *c64font, *c64hiresfont, *cgafont, *cocofont, *coco2font,
		*coupefont, *cpcfont, *fantasyfont, *fdskanafont, *fdslikefont, *fdsromanfont, *finalffont,
		*futharkfont, *gaiafont, *hirafont, *jpfont, *kongfont, *manafont, *mlfont, *motfont,
		*msxmode0font, *msxmode1font, *petfont, *pstartfont, *saturnfont, *scififont, *sherwoodfont,
		*sinqlfont, *spectrumfont, *speclgfont, *ti99font, *trsfont, *z2font, *zxfont, *lisafont;
//extern FONT custom_fonts[MAXFONTS];
extern PALETTE  RAMpal;
extern byte     *colordata;
//extern byte     *tilebuf;
extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern comboclass *combo_class_buf;
extern guydata  *guysbuf;
extern item_drop_object    item_drop_sets[MAXITEMDROPSETS];
extern ZCHEATS  zcheats;
extern byte     use_cheats;
extern byte     use_tiles;
extern char     palnames[MAXLEVELS][17];

extern bool is_large;

extern bool standalone_mode;
extern char *standalone_quest;
extern bool skip_title;

extern int draw_screen_clip_rect_x1; //Used by the ending, bu could be used to change the drawn screen size. 
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;

/*
extern tiledata *newtilebuf, *grabtilebuf;
extern newcombo *combobuf;
extern word animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos;
extern word animated_combo_table2[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table24[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos2;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tiles
*/
extern bool ewind_restart;
extern word     msgclk, msgstr, msgpos, msgptr, msg_count, msgcolour, msgspeed,msg_w,
       msg_h,
       msg_count,
       msgorig,
       msg_xpos,
       msg_ypos,
       cursor_x,
       cursor_y;
extern bool msg_onscreen, msg_active,msgspace;
extern FONT	*msgfont;
extern word     door_combo_set_count;
extern word     introclk, intropos, dmapmsgclk, linkedmsgclk;
extern short    lensclk;
extern int     lensid;
extern int    Bpos;
extern byte screengrid[22];
extern byte ffcgrid[4];
extern volatile int logic_counter;
#ifdef _SCRIPT_COUNTER
extern volatile int script_counter;
#endif
extern bool halt;
extern bool screenscrolling;
extern bool close_button_quit;
extern int jwin_pal[jcMAX];
extern int gui_colorset;
extern int fullscreen;
extern byte disable_triplebuffer, can_triplebuffer_in_windowed_mode;
extern byte frame_rest_suggest, forceExit, zc_vsync;
extern byte zc_color_depth;
extern byte use_debug_console, use_win32_proc; //windows only

#ifdef _SCRIPT_COUNTER
void update_script_counter();
#endif

extern PALETTE tempbombpal;
extern bool usebombpal;

extern int slot_arg, slot_arg2;
extern char *SAVE_FILE;

extern int homescr,currscr,frame,currmap,dlevel,warpscr,worldscr;
extern int newscr_clk,opendoors,currdmap,fadeclk,currgame,listpos;
extern int lastentrance,lastentrance_dmap, prices[3],loadside, Bwpn, Awpn;
extern int digi_volume,midi_volume,sfx_volume,emusic_volume,currmidi,hasitem,whistleclk,pan_style;
extern bool analog_movement;
extern int joystick_index,Akey,Bkey,Skey,Lkey,Rkey,Pkey,Exkey1,Exkey2,Exkey3,Exkey4,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Exbtn1,Exbtn2,Exbtn3,Exbtn4,Quit;
extern int js_stick_1_x_stick, js_stick_1_x_axis, js_stick_1_x_offset;
extern int js_stick_1_y_stick, js_stick_1_y_axis, js_stick_1_y_offset;
extern int js_stick_2_x_stick, js_stick_2_x_axis, js_stick_2_x_offset;
extern int js_stick_2_y_stick, js_stick_2_y_axis, js_stick_2_y_offset;
extern int DUkey, DDkey, DLkey, DRkey, DUbtn, DDbtn, DLbtn, DRbtn, ss_after, ss_speed, ss_density, ss_enable;
extern int hs_startx, hs_starty, hs_xdist, hs_ydist, clockclk, clock_zoras[eMAXGUYS];
extern int swordhearts[4], currcset, gfc, gfc2, pitx, pity, refill_what, refill_why;
extern int heart_beep_timer, new_enemy_tile_start, nets, magicitem, nayruitem, title_version;
extern int magiccastclk, castx, casty, quakeclk, wavy, df_x, df_y, nl1_x, nl1_y, nl2_x, nl2_y, magicdrainclk, conveyclk, memrequested;
extern dword fps_secs;
extern float avgfps;

extern bool do_cheat_goto, do_cheat_light;
extern bool blockmoving;
extern bool Throttlefps, ClickToFreeze, Paused, Advance, ShowFPS, Showpal, Playing, FrameSkip, TransLayers, disableClickToFreeze;
extern bool refreshpal,blockpath,__debug,loaded_guys,freeze_guys;
extern bool loaded_enemies,drawguys,details,debug_enabled,watch;
extern bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown,Ex1down,Ex2down,Ex3down,Ex4down,AUdown,ADdown,ALdown,ARdown,F12,F11,F5,keyI, keyQ;
extern bool SystemKeys,NESquit,volkeys,useCD,boughtsomething;
extern bool fixed_door, darkroom,naturaldark,BSZ;            //,NEWSUBSCR;
extern bool hookshot_used, hookshot_frozen, pull_link, add_chainlink;
extern bool del_chainlink, hs_fix, cheat_superman, gofast, checklink;
extern bool ewind_restart, didpit, heart_beep, pausenow, castnext;
extern bool add_df1asparkle, add_df1bsparkle, add_nl1asparkle, add_nl1bsparkle, add_nl2asparkle, add_nl2bsparkle;
extern bool is_on_conveyor, activated_timed_warp;

extern byte COOLSCROLL;

extern int SnapshotFormat, NameEntryMode;

extern int add_asparkle, add_bsparkle;

extern bool show_layer_0, show_layer_1, show_layer_2, show_layer_3, show_layer_4, show_layer_5, show_layer_6, show_layer_over, show_layer_push, show_sprites, show_ffcs, show_hitboxes, show_walkflags, show_ff_scripts;

extern int    cheat_goto_dmap, cheat_goto_screen;
extern char   cheat_goto_dmap_str[4];
extern char   cheat_goto_screen_str[3];
extern short  visited[6];
extern byte   guygrid[176];
extern mapscr tmpscr[2];
extern mapscr tmpscr2[6];
extern mapscr tmpscr3[6];
extern char   sig_str[44];
extern ffscript *ffscripts[512];
extern ffscript *itemscripts[256];
extern ffscript *globalscripts[NUMSCRIPTGLOBAL];

extern ffscript *guyscripts[256];
extern ffscript *wpnscripts[256];
extern ffscript *linkscripts[3];
extern ffscript *screenscripts[256];
extern SAMPLE customsfxdata[WAV_COUNT];
extern int sfxdat;

#define MAX_ZCARRAY_SIZE	4096
typedef ZCArray<long> ZScriptArray;
extern ZScriptArray localRAM[MAX_ZCARRAY_SIZE];
extern byte arrayOwner[MAX_ZCARRAY_SIZE];

dword getNumGlobalArrays();

extern int  resx,resy,scrx,scry;
extern bool sbig;                                           // big screen
extern bool sbig2;	//BIGGER SCREEN!!!!
extern int screen_scale; //user adjustable screen size.

extern bool scanlines;                                      //do scanlines if sbig==1
extern bool toogam;
extern bool ignoreSideview;

extern int cheat;                                           // 0 = none; 1,2,3,4 = cheat level

extern int  mouse_down;                                     // used to hold the last reading of 'gui_mouse_b()' status
extern int idle_count, active_count;
extern char *qstpath;
extern char *qstdir;
extern gamedata *saves;
extern gamedata *game;

extern volatile int lastfps;
extern volatile int framecnt;
extern void throttleFPS();

// quest file data
extern zquestheader QHeader;
extern byte                quest_rules[QUESTRULES_SIZE];
extern byte                extra_rules[EXTRARULES_SIZE];
extern byte                midi_flags[MIDIFLAGS_SIZE];
extern byte                music_flags[MUSICFLAGS_SIZE];
extern word                map_count;
extern MsgStr              *MsgStrings;
extern int				   msg_strings_size;
extern DoorComboSet        *DoorComboSets;
extern dmap                *DMaps;
extern miscQdata           QMisc;
extern std::vector<mapscr> TheMaps;
extern zcmap               *ZCMaps;
extern byte                *quest_file;

/**********************************/
/*********** Misc Data ************/
/**********************************/

extern const char startguy[8];
extern const char gambledat[12*6];
extern const byte stx[4][9];
extern const byte sty[4][9];
extern const byte ten_rupies_x[10];
extern const byte ten_rupies_y[10];
extern zctune tunes[MAXMIDIS];
//extern zcmidi_ tunes[MAXMIDIS];
//extern emusic enhancedMusic[MAXMUSIC];
#endif

/*** end of zelda.h ***/

