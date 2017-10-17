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

#include "zdefs.h"
#include "zc_sys.h"
#include "zeldadat.h"
#include "sfx.h"
#include "zcmusic.h"
#include "jwin.h"

#define  MAXMUSIC     MUSIC_COUNT+MAXMIDIS
#define  WAV_COUNT    WAV_ZELDA+1

#define MAX_IDLE      72000                                 // 20 minutes

// saved games
//#define MAXSAVES      24
#define MAXSAVES      15

// game.maps[] flags
#define mSECRET           1                                 // only overworld and caves use this
#define mVISITED          2                                 // only overworld uses this

#define mDOOR_UP          1                                 // only dungeons use this
#define mDOOR_DOWN        2                                 //        ''
#define mDOOR_LEFT        4                                 //        ''
#define mDOOR_RIGHT       8                                 //        ''

#define mITEM            16                                 // item (main screen)
#define mBELOW           32                                 // special item (underground)
#define mNEVERRET        64                                 // enemy never returns
#define mTMPNORET       128                                 // enemies don't return until you leave the dungeon

#define mLOCKBLOCK      256                                 // if the lockblock on the screen has been triggered
#define mBOSSLOCKBLOCK  512                                 // if the bosslockblock on the screen has been triggered

/*********************************/
/******** Enums & Structs ********/
/*********************************/

enum { qQUIT=1, qRESET, qEXIT, qGAMEOVER, qCONT, qWON, qERROR };

//magic types
enum  { mgc_none, mgc_dinsfire, mgc_nayruslove, mgc_faroreswind };

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
void waitvsync();
int  input_idle(bool checkmouse);
int  after_time();
void hit_close_button();
*/
// zelda.cc
void addLwpn(int x,int y,int id,int type,int power,int dir);
void ALLOFF();
fix  LinkX();
fix  LinkY();
int  LinkNayrusLoveShieldClk();
int  LinkLStep();
fix  LinkModifiedX();
fix  LinkModifiedY();
fix  GuyX(int j);
fix  GuyY(int j);
int  GuyID(int j);
int  GuyMisc(int j);
void StunGuy(int j);
bool  GuySuperman(int j);
int  GuyCount();
int  LinkDir();
void add_grenade(int wx, int wy, int size);
fix distance(int x1, int y1, int x2, int y2);
bool getClock();
void setClock(bool state);
void CatchBrang();
int LinkAction();

void dointro();
int  init_game();
int  cont_game();
void restart_level();
int  load_quest(gamedata *g, bool report=true);
void show_details();
//int  init_palnames();

inline void sfx(int index)         { sfx(index,128,false); }
inline void sfx(int index,int pan) { sfx(index,pan,false); }

//inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }

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
extern COLOR_MAP trans_table;
extern BITMAP     *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *screen2, *fps_undo, *msgdisplaybuf, *pricesdisplaybuf, *tb_page[3], *real_screen;
extern DATAFILE *data, *sfxdata, *fontsdata, *mididata;
extern SAMPLE   wav_refill;
extern FONT     *zfont, *z3font, *deffont, *lfont, *lfont_l, *pfont, *mfont;
extern PALETTE  RAMpal;
extern byte     *tilebuf, *colordata;
extern newcombo *combobuf;
extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern guydata  *guysbuf;
extern ZCHEATS  zcheats;
extern byte     use_cheats;
extern byte     use_tiles;
extern char     palnames[256][17];

extern word animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tiles
extern bool ewind_restart;
extern word     msgclk, msgstr, msgpos, msg_count;
extern word     door_combo_set_count;
extern word     introclk, intropos, dmapmsgclk, linkedmsgclk;
extern short    Bpos, lensclk, lenscnt;
extern byte screengrid[22];
extern int logic_counter;
extern bool drawit;
extern bool halt;
extern bool screenscrolling;
extern bool close_button_quit;
extern int jwin_pal[jcMAX];
extern int gui_colorset;

extern int homescr,currscr,frame,currmap,dlevel,warpscr,worldscr;
extern int newscr_clk,opendoors,currdmap,fadeclk,currgame,listpos;
extern int lastentrance,lastentrance_dmap,prices[3][2],loadside, Bwpn, Awpn;
extern int digi_volume,midi_volume,currmidi,wand_x,wand_y,hasitem,whistleclk,pan_style;
extern int Akey,Bkey,Skey,Lkey,Rkey,Pkey,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Quit;
extern int DUkey, DDkey, DLkey, DRkey, ss_after, ss_speed, ss_density;
extern int arrow_x, arrow_y, brang_x, brang_y, chainlink_x, chainlink_y;
extern int hs_startx, hs_starty, hs_xdist, hs_ydist, clockclk, clock_zoras;
extern int swordhearts[4], currcset, gfc, gfc2, pitx, pity, refill_what;
extern int heart_beep_timer, new_enemy_tile_start, nets, magictype;
extern int magiccastclk, castx, casty, df_x, df_y, nl1_x, nl1_y, nl2_x, nl2_y, magicdrainclk, conveyclk, memrequested;
extern dword fps_secs;
extern float avgfps;

extern bool nosecretsounds;
extern bool blockmoving;
extern bool Throttlefps, Paused, Advance, ShowFPS, Showpal, Playing, FrameSkip, TransLayers;
extern bool refreshpal,blockpath,wand_dead,debug,loaded_guys,freeze_guys;
extern bool loaded_enemies,drawguys,details,DXtitle,debug_enabled,watch;
extern bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown;
extern bool SystemKeys,NESquit,volkeys,useCD,boughtsomething;
extern bool fixed_door, darkroom,BSZ,COOLSCROLL;            //,NEWSUBSCR;
extern bool hookshot_used, hookshot_frozen, pull_link, add_chainlink;
extern bool del_chainlink, hs_fix, cheat_superman, gofast, checklink;
extern bool ewind_restart, didpit, heart_beep, pausenow, castnext;
extern bool add_df1asparkle, add_df1bsparkle, add_nl1asparkle, add_nl1bsparkle, add_nl2asparkle, add_nl2bsparkle;
extern bool is_on_conveyor, activated_timed_warp;

extern int add_asparkle, add_bsparkle;

extern int    cheat_goto_map, cheat_goto_screen;
extern char   cheat_goto_map_str[4];
extern char   cheat_goto_screen_str[3];
extern short  visited[6];
extern byte   guygrid[176];
extern mapscr tmpscr[2];
extern mapscr tmpscr2[6];
extern mapscr tmpscr3[6];
extern char   sig_str[44];

extern int  VidMode,resx,resy,scrx,scry;
extern bool sbig;                                           // big screen
extern bool scanlines;                                      //do scanlines if sbig==1
extern bool toogam;

extern int cheat;                                           // 0 = none; 1,2,3,4 = cheat level

extern int  mouse_down;                                     // used to hold the last reading of 'gui_mouse_b()' status
extern int idle_count;
extern char *qstpath;
extern char *qstdir;
extern gamedata *saves;
extern gamedata game;

extern volatile int lastfps;
extern volatile int framecnt;
extern volatile int myvsync;

// quest file data
extern zquestheader QHeader;
extern byte         quest_rules[QUESTRULES_SIZE];
extern byte         midi_flags[MIDIFLAGS_SIZE];
extern word         map_count;
extern MsgStr       *MsgStrings;
extern DoorComboSet *DoorComboSets;
extern dmap         *DMaps;
extern miscQdata    QMisc;
extern mapscr       *TheMaps;

/**********************************/
/*********** Misc Data ************/
/**********************************/

extern const char startguy[8];
extern const char gambledat[12*6];
extern const byte stx[4][9];
extern const byte sty[4][9];
extern const byte ten_rupies_x[10];
extern const byte ten_rupies_y[10];
extern music tunes[MAXMUSIC];
#endif

/*** end of zelda.h ***/
