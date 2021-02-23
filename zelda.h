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
#define  MAXMUSIC     MUSIC_COUNT+MAXMIDIS


extern "C" {
char * getcwd(char *_buf, size_t _size);
}


#define MAX_IDLE      72000     // 20 minutes

// system colors
#define vc(x)         ((x)+224)
#define BLACK         253
#define WHITE         254

// saved games
//#define MAXSAVES      24
#define MAXSAVES      15

// game.maps[] flags
#define mSECRET       1         // only overworld and caves use this
#define mVISITED      2         // only overworld uses this

#define mDOOR_UP      1         // only dungeons use this
#define mDOOR_DOWN    2         //        ''
#define mDOOR_LEFT    4         //        ''
#define mDOOR_RIGHT   8         //        ''

#define mITEM         16        // item (main screen)
#define mBELOW        32        // special item (underground)
#define mNEVERRET     64        // enemy never returns
#define mTMPNORET     128       // enemies don't return until you leave the dungeon


/*********************************/
/******** Enums & Structs ********/
/*********************************/

enum { qQUIT=1, qRESET, qEXIT, qGAMEOVER, qCONT, qWON, qERROR };

enum actiontype { none, walking, attacking, freeze, holding1, holding2,
                  rafting, gothit, inwind, scrolling, won, swimming, hopping,
                  swimhit, swimhold1, swimhold2, casting };
//magic types
enum  { mgc_none, mgc_dinsfire };

// "special" walk flags
enum { spw_none, spw_door, spw_clipright, spw_floater, spw_trap, spw_halfstep,
       spw_water, spw_wizzrobe, spw_clipbottomright };

// death & fade defs
enum { fade_none, fade_flicker, fade_invisible, fade_flash_die,
       fade_blue_poof };



/*********************************/
/*********** Procedures **********/
/*********************************/

// link.cc
int  Bweapon(int pos);
void selectBwpn(int step);
void selectAwpn(int step);
void getitem(int id);

// aglogo.cc
int  aglogo(BITMAP* frame);

// maps.cc
void clear_dmap(byte i);
void clear_dmaps();
int  isdungeon();
int  cmb(int combo,int cset);
int  tcmb(int pos);
int  tcmbdat(int pos);
int  tcmbflag(int pos);
int  MAPDATA(int x,int y);
int  MAPCOMBO(int x,int y);
int  COMBOTYPE(int x,int y);
int  MAPFLAG(int x,int y);
//int  FLAG(int combo);
int  WARPCODE(int dmap,int scr,int dw);
bool iswater(int combo);
bool iswater_type(int type);
bool isstepable(int combo);
bool hiddenstair(int tmp,bool redraw);
void hidden_entrance(int tmp,bool refresh,bool high16only=false);
bool findentrance(int x,int y,int flag,bool setmap);
bool hitcombo(int x, int y, int combotype);
bool hitflag(int x, int y, int flagtype);
int  nextscr(int dir);
void bombdoor(int x,int y);
void putdoor(int t,int side,int door,bool redraw=true);
void openshutters();
void loadscr(int tmp,int scr,int ldir);
void putscr(BITMAP* dest,int x,int y,mapscr* screen);
bool _walkflag(int x,int y,int cnt);
bool hit_walkflag(int x,int y,int cnt);
void setmapflag();
void unsetmapflag();
bool getmapflag();
void setmapflag(int flag);
void unsetmapflag(int flag);
bool getmapflag(int flag);
void map_bkgsfx();
int  onViewMap();
//void do_overhead(int type);
void do_layer2(int type, mapscr* overscreen, int x, int y);

// subscr.cc
void blueframe(BITMAP* dest,int x,int y,int xsize,int ysize);
void putsubscr(BITMAP* dest,int x,int y,bool showtime=false);
void lifemeter(BITMAP* dest,int x,int y);
void puttriframe(BITMAP* dest,int y);
void putxnum(BITMAP *dest,int x,int y,int num);
void dosubscr();
void markBmap(int dir);
void putBmap(BITMAP *dest);
void reset_subscr_items();

// guys.cc
bool m_walkflag(int x,int y,int special);
void addEwpn(int x,int y,int id,int type,int power,int dir);
int  enemy_dp(int index);
int  ewpn_dp(int index);
int  hit_enemy(int index,int wpnId,int power,int wpnx,int wpny,int dir);
void enemy_scored(int index);
void addguy(int x,int y,int id,int clk);
void additem(int x,int y,int id,int pickup);
void addenemy(int x,int y,int id,int clk);
void kill_em_all();
int  GuyHit(int tx,int ty,int txsz,int tysz);
int  GuyHit(int index,int tx,int ty,int txsz,int tysz);
bool hasMainGuy();
void EatLink(int index);
void GrabLink(int index);
bool CarryLink();
void movefairy(fix &x,fix &y,int misc);
void killfairy(int misc);
void addfires();
void loadguys();
void loadenemies();
void setupscreen();
void putmsg();
void putprices(bool sign);
void domoney();
void domagic();
void check_collisions();
void getBigTri();
bool isflier(int id);

// sprite.cc
void putitem(BITMAP* dest,int x,int y,int item_id);


// title.cc
void load_game_icon(gamedata *g);
void update_game_icons();


// zc_sys.cc
void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int from,int to);
void go();
void comeback();
void waitvsync(bool fast);
int  input_idle(bool checkmouse);
int  after_time();


// zelda.cc
void ALLOFF();
fix  LinkX();
fix  LinkY();
int  LinkDir();
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

inline void addLwpn(int x,int y,int id,int type,int power,int dir);

inline void sfx(int index)         { sfx(index,128,false); }
inline void sfx(int index,int pan) { sfx(index,pan,false); }

inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }



/**********************************/
/******** Global Variables ********/
/**********************************/

extern BITMAP   *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *fps_undo, *msgdisplaybuf;
extern BITMAP   *mappic;
extern DATAFILE *data, *sfxdata, *mididata;
extern SAMPLE   wav_refill;
extern FONT     *zfont, *deffont, *mfont;
extern PALETTE  RAMpal;
extern byte     *tilebuf, *colordata;
extern newcombo *combobuf;
extern word animated_combo_table[MAXCOMBOS][2]; //[0]=original tile, [1]=clock
extern word     msgclk, msgstr, msgpos;
extern short    Bpos, lensclk, lenscnt;
extern byte screengrid[22];

extern int homescr,currscr,frame,currmap,dlevel,warpscr,worldscr;
extern int newscr_clk,opendoors,currdmap,fadeclk,currgame,listpos;
extern int lastentrance,prices[3],loadside, Bwpn, Awpn;
extern int digi_volume,midi_volume,currmidi,wand_x,wand_y,hasitem,whistleclk,pan_style;
extern int Akey,Bkey,Skey,Lkey,Rkey,Pkey,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Quit;
extern int DUkey, DDkey, DLkey, DRkey, ss_after, ss_speed, ss_density;
extern int arrow_x, arrow_y, brang_x, brang_y, chainlink_x, chainlink_y;
extern int chainlinks_count, hs_startx, hs_starty, clockclk, clock_zoras;
extern int swordhearts[4], currcset, gfc, gfc2, pitx, pity, refill_what;
extern int heart_beep_timer, new_enemy_tile_start, nets, magictype;
extern int magiccastclk, castx, casty, df_x, df_y, magicdrainclk, conveyclk;

extern bool Vsync, Paused, Advance, ShowFPS, Showpal, Playing;
extern bool refreshpal,blockpath,wand_dead,debug,loaded_guys,freeze_guys;
extern bool loaded_enemies,drawguys,details,DXtitle,debug_enabled,watch;
extern bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown;
extern bool SystemKeys,NESquit,useCD,boughtsomething, add_asparkle;
extern bool add_bsparkle, fixed_door, darkroom,BSZ,COOLSCROLL,NEWSUBSCR;
extern bool hookshot_used, hookshot_frozen, pull_link, add_chainlink;
extern bool del_chainlink, hs_fix, cheat_superman, gofast;
extern bool ewind_restart, didpit, heart_beep, pausenow, castnext;
extern bool add_df1asparkle, add_df1bsparkle, is_on_conveyor, was_on_conveyor;

extern short  visited[6];
extern byte   guygrid[176];
extern mapscr tmpscr[2];
extern char   sig_str[44];

extern int  VidMode,resx,resy,scrx,scry;
extern bool sbig; // big screen
extern bool toogam;

extern int  mouse_down;  // used to hold the last reading of 'mouse_b' status
extern char *qstpath;
extern gamedata *saves;
extern gamedata game;

extern volatile int lastfps;
extern volatile int framecnt;
extern volatile int myvsync;


// quest file data
extern zquestheader QHeader;
extern MsgStr       *MsgStrings;
extern dmap         *DMaps;
extern miscQdata    QMisc;
extern mapscr       *TheMaps;



/**********************************/
/*********** Misc Data ************/
/**********************************/

extern const char startguy[8];
extern const char gambledat[12*6];
extern const byte stx0[9];
extern const byte stx1[9];
extern const byte stx2[9];
extern const byte stx3[9];
extern const byte sty0[9];
extern const byte sty1[9];
extern const byte sty2[9];
extern const byte sty3[9];
extern const byte ten_rupies_x[10];
extern const byte ten_rupies_y[10];
extern music tunes[MAXMUSIC];


#endif

/*** end of zelda.h ***/
