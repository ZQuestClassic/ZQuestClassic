#ifndef _ZQUEST_H_
#define _ZQUEST_H_

#include "midi.h"
#include "jwin.h"
#include "jwinfsel.h"

#define  INTERNAL_VERSION  0xA721

// the following are used by both zelda.cc and zquest.cc

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

#define MAXARROWS 8
#define SHADOW_DEPTH 2
#define MSG_VSYNC MSG_USER

#define edc  208 //246
#define edi  209 //247
//#define ed0  248
//#define ed1  249
//#define ed7  250
//#define ed11 251
//#define ed14 252
//#define ed15 253
//#define FLASH 243

extern bool cancelgetnum;
extern bool disable_saving, OverwriteProtection;
extern int zq_scale;

enum
{
  m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_layers,
  m_menucount
};

enum {MOUSE_BMP_NORMAL=0, MOUSE_BMP_POINT_BOX, MOUSE_BMP_FLAG, MOUSE_BMP_BOX, MOUSE_BMP_SWORD, MOUSE_BMP_POTION, MOUSE_BMP_WAND, MOUSE_BMP_LENS, MOUSE_BMP_GLOVE, MOUSE_BMP_HOOKSHOT, MOUSE_BMP_BLANK, MOUSE_BMP_MAX};
enum {ICON_BMP_ITEM=0, ICON_BMP_WARP, ICON_BMP_RETURN, ICON_BMP_WARPDEST, ICON_BMP_FLAG, ICON_BMP_MAX};


extern int CSET_SIZE;
extern int CSET_SHFT;

void update_combo_cycling();

extern int coord_timer, coord_frame;
extern int blackout_color, zq_screen_w, zq_screen_h, minimap_x, minimap_y;
extern int combolist_w, combolist_h, combolist_x, combolist_y, mapscreen_x;
extern int mapscreen_y, mapscreensize, showedges, showallpanels, panel_1_x;
extern int panel_1_y, panel_2_x, panel_2_y, panel_3_x, panel_3_y, panel_4_x;
extern int panel_4_y, panel_5_x, panel_5_y, panel_6_x, panel_6_y, panel_7_x;
extern int panel_7_y, showpanelcomboscrollers, comboscroller_x;
extern int comboscroller_y, comboscroller_w, comboscroller_h;
extern int jwin_pal[jcMAX];

extern bool canfill;                                        //to prevent double-filling (which stops undos)
extern bool resize_mouse_pos;                               //for eyeball combos
extern int lens_hint_item[MAXITEMS][2];                     //aclk, aframe
extern int lens_hint_weapon[MAXWPNS][5];                    //aclk, aframe, dir, x, y
extern int mode, switch_mode, orig_mode;
extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table;
extern char *datafile_str;
extern DATAFILE *zcdata, *fontsdata;
extern MIDI *song;
extern FONT *zfont,*z3font,*sfont,*lfont,*lfont_l,*pfont, *spfont;
extern BITMAP *menu1,*menu3, *mapscreenbmp, *combo_bmp, *tmp_scr, *screen2, *mouse_bmp[MOUSE_BMP_MAX][2], *icon_bmp[ICON_BMP_MAX][4], *select_bmp[2],*dmapbmp_small, *dmapbmp_large;
extern BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushscreen; //, *brushshadowbmp;
extern byte *tilebuf,*colordata, *trashbuf;
extern newcombo *combobuf;
extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern guydata  *guysbuf;
extern newcombo curr_combo;
extern PALETTE RAMpal;
extern midi_info Midi_Info;
extern bool zq_showpal;
extern bool combo_cols;


extern int alignment_arrow_timer;
extern int  Flip,Combo,CSet,First;
extern int  Flags,Flag,menutype;
extern int MouseScroll, SavePaths, CycleOn;
extern bool Vsync, ShowFPS;
extern int ComboBrush;                                      //show the brush instead of the normal mouse
extern int ComboBrushPause;                                 //temporarily disable the combo brush
extern int BrushPosition;                                   //top left, middle, bottom right, etc.
extern int FloatBrush;                                      //makes the combo brush float a few pixels up and left
//complete with shadow
extern int OpenLastQuest;                                   //makes the program reopen the quest that was
//open at the time you quit
extern int ShowMisalignments;                               //makes the program display arrows over combos that are
//not aligned with the next screen.
extern int AnimationOn;                                     //animate the combos in zquest?
extern int AutoBackup;                                      //use auto-backup feature?
extern int ImportMapBias;                                   //tells what has precedence on map importing
extern int BrushWidth, BrushHeight;
extern bool quit,saved;
extern bool debug;
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
extern int memrequested;
extern byte Color;

extern int fill_type;

extern char *filepath,*temppath,*midipath,*datapath,*imagepath,*tmusicpath;
extern char *helpbuf;

extern int rules_page, rules_pages;
extern int init_page, init_pages;
extern int doorcombos_page, doorcombos_pages;
extern bool rules_done, save_rules;
extern bool init_done, save_init;
extern bool doorcombos_done, save_doorcombos;

void loadlvlpal(int level);

// quest data
extern zquestheader header;
extern byte         quest_rules[QUESTRULES_SIZE];
extern byte         midi_flags[MIDIFLAGS_SIZE];
extern word         map_count;
extern miscQdata    misc;
extern mapscr       *TheMaps;
extern dmap         *DMaps;
extern MsgStr       *MsgStrings;
extern DoorComboSet *DoorComboSets;
extern music        *customMIDIs;
extern ZCHEATS      zcheats;
extern byte         use_cheats;
extern byte         use_tiles;
extern zinitdata    zinit;
extern char         palnames[256][17];
extern quest_template QuestTemplates[MAXQTS];
extern byte         *undotilebuf;
extern newcombo     *undocombobuf;
extern word animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tiles

extern char   fontsdat_sig[52];

// qst.cc helpers

void crash();
void crash2();
void crash3();
void crash4();
bool bad_version(int ver);
fix LinkModifiedX();
fix LinkModifiedY();

extern MENU colors_menu[];

void rebuild_trans_table();

int onB();
int d_vsync_proc(int msg,DIALOG *d,int c);
int d_nbmenu_proc(int msg,DIALOG *d,int c);
int getnumber(char *prompt,int initialval);
int gethexnumber(char *prompt,int initialval);

void update_combo_cycling();

int onOptions();
int onReTemplate();
int onUndo();
int onCopy();
int onPaste();
int onPasteAll();
int onPasteToAll();
int onPasteAllToAll();
int onDelete();
int onDeleteMap();
int onD();
int onIncMap();
int onDecMap();
int onDefault_Pals();
int onDefault_Combos();
int onDefault_Sprites();
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
int onLeft();
int onRight();
int onUp();
int onDown();
int onPgUp();
int onPgDn();
int onPlus();
int onMinus();
bool getname(char *prompt,char *ext,char *def,bool usefilename);
bool getname_nogo(char *prompt,char *ext,EXT_LIST *list,char *def,bool usefilename);
//bool getname_nogo(char *prompt,char *ext,char *def,bool usefilename);
int playMIDI();
int playTune();
int stopMIDI();

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
extern int  px,py,pblack,pwhite;
extern double scale;
extern bool vp_showpal, vp_showsize, vp_center;

inline int pal_sum(RGB p) { return p.r + p.g + p.b; }

void get_bw(RGB *pal,int &black,int &white);
void draw_bw_mouse(int white, int old_mouse, int new_mouse);
int load_the_pic();
int onViewPic();
int load_the_map();
int onViewMap();

//static char *dirstr[4] = {"up","down","left","right"};
//char _pathstr[25]="up,up,up,up";

char *pathstr(byte path[]);
void drawpanel(int panel);
void refresh(int flags);
void select_scr();
void select_combo();
void update_combobrush();
void draw();
void replace(int c);
void draw_block(int start,int w,int h);
void fill(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal);
void fill2(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal);
int d_wflag_proc(int msg,DIALOG *d,int c);

/**************************/
/*****     Mouse      *****/
/**************************/

void doxypos(byte &px,byte &py,int color,int mask);
void doflags();
void peek(int c);
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
int d_sel_scombo_proc(int msg, DIALOG *d, int c);
void select_scombo(int &pos);
char *flaglist(int index, int *list_size);
int select_data(char *prompt,int index,char *(proc)(int,int*), FONT *title_font);
int select_data(char *prompt,int index,char *(proc)(int,int*),char *b1, char *b2, FONT *title_font);
int select_flag(int &f);
int d_scombo_proc(int msg,DIALOG *d,int c);
int d_scombo2_proc(int msg, DIALOG *d, int c);
int onSecretF();
int onSecretCombo();
int onUnderCombo();

typedef struct item_struct
{
  char *s;
  int i;
} item_struct;

extern item_struct bii[iMax];


typedef struct weapon_struct
{
  char *s;
  int i;
} weapon_struct;

extern weapon_struct biw[wMAX];

void build_bii_list(bool usenone);
char *itemlist(int index, int *list_size);
int select_item(char *prompt,int item,int allow_edit);


void build_biw_list();
char *weaponlist(int index, int *list_size);
int select_weapon(char *prompt,int weapon);

//char *doors_string[8]={"wall","passage","locked","shutter","bombable","walk thru","1-way shutter","boss"};
char *doorslist(int index, int *list_size);
void edit_door(int side);
int d_combo_proc(int msg,DIALOG *d,int c);
char *doorcombosetlist(int index, int *list_size);
int onDoors();
int onScrData();
char *flaglist(int index, int *list_size);
char *itemslist(int index, int *list_size);
char *weaponslist(int index, int *list_size);
char *roomslist(int index, int *list_size);
char *guyslist(int index, int *list_size);
char *msgslist(int index, int *list_size);

//static char number_str_buf[32];
//int number_list_size=1;
//bool number_list_zero=false;

char *numberlist(int index, int *list_size);

//static char dmap_str_buf[37];
//int dmap_list_size=1;
//bool dmap_list_zero=false;

char *dmaplist(int index, int *list_size);

char *hexnumlist(int index, int *list_size);
char *maplist(int index, int *list_size);
char *gotomaplist(int index, int *list_size);
char *midilist(int index, int *list_size);
char *custommidilist(int index, int *list_size);
char *levelnumlist(int index, int *list_size);
int onMapCount();
int onGotoMap();
int onFlags();
int onItem();
int onRType();
int onGuy();
int onString();
int onEndString();
int onCatchall();
int onScreenPalette();
int onFullScreen();
int isFullScreen();

//bool edit_ins_mode=true;

void put_msg_str(char *s,int x,int y,int fg,int bg,int pos);
int d_msg_edit_proc(int msg,DIALOG *d,int c);

//char msgbuf[73];

int d_nlist_proc(int msg,DIALOG *d,int c);
int d_nilist_proc(int msg,DIALOG *d,int c);
void editmsg(int index);
int strlist_del();
void fix_string(word &str, byte index);
int onStrings();

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
int d_grid_proc(int msg,DIALOG *d,int c);
void drawxmap(int map,int xoff,bool large);
int d_xmaplist_proc(int msg,DIALOG *d,int c);

//int xmapspecs[4] = {0,0,84+58,118+32+5};

int onXslider(void *dp3,int d2);

//char *dmaptype_str[dmMAX] = { "Dngn","Overw","Cave","BS-Ow" };

char *typelist(int index, int *list_size);
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

/************************************/
/**********  onDoorCombos  **********/
/************************************/

void fix_dcs(int index, int dir);
void editdoorcomboset(int index);

char *doorcombosetlist(int index, int *list_size);

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

void edit_midi(int i);
int d_midilist_proc(int msg,DIALOG *d,int c);
int onMidis();

/*******************************/
/****  onTest/onTestOptions  ***/
/*******************************/

int onTest();
int onTestOptions();

/*******************************/
/**********  onWarp  ***********/
/*******************************/

char *warptypelist(int index, int *list_size);

//int warpdmapxy[6] = {188,126,188,100,188,112};

int d_warpdestsel_proc(int msg,DIALOG *d,int c);
int onWarps();
int onTileWarp();
int onTimedWarp();
int onItemWarp();
int onSideWarp();
void EditWindWarp(int level);
int onWhistle();

/*******************************/
/*********** onPath ************/
/*******************************/

char *dirlist(int index, int *list_size);

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

/********************************/
/********* onWarpRings **********/
/********************************/

//int curr_ring;

void EditWarpRingScr(int ring,int index);
int d_warplist_proc(int msg,DIALOG *d,int c);
int d_wclist_proc(int msg,DIALOG *d,int c);
char *wclist(int index, int *list_size);

//int warpringdmapxy[8] = {160,106,160,80,160,92,160,144};

int select_warp();
void EditWarpRing(int ring);
int onWarpRings();

/********************************/
/********** onEnemies ***********/
/********************************/

char *pattern_list(int index, int *list_size);
int onPattern();
int onEnemyFlags();
char *enemy_viewer(int index, int *list_size);

typedef struct enemy_struct
{
  char *s;
  int i;
} enemy_struct;

//enemy_struct bie[eMAXGUYS];
//enemy_struct ce[100];
//int enemy_type=0,bie_cnt=-1,ce_cnt;

//enemy_struct big[eMAXGUYS];
//enemy_struct cg[100];
//int big_cnt=-1,cg_cnt;

void build_bie_list();
char *enemylist(int index, int *list_size);
int select_enemy(char *prompt,int enemy);

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
int onRules();
int d_line_proc(int msg, DIALOG *d, int c);
//char *subscrtype_str[sstMAX] = { "Original","New Subscreen","Revision 2" };
char *subscrtypelist(int index, int *list_size);
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

int EditColors(char *caption,int first,int count,byte *label);
int onColors_Main();
int onColors_Levels();
int onColors_Sprites();
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
int onHelp();
int edit_layers(mapscr* tempscr);
void autolayer(mapscr* tempscr, int layer, int al[6][3]);
int findblankcombo();
int onLayers();

// **** Timers ****

//volatile int lastfps=0;
//volatile int framecnt=0;
//volatile int myvsync = 0;

void myvsync_callback();
void fps_callback();
void rebuild_trans_table();

/********************/
/******  MAIN  ******/
/********************/

void custom_vsync();
int main(int argc,char **argv);
int d_nbmenu_proc(int msg,DIALOG *d,int c);
void center_zquest_dialogs();
void animate_coords();
void do_animations();
#endif
/* end */
