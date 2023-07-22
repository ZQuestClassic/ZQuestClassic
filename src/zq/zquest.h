#ifndef _ZQUEST_H_
#define _ZQUEST_H_

#include <string>
#include <vector>
#include "midi.h"
#include "jwin.h"
#include "base/jwinfsel.h"
#include "zcmusic.h"
#include "sprite.h"
#include "gamedata.h"
#include "parser/parserDefs.h"
#include "zfix.h"
#include "base/fonts.h"

#define  INTERNAL_VERSION  0xA721

// the following are used by both zelda.cc and zquest.cc

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

#define MAXARROWS 8
#define SHADOW_DEPTH 2

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
extern int32_t midi_strict; //L
#endif
extern bool cancelgetnum;

extern int32_t RulesetDialog;

extern bool disable_saving, OverwriteProtection;
extern int32_t TileProtection;

extern uint8_t PreFillTileEditorPage, PreFillComboEditorPage, PreFillMapTilePage;
extern int32_t DMapEditorLastMaptileUsed;

enum
{
    m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_layers, m_coords2,
    m_menucount
};

enum {MOUSE_BMP_NORMAL=0, MOUSE_BMP_POINT_BOX, MOUSE_BMP_FLAG, MOUSE_BMP_BOX, MOUSE_BMP_SWORD, MOUSE_BMP_POTION, MOUSE_BMP_WAND, MOUSE_BMP_LENS, MOUSE_BMP_GLOVE, MOUSE_BMP_HOOKSHOT, MOUSE_BMP_WAND2, MOUSE_BMP_BLANK, MOUSE_BMP_MAX};
enum
{
	ZQM_NORMAL,
	ZQM_POINT_BOX,
	ZQM_BOX,
	ZQM_SWORD,
	ZQM_POTION,
	ZQM_WAND,
	ZQM_LENS,
	ZQM_GLOVE_OPEN, ZQM_GLOVE_CLOSED,
	ZQM_HOOK_PLAIN, ZQM_HOOK_PLUS, ZQM_HOOK_MINUS, ZQM_HOOK_X,
	ZQM_SEL_WAND_PLAIN, ZQM_SEL_WAND_PLUS, ZQM_SEL_WAND_MINUS, ZQM_SEL_WAND_X,
	ZQM_FLAG_0, ZQM_FLAG_1, ZQM_FLAG_2, ZQM_FLAG_3, ZQM_FLAG_4, ZQM_FLAG_5, ZQM_FLAG_6, ZQM_FLAG_7, ZQM_FLAG_8, ZQM_FLAG_9, ZQM_FLAG_10, ZQM_FLAG_11, ZQM_FLAG_12, ZQM_FLAG_13, ZQM_FLAG_14, ZQM_FLAG_15,
	ZQM_BLANK
};
enum {ICON_BMP_ITEM=0, ICON_BMP_WARP, ICON_BMP_WARPDEST, ICON_BMP_FLAG, ICON_BMP_RETURN_A, ICON_BMP_RETURN_B, ICON_BMP_RETURN_C, ICON_BMP_RETURN_D, ICON_BMP_MAX};


extern int32_t CSET_SIZE;
extern int32_t CSET_SHFT;

void update_combo_cycling();
void update_freeform_combos();

bool layers_valid(mapscr *tempscr);
void fix_layers(mapscr *tempscr, bool showwarning);

extern int32_t coord_timer, coord_frame;
extern int32_t blackout_color, zq_screen_w, zq_screen_h;
extern int32_t jwin_pal[jcMAX];

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

extern int32_t mapscreen_x, mapscreen_y, mapscreensize, showedges, showallpanels;
extern int32_t mouse_scroll_h;
extern int32_t tooltip_timer, tooltip_maxtimer;

extern bool canfill;                                        //to prevent double-filling (which stops undos)
extern bool resize_mouse_pos;                               //for eyeball combos

extern int32_t lens_hint_item[MAXITEMS][2];                     //aclk, aframe
extern int32_t lens_hint_weapon[MAXWPNS][5];                    //aclk, aframe, dir, x, y
//extern int32_t mode, switch_mode, orig_mode;
extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table, trans_table2;
extern char *datafile_str;
extern RGB_MAP zq_rgb_table;
extern DATAFILE *zcdata, *fontsdata;
extern size_t fontsdat_cnt;
extern MIDI *song;
extern BITMAP *menu1,*menu3, *mapscreenbmp, *tmp_scr, *screen2, *mouse_bmp[MOUSE_BMP_MAX][4], *mouse_bmp_1x[MOUSE_BMP_MAX][4], *icon_bmp[ICON_BMP_MAX][4], *flag_bmp[16][4], *panel_button_icon_bmp[m_menucount][4], *select_bmp[2],*dmapbmp_small, *dmapbmp_large;
extern BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushscreen, *tooltipbmp, *tooltipbmp2; //, *brushshadowbmp;
extern byte *colordata, *trashbuf;
//extern byte *tilebuf;
extern comboclass *combo_class_buf;
extern wpndata  *wpnsbuf;
extern guydata  *guysbuf;
extern item_drop_object    item_drop_sets[MAXITEMDROPSETS];
extern newcombo curr_combo;
extern PALETTE RAMpal;
extern midi_info Midi_Info;
extern bool zq_showpal;
extern bool combo_cols;
extern bool zoomed_minimap;


extern int32_t alignment_arrow_timer;
extern int32_t  Flip,Combo,CSet;
extern int32_t  Flags,Flag,menutype;
extern int32_t MouseScroll, SavePaths, CycleOn, InvalidStatic, NoScreenPreview,WarnOnInitChanged,DisableLPalShortcuts,DisableCompileConsole,skipLayerWarning,numericalFlags;
extern int32_t Frameskip, RequestedFPS, zqColorDepth, zqUseWin32Proc;
extern bool Vsync, ShowFPS, SaveDragResize, DragAspect, SaveWinPos;
extern double aspect_ratio;
extern int window_min_width, window_min_height;
extern int32_t ComboBrush;                                      //show the brush instead of the normal mouse
extern int32_t ComboBrushPause;                                 //temporarily disable the combo brush
extern int32_t FloatBrush;                                      //makes the combo brush float a few pixels up and left complete with shadow

extern int32_t OpenLastQuest;                                   //makes the program reopen the quest that was
//open at the time you quit
extern int32_t ShowMisalignments;                               //makes the program display arrows over combos that are
//not aligned with the next screen.
extern int32_t AnimationOn;                                     //animate the combos in zquest?
extern int32_t AutoBackupRetention;                             //use auto-backup feature?  if so, how many backups (1-10) to keep
extern int32_t AutoSaveInterval;                                //how often a timed autosave is made (not overwriting the current file)
extern int32_t UncompressedAutoSaves;                           //should timed saves be uncompressed/encrypted?
extern time_t auto_save_time_start, auto_save_time_current;
extern double auto_save_time_diff;
extern int32_t AutoSaveRetention;                               //how many autosaves of a quest to keep
extern int32_t ImportMapBias;                                   //tells what has precedence on map importing
extern int32_t BrushWidth, BrushHeight;
extern bool quit,saved;
extern bool __debug;
extern int32_t LayerMaskInt[7];
extern int32_t CurrentLayer;
extern int32_t DuplicateAction[4];
extern int32_t OnlyCheckNewTilesForDuplicates;
/*
  , HorizontalDuplicateAction;
  int32_t VerticalDuplicateAction, BothDuplicateAction;
  */
extern word msg_count, qt_count;
extern word door_combo_set_count;
extern int32_t LeechUpdate;
extern int32_t LeechUpdateTiles;
extern int32_t SnapshotFormat;
extern int32_t memrequested;
extern byte Color;

extern byte compile_tune;

extern ZCMUSIC *zcmusic;
extern volatile int32_t myvsync;

extern int32_t fill_type;

extern bool first_save;
extern char *filepath,*temppath,*midipath,*datapath,*imagepath,*tmusicpath,*last_timed_save;
extern char *helpbuf;
extern std::string helpstr;

extern int32_t rules_page, rules_pages;
extern int32_t init_page, init_pages;
extern int32_t doorcombos_page, doorcombos_pages;
extern bool rules_done, save_rules;
extern bool init_done, save_init;
extern bool doorcombos_done, save_doorcombos;

extern int32_t window_width, window_height;
void set_last_timed_save(char const* buf);
void loadlvlpal(int32_t level);

bool get_debug();
void set_debug(bool d);


// quest data
extern zquestheader        header;
extern byte                quest_rules[QUESTRULES_NEW_SIZE];
extern byte                extra_rules[EXTRARULES_SIZE];
extern byte                midi_flags[MIDIFLAGS_SIZE];
extern byte                music_flags[MUSICFLAGS_SIZE];
extern word                map_count;
extern miscQdata           misc;
extern std::vector<mapscr> TheMaps;
extern std::vector<word>   map_autolayers;
extern zcmap               *ZCMaps;
extern dmap                *DMaps;
extern MsgStr              *MsgStrings;
extern int32_t				   msg_strings_size;
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

extern char   fontsdat_sig[52];

extern uint8_t console_is_open;

// qst.cc helpers
bool bad_version(int32_t ver);
zfix HeroModifiedX();
zfix HeroModifiedY();

extern MENU colors_menu[];

void rebuild_trans_table();
void rebuild_string_list();

int32_t onResetTransparency();
int32_t d_vsync_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_nbmenu_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t getnumber(const char *prompt,int32_t initialval);
int32_t gettilepagenumber(const char *prompt, int32_t initialval);

void about_module(const char *prompt,int32_t initialval);
void savesometiles(const char *prompt,int32_t initialval);
void writesometiles_to(const char *prompt,int32_t initialval);

void savesomecombos(const char *prompt,int32_t initialval);
void writesomecombos_to(const char *prompt,int32_t initialval);
void writesomecombos(const char *prompt,int32_t initialval);
void loadcombopack(const char *prompt,int32_t initialval);

void savesomecomboaliases(const char *prompt,int32_t initialval);
void writesomecomboaliases_to(const char *prompt,int32_t initialval);

int32_t writeonedmap(PACKFILE *f, int32_t i);
int32_t readonedmap(PACKFILE *f, int32_t index);
int32_t writesomedmaps(PACKFILE *f, int32_t first, int32_t last, int32_t max);
int32_t readsomedmaps(PACKFILE *f);
//need readsomedmaps_to, with a starting index, in the future
void savesomedmaps(const char *prompt,int32_t initialval);

void do_importdoorset(const char *prompt,int32_t initialval);
void do_exportdoorset(const char *prompt,int32_t initialval);

int32_t gettilepagenumber(const char *prompt, int32_t initialval);
int32_t gethexnumber(const char *prompt,int32_t initialval);

void update_combo_cycling();

bool confirmBox(const char *m1, const char *m2 = NULL, const char *m3 = NULL);
int32_t onSelectSFX();
int32_t onOptions();
void fix_drawing_mode_menu();
void reset_relational_tile_grid();
int32_t onDrawingMode();
int32_t onDrawingModeNormal();
int32_t onDrawingModeRelational();
int32_t onDrawingModeDungeon();
int32_t onDrawingModeAlias();
int32_t onDrawingModePool();
int32_t onReTemplate();
int32_t onUndo();
int32_t onRedo();
int32_t onCopy();
int32_t onFlipDMapHorizontal(int32_t d);
int32_t onFlipDMapVertical(int32_t d);
int32_t onFlipMapHorizontal();
int32_t onFlipMapVertical();
int32_t onFlipScreenHorizontal();
int32_t onFlipScreenVertical();
int32_t onPaste();
int32_t onPasteAll();
int32_t onPasteToAll();
int32_t onPasteAllToAll();
int32_t onPasteUnderCombo();
int32_t onPasteSecretCombos();
int32_t onPasteFFCombos();
int32_t onPasteWarps();
int32_t onPasteScreenData();
int32_t onPasteWarpLocations();
int32_t onPasteDoors();
int32_t onPasteLayers();
int32_t onPastePalette();
int32_t onPasteRoom();
int32_t onPasteGuy();
int32_t onPasteEnemies();
int32_t onDelete();
int32_t onDeleteMap();
int32_t onToggleDarkness();
int32_t onIncMap();
int32_t onDecMap();
int32_t onDecColour();
int32_t onIncColour();
int32_t onDefault_Pals();
int32_t onDefault_Combos();
int32_t onDefault_Items();
int32_t onDefault_Guys();
int32_t onDefault_Weapons();
int32_t onDefault_SFX();
int32_t onDefault_Tiles();
int32_t onDefault_MapStyles();
int32_t onPgUp();
int32_t onPgDn();
int32_t onIncreaseCSet();
int32_t onDecreaseCSet();
int32_t onGotoPage();

bool getname(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
bool getname_nogo(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
//bool getname_nogo(char *prompt,char *ext,char *def,bool usefilename);

int32_t playTune1();
int32_t playTune2();
int32_t playTune3();
int32_t playTune4();
int32_t playTune5();
int32_t playTune6();
int32_t playTune7();
int32_t playTune8();
int32_t playTune9();
int32_t playTune10();
int32_t playTune11();
int32_t playTune12();
int32_t playTune13();
int32_t playTune14();
int32_t playTune15();
int32_t playTune16();
int32_t playTune17();
int32_t playTune18();
int32_t playTune19();

int32_t changeTrack();
int32_t playMusic();
int32_t playTune(int32_t pos);
int32_t stopMusic();

int32_t onQMiscValues();

int32_t onTemplates();

//  +----------+
//  |          |
//  | View Pic |
//  |          |
//  |          |
//  |          |
//  +----------+

extern BITMAP *pic;
extern PALETTE picpal;
extern int32_t  pblack,pwhite;
extern bool vp_showpal, vp_showsize, vp_center;

INLINE int32_t pal_sum(RGB p)
{
    return p.r + p.g + p.b;
}

void get_bw(RGB *pal,int32_t &black,int32_t &white);
void draw_bw_mouse(int32_t white, int32_t old_mouse, int32_t new_mouse);
int32_t load_the_pic(BITMAP **dst, PALETTE dstpal);
int load_the_pic_new(BITMAP **dst, PALETTE dstpal);
int32_t onViewPic();
int32_t load_the_map();
int32_t onViewMap();

//static char *dirstr[4] = {"up","down","left","right"};
//char _pathstr[25]="up,up,up,up";

char *pathstr(byte path[]);
void drawpanel(int32_t panel);
void refresh(int32_t flags, bool update = false);
void select_scr();
void select_combo(int32_t list);
void update_combobrush();
void draw(bool);
void replace(int32_t c);
void draw_block(int32_t start,int32_t w,int32_t h);
int32_t d_wflag_proc(int32_t msg,DIALOG *d,int32_t c);

/**************************/
/*****     Mouse      *****/
/**************************/

void doxypos(byte &px,byte &py,int32_t color,int32_t mask, bool immediately);
void doxypos(byte &px,byte &py,int32_t color,int32_t mask, bool immediately,  int32_t cursoroffx, int32_t cursoroffy, int32_t iconw, int32_t iconh);
void doflags();
void set_brush_width(int32_t width);
void set_brush_height(int32_t height);

int32_t set_brush_width_1();
int32_t set_brush_width_2();
int32_t set_brush_width_3();
int32_t set_brush_width_4();
int32_t set_brush_width_5();
int32_t set_brush_width_6();
int32_t set_brush_width_7();
int32_t set_brush_width_8();
int32_t set_brush_width_9();
int32_t set_brush_width_10();
int32_t set_brush_width_11();
int32_t set_brush_width_12();
int32_t set_brush_width_13();
int32_t set_brush_width_14();
int32_t set_brush_width_15();
int32_t set_brush_width_16();

int32_t set_brush_height_1();
int32_t set_brush_height_2();
int32_t set_brush_height_3();
int32_t set_brush_height_4();
int32_t set_brush_height_5();
int32_t set_brush_height_6();
int32_t set_brush_height_7();
int32_t set_brush_height_8();
int32_t set_brush_height_9();
int32_t set_brush_height_10();
int32_t set_brush_height_11();

int32_t set_flood();
int32_t set_fill_4();
int32_t set_fill_8();
int32_t set_fill2_4();
int32_t set_fill2_8();

void flood();
void flood_flag();
void fill_4();
void fill_4_flag();
void fill_8();
void fill_8_flag();
void fill2_4();
void fill2_8();
void set_brush_width(int32_t width);
void set_brush_height(int32_t height);
void domouse();
void restore_mouse();
int32_t onShowPal();
int32_t onTestGUI();
int32_t onCSetFix();
int32_t onWaterSolidity();
int32_t onEffectFix();
int32_t onTemplate();
int32_t onComboPage();
int32_t select_data(const char *prompt,int32_t index,const char *(proc)(int32_t,int32_t*), FONT *title_font, void (*copyFunc)(int32_t, int32_t)=0);
int32_t select_data(const char *prompt,int32_t index,const char *(proc)(int32_t,int32_t*), const char *b1, const char *b2, FONT *title_font, void (*copyFunc)(int32_t, int32_t)=0);
int32_t select_flag(int32_t &f);
int32_t d_scombo_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ffcombolist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_comboalist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onSecretF();
int32_t onSecretCombo();
int32_t onUnderCombo();
int32_t load_zmod_module_file();
int32_t onImportFFScript();
int32_t onImportItemScript();
int32_t onImportNPCScript();
int32_t onImportSCREENScript();
int32_t onImportHEROScript();
int32_t onImportITEMSPRITEScript();
int32_t onImportComboScript();
int32_t onImportDMapScript();
int32_t onImportLWPNScript();
int32_t onImportEWPNScript();
int32_t onImportGScript();
int32_t onCompileScript();
int32_t onSlotAssign();
int32_t onExportZASM();
int32_t onImportZASM();


typedef struct item_struct
{
    char *s;
    int32_t i;
} item_struct;

extern item_struct bii[iMax+1];

typedef struct weapon_struct
{
    char *s;
    int32_t i;
} weapon_struct;

extern weapon_struct biw[wMAX];

typedef std::pair<std::string, int32_t> script_struct;
void build_biitems_list();
void build_bidcomboscripts_list();
extern script_struct biitems[NUMSCRIPTITEM]; //item script
extern int32_t biitems_cnt;


//npc script list for editors
void build_binpcs_list();
extern script_struct binpcs[NUMSCRIPTGUYS]; //item script
extern int32_t binpcs_cnt;

//lweapon script list for editors
void build_bilweapons_list();
extern script_struct bilweapons[NUMSCRIPTWEAPONS]; //item script
extern int32_t bilweapons_cnt;

//eweapon script list for editors
void build_bieweapons_list();
extern script_struct bieweapons[NUMSCRIPTWEAPONS]; //item script
extern int32_t bieweapons_cnt;

//player script list for editors
void build_bihero_list();
extern script_struct bihero[NUMSCRIPTPLAYER]; //item script
extern int32_t bihero_cnt;

//screen script list for editors
void build_biscreens_list();
extern script_struct biscreens[NUMSCRIPTSCREEN]; //item script
extern int32_t biscreens_cnt;

//dmap script list for editors
void build_bidmaps_list();
extern script_struct bidmaps[NUMSCRIPTSDMAP]; //item script
extern int32_t bidmaps_cnt;

//dmap script list for editors
void build_biitemsprites_list();
extern script_struct biditemsprites[NUMSCRIPTSDMAP]; //item script
extern int32_t biitemsprites_cnt;

//extern script_struct biffs[NUMSCRIPTFFC];

typedef struct command_struct
{
    char *s;
    int32_t i;
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
    cmdImportZASM,
	cmdImportGScript, //Deprecated
	cmdImportItemScript, //Deprecated
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
    cmdCustomHero,
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
    cmdQuestRules,
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
    cmdZScriptRules,
    cmdExportZASM,
    cmdHeroRules,
    cmdZScriptCompilerRules,
    cmdWeaponRules,
    cmdScreenScript,
    cmdScreenSnapshot,
	cmdViewL2BG,
	cmdViewL3BG,
	cmdBottleTypes,
	cmdBottleShopTypes,
	cmdWaterSolidFix,
	cmdEffectSquareFix,
	cmdTestQuest,
    cmdRedo,
	cmdDrawingModePool,
	cmdQRSearch,
	cmdQuickCompile,
    cmdMAX
};

extern command_struct bic[cmdMAX];

extern combo_alias combo_aliases[MAXCOMBOALIASES];
extern combo_pool combo_pools[MAXCOMBOPOOLS];
//int32_t combo_apos;
//int32_t combo_alistpos;

int32_t set_comboaradio(byte layermask);
extern int32_t alias_origin;
void draw_combo_alias_thumbnail(BITMAP *dest, combo_alias *combo, int32_t x, int32_t y, int32_t size);

void build_bii_list(bool usenone);
const char *itemlist(int32_t index, int32_t *list_size);
const char *itemlist_num(int32_t index, int32_t *list_size);

void build_biw_list();
const char *weaponlist(int32_t index, int32_t *list_size);
const char *weaponlist_num(int32_t index, int32_t *list_size);
int32_t select_weapon(const char *prompt,int32_t weapon);

//char *doors_string[8]={"wall","passage","locked","shutter","bombable","walk thru","1-way shutter","boss"};
const char *doorslist(int32_t index, int32_t *list_size);
void edit_door(int32_t side);
int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c);
const char *doorcombosetlist(int32_t index, int32_t *list_size);
int32_t onDoors();
int32_t onScrData();
int32_t onScreenScript();
const char *nslist(int32_t index, int32_t *list_size);
const char *roomslist(int32_t index, int32_t *list_size);
const char *guyslist(int32_t index, int32_t *list_size);
const char *msgslist(int32_t index, int32_t *list_size);

//static char number_str_buf[32];
//int32_t number_list_size=1;
//bool number_list_zero=false;

const char *numberlist(int32_t index, int32_t *list_size);

//static char dmap_str_buf[37];
//int32_t dmap_list_size=1;
//bool dmap_list_zero=false;

const char *dmaplist(int32_t index, int32_t *list_size);

char *hexnumlist(int32_t index, int32_t *list_size);
const char *maplist(int32_t index, int32_t *list_size);
const char *gotomaplist(int32_t index, int32_t *list_size);
const char *nextmaplist(int32_t index, int32_t *list_size);
const char *midilist(int32_t index, int32_t *list_size);
const char *screenmidilist(int32_t index, int32_t *list_size);
const char *custommidilist(int32_t index, int32_t *list_size);
const char *enhancedmusiclist(int32_t index, int32_t *list_size);
const char *levelnumlist(int32_t index, int32_t *list_size);
int32_t onGotoMap();
int32_t onFlags();
int32_t onUsedCombos();
int32_t onItem();
int32_t onZScriptSettings();
int32_t onZScriptCompilerSettings();
int32_t onRoom();
int32_t onEndString();
int32_t onScreenPalette();
int32_t onDecScrPal();
int32_t onIncScrPal();
int32_t onDecScrPal16();
int32_t onIncScrPal16();
int32_t onFullScreen();
int32_t isFullScreen();
int32_t onToggleGrid(bool color);
int32_t onToggleGrid();
int32_t onToggleShowScripts();
int32_t onToggleShowSquares();
int32_t onToggleShowFFCs();
int32_t onToggleShowInfo();

//char msgbuf[MSGSTRS*3];

int32_t d_ndroplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_idroplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_nidroplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_wlist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t enelist_proc(int32_t msg,DIALOG *d,int32_t c,bool use_abc_list);
INLINE int32_t d_enelist_proc(int32_t msg,DIALOG *d,int32_t c)
{
    return enelist_proc(msg,d,c,true);
}
INLINE int32_t d_enelistnoabc_proc(int32_t msg,DIALOG *d,int32_t c)
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

int32_t d_tri_frame_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_tri_edit_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onTriPieces();

/**********************************/
/***********  onDMaps  ************/
/**********************************/

int32_t d_maptile_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t editdmapmaps(int32_t index);
int32_t d_hexedit_proc(int32_t msg,DIALOG *d,int32_t c);
void drawgrid(BITMAP *dest,int32_t x,int32_t y,int32_t grid,int32_t fg,int32_t bg,int32_t div);
void drawgrid(BITMAP *dest,int32_t x,int32_t y,int32_t w, int32_t h, int32_t tw, int32_t th, int32_t *grid,int32_t fg,int32_t bg,int32_t div);
void drawgrid_s(BITMAP *dest,int32_t x,int32_t y,int32_t grid,int32_t fg,int32_t bg,int32_t div);
void drawdmap(int32_t dmap);
int32_t d_dmaplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_dropdmaplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_dropdmaptypelist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_grid_proc(int32_t msg,DIALOG *d,int32_t c);
void drawxmap(int32_t map,int32_t xoff,bool large);
int32_t d_xmaplist_proc(int32_t msg,DIALOG *d,int32_t c);

//int32_t xmapspecs[4] = {0,0,84+58,118+32+5};

int32_t onXslider(void *dp3,int32_t d2);

//char *dmaptype_str[dmMAX] = { "Dngn","Overw","Cave","BS-Ow" };

const char *typelist(int32_t index, int32_t *list_size);
void put_title_str(char *s,int32_t x,int32_t y,int32_t fg,int32_t bg,int32_t pos,int32_t lines,int32_t cpl);
int32_t d_title_edit_proc(int32_t msg,DIALOG *d,int32_t c);
void put_intro_str(char *s,int32_t x,int32_t y,int32_t fg,int32_t bg,int32_t pos);
int32_t d_intro_edit_proc(int32_t msg,DIALOG *d,int32_t c);

//char dmap_title[21];
//char dmap_name[33];
//char dmap_intro[73];

void editdmap(int32_t index);

//int32_t selectdmapxy[6] = {164-74,108+8+24,164,80+44+24,164,90+44+24};

int32_t onDmaps();
int32_t onEditSFX(int32_t index);

/************************************/
/**********  onDoorCombos  **********/
/************************************/

void fix_dcs(int32_t index, int32_t dir);
void editdoorcomboset(int32_t index);

const char *doorcombosetlist(int32_t index, int32_t *list_size);

int32_t doorcombosetlist_del();
int32_t copydcs();
int32_t pastedcs();
int32_t replacedcs();
void reset_doorcomboset(int32_t index);
void fix_door_combo_set(word &door_combo_set, byte index);
void init_doorcombosets();
int32_t onDoorCombos();

/*******************************/
/**********  onMidis  **********/
/*******************************/

void edit_tune(int32_t i);
int32_t d_midilist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onMidis();

/*******************************/
/******  onEnhancedMusic  ******/
/*******************************/

void edit_music(int32_t i);
int32_t d_musiclist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onEnhancedMusic();

/*******************************/
/****  onTest/onTestOptions  ***/
/*******************************/

int32_t onTest();
int32_t onTestOptions();

/*******************************/
/**********  onWarp  ***********/
/*******************************/

const char *warptypelist(int32_t index, int32_t *list_size);

//int32_t warpdmapxy[6] = {188,126,188,100,188,112};

int32_t d_warpdestsel_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onTileWarpIndex(int32_t index);
int32_t onTileWarp();
int32_t onTimedWarp();
int32_t onItemWarp();
int32_t onSideWarp();
void EditWindWarp(int32_t level);
int32_t onWhistle();



const char *dirlist(int32_t index, int32_t *list_size);

int32_t onPath();

void EditInfoType(int32_t index);
int32_t onInfoTypes();

void EditShopType(int32_t index);
int32_t onShopTypes();

int32_t onBottleTypes();

int32_t onBottleShopTypes();

int32_t count_item_drop_sets();
void EditItemDropSet(int32_t index);
int32_t onItemDropSets();

/********************************/
/********* onWarpRings **********/
/********************************/

//int32_t curr_ring;

void EditWarpRingScr(int32_t ring,int32_t index);
int32_t d_warplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_wclist_proc(int32_t msg,DIALOG *d,int32_t c);
const char *wclist(int32_t index, int32_t *list_size);

//int32_t warpringdmapxy[8] = {160,106,160,80,160,92,160,144};

int32_t select_warp();
void EditWarpRing(int32_t ring);
int32_t onWarpRings();

/********************************/
/********** onEnemies ***********/
/********************************/

const char *pattern_list(int32_t index, int32_t *list_size);
int32_t onPattern();
int32_t onEnemyFlags();
const char *enemy_viewer(int32_t index, int32_t *list_size);

typedef struct enemy_struct
{
    char *s;
    int32_t i;
} enemy_struct;

#define zqMAXGUYS    gDUMMY1

extern enemy_struct bie[eMAXGUYS];
extern enemy_struct big[zqMAXGUYS];
void build_bie_list(bool hide);
void build_big_list(bool hide);
const char *enemylist(int32_t index, int32_t *list_size);
const char *guylist(int32_t index, int32_t *list_size);
int32_t efrontfacingtile(int32_t id);
int32_t select_enemy(const char *prompt,int32_t enemy,bool hide,bool edit,int32_t& exit_status);

//uint8_t check[2] = { ';'+128,0 };

int32_t onEnemies();

/*******************************/
/********** onHeader ***********/
/*******************************/

//char author[65],title[65],password[32];
int32_t d_showedit_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onHeader();

int32_t PickRuleset();
int32_t PickRuleTemplate();
int32_t onCheats();
int32_t RulesPage_1();
int32_t RulesPage_2();
int32_t RulesPage_3();
int32_t RulesPage_4();
int32_t RulesPage_5();
int32_t RulesPage_6();
int32_t RulesPage_7();
int32_t RulesPage_8();
int32_t RulesPage_9();
int32_t RulesPage_10();
int32_t onAnimationRules();
int32_t onWeaponRules();
int32_t onHeroRules();
int32_t onComboRules();
int32_t onItemRules();
int32_t onEnemyRules();
int32_t onFixesRules();
int32_t onMiscRules();
int32_t d_line_proc(int32_t msg, DIALOG *d, int32_t c);
const char *subscrtypelist(int32_t index, int32_t *list_size);
int32_t InitPage_1();
int32_t InitPage_2();
int32_t InitPage_3();
int32_t InitPage_4();
int32_t InitPage_5();
int32_t InitPage_6();
int32_t InitPage_7();
int32_t InitPage_8();
int32_t onInit();
int32_t onItemProps();
int32_t onSubscreen();

void get_cset(int32_t dataset,int32_t row,RGB *pal);
void draw_edit_dataset_specs(int32_t index,int32_t copy);
void init_colormixer();
void colormixer(int32_t color,int32_t gray,int32_t ratio);
bool do_x_button(BITMAP *dest, int32_t x, int32_t y);
bool do_question_button(BITMAP *dest, int32_t x, int32_t y);
bool edit_dataset(int32_t dataset);
int32_t pal_index(RGB *pal,RGB c);
bool grab_dataset(int32_t dataset);

//byte cset_hold[15][16*3];
//byte cset_hold_cnt;
//bool cset_ready = false;
//int32_t cset_count,cset_first;
//PALETTE pal,undopal;

//byte rc[16] = {253,248,0,0,0,0,0,246,247,249,250,251,252,240,255,254};

void undo_pal();
void calc_dark(int32_t first);
void edit_cycles(int32_t level);
void draw_cset_proc(DIALOG *d);
int32_t d_cset_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t d_dummy_proc(int32_t msg,DIALOG *d,int32_t c);

//byte mainpal_csets[30]    = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
//byte levelpal_csets[26]   = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
//byte levelpal2_csets[26]  = { 2,3,4,9,2,0,1,2,3,4, 5, 6, 7,       15,15,15,15, 8,  9,9,9,9,9,9,9,9 };
//byte spritepal_csets[30]  = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
//byte spritepal2_csets[30] = { 15,16,17,18,19,20,21,22,23,24,25,26,27,28,29, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };

int32_t EditColors(const char *caption,int32_t first,int32_t count,byte *label);
//int32_t onColors_Main();
//int32_t onColors_Levels();
//int32_t onColors_Sprites();
int32_t d_maptile_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onMapStyles();
int32_t d_misccolors_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onMiscColors();
int32_t onTestBox();


int32_t d_ticsedit_proc(int32_t msg,DIALOG *d,int32_t c);

// ****  Palette cycling  ****

//static int32_t palclk[3];
//static int32_t palpos[3];

void reset_pal_cycling();
void cycle_palette();

/********************/
/******  Help  ******/
/********************/

void doHelp(int32_t bg,int32_t fg);
int32_t onZstringshelp();
int32_t onHelp();
int32_t onZScripthelp();
int32_t edit_layers(mapscr* tempscr);
void autolayer(mapscr* tempscr, int32_t layer, int32_t al[6][3]);
int32_t onLayers();

// **** Timers ****

//volatile int32_t lastfps=0;
//volatile int32_t framecnt=0;
extern volatile int32_t myvsync;

void myvsync_callback();
void fps_callback();

extern bool update_hw_pal;
void update_hw_screen(bool force);

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
int32_t get_currdmap();
int32_t current_item(int32_t item_type);
int32_t current_item_power(int32_t item_type);
//int32_t current_item_id(int32_t item_type, bool checkmagic, bool jinx_check);

bool can_use_item(int32_t item_type, int32_t item);
bool has_item(int32_t item_type, int32_t it);
int32_t get_bmaps(int32_t si);

bool no_subscreen();

extern int32_t Awpn, Bwpn, Bpos, Xwpn, Ywpn;
extern sprite_list Sitems;

int32_t main(int32_t argc,char **argv);
int32_t d_nbmenu_proc(int32_t msg,DIALOG *d,int32_t c);
void center_zquest_dialogs();
void center_zscript_dialogs();
void animate_coords();
void do_animations();
int32_t onZQVidMode();
bool is_zquest();
int32_t save_config_file();
int32_t d_timer_proc(int32_t msg, DIALOG *d, int32_t c);
void check_autosave();

void debug_pos(size_and_pos const& pos, int color = 0xED);
void textbox_out(BITMAP* dest, FONT* font, int x, int y, int fg, int bg, char const* str, int align, size_and_pos* dims = nullptr);
void highlight_sqr(BITMAP* dest, int color, int x, int y, int w, int h, int thick = 2);
void highlight_sqr(BITMAP* dest, int color, size_and_pos const& rec, int thick = 2);
void highlight_frag(BITMAP* dest, int color, int x1, int y1, int w, int h, int fw, int fh, int thick = 2);
void highlight_frag(BITMAP* dest, int color, size_and_pos const& rec, int thick = 2);
void draw_ttip(BITMAP* dest);
void draw_ttip2(BITMAP* dest);
void draw_ttips();
void update_tooltip(int32_t x, int32_t y, size_and_pos const& sqr, char const* tipmsg, double scale = 1);
void update_tooltip(int32_t x, int32_t y, int32_t trigger_x, int32_t trigger_y, int32_t trigger_w, int32_t trigger_h, char const* tipmsg, int fw = -1, int fh = -1, double scale = 1);
void update_tooltip2(int32_t x, int32_t y, size_and_pos const& sqr, char const* tipmsg, double scale = 1);
void update_tooltip2(int32_t x, int32_t y, int32_t trigger_x, int32_t trigger_y, int32_t trigger_w, int32_t trigger_h, char const* tipmsg, int fw = -1, int fh = -1, double scale = 1);
void clear_tooltip();
void clear_tooltip2();
void ZQ_ClearQuestPath();
void cflag_help(int32_t id);
void ctype_help(int32_t id);
void questrev_help();
void questminrev_help();

//throw together midi_strict hack
#ifdef ALLEGRO_LINUX_GP2X
extern int32_t midi_strict;
#endif

#define mDOOR_UP         0x0001                               // only dungeons use this
#define mDOOR_DOWN       0x0002                               //        ''
#define mDOOR_LEFT       0x0004                               //        ''
#define mDOOR_RIGHT      0x0008                               //        ''

#define mITEM            0x0010                               // item (main screen)
#define mSPECIALITEM           0x0020                               // special item (underground)
#define mNEVERRET        0x0040                               // enemy never returns
#define mTMPNORET        0x0080                               // enemies don't return until you leave the dungeon

#define mLOCKBLOCK       0x0100                               // if the lockblock on the screen has been triggered
#define mBOSSLOCKBLOCK   0x0200                               // if the bosslockblock on the screen has been triggered
#define mCHEST           0x0400                               // if the unlocked check on this screen has been opened
#define mLOCKEDCHEST     0x0800                               // if the locked chest on this screen has been opened

#define mBOSSCHEST       0x1000                               // if the boss chest on this screen has been opened
#define mSECRET          0x2000                               // only overworld and caves use this
#define mVISITED         0x4000                               // only overworld uses this
#define mLIGHTBEAM       0x8000                               // light beam triggers completed

#define mMAXIND          16
#endif
/* end */
