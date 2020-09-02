#ifndef _ZQUEST_H_
#define _ZQUEST_H_

#include <string>
#include <vector>
#include "midi.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "zcmusic.h"
#include "sprite.h"
#include "gamedata.h"

#define  INTERNAL_VERSION  0xA721

// the following are used by both zelda.cc and zquest.cc

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

#define MAXARROWS 8
#define SHADOW_DEPTH 2
#define MSG_VSYNC MSG_USER

#define rtgxo 2
#define rtgyo 2

#define edc  208 //246
#define edi  209 //247
//#define ed0  248
//#define ed1  249
//#define ed7  250
//#define ed11 251
//#define ed14 252
//#define ed15 253
//#define FLASH 243

#ifdef ALLEGRO_MACOSX
extern int midi_strict; //L
#endif
extern bool cancelgetnum;
extern char headerguard;

extern bool is_large;
extern int RulesetDialog;

extern bool disable_saving, OverwriteProtection;
extern int zq_scale, TileProtection;

void setZScriptVersion(int); //Intentionally does nothing >_<

extern unsigned char PreFillTileEditorPage, PreFillComboEditorPage, PreFillMapTilePage;
extern int DMapEditorLastMaptileUsed;

enum
{
    m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_layers, m_coords2,
    m_menucount
};

enum {MOUSE_BMP_NORMAL=0, MOUSE_BMP_POINT_BOX, MOUSE_BMP_FLAG, MOUSE_BMP_BOX, MOUSE_BMP_SWORD, MOUSE_BMP_POTION, MOUSE_BMP_WAND, MOUSE_BMP_LENS, MOUSE_BMP_GLOVE, MOUSE_BMP_HOOKSHOT, MOUSE_BMP_WAND2, MOUSE_BMP_BLANK, MOUSE_BMP_MAX};
enum {ICON_BMP_ITEM=0, ICON_BMP_WARP, ICON_BMP_WARPDEST, ICON_BMP_FLAG, ICON_BMP_RETURN_A, ICON_BMP_RETURN_B, ICON_BMP_RETURN_C, ICON_BMP_RETURN_D, ICON_BMP_MAX};


extern int CSET_SIZE;
extern int CSET_SHFT;

void update_combo_cycling();
void update_freeform_combos();

bool layers_valid(mapscr *tempscr);
void fix_layers(mapscr *tempscr, bool showwarning);

extern int coord_timer, coord_frame;
extern int blackout_color, zq_screen_w, zq_screen_h;
extern int jwin_pal[jcMAX];

extern size_and_pos minimap;

extern size_and_pos combolist1;
extern size_and_pos combolist1scrollers;
extern size_and_pos combolist2;
extern size_and_pos combolist2scrollers;
extern size_and_pos combolist3;
extern size_and_pos combolist3scrollers;

extern size_and_pos comboaliaslist1;
extern size_and_pos comboalias_preview1;
extern size_and_pos comboaliaslist2;
extern size_and_pos comboalias_preview2;
extern size_and_pos comboaliaslist3;
extern size_and_pos comboalias_preview3;

extern size_and_pos combo_preview;
extern size_and_pos combolist_window;
extern size_and_pos panel[9];
extern size_and_pos panel_button[9];
extern size_and_pos favorites_window;
extern size_and_pos favorites_list;

extern size_and_pos commands_window;
extern size_and_pos commands_list;
extern size_and_pos dummy_panel;

extern size_and_pos tooltip_box;
extern size_and_pos tooltip_trigger;

extern int mapscreen_x, mapscreen_y, mapscreensize, showedges, showallpanels;
extern int mouse_scroll_h;
extern int tooltip_timer, tooltip_maxtimer;

extern bool canfill;                                        //to prevent double-filling (which stops undos)
extern bool resize_mouse_pos;                               //for eyeball combos

extern int lens_hint_item[MAXITEMS][2];                     //aclk, aframe
extern int lens_hint_weapon[MAXWPNS][5];                    //aclk, aframe, dir, x, y
//extern int mode, switch_mode, orig_mode;
extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table, trans_table2;
extern char *datafile_str;
extern RGB_MAP zq_rgb_table;
extern DATAFILE *zcdata, *fontsdata;
extern MIDI *song;
extern FONT *nfont, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
       *goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont;
extern BITMAP *menu1,*menu3, *mapscreenbmp, *tmp_scr, *screen2, *mouse_bmp[MOUSE_BMP_MAX][4], *mouse_bmp_1x[MOUSE_BMP_MAX][4], *icon_bmp[ICON_BMP_MAX][4], *panel_button_icon_bmp[m_menucount][4], *select_bmp[2],*dmapbmp_small, *dmapbmp_large;
extern BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushscreen, *tooltipbmp; //, *brushshadowbmp;
extern byte *colordata, *trashbuf;
//extern byte *tilebuf;
extern comboclass *combo_class_buf;
extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern guydata  *guysbuf;
extern item_drop_object    item_drop_sets[MAXITEMDROPSETS];
extern newcombo curr_combo;
extern PALETTE RAMpal;
extern midi_info Midi_Info;
extern bool zq_showpal;
extern bool combo_cols;


extern int alignment_arrow_timer;
extern int  Flip,Combo,CSet,First[3];
extern int  Flags,Flag,menutype;
extern int MouseScroll, SavePaths, CycleOn, InvalidStatic;
extern int Frameskip, RequestedFPS, zqColorDepth, zqUseWin32Proc;
extern bool Vsync, ShowFPS;
extern int ComboBrush;                                      //show the brush instead of the normal mouse
extern int ComboBrushPause;                                 //temporarily disable the combo brush
extern int BrushPosition;                                   //top left, middle, bottom right, etc.
extern int FloatBrush;                                      //makes the combo brush float a few pixels up and left complete with shadow

extern int OpenLastQuest;                                   //makes the program reopen the quest that was
//open at the time you quit
extern int ShowMisalignments;                               //makes the program display arrows over combos that are
//not aligned with the next screen.
extern int AnimationOn;                                     //animate the combos in zquest?
extern int AutoBackupRetention;                             //use auto-backup feature?  if so, how many backups (1-10) to keep
extern int AutoSaveInterval;                                //how often a timed autosave is made (not overwriting the current file)
extern int UncompressedAutoSaves;                           //should timed saves be uncompressed/encrypted?
extern time_t auto_save_time_start, auto_save_time_current;
extern double auto_save_time_diff;
extern int AutoSaveRetention;                               //how many autosaves of a quest to keep
extern int ImportMapBias;                                   //tells what has precedence on map importing
extern int BrushWidth, BrushHeight;
extern bool quit,saved;
extern bool __debug;
extern byte LayerMask[2];                                   //determines which layers are on or off.  0-15
extern int LayerMaskInt[7];
extern int CurrentLayer;
extern int DuplicateAction[4];
extern int OnlyCheckNewTilesForDuplicates;
/*
  , HorizontalDuplicateAction;
  int VerticalDuplicateAction, BothDuplicateAction;
  */
extern word msg_count, qt_count;
extern word door_combo_set_count;
extern int LeechUpdate;
extern int LeechUpdateTiles;
extern int SnapshotFormat;
extern int memrequested;
extern byte Color;

extern ZCMUSIC *zcmusic;
extern volatile int myvsync;
extern BITMAP *hw_screen;

extern int fill_type;

extern bool first_save;
extern char *filepath,*temppath,*midipath,*datapath,*imagepath,*tmusicpath,*last_timed_save;
extern char *helpbuf;
extern std::string helpstr;

extern int rules_page, rules_pages;
extern int init_page, init_pages;
extern int doorcombos_page, doorcombos_pages;
extern bool rules_done, save_rules;
extern bool init_done, save_init;
extern bool doorcombos_done, save_doorcombos;

void loadlvlpal(int level);

bool get_debug();
void set_debug(bool d);

// quest data
extern zquestheader        header;
extern byte                quest_rules[QUESTRULES_SIZE];
extern byte                extra_rules[EXTRARULES_SIZE];
extern byte                midi_flags[MIDIFLAGS_SIZE];
extern byte                music_flags[MUSICFLAGS_SIZE];
extern word                map_count;
extern miscQdata           misc;
extern std::vector<mapscr> TheMaps;
extern zcmap               *ZCMaps;
extern dmap                *DMaps;
extern MsgStr              *MsgStrings;
extern int				   msg_strings_size;
extern DoorComboSet        *DoorComboSets;
extern class zctune        *customtunes;
//extern emusic            *enhancedMusic;
extern ZCHEATS             zcheats;
extern byte                use_cheats;
extern byte                use_tiles;
extern zinitdata           zinit;
extern char                palnames[MAXLEVELS][17];
extern quest_template      QuestTemplates[MAXQTS];
//extern byte              *undotilebuf;
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
extern char   fontsdat_sig[52];

// qst.cc helpers
bool bad_version(int ver);
fix LinkModifiedX();
fix LinkModifiedY();

extern MENU colors_menu[];

void rebuild_trans_table();
void rebuild_string_list();

int onResetTransparency();
int d_vsync_proc(int msg,DIALOG *d,int c);
int d_nbmenu_proc(int msg,DIALOG *d,int c);
int getnumber(const char *prompt,int initialval);
int gettilepagenumber(const char *prompt, int initialval);
void savesometiles(const char *prompt,int initialval);
void savesomedmaps(const char *prompt,int initialval);
void writesometiles_to(const char *prompt,int initialval);
void savesomecombos(const char *prompt,int initialval);
void writesomecombos_to(const char *prompt,int initialval);
void writesomecombos(const char *prompt,int initialval);
void loadcombopack(const char *prompt,int initialval);
void savesomecomboaliases(const char *prompt,int initialval);
void writesomecomboaliases_to(const char *prompt,int initialval);
void do_importdoorset(const char *prompt,int initialval);
void do_exportdoorset(const char *prompt,int initialval);
int gethexnumber(const char *prompt,int initialval);

void update_combo_cycling();

int onSelectSFX();
int onOptions();
void fix_drawing_mode_menu();
int onDrawingMode();
int onDrawingModeNormal();
int onDrawingModeRelational();
int onDrawingModeDungeon();
int onDrawingModeAlias();
int onReTemplate();
int onUndo();
int onCopy();
int onFlipDMapHorizontal(int d);
int onFlipDMapVertical(int d);
int onFlipMapHorizontal();
int onFlipMapVertical();
int onFlipScreenHorizontal();
int onFlipScreenVertical();
int onH();
int onPaste();
int onPasteAll();
int onPasteToAll();
int onPasteAllToAll();
int onPasteUnderCombo();
int onPasteSecretCombos();
int onPasteFFCombos();
int onPasteWarps();
int onPasteScreenData();
int onPasteWarpLocations();
int onPasteDoors();
int onPasteLayers();
int onPastePalette();
int onPasteRoom();
int onPasteGuy();
int onPasteEnemies();
int onDelete();
int onDeleteMap();
int onToggleDarkness();
int onIncMap();
int onDecMap();
int onDecColour();
int onIncColour();
int onDefault_Pals();
int onDefault_Combos();
int onDefault_Items();
int onDefault_Guys();
int onDefault_Weapons();
int onDefault_SFX();
int onDefault_Tiles();
int onDefault_MapStyles();
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
int on15();
int onLeft();
int onRight();
int onUp();
int onDown();
int onPgUp();
int onPgDn();
int onIncreaseCSet();
int onDecreaseCSet();
int onGotoPage();

bool getname(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
bool getname_nogo(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
//bool getname_nogo(char *prompt,char *ext,char *def,bool usefilename);


int playTune1();
int playTune2();
int playTune3();
int playTune4();
int playTune5();
int playTune6();
int playTune7();
int playTune8();
int playTune9();
int playTune10();
int playTune11();
int playTune12();
int playTune13();
int playTune14();
int playTune15();
int playTune16();
int playTune17();
int playTune18();
int playTune19();

int changeTrack();
int playMusic();
int playTune(int pos);
int stopMusic();

int onTemplates();

//  +----------+
//  |          |
//  | View Pic |
//  |          |
//  |          |
//  |          |
//  +----------+

extern BITMAP *pic;
extern PALETTE picpal;
extern int  pblack,pwhite;
extern double scale;
extern bool vp_showpal, vp_showsize, vp_center;

INLINE int pal_sum(RGB p)
{
    return p.r + p.g + p.b;
}

void get_bw(RGB *pal,int &black,int &white);
void draw_bw_mouse(int white, int old_mouse, int new_mouse);
int load_the_pic(BITMAP **dst, PALETTE dstpal);
int onViewPic();
int load_the_map();
int onViewMap();

//static char *dirstr[4] = {"up","down","left","right"};
//char _pathstr[25]="up,up,up,up";

char *pathstr(byte path[]);
void drawpanel(int panel);
void refresh(int flags);
void select_scr();
void select_combo(int list);
void update_combobrush();
void draw(bool);
void replace(int c);
void draw_block(int start,int w,int h);
void fill(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal, bool only_cset);
void fill2(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal, bool only_cset);
int d_wflag_proc(int msg,DIALOG *d,int c);

/**************************/
/*****     Mouse      *****/
/**************************/

void doxypos(byte &px,byte &py,int color,int mask, bool immediately);
void doxypos(byte &px,byte &py,int color,int mask, bool immediately,  int cursoroffx, int cursoroffy, int iconw, int iconh);
void doflags();
void set_brush_width(int width);
void set_brush_height(int height);

int set_brush_width_1();
int set_brush_width_2();
int set_brush_width_3();
int set_brush_width_4();
int set_brush_width_5();
int set_brush_width_6();
int set_brush_width_7();
int set_brush_width_8();
int set_brush_width_9();
int set_brush_width_10();
int set_brush_width_11();
int set_brush_width_12();
int set_brush_width_13();
int set_brush_width_14();
int set_brush_width_15();
int set_brush_width_16();

int set_brush_height_1();
int set_brush_height_2();
int set_brush_height_3();
int set_brush_height_4();
int set_brush_height_5();
int set_brush_height_6();
int set_brush_height_7();
int set_brush_height_8();
int set_brush_height_9();
int set_brush_height_10();
int set_brush_height_11();

int set_flood();
int set_fill_4();
int set_fill_8();
int set_fill2_4();
int set_fill2_8();

void flood();
void fill_4();
void fill_8();
void fill2_4();
void fill2_8();
void set_brush_width(int width);
void set_brush_height(int height);
void domouse();
void restore_mouse();
int onShowPal();
int onTestGUI();
int onCSetFix();
int onTemplate();
int onComboPage();
const char *flaglist(int index, int *list_size);
int select_data(const char *prompt,int index,const char *(proc)(int,int*), FONT *title_font, void (*copyFunc)(int, int)=0);
int select_data(const char *prompt,int index,const char *(proc)(int,int*), const char *b1, const char *b2, FONT *title_font, void (*copyFunc)(int, int)=0);
int select_flag(int &f);
int d_scombo_proc(int msg,DIALOG *d,int c);
int d_ffcombolist_proc(int msg,DIALOG *d,int c);
int d_comboalist_proc(int msg,DIALOG *d,int c);
int onSecretF();
int onSecretCombo();
int onUnderCombo();
int onImportFFScript();
int onImportItemScript();
int onImportGScript();
int onCompileScript();


typedef struct item_struct
{
    char *s;
    int i;
} item_struct;

extern item_struct bii[iMax+1];

typedef struct weapon_struct
{
    char *s;
    int i;
} weapon_struct;

extern weapon_struct biw[wMAX];

typedef std::pair<std::string, int> script_struct;
void build_biitems_list();
extern script_struct biitems[NUMSCRIPTFFC]; //item script
extern int biitems_cnt;

//extern script_struct biffs[NUMSCRIPTFFC];

typedef struct command_struct
{
    char *s;
    int i;
} command_struct;

enum
{
    cmdNULL,
    cmdAbout,
    cmdCatchall,
    cmdChangeTrack,
    cmdCheats,
    cmdCSetFix,
    cmdDrawingModeAlias,
    cmdEditComboAlias,
    cmdCombos,
    cmdCompileScript,
    cmdCopy,
    cmdDefault_Combos,
    cmdDeleteMap,
    cmdDelete,
    cmdDmaps,
    cmdDoorCombos,
    cmdDoors,
    cmdPasteDoors,
    cmdDrawingModeDungeon,
    cmdEndString,
    cmdCustomEnemies,
    cmdDefault_Guys,
    cmdEnemies,
    cmdPasteEnemies,
    cmdEnhancedMusic,
    cmdCmdExit,
    cmdExport_Combos,
    cmdExport_DMaps,
    cmdExport_Map,
    cmdExport_Pals,
    cmdExport_ZQT,
    cmdExport_Msgs,
    cmdExport_Subscreen,
    cmdExport_Tiles,
    cmdExport_UnencodedQuest,
    cmdExport_ZGP,
    cmdFlags,
    cmdPasteFFCombos,
    cmdSelectFFCombo,
    cmdFullScreen,
    cmdIcons,
    cmdGotoMap,
    cmdGuy,
    cmdPasteGuy,
    cmdHeader,
    cmdHelp,
    cmdImportFFScript,
    cmdImportGScript,
    cmdImportItemScript,
    cmdImport_Combos,
    cmdImport_DMaps,
    cmdImport_ZGP,
    cmdImport_Map,
    cmdImport_Pals,
    cmdImport_ZQT,
    cmdImport_Msgs,
    cmdImport_Subscreen,
    cmdImport_Tiles,
    cmdImport_UnencodedQuest,
    cmdInfoTypes,
    cmdInit,
    cmdIntegrityCheckAll,
    cmdIntegrityCheckRooms,
    cmdIntegrityCheckWarps,
    cmdItem,
    cmdCustomItems,
    cmdLayers,
    cmdPasteLayers,
    cmdColors_Levels,
    cmdCustomLink,
    cmdUsedCombos,
    cmdColors_Main,
    cmdMapCount,
    cmdDefault_MapStyles,
    cmdMapStyles,
    cmdSubscreen,
    cmdString,
    cmdMidis,
    cmdMiscColors,
    cmdNew,
    cmdDrawingModeNormal,
    cmdOpen,
    cmdOptions,
    cmdScreenPalette,
    cmdDefault_Pals,
    cmdPaste,
    cmdPasteAll,
    cmdPasteAllToAll,
    cmdPasteToAll,
    cmdPath,
    cmdPlayMusic,
    cmdX,
    cmdQuestTemplates,
    cmdReTemplate,
    cmdDrawingModeRelational,
    cmdRevert,
    cmdRType,
    cmdPasteRoom,
    cmdAnimationRules,
    cmdSave,
    cmdSaveAs,
    cmdPasteScreenData,
    cmdScrData,
    cmdPasteSecretCombos,
    cmdSecretCombo,
    cmdEditSFX,
    cmdShopTypes,
    cmdSideWarp,
    cmdColors_Sprites,
    cmdDefault_Weapons,
    cmdStopMusic,
    cmdStrings,
    cmdEditSubscreens,
    cmdSnapshot,
    cmdPlayTune,
    cmdTemplate,
    cmdTemplates,
    cmdTileWarp,
    cmdDefault_Tiles,
    cmdTiles,
    cmdToggleGrid,
    cmdTriPieces,
    cmdUnderCombo,
    cmdPasteUnderCombo,
    cmdUndo,
    cmdZQVidMode,
    cmdViewMap,
    cmdShowPal,
    cmdViewPic,
    cmdPasteWarpLocations,
    cmdWarpRings,
    cmdPasteWarps,
    cmdCustomWpns,
    cmdShowDark,
    cmdShowWalkable,
    cmdShowFlags,
    cmdShowCSet,
    cmdShowType,
    cmdComboRules,
    cmdItemRules,
    cmdEnemyRules,
    cmdFixesRules,
    cmdMiscRules,
    cmdDefault_Items,
    cmdItemDropSets,
    cmdPastePalette,
    cmdCompatRules,
    cmdComboLocations,
    cmdComboTypeLocations,
    cmdEnemyLocations,
    cmdItemLocations,
    cmdScriptLocations,
    cmdWhatLinksHere,
    cmdIntegrityCheck,
    cmdSaveZQuestSettings,
    cmdOnClearQuestFilepath,
    cmdFindBuggyNext,
    cmdMAX
};

extern command_struct bic[cmdMAX];

extern combo_alias combo_aliases[MAXCOMBOALIASES];
//int combo_apos;
//int combo_alistpos;

int set_comboaradio(byte layermask);
extern int alias_origin;
void draw_combo_alias_thumbnail(BITMAP *dest, combo_alias *combo, int x, int y, int size);

void build_bii_list(bool usenone);
const char *itemlist(int index, int *list_size);
int select_item(const char *prompt,int item,bool is_editor,int &exit_status);

void build_biw_list();
const char *weaponlist(int index, int *list_size);
int select_weapon(const char *prompt,int weapon);

void build_bir_list();
const char *roomlist(int index, int *list_size);
int select_room(const char *prompt,int room);


//char *doors_string[8]={"wall","passage","locked","shutter","bombable","walk thru","1-way shutter","boss"};
const char *doorslist(int index, int *list_size);
void edit_door(int side);
int d_combo_proc(int msg,DIALOG *d,int c);
const char *doorcombosetlist(int index, int *list_size);
int onDoors();
int onScrData();
const char *nslist(int index, int *list_size);
const char *flaglist(int index, int *list_size);
const char *roomslist(int index, int *list_size);
const char *guyslist(int index, int *list_size);
const char *msgslist(int index, int *list_size);

//static char number_str_buf[32];
//int number_list_size=1;
//bool number_list_zero=false;

const char *numberlist(int index, int *list_size);

//static char dmap_str_buf[37];
//int dmap_list_size=1;
//bool dmap_list_zero=false;

const char *dmaplist(int index, int *list_size);

char *hexnumlist(int index, int *list_size);
const char *maplist(int index, int *list_size);
const char *gotomaplist(int index, int *list_size);
const char *nextmaplist(int index, int *list_size);
const char *midilist(int index, int *list_size);
const char *screenmidilist(int index, int *list_size);
const char *custommidilist(int index, int *list_size);
const char *enhancedmusiclist(int index, int *list_size);
const char *levelnumlist(int index, int *list_size);
int onMapCount();
int onGotoMap();
int onFlags();
int onUsedCombos();
int onItem();
int onRType();
int onGuy();
int onString();
int onEndString();
int onCatchall();
int onScreenPalette();
int onDecScrPal();
int onIncScrPal();
int onDecScrPal16();
int onIncScrPal16();
int onFullScreen();
int isFullScreen();
int onToggleGrid();
int onToggleShowScripts();
int onToggleShowSquares();
int onToggleShowInfo();

//char msgbuf[MSGSTRS*3];

int d_ndroplist_proc(int msg,DIALOG *d,int c);
int d_idroplist_proc(int msg,DIALOG *d,int c);
int d_nidroplist_proc(int msg,DIALOG *d,int c);
int d_ilist_proc(int msg,DIALOG *d,int c);
int d_wlist_proc(int msg,DIALOG *d,int c);
int enelist_proc(int msg,DIALOG *d,int c,bool use_abc_list);
INLINE int d_enelist_proc(int msg,DIALOG *d,int c)
{
    return enelist_proc(msg,d,c,true);
}
INLINE int d_enelistnoabc_proc(int msg,DIALOG *d,int c)
{
    return enelist_proc(msg,d,c,false);
}


/**********************************/
//        Triforce Pieces         //
/**********************************/

/*
  static byte triframe_points[9*4] =
  {
  0,2,2,0,  2,0,4,2,  0,2,4,2,  1,1,3,1,  2,0,2,2,
  1,1,1,2,  1,1,2,2,  3,1,3,2,  3,1,2,2
  };
  */

int d_tri_frame_proc(int msg,DIALOG *d,int c);
int d_tri_edit_proc(int msg,DIALOG *d,int c);
int onTriPieces();

/**********************************/
/***********  onDMaps  ************/
/**********************************/

int d_maptile_proc(int msg,DIALOG *d,int c);
int editdmapmaps(int index);
int d_hexedit_proc(int msg,DIALOG *d,int c);
int xtoi(char *hexstr);
void drawgrid(BITMAP *dest,int x,int y,int grid,int fg,int bg,int div);
void drawgrid(BITMAP *dest,int x,int y,int w, int h, int tw, int th, int *grid,int fg,int bg,int div);
void drawgrid_s(BITMAP *dest,int x,int y,int grid,int fg,int bg,int div);
void drawdmap(int dmap);
int d_dmaplist_proc(int msg,DIALOG *d,int c);
int d_dropdmaplist_proc(int msg,DIALOG *d,int c);
int d_dropdmaptypelist_proc(int msg,DIALOG *d,int c);
int d_grid_proc(int msg,DIALOG *d,int c);
void drawxmap(int map,int xoff,bool large);
int d_xmaplist_proc(int msg,DIALOG *d,int c);

//int xmapspecs[4] = {0,0,84+58,118+32+5};

int onXslider(void *dp3,int d2);

//char *dmaptype_str[dmMAX] = { "Dngn","Overw","Cave","BS-Ow" };

const char *typelist(int index, int *list_size);
void put_title_str(char *s,int x,int y,int fg,int bg,int pos,int lines,int cpl);
int d_title_edit_proc(int msg,DIALOG *d,int c);
void put_intro_str(char *s,int x,int y,int fg,int bg,int pos);
int d_intro_edit_proc(int msg,DIALOG *d,int c);

//char dmap_title[21];
//char dmap_name[33];
//char dmap_intro[73];

void editdmap(int index);

//int selectdmapxy[6] = {164-74,108+8+24,164,80+44+24,164,90+44+24};

int onDmaps();
int onEditSFX(int index);

/************************************/
/**********  onDoorCombos  **********/
/************************************/

void fix_dcs(int index, int dir);
void editdoorcomboset(int index);

const char *doorcombosetlist(int index, int *list_size);

int doorcombosetlist_del();
int copydcs();
int pastedcs();
int replacedcs();
void reset_doorcomboset(int index);
void fix_door_combo_set(word &door_combo_set, byte index);
void init_doorcombosets();
int onDoorCombos();

/*******************************/
/**********  onMidis  **********/
/*******************************/

void edit_tune(int i);
int d_midilist_proc(int msg,DIALOG *d,int c);
int onMidis();

/*******************************/
/******  onEnhancedMusic  ******/
/*******************************/

void edit_music(int i);
int d_musiclist_proc(int msg,DIALOG *d,int c);
int onEnhancedMusic();

/*******************************/
/****  onTest/onTestOptions  ***/
/*******************************/

int onTest();
int onTestOptions();

/*******************************/
/**********  onWarp  ***********/
/*******************************/

const char *warptypelist(int index, int *list_size);

//int warpdmapxy[6] = {188,126,188,100,188,112};

int d_warpdestsel_proc(int msg,DIALOG *d,int c);
int onTileWarpIndex(int index);
int onTileWarp();
int onTimedWarp();
int onItemWarp();
int onSideWarp();
void EditWindWarp(int level);
int onWhistle();

/*******************************/
/*********** onPath ************/
/*******************************/

const char *dirlist(int index, int *list_size);

int onPath();

/********************************/
/********* onInfoTypes **********/
/********************************/

void EditInfoType(int index);
int onInfoTypes();

/********************************/
/********* onShopTypes **********/
/********************************/

void EditShopType(int index);
int onShopTypes();

/***********************************/
/********* onItemDropSets **********/
/***********************************/

int count_item_drop_sets();
void EditItemDropSet(int index);
int onItemDropSets();

/********************************/
/********* onWarpRings **********/
/********************************/

//int curr_ring;

void EditWarpRingScr(int ring,int index);
int d_warplist_proc(int msg,DIALOG *d,int c);
int d_wclist_proc(int msg,DIALOG *d,int c);
const char *wclist(int index, int *list_size);

//int warpringdmapxy[8] = {160,106,160,80,160,92,160,144};

int select_warp();
void EditWarpRing(int ring);
int onWarpRings();

/********************************/
/********** onEnemies ***********/
/********************************/

const char *pattern_list(int index, int *list_size);
int onPattern();
int onEnemyFlags();
const char *enemy_viewer(int index, int *list_size);

typedef struct enemy_struct
{
    char *s;
    int i;
} enemy_struct;

#define zqMAXGUYS    gDUMMY1

extern enemy_struct bie[eMAXGUYS];
extern enemy_struct big[zqMAXGUYS];
void build_bie_list(bool hide);
void build_big_list(bool hide);
const char *enemylist(int index, int *list_size);
const char *guylist(int index, int *list_size);
int efrontfacingtile(int id);
int select_enemy(const char *prompt,int enemy,bool hide,bool edit,int& exit_status);
int select_guy(const char *prompt,int guy);

//unsigned char check[2] = { ';'+128,0 };

int onEnemies();

/*******************************/
/********** onHeader ***********/
/*******************************/

//char author[65],title[65],password[32];
int d_showedit_proc(int msg,DIALOG *d,int c);
int onHeader();

//static ZCHEATS tmpcheats;
int onCheats();
int RulesPage_1();
int RulesPage_2();
int RulesPage_3();
int RulesPage_4();
int RulesPage_5();
int RulesPage_6();
int RulesPage_7();
int RulesPage_8();
int RulesPage_9();
int RulesPage_10();
int onAnimationRules();
int onComboRules();
int onItemRules();
int onEnemyRules();
int onFixesRules();
int onMiscRules();
int d_line_proc(int msg, DIALOG *d, int c);
const char *subscrtypelist(int index, int *list_size);
int InitPage_1();
int InitPage_2();
int InitPage_3();
int InitPage_4();
int InitPage_5();
int InitPage_6();
int InitPage_7();
int InitPage_8();
int onInit();
int onItemProps();
int onSubscreen();

int writeonedmap(PACKFILE *f, int i);
int readonedmap(PACKFILE *f, int index);
int writesomedmaps(PACKFILE *f, int first, int last, int max);
int readsomedmaps(PACKFILE *f);
//need readsomedmaps_to, with a starting index, in the future

void get_cset(int dataset,int row,RGB *pal);
void draw_edit_dataset_specs(int index,int copy);
void init_colormixer();
void colormixer(int color,int gray,int ratio);
bool do_x_button(BITMAP *dest, int x, int y);
void edit_dataset(int dataset);
int pal_index(RGB *pal,RGB c);
void grab_dataset(int dataset);

//byte cset_hold[15][16*3];
//byte cset_hold_cnt;
//bool cset_ready = false;
//int cset_count,cset_first;
//PALETTE pal,undopal;

//byte rc[16] = {253,248,0,0,0,0,0,246,247,249,250,251,252,240,255,254};

void undo_pal();
void calc_dark(int first);
void edit_cycles(int level);
void draw_cset_proc(DIALOG *d);
int d_cset_proc(int msg,DIALOG *d,int c);

int d_dummy_proc(int msg,DIALOG *d,int c);

//byte mainpal_csets[30]    = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
//byte levelpal_csets[26]   = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
//byte levelpal2_csets[26]  = { 2,3,4,9,2,0,1,2,3,4, 5, 6, 7,       15,15,15,15, 8,  9,9,9,9,9,9,9,9 };
//byte spritepal_csets[30]  = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
//byte spritepal2_csets[30] = { 15,16,17,18,19,20,21,22,23,24,25,26,27,28,29, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };

int EditColors(const char *caption,int first,int count,byte *label);
//int onColors_Main();
//int onColors_Levels();
//int onColors_Sprites();
int d_maptile_proc(int msg,DIALOG *d,int c);
int onMapStyles();
int d_misccolors_proc(int msg,DIALOG *d,int c);
int onMiscColors();
int onTestBox();


int d_ticsedit_proc(int msg,DIALOG *d,int c);

// ****  Palette cycling  ****

//static int palclk[3];
//static int palpos[3];

void reset_pal_cycling();
void cycle_palette();

/********************/
/******  Help  ******/
/********************/

void doHelp(int bg,int fg);
int onshieldblockhelp();
int onHelp();
int onZstringshelp();
int onZScripthelp();
int edit_layers(mapscr* tempscr);
void autolayer(mapscr* tempscr, int layer, int al[6][3]);
int findblankcombo();
int onLayers();
int PickRuleset();

// **** Timers ****

//volatile int lastfps=0;
//volatile int framecnt=0;
extern volatile int myvsync;

void myvsync_callback();
void fps_callback();

extern BITMAP *hw_screen;

/********************/
/******  MAIN  ******/
/********************/

void custom_vsync();
void switch_out();
void switch_in();
void Z_eventlog(const char *format,...);
void Z_scripterrlog(const char * const format,...);
void zprint(const char * const format,...);
void zprint2(const char * const format,...);
int get_currdmap();
int current_item(int item_type);
int current_item_power(int item_type);
int current_item_id(int item_type, bool checkmagic);
bool can_use_item(int item_type, int item);
bool has_item(int item_type, int it);
int get_bmaps(int si);

bool no_subscreen();

extern int Awpn, Bwpn, Bpos;
extern sprite_list Sitems;

int main(int argc,char **argv);
int d_nbmenu_proc(int msg,DIALOG *d,int c);
void center_zquest_dialogs();
void animate_coords();
void do_animations();
int onZQVidMode();
bool is_zquest();
int save_config_file();
int d_timer_proc(int msg, DIALOG *d, int c);
void check_autosave();

void update_tooltip(int x, int y, int trigger_x, int trigger_y, int trigger_w, int trigger_h, char *tipmsg);
void clear_tooltip();
void ZQ_ClearQuestPath();
void cflag_help(int id);
void ctype_help(int id);
void questrev_help();
void questminrev_help();

//throw together midi_strict hack
#ifdef ALLEGRO_LINUX_GP2X
extern int midi_strict;
#endif

// game.maps[] flags
#define mSECRET             8192                                 // only overworld and caves use this
#define mVISITED            16384                                 // only overworld uses this

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
//#define mOTHER           8192                                 // overwrite this value, use for expansion
//#define mOTHER2         16384                                 // overwrite this value, use for expansion
#define mOTHER3         32768                                 // overwrite this value, use for expansion
#endif
/* end */

