/*
  zelda.h
  Jeremy Craner, 1999
  Definitions, function prototypes, etc. for zelda.cc
*/


#ifndef _ZELDA_H_
#define _ZELDA_H_


/*********************************/
/********** Definitions **********/
/*********************************/

#include "zeldadat.h"
#include "zdefs.h"
#include "colors.cc"


// system colors
#define vc(x)         ((x)+224)

// saved games
#define MAXSAVES      24

// map flags
#define mfITEM        16        // item (main screen)
#define mfSPITEM      32        // special item (underground)
#define mfNEVERRET    64        // enemy never returns
#define mfTMPNORET    128       // enemies don't return until you leave the dungeon


/*********************************/
/******** Enums & Structs ********/
/*********************************/

enum { qQUIT=1, qRESET, qEXIT, qGAMEOVER, qCONT, qWON, qERROR };

enum actiontype { none, walking, attacking, freeze, holding1, holding2,
                  rafting, gothit, inwind, scrolling, won };

// "special" walk flags
enum { spw_none, spw_door, spw_clipright, spw_floater, spw_trap, spw_halfstep,
       spw_water };

// death & fade defs
enum { fade_none, fade_flicker, fade_invisible, fade_flash_die,
       fade_blue_poof };

enum { isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100 };


typedef struct gamedata {
  char  name[9];
  byte  quest,lvlitems[16];
  short life,maxlife,drupy,rupies,deaths;
  byte  sword,misc,brang,bombs,arrow,candle,whistle,bait,letter,potion,wand,
        ring,keys,maxbombs,wlevel;
  byte  extra[2];
  char  version[9];
  char  title[65];
  byte  hasplayed;
  byte  expansion[128];
  // 256 bytes so far

  byte  bmaps[MAXDMAPS*64]; // the dungeon progress maps
  byte  maps[MAXMAPS*128];  // info on map changes, items taken, etc.
  byte  guys[MAXMAPS*128];  // guy counts (though dungeon guys are reset on entry)
  char  qstpath[260];
} gamedata;




/*********************************/
/*********** Procedures **********/
/*********************************/

// link.cc
int  Bweapon(int pos);
void selectBwpn(int step);
void getitem(int id);

// aglogo.cc
int aglogo(BITMAP* frame);

// pal.cc
RGB  _RGB(byte *si);
RGB  _RGB(int r,int g,int b);
RGB  invRGB(RGB s);
RGB  mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio);
RGB  NESpal(int i);
void copy_pal(RGB *src,RGB *dest);
void loadtitlepal(int clear,byte *dataofs,int shift);
void loadfullpal();
void loadlvlpal(int level);
void ringcolor();
void loadpalset(int cset,int dataset);
void loadfadepal(int dataset);
void fade(int level,bool blackall,bool fromblack);
void lighting(int funct);
void dryuplake();
void rehydratelake();

// tiles.cc
void puttile8(BITMAP* dest,int tile,int x,int y,byte cset,int flip);
void overtile8(BITMAP* dest,int tile,int x,int y,byte cset,int flip);
void puttile16(BITMAP* dest,int tile,int x,int y,byte cset,int flip);
void overtile16(BITMAP* dest,int tile,int x,int y,byte cset,int flip);
void putblock(BITMAP* dest,int tile,int x,int y,int cset,int flip,int mask);
void overblock(BITMAP* dest,int tile,int x,int y,int cset,int flip,int mask);
void mirrorblock(BITMAP* dest,int tile,int x,int y,int cset,bool odd);
void mirrorblock_v(BITMAP* dest,int tile,int x,int y,int cset,bool odd);
void putcombo(BITMAP* dest,int x,int y,word cmbdat);
void overcombo(BITMAP* dest,int x,int y,word cmbdat);

// maps.cc
int  dungeon();
word cmb(int combo,int cset,int flags);
word tcmb(int pos);
int  MAPDATA(int x,int y);
#define  MAPCOMBO(x,y)  (MAPDATA((x),(y))&0x1FF)
int  MAPFLAG(int x,int y);
int  WARPCODE(int dmap,int scr,int dw);
bool iswater(int combo);
bool hiddenstair(int tmp,bool redraw);
void hidden_entrance(int tmp,bool refresh);
void findentrance(int x,int y,int flag);
void doburn(int x,int y);
void dobomb(int x,int y);
void bombdoor(int x,int y);
void putdoor(int t,int side,int door,bool redraw=true);
void openshutters();
void loadscr(int tmp,int scr,int ldir);
void putscr(BITMAP* dest,int x,int y,mapscr* screen);
bool _walkflag(int x,int y,int cnt);
bool hit_walkflag(int x,int y,int cnt);
void setmapflag();
bool getmapflag();
void setmapflag(int flag);
bool getmapflag(int flag);
void map_bkgsfx();

// subscr.cc
void blueframe(BITMAP* dest,int x,int y,int xsize,int ysize);
void putsubscr(BITMAP* dest,int x,int y);
void lifemeter(BITMAP* dest,int x,int y);
void puttriframe(BITMAP* dest,int y);
void putxnum(BITMAP *dest,int x,int y,int num);
void dosubscr();
void markBmap(int dir);
void putBmap(BITMAP *dest);

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
void check_collisions();
void getBigTri();


// zelda.cc
int  init_game();
int  cont_game();
void restart_level();
bool game_vid_mode(int mode,int wait);
void init_NES_mode();
void advanceframe();
void updatescr();
void syskeys();
void System();
void system_pal();
void game_pal();
int onSave();
int onQuit();
int onReset();
int onExit();
int PopUp_dialog(DIALOG *d,int f);

int  next_press_key();
int  next_press_btn();
bool ReadKey(int k);
bool Up();
bool Down();
bool Left();
bool Right();
bool cAbtn();
bool cBbtn();
bool cSbtn();
bool rUp();
bool rDown();
bool rLeft();
bool rRight();
bool rAbtn();
bool rBbtn();
bool rSbtn();

void blackscr(int fcnt,bool showsubscr);
void openscreen();
void ALLOFF();
fix  LinkX();
fix  LinkY();
int  LinkDir();
void setClock(bool state);
void CatchBrang();
int  TriforceCount();

void jukebox(int index);
void jukebox(int index,int loop);
void play_DmapMidi();
void master_volume(int dv,int mv);
int  sfx_count();
void sfx_cleanup();
bool sfx_init(int index);
void sfx(int index,int pan,bool loop);
void cont_sfx(int index);
void stop_sfx(int index);
void adjust_sfx(int index,int pan,bool loop);
void pause_sfx(int index);
void resume_sfx(int index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int index);
void kill_sfx();
int  pan(int x);


inline void sfx(int index)         { sfx(index,128,false); }
inline void sfx(int index,int pan) { sfx(index,pan,false); }

inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }



/**********************************/
/******** Global Variables ********/
/**********************************/

extern BITMAP   *framebuf, *scrollbuf, *tmp_bmp;
extern DATAFILE *data,*sfxdata,*mididata;
extern FONT     *zfont;
extern PALETTE   RAMpal;
extern byte     *tilebuf,*colordata;
extern combo    *combobuf,*spritebuf;
extern int homescr,currscr,frame,currmap,dlevel,worldscr,newscr_clk,warpscr;
extern int opendoors,currdmap,fadeclk,hasitem,lastentrance,whistleclk;
extern word msgpos,msgclk,msgstr,Bwpn,Bpos;
extern bool Vsync, Paused, Advance, ShowFPS, Showpal, refreshpal, wand_dead,
            debug,loaded_guys,loaded_enemies,drawguys,details,SystemKeys,
            Playing,freeze_guys,watch,darkroom;
extern mapscr tmpscr[2];
extern gamedata game;
extern int digi_volume,midi_volume,currmidi,wand_x,wand_y,Quit;
extern int Akey,Bkey,Skey,Abtn,Bbtn,Sbtn;
extern bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,blockpath,debug,toogam;
extern int prices[3],loadside;
extern short visited[6];
extern byte guygrid[176];
extern int sle_x,sle_y,sle_cnt,sle_clk;


// quest file data
extern zquestheader QHeader;
extern MsgStr       *MsgStrings;
extern dmap         *DMaps;
extern miscQdata    QMisc;
extern mapscr       *TheMaps;



/**********************************/
/*********** Misc Data ************/
/**********************************/

const char startguy[8] = {-13,-13,-13,-14,-15,-18,-21,-40};
const char gambledat[12*6] ={ 20,-10,-10, 20,-10,-10, 20,-40,-10, 20,-10,-40,
	                      50,-10,-10, 50,-10,-10, 50,-40,-10, 50,-10,-40,
                              -10,20,-10, -10,20,-10, -40,20,-10, -10,20,-40,
                              -10,50,-10, -10,50,-10, -40,50,-10, -10,50,-40,
                              -10,-10,20, -10,-10,20, -10,-40,20, -40,-10,20,
                              -10,-10,50, -10,-10,50, -10,-40,50, -40,-10,50 };
const byte stx0[9] = { 48, 80, 80, 96,112,144,160,160,192};
const byte stx1[9] = { 48, 80, 80, 96,128,144,160,160,192};
const byte stx2[9] = { 80, 80,128,128,160,160,192,192,208};
const byte stx3[9] = { 32, 48, 48, 80, 80,112,112,160,160};
const byte sty0[9] = {112, 64,128, 96, 80, 96, 64,128,112};
const byte sty1[9] = { 48, 32, 96, 64, 80, 64, 32, 96, 48};
const byte sty2[9] = { 32,128, 64, 96, 64, 96, 48,112, 80};
const byte sty3[9] = { 80, 48,112, 64, 96, 64, 96, 32,128};

const byte ten_rupies_x[10] = {120,112,128,96,112,128,144,112,128,120};
const byte ten_rupies_y[10] = {49,65,65,81,81,81,81,97,97,113};



#define MAXMUSIC MUSIC_COUNT+MAXMIDIS

music tunes[MAXMUSIC] = {
 // (title)             (s) (ls) (le) (l) (vol) (midi)
 { "Z1 Dungeon",         0,  -1,  -1,  1,  168,  NULL },
 { "Z1 Ending",          0,  129, 225, 1,  132,  NULL },
 { "Z1 Game Over",       0,  -1,  -1,  1,  176,  NULL },
 { "Z1 Level 9",         0,  -1,  -1,  1,  255,  NULL },
 { "Z1 Overworld",       0,  17,  -1,  1,  200,  NULL },
 { "Z1 Title",           0,  -1,  -1,  0,  160,  NULL },
 { "Z1 Triforce",        0,  -1,  -1,  0,  160,  NULL },
};


#endif

/*** end of zelda.h ***/
