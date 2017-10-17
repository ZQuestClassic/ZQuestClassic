//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_misc.cc
//
//  Misc. stuff for ZQuest.
//
//--------------------------------------------------------

//inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
//inline void SCRFIX() {}

#ifndef _ZQ_MISC_H_
#define _ZQ_MISC_H_

#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"

#define ITEMCNT   iMax
#define WPNCNT    wMAX
#define MAXROOMTYPES   rMAX
#define MAXWARPTYPES   wtMAX-1                              // minus whistle
#define MAXWARPEFFECTS   weMAX
#define MAXCOMBOTYPES  cMAX
#define MAXFLAGS    mfMAX
#define zqMAXGUYS    9
#define MAXPATTERNS  2
#define MAXMIDIS_ZQ  4+MAXMIDIS

#define cWALK      1
#define cFLAGS     2
#define cDARK      4
#define cCSET      8
#define cNODARK    16
#define cNOITEM    32
#define cCTYPE     64
#define cDEBUG     128

#define rMAP       1
#define rCOMBOS    2
#define rSCRMAP    4
#define rMENU      8
#define rCOMBO     16
#define rALL       0x0FF
#define rCLEAR     0x100

#define C(x)   ((x)-'a'+1)

inline int popup_menu(MENU *menu,int x,int y)
{
  while(gui_mouse_b());
  return jwin_do_menu(menu,x,y);
}

inline int bit(int val,int b)
{
  return (val>>b)&1;
}

enum { ftBIN=1, ftBMP, ftTIL, ftZGP, ftQSU };
extern char *imgstr[6];
extern char *item_string[ITEMCNT];
extern char *weapon_string[WPNCNT];
extern char *roomtype_string[MAXROOMTYPES];
extern char *catchall_string[MAXROOMTYPES];
extern char *warptype_string[MAXWARPTYPES];
extern char *warpeffect_string[MAXWARPEFFECTS];
extern char *combotype_string[MAXCOMBOTYPES];
extern char *flag_string[MAXFLAGS];
extern char *guy_string[eMAXGUYS];
extern char *pattern_string[MAXPATTERNS];
extern char *short_pattern_string[MAXPATTERNS];
extern char *midi_string[MAXMIDIS_ZQ];
extern int gocnt;

int filetype(char *path);
void load_mice();
void load_icons();
void load_selections();
void load_arrows();
void dump_pal();

int bound(int &x,int low,int high);
int wrap(int x,int low,int high);
bool readfile(char *path,void *buf,int count);
bool writefile(char *path,void *buf,int count);

/***  from allegro's guiproc.c  ***/
void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg);

RGB _RGB(byte *si);
RGB _RGB(int r,int g,int b);
RGB invRGB(RGB s);
RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio);

void reset_pal_cycling();
void cycle_palette();
void load_cset(RGB *pal,int cset_index,int dataset);
void set_pal();
void loadlvlpal(int level);
void loadfadepal(int dataset);

void setup_lcolors();
void refresh_pal();

void refresh(int flags);
void domouse();
void init_doorcombosets();

int onNew();
int onOpen();
int onOpen2();
int onRevert();
int onSave();
int onSaveAs();
int onQuestTemplates();

int onUndo();
int onCopy();
int onPaste();
int onPasteAll();
int onPasteToAll();
int onPasteAllToAll();
int onDelete();
int onDeleteMap();

int onTemplate();
int onDoors();
int onCSetFix();
int onFlags();
int onShowPal();
int onReTemplate();

int playTune();
int playMIDI();
int stopMIDI();
int onKeyFile();

int onUp();
int onDown();
int onLeft();
int onRight();
int onPgUp();
int onPgDn();
int onPlus();
int onMinus();

int  onHelp();
void doHelp(int bg,int fg);

int onScrData();
int onGuy();
int onEndString();
int onString();
int onRType();
int onCatchall();
int onItem();
int onWarp();
int onWarp2();
int onPath();
int onEnemies();
int onEnemyFlags();
int onUnderCombo();
int onSecretCombo();

int onHeader();
int onRules();
int onRules2();
int onCheats();
int onStrings();
int onDmaps();
int onTiles();
int onCombos();
int onMidis();
int onShopTypes();
int onInfoTypes();
int onWarpRings();
int onWhistle();
int onMiscColors();
int onMapStyles();
int onTemplates();
int onDoorCombos();
int onTriPieces();
int onIcons();
int onInit();
int onLayers();
int onScreenPalette();
int xtoi(char *hexstr);

int onColors_Main();
int onColors_Levels();
int onColors_Sprites();

int onImport_Map();
int onImport_DMaps();
int onImport_Msgs();
int onImport_Combos();
int onImport_Tiles();
int onImport_Pals();
int onImport_ZGP();
int onImport_ZQT();
int onImport_UnencodedQuest();

int onExport_Map();
int onExport_DMaps();
int onExport_Msgs();
int onExport_Combos();
int onExport_Tiles();
int onExport_Pals();
int onExport_ZGP();
int onExport_ZQT();
int onExport_UnencodedQuest();

int onGotoMap();
int onMapCount();

int onViewPic();
int onViewMap();
int onComboPage();

int onDefault_Pals();
int onDefault_Tiles();
int onDefault_Combos();
int onDefault_Sprites();
int onDefault_MapStyles();

int onCustomItems();
int onCustomWpns();
int onCustomLink();
int onCustomGuys();

int onTest();
int onTestOptions();

int onOptions();

void edit_combo(int c,bool freshen,int cs);

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg,bool value);
void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int value);
void KeyFileName(char *kfname);

int onSpacebar();
int onSnapshot();
void go();
void comeback();
int checksave();
int onExit();
int onAbout();
int onW();
int onF();
int onQ();
int onL();
int onM();
void setFlagColor();
int onTimes();
int onDivide();
int on0();
int on1();
int on2();
int on3();
int on4();
int on5();
int on6();
int on7();
int on8();
int on9();
int on10();
int on11();
int on12();
int on13();
int on14();
int onD();
int onIncMap();
int onDecMap();
int onDumpScr();
#endif
