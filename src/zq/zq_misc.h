#ifndef ZQ_MISC_H_
#define ZQ_MISC_H_

#include "base/mapscr.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/fonts.h"
#include "gui/jwin.h"
#include "sfx.h"
#include "zq/render_tooltip.h"

#define MAXSCREENS 128
#define MAXROOMTYPES   rMAX
#define MAXWARPTYPES   wtMAX-1                              // minus whistle
#define MAXWARPEFFECTS   weMAX
#define MAXCOMBOTYPES  cMAX
#define MAXFLAGS    mfMAX
#define MAXPATTERNS  7

#define cWALK         0x001
#define cFLAGS        0x002
#define cDARK         0x004
#define cCSET         0x008
#define cNODARK       0x010
#define cNOITEM       0x020
#define cCTYPE        0x040
#define cDEBUG        0x080
#define cNEWDARK      0x100

#define rMAP        0x0001
#define rCOMBOS     0x0002
#define rSCRMAP     0x0004
#define rMENU       0x0008
#define rCOMBO      0x0010
#define rFAVORITES  0x0020
#define rCOMMANDS   0x0040
#define rOTHER      0x0080
#define rALL        0x1FFF
#define rCLEAR      0x1000
#define rNOCURSOR   0x2000

#define C(x)   ((x)-'a'+1)

INLINE int32_t bit(int32_t val,int32_t b)
{
    return (val>>b)&1;
}

enum { ftBIN=1, ftBMP, ftTIL, ftZGP, ftQSU, ftZQT, ftQST, ftMAX };
extern const char *imgstr[ftMAX];
extern char ns_string[4];
extern const char *old_item_string[iLast];
extern const char *old_weapon_string[wLast];
extern const char *old_sfx_string[Z35];
extern char *item_string[MAXITEMS];
extern char *weapon_string[MAXWPNS];
extern char *sfx_string[WAV_COUNT];
extern const char *warptype_string[MAXWARPTYPES];
extern const char *warpeffect_string[MAXWARPEFFECTS];
extern const char	*old_guy_string[OLDMAXGUYS];
extern char *guy_string[eMAXGUYS];
extern const char *pattern_string[MAXPATTERNS];
extern const char *short_pattern_string[MAXPATTERNS];
extern const char *walkmisc1_string[e1tLAST];
extern const char *walkmisc2_string[e2tTRIBBLE+1];
extern const char *walkmisc7_string[e7tEATHURT+1];
extern const char *walkmisc9_string[e9tARMOS+1];
extern const char *patramisc28_string[patratLAST];
extern const char *midi_string[MAXCUSTOMMIDIS_ZQ];
extern const char *screen_midi_string[MAXCUSTOMMIDIS_ZQ+1];
extern const char *enetype_string[eeMAX];
extern const char *eneanim_string[aMAX];
extern const char *eweapon_string[wMax-wEnemyWeapons];
extern int32_t gocnt;

int32_t filetype(const char *path);
void load_mice();
void load_icons();
void load_panel_button_icons();
void load_selections();
void load_arrows();
void dump_pal();

bool readfile(const char *path,void *buf,int32_t count);
bool writefile(const char *path,void *buf,int32_t count);

/***  from allegro's guiproc.c  ***/
void dotted_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t fg, int32_t bg);

RGB _RGB(byte *si);
RGB _RGB(int32_t r,int32_t g,int32_t b);
RGB invRGB(RGB s);
RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio);

void reset_pal_cycling();
void cycle_palette();
void load_cset(RGB *pal,int32_t cset_index,int32_t dataset);
void set_pal();
void loadlvlpal(int32_t level);
void loadfadepal(int32_t dataset);

ALLEGRO_COLOR real_lc1(int pal);
ALLEGRO_COLOR real_lc2(int pal);
void refresh_pal();

void domouse();
void init_doorcombosets();

int32_t onNew();
int32_t onOpen();
int32_t onOpen2();
int32_t onRevert();
int32_t onSave();
int32_t onSaveAs();

int32_t onUndo();
int32_t onCopy();
int32_t onPaste();
int32_t onPasteAll();
int32_t onPasteToAll();
int32_t onPasteAllToAll();
int32_t onDelete();
int32_t onDeleteMap();

int32_t onPasteUnderCombo();
int32_t onPasteSecretCombos();
int32_t onPasteFFCombos();
int32_t onPasteWarps();
int32_t onPasteScreenData();
int32_t onPasteWarpLocations();
int32_t onPasteDoors();
int32_t onPasteLayers();
int32_t onPasteRoom();
int32_t onPasteGuy();
int32_t onPasteEnemies();

int32_t onTemplate();
int32_t onDoors();
int32_t onCSetFix();
int32_t onFlags();
int32_t onShowPal();
int32_t onReTemplate();

int32_t playTune();
int32_t playMIDI();
int32_t stopMIDI();
int32_t onKeyFile();

int32_t onPgUp();
int32_t onPgDn();
int32_t onIncreaseCSet();
int32_t onDecreaseCSet();

int32_t  onHelp();
void doHelp();

int32_t onScrData();
int32_t onGuy();
int32_t onEndString();
int32_t onString();
int32_t onRType();
int32_t onCatchall();
int32_t onItem();
int32_t onWarp();
int32_t onWarp2();
int32_t onPath();
int32_t onEnemies();
int32_t onEnemyFlags();
int32_t onUnderCombo();
int32_t onSecretCombo();

int32_t onHeader();
int32_t onAnimationRules();
int32_t onWeaponRules();
int32_t onHeroRules();
int32_t onComboRules();
int32_t onItemRules();
int32_t onEnemyRules();
int32_t onFixesRules();
int32_t onMiscRules();
int32_t onCompatRules();
int32_t onRules2();
int32_t onCheats();
int32_t onStrings();
int32_t onDmaps();
int32_t onMaps();
int32_t onTiles();
int32_t onCombos();
int32_t onMidis();
int32_t onShopTypes();
int32_t onInfoTypes();
int32_t onWarpRings();
int32_t onWhistle();
int32_t onMiscColors();
int32_t onMapStyles();
int32_t onDoorCombos();
int32_t onTriPieces();
int32_t onIcons();
int32_t onInit();
int32_t onLayers();
int32_t onScreenPalette();

int32_t onColors_Main();
int32_t onColors_Levels();
int32_t onColors_Sprites();

int32_t onImport_Map();
int32_t onImport_DMaps();
int32_t onImport_Msgs();
int32_t onImport_Combos();
int32_t onImport_Tiles();
int32_t onImport_Guys();
int32_t onImport_Pals();
int32_t onImport_ZGP();

int32_t onExport_Map();
int32_t onExport_DMaps();
int32_t onExport_Msgs();
int32_t onExport_MsgsText();
int32_t onExport_Combos();
int32_t onExport_Tiles();
int32_t onExport_Guys();
int32_t onExport_Pals();
int32_t onExport_ZGP();
int32_t onExport_Tilepack();
int32_t onImport_Tilepack();
int32_t onImport_Tilepack_To();

int32_t onExport_Combopack();
int32_t onImport_Combopack_To();
int32_t onImport_Combopack();

int32_t onImport_Comboaliaspack();
int32_t onImport_Comboaliaspack_To();
int32_t onExport_Comboaliaspack();

int32_t writezdoorsets(PACKFILE *f, int32_t first, int32_t count);
int32_t readzdoorsets(PACKFILE *f, int32_t first, int32_t count, int32_t deststart);
int32_t writeonezdoorset(PACKFILE *f, int32_t index);
int32_t readonezdoorset(PACKFILE *f, int32_t index);

int32_t onExport_Doorset();
int32_t onImport_Doorset();

int32_t onGotoMap();

int32_t onViewPic();
int32_t onViewMap();
int32_t onViewMapEx(bool skipmenu);
int32_t onComboPage();

int32_t onDefault_Pals();
int32_t onDefault_Tiles();
int32_t onDefault_Combos();
int32_t onDefault_Sprites();
int32_t onDefault_MapStyles();

int32_t onCustomItems();
int32_t onCustomWpns();
int32_t onCustomHero();
int32_t onCustomGuys();

int32_t onTest();
int32_t onTestOptions();

int32_t onOptions();

bool edit_combo(int32_t c,bool freshen,int32_t cs);

void draw_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg,int32_t value);
void KeyFileName(char *kfname);

int32_t onSpacebar();
int32_t onMenuSnapshot();
int32_t onSnapshot();
char const* getSnapName();
int32_t onMapscrSnapshot();
int32_t onClearQuestFilepath();
void go();
void comeback();
int32_t checksave();
int32_t onExit();
int32_t onAbout();
int32_t onShowCSet();
int32_t onShowCType();
int32_t onShowWalkability();
int32_t onPreviewMode();
int32_t onShowFlags();
int32_t onP();
int32_t onShowComboInfoCSet();
int32_t onShowDarkness();

int32_t onIncreaseFlag();
int32_t onDecreaseFlag();
int32_t onToggleDarkness();
int32_t onIncMap();
int32_t onDecMap();
int32_t onDumpScr();

int32_t onExport_Package();

#endif
