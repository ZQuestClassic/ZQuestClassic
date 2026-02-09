#ifndef ZQUEST_H_
#define ZQUEST_H_

#include <string>
#include <vector>
#include "base/initdata.h"
#include "midi.h"
#include "gui/jwin.h"
#include <base/new_menu.h>
#include "base/jwinfsel.h"
#include "sound/zcmusic.h"
#include "sound/zcmixer.h"
#include "sprite.h"
#include "gamedata.h"
#include "parser/parserDefs.h"
#include "base/zfix.h"
#include "base/fonts.h"
#include "base/cpool.h"
#include "base/misctypes.h"
#include "zq/zq_class.h"
#include "items.h"

#define  INTERNAL_VERSION  0xA721

// the following are used by both zelda.cc and zquest.cc

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

#define MAXARROWS 8
#define SHADOW_DEPTH 2

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

extern int32_t RulesetDialog;

extern bool disable_saving, OverwriteProtection;
extern int32_t TileProtection, ComboProtection;

extern uint8_t PreFillTileEditorPage, PreFillComboEditorPage;
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

void zq_exit(int code);

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

extern int32_t mapscreen_x, mapscreen_y, showedges, showallpanels;
extern int32_t mouse_scroll_h;
extern int32_t tooltip_timer, tooltip_maxtimer;

extern BITMAP* asset_icons_bmp;
extern BITMAP* asset_engravings_bmp;
extern BITMAP* asset_mouse_bmp;
extern BITMAP* asset_select_bmp;
extern BITMAP* asset_arrows_bmp;
extern MIDI* asset_tunes_midi;
extern PALETTE asset_pal;

extern int32_t lens_hint_weapon[MAXWPNS][5];                    //aclk, aframe, dir, x, y
extern RGB_MAP* zq_rgb_table;
extern MIDI *song;
extern BITMAP *mapscreenbmp, *screen2, *mouse_bmp[MOUSE_BMP_MAX][4], *mouse_bmp_1x[MOUSE_BMP_MAX][4], *icon_bmp[ICON_BMP_MAX][4], *flag_bmp[16][4], *panel_button_icon_bmp[m_menucount][4], *select_bmp[2],*dmapbmp_small, *dmapbmp_large;
extern BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushscreen;
extern byte *colordata, *trashbuf;
extern comboclass *combo_class_buf;
extern wpndata  *wpnsbuf;
extern guydata  *guysbuf;
extern item_drop_object    item_drop_sets[MAXITEMDROPSETS];
extern midi_info Midi_Info;
extern bool zq_showpal;
extern bool zoomed_minimap;
extern int32_t scrolling_destdmap, cur_dmap;


extern int32_t alignment_arrow_timer;
extern int32_t  Flip,Combo,CSet;
extern int32_t  Flags,Flag,menutype;
extern int32_t MouseScroll, SavePaths, CycleOn, NoScreenPreview,
	DisableLPalShortcuts,DisableCompileConsole,skipLayerWarning,numericalFlags,DragCenterOfSquares, SmartFFCPlacement;
extern uint8_t InvalidBG;
extern int32_t Frameskip, RequestedFPS, zqColorDepth, zqUseWin32Proc;
extern bool ShowFPS, SaveDragResize, DragAspect, SaveWinPos;
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
extern bool quit, saved, autosaved;
extern bool __debug;
extern int32_t LayerMaskInt[7];
extern int32_t CurrentLayer;
extern int32_t DuplicateAction[4];
extern int32_t OnlyCheckNewTilesForDuplicates;
/*
  , HorizontalDuplicateAction;
  int32_t VerticalDuplicateAction, BothDuplicateAction;
  */
extern word msg_count;
extern word door_combo_set_count;
extern int32_t LeechUpdate;
extern int32_t LeechUpdateTiles;
extern int32_t SnapshotFormat;
extern byte SnapshotScale;
extern byte Color;

extern byte compile_tune;

extern ZCMUSIC *zcmusic;
extern ZCMIXER* zcmixer;
extern volatile int32_t myvsync;

extern int32_t fill_type;

extern bool first_save;
extern char *filepath,*midipath,*datapath,*imagepath,*tmusicpath,*last_timed_save;
extern char temppath[4096];
extern std::string helpstr;

extern int32_t rules_page, rules_pages;
extern int32_t init_page, init_pages;
extern int32_t doorcombos_page, doorcombos_pages;
extern bool rules_done, save_rules;
extern bool init_done, save_init;
extern bool doorcombos_done, save_doorcombos;

extern int32_t window_width, window_height;
void set_last_timed_save(char const* buf);
void mark_save_dirty();
void loadlvlpal(int32_t level);

bool get_debug();
void set_debug(bool d);


// quest data
extern zquestheader header;
extern zctune customtunes[MAXCUSTOMMIDIS];
extern int32_t msg_strings_size;
extern ZCHEATS zcheats;
extern byte use_cheats;
extern byte use_tiles;
extern char palnames[MAXLEVELS][17];

extern char fontsdat_sig[52];

extern uint8_t console_is_open;

// qst.cc helpers
bool bad_version(int32_t ver);
zfix HeroModifiedX();
zfix HeroModifiedY();

extern NewMenu colors_menu;

void toggle_combo_cols_mode();

void rebuild_trans_table();
void rebuild_string_list();

int32_t onResetTransparency();
int32_t d_vsync_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_nbmenu_proc(int32_t msg,DIALOG *d,int32_t c);

void savesometiles(const char *prompt,int32_t initialval);
void writesometiles_to(const char *prompt,int32_t initialval);

void savesomecombos(const char *prompt,int32_t initialval);
void writesomecombos_to(const char *prompt,int32_t initialval);
void writesomecombos(const char *prompt,int32_t initialval);
void loadcombopack(const char *prompt,int32_t initialval);

void savesomecomboaliases(const char *prompt,int32_t initialval);
void writesomecomboaliases_to(const char *prompt,int32_t initialval);

int32_t writesomedmaps(PACKFILE *f, int32_t first, int32_t last, int32_t max);
int32_t readsomedmaps(PACKFILE *f);
//need readsomedmaps_to, with a starting index, in the future
void savesomedmaps(const char *prompt,int32_t initialval);

void do_importdoorset(const char *prompt,int32_t initialval);
void do_exportdoorset(const char *prompt,int32_t initialval);

void update_combo_cycling();

enum {dm_normal, dm_alias = 3, dm_cpool, dm_auto, dm_max};

int32_t onSelectSFX();
int32_t onOptions();
void fix_drawing_mode_menu();
int32_t onDrawingMode();
int32_t onDrawingModeNormal();
int32_t onDrawingModeAlias();
int32_t onDrawingModePool();
int32_t onDrawingModeAuto();
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

int32_t playZCForever();
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
int32_t load_the_map(bool skipmenu);
void close_the_map();
int32_t onViewMap();

char *pathstr(byte path[]);
void drawpanel(int32_t panel);
void refresh(int32_t flags, bool update = false);
void select_scr();
void select_combo(int32_t list);
void update_combobrush();
void change_autocombo_height(int32_t change);
void draw(bool);
int32_t d_wflag_proc(int32_t msg,DIALOG *d,int32_t c);

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
int32_t onSecretF();
int32_t onSecretCombo();
int32_t onUnderCombo();
int32_t onCompileScript();
int32_t onSlotPreview();

typedef struct weapon_struct
{
    char *s;
    int32_t i;
} weapon_struct;

extern weapon_struct biw[MAXWPNS];

int32_t set_comboaradio(byte layermask);
extern int32_t alias_origin;
void draw_combo_alias_thumbnail(BITMAP *dest, combo_alias const* combo, int32_t x, int32_t y, int32_t size);

void build_biw_list();

int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onDoors();
int32_t onScrData();
int32_t onScreenScript();
const char *msgslist(int32_t index, int32_t *list_size);

const char *gotomaplist(int32_t index, int32_t *list_size);
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
int32_t onZoomIn();
int32_t onZoomOut();
int32_t onFullScreen();
int32_t isFullScreen();
int32_t onToggleGrid(bool color);
int32_t onToggleShowScripts();
int32_t onToggleScreenGrid();
int32_t onToggleRegionGrid();
int32_t onToggleCurrentScreenOutline();
int32_t onToggleShowSquares();
int32_t onToggleShowFFCs();
int32_t onToggleShowInfo();
int32_t onToggleHighlightLayer();

int32_t d_ndroplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_idroplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_nidroplist_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t onTriPieces();

int32_t d_hexedit_proc(int32_t msg,DIALOG *d,int32_t c);
void drawdmap(int32_t dmap);
void drawdmap_screen(int32_t x, int32_t y, int32_t w, int32_t h, int32_t dmap);
int32_t d_dropdmaplist_proc(int32_t msg,DIALOG *d,int32_t c);
void drawxmap(ALLEGRO_BITMAP* dest, int32_t map, int32_t xoff, bool large, int dx, int dy);

int32_t onDmaps();

int32_t onDoorCombos();

int32_t onMusic();
int32_t onMidis();

const char *warptypelist(int32_t index, int32_t *list_size);

int32_t onTileWarpIndex(int32_t index);
int32_t onTileWarp();
int32_t onSideWarp();

const char *dirlist(int32_t index, int32_t *list_size);

int32_t onPath();

int32_t onInfoTypes();
int32_t onShopTypes();

int32_t onBottleTypes();
int32_t onBottleShopTypes();
int32_t onSaveMenus();

int32_t count_item_drop_sets();
int32_t onItemDropSets();

int32_t d_warplist_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_wclist_proc(int32_t msg,DIALOG *d,int32_t c);
const char *wclist(int32_t index, int32_t *list_size);

int32_t onWarpRings();

typedef struct enemy_struct
{
    char *s;
    int32_t i;
} enemy_struct;

#define zqMAXGUYS    gDUMMY1

extern enemy_struct bie[eMAXGUYS];
void build_bie_list(bool hide);
int32_t efrontfacingtile(int32_t id);

int32_t onEnemies();

int32_t onHeader();

int32_t PickRuleset();
int32_t PickRuleTemplate();
int32_t onCheats();
int32_t onInit();

void get_cset(int32_t dataset,int32_t row,RGB *pal);
void draw_edit_dataset_specs(int32_t index,int32_t copy);
void init_colormixer();
void colormixer(int32_t color,int32_t gray,int32_t ratio);
bool do_x_button(BITMAP *dest, int32_t x, int32_t y);
bool do_question_button(BITMAP *dest, int32_t x, int32_t y);
bool edit_dataset(int32_t dataset);
int32_t pal_index(RGB *pal,RGB c);
bool grab_dataset(int32_t dataset);

void undo_pal();
void calc_dark(int32_t first);
void edit_cycles(int32_t level);
void draw_cset_proc(DIALOG *d);
int32_t d_cset_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t d_dummy_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t EditColors(const char *caption,int32_t first,int32_t count,byte *label);
int32_t onMapStyles();
int32_t onMiscColors();

int32_t d_ticsedit_proc(int32_t msg,DIALOG *d,int32_t c);

void reset_pal_cycling();
void cycle_palette();

int32_t onZstringshelp();
int32_t onHelp();
int32_t onLayers();

extern volatile int32_t myvsync;

void myvsync_callback();
void fps_callback();

extern bool update_hw_pal;
void update_hw_screen();

extern volatile int32_t framecnt;
void anim_hw_screen();
void custom_vsync();
void switch_out();
void switch_in();
void Z_eventlog(const char *format,...);
void Z_scripterrlog(const char * const format,...);
void zprint(const char * const format,...);
void zprint2(const char * const format,...);
int32_t get_currdmap();

bool can_use_item(int32_t item_type, int32_t item);
bool has_item(int32_t item_type, int32_t it);
int32_t get_bmaps(int32_t si);

bool no_subscreen();

extern ButtonItemData Awpn, Bwpn, Xwpn, Ywpn;

int32_t main(int32_t argc,char **argv);
int32_t d_nbmenu_proc(int32_t msg,DIALOG *d,int32_t c);
void center_zquest_dialogs();
void center_zscript_dialogs();
void animate_coords();
void do_animations();
int32_t onZQVidMode();
int32_t save_config_file();
int32_t d_timer_proc(int32_t msg, DIALOG *d, int32_t c);
void check_autosave();

void textbox_out(BITMAP* dest, FONT* font, int x, int y, int fg, int bg, char const* str, int align, size_and_pos* dims = nullptr);
void highlight_sqr(BITMAP* dest, int color, int x, int y, int w, int h, int thick = 2);
void highlight_sqr(BITMAP* dest, int color, size_and_pos const& rec, int thick = 2);
void highlight_frag(BITMAP* dest, int color, int x1, int y1, int w, int h, int fw, int fh, int thick = 2);
void highlight_frag(BITMAP* dest, int color, size_and_pos const& rec, int thick = 2);
void highlight(BITMAP* dest, size_and_pos& hl);
std::pair<int, int> get_box_text_size(char const* tipmsg, double txscale);
void draw_box(BITMAP* destbmp, size_and_pos* pos, char const* tipmsg, double txscale = 1);
void update_tooltip(int32_t x, int32_t y, size_and_pos const& sqr, char const* tipmsg, double scale = 1);
void update_tooltip(int32_t x, int32_t y, int32_t trigger_x, int32_t trigger_y, int32_t trigger_w, int32_t trigger_h, char const* tipmsg, int fw = -1, int fh = -1, double scale = 1);
void ZQ_ClearQuestPath();
void cflag_help(int32_t id);
void ctype_help(int32_t id);
void questrev_help();
void questminrev_help();

//throw together midi_strict hack
#ifdef ALLEGRO_LINUX_GP2X
extern int32_t midi_strict;
#endif

#endif
/* end */
