//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_misc.cc
//
//  Misc. stuff for ZQuest.
//
//--------------------------------------------------------

//INLINE void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
//INLINE void SCRFIX() {}

#ifndef _ZQ_MISC_H_
#define _ZQ_MISC_H_

#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "sfx.h"

#define MAXSCREENS 128
#define MAXROOMTYPES   rMAX
#define MAXWARPTYPES   wtMAX-1                              // minus whistle
#define MAXWARPEFFECTS   weMAX
#define MAXCOMBOTYPES  cMAX
#define MAXFLAGS    mfMAX
#define MAXPATTERNS  6
#define MAXCUSTOMMIDIS_ZQ  (4+MAXCUSTOMMIDIS)

#define cWALK            1
#define cFLAGS           2
#define cDARK            4
#define cCSET            8
#define cNODARK         16
#define cNOITEM         32
#define cCTYPE          64
#define cDEBUG         128

#define rMAP             1
#define rCOMBOS          2
#define rSCRMAP          4
#define rMENU            8
#define rCOMBO          16
#define rFAVORITES      32
#define rCOMMANDS       64
#define rOTHER         128
#define rALL        0x0FFF
#define rCLEAR      0x1000
#define rNOCURSOR   0x2000

#define C(x)   ((x)-'a'+1)

INLINE int popup_menu(MENU *menu,int x,int y)
{
    while(gui_mouse_b())
    {
        /* do nothing */
    }
    
    return jwin_do_menu(menu,x,y);
}

INLINE int bit(int val,int b)
{
    return (val>>b)&1;
}

enum { ftBIN=1, ftBMP, ftTIL, ftZGP, ftQSU, ftZQT, ftQST, ftMAX };
extern const char *imgstr[ftMAX];
extern char ns_string[4];
extern const char *old_item_string[iLast];
extern const char *old_weapon_string[wLast];
extern const char *old_sfx_string[Z35];
extern char *item_string[ITEMCNT];
extern char *weapon_string[WPNCNT];
extern char *sfx_string[WAV_COUNT];
extern const char *roomtype_string[MAXROOMTYPES];
extern const char *catchall_string[MAXROOMTYPES];
extern const char *warptype_string[MAXWARPTYPES];
extern const char *warpeffect_string[MAXWARPEFFECTS];
//extern const char *combotype_string[MAXCOMBOTYPES];
extern const char *flag_string[MAXFLAGS];
extern const char *flag_help_string[(mfMAX)*3];
extern const char *itemclass_help_string[(itype_last-20)*3];
extern const char *combotype_help_string[cMAX*3];
extern const char	*old_guy_string[OLDMAXGUYS];
extern char *guy_string[eMAXGUYS];
extern const char *pattern_string[MAXPATTERNS];
extern const char *short_pattern_string[MAXPATTERNS];
extern const char *walkmisc1_string[e1tLAST];
extern const char *walkmisc2_string[e2tTRIBBLE+1];
extern const char *walkmisc7_string[e7tEATHURT+1];
extern const char *walkmisc9_string[e9tARMOS+1];
extern const char *midi_string[MAXCUSTOMMIDIS_ZQ];
extern const char *screen_midi_string[MAXCUSTOMMIDIS_ZQ+1];
extern const char *enetype_string[eeMAX];
extern const char *eneanim_string[aMAX];
//extern char *itemset_string[isMAX];
extern const char *eweapon_string[wMax-wEnemyWeapons];
extern int gocnt;

int filetype(const char *path);
void load_mice();
void load_icons();
void load_panel_button_icons();
void load_selections();
void load_arrows();
void dump_pal();

int wrap(int x,int low,int high);
bool readfile(const char *path,void *buf,int count);
bool writefile(const char *path,void *buf,int count);

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

int onPasteUnderCombo();
int onPasteSecretCombos();
int onPasteFFCombos();
int onPasteWarps();
int onPasteScreenData();
int onPasteWarpLocations();
int onPasteDoors();
int onPasteLayers();
int onPasteRoom();
int onPasteGuy();
int onPasteEnemies();

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
int onIncreaseCSet();
int onDecreaseCSet();

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
int onAnimationRules();
int onComboRules();
int onItemRules();
int onEnemyRules();
int onFixesRules();
int onMiscRules();
int onCompatRules();
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

int onImport_DMaps_old();
int onExport_DMaps_old();

int onImport_Msgs();
int onImport_Combos();
int onImport_Combos_old();
int onImport_Tiles();

int onImport_Tiles_old();
int onExport_Tiles_old();

int onImport_Guys();
int onImport_Subscreen();
int onImport_Pals();
int onImport_ZGP();
int onImport_ZQT();
int onImport_UnencodedQuest();

int onExport_Map();
int onExport_DMaps();
int onExport_Msgs();
int onExport_MsgsText();
int onExport_Combos();
int onExport_Combos_old();
int onExport_Tiles();
int onExport_Guys();
int onExport_Subscreen();
int onExport_Pals();
int onExport_ZGP();
int onExport_ZQT();
int onExport_UnencodedQuest();
int onExport_Tilepack();
int onImport_Tilepack();
int onImport_Tilepack_To();

int onExport_Combopack();
int onImport_Combopack_To();
int onImport_Combopack();

int onImport_Comboaliaspack();
int onImport_Comboaliaspack_To();
int onExport_Comboaliaspack();

int writezdoorsets(PACKFILE *f, int first, int count);
int readzdoorsets(PACKFILE *f, int first, int count);
int onExport_Doorset();
int onImport_Doorset();

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

bool edit_combo(int c,bool freshen,int cs);

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg,bool value);
void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int value);
void KeyFileName(char *kfname);

int onSpacebar();
int onSnapshot();
int onMapscrSnapshot();
int onSaveZQuestSettings();
int onClearQuestFilepath();
void go();
void comeback();
int checksave();
int onExit();
int onAbout();
int onShowCSet();
int onShowCType();
int onShowWalkability();
int onPreviewMode();
int onShowFlags();
int onP();
int onShowComboInfoCSet();
int onShowDarkness();
int onJ();
int onM();
void setFlagColor();
int onIncreaseFlag();
int onDecreaseFlag();
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
int onToggleDarkness();
int onIncMap();
int onDecMap();
int onDumpScr();
#endif

