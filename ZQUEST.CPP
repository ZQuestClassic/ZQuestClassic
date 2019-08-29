//system_pal();
//system_pal();
//jwin_alert("Title","Line1","Line2","Line3","Button1 Text","Button2 Text",Button1_key,Button2_key,font);
//game_pal();

/*
  int x=gui_mouse_x();
  int y=gui_mouse_y();
  double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
  double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
  int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
  int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
  //  if(isinRect(x,y,0,16,255,191))
  if(isinRect(x,y,startxint,startyint,
  int(startx+(256*mapscreensize)-1),
  int(starty+(176*mapscreensize)-1)))

  int cx=(x-startxint)/int(16*mapscreensize);
  int cy=(y-startyint)/int(16*mapscreensize);
  int c=(cy*16)+cx;
  */

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

/*
  #define  INTERNAL_VERSION  0xA721
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <dir.h>

#include "zc_alleg.h"

#include <png.h>
#include <pngconf.h>

#include <loadpng.h>
#include <jpgalleg.h>

#include "gui.h"
#include "load_gif.h"
#include "save_gif.h"
#include "editbox.h"
#include "zq_misc.h"

#include "zquest.h"
#include "zquestdat.h"

// the following are used by both zelda.cc and zquest.cc
#include "zdefs.h"
#include "tiles.h"
#include "colors.h"
#include "qst.h"
#include "zsys.h"
#include "zcmusic.h"

#include "midi.h"
#include "sprite.h"
#include "items.h"
#include "fontsdat.h"
#include "jwinfsel.h"
#include "zq_class.h"
#include "subscr.h"
#include "zq_subscr.h"
#include "ffscript.h"


#ifndef ALLEGRO_DOS
#include "zqscale.h"
#endif


#ifdef ALLEGRO_DOS
static char *data_path_name   = "dos_data_path";
static char *midi_path_name   = "dos_midi_path";
static char *image_path_name  = "dos_image_path";
static char *tmusic_path_name = "dos_tmusic_path";
static char *last_quest_name  = "dos_last_quest";
static char *qtname_name      = "dos_qtname%d";
static char *qtpath_name      = "dos_qtpath%d";
#elif defined(ALLEGRO_WINDOWS)
static char *data_path_name   = "win_data_path";
static char *midi_path_name   = "win_midi_path";
static char *image_path_name  = "win_image_path";
static char *tmusic_path_name = "win_tmusic_path";
static char *last_quest_name  = "win_last_quest";
static char *qtname_name      = "win_qtname%d";
static char *qtpath_name      = "win_qtpath%d";
#elif defined(ALLEGRO_LINUX)
static char *data_path_name   = "linux_data_path";
static char *midi_path_name   = "linux_midi_path";
static char *image_path_name  = "linux_image_path";
static char *tmusic_path_name = "linux_tmusic_path";
static char *last_quest_name  = "linux_last_quest";
static char *qtname_name      = "linux_qtname%d";
static char *qtpath_name      = "linux_qtpath%d";
#elif defined(ALLEGRO_MACOSX)
static char *data_path_name   = "macosx_data_path";
static char *midi_path_name   = "macosx_midi_path";
static char *image_path_name  = "macosx_image_path";
static char *tmusic_path_name = "macosx_tmusic_path";
static char *last_quest_name  = "macosx_last_quest";
static char *qtname_name      = "macosx_qtname%d";
static char *qtpath_name      = "macosx_qtpath%d";
#endif

#include "zq_init.h"
#include "zq_doors.h"
#include "zq_rules.h"
#include "zq_cset.h"

int startdmapxy[6] = {-1000, -1000, -1000, -1000, -1000, -1000};
bool cancelgetnum;
//int VidMode;

int playing_field_offset=0;
int passive_subscreen_height=56;
int passive_subscreen_offset=0;

bool disable_saving=false, OverwriteProtection;
int scale_arg;
int zq_scale;
bool halt=false;
bool show_sprites=true;

int CSET_SIZE = 16;
int CSET_SHFT = 4;
//editbox_data temp_eb_data;
/*
  #define CSET(x)         ((x)<<CSET_SHFT)
  #define csBOSS          14
  */

/*
  enum { m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_layers,
  m_menucount };
  */
void update_combo_cycling();
void update_freeform_combos();

/*
  #define MAXMICE 14
  #define MAXARROWS 8
  #define SHADOW_DEPTH 2
  */
int coord_timer=0, coord_frame=0;
int blackout_color, zq_screen_w, zq_screen_h, minimap_x, minimap_y, minimap_w, minimap_h;
int combolist_w, combolist_h, combolist_x, combolist_y, mapscreen_x;
int mapscreen_y, mapscreensize, showedges, showallpanels;
int panel_1_x, panel_1_y, panel_1_w, panel_1_h;
int panel_2_x, panel_2_y, panel_2_w, panel_2_h;
int panel_3_x, panel_3_y, panel_3_w, panel_3_h;
int panel_4_x, panel_4_y, panel_4_w, panel_4_h;
int panel_5_x, panel_5_y, panel_5_w, panel_5_h;
int panel_6_x, panel_6_y, panel_6_w, panel_6_h;
int panel_7_x, panel_7_y, panel_7_w, panel_7_h;
int panel_8_x, panel_8_y, panel_8_w, panel_8_h;
int panel_9_x, panel_9_y, panel_9_w, panel_9_h;
int showpanelcomboscrollers, comboscroller_x;
int comboscroller_y, comboscroller_w, comboscroller_h, combolist_ha;
int draw_mode=0;

int readsize, writesize;

item_drop_object item_drop_set[256];

int showxypos_x;
int showxypos_y;
int showxypos_color;
bool showxypos_icon=false;

int showxypos_cursor_x;
int showxypos_cursor_y;
bool showxypos_cursor_icon=false;

bool close_button_quit=false;
bool canfill=true;                                          //to prevent double-filling (which stops undos)
bool resize_mouse_pos=false;                                //for eyeball combos
int lens_hint_item[MAXITEMS][2];                            //aclk, aframe
int lens_hint_weapon[MAXWPNS][5];                           //aclk, aframe, dir, x, y
                                                            //int mode, switch_mode, orig_mode;
#ifdef ALLEGRO_MACOSX
int tempmode=GFX_AUTODETECT_FULLSCREEN;
#else
int tempmode=GFX_AUTODETECT;
#endif
RGB_MAP zq_rgb_table;
COLOR_MAP trans_table;
char *datafile_str;
DATAFILE *zcdata=NULL, *fontsdata=NULL;
MIDI *song=NULL;
FONT       *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gbzfont,
           *goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font;
BITMAP *menu1, *menu3, *mapscreenbmp, *combo_bmp, *tmp_scr, *screen2, *mouse_bmp[MOUSE_BMP_MAX][3], *icon_bmp[ICON_BMP_MAX][4], *select_bmp[2], *dmapbmp_small, *dmapbmp_large;
BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushscreen;//*brushshadowbmp;
byte *colordata=NULL, *trashbuf=NULL;
tiledata *newtilebuf;
newcombo *combobuf;
itemdata *itemsbuf;
wpndata  *wpnsbuf;
guydata  *guysbuf;
newcombo curr_combo;
PALETTE RAMpal;
midi_info Midi_Info;
bool zq_showpal=false;
bool combo_cols=false;
ffscript ffscripts[256][1000];

zinitdata zinit;

//int fadeclk = -1;
//int frame = 8;

int alignment_arrow_timer=0;
int  Flip=0,Combo=0,CSet=2,First=0;
int  Flags=0,Flag=1,menutype=m_block;
int MouseScroll, SavePaths, CycleOn, ShowGrid, GridColor, TileProtection;
bool Vsync, ShowFPS;
int ComboBrush;                                             //show the brush instead of the normal mouse
int ComboBrushPause;                                        //temporarily disable the combo brush
int BrushPosition;                                          //top left, middle, bottom right, etc.
int FloatBrush;                                             //makes the combo brush float a few pixels up and left
                                                            //complete with shadow
int OpenLastQuest;                                          //makes the program reopen the quest that was
                                                            //open at the time you quit
int ShowMisalignments;                                      //makes the program display arrows over combos that are
                                                            //not aligned with the next screen.
int AnimationOn;                                            //animate the combos in zquest?
int AutoBackup;                                             //use auto-backup feature?
int ImportMapBias;                                          //tells what has precedence on map importing
int BrushWidth=1, BrushHeight=1;
bool quit=false,saved=true;
bool __debug=false;
//bool usetiles=true;
byte LayerMask[2];                                          //determines which layers are on or off.  0-15
int LayerMaskInt[7];
int CurrentLayer=0;
int DuplicateAction[4];
int OnlyCheckNewTilesForDuplicates;
/*
  , HorizontalDuplicateAction;
  int VerticalDuplicateAction, BothDuplicateAction;
  */
word msg_count, qt_count;
int LeechUpdate;
int LeechUpdateTiles;
int SnapshotFormat;

int memrequested;
byte Color;
int jwin_pal[jcMAX];
int gui_colorset=0;

combo_alias combo_aliases[MAXCOMBOALIASES];
static int combo_apos=0;
static int combo_alistpos=0;

bool trip=false;

int fill_type=1;

char *filepath,*temppath,*midipath,*datapath,*imagepath,*tmusicpath;
char *helpbuf;

ZCMUSIC *zcmusic = NULL;
extern int prv_mode;
int prv_warp = 0;
int prv_twon = 0;
int ff_combo = 0;

int alias_origin=0;

void loadlvlpal(int level);
bool get_debug()
{
  //return false; //change later
  return __debug;
}

void set_debug(bool d)
{
  __debug=d;
  return;
}

// **** Timers ****

volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync = 0;

void myvsync_callback()
{
  ++myvsync;
}

END_OF_FUNCTION(myvsync_callback)



  // quest data
  zquestheader header;
byte         quest_rules[QUESTRULES_SIZE];
byte         midi_flags[MIDIFLAGS_SIZE];
byte         music_flags[MUSICFLAGS_SIZE];
word         map_count;
miscQdata    misc;
mapscr       *TheMaps;
dmap         *DMaps;
MsgStr       *MsgStrings;
//DoorComboSet *DoorComboSets;
zcmidi       *customMIDIs;
emusic       *enhancedMusic;
ZCHEATS      zcheats;
byte         use_cheats;
byte         use_tiles;
extern zinitdata zinit;
char         palnames[256][17];
quest_template QuestTemplates[MAXQTS];
//byte         *undotilebuf;
tiledata     *newundotilebuf;
newcombo     *undocombobuf;
word animated_combo_table[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table4[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos;
bool blank_tile_table[NEWMAXTILES];                         //keeps track of blank tiles
bool blank_tile_quarters_table[NEWMAXTILES*4];              //keeps track of blank tiles

char   fontsdat_sig[52];
char   zquestdat_sig[52];
char   qstdat_sig[52];

int gme_track=0;

// qst.cc helpers

bool bad_version(int ver)
{
  if(ver < 0x170)
    return true;

  return false;
}

fix LinkModifiedX()
{
  if (resize_mouse_pos)
  {
    return (fix)((gui_mouse_x()/mapscreensize)-((8*mapscreensize)-1));
  }
  else
  {
    return (fix)(gui_mouse_x()-7);
  }
}

fix LinkModifiedY()
{
  if (resize_mouse_pos)
  {
    return (fix)((gui_mouse_y()/mapscreensize)-((8*mapscreensize)-1));
  }
  else
  {
    return (fix)(gui_mouse_y()-7);
  }
}

#include "zq_misc.h"

static MENU import_menu[] =
{
  { "&Map",             onImport_Map,            NULL },
  { "&DMaps",           onImport_DMaps,          NULL },
  { "&Tiles",           onImport_Tiles,          NULL },
  { "Su&bscreen",       onImport_Subscreen,      NULL },
  { "&Palettes",        onImport_Pals,           NULL },
  { "&String Table",    onImport_Msgs,           NULL },
  { "&Combo Table",     onImport_Combos,         NULL },
  { "&Graphics Pack",   onImport_ZGP,            NULL },
  { "&Quest Template",  onImport_ZQT,            NULL },
  { "&Unencoded Quest", onImport_UnencodedQuest, NULL },
  { NULL }
};

static MENU export_menu[] =
{
  { "&Map",             onExport_Map,       NULL },
  { "&DMaps",           onExport_DMaps,     NULL },
  { "&Tiles",           onExport_Tiles,     NULL },
  { "Su&bscreen",       onExport_Subscreen, NULL },
  { "&Palettes",        onExport_Pals,      NULL },
  { "&String Table",    onExport_Msgs,      NULL },
  { "&Combo Table",     onExport_Combos,    NULL },
  { "&Graphics Pack",   onExport_ZGP,       NULL },
  { "&Quest Template",  onExport_ZQT,       NULL },
  { "&Unencoded Quest", onExport_UnencodedQuest, NULL },
  { NULL }
};

static MENU file_menu[] =
{
  { "&New",                   onNew,             NULL },
  { "&Open\tF3",              onOpen,            NULL },
  { "&Save\tF2",              onSave,            NULL },
  { "Save &as...",            onSaveAs,          NULL },
  { "Save as &2.10...",       onSaveAs210,       NULL },
  { "&Revert",                onRevert,          NULL },
  { "Quest &Templates...",    onQuestTemplates,  NULL },
  { "" },
  { "&Import",                NULL,              import_menu },
  { "&Export",                NULL,              export_menu },
  { "" },
  { "E&xit\tESC",             onExit,            NULL },
  //   { "Test box",               onTestBox,         NULL },
  { NULL }
};

static MENU maps_menu[] =
{
  { "&Goto Map",       onGotoMap,     NULL },
  { "Map &Count",      onMapCount,    NULL },
  { NULL }
};

static MENU misc_menu[] =
{
  { "Shop Types",             onShopTypes,            NULL },
  { "Info Types",             onInfoTypes,            NULL },
  { "Warp Rings",             onWarpRings,            NULL },
  { "Whistle Warps",          onWhistle,              NULL },
  { "Triforce Pieces",        onTriPieces,            NULL },
  { "End String",             onEndString,            NULL },
  { "Item Properties",        onItemProps,            NULL },
  { "Subscreens",             onEditSubscreens,       NULL },
  { "Master Subscreen Type",  onSubscreen,            NULL },
//  { "Screen Opening/Closing", onScreenOpeningClosing, NULL },
  { NULL }
};

static MENU spr_menu[] =
{
  { "&Items",             onCustomItems,  NULL },
  { "&Weapons/Misc",      onCustomWpns,   NULL },
  { "&Link",              onCustomLink,   NULL },
  //   { "Guys/Enemies",      onCustomGuys,   NULL },
  { NULL }
};

MENU colors_menu[] =
{
  { "&Main",           onColors_Main,       NULL },
  { "&Levels",         onColors_Levels,     NULL },
  { "&Sprites",        onColors_Sprites,    NULL },
  { NULL }
};

static MENU defs_menu[] =
{
  { "&Palettes",       onDefault_Pals,      NULL },
  { "&Tiles",          onDefault_Tiles,     NULL },
  { "&Combos",         onDefault_Combos,    NULL },
  { "&Sprites",        onDefault_Sprites,   NULL },
  { "&Map Styles",     onDefault_MapStyles, NULL },
  { NULL }
};

int onEditComboAlias();

static MENU graphics_menu[] =
{
  { "&Palettes\t ",     NULL,          colors_menu },
  { "&Sprites\t ",      NULL,          spr_menu },
  { "&Combos",         onCombos,      NULL },
  { "&Tiles",          onTiles,       NULL },
  { "&Game icons",      onIcons,       NULL },
  { "Misc &colors",     onMiscColors,  NULL },
  { "&Map styles",      onMapStyles,   NULL },
  { "&Door Combo Sets", onDoorCombos,  NULL },
  { "Combo &Aliases",   onEditComboAlias, NULL },
  { NULL }
};

static MENU music_menu[] =
{
  { "&MIDIs",              onMidis,          NULL },
  { "&Enhanced Music",     onEnhancedMusic,  NULL },
  { NULL }
};

static MENU quest_menu[] =
{
  { "Maps\t ",         NULL,          maps_menu },
  { "Misc Data\t ",    NULL,          misc_menu },
  { "&Graphics\t ",    NULL,          graphics_menu },
  { "" },
  { "&Header",         onHeader,      NULL },
  { "&Rules",          onRules,       NULL },
  //   { "R&ules2",         onRules2,     NULL },
  { "Ch&eats",         onCheats,      NULL },
  { "&Strings",        onStrings,     NULL },
  { "&DMaps",          onDmaps,       NULL },
  { "&Music",          NULL,          music_menu },
  { "&Init Data",      onInit,        NULL },
  //   { "" },
  //   { "Test",            onTest,        NULL },
  //   { "Test Options",    onTestOptions, NULL },
  { "" },
  { "Template",           onTemplates,          NULL },
  { "Defaults\t ",        NULL,          defs_menu },
  { NULL }
};

static MENU paste_menu[] =
{
  { "Paste To All",         onPasteToAll,      NULL },
  { "Paste All To All",     onPasteAllToAll,   NULL },
  { NULL }
};

static MENU paste_item_menu[] =
{
  { "Undercombo",			onPasteUnderCombo,      NULL },
  { "Secret Combos",		onPasteSecretCombos,   NULL },
  { "Freeform Combos",		onPasteFFCombos,   NULL },
  { "Screen Data",			onPasteScreenData,   NULL },
  { "Warps",				onPasteWarps,   NULL },
  { "Warp Return",			onPasteWarpLocations,   NULL },
  { "Enemies",				onPasteEnemies,   NULL },
  { "Room Type Data",		onPasteRoom,   NULL },
  { "Guy/String",			onPasteGuy,   NULL },
  { "Doors",				onPasteDoors,   NULL },
  { "Layers",				onPasteLayers,   NULL },
  { NULL }
};

static MENU edit_menu[] =
{
  { "&Undo\tU",               onUndo,            NULL },
  { "&Copy\tC",               onCopy,            NULL },
  { "&Paste\tV",              onPaste,           NULL },
  { "Paste All",              onPasteAll,        NULL },
  { "&Adv. Paste\t ",         NULL,              paste_menu },
  { "Paste &Spec.\t ",        NULL,              paste_item_menu },
  { "&Delete\tDel",           onDelete,          NULL },
  { "" },
  { "Delete &Map",            onDeleteMap,       NULL },
  { NULL }
};

static MENU drawing_mode_menu[] =
{
  { "&Normal",                onDrawingModeNormal,       NULL },
  { "&Relational",            onDrawingModeRelational,   NULL },
  { "&Dungeon Carving",       onDrawingModeDungeon,      NULL },
  { "&Combo Alias",           onDrawingModeAlias,        NULL },
  { NULL }
};

static MENU tool_menu[] =
{
  { "&Template\tF5",          onTemplate,   NULL },
  { "&Doors\tF6",             onDoors,      NULL },
  { "&Color set fix\tF7",     onCSetFix,    NULL },
  { "&Flags\tF8",             onFlags,      NULL },
  { "&List Combos Used\t'",   onUsedCombos, NULL },
  { "" },
  { "&Preview Mode\tX",       onX,          NULL },
  { "Drawing &Mode",          NULL,         drawing_mode_menu },
  { "" },
  { "&View Palette",          onShowPal,    NULL },
  { "Re-apply Template",      onReTemplate, NULL },
  { "Import FF Script",       onImportFFScript, NULL},
  //  { "Test &GUI",          onTestGUI,    NULL },
  { NULL }
};

/*
  static MENU warp_menu[] =
  {
  { "&Tile\tF10",     onTileWarp,       NULL },
  { "&Side\tF11",     onSideWarp,       NULL },
  { "&Item",          onItemWarp,       NULL },
  { "Ti&med",         onTimedWarp,      NULL },
  };
  */

int onSelectFFCombo();

static MENU data_menu[] =
{
  { "&Screen Data\tF9",    onScrData,        NULL },
  { "&Freeform Combos",    onSelectFFCombo,     NULL },
  //  { "&Warps\t ",           onWarps,          NULL },
  { "&Tile Warp\tF10",     onTileWarp,           NULL },
  { "&Side Warp\tF11",     onSideWarp,          NULL },
  { "&Enemy Flags\tF12",   onEnemyFlags,     NULL },
  { "&Path",               onPath,           NULL },
  { "" },
  { "&Item\tI",            onItem,           NULL },
  { "&Guy\tG",             onGuy,            NULL },
  { "&Message String\tS",  onString,         NULL },
  { "&Room Type\tR",       onRType,          NULL },
  { "Catch All\tA",        onCatchall,       NULL, D_DISABLED },
  { "&Enemies\tE",         onEnemies,        NULL },
  { "&Under Combo",        onUnderCombo,     NULL },
  { "&Secret Combos",      onSecretCombo,    NULL },
  { "&Layers",             onLayers,         NULL },
  { "&Palette\tF4",        onScreenPalette,  NULL },
  { NULL }
};

static MENU etc_menu[] =
{
  { "&Help...",            onHelp,        NULL },
  { "&About...",           onAbout,       NULL },
  { "Video &Mode...",      onZQVidMode,   NULL },
  { "&Options...",         onOptions,     NULL },
  { "&Fullscreen",         onFullScreen,  NULL },
  { "" },
  { "&View pic...\tP",     onViewPic,     NULL },
  { "View &map...",        onViewMap,     NULL },
  { "" },
  { "&Lost woods",         playTune,      NULL },
  { "&Play music",         playMusic,     NULL },
  { "&Change track",       changeTrack,   NULL },
  { "&Stop tunes",         stopMusic,     NULL },
  { "" },
  { "&Take Snapshot\tZ",   onSnapshot,    NULL },
  { NULL }
};

static MENU the_menu[] =
{
  { "&File",          NULL,            (MENU *) file_menu },
  { "&Quest",         NULL,            (MENU *) quest_menu },
  { "&Edit",          NULL,            (MENU *) edit_menu },
  { "&Tools",         NULL,            (MENU *) tool_menu },
  { "&Data",          NULL,            (MENU *) data_menu },
  { "Et&c",           NULL,            (MENU *) etc_menu },
  { NULL }
};

void rebuild_trans_table();
int launchPicViewer(BITMAP **pictoview, PALETTE pal,
					int *px, int *py, double *scale, bool isviewingmap);

int onB()
{
  restore_mouse();
  rebuild_trans_table();
  jwin_alert("Notice","Translucency Table Rebuilt",NULL,NULL,"OK",NULL,13,27,lfont);

  refresh(rALL);
  return D_O_K;
}

extern int zqwin_scale;

int onFullScreen()
{
#ifdef ALLEGRO_DOS
  return;
#endif
  get_palette(RAMpal);
  show_mouse(NULL);
  bool windowed=is_windowed_mode()!=0;
  if (windowed)
  {
#ifdef ALLEGRO_MACOSX
    zqwin_set_scale(2);
#else
    zqwin_set_scale(1);
#endif
  }
  else
  {
    zqwin_set_scale(scale_arg);
  }

  int ret=set_gfx_mode(windowed?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED,zq_screen_w,zq_screen_h,0,0);
  if(ret!=0)
  {
    if (zqwin_scale==1&&!windowed)
    {
      zqwin_set_scale(2);
      ret=set_gfx_mode(windowed?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED,zq_screen_w,zq_screen_h,0,0);
      if(ret!=0)
      {
        Z_message("Can't set video mode (%d).\n", ret);
        Z_message(allegro_error);
        exit(1);
      }
    }
    else
    {
      Z_message("Can't set video mode (%d).\n", ret);
      Z_message(allegro_error);
      exit(1);
    }
  }
  set_palette(RAMpal);
  gui_mouse_focus=0;
  gui_bg_color=jwin_pal[jcBOX];
  gui_fg_color=jwin_pal[jcBOXFG];
  gui_mg_color=jwin_pal[jcMEDDARK];
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  zqwin_set_scale(zq_scale);
  set_palette(RAMpal);
  position_mouse(zq_screen_w/2,zq_screen_h/2);
  show_mouse(screen);
  set_display_switch_mode(SWITCH_BACKGROUND);
  set_display_switch_callback(SWITCH_OUT, music_pause);
  set_display_switch_callback(SWITCH_IN, music_resume);
  return D_REDRAW;
}

int onEnter()
{
#ifdef ALLEGRO_DOS
#else
  if (key[KEY_ALT]||key[KEY_ALTGR])
  {
    return onFullScreen();
  }
#endif
  return D_O_K;
}

//PROC, x, y, w, h, fg, bg, key, flags, d1, d2, *dp, *dp2, *dp3

//*text, (*proc), *child, flags, *dp

int d_nbmenu_proc(int msg,DIALOG *d,int c);


int onY()
{
/*
  int i=Map.getCurrMap();
  int j=Map.getCurrScr();

  int scr=i*MAPSCRS+j;

  al_trace("screen %d\n--------------------\n", scr);
  al_trace("valid: %d\n", TheMaps[scr].valid);
  al_trace("guy: %d\n", TheMaps[scr].guy);
  al_trace("str: %d\n", TheMaps[scr].str);
  al_trace("room: %d\n", TheMaps[scr].room);
  al_trace("item: %d\n", TheMaps[scr].item);
  for(int k=0;k<4;k++)
  {
    al_trace("tilewarptype[k]: %d\n", TheMaps[scr].tilewarptype[k]);
  }
  al_trace("door_combo_set: %d\n", TheMaps[scr].door_combo_set);
  for(int k=0;k<4;k++)
  {
    al_trace("warpreturnx[k]: %d\n", TheMaps[scr].warpreturnx[k]);
  }
  for(int k=0;k<4;k++)
  {
    al_trace("warpreturny[k]: %d\n", TheMaps[scr].warpreturny[k]);
  }
  al_trace("warpreturnc: %d\n", TheMaps[scr].warpreturnc);
  al_trace("stairx: %d\n", TheMaps[scr].stairx);
  al_trace("stairy: %d\n", TheMaps[scr].stairy);
  al_trace("itemx: %d\n", TheMaps[scr].itemx);
  al_trace("itemy: %d\n", TheMaps[scr].itemy);
  al_trace("color: %d\n", TheMaps[scr].color);
  al_trace("enemyflags: %d\n", TheMaps[scr].enemyflags);
  for(int k=0; k<4; k++)
  {
    al_trace("door[k]: %d\n", TheMaps[scr].door[k]);
  }
  for(int k=0;k<4;k++)
  {
    al_trace("tilewarpdmap[k]: %d\n", TheMaps[scr].tilewarpdmap[k]);
  }
  for(int k=0;k<4;k++)
  {
    al_trace("tilewarpscr[k]: %d\n", TheMaps[scr].tilewarpscr[k]);
  }
  al_trace("exitdir: %d\n", TheMaps[scr].exitdir);
  for(int k=0; k<10; k++)
  {
    {
      al_trace("enemy[k]: %d\n", TheMaps[scr].enemy[k]);
    }
  }
  al_trace("pattern: %d\n", TheMaps[scr].pattern);
  for(int k=0;k<4;k++)
  {
    al_trace("sidewarptype[k]: %d\n", TheMaps[scr].sidewarptype[k]);
  }
  al_trace("warparrivalx: %d\n", TheMaps[scr].warparrivalx);
  al_trace("warparrivaly: %d\n", TheMaps[scr].warparrivaly);
  for(int k=0; k<4; k++)
  {
    al_trace("path[k]: %d\n", TheMaps[scr].path[k]);
  }
  for(int k=0;k<4;k++)
  {
    al_trace("sidewarpscr[k]: %d\n", TheMaps[scr].sidewarpscr[k]);
  }
  for(int k=0;k<4;k++)
  {
    al_trace("sidewarpdmap[k]: %d\n", TheMaps[scr].sidewarpdmap[k]);
  }
  al_trace("sidewarpindex: %d\n", TheMaps[scr].sidewarpindex);
  al_trace("undercombo: %d\n", TheMaps[scr].undercombo);
  al_trace("undercset: %d\n", TheMaps[scr].undercset);
  al_trace("catchall: %d\n", TheMaps[scr].catchall);
  al_trace("flags: %d\n", TheMaps[scr].flags);
  al_trace("flags2: %d\n", TheMaps[scr].flags2);
  al_trace("flags3: %d\n", TheMaps[scr].flags3);
  al_trace("flags4: %d\n", TheMaps[scr].flags4);
  al_trace("flags5: %d\n", TheMaps[scr].flags5);
  al_trace("noreset: %d\n", TheMaps[scr].noreset);
  al_trace("nocarry: %d\n", TheMaps[scr].nocarry);
  al_trace("flags6: %d\n", TheMaps[scr].flags6);
  al_trace("flags7: %d\n", TheMaps[scr].flags7);
  al_trace("flags8: %d\n", TheMaps[scr].flags8);
  al_trace("flags9: %d\n", TheMaps[scr].flags9);
  al_trace("flags10: %d\n", TheMaps[scr].flags10);
  al_trace("csensitive: %d\n", TheMaps[scr].csensitive);
  for(int k=0; k<6; k++)
  {
    al_trace("layermap[k]: %d\n", TheMaps[scr].layermap[k]);
  }
  for(int k=0; k<6; k++)
  {
    al_trace("layerscreen[k]: %d\n", TheMaps[scr].layerscreen[k]);
  }
  for(int k=0; k<6; k++)
  {
    al_trace("layeropacity[k]: %d\n", TheMaps[scr].layeropacity[k]);
  }
  al_trace("timedwarptics: %d\n", TheMaps[scr].timedwarptics);
  al_trace("nextmap: %d\n", TheMaps[scr].nextmap);
  al_trace("nextscr: %d\n", TheMaps[scr].nextscr);
  for(int k=0; k<128; k++)
  {
    al_trace("secretcombo[k]: %d\n", TheMaps[scr].secretcombo[k]);
  }
  for(int k=0; k<128; k++)
  {
    al_trace("secretcset[k]: %d\n", TheMaps[scr].secretcset[k]);
  }
  for(int k=0; k<128; k++)
  {
    al_trace("secretflag[k]: %d\n", TheMaps[scr].secretflag[k]);
  }
  for(int k=0; k<16*11; k++)
  {
    al_trace("data[k]: %d\n", TheMaps[scr].data[k]);
  }
  for(int k=0; k<16*11; k++)
  {
    al_trace("sflag[k]: %d\n", TheMaps[scr].sflag[k]);
  }
  for(int k=0; k<16*11; k++)
  {
    al_trace("cset[k]: %d\n", TheMaps[scr].cset[k]);
  }
  al_trace("screen_midi: %d\n", TheMaps[scr].screen_midi);
  al_trace("numff: %d\n", TheMaps[scr].numff);
  for(int k=0;k<32;k++)
  {
    if((TheMaps[scr].numff>>k)&1)
    {
      al_trace("ffdata[k]: %d\n", TheMaps[scr].ffdata[k]);
      al_trace("ffcset[k]: %d\n", TheMaps[scr].ffcset[k]);
      al_trace("ffdelay[k]: %d\n", TheMaps[scr].ffdelay[k]);
      al_trace("ffx[k]: %d\n", TheMaps[scr].ffx[k]);
      al_trace("ffy[k]: %d\n", TheMaps[scr].ffy[k]);
      al_trace("ffxdelta[k]: %d\n", TheMaps[scr].ffxdelta[k]);
      al_trace("ffydelta[k]: %d\n", TheMaps[scr].ffydelta[k]);
      al_trace("ffxdelta2[k]: %d\n", TheMaps[scr].ffxdelta2[k]);
      al_trace("ffydelta2[k]: %d\n", TheMaps[scr].ffydelta2[k]);
      al_trace("fflink[k]: %d\n", TheMaps[scr].fflink[k]);
      al_trace("ffwidth[k]: %d\n", TheMaps[scr].ffwidth[k]);
      al_trace("ffheight[k]: %d\n", TheMaps[scr].ffheight[k]);
      al_trace("ffflags[k]: %d\n", TheMaps[scr].ffflags[k]);
    }
  }
  al_trace("\n\n");
*/
  return D_O_K;
}

int onToggleGrid()
{
  if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
  {
    GridColor=(GridColor+8)%16;
  }
  else
  {
    ShowGrid=!ShowGrid;
  }
  return D_O_K;
}

static DIALOG dialogs[] =
{
  // still unused:  hjknty
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)         (d2)     (dp) */
  { d_nbmenu_proc,     0,    0,    0,    13,    0,    0,    0,       D_USER,  0,             0,       (void *) the_menu },

  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_UP,         0,       (void *) onUp },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_DOWN,       0,       (void *) onDown },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_LEFT,       0,       (void *) onLeft },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_RIGHT,      0,       (void *) onRight },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PGUP,       0,       (void *) onPgUp },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PGDN,       0,       (void *) onPgDn },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PLUS_PAD,   0,       (void *) onPlus },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_MINUS_PAD,  0,       (void *) onMinus },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_ASTERISK,   0,       (void *) onTimes },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_SLASH_PAD,  0,       (void *) onDivide },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,         0,       (void *) onHelp },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,         0,       (void *) onSave },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F3,         0,       (void *) onOpen },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F4,         0,       (void *) onScreenPalette },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F5,         0,       (void *) onTemplate },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,         0,       (void *) onDoors },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,         0,       (void *) onCSetFix },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,         0,       (void *) onFlags },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F9,         0,       (void *) onScrData },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F10,        0,       (void *) onTileWarp },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F11,        0,       (void *) onSideWarp },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,        0,       (void *) onEnemyFlags },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    27,      0,       0,              0,       (void *) onExit },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'a',     0,       0,              0,       (void *) onCatchall },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'b',     0,       0,              0,       (void *) onB },   //
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'c',     0,       0,              0,       (void *) onCopy },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'd',     0,       0,              0,       (void *) onD },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'e',     0,       0,              0,       (void *) onEnemies },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'f',     0,       0,              0,       (void *) onF },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'g',     0,       0,              0,       (void *) onGuy },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'i',     0,       0,              0,       (void *) onItem },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'l',     0,       0,              0,       (void *) onL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'm',     0,       0,              0,       (void *) onM },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'o',     0,       0,              0,       (void *) onDrawingMode },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'p',     0,       0,              0,       (void *) onViewPic },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'q',     0,       0,              0,       (void *) onQ },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'r',     0,       0,              0,       (void *) onRType },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    's',     0,       0,              0,       (void *) onString },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'u',     0,       0,              0,       (void *) onUndo },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'v',     0,       0,              0,       (void *) onPaste },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'w',     0,       0,              0,       (void *) onW },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'x',     0,       0,              0,       (void *) onX },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'y',     0,       0,              0,       (void *) onY },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'z',     0,       0,              0,       (void *) onSnapshot },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '0',     0,       0,              0,       (void *) on0 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '1',     0,       0,              0,       (void *) on1 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '2',     0,       0,              0,       (void *) on2 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '3',     0,       0,              0,       (void *) on3 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '4',     0,       0,              0,       (void *) on4 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '5',     0,       0,              0,       (void *) on5 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '6',     0,       0,              0,       (void *) on6 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '7',     0,       0,              0,       (void *) on7 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '8',     0,       0,              0,       (void *) on8 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '9',     0,       0,              0,       (void *) on9 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    ')',     0,       0,              0,       (void *) on10 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '!',     0,       0,              0,       (void *) on11 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '@',     0,       0,              0,       (void *) on12 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '#',     0,       0,              0,       (void *) on13 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '$',     0,       0,              0,       (void *) on14 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    ',',     0,       0,              0,       (void *) onDecMap },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '.',     0,       0,              0,       (void *) onIncMap },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '<',     0,       0,              0,       (void *) onDecScrPal },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '>',     0,       0,              0,       (void *) onIncScrPal },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TILDE,      0,       (void *) onToggleGrid },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    39,      0,       0,              0,       (void *) onUsedCombos },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_SPACE,      0,       (void *) onSpacebar },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_DEL,        0,       (void *) onDelete },//
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_ENTER,      0,       (void *) onEnter },
  { NULL,              0,    0,    0,    0,    0,    0,    0,       0,       0,              0,       NULL }
};

static DIALOG getnum_dlg[] =
{
  // (dialog proc)       (x)   (y)    (w)     (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,        80,   80,     160,    72,   vc(0),              vc(11),           0,       D_EXIT,     0,             0,       NULL },
  { jwin_rtext_proc,      114,  104+4,  48,     8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Value:" },
  { jwin_edit_proc,       168,  104,    48,     16,    0,                 0,                0,       0,          6,             0,       NULL },
  { jwin_button_proc,     90,   126,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  126,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int getnumber(char *prompt,int initialval)
{
  cancelgetnum=true;
  char buf[20];
  sprintf(buf,"%d",initialval);
  getnum_dlg[0].dp=prompt;
  getnum_dlg[0].dp2=lfont;
  getnum_dlg[2].dp=buf;
  int ret=zc_popup_dialog(getnum_dlg,2);
  if (ret!=0&&ret!=4)
  {
    cancelgetnum=false;
  }
  if(ret==3)
    return atoi(buf);
  return initialval;
}



int gethexnumber(char *prompt,int initialval)
{
  char buf[20];
  sprintf(buf,"%X",initialval);
  getnum_dlg[0].dp=prompt;
  getnum_dlg[0].dp2=lfont;
  getnum_dlg[2].dp=buf;

  if(zc_popup_dialog(getnum_dlg,2)==3)
    return xtoi(buf);
  return -1;
}

void update_combo_cycling()
{
  Map.update_combo_cycling();
}

void update_freeform_combos()
{
  Map.update_freeform_combos();
}

/***********************/
/*** dialog handlers ***/
/***********************/

extern char *colorlist(int index, int *list_size);

static DIALOG options_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,       80,   72-24,   160+28,  238,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "ZQuest Options" },
  { d_dummy_proc,         0,     0,     0,  0,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_button_proc,    111,   260,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,    191,   260,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_check_proc,      98,   72,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Mouse scroll" },
  { jwin_check_proc,      98,   82,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Save paths" },
  { jwin_check_proc,      98,   92,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Palette cycle" },
  { jwin_check_proc,      98,   102,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Vsync" },
  { jwin_check_proc,      98,   112,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Show FPS" },
  { jwin_check_proc,      98,   122,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Combo Brush" },
  { jwin_check_proc,      98,   132,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Floating Brush" },
  { jwin_check_proc,      98,   142,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Reload Last" },
  { jwin_check_proc,      98,   152,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Show Misaligns" },
  { jwin_check_proc,      98,   162,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Animate Combos" },
  { jwin_check_proc,      98,   172,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Auto-backup" },
  { jwin_check_proc,      98,   182,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Overwrite Protection" },
  { jwin_check_proc,      98,   192,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Strict MIDI" },
  { jwin_check_proc,      98,   202,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Tile Protection" },
  { jwin_check_proc,      98,   212,  128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Show Grid" },
  { jwin_text_proc,       98,   226,  128+1,  8+1,    0,       0,      0,       0,          0,             0,       (void *) "Grid Color:" },
  { jwin_droplist_proc,  150,   222,  100,    16,     0,       0,      0,       0,          0,             0,       (void *) colorlist },
  { jwin_text_proc,       98,   244,  128+1,  8+1,    0,       0,      0,       0,          0,             0,       (void *) "Snapshot Format:" },
  { jwin_droplist_proc,  179,   240,  55,     16,     0,       0,      0,       0,          0,             0,       (void *) snapshotformatlist },
  { NULL }
};
#ifdef ALLEGRO_LINUX_GP2X
bool midi_strict=false;
#endif
int onOptions()
{
  options_dlg[0].dp2=lfont;
  reset_combo_animations();
  go();
  options_dlg[4].flags = MouseScroll ? D_SELECTED : 0;
  options_dlg[5].flags = SavePaths ? D_SELECTED : 0;
  options_dlg[6].flags = CycleOn ? D_SELECTED : 0;
  options_dlg[7].flags = Vsync ? D_SELECTED : 0;
  options_dlg[8].flags = ShowFPS ? D_SELECTED : 0;
  options_dlg[9].flags = ComboBrush ? D_SELECTED : 0;
  options_dlg[10].flags = FloatBrush ? D_SELECTED : 0;
  options_dlg[11].flags = OpenLastQuest ? D_SELECTED : 0;
  options_dlg[12].flags = ShowMisalignments ? D_SELECTED : 0;
  options_dlg[13].flags = AnimationOn ? D_SELECTED : 0;
  options_dlg[14].flags = AutoBackup ? D_SELECTED : 0;
  options_dlg[15].flags = OverwriteProtection ? D_SELECTED : 0;
  options_dlg[16].flags = midi_strict ? D_SELECTED : 0;
  options_dlg[17].flags = TileProtection ? D_SELECTED : 0;
  options_dlg[18].flags = ShowGrid ? D_SELECTED : 0;
  options_dlg[20].d1 = GridColor;
  options_dlg[22].d1 = SnapshotFormat;
  if(zc_popup_dialog(options_dlg,-1) == 2)
  {
    MouseScroll         = options_dlg[4].flags & D_SELECTED ? 1 : 0;
    SavePaths           = options_dlg[5].flags & D_SELECTED ? 1 : 0;
    CycleOn             = options_dlg[6].flags & D_SELECTED ? 1 : 0;
    Vsync               = options_dlg[7].flags & D_SELECTED ? 1 : 0;
    ShowFPS             = options_dlg[8].flags & D_SELECTED ? 1 : 0;
    ComboBrush          = options_dlg[9].flags & D_SELECTED ? 1 : 0;
    FloatBrush          = options_dlg[10].flags & D_SELECTED ? 1 : 0;
    OpenLastQuest       = options_dlg[11].flags & D_SELECTED ? 1 : 0;
    ShowMisalignments   = options_dlg[12].flags & D_SELECTED ? 1 : 0;
    AnimationOn         = options_dlg[13].flags & D_SELECTED ? 1 : 0;
    AutoBackup          = options_dlg[14].flags & D_SELECTED ? 1 : 0;
    OverwriteProtection = options_dlg[15].flags & D_SELECTED ? 1 : 0;
    midi_strict         = options_dlg[16].flags & D_SELECTED ? 1 : 0;
    TileProtection      = options_dlg[17].flags & D_SELECTED ? 1 : 0;
    ShowGrid            = options_dlg[18].flags & D_SELECTED ? 1 : 0;
    GridColor           = options_dlg[20].d1;
    SnapshotFormat      = options_dlg[22].d1;
  }
  setup_combo_animations();
  refresh(rALL);
  comeback();
  return D_O_K;
}

enum {dm_normal, dm_relational, dm_dungeon, dm_alias, dm_max};
char *dm_names[dm_max]=
{
  "Normal",
  "Relational",
  "Dungeon",
  "Alias"
};

byte relational_tile_grid[11+(rtgyo*2)][16+(rtgxo*2)];

void fix_drawing_mode_menu()
{
  for (int i=0; i<dm_max; ++i)
  {
    drawing_mode_menu[i].flags=0;
  }
  drawing_mode_menu[draw_mode].flags=D_SELECTED;
}

int onDrawingMode()
{
  draw_mode=(draw_mode+1)%dm_max;
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  fix_drawing_mode_menu();
  restore_mouse();
  return D_O_K;
}

int onDrawingModeNormal()
{
  draw_mode=dm_normal;
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  fix_drawing_mode_menu();
  restore_mouse();
  return D_O_K;
}

int onDrawingModeRelational()
{
  draw_mode=dm_relational;
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  fix_drawing_mode_menu();
  restore_mouse();
  return D_O_K;
}

int onDrawingModeDungeon()
{
  draw_mode=dm_dungeon;
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  fix_drawing_mode_menu();
  restore_mouse();
  return D_O_K;
}

int onDrawingModeAlias()
{
  draw_mode=dm_alias;
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  fix_drawing_mode_menu();
  restore_mouse();
  return D_O_K;
}

int onReTemplate()
{
  if(jwin_alert("Confirm Overwrite","Apply template to","all screens on this map?",NULL,"&Yes","&No",'y','n',lfont)==1)
  {
    Map.TemplateAll();
    refresh(rALL);
  }
  return D_O_K;
}

int onUndo()
{
  Map.Uhuilai();
  refresh(rALL);
  return D_O_K;
}

extern short ffposx[32];
extern short ffposy[32];
extern long ffprvx[32];
extern long ffprvy[32];

int onCopy()
{
  if(prv_mode)
  {
    Map.set_prvcmb(Map.get_prvcmb()==0?1:0);
	for(int i=0;i<32;i++)
	{
	  ffposx[i]=-1000;
	  ffposy[i]=-1000;
	  ffprvx[i]=-10000000;
	  ffprvy[i]=-10000000;
	}
    return D_O_K;
  }
  Map.Copy();
  return D_O_K;
}

int onPaste()
{
  Map.Paste();
  refresh(rALL);
  return D_O_K;
}

int onPasteAll()
{
  Map.PasteAll();
  refresh(rALL);
  return D_O_K;
}

int onPasteToAll()
{

  Map.PasteToAll();
  refresh(rALL);
  return D_O_K;
}

int onPasteAllToAll()
{
  Map.PasteAllToAll();
  refresh(rALL);
  return D_O_K;
}

int onPasteUnderCombo()
{
  Map.PasteUnderCombo();
  refresh(rALL);
  return D_O_K;
}

int onPasteSecretCombos()
{
  Map.PasteSecretCombos();
  refresh(rALL);
  return D_O_K;
}

int onPasteFFCombos()
{
  Map.PasteFFCombos();
  refresh(rALL);
  return D_O_K;
}

int onPasteWarps()
{
  Map.PasteWarps();
  refresh(rALL);
  return D_O_K;
}

int onPasteScreenData()
{
  Map.PasteScreenData();
  refresh(rALL);
  return D_O_K;
}

int onPasteWarpLocations()
{
  Map.PasteWarpLocations();
  refresh(rALL);
  return D_O_K;
}

int onPasteDoors()
{
  Map.PasteDoors();
  refresh(rALL);
  return D_O_K;
}

int onPasteLayers()
{
  Map.PasteLayers();
  refresh(rALL);
  return D_O_K;
}

int onPasteRoom()
{
  Map.PasteRoom();
  refresh(rALL);
  return D_O_K;
}

int onPasteGuy()
{
  Map.PasteGuy();
  refresh(rALL);
  return D_O_K;
}

int onPasteEnemies()
{
  Map.PasteEnemies();
  refresh(rALL);
  return D_O_K;
}

int onDelete()
{
  restore_mouse();
  if(Map.CurrScr()->valid&mVALID)
  {
    if(jwin_alert("Confirm Delete","Delete this screen?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
    {
      Map.Ugo();
      Map.clearscr(Map.getCurrScr());
      refresh(rALL);
    }
  }
  memset(relational_tile_grid,(draw_mode==dm_relational?1:0),(11+(rtgyo*2))*(16+(rtgxo*2)));
  saved=false;
  return D_O_K;

}

int onDeleteMap()
{
  if(jwin_alert("Confirm Delete","Clear this entire map?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    Map.clearmap(false);
    refresh(rALL);
  }
  return D_O_K;
}

int onD()
{
  Map.CurrScr()->flags^=4;
  refresh(rMAP+rMENU);
  saved=false;
  return D_O_K;
}

int onIncMap()
{
  int m=Map.getCurrMap();
  int oldcolor=Map.getcolor();
  Map.setCurrMap(m+1>=map_count?0:m+1);
  int newcolor=Map.getcolor();
  if (newcolor!=oldcolor)
  {
    rebuild_trans_table();
  }
  refresh(rALL);
  return D_O_K;
}

int onDecMap()
{
  int m=Map.getCurrMap();
  int oldcolor=Map.getcolor();
  Map.setCurrMap((m-1<0)?map_count-1:min(m-1,map_count-1));
  int newcolor=Map.getcolor();
  if (newcolor!=oldcolor)
  {
    rebuild_trans_table();
  }
  refresh(rALL);
  return D_O_K;
}


int onDefault_Pals()
{
  if(jwin_alert("Confirm Reset","Reset all palette data?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    saved=false;
    if (!init_colordata(true, &header, &misc))
    {
      jwin_alert("Error","Palette reset failed.",NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    refresh_pal();
  }
  return D_O_K;
}

int onDefault_Combos()
{
  if(jwin_alert("Confirm Reset","Reset combo data?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    saved=false;
    if (!init_combos(true, &header))
    {
      jwin_alert("Error","Combo reset failed.",NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    refresh(rALL);
  }

  return D_O_K;
}

int onDefault_Sprites()
{
  if(jwin_alert("Confirm Reset","Reset sprite data?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    saved=false;
    reset_items(true, &header);
    reset_wpns(true, &header);
  }
  return D_O_K;
}


int onDefault_Tiles()
{
  if(jwin_alert("Confirm Reset","Reset all tiles?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    saved=false;
    if (!init_tiles(true, &header))
    {
      jwin_alert("Error","Tile reset failed.",NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    refresh(rALL);
  }
  return D_O_K;
}

int onDefault_MapStyles()
{
  if(jwin_alert("Confirm Reset","Reset all map styles?", NULL, NULL, "Yes", "Cancel", 'y', 27,lfont) == 1)
  {
    saved=false;
    reset_mapstyles(true, &header, &misc);
  }
  return D_O_K;
}


int on0() { saved=false; Map.setcolor(0); refresh(rSCRMAP); return D_O_K; }
int on1() { saved=false; Map.setcolor(1); refresh(rSCRMAP); return D_O_K; }
int on2() { saved=false; Map.setcolor(2); refresh(rSCRMAP); return D_O_K; }
int on3() { saved=false; Map.setcolor(3); refresh(rSCRMAP); return D_O_K; }
int on4() { saved=false; Map.setcolor(4); refresh(rSCRMAP); return D_O_K; }
int on5() { saved=false; Map.setcolor(5); refresh(rSCRMAP); return D_O_K; }

int on6() { saved=false; Map.setcolor(6); refresh(rSCRMAP); return D_O_K; }
int on7() { saved=false; Map.setcolor(7); refresh(rSCRMAP); return D_O_K; }
int on8() { saved=false; Map.setcolor(8); refresh(rSCRMAP); return D_O_K; }
int on9() { saved=false; Map.setcolor(9); refresh(rSCRMAP); return D_O_K; }
int on10() { saved=false; Map.setcolor(10); refresh(rSCRMAP); return D_O_K; }
int on11() { saved=false; Map.setcolor(11); refresh(rSCRMAP); return D_O_K; }
int on12() { saved=false; Map.setcolor(12); refresh(rSCRMAP); return D_O_K; }
int on13() { saved=false; Map.setcolor(13); refresh(rSCRMAP); return D_O_K; }
int on14() { saved=false; Map.setcolor(14); refresh(rSCRMAP); return D_O_K; }

int onLeft()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(2);
    refresh(rALL);
  }
  else if ((First>0)&&(draw_mode!=dm_alias))
    {
      First-=1;
      refresh(rCOMBOS);
    }
    else if ((combo_alistpos>0)&&(draw_mode==dm_alias))
      {
        combo_alistpos-=1;
        refresh(rCOMBOS);
      }
      clear_keybuf();
  return D_O_K;
}

int onRight()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(3);
    refresh(rALL);

  }
  else if ((First<(MAXCOMBOS-(combolist_w*combolist_h)))&&(draw_mode!=dm_alias))
    {
      First+=1;
      refresh(rCOMBOS);
    }
    else if ((combo_alistpos<(MAXCOMBOALIASES-(combolist_w*combolist_h)))&&(draw_mode==dm_alias))
      {
        combo_alistpos+=1;
        refresh(rCOMBOS);
      }
      clear_keybuf();
  return D_O_K;
}

int onUp()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(0);
    refresh(rALL);
  }
  else if((First>0)&&(draw_mode!=dm_alias))
    {
      First-=min(First,combolist_w);
      refresh(rCOMBOS);
    }
    else if((combo_alistpos>0)&&(draw_mode==dm_alias))
      {
        combo_alistpos-=min(combo_alistpos,combolist_w);
        refresh(rCOMBOS);
      }
      clear_keybuf();
  return D_O_K;
}

int onDown()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(1);
    refresh(rALL);
  }
  else if((First<(MAXCOMBOS-(combolist_w*combolist_h)))&&(draw_mode!=dm_alias))
    {
      First+=min((MAXCOMBOS-combolist_w)-First,combolist_w);
      refresh(rCOMBOS);
    }
    else if((combo_alistpos<(MAXCOMBOALIASES-(combolist_w*combolist_ha)))&&(draw_mode==dm_alias))
      {
        combo_alistpos+=min((MAXCOMBOALIASES-combolist_w)-combo_alistpos,combolist_w);
        refresh(rCOMBOS);
      }
      clear_keybuf();
  return D_O_K;
}

int onPgUp()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT] &&
     !key[KEY_ZC_LCONTROL] && !key[KEY_ZC_RCONTROL])
  {
    menutype=wrap(menutype-1,0,m_menucount-1);
    refresh(rMENU);
  }
  else if((First>0)&&(draw_mode!=dm_alias))
    {
      if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      {
        First-=min(First,256);
      }
      else
      {
        First-=min(First,(combolist_w*combolist_h));
      }
      refresh(rCOMBOS);
    }
    else if((combo_alistpos>0)&&(draw_mode==dm_alias))
      {
        if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          combo_alistpos=0;
        }
        else
        {
          combo_alistpos-=min(combo_alistpos,(combolist_w*combolist_ha));
        }
        refresh(rCOMBOS);
      }
      return D_O_K;
}

int onPgDn()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT] &&
     !key[KEY_ZC_LCONTROL] && !key[KEY_ZC_RCONTROL])
  {
    menutype=wrap(menutype+1,0,m_menucount-1);
    refresh(rMENU);
  }
  else if((First<(MAXCOMBOS-(combolist_w*combolist_h)))&&(draw_mode!=dm_alias))
    {
      if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      {
        First+=min((MAXCOMBOS-256)-First,256);
      }
      else
      {
        First+=min((MAXCOMBOS-(combolist_w*combolist_h))-First,(combolist_w*combolist_h));
      }
      refresh(rCOMBOS);
    }
    else if((combo_alistpos<(MAXCOMBOALIASES-(combolist_w*combolist_ha)))&&(draw_mode==dm_alias))
      {
        if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          //combo_apos+=min((MAXCOMBOALIASES-256)-combo_apos,256);
          combo_alistpos=MAXCOMBOALIASES-(combolist_w*combolist_ha);
        }
        else
        {
          combo_alistpos+=min((MAXCOMBOALIASES-(combolist_w*combolist_ha))-combo_alistpos,(combolist_w*combolist_ha));
        }
        refresh(rCOMBOS);
      }
      return D_O_K;
}

int onPlus()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT] &&
     !key[KEY_ZC_LCONTROL] && !key[KEY_ZC_RCONTROL] &&
     !key[KEY_ALT] && !key[KEY_ALTGR])
  {
    CSet=wrap(CSet+1,0,11);
    refresh(rCOMBOS+rMENU+rCOMBO);
  }
  else if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
    {
      int drawmap, drawscr;
      if (CurrentLayer==0)
      {
        drawmap=Map.getCurrMap();
        drawscr=Map.getCurrScr();
      }
      else
      {
        drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
        drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];

        if (drawmap<0)
        {
          return D_O_K;
        }
      }
      if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
      {
        return D_O_K;
      }
      saved=false;
      Map.Ugo();
      int changeby=1;
      if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      {
        changeby*=16;
      }
      if (key[KEY_ALT] || key[KEY_ALTGR])
      {
        changeby*=256;
      }
      for(int i=0; i<176; i++)
      {
        int temp=Map.AbsoluteScr(drawmap, drawscr)->data[i];

        temp+=changeby;
        if (temp>=MAXCOMBOS)
        {
          temp=temp-MAXCOMBOS;
        }
        Map.AbsoluteScr(drawmap, drawscr)->data[i]=temp;
      }
      refresh(rMAP+rSCRMAP);
    }
    return D_O_K;
}

int onMinus()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT] &&
     !key[KEY_ZC_LCONTROL] && !key[KEY_ZC_RCONTROL] &&
     !key[KEY_ALT] && !key[KEY_ALTGR])
  {
    CSet=wrap(CSet-1,0,11);
    refresh(rCOMBOS+rMENU+rCOMBO);
  }
  else if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
    {
      int drawmap, drawscr;
      if (CurrentLayer==0)
      {
        drawmap=Map.getCurrMap();
        drawscr=Map.getCurrScr();
      }
      else
      {
        drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
        drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
        if (drawmap<0)
        {
          return D_O_K;
        }
      }
      if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
      {
        return D_O_K;
      }
      saved=false;
      Map.Ugo();
      int changeby=1;
      if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      {
        changeby*=16;
      }
      if (key[KEY_ALT] || key[KEY_ALTGR])
      {
        changeby*=256;
      }
      for(int i=0; i<176; i++)
      {
        int temp=Map.AbsoluteScr(drawmap, drawscr)->data[i];
        temp-=changeby;
        if (temp<0)
        {
          temp=MAXCOMBOS+temp;
        }
        Map.AbsoluteScr(drawmap, drawscr)->data[i]=temp;
      }
      refresh(rMAP+rSCRMAP);
    }
    return D_O_K;
}

bool getname(char *prompt,char *ext,char *def,bool usefilename)
{
  strcpy(temppath,def);

  if(!usefilename)
  {

    int i=strlen(temppath);
    while(i>=0 && temppath[i]!='\\' && temppath[i]!='/')
      temppath[i--]=0;
  }

  go();
  //  int ret = file_select_ex(prompt,temppath,ext,255,-1,-1);
  int ret = jwin_file_select_ex(prompt,temppath,ext,2048,-1,-1,lfont);
  comeback();
  return ret!=0;
}


bool getname_nogo(char *prompt,char *ext,EXT_LIST *list,char *def,bool usefilename)
{
  strcpy(temppath,def);

  if(!usefilename)
  {
    int i=strlen(temppath);
    while(i>=0 && temppath[i]!='\\' && temppath[i]!='/')
      temppath[i--]=0;
  }

  //  int ret = file_select_ex(prompt,temppath,ext,255,-1,-1);
  int ret=0;
  int sel=0;
  if(list==NULL)
  {
    ret = jwin_file_select_ex(prompt,temppath,ext,2048,-1,-1,lfont);
  }
  else
  {
    ret = jwin_file_browse_ex(prompt, temppath, list, &sel, 2048, -1, -1, lfont);
  }
  return ret!=0;
}


static char track_number_str_buf[32];
char *tracknumlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,255);
    sprintf(track_number_str_buf,"%02d",index+1);
    return track_number_str_buf;
  }
  *list_size=zcmusic_get_tracks(zcmusic);
  return NULL;
}

static DIALOG change_track_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      60-12,   40,   200-16,  72,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select Track" },
  { d_dummy_proc,       160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_droplist_proc, 72-12,   60+4,   161,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) tracknumlist },
  { jwin_button_proc,   70,   87,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   150,  87,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};
//  return list_dlg[2].d1;

int changeTrack()
{
  restore_mouse();
  change_track_dlg[0].dp2=lfont;
  change_track_dlg[2].d1=gme_track;
  if(zc_popup_dialog(change_track_dlg,2)==3)
  {
    gme_track=change_track_dlg[2].d1;
    zcmusic_change_track(zcmusic, gme_track);
  }
  return D_O_K;
}


int playMusic()
{
  char *ext=(char *)malloc(4);
  bool ismidi=false;
  char allmusic_types[256];
  sprintf(allmusic_types, "%s;mid", zcmusic_types);
  if(getname("Load Music",(char*)allmusic_types,midipath,false))
  {
    strcpy(midipath,temppath);

    ext=get_extension(midipath);
    if (
        (stricmp(ext,"ogg")==0)||
        (stricmp(ext,"mp3")==0)||
        (stricmp(ext,"it")==0)||
        (stricmp(ext,"xm")==0)||
        (stricmp(ext,"s3m")==0)||
        (stricmp(ext,"mod")==0)||
        (stricmp(ext,"spc")==0)||
        (stricmp(ext,"gym")==0)||
        (stricmp(ext,"nsf")==0)||
        (stricmp(ext,"gbs")==0)||
        (stricmp(ext,"vgm")==0)
       )
    {
      ismidi=false;
    }
    else if ((stricmp(ext,"mid")==0))
    {
      ismidi=true;
    }
    else
    {
      return D_O_K;
    }

    stop_midi();
    if(zcmusic != NULL)
    {
      zcmusic_stop(zcmusic);
      zcmusic_unload_file(zcmusic);
      zcmusic = NULL;
    }
    if (ismidi)
    {
      if((song=load_midi(midipath))!=NULL)
      {
        if(play_midi(song,true)==0)
        {
          etc_menu[9].flags=0;
          etc_menu[10].flags=D_SELECTED;
          etc_menu[11].flags=D_DISABLED;
        }
      }
    }
    else
    {
      gme_track=0;
      zcmusic = (ZCMUSIC*)zcmusic_load_file(midipath);
      if (zcmusic!=NULL)
      {
        zcmusic_play(zcmusic, 128, true);
        etc_menu[9].flags=0;
        etc_menu[10].flags=D_SELECTED;
        etc_menu[11].flags=(zcmusic_get_tracks(zcmusic)<2)?D_DISABLED:0;
      }
    }
  }
  return D_O_K;
}

int playTune()
{
  stop_midi();
  if(zcmusic != NULL)
  {
    zcmusic_stop(zcmusic);
    zcmusic_unload_file(zcmusic);
    zcmusic = NULL;
  }
  if(play_midi((MIDI*)zcdata[MID_LOSTWOODS].dat,true)==0)
  {
    etc_menu[9].flags=D_SELECTED;
    etc_menu[10].flags=0;
    etc_menu[11].flags=D_DISABLED;
  }
  return D_O_K;
}

int stopMusic()
{
  stop_midi();
  if(zcmusic != NULL)
  {
    zcmusic_stop(zcmusic);
    zcmusic_unload_file(zcmusic);
    zcmusic = NULL;
  }
  etc_menu[9].flags = etc_menu[10].flags = 0;
  etc_menu[11].flags=D_DISABLED;
  return D_O_K;
}

#include "zq_files.h"

int onTemplates()
{
  edit_qt();
  return D_O_K;
}

//  +----------+
//  |          |
//  | View Pic |
//  |          |
//  |          |
//  |          |
//  +----------+

BITMAP *pic=NULL;
BITMAP *map=NULL;
PALETTE picpal;
PALETTE mappal;
int  picx=0,picy=0,mapx=0,mapy=0,pblack,pwhite;

double picscale=1.0,mapscale=1.0;
bool vp_showpal=true, vp_showsize=true, vp_center=true;

//INLINE int pal_sum(RGB p) { return p.r + p.g + p.b; }

void get_bw(RGB *pal,int &black,int &white)
{
  black=white=1;
  for(int i=1; i<256; i++)
  {
    if(pal_sum(pal[i])<pal_sum(pal[black]))
      black=i;
    if(pal_sum(pal[i])>pal_sum(pal[white]))
      white=i;
  }
}

void draw_bw_mouse(int white, int old_mouse, int new_mouse)
{
  blit(mouse_bmp[old_mouse][0],mouse_bmp[new_mouse][0],0,0,0,0,16,16);
  for(int y=0; y<16; y++)
  {
    for(int x=0; x<16; x++)
    {
      if(getpixel(mouse_bmp[new_mouse][0],x,y)!=0)
      {
        putpixel(mouse_bmp[new_mouse][0],x,y,white);
      }
    }
  }
}

int load_the_pic(BITMAP **dst, PALETTE dstpal)
{
  PALETTE temppal;

  for(int i=0; i<256; i++)
  {
    temppal[i]=dstpal[i];
    dstpal[i]=RAMpal[i];
  }
  // set up the new palette
  for(int i=0; i<64; i++)
  {
    dstpal[i].r = i;
    dstpal[i].g = i;
    dstpal[i].b = i;
  }
  set_palette(dstpal);

  BITMAP *graypic = create_bitmap_ex(8,SCREEN_W,SCREEN_H);

  // gray scale the current frame
  for(int y=0; y<SCREEN_H; y++)
  {
    for(int x=0; x<SCREEN_W; x++)
    {
      int c = screen->line[y][x];
      int gray = min((temppal[c].r*42 + temppal[c].g*75 + temppal[c].b*14) >> 7, 63);
      graypic->line[y][x] = gray;
    }
  }
  blit(graypic,screen,0,0,0,0,320,240);
  destroy_bitmap(graypic);
  char extbuf[2][80];
  memset(extbuf[0],0,80);
  memset(extbuf[1],0,80);
  sprintf(extbuf[0], "View Image (%s", snapshotformat_str[0][1]);
  sprintf(extbuf[1], snapshotformat_str[0][1]);

  for (int i=1; i<ssfmtMAX; ++i)
  {
    sprintf(extbuf[0], "%s, %s", extbuf[0], snapshotformat_str[i][1]);
    sprintf(extbuf[1], "%s;%s", extbuf[1], snapshotformat_str[i][1]);
  }
  sprintf(extbuf[0], "%s)", extbuf[0]);

  int gotit = getname(extbuf[0],extbuf[1],imagepath,true);
  if(!gotit)
  {
    set_palette(temppal);
    get_palette(dstpal);
    return 1;
  }
  strcpy(imagepath,temppath);

  if(*dst)
  {
    destroy_bitmap(*dst);
  }

  for(int i=0; i<256; i++)
  {
    dstpal[i].r = 0;
    dstpal[i].g = 0;
    dstpal[i].b = 0;
  }

  *dst = load_bitmap(imagepath,picpal);

  if(!*dst)
  {
    jwin_alert("Error","Error loading image:",imagepath,NULL,"OK",NULL,13,27,lfont);
    return 2;
  }

  //  get_bw(picpal,pblack,pwhite);
  //  draw_bw_mouse(pwhite);
  //  gui_bg_color = pblack;
  //  gui_fg_color = pwhite;

  if(vp_center)
  {
    picx=picy=0;
  }
  else
  {
    picx=(*dst)->w-zq_screen_w;
    picy=(*dst)->h-zq_screen_h;
  }
  return 0;
}

int onViewPic()
{
	return launchPicViewer(&pic,picpal,&picx,&picy,&picscale,false);
}

int launchPicViewer(BITMAP **pictoview, PALETTE pal, int *px, int *py, double *scale, bool isviewingmap)
{
  restore_mouse();
  BITMAP *buf;
  bool done=false, redraw=true;

  go();
  scare_mouse();
  clear_bitmap(screen);

  if(!*pictoview && (isviewingmap ? load_the_map() : load_the_pic(pictoview,pal)))
  {
    set_palette(RAMpal);
    unscare_mouse();
    comeback();
    return D_O_K;
  }
  get_bw(pal,pblack,pwhite);

  buf = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
  if(!buf)
  {
    jwin_alert("Error","Error creating temp bitmap",NULL,NULL,"OK",NULL,13,27,lfont);
    return D_O_K;
  }

  //  go();
  //  scare_mouse();
  //  clear_bitmap(screen);
  set_palette(pal);

  do
  {
    if(redraw)
    {
      clear_to_color(buf,pblack);
      stretch_blit(*pictoview,buf,0,0,(*pictoview)->w,(*pictoview)->h,
                   int(zq_screen_w+(*px-(*pictoview)->w)* *scale)/2,int(zq_screen_h+(*py-(*pictoview)->h)* *scale)/2,
                   int((*pictoview)->w* *scale),int((*pictoview)->h* *scale));

      if(vp_showpal)
        for(int i=0; i<256; i++)
          rectfill(buf,((i&15)<<2)+256,((i>>4)<<2)+176,((i&15)<<2)+259,((i>>4)<<2)+179,i);
      if(vp_showsize)
      {
        //        text_mode(pblack);
        textprintf_ex(buf,font,0,232,pwhite,pblack,"%dx%d %.2f%%",(*pictoview)->w,(*pictoview)->h,*scale*100.0);
      }
      custom_vsync();
      blit(buf,screen,0,0,0,0,zq_screen_w,zq_screen_h);
      redraw=false;
    }

    int step = 4;
    if(*scale < 1.0)
      step = int(4.0/ *scale);
    if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      step <<= 2;
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      step = 1;

    if(key[KEY_UP])    { *py+=step; redraw=true; }
    if(key[KEY_DOWN])  { *py-=step; redraw=true; }
    if(key[KEY_LEFT])  { *px+=step; redraw=true; }
    if(key[KEY_RIGHT]) { *px-=step; redraw=true; }

    if(keypressed() && !redraw)
      switch(readkey()>>8)
      {
        case KEY_PGUP:  *scale*=0.95; if(*scale<0.1) *scale=0.1; redraw=true; break;
        case KEY_PGDN:  *scale/=0.95; if(*scale>5.0) *scale=5.0; redraw=true; break;
        case KEY_HOME:  *scale/=2.0; if(*scale<0.1) *scale=0.1; redraw=true; break;
        case KEY_END:   *scale*=2.0; if(*scale>5.0) *scale=5.0; redraw=true; break;
        case KEY_TILDE: *scale=0.5; redraw=true; break;
        case KEY_Z:     *px=(*pictoview)->w-zq_screen_w; *py=(*pictoview)->h-zq_screen_h; vp_center=false; redraw=true; break;

        case KEY_1:     *scale=1.0; redraw=true; break;
        case KEY_2:     *scale=2.0; redraw=true; break;
        case KEY_3:     *scale=3.0; redraw=true; break;
        case KEY_4:     *scale=4.0; redraw=true; break;
        case KEY_5:     *scale=5.0; redraw=true; break;
        case KEY_C:     *px=*py=0; redraw=vp_center=true; break;
        case KEY_S:     vp_showsize = !vp_showsize; redraw=true; break;
        case KEY_D:     vp_showpal = !vp_showpal; redraw=true; break;
        case KEY_P:		if(isviewingmap) break;
		case KEY_ESC:   done=true; break;
        case KEY_SPACE:
			if(isviewingmap ? load_the_map() : load_the_pic(pictoview,pal)==2)
        {
          done=true;
        }
        else
        {
          redraw=true;
          gui_bg_color = pblack;
          gui_fg_color = pwhite;
          *scale=1.0;
          set_palette(pal);
        }
        get_bw(pal,pblack,pwhite);
        break;
      }
  } while(!done);

  destroy_bitmap(buf);
  set_palette(RAMpal);
  unscare_mouse();

  comeback();
  position_mouse_z(0);
  return D_O_K;
}

static DIALOG loadmap_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 48,   72,   224+1,  112+1,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "View Map" },
  { d_dummy_proc,      160,  80,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       80,   96+2,   96,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Res - Size" },
  // 3
  { jwin_radio_proc,      64,   104+4,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1/4 - 352KB" },
  { jwin_radio_proc,      64,   112+6,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1/2 - 1.4MB" },
  { jwin_radio_proc,      64,   120+8,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full- 5.5MB" },
  { jwin_text_proc,       192,  96+2,   96+1,   8+1,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Options" },
  // 7
  { jwin_check_proc,      192,  104+4,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Walk" },
  { jwin_check_proc,      192,  112+6,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Flags" },
  { jwin_check_proc,      192,  120+8,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Dark" },
  { jwin_check_proc,      192,  128+10,  96+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Items" },
  // 11
  { jwin_button_proc,     90,   152,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  152,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int load_the_map()
{
  static int res = 1;
  static int flags = cDEBUG;

  loadmap_dlg[0].dp2    = lfont;
  loadmap_dlg[3].flags  = (res==2) ? D_SELECTED : 0;
  loadmap_dlg[4].flags  = (res==1) ? D_SELECTED : 0;
  loadmap_dlg[5].flags  = (res==0) ? D_SELECTED : 0;
  loadmap_dlg[7].flags  = (flags&cWALK)   ? D_SELECTED : 0;
  loadmap_dlg[8].flags  = (flags&cFLAGS)  ? D_SELECTED : 0;
  loadmap_dlg[9].flags  = (flags&cNODARK) ? 0 : D_SELECTED;
  loadmap_dlg[10].flags = (flags&cNOITEM) ? 0 : D_SELECTED;

  if(zc_popup_dialog(loadmap_dlg,11) != 11)
  {
    return 1;
  }

  flags = cDEBUG;
  if(loadmap_dlg[3].flags&D_SELECTED)  res=2;
  if(loadmap_dlg[4].flags&D_SELECTED)  res=1;
  if(loadmap_dlg[5].flags&D_SELECTED)  res=0;
  if(loadmap_dlg[7].flags&D_SELECTED)  flags|=cWALK;
  if(loadmap_dlg[8].flags&D_SELECTED)  flags|=cFLAGS;
  if(!(loadmap_dlg[9].flags&D_SELECTED))  flags|=cNODARK;
  if(!(loadmap_dlg[10].flags&D_SELECTED)) flags|=cNOITEM;

  if(map)
  {
    destroy_bitmap(map);
  }


  map = create_bitmap_ex(8,(256*16)>>res,(176*8)>>res);

  if(!map)
  {
    jwin_alert("Error","Error creating bitmap.",NULL,NULL,"OK",NULL,13,27,lfont);
    return 2;
  }

  for(int y=0; y<8; y++)
  {
    for(int x=0; x<16; x++)
    {
      Map.draw(screen2, 0, 0, flags, -1, y*16+x);
      stretch_blit(screen2, map, 0, 0, 256, 176, x<<(8-res), (y*176)>>res, 256>>res,176>>res);
    }
  }

  memcpy(mappal,RAMpal,sizeof(RAMpal));
  vp_showpal = false;
  get_bw(picpal,pblack,pwhite);
  mapx = mapy = 0;
  mapscale = 0.125*(1<<res);
  imagepath[0] = 0;
  return 0;
}

int onViewMap()
{
  int temp_aligns=ShowMisalignments;
  ShowMisalignments=0;
  /*if(load_the_map()==0)
  {*/
    launchPicViewer(&map,mappal,&mapx, &mapy, &mapscale,true);
  //}
  ShowMisalignments=temp_aligns;
  return D_O_K;
}

/*
  int onTestBox() {
  char box_text[30];
  box_start(0, "hi", lfont, sfont);
  for (int i=0; i<30; i++) {
  sprintf(box_text, "%d", i);
  box_out(box_text); box_out("     "); box_out(box_text); box_out("     "); box_out(box_text); box_out("     "); box_eol();
  rest(100);
  }
  box_end(true);
  return D_O_K;
  }
  */
static char *dirstr[4] = {"Up","Down","Left","Right"};
char _pathstr[25]="Up,Up,Up,Up";

char *pathstr(byte path[])
{
  sprintf(_pathstr,"%s,%s,%s,%s",dirstr[path[0]],dirstr[path[1]],
          dirstr[path[2]],dirstr[path[3]]);
  return _pathstr;
}

void textprintf_disabled(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color_hl, int color_sh, AL_CONST char *format, ...)
{
  char buf[512];
  va_list ap;
  ASSERT(bmp);
  ASSERT(f);
  ASSERT(format);

  va_start(ap, format);
  uvszprintf(buf, sizeof(buf), format, ap);
  va_end(ap);


  textout_ex(bmp, f, buf, x+1, y+1, color_hl, -1);

  textout_ex(bmp, f, buf, x, y, color_sh, -1);
}

void textprintf_centre_disabled(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color_hl, int color_sh, AL_CONST char *format, ...)
{
  char buf[512];
  va_list ap;
  ASSERT(bmp);
  ASSERT(f);
  ASSERT(format);

  va_start(ap, format);
  uvszprintf(buf, sizeof(buf), format, ap);
  va_end(ap);

  textout_centre_ex(bmp, f, buf, x+1, y+1, color_hl, -1);
  textout_centre_ex(bmp, f, buf, x, y, color_sh, -1);
}

char *strip_extra_spaces(char *string)
{
  int len=strlen(string);
  char *src=(char *)malloc(len+1);
  char *tmpsrc=src;
  memcpy(src,string,len+1);
  memset(src,0,len+1);

  for (unsigned int i=0; string[i]&&i<=strlen(string); i++)
  {
    *tmpsrc=string[i];
    if (*tmpsrc==' ')
    {
      while(string[i+1]==' '&&i<=strlen(string))
      {
        i++;
      }
    }
    tmpsrc++;
  }
  memcpy(string,src,len);
  return string;
}

void drawpanel(int panel)
{
  mapscr *scr=Map.CurrScr();
  int NextCombo=combobuf[Combo].nextcombo;
  int NextCSet=combobuf[Combo].nextcset;
  if(prv_mode)
  {
    jwin_draw_frame(menu1,0,panel_8_y,panel_8_x+panel_1_w, panel_1_h, FR_WIN);
    rectfill(menu1,panel_8_x,panel_8_y+2,panel_8_x+panel_1_w-3,panel_8_y+panel_1_h-3,jwin_pal[jcBOX]);
  }
  else
  {
    jwin_draw_frame(menu1,0,panel_1_y,panel_1_x+panel_1_w,panel_1_h, FR_WIN);
    rectfill(menu1,panel_1_x,panel_1_y+2,panel_1_x+panel_1_w-3,panel_1_y+panel_1_h-3,jwin_pal[jcBOX]);
    if (showpanelcomboscrollers)
    {
      jwin_draw_frame(menu1,comboscroller_x,comboscroller_y,comboscroller_w,comboscroller_h,FR_ETCHED);
      for (int i=0; i<3; i++)
      {
        _allegro_hline(menu1,comboscroller_x+5-i,comboscroller_y+4+i, comboscroller_x+5+i, vc(0));
      }
      jwin_draw_frame(menu1,comboscroller_x,comboscroller_y+comboscroller_h-2,comboscroller_w,comboscroller_h,FR_ETCHED);
      for (int i=0; i<3; i++)
      {
        _allegro_hline(menu1,comboscroller_x+5-i,comboscroller_y+comboscroller_h+4-i, comboscroller_x+5+i, vc(0));
      }
    }
    textprintf_disabled(menu1,spfont,panel_1_x+panel_1_w-7,panel_1_y+3,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"%d",menutype+1);
    switch(panel)
    {
      case m_block:
      {
        char name[256], shortname[256];
        strncpy(name,get_filename(filepath),255);
        if (name[0]==0)
        {
          sprintf(name, "[Untitled]");
        }
        strip_extra_spaces(name);
        shorten_string(shortname, name, pfont, 255, (panel_1_x+panel_1_w-86)-(panel_1_x+1)-4);
        set_clip_rect(menu1,panel_1_x,panel_1_y,panel_1_x+panel_1_w-5,panel_1_y+46);
        extract_name(filepath,name,FILENAME8__);
        textprintf_disabled(menu1,pfont,panel_1_x+1,panel_1_y+3,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"File:");
        textprintf_ex(menu1,pfont,panel_1_x+1,panel_1_y+13,vc(0),-1,"%s",shortname);
        textprintf_disabled(menu1,pfont,panel_1_x+1,panel_1_y+24,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Combo:");
        textprintf_ex(menu1,pfont,panel_1_x+1+text_length(pfont, "Combo: "),panel_1_y+24,vc(0),-1,"%d",Combo);
        textprintf_disabled(menu1,pfont,panel_1_x+1,panel_1_y+34,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Type:");
        textprintf_ex(menu1,pfont,panel_1_x+1+text_length(pfont, "Type: "),panel_1_y+34,vc(0),-1,"%s",combotype_string[combobuf[Combo].type]);

        textprintf_centre_disabled(menu1,spfont,panel_1_x+panel_1_w-76,panel_1_y+3,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Combo");
        jwin_draw_frame(menu1,panel_1_x+panel_1_w-86,panel_1_y+9,20, 20, FR_DEEP);
        put_combo(menu1,panel_1_x+panel_1_w-84,panel_1_y+11,Combo,CSet,0,0);

        textprintf_centre_disabled(menu1,spfont,panel_1_x+panel_1_w-52,panel_1_y+3,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Walk");
        jwin_draw_frame(menu1,panel_1_x+panel_1_w-62,panel_1_y+9,20, 20, FR_DEEP);
        put_combo(menu1,panel_1_x+panel_1_w-60,panel_1_y+11,Combo,CSet,0,0);
        put_walkflags(menu1,panel_1_x+panel_1_w-60,panel_1_y+11,Combo);

        textprintf_centre_disabled(menu1,spfont,panel_1_x+panel_1_w-28,panel_1_y+3,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Next");
        jwin_draw_frame(menu1,panel_1_x+panel_1_w-38,panel_1_y+9,20, 20, FR_DEEP);
        if (NextCombo>0)
        {
          put_combo(menu1,panel_1_x+panel_1_w-36,panel_1_y+11,NextCombo,NextCSet,0,0);
        }
        else
        {
          for (int dy=0; dy<16; dy++)
          {
            for (int dx=0; dx<16; dx++)
            {
              menu1->line[dy+panel_1_y+11][dx+panel_1_x+panel_1_w-36]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
            }
          }
        }

        textprintf_disabled(menu1,spfont,panel_1_x+panel_1_w-28,panel_1_y+32,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"CSet");
        textprintf_ex(menu1,spfont,panel_1_x+panel_1_w-10,panel_1_y+32,jwin_pal[jcBOXFG],-1,"%d", CSet);

        textprintf_disabled(menu1,spfont,panel_1_x+panel_1_w-32,panel_1_y+39,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Layer");
        textprintf_ex(menu1,spfont,panel_1_x+panel_1_w-10,panel_1_y+39,jwin_pal[jcBOXFG],-1,"%d", CurrentLayer);
      }
      break;
      case m_coords:
        set_clip_rect(menu1,panel_2_x,panel_2_y,panel_2_x+panel_2_w-5,panel_2_y+46);
        for (int i=0; i<4; i++)
        {
          jwin_draw_frame(menu1,panel_2_x+14+(32*i),panel_2_y+4,20,20,FR_DEEP);
          blit(icon_bmp[i][coord_frame], menu1, 0, 0, panel_2_x+16+(32*i),panel_2_y+6, 16, 16);
        }

        textprintf_centre_ex(menu1,font,panel_2_x+24+0*32,panel_2_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->itemx);
        textprintf_centre_ex(menu1,font,panel_2_x+24+1*32,panel_2_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->stairx);
        //textprintf_centre_ex(menu1,font,panel_2_x+24+2*32,panel_2_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturnx);
        textprintf_centre_ex(menu1,font,panel_2_x+24+2*32,panel_2_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warparrivalx);
        textprintf_centre_ex(menu1,font,panel_2_x+24+3*32,panel_2_y+26,jwin_pal[jcBOXFG],-1,"%d",Flag);



        textprintf_centre_ex(menu1,font,panel_2_x+24+0*32,panel_2_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->itemy);
        textprintf_centre_ex(menu1,font,panel_2_x+24+1*32,panel_2_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->stairy);
        //textprintf_centre_ex(menu1,font,panel_2_x+24+2*32,panel_2_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturny);
        textprintf_centre_ex(menu1,font,panel_2_x+24+2*32,panel_2_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warparrivaly);

        break;
      case m_coords2:
        set_clip_rect(menu1,panel_9_x,panel_9_y,panel_9_x+panel_9_w-5,panel_9_y+46);
        for (int i=0; i<4; i++)
        {
          jwin_draw_frame(menu1,panel_9_x+14+(32*i),panel_9_y+4,20,20,FR_DEEP);
          blit(icon_bmp[ICON_BMP_RETURN_A+i][coord_frame], menu1, 0, 0, panel_9_x+16+(32*i),panel_9_y+6, 16, 16);
        }

        textprintf_centre_ex(menu1,font,panel_9_x+24+0*32,panel_9_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturnx[0]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+1*32,panel_9_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturnx[1]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+2*32,panel_9_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturnx[2]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+3*32,panel_9_y+26,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturnx[3]);
        //textprintf_centre_ex(menu1,font,panel_9_x+24+4*32,panel_9_y+26,jwin_pal[jcBOXFG],-1,"%d",Flag);



        textprintf_centre_ex(menu1,font,panel_9_x+24+0*32,panel_9_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturny[0]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+1*32,panel_9_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturny[1]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+2*32,panel_9_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturny[2]);
        textprintf_centre_ex(menu1,font,panel_9_x+24+3*32,panel_9_y+34,jwin_pal[jcBOXFG],-1,"%d",scr->warpreturny[3]);

        break;
      case m_flags:
      {
        set_clip_rect(menu1,panel_3_x,panel_3_y,panel_3_x+panel_3_w-5,panel_3_y+46);

        byte f=scr->flags;
        byte wf=scr->flags2;
        byte f3=scr->flags3;
        char *flagheader="ESWSLEDHETGSOMLWRDIB";
        char flagdata[30];
        for (byte i=0; i<strlen(flagheader); ++i)
        {
          textprintf_centre_disabled(menu1,font,panel_3_x+37+(i*6),panel_3_y+6,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"%c",flagheader[i]);
        }
        textprintf_disabled(menu1,font,panel_3_x+5,panel_3_y+14,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Flags:");

        sprintf(flagdata,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",bit(f3,7),bit(f3,6),bit(f3,5),bit(f3,4),bit(f3,3),bit(f3,2),bit(f3,1),bit(f3,0),bit(wf,7),bit(wf,6),bit(wf,5),bit(wf,4),bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0));
        for (byte i=0; i<strlen(flagheader); ++i)
        {
          textprintf_centre_ex(menu1,font,panel_3_x+37+(i*6),panel_3_y+14,jwin_pal[jcBOXFG],-1,"%c",flagdata[i]);
        }
        f=scr->enemyflags;
        char *enemyflagheader="BILFR24Z";
        char enemyflagdata[30];
        for (byte i=0; i<strlen(enemyflagheader); ++i)
        {
          textprintf_centre_disabled(menu1,font,panel_3_x+43+(i*6),panel_3_y+26,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"%c",enemyflagheader[i]);
        }
        textprintf_disabled(menu1,font,panel_3_x+5,panel_3_y+34,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Enemy:");
        sprintf(enemyflagdata,"%d%d%d%d%d%d%d%d",bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0));
        for (byte i=0; i<strlen(enemyflagheader); ++i)
        {
          textprintf_centre_ex(menu1,font,panel_3_x+43+(i*6),panel_3_y+34,jwin_pal[jcBOXFG],-1,"%c",enemyflagdata[i]);
        }
        textprintf_disabled(menu1,font,panel_3_x+101,panel_3_y+26,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Pattern:");
        textprintf_ex(menu1,font,panel_3_x+101,panel_3_y+34,jwin_pal[jcBOXFG],-1,"%s",short_pattern_string[(Map.CurrScr()->pattern)]);
      }
      break;
      case m_guy:
      {
        set_clip_rect(menu1,panel_4_x,panel_4_y,panel_4_x+panel_4_w-5,panel_4_y+46);
        char buf[73], shortbuf[73];
        strncpy(buf,MsgString(scr->str),72);
        buf[72] = '\0';
        if ((scr->str)==0)
        {
          sprintf(buf, "(None)");
        }
        strip_extra_spaces(buf);
        shorten_string(shortbuf, buf, pfont, 72, 140);
        textprintf_disabled(menu1,pfont,panel_4_x+6,panel_1_y+8,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Guy:");
        textprintf_disabled(menu1,pfont,panel_4_x+6,panel_1_y+16,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"String:");
        textprintf_disabled(menu1,pfont,panel_4_x+6,panel_1_y+24,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Room:");
        textprintf_ex(menu1,pfont,panel_4_x+40-16,panel_4_y+8,jwin_pal[jcBOXFG],-1,"%s",guy_string[scr->guy]);
        textprintf_ex(menu1,pfont,panel_4_x+40-6,panel_4_y+16,jwin_pal[jcBOXFG],-1,"%s",shortbuf);
        textprintf_ex(menu1,pfont,panel_4_x+40-10,panel_4_y+24,jwin_pal[jcBOXFG],-1,"%s",roomtype_string[scr->room]);
        int rtype=scr->room;
        if(strcmp(catchall_string[rtype]," "))
        {
          textprintf_disabled(menu1,pfont,panel_4_x+6,panel_1_y+32,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"%s:",catchall_string[rtype]);
          int xofs=text_length(pfont,catchall_string[rtype])+5;
          switch(rtype)
          {
            case rSP_ITEM:
            textprintf_ex(menu1,pfont,panel_4_x+7+xofs,panel_4_y+32,jwin_pal[jcBOXFG],-1,"%s",item_string[scr->catchall]);
            break;
            default:
            textprintf_ex(menu1,pfont,panel_4_x+7+xofs,panel_4_y+32,jwin_pal[jcBOXFG],-1,"%d",scr->catchall);
          }
        }
      }
      break;
      case m_warp:
        set_clip_rect(menu1,panel_5_x,panel_5_y,panel_5_x+panel_5_w-5,panel_5_y+46);

        textprintf_disabled(menu1,font,panel_5_x+7,panel_5_y+6,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Tile Warp:");
        textprintf_disabled(menu1,font,panel_5_x+7,panel_5_y+14,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Type:");
        textprintf_disabled(menu1,font,panel_5_x+7,panel_5_y+26,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Side Warp:");
        textprintf_disabled(menu1,font,panel_5_x+7,panel_5_y+34,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Type:");
        textprintf_ex(menu1,font,panel_5_x+59,panel_5_y+6,jwin_pal[jcBOXFG],-1,"%d:%02X",Map.CurrScr()->tilewarpdmap[0],scr->tilewarpscr[0]);
        textprintf_ex(menu1,font,panel_5_x+59,panel_5_y+14,jwin_pal[jcBOXFG],-1,"%s",warptype_string[scr->tilewarptype[0]]);

        textprintf_ex(menu1,font,panel_5_x+59,panel_5_y+26,jwin_pal[jcBOXFG],-1,"%d:%02X",Map.CurrScr()->sidewarpdmap[0],scr->sidewarpscr[0]);
        textprintf_ex(menu1,font,panel_5_x+59,panel_5_y+34,jwin_pal[jcBOXFG],-1,"%s",warptype_string[scr->sidewarptype[0]]);
        break;
        case m_misc:
        {
          set_clip_rect(menu1,panel_6_x,panel_6_y,panel_6_x+panel_6_w-5,panel_6_y+46);

          textprintf_disabled(menu1,font,panel_6_x+7,panel_6_y+14,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Triggers:");
          byte wf=scr->flags2;
          char *triggerheader="TBLR";
          char triggerdata[30];
          for (byte i=0; i<strlen(triggerheader); ++i)
          {
            textprintf_centre_disabled(menu1,font,panel_6_x+57+(i*6),panel_6_y+6,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"%c",triggerheader[i]);
          }
          sprintf(triggerdata,"%d%d%d%d",bit(wf,0),bit(wf,1),bit(wf,2),bit(wf,3));
          for (byte i=0; i<strlen(triggerheader); ++i)
          {
            textprintf_centre_ex(menu1,font,panel_6_x+57+(i*6),panel_6_y+14,jwin_pal[jcBOXFG],-1,"%c",triggerdata[i]);
          }

          textprintf_disabled(menu1,font,panel_6_x+7,panel_6_y+26,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Path:");
          textprintf_disabled(menu1,font,panel_6_x+7,panel_6_y+34,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Exit dir:");
          textprintf_ex(menu1,font,panel_6_x+54,panel_6_y+26,jwin_pal[jcBOXFG],-1,"%s",scr->flags&64?pathstr(scr->path):"(None)");
          textprintf_ex(menu1,font,panel_6_x+54,panel_6_y+34,jwin_pal[jcBOXFG],-1,"%s",scr->flags&64?dirstr[scr->exitdir]:"(None)");
        }
        break;
      case m_layers:
      {
        set_clip_rect(menu1,panel_7_x,panel_7_y,panel_7_x+panel_7_w-5,panel_7_y+46);

        textprintf_centre_disabled(menu1,font,panel_7_x+88,panel_7_y+2,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"Layers");
        textprintf_centre_disabled(menu1,font,panel_7_x+13,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"0");
        draw_checkbox(menu1,panel_7_x+9,panel_7_y+20,vc(1),vc(14), LayerMaskInt[0]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+38,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"1");
        draw_checkbox(menu1,panel_7_x+34,panel_7_y+20,vc(1),vc(14), LayerMaskInt[1]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+63,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"2");
        draw_checkbox(menu1,panel_7_x+59,panel_7_y+20,vc(1),vc(14), LayerMaskInt[2]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+88,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"3");
        draw_checkbox(menu1,panel_7_x+84,panel_7_y+20,vc(1),vc(14), LayerMaskInt[3]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+113,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"4");
        draw_checkbox(menu1,panel_7_x+109,panel_7_y+20,vc(1),vc(14), LayerMaskInt[4]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+138,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"5");
        draw_checkbox(menu1,panel_7_x+134,panel_7_y+20,vc(1),vc(14), LayerMaskInt[5]!=0);
        textprintf_centre_disabled(menu1,font,panel_7_x+163,panel_7_y+11,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"6");
        draw_checkbox(menu1,panel_7_x+159,panel_7_y+20,vc(1),vc(14), LayerMaskInt[6]!=0);
        draw_layerradio(menu1,panel_7_x+9,panel_7_y+30,vc(1),vc(14), CurrentLayer);

        textprintf_disabled(menu1,spfont,panel_7_x+panel_7_w-28,panel_7_y+36,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"CSet");
        textprintf_ex(menu1,spfont,panel_7_x+panel_7_w-10,panel_7_y+36,jwin_pal[jcBOXFG],-1,"%d", CSet);
      }
      break;
    }                                                         //switch(menutype)
  }
}

void refresh(int flags)
{
  // CPage = Map.CurrScr()->cpage;
  int curscr;
  if(flags&rCLEAR)
    clear_to_color(menu1,vc(0));

  if(flags&rMAP)
  {
    curscr=Map.getCurrScr();
    Map.setCurrScr(curscr);                                 // to update palette
    clear_to_color(mapscreenbmp,vc(0));
    Map.draw(mapscreenbmp, showedges?16:0, showedges?16:0, Flags, -1, -1);
    if (showedges)
    {
      if (Map.getCurrScr()<128)
      {
        //not the first row of screens
        if (Map.getCurrScr()>15)
        {
          Map.drawrow(mapscreenbmp, 16, 0, Flags, 160, -1, Map.getCurrScr()-16);
        }
        else
        {
          Map.drawstaticrow(mapscreenbmp, 16, 0);
        }
        //not the last row of screens
        if (Map.getCurrScr()<112)
        {
          Map.drawrow(mapscreenbmp, 16, 192, Flags, 0, -1, Map.getCurrScr()+16);
        }
        else
        {
          Map.drawstaticrow(mapscreenbmp, 16, 192);
        }
        //not the first column of screens
        if (Map.getCurrScr()&0x0F)
        {
          Map.drawcolumn(mapscreenbmp, 0, 16, Flags, 15, -1, Map.getCurrScr()-1);
        }
        else
        {
          Map.drawstaticcolumn(mapscreenbmp, 0, 16);
        }
        //not the last column of screens
        if ((Map.getCurrScr()&0x0F)<15)
        {
          Map.drawcolumn(mapscreenbmp, 272, 16, Flags, 0, -1, Map.getCurrScr()+1);
        }
        else
        {
          Map.drawstaticcolumn(mapscreenbmp, 272, 16);
        }
        //not the first row or first column of screens
        if ((Map.getCurrScr()>15)&&(Map.getCurrScr()&0x0F))
        {
          Map.drawblock(mapscreenbmp, 0, 0, Flags, 175, -1, Map.getCurrScr()-17);
        }
        else
        {
          Map.drawstaticblock(mapscreenbmp, 0, 0);
        }
        //not the first row or last column of screens
        if ((Map.getCurrScr()>15)&&((Map.getCurrScr()&0x0F)<15))
        {
          Map.drawblock(mapscreenbmp, 272, 0, Flags, 160, -1, Map.getCurrScr()-15);
        }
        else
        {
          Map.drawstaticblock(mapscreenbmp, 272, 0);
        }
        //not the last row or first column of screens
        if ((Map.getCurrScr()<112)&&(Map.getCurrScr()&0x0F))
        {
          Map.drawblock(mapscreenbmp, 0, 192, Flags, 15, -1, Map.getCurrScr()+15);
        }
        else
        {
          Map.drawstaticblock(mapscreenbmp, 0, 192);
        }
        //not the last row or last column of screens
        if ((Map.getCurrScr()<112)&&((Map.getCurrScr()&0x0F)<15))
        {
          Map.drawblock(mapscreenbmp, 272, 192, Flags, 0, -1, Map.getCurrScr()+17);
        }
        else
        {
          Map.drawstaticblock(mapscreenbmp, 272, 192);
        }
      }
    }
    if (showxypos_icon)
    {
      rectfill(mapscreenbmp,showxypos_x+(showedges?16:0),showxypos_y+(showedges?16:0),showxypos_x+(showedges?16:0)+15,showxypos_y+(showedges?16:0)+15,showxypos_color);
    }
    if (showxypos_cursor_icon)
    {
      rect(mapscreenbmp,showxypos_cursor_x+(showedges?16:0),showxypos_cursor_y+(showedges?16:0),showxypos_cursor_x+(showedges?16:0)+15,showxypos_cursor_y+(showedges?16:0)+15,vc(15));
    }
    if (mapscreensize==1)
    {
      blit(mapscreenbmp,menu1,0,0,mapscreen_x,mapscreen_y,mapscreenbmp->w,mapscreenbmp->h);
    }
    else
    {
      stretch_blit(mapscreenbmp,menu1,0,0,mapscreenbmp->w,mapscreenbmp->h,mapscreen_x,mapscreen_y,int(mapscreensize*mapscreenbmp->w),int(mapscreensize*mapscreenbmp->h));
    }
    if (showedges)
    {
      //top preview
      for(int j=0; j<int(16*mapscreensize); j++)
      {
        for(int i=0; i<298*mapscreensize; i++)
        {
          if(((i^j)&1)==0)
          {
            putpixel(menu1,mapscreen_x+i,mapscreen_y+j,vc(0));
          }
        }
      }
      //bottom preview
      for(int j=int(192*mapscreensize); j<int(208*mapscreensize); j++)
      {
        for(int i=0; i<298*mapscreensize; i++)
        {
          if(((i^j)&1)==0)
          {
            putpixel(menu1,mapscreen_x+i,mapscreen_y+j,vc(0));
          }
        }
      }
      //left preview
      for(int j=int(16*mapscreensize); j<int(192*mapscreensize); j++)
      {
        for(int i=0; i<16*mapscreensize; i++)
        {
          if(((i^j)&1)==0)
          {
            putpixel(menu1,mapscreen_x+i,mapscreen_y+j,vc(0));
          }
        }

      }
      //right preview
      for(int j=int(16*mapscreensize); j<int(192*mapscreensize); j++)
      {
        for(int i=int(272*mapscreensize); i<int(298*mapscreensize); i++)
        {
          if(((i^j)&1)==0)
          {
            putpixel(menu1,mapscreen_x+i,mapscreen_y+j,vc(0));
          }
        }
      }
    }
    if(!(Flags&cDEBUG))
    {
      for(int j=int(168*mapscreensize); j<int(176*mapscreensize); j++)
      {
        for(int i=0; i<int(256*mapscreensize); i++)
        {

          if(((i^j)&1)==0)
          {
            putpixel(menu1,int(mapscreen_x+(showedges?(16*mapscreensize):0)+i),
                     int(mapscreen_y+(showedges?(16*mapscreensize):0)+j),vc(blackout_color));
          }
        }
      }
    }
    if((Map.isDark()) && !(Flags&cNODARK))
    {
      for(int j=0; j<80*mapscreensize; j++)
      {
        for(int i=0; i<(80*mapscreensize)-j; i++)
        {
          if(((i^j)&1)==0)
          {
            putpixel(menu1,int(mapscreen_x+(showedges?(16*mapscreensize):0))+i,
                     int(mapscreen_y+(showedges?(16*mapscreensize):0)+j),vc(blackout_color));
          }
        }
      }
    }
    double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
    double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
    int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
    int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
    if(((ComboBrush && !ComboBrushPause)||draw_mode==dm_alias) && isinRect(gui_mouse_x(),gui_mouse_y(),startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
    {
      set_mouse_sprite(mouse_bmp[MOUSE_BMP_BLANK][0]);
      int mx=gui_mouse_x()/16*16;
      int my=gui_mouse_y()/16*16;
      clear_bitmap(brushscreen);
      int tempbw=BrushWidth;
      int tempbh=BrushHeight;
      if(draw_mode==dm_alias)
      {
        BrushWidth = combo_aliases[combo_apos].width+1;
        BrushHeight = combo_aliases[combo_apos].height+1;
      }
      if ((FloatBrush)&&(draw_mode!=dm_alias))
      {
        blit(brushbmp, brushscreen, 0, 0, mx-2, my-18, BrushWidth*16, BrushHeight*16);
        //shadow
        for(int i=0; i<SHADOW_DEPTH; i++)
        {
          for(int j=0; j<BrushHeight*16; j++)
          {
            if((((i^j)&1)==1) && (my+j)<192)
            {
              putpixel(brushscreen,mx+i+(BrushWidth*16)-SHADOW_DEPTH,my+j-16,vc(0));
            }
          }
        }
        for(int i=0; i<BrushWidth*16; i++)
        {
          for(int j=0; j<SHADOW_DEPTH; j++)
          {
            if((((i^j)&1)==1) && (mx+i)<256)
            {
              putpixel(brushscreen,mx+i,my+j+(BrushHeight*16)-SHADOW_DEPTH-16,vc(0));
            }
          }
        }
      }
      else
      {
        if(draw_mode!=dm_alias)
        {
          blit(brushbmp, brushscreen, 0, 0, mx, my-16, BrushWidth*16, BrushHeight*16);
        }
        else
        {
          combo_alias *combo = &combo_aliases[combo_apos];
          switch(alias_origin) {
            case 0:
            blit(brushbmp, brushscreen, 0, 0, mx, my-16, BrushWidth*16, BrushHeight*16); break;
            case 1:
            blit(brushbmp, brushscreen, (mx-(combo->width+1)*16)+16<0?((combo->width+1)*16)-mx-16:0, 0, max((mx-(combo->width+1)*16)+16,0), my-16, BrushWidth*16, BrushHeight*16); break;
            case 2:
            blit(brushbmp, brushscreen, 0, ((my-16)-(combo->height+1)*16)+16<0?((combo->height+1)*16)-my:0, mx, max(((my-16)-(combo->height+1)*16)+16,0), BrushWidth*16, BrushHeight*16); break;
            case 3:
            blit(brushbmp, brushscreen, (mx-(combo->width+1)*16)+16<0?((combo->width+1)*16)-mx-16:0, ((my-16)-(combo->height+1)*16)+16<0?((combo->height+1)*16)-my:0, max((mx-(combo->width+1)*16)+16,0), max(((my-16)-(combo->height+1)*16)+16,0), BrushWidth*16, BrushHeight*16); break;
          }
        }
      }
      masked_blit(brushscreen, menu1, 0, 0, 0, 16, 256, 176);
      BrushWidth=tempbw;
      BrushHeight=tempbh;
    }
    if (ShowGrid)
    {
      for (int x=16; x<256; x+=16)
      {
        _allegro_vline(menu1, x, 0+16, 175+16, vc(GridColor));
      }
      for (int y=16; y<176; y+=16)
      {
        _allegro_hline(menu1, 0, y+16, 255, vc(GridColor));
      }
    }
  }
  if(flags&rSCRMAP)
  {
    //  text_mode(vc(0));
    rectfill(menu1, minimap_x-1, minimap_y-1,minimap_x+minimap_w-1,minimap_y+minimap_h-1,jwin_pal[jcBOX]);
    jwin_draw_frame(menu1,minimap_x,minimap_y+9,54, 33, FR_DEEP);
    rectfill(menu1, minimap_x+27, minimap_y+12+24,minimap_x+50,minimap_y+12+26,vc(0));
    _allegro_hline(menu1, minimap_x+2,    minimap_y+11,    minimap_x+2+49,  vc(0));
    _allegro_hline(menu1, minimap_x+2,    minimap_y+12+27, minimap_x+2+49,  vc(0));
    _allegro_vline(menu1, minimap_x+2,    minimap_y+12,    minimap_y+12+26, vc(0));
    _allegro_vline(menu1, minimap_x+2+49, minimap_y+12,    minimap_y+12+26, vc(0));
    if(Map.getCurrMap()<Map.getMapCount())
    {
      for(int i=0; i<MAPSCRS; i++)
      {
        if (Map.Scr(i)->valid&mVALID)
        {
          //vc(0)
          rectfill(menu1,(i&15)*3+minimap_x+3,(i/16)*3+minimap_y+12,(i&15)*3+2+minimap_x+3,(i/16)*3+minimap_y+12+2, lc1((Map.Scr(i)->color)&15));
          if(((Map.Scr(i)->color)&15)>0)
          {
            putpixel(menu1,(i&15)*3+1+minimap_x+3,(i/16)*3+minimap_y+12+1,lc2((Map.Scr(i)->color)&15));
          }
        }
        else
        {
          for (int dy=0; dy<3; dy++)
          {
            for (int dx=0; dx<3; dx++)
            {
              menu1->line[dy+(i/16)*3+minimap_y+12][dx+(i&15)*3+minimap_x+3]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
            }
          }
        }
      }
      int s=Map.getCurrScr();
      rect(menu1,(s&15)*3+minimap_x+3,(s/16)*3+minimap_y+12,(s&15)*3+2+minimap_x+3,(s/16)*3+minimap_y+12+2,vc(15));

      textprintf_disabled(menu1,font,minimap_x,minimap_y,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"M");
      textprintf_ex(menu1,font,minimap_x+8,minimap_y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-3d",Map.getCurrMap()+1);

      textprintf_disabled(menu1,font,minimap_x+36,minimap_y,jwin_pal[jcLIGHT],jwin_pal[jcMEDDARK],"S");
      textprintf_ex(menu1,font,minimap_x+43,minimap_y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%02X",s);
    }
  }
  if(flags&rCOMBOS)
  {
    if(draw_mode!=dm_alias)
    {
      int drawmap, drawscr;
      drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
      drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      for(int i=0; i<(combolist_w*combolist_h); i++)
      {
        put_combo(menu1,(i&(combolist_w-1))*16+combolist_x,(i/combolist_w)*16+combolist_y,i+First,CSet,0,0);
      }
      int rect_pos=Combo-First;
      if((rect_pos>=0)&&(rect_pos<(First+(combolist_w*combolist_h))))
        rect(menu1,(rect_pos&(combolist_w-1))*16+combolist_x,(rect_pos/combolist_w)*16+combolist_y,((rect_pos&(combolist_w-1))*16+combolist_x)+15,((rect_pos/combolist_w)*16+combolist_y)+15,255);
    }
    else
    {
      for(int i=0; i<(combolist_w*combolist_ha); i++)
      {
        //        put_combo(menu1,(i&(combolist_w-1))*16+combolist_x,(i/combolist_w)*16+combolist_y,combo_aliases[combo_alistpos+i].combos[0],combo_aliases[combo_alistpos+i].csets[0],0,0);
        draw_combo_alias_thumbnail(menu1, &combo_aliases[combo_alistpos+i], (i&(combolist_w-1))*16+combolist_x,(i/combolist_w)*16+combolist_y);
      }
      //      int i=0;
      BITMAP *prv = create_bitmap_ex(8,64,64);
      clear_bitmap(prv);
      int scalefactor = 1;
      if((combo_aliases[combo_apos].width>7)||(combo_aliases[combo_apos].height>7))
      {
        scalefactor=4;
      }
      else if((combo_aliases[combo_apos].width>3)||(combo_aliases[combo_apos].height>3))
        {
          scalefactor=2;
        }
        stretch_blit(brushbmp, prv, 0,0,scalefactor*64,min(scalefactor*64,176),0,0,64,scalefactor==4?44:64);
      blit(prv,menu1,0,0,256,176,64,64);
      int rect_pos=combo_apos-combo_alistpos;
      if((rect_pos>=0)&&(rect_pos<(combo_alistpos+(combolist_w*combolist_ha))))
        rect(menu1,(rect_pos&(combolist_w-1))*16+combolist_x,(rect_pos/combolist_w)*16+combolist_y,((rect_pos&(combolist_w-1))*16+combolist_x)+15,((rect_pos/combolist_w)*16+combolist_y)+15,255);
    }
  }
  if(flags&rCOMBO)
  {
    int drawmap, drawscr;
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    put_combo(menu1,304,0,(draw_mode==dm_alias)?combo_aliases[combo_apos].combos[0]:Combo,(draw_mode==dm_alias)?combo_aliases[combo_apos].csets[0]:CSet,0,0);
  }
  if(flags&rMENU)
  {
    if (!showallpanels)
    {
      drawpanel(menutype);
    }
    else
    {
      for (int i=0; i<m_menucount; i++)
      {
        drawpanel(i);
      }
    }
    set_clip_rect(menu1,0,0,zq_screen_w-1,zq_screen_h-1);
  }                                                         //if(flags&rMENU)

  if(zq_showpal)
  {
    for(int i=0; i<256; i++)
    {
      rectfill(menu1,((i&15)<<2)+256,((i>>4)<<2)+176,((i&15)<<2)+259,((i>>4)<<2)+179,i);
    }
  }

  if(ShowFPS)
  {
    textprintf_shadowed_ex(menu1,sfont,0,prv_mode?32:16,vc(15),vc(0),-1,"FPS:%-3d",lastfps);
  }
  if(prv_mode)
  {
    textout_shadowed_ex(menu1,sfont,"Preview Mode",0,16,vc(15),vc(0),-1);
    if(prv_twon)
    {
      textprintf_shadowed_ex(menu1,sfont,0,24,vc(15),vc(0),-1,"T Warp=%d tics", Map.get_prvtime());
    }
  }

  if(draw_mode!=dm_normal)
  {
    textout_shadowed_right_ex(menu1,sfont,dm_names[draw_mode],255,16,vc(15),vc(0),-1);
  }

  scare_mouse();
  if(flags&rCLEAR)
  {
    blit(menu1,screen,0,0,0,0,zq_screen_w,zq_screen_h);
  }
  else
  {
    blit(menu1,screen,0,16,0,16,zq_screen_w,zq_screen_h-16);
    if(flags&rCOMBO)
    {
      blit(menu1,screen,304,0,304,0,16,16);
    }
  }

  ComboBrushPause=0;
  unscare_mouse();
  SCRFIX();
}

void select_scr()
{
  if(Map.getCurrMap()>=Map.getMapCount())
    return;
  int tempcb=ComboBrush;
  ComboBrush=0;
  //scooby
  while(gui_mouse_b())
  {
    int x=gui_mouse_x(); if(x>minimap_x+3+47)   x=minimap_x+3+47;       if(x<minimap_x+3)   x=minimap_x+3;
    int y=gui_mouse_y(); if(y>minimap_y+9+3+26) y=minimap_y+9+3+26;     if(y<minimap_y+9+3) y=minimap_y+9+3;
    int s=(((y-(minimap_y+9+3))/3)<<4)+((x-(minimap_x+3))/3);

    if(s>=MAPSCRS)
      s-=16;
    if(s!=Map.getCurrScr())
    {
      Map.setCurrScr(s);
      //      vsync();
      //      refresh(rALL);
    }
    do_animations();
  }
  ComboBrush=tempcb;
}

void select_combo()
{
  int tempcb=ComboBrush;
  ComboBrush=0;
  while(gui_mouse_b())
  {
    int x=gui_mouse_x(); if(x<combolist_x) x=combolist_x; if(x>combolist_x+(combolist_w*16)-1) x=combolist_x+(combolist_w*16)-1;
    int y=gui_mouse_y(); if(y<combolist_y) y=combolist_y; if(y>combolist_y+(combolist_h*16)-1) y=combolist_y+(combolist_h*16)-1;
    Combo=(((y-combolist_y)>>4)*combolist_w)+((x-combolist_x)>>4)+First;
    do_animations();
    //    vsync();
    //    refresh(rALL);
  }
  ComboBrush=tempcb;
}

void select_comboa()
{
  int tempcb=ComboBrush;
  ComboBrush=0;
  while(gui_mouse_b())
  {
    int x=gui_mouse_x(); if(x<combolist_x) x=combolist_x; if(x>combolist_x+(combolist_w*16)-1) x=combolist_x+(combolist_w*16)-1;
    int y=gui_mouse_y(); if(y<combolist_y) y=combolist_y; if(y>combolist_y+(combolist_ha*16)-1) y=combolist_y+(combolist_ha*16)-1;
    combo_apos=(((y-combolist_y)>>4)*combolist_w)+((x-combolist_x)>>4)+combo_alistpos;
    do_animations();
  }
  ComboBrush=tempcb;
}

void update_combobrush()
{
  clear_bitmap(brushbmp);
  if(draw_mode==dm_alias)
  {
    //int count=(combo_aliases[combo_apos].width+1)*(combo_aliases[combo_apos].height+1)*(comboa_lmasktotal(combo_aliases[combo_apos].layermask));
    for(int z=0;z<=comboa_lmasktotal(combo_aliases[combo_apos].layermask);z++)
    {
      for(int y=0;y<=combo_aliases[combo_apos].height;y++)
      {
        for(int x=0;x<=combo_aliases[combo_apos].width;x++)
        {
          int position = ((y*(combo_aliases[combo_apos].width+1))+x)+((combo_aliases[combo_apos].width+1)*(combo_aliases[combo_apos].height+1)*z);
          if(combo_aliases[combo_apos].combos[position])
          {
            if(z==0)
            {
              putcombo(brushbmp,x<<4,y<<4,combo_aliases[combo_apos].combos[position],combo_aliases[combo_apos].csets[position]);
            }
            else
            {
              overcombo(brushbmp,x<<4,y<<4,combo_aliases[combo_apos].combos[position],combo_aliases[combo_apos].csets[position]);
            }
          }
        }
      }
    }
    switch(alias_origin)
    {
      case 0:
      //if(!(combo_aliases[combo_apos].combos[0]))
      textprintf_shadowed_ex(brushbmp, sfont, 6, 6, vc(15), vc(0), -1, "x"); break;
      case 1:
      //if(!(combo_aliases[combo_apos].combos[combo_aliases[combo_apos].width]))
      textprintf_shadowed_ex(brushbmp, sfont, 6+(combo_aliases[combo_apos].width*16), 6, vc(15), vc(0), -1, "x"); break;
      case 2:
      //if(!(combo_aliases[combo_apos].combos[(combo_aliases[combo_apos].width+1)*combo_aliases[combo_apos].height]))
      textprintf_shadowed_ex(brushbmp, sfont, 6, 6+(combo_aliases[combo_apos].height*16), vc(15), vc(0), -1, "x"); break;
      case 3:
      //if(!(combo_aliases[combo_apos].combos[(combo_aliases[combo_apos].width+1)*(combo_aliases[combo_apos].height)-1]))
      textprintf_shadowed_ex(brushbmp, sfont, 6+(combo_aliases[combo_apos].width*16), 6+(combo_aliases[combo_apos].height*16), vc(15), vc(0), -1, "x"); break;
    }
  }
  else
  {
    if(combo_cols==false)
    {
      for(int i=0; i<256; i++)
      {
        if (((i%COMBOS_PER_ROW)<BrushWidth)&&((i/COMBOS_PER_ROW)<BrushHeight)) {
          put_combo(brushbmp,(i%COMBOS_PER_ROW)<<4,(i/COMBOS_PER_ROW)<<4,Combo+i,CSet,0,0);
        }
      }
    }
    else
    {
      int c = 0;

      for(int i=0; i<256; i++)
      {
        if (((i%COMBOS_PER_ROW)<BrushWidth)&&((i/COMBOS_PER_ROW)<BrushHeight)) {
          put_combo(brushbmp,(i%COMBOS_PER_ROW)<<4,(i/COMBOS_PER_ROW)<<4,Combo+c,CSet,0,0);
        }
        ++c;
        if((i&3)==3)
          c+=48;
        if((i%COMBOS_PER_ROW)==(COMBOS_PER_ROW-1))
          c-=256;
      }
    }
  }
}

byte relational_source_grid[256]=
{
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  16, 16, 17, 17, 18, 18, 19, 19, 16, 16, 17, 17, 18, 18, 19, 19,
  20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23,
  24, 24, 24, 24, 25, 25, 25, 25, 24, 24, 24, 24, 25, 25, 25, 25,
  26, 27, 26, 27, 26, 27, 26, 27, 28, 29, 28, 29, 28, 29, 28, 29,
  30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
  31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32,
  33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
  34, 35, 36, 37, 34, 35, 36, 37, 34, 35, 36, 37, 34, 35, 36, 37,
  38, 38, 39, 39, 38, 38, 39, 39, 38, 38, 39, 39, 38, 38, 39, 39,
  40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
  41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
  42, 43, 42, 43, 42, 43, 42, 43, 42, 43, 42, 43, 42, 43, 42, 43,
  44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
  45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46
};


void draw()
{
  saved=false;
  Map.Ugo();
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }

  if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
  {
    Map.CurrScr()->valid|=mVALID;
    Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
    Map.setcolor(Color);
  }
  refresh(rMAP+rSCRMAP);
  while(gui_mouse_b())
  {
    int x=gui_mouse_x();
    int y=gui_mouse_y();
    double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
    double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
    int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
    int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
    if(isinRect(x,y,startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
    {
      int cxstart=(x-startxint)/int(16*mapscreensize);
      int cystart=(y-startyint)/int(16*mapscreensize);
      int cstart=(cystart*16)+cxstart;
      combo_alias *combo = &combo_aliases[combo_apos];
      switch (draw_mode)
      {
        case dm_normal:
        {
          int cc=Combo;
          if (!combo_cols)
          {
            for (int cy=0; cy+cystart<11&&cy<BrushHeight; cy++)
            {
              for (int cx=0; cx+cxstart<16&&cx<BrushWidth; cx++)
              {
                int c=cstart+(cy*16)+cx;
                if (!(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
                {
                  Map.AbsoluteScr(drawmap, drawscr)->data[c]=cc+cx;
                }
                Map.AbsoluteScr(drawmap, drawscr)->cset[c]=CSet;
              }
              cc+=20;
            }
          }
          else
          {
            int p=Combo/256;
            int pc=Combo%256;
            for (int cy=0; cy+cystart<11&&cy<BrushHeight; cy++)
            {
              for (int cx=0; cx+cxstart<16&&cx<BrushWidth; cx++)
              {
                int c=cstart+(cy*16)+cx;
                cc=((cx/4)*52)+(cy*4)+(cx%4)+pc;
                if (cc>=0&&cc<256)
                {
                  cc+=(p*256);
                  Map.AbsoluteScr(drawmap, drawscr)->data[c]=cc;
                  Map.AbsoluteScr(drawmap, drawscr)->cset[c]=CSet;
                }
              }
            }
          }
          update_combobrush();
        }
        break;
        case dm_relational:
        {
          int c2,c3;
          int cx, cy, cx2, cy2;
          cy=cstart>>4;
          cx=cstart&15;
          if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
          {
            relational_tile_grid[(cy+rtgyo)][cx+rtgxo]=1;
            Map.AbsoluteScr(drawmap, drawscr)->data[cstart]=Combo+47;
            Map.AbsoluteScr(drawmap, drawscr)->cset[cstart]=CSet;
          }
          else
          {
            relational_tile_grid[(cy+rtgyo)][cx+rtgxo]=0;
          }
          for (int y2=-1; y2<2; ++y2)
          {
            cy2=cy+y2;
            if ((cy2>11)||(cy2<0))
            {
              continue;
            }
            for (int x2=-1; x2<2; ++x2)
            {
              cx2=cx+x2;
              if ((cx2>15)||(cx2<0))
              {
                continue;
              }
              c2=cstart+(y2*16)+x2;
              c3=((relational_tile_grid[((cy2-1)+rtgyo)][(cx2+1)+rtgxo]?1:0)<<0)+
                ((relational_tile_grid[((cy2-1)+rtgyo)][(cx2-1)+rtgxo]?1:0)<<1)+
                ((relational_tile_grid[((cy2+1)+rtgyo)][(cx2-1)+rtgxo]?1:0)<<2)+
                ((relational_tile_grid[((cy2+1)+rtgyo)][(cx2+1)+rtgxo]?1:0)<<3)+
                ((relational_tile_grid[(( cy2 )+rtgyo)][(cx2+1)+rtgxo]?1:0)<<4)+
                ((relational_tile_grid[((cy2-1)+rtgyo)][( cx2 )+rtgxo]?1:0)<<5)+
                ((relational_tile_grid[(( cy2 )+rtgyo)][(cx2-1)+rtgxo]?1:0)<<6)+
                ((relational_tile_grid[((cy2+1)+rtgyo)][( cx2 )+rtgxo]?1:0)<<7);
              if (relational_tile_grid[((c2>>4)+rtgyo)][(c2&15)+rtgxo]==0)
              {
                Map.AbsoluteScr(drawmap, drawscr)->data[c2]=Combo+relational_source_grid[c3];
                Map.AbsoluteScr(drawmap, drawscr)->cset[c2]=CSet;
              }
            }
          }
        }
        break;
        case dm_dungeon:
        {
          int c2,c3,c4;
          int cx, cy, cx2, cy2;
          cy=cstart>>4;
          cx=cstart&15;
          if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
          {
            relational_tile_grid[(cy+rtgyo)][cx+rtgxo]=0;
            for (int y2=-1; y2<2; ++y2)
            {
              cy2=cy+y2;
              if ((cy2>11)||(cy2<0))
              {
                continue;
              }
              for (int x2=-1; x2<2; ++x2)
              {
                cx2=cx+x2;
                if ((cx2>15)||(cx2<0))
                {
                  continue;
                }
                if (relational_tile_grid[(cy2+rtgyo)][cx2+rtgxo]!=0)
                {
                  relational_tile_grid[(cy2+rtgyo)][cx2+rtgxo]=1;
                };
              }
            }
            Map.AbsoluteScr(drawmap, drawscr)->data[cstart]=Combo;
            Map.AbsoluteScr(drawmap, drawscr)->cset[cstart]=CSet;
          }
          else
          {
            relational_tile_grid[(cy+rtgyo)][cx+rtgxo]=2;
            for (int y2=-1; y2<2; ++y2)
            {
              cy2=cy+y2;
              if ((cy2>11)||(cy2<0))
              {
                continue;
              }
              for (int x2=-1; x2<2; ++x2)
              {
                cx2=cx+x2;
                if ((cx2>15)||(cx2<0))
                {
                  continue;
                }
                if (relational_tile_grid[(cy2+rtgyo)][cx2+rtgxo]==0)
                {
                  relational_tile_grid[(cy2+rtgyo)][cx2+rtgxo]=1;
                };
              }
            }
            Map.AbsoluteScr(drawmap, drawscr)->data[cstart]=Combo+48+47;
            Map.AbsoluteScr(drawmap, drawscr)->cset[cstart]=CSet;
          }
          for (int y2=0; y2<11; ++y2)
          {
            for (int x2=0; x2<16; ++x2)
            {
              c2=(y2*16)+x2;
              c4=relational_tile_grid[((y2)+rtgyo)][(x2)+rtgxo];
              c3=(((relational_tile_grid[((y2-1)+rtgyo)][(x2+1)+rtgxo]>c4)?1:0)<<0)+
                (((relational_tile_grid[((y2-1)+rtgyo)][(x2-1)+rtgxo]>c4)?1:0)<<1)+
                (((relational_tile_grid[((y2+1)+rtgyo)][(x2-1)+rtgxo]>c4)?1:0)<<2)+
                (((relational_tile_grid[((y2+1)+rtgyo)][(x2+1)+rtgxo]>c4)?1:0)<<3)+
                (((relational_tile_grid[(( y2 )+rtgyo)][(x2+1)+rtgxo]>c4)?1:0)<<4)+
                (((relational_tile_grid[((y2-1)+rtgyo)][( x2 )+rtgxo]>c4)?1:0)<<5)+
                (((relational_tile_grid[(( y2 )+rtgyo)][(x2-1)+rtgxo]>c4)?1:0)<<6)+
                (((relational_tile_grid[((y2+1)+rtgyo)][( x2 )+rtgxo]>c4)?1:0)<<7);
              if (relational_tile_grid[(y2+rtgyo)][x2+rtgxo]<2)
              {
                Map.AbsoluteScr(drawmap, drawscr)->data[c2]=Combo+relational_source_grid[c3]+(48*c4);
                Map.AbsoluteScr(drawmap, drawscr)->cset[c2]=CSet;
              }
            }
          }
        }
        break;
        case dm_alias:
        if (!combo->layermask)
        {
          int ox=0, oy=0;
          switch(alias_origin)
          {
            case 0:
            ox=0; oy=0; break;
            case 1:
            ox=(combo->width); oy=0; break;
            case 2:
            ox=0; oy=(combo->height); break;
            case 3:
            ox=(combo->width); oy=(combo->height); break;
          }
          for (int cy=0; cy-oy+cystart<11&&cy<=combo->height; cy++)
          {
            for (int cx=0; cx-ox+cxstart<16&&cx<=combo->width; cx++)
            {
              if((cx+cxstart-ox>=0)&&(cy+cystart-oy>=0))
              {
                int c=cstart+((cy-oy)*16)+cx-ox;
                int p=(cy*(combo->width+1))+cx;
                if(combo->combos[p])
                {
                  Map.AbsoluteScr(drawmap, drawscr)->data[c]=combo->combos[p];
                  Map.AbsoluteScr(drawmap, drawscr)->cset[c]=combo->csets[p];
                }
              }
            }
          }
        }
        else
        {
          int amap=0, ascr=0;
          int lcheck = 1;
          int laypos = 0;
          int ox=0, oy=0;
          switch(alias_origin)
          {
            case 0:
            ox=0; oy=0; break;
            case 1:
            ox=(combo->width); oy=0; break;
            case 2:
            ox=0; oy=(combo->height); break;
            case 3:
            ox=(combo->width); oy=(combo->height); break;
          }
          for (int cz=0; cz<7; cz++, lcheck<<=1)
          {
            if(!cz)
            {
              amap = Map.getCurrMap();
              ascr = Map.getCurrScr();
            }
            else
            {
              if(cz==1) lcheck>>=1;
              if(combo->layermask&lcheck)
              {
                amap = Map.CurrScr()->layermap[cz-1]-1;
                ascr = Map.CurrScr()->layerscreen[cz-1];
                laypos++;
              }
            }
            for (int cy=0; cy-oy+cystart<11&&cy<=combo->height; cy++)
            {
              for (int cx=0; cx-ox+cxstart<16&&cx<=combo->width; cx++)
              {
                if((!cz)||/*(Map.CurrScr()->layermap[cz>0?cz-1:0])*/amap>=0)
                {
                  if((cz==0)||(combo->layermask&lcheck))
                  {
                    if((cx+cxstart-ox>=0)&&(cy+cystart-oy>=0))
                    {
                      int c=cstart+((cy-oy)*16)+cx-ox;
                      int p=((cy*(combo->width+1))+cx)+((combo->width+1)*(combo->height+1)*laypos);
                      if((combo->combos[p])&&(amap>=0)) {
                        Map.AbsoluteScr(amap, ascr)->data[c]=combo->combos[p];
                        Map.AbsoluteScr(amap, ascr)->cset[c]=combo->csets[p];
                      }
                    }
                  }
                }
              }
            }
          }
        }
        break;
      }
    }
    do_animations();
  }
}




void replace(int c)
{
  saved=false;
  Map.Ugo();
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }

  int targetcombo = Map.AbsoluteScr(drawmap, drawscr)->data[c];
  int targetcset  = Map.AbsoluteScr(drawmap, drawscr)->cset[c];

  if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
  {
    for(int i=0; i<176; i++)
    {
      if ((Map.AbsoluteScr(drawmap, drawscr)->cset[i])==targetcset)
      {
        Map.AbsoluteScr(drawmap, drawscr)->cset[i]=CSet;
      }
    }
  }
  else
  {
    for(int i=0; i<176; i++)
    {
      if (((Map.AbsoluteScr(drawmap, drawscr)->data[i])==targetcombo) &&
          ((Map.AbsoluteScr(drawmap, drawscr)->cset[i])==targetcset) )
      {
        Map.AbsoluteScr(drawmap, drawscr)->data[i]=Combo;
        Map.AbsoluteScr(drawmap, drawscr)->cset[i]=CSet;
      }
    }
  }
  refresh(rMAP);
}

void draw_block(int start,int w,int h)
{
  saved=false;
  Map.Ugo();
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }

  if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
  {
    Map.CurrScr()->valid|=mVALID;
    Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
    Map.setcolor(Color);
  }
  for(int y=0; y<h && (y<<4)+start < 176; y++)
    for(int x=0; x<w && (start&15)+x < 16; x++)
    {
      Map.AbsoluteScr(drawmap, drawscr)->data[start+(y<<4)+x]=Combo+(y*4)+x;
      Map.AbsoluteScr(drawmap, drawscr)->cset[start+(y<<4)+x]=CSet;

    }
  refresh(rMAP+rSCRMAP);
}

void fill(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal, bool only_cset)
{
  if (!only_cset)
  {
    if ((fillscr->data[((sy<<4)+sx)])!=targetcombo)
      return;
  }
  if ((fillscr->cset[((sy<<4)+sx)])!=targetcset)
    return;
  if (!only_cset)
  {
    fillscr->data[((sy<<4)+sx)]=Combo;
  }
  fillscr->cset[((sy<<4)+sx)]=CSet;
  if((sy>0) && (dir!=down))                                 // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx)]&0x7FF)==target))
    fill(fillscr, targetcombo, targetcset, sx, sy-1, up, diagonal, only_cset);
  if((sy<10) && (dir!=up))                                  // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx)]&0x7FF)==target))
    fill(fillscr, targetcombo, targetcset, sx, sy+1, down, diagonal, only_cset);
  if((sx>0) && (dir!=right))                                // && ((Map.CurrScr()->data[((sy<<4)+sx-1)]&0x7FF)==target))
    fill(fillscr, targetcombo, targetcset, sx-1, sy, left, diagonal, only_cset);
  if((sx<15) && (dir!=left))                                // && ((Map.CurrScr()->data[((sy<<4)+sx+1)]&0x7FF)==target))
    fill(fillscr, targetcombo, targetcset, sx+1, sy, right, diagonal, only_cset);
  if (diagonal==1)
  {
    if((sy>0) && (sx>0) && (dir!=r_down))                   // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx-1)]&0x7FF)==target))
      fill(fillscr, targetcombo, targetcset, sx-1, sy-1, l_up, diagonal, only_cset);
    if((sy<10) && (sx<15) && (dir!=l_up))                   // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx+1)]&0x7FF)==target))
      fill(fillscr, targetcombo, targetcset, sx+1, sy+1, r_down, diagonal, only_cset);
    if((sx>0) && (sy<10) && (dir!=r_up))                    // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx-1)]&0x7FF)==target))
      fill(fillscr, targetcombo, targetcset, sx-1, sy+1, l_down, diagonal, only_cset);
    if((sx<15) && (sy>0) && (dir!=l_down))                  // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx+1)]&0x7FF)==target))
      fill(fillscr, targetcombo, targetcset, sx+1, sy-1, r_up, diagonal, only_cset);
  }

}


void fill2(mapscr* fillscr, int targetcombo, int targetcset, int sx, int sy, int dir, int diagonal, bool only_cset)
{
  if (!only_cset)
  {
    if((fillscr->data[((sy<<4)+sx)])==targetcombo)
      return;
  }
  if((fillscr->cset[((sy<<4)+sx)])==targetcset)
    return;
  if (!only_cset)
  {
    fillscr->data[((sy<<4)+sx)]=Combo;
  }
  fillscr->cset[((sy<<4)+sx)]=CSet;
  if((sy>0) && (dir!=down))                                 // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx)]&0x7FF)!=target))
    fill2(fillscr, targetcombo, targetcset, sx, sy-1, up, diagonal, only_cset);
  if((sy<10) && (dir!=up))                                  // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx)]&0x7FF)!=target))
    fill2(fillscr, targetcombo, targetcset, sx, sy+1, down, diagonal, only_cset);
  if((sx>0) && (dir!=right))                                // && ((Map.CurrScr()->data[((sy<<4)+sx-1)]&0x7FF)!=target))
    fill2(fillscr, targetcombo, targetcset, sx-1, sy, left, diagonal, only_cset);
  if((sx<15) && (dir!=left))                                // && ((Map.CurrScr()->data[((sy<<4)+sx+1)]&0x7FF)!=target))
    fill2(fillscr, targetcombo, targetcset, sx+1, sy, right, diagonal, only_cset);
  if (diagonal==1)
  {
    if((sy>0) && (sx>0) && (dir!=r_down))                   // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx-1)]&0x7FF)!=target))
      fill2(fillscr, targetcombo, targetcset, sx-1, sy-1, l_up, diagonal, only_cset);
    if((sy<10) && (sx<15) && (dir!=l_up))                   // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx+1)]&0x7FF)!=target))
      fill2(fillscr, targetcombo, targetcset, sx+1, sy+1, r_down, diagonal, only_cset);
    if((sx>0) && (sy<10) && (dir!=r_up))                    // && ((Map.CurrScr()->data[(((sy+1)<<4)+sx-1)]&0x7FF)!=target))
      fill2(fillscr, targetcombo, targetcset, sx-1, sy+1, l_down, diagonal, only_cset);
    if((sx<15) && (sy>0) && (dir!=l_down))                  // && ((Map.CurrScr()->data[(((sy-1)<<4)+sx+1)]&0x7FF)!=target))
      fill2(fillscr, targetcombo, targetcset, sx+1, sy-1, r_up, diagonal, only_cset);
  }
}

#include "zq_tiles.h"                                       // tile and combo code

/**************************/
/*****     Mouse      *****/
/**************************/

void doxypos(byte &px,byte &py,int color,int mask)
{
  int tempcb=ComboBrush;
  ComboBrush=0;
  scare_mouse();
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_POINT_BOX][0]);
  unscare_mouse();

  int oldpx=px, oldpy=py;
  double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
  double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
  int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
  int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
  showxypos_x=px;
  showxypos_y=py;
  showxypos_color=vc(color);
  showxypos_icon=true;
  bool canedit=false;
  bool done=false;
  while(!done && !(gui_mouse_b()&2))
  {
    //    rectfill(screen,px*mapscreensize+startxint,py*mapscreensize+startyint,px*mapscreensize+startxint+(16*mapscreensize-1),py*mapscreensize+startyint+(16*mapscreensize-1),vc(color));
    int x=gui_mouse_x();
    int y=gui_mouse_y();

    if(!gui_mouse_b())
    {
      canedit=true;
    }
    if(canedit && gui_mouse_b()==1 && isinRect(x,y,startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
    {
      scare_mouse();
      zq_set_mouse_range(startxint,startyint,int(startxint+(256*mapscreensize)-1),int(startyint+(176*mapscreensize)-1));
      //      blit(screen,menu1,0,0,0,0,zq_screen_w,zq_screen_h);
      while(gui_mouse_b()==1)
      {
        /*
          do_animations();
          x=gui_mouse_x();
          y=gui_mouse_y();
          blit(menu1,screen,0,16,0,16,256,176);
          rect(screen, x&mask, y&mask, (x&mask)+15, (y&mask)+15, vc(15));
          */
        x=int((gui_mouse_x()-(showedges?int(16*mapscreensize):0))/mapscreensize);
        y=int((gui_mouse_y()-16-(showedges?int(16*mapscreensize):0))/mapscreensize);
        showxypos_cursor_icon=true;
        showxypos_cursor_x=x&mask;
        showxypos_cursor_y=y&mask;
        do_animations();
        refresh(rALL);
        textprintf_ex(screen,font,700,500,vc(15),vc(0),"%d %d %d %d",startxint,startyint,int(startxint+(256*mapscreensize)-1),int(startyint+(176*mapscreensize)-1));
        textprintf_ex(screen,font,700,510,vc(15),vc(0),"%d %d %d %d %d %d",x,y,gui_mouse_x(),gui_mouse_y(),showxypos_cursor_x,showxypos_cursor_y);
        //        draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y);
        //        draw_sprite(screen,mouse_bmp[3],x&mask,y&mask);
      }
      if(gui_mouse_b()==0)
      {
        px=byte(x&mask);
        py=byte(y&mask);
        //        rectfill(menu1,px,py+16,px+15,py+31,vc(color));
      }
      //      blit(menu1,screen,0,16,0,16,256,176);
      zq_set_mouse_range(0,0,zq_screen_w-1,zq_screen_h-1);
      unscare_mouse();
      done=true;
    }
    if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ESC:
        case KEY_ENTER:
        goto finished;
      }
    }
    do_animations();
    refresh(rALL);
  }

finished:
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  refresh(rMAP+rMENU);
  while(gui_mouse_b());
  showxypos_x=-1000;
  showxypos_y=-1000;
  showxypos_color=-1000;
  showxypos_icon=false;
  showxypos_cursor_x=-1000;
  showxypos_cursor_y=-1000;
  showxypos_cursor_icon=false;

  if (px!=oldpx||py!=oldpy)
  {
    saved=false;
  }
  ComboBrush=tempcb;
}

void doflags()
{
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_FLAG][0]);
  int of=Flags;
  Flags=cFLAGS;
  refresh(rMAP);

  bool canedit=false;
  while(!(gui_mouse_b()&2))
  {
    int x=gui_mouse_x();
    int y=gui_mouse_y();
    double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
    double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
    int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
    int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
    int cx=(x-startxint)/int(16*mapscreensize);
    int cy=(y-startyint)/int(16*mapscreensize);
    int c=(cy*16)+cx;

    if(!gui_mouse_b())
      canedit=true;
    if(canedit && gui_mouse_b()==1 && isinRect(x,y,startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
    {
      saved=false;
      if (CurrentLayer==0)
      {
        Map.CurrScr()->sflag[c]=Flag;
      }
      else
      {
        TheMaps[(Map.CurrScr()->layermap[CurrentLayer-1]-1)*MAPSCRS+(Map.CurrScr()->layerscreen[CurrentLayer-1])].sflag[c]=Flag;
        //      Map.CurrScr()->sflag[c]=Flag;
      }
      refresh(rMAP);
    }
    if (mouse_z)
    {
      for (int i=0; i<abs(mouse_z); ++i)
      {
        if (mouse_z>0)
        {
          onTimes();
        }
        else
        {
          onDivide();
        }
      }
      position_mouse_z(0);
    }
    if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ESC:
        case KEY_ENTER:     goto finished;
        case KEY_ASTERISK:  onTimes();  break;
        case KEY_SLASH_PAD: onDivide(); break;
        case KEY_UP:        onUp();     break;
        case KEY_DOWN:      onDown();   break;
        case KEY_LEFT:      onLeft();   break;
        case KEY_RIGHT:     onRight();  break;
        case KEY_PGUP:      onPgUp();   break;
        case KEY_PGDN:      onPgDn();   break;
        case KEY_COMMA:     onDecMap(); break;
        case KEY_STOP:      onIncMap(); break;
      }
    }
    do_animations();
    refresh(rALL);
  }

finished:
  Flags=of;
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  refresh(rMAP+rMENU);
  while(gui_mouse_b());
}

void peek(int c)
{
  char buf1[40],buf2[60],buf3[40];
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  sprintf(buf1,"Pos:  %-3d  Combo: %-3d", c, Map.AbsoluteScr(drawmap, drawscr)->data[c]);
  sprintf(buf2,"CSet: %-3d  Flags: %-3d, %-3d", Map.AbsoluteScr(drawmap, drawscr)->cset[c], Map.CurrScr()->sflag[c], combobuf[Map.CurrScr()->data[c]].flag);
  sprintf(buf3,"Combo type: %-9s",combotype_string[combobuf[(Map.AbsoluteScr(drawmap, drawscr)->data[c])].type]);
  jwin_alert("Combo Data",buf1,buf2,buf3,"OK",NULL,13,27,lfont);
}

void set_brush_width(int width);
void set_brush_height(int height);

int set_brush_width_1() {set_brush_width(1); return D_O_K;}
int set_brush_width_2() {set_brush_width(2); return D_O_K;}
int set_brush_width_3() {set_brush_width(3); return D_O_K;}
int set_brush_width_4() {set_brush_width(4); return D_O_K;}
int set_brush_width_5() {set_brush_width(5); return D_O_K;}
int set_brush_width_6() {set_brush_width(6); return D_O_K;}
int set_brush_width_7() {set_brush_width(7); return D_O_K;}
int set_brush_width_8() {set_brush_width(8); return D_O_K;}
int set_brush_width_9() {set_brush_width(9); return D_O_K;}
int set_brush_width_10() {set_brush_width(10); return D_O_K;}
int set_brush_width_11() {set_brush_width(11); return D_O_K;}
int set_brush_width_12() {set_brush_width(12); return D_O_K;}
int set_brush_width_13() {set_brush_width(13); return D_O_K;}
int set_brush_width_14() {set_brush_width(14); return D_O_K;}
int set_brush_width_15() {set_brush_width(15); return D_O_K;}
int set_brush_width_16() {set_brush_width(16); return D_O_K;}

int set_brush_height_1() {set_brush_height(1); return D_O_K;}
int set_brush_height_2() {set_brush_height(2); return D_O_K;}
int set_brush_height_3() {set_brush_height(3); return D_O_K;}
int set_brush_height_4() {set_brush_height(4); return D_O_K;}
int set_brush_height_5() {set_brush_height(5); return D_O_K;}
int set_brush_height_6() {set_brush_height(6); return D_O_K;}
int set_brush_height_7() {set_brush_height(7); return D_O_K;}
int set_brush_height_8() {set_brush_height(8); return D_O_K;}
int set_brush_height_9() {set_brush_height(9); return D_O_K;}
int set_brush_height_10() {set_brush_height(10); return D_O_K;}
int set_brush_height_11() {set_brush_height(11); return D_O_K;}

static MENU brush_width_menu[] =
{
  { "1",          set_brush_width_1,   NULL },
  { "2",          set_brush_width_2,   NULL },
  { "3",          set_brush_width_3,   NULL },
  { "4",          set_brush_width_4,   NULL },
  { "5",          set_brush_width_5,   NULL },
  { "6",          set_brush_width_6,   NULL },
  { "7",          set_brush_width_7,   NULL },
  { "8",          set_brush_width_8,   NULL },
  { "9",          set_brush_width_9,   NULL },
  { "10",          set_brush_width_10,   NULL },
  { "11",          set_brush_width_11,   NULL },
  { "12",          set_brush_width_12,   NULL },
  { "13",          set_brush_width_13,   NULL },
  { "14",          set_brush_width_14,   NULL },
  { "15",          set_brush_width_15,   NULL },
  { "16",          set_brush_width_16,   NULL },
  { NULL }
};

static MENU brush_height_menu[] =
{

  { "1",          set_brush_height_1,   NULL },
  { "2",          set_brush_height_2,   NULL },
  { "3",          set_brush_height_3,   NULL },
  { "4",          set_brush_height_4,   NULL },
  { "5",          set_brush_height_5,   NULL },
  { "6",          set_brush_height_6,   NULL },
  { "7",          set_brush_height_7,   NULL },
  { "8",          set_brush_height_8,   NULL },
  { "9",          set_brush_height_9,   NULL },
  { "10",         set_brush_height_10,  NULL },
  { "11",         set_brush_height_11,  NULL },
  { NULL }
};

int set_flood();
int set_fill_4();
int set_fill_8();
int set_fill2_4();
int set_fill2_8();

void flood()
{
  // int start=0, w=0, h=0;
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  saved=false;
  Map.Ugo();

  if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
  {
    Map.CurrScr()->valid|=mVALID;
    Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
    Map.setcolor(Color);
  }
  /* for(int y=0; y<h && (y<<4)+start < 176; y++)
    for(int x=0; x<w && (start&15)+x < 16; x++)
    */
  if (!(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
  {
    for(int i=0; i<176; i++)
    {
      Map.AbsoluteScr(drawmap, drawscr)->data[i]=Combo;
    }
  }
  for(int i=0; i<176; i++)
  {
    Map.AbsoluteScr(drawmap, drawscr)->cset[i]=CSet;
  }
  refresh(rMAP+rSCRMAP);
}

void fill_4()
{
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  int x=gui_mouse_x();
  int y=gui_mouse_y();
  int by= (((y&0xF0)-16)>>4);
  int bx= (x>>4);

  if ((Map.AbsoluteScr(drawmap,drawscr)->data[(by<<4)+bx]!=Combo)
      || (Map.AbsoluteScr(drawmap,drawscr)->cset[(by<<4)+bx]!=CSet))
  {
    saved=false;
    Map.Ugo();
    if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
    {
      Map.CurrScr()->valid|=mVALID;
      Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
      Map.setcolor(Color);
    }
    fill(Map.AbsoluteScr(drawmap, drawscr),
         (Map.AbsoluteScr(drawmap, drawscr)->data[(by<<4)+bx]),
         (Map.AbsoluteScr(drawmap, drawscr)->cset[(by<<4)+bx]), bx, by, 255, 0, (key[KEY_LSHIFT]||key[KEY_RSHIFT]));
    refresh(rMAP+rSCRMAP);
  }
}

void fill_8()
{
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  int x=gui_mouse_x();
  int y=gui_mouse_y();
  int by= (((y&0xF0)-16)>>4);
  int bx= (x>>4);
  if ((Map.AbsoluteScr(drawmap,drawscr)->data[(by<<4)+bx]!=Combo)
      || (Map.AbsoluteScr(drawmap,drawscr)->cset[(by<<4)+bx]!=CSet))
  {
    saved=false;
    Map.Ugo();
    if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
    {
      Map.CurrScr()->valid|=mVALID;
      Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
      Map.setcolor(Color);
    }
    fill(Map.AbsoluteScr(drawmap, drawscr),
         (Map.AbsoluteScr(drawmap, drawscr)->data[(by<<4)+bx]),
         (Map.AbsoluteScr(drawmap, drawscr)->cset[(by<<4)+bx]), bx, by, 255, 1, (key[KEY_LSHIFT]||key[KEY_RSHIFT]));
    refresh(rMAP+rSCRMAP);
  }
}

void fill2_4()
{
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }

  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  int x=gui_mouse_x();
  int y=gui_mouse_y();
  int by= (((y&0xF0)-16)>>4);
  int bx= (x>>4);

  saved=false;
  Map.Ugo();
  if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
  {
    Map.CurrScr()->valid|=mVALID;
    Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
    Map.setcolor(Color);
  }
  fill2(Map.AbsoluteScr(drawmap, drawscr), Combo, CSet, bx, by, 255, 0, (key[KEY_LSHIFT]||key[KEY_RSHIFT]));
  refresh(rMAP+rSCRMAP);
}

void fill2_8()
{
  int drawmap, drawscr;
  if (CurrentLayer==0)
  {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  }
  else
  {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0)
    {
      return;
    }
  }
  int x=gui_mouse_x();
  int y=gui_mouse_y();
  int by= (((y&0xF0)-16)>>4);
  int bx= (x>>4);

  saved=false;
  Map.Ugo();
  if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
  {
    Map.CurrScr()->valid|=mVALID;
    Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
    Map.setcolor(Color);
  }
  fill2(Map.AbsoluteScr(drawmap, drawscr), Combo, CSet, bx, by, 255, 1, (key[KEY_LSHIFT]||key[KEY_RSHIFT]));
  refresh(rMAP+rSCRMAP);
}

static MENU fill_menu[] =
{
  { "Flood",                   set_flood,   NULL },
  { "Fill (4-way)",            set_fill_4,  NULL },
  { "Fill (8-way)",            set_fill_8,  NULL },
  { "Fill2 (4-way)",           set_fill2_4, NULL },
  { "Fill2 (8-way)",           set_fill2_8, NULL },
  { NULL }
};

int set_flood()
{
  for (int x=0; x<5; x++)
  {
    fill_menu[x].flags=0;
  }
  fill_menu[0].flags=D_SELECTED;
  fill_type=0;
  return D_O_K;
}

int set_fill_4()
{
  for (int x=0; x<5; x++)
  {
    fill_menu[x].flags=0;
  }
  fill_menu[1].flags=D_SELECTED;
  fill_type=1;
  return D_O_K;
}

int set_fill_8()
{
  for (int x=0; x<5; x++)
  {
    fill_menu[x].flags=0;
  }
  fill_menu[2].flags=D_SELECTED;
  fill_type=2;
  return D_O_K;
}

int set_fill2_4()
{
  for (int x=0; x<5; x++)
  {
    fill_menu[x].flags=0;
  }
  fill_menu[3].flags=D_SELECTED;
  fill_type=3;
  return D_O_K;
}

int set_fill2_8()
{
  for (int x=0; x<5; x++)
  {
    fill_menu[x].flags=0;
  }
  fill_menu[4].flags=D_SELECTED;
  fill_type=4;
  return D_O_K;
}

static MENU draw_rc_menu[] =
{
  { "Draw 1x2 Block",          NULL,  NULL },
  { "Draw 2x1 Block",          NULL,  NULL },
  { "Draw 2x2 Block",          NULL,  NULL },
  { "Draw 4x2 Block",          NULL,  NULL },
  { "" },
  { "Set Brush Width\t ",      NULL,  brush_width_menu },
  { "Set Brush Height\t ",     NULL,  brush_height_menu },
  { "" },
  { "Set Fill type\t ",        NULL,  fill_menu },
  { "" },
  { "Replace",                 NULL,  NULL },
  { "Properties",              NULL,  NULL },
  { "Select Combo",            NULL,  NULL },
  { "Scroll to Combo",         NULL,  NULL },
  { "Edit Combo",              NULL,  NULL },
  { NULL }
};

void set_brush_width(int width)
{
  for (int x=0; x<16; x++)
  {
    brush_width_menu[x].flags=0;
  }
  BrushWidth=width;
  brush_width_menu[width-1].flags=D_SELECTED;
  refresh(rALL);
}

void set_brush_height(int height)
{
  for (int x=0; x<11; x++)
  {
    brush_height_menu[x].flags=0;
  }
  BrushHeight=height;
  brush_height_menu[height-1].flags=D_SELECTED;
  refresh(rALL);
}

void restore_mouse()
{
  ComboBrushPause=1;
  scare_mouse();
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  unscare_mouse();
}

static int comboa_cnt=0;
static int layer_cnt=0;

void domouse()
{
  static bool mouse_down = false;
  static int scrolldelay = 0;
  int x=gui_mouse_x();

  int y=gui_mouse_y();
  double startx=mapscreen_x+(showedges?(16*mapscreensize):0);
  double starty=mapscreen_y+(showedges?(16*mapscreensize):0);
  int startxint=mapscreen_x+(showedges?int(16*mapscreensize):0);
  int startyint=mapscreen_y+(showedges?int(16*mapscreensize):0);
  int cx=(x-startxint)/int(16*mapscreensize);
  int cy=(y-startyint)/int(16*mapscreensize);
  int c=(cy*16)+cx;

  int redraw=0;

  update_combobrush();
  //  put_combo(brushbmp,0,0,Combo,CSet,0,0);

  if(!isinRect(x,y,startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
  {
    restore_mouse();
  }

  ++scrolldelay;
  if(MouseScroll && (x>=combolist_x) && (x<combolist_x+(16*combolist_w)) && (key[KEY_LSHIFT] || key[KEY_RSHIFT] || (scrolldelay&3)==0) )
  {
    if ((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])&&(key[KEY_ALT] || key[KEY_ALTGR]))
    {
      if(y>=combolist_y-16 && y<=combolist_y && First)
      {
        First=0;
        redraw|=rCOMBOS;
      }
      if(y>=combolist_y+(combolist_h*16)-1 && y<combolist_y+(combolist_h*16)+16 && First<(MAXCOMBOS-(combolist_w*combolist_h)))
      {
        First=MAXCOMBOS-(combolist_w*combolist_h);
        redraw|=rCOMBOS;
      }
    }
    else if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
      if(y>=combolist_y-16 && y<=combolist_y && First)
      {
        First-=min(First,256);
        redraw|=rCOMBOS;
      }
      if(y>=combolist_y+(combolist_h*16)-1 && y<combolist_y+(combolist_h*16)+16 && First<(MAXCOMBOS-(combolist_w*combolist_h)))
      {
        First+=min((MAXCOMBOS-256)-First,256);
        redraw|=rCOMBOS;
      }
    }
    else if (key[KEY_ALT] || key[KEY_ALTGR])
    {
      if(y>=combolist_y-16 && y<=combolist_y && First)
      {
        First-=min(First,(combolist_w*combolist_h));
        redraw|=rCOMBOS;
      }
      if(y>=combolist_y+(combolist_h*16)-1 && y<combolist_y+(combolist_h*16)+16 && First<(MAXCOMBOS-(combolist_w*combolist_h)))
      {
        First+=min((MAXCOMBOS-(combolist_w*combolist_h))-First,(combolist_w*combolist_h));
        redraw|=rCOMBOS;
      }
    }
    else
    {
      if(y>=combolist_y-16 && y<=combolist_y && First)
      {
        First-=min(First,combolist_w);
        redraw|=rCOMBOS;
      }
      if(y>=combolist_y+(combolist_h*16)-1 && y<combolist_y+(combolist_h*16)+16 && First<(MAXCOMBOS-(combolist_w*combolist_h)))
      {
        First+=min((MAXCOMBOS-combolist_w)-First,combolist_w);
        redraw|=rCOMBOS;
      }
    }
  }
  if(gui_mouse_b()==0)
  {
    mouse_down = false;
    canfill=true;
  }
  else if(gui_mouse_b()&1)
  {
    //on the map screen
    //if(isinRect(x,y,0,16,255,191))
    if(isinRect(x,y,startxint,startyint,int(startx+(256*mapscreensize)-1),int(starty+(176*mapscreensize)-1)))
    {
      if (!((key[KEY_ZC_LCONTROL])||(key[KEY_ZC_RCONTROL])))
      {
        draw();
      }
      else
      {
        if (canfill)
        {
          switch (fill_type)
          {
            case 0: flood();   break;
            case 1: fill_4();  break;
            case 2: fill_8();  break;
            case 3: fill2_4(); break;
            case 4: fill2_8(); break;
          }
          canfill=false;
        }
      }
    }
    //on the minimap
    if(isinRect(x,y,minimap_x+3,minimap_y+12,minimap_x+minimap_w-5,minimap_y+minimap_h-4))
    {
      select_scr();
    }
    if(menutype==m_coords)
    {
      if(isinRect(x,y,panel_1_x+16,panel_1_y+6,panel_1_x+16+15,panel_1_y+6+15))
      {
        doxypos(Map.CurrScr()->itemx,Map.CurrScr()->itemy,11,0xF8);
      }
      if(isinRect(x,y,panel_1_x+48,panel_1_y+6,panel_1_x+48+15,panel_1_y+6+15))
      {
        doxypos(Map.CurrScr()->stairx,Map.CurrScr()->stairy,14,0xF0);
      }
      /*if(isinRect(x,y,panel_1_x+80,panel_1_y+6,panel_1_x+80+15,panel_1_y+6+15))
        {
        doxypos(Map.CurrScr()->warpreturnx,Map.CurrScr()->warpreturny,9,0xF8);
        }*/
      if(isinRect(x,y,panel_1_x+80,panel_1_y+6,panel_1_x+80+15,panel_1_y+6+15))
      {
        doxypos(Map.CurrScr()->warparrivalx,Map.CurrScr()->warparrivaly,10,0xF8);
      }
      if(isinRect(x,y,panel_1_x+112,panel_1_y+6,panel_1_x+112+15,panel_1_y+6+15))
      {
        doflags();
      }
    }
    if(menutype==m_coords2)
    {
      if(isinRect(x,y,panel_9_x+16,panel_9_y+6,panel_9_x+16+15,panel_9_y+6+15))
      {
        doxypos(Map.CurrScr()->warpreturnx[0],Map.CurrScr()->warpreturny[0],9,0xF8);
      }
      if(isinRect(x,y,panel_9_x+48,panel_9_y+6,panel_9_x+48+15,panel_9_y+6+15))
      {
        doxypos(Map.CurrScr()->warpreturnx[1],Map.CurrScr()->warpreturny[1],9,0xF8);
      }
      if(isinRect(x,y,panel_9_x+80,panel_9_y+6,panel_9_x+80+15,panel_9_y+6+15))
      {
        doxypos(Map.CurrScr()->warpreturnx[2],Map.CurrScr()->warpreturny[2],9,0xF8);
      }
      if(isinRect(x,y,panel_9_x+112,panel_9_y+6,panel_9_x+112+15,panel_9_y+6+15))
      {
        doxypos(Map.CurrScr()->warpreturnx[3],Map.CurrScr()->warpreturny[3],9,0xF8);
      }
    }
    else if (menutype==m_layers)
    {
      if(isinRect(x, y, panel_7_x+9,panel_7_y+20,panel_7_x+9+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+9,panel_7_y+20,vc(1),vc(14), LayerMaskInt[0]);
      }
      if(isinRect(x, y, panel_7_x+34,panel_7_y+20,panel_7_x+34+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+34,panel_7_y+20,vc(1),vc(14), LayerMaskInt[1]);
      }
      if(isinRect(x, y, panel_7_x+59,panel_7_y+20,panel_7_x+59+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+59,panel_7_y+20,vc(1),vc(14), LayerMaskInt[2]);
      }
      if(isinRect(x, y, panel_7_x+84,panel_7_y+20,panel_7_x+84+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+84,panel_7_y+20,vc(1),vc(14), LayerMaskInt[3]);
      }
      if(isinRect(x, y, panel_7_x+109,panel_7_y+20,panel_7_x+109+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+109,panel_7_y+20,vc(1),vc(14), LayerMaskInt[4]);
      }
      if(isinRect(x, y, panel_7_x+134,panel_7_y+20,panel_7_x+134+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+134,panel_7_y+20,vc(1),vc(14), LayerMaskInt[5]);
      }
      if(isinRect(x, y, panel_7_x+159,panel_7_y+20,panel_7_x+159+8,panel_7_y+20+8))
      {
        do_checkbox(menu1,panel_7_x+159,panel_7_y+20,vc(1),vc(14), LayerMaskInt[6]);
      }
      if(isinRect(x, y, panel_7_x+9,panel_7_y+30, panel_7_x+9+(6*25)+8,panel_7_y+30+8))
      {
        do_layerradio(menu1,panel_7_x+9,panel_7_y+30,vc(1),vc(14), CurrentLayer);
      }
      redraw|=rMENU;
    }
    if(draw_mode!=dm_alias)
    {
      if(isinRect(x,y,comboscroller_x,comboscroller_y,comboscroller_x+comboscroller_w,comboscroller_y+comboscroller_h-2) && First>0 && !mouse_down)
      {
        if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          First-=min(First,256);
        }
        else
        {
          First-=min(First,(combolist_w*combolist_h));
        }
        redraw|=rCOMBOS;
      }
      if(isinRect(x,y,comboscroller_x,comboscroller_y+comboscroller_h-1,comboscroller_x+comboscroller_w,comboscroller_y+comboscroller_h*2-3) && First<(MAXCOMBOS-(combolist_w*combolist_h)) && !mouse_down)
      {
        if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          First+=min((MAXCOMBOS-256)-First,256);
        }
        else
        {
          First+=min((MAXCOMBOS-(combolist_w*combolist_h))-First,(combolist_w*combolist_h));
        }
        redraw|=rCOMBOS;
      }
      if((isinRect(x,y,panel_1_x+panel_1_w-28,panel_1_y+32,panel_1_x+panel_1_w-28+24,panel_1_y+32+5) && menutype==m_block && !mouse_down) ||
         (isinRect(x,y,panel_7_x+panel_7_w-28,panel_7_y+36,panel_7_x+panel_7_w-28+24,panel_7_y+36+5) && menutype==m_layers && !mouse_down))
      {
        CSet=wrap(CSet+1,0,11);
        refresh(rCOMBOS+rMENU+rCOMBO);
      }
      if(isinRect(x,y,panel_1_x+panel_1_w-32,panel_1_y+39,panel_1_x+panel_1_w-32+28,panel_1_y+39+5) && menutype==m_block && !mouse_down)
      {
        bool validlayer=false;
        while (!validlayer)
        {
          CurrentLayer=wrap(CurrentLayer+1,0,6);
          if ((CurrentLayer==0)||(Map.CurrScr()->layermap[CurrentLayer-1]))
          {
            validlayer=true;
          }
        }
        refresh(rMENU);
      }
      if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_h*16)-1))
      {
        select_combo();
      }
    }
    else
    {
      if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_ha*16)-1))
      {
        select_comboa();
      }
    }
    mouse_down = true;
  }
  else if(gui_mouse_b()&2)
  {
    if(isinRect(x,y,startxint,startyint,
                int(startx+(256*mapscreensize)-1),
                int(starty+(176*mapscreensize)-1)))
    {
      ComboBrushPause=1;
      refresh(rMAP);
      restore_mouse();
      ComboBrushPause=0;
      int m = popup_menu(draw_rc_menu,x,y);
      switch(m)
      {
        case 0: draw_block(c,1,2);                 break;
        case 1: draw_block(c,2,1);                 break;
        case 2: draw_block(c,2,2);                 break;
        case 3: draw_block(c,4,2);                 break;

        case 10: replace(c);                       break;
        case 11: peek(c);                          break;
        case 12:
        case 13:
        {
          int drawmap, drawscr;
          if (CurrentLayer==0)
          {
            drawmap=Map.getCurrMap();
            drawscr=Map.getCurrScr();
          }
          else
          {
            drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
            drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
            if (drawmap<0)
            {
              return;
            }
          }
          if (m==12)
          {
            Combo=Map.AbsoluteScr(drawmap, drawscr)->data[c];
          }
          if (m==13||(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
          {
            First=vbound((Map.AbsoluteScr(drawmap, drawscr)->data[c]/combolist_w*combolist_w)-(combolist_w*combolist_h/2),0,MAXCOMBOS-(combolist_w*combolist_h));
          }
        }
        break;
        case 14:
        {
          int drawmap, drawscr;
          if (CurrentLayer==0)
          {

            drawmap=Map.getCurrMap();
            drawscr=Map.getCurrScr();
          }
          else
          {
            drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
            drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
            if (drawmap<0)
            {
              return;
            }
          }
          edit_combo(Map.AbsoluteScr(drawmap, drawscr)->data[c],true,Map.AbsoluteScr(drawmap, drawscr)->cset[c]);
        }
        break;
      }
    }
    //    else if(x>=256 && y>=16)
    else if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_h*16)-1))
    {
      if(draw_mode != dm_alias)
      {
        select_combo();
        //      if(gui_mouse_x()>=256 && gui_mouse_y()>=16)
        if(isinRect(gui_mouse_x(),gui_mouse_y(),combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_h*16)-1))
        {
          reset_combo_animations();
          edit_combo(((gui_mouse_y()&0xF0)>>2)-4+((gui_mouse_x()-256)>>4)+First,true,CSet);
          setup_combo_animations();
          //        refresh(rALL);
          redraw|=rALL;
        }
      }
    }
    else
    {
      if((isinRect(x,y,panel_1_x+panel_1_w-28,panel_1_y+32,panel_1_x+panel_1_w-28+24,panel_1_y+32+5) && menutype==m_block && !mouse_down) ||
         (isinRect(x,y,panel_7_x+panel_7_w-28,panel_7_y+36,panel_7_x+panel_7_w-28+24,panel_7_y+36+5) && menutype==m_layers && !mouse_down))
      {
        CSet=wrap(CSet-1,0,11);
        refresh(rCOMBOS+rMENU+rCOMBO);
      }
      if(isinRect(x,y,panel_1_x+panel_1_w-32,panel_1_y+39,panel_1_x+panel_1_w-32+28,panel_1_y+39+5) && menutype==m_block && !mouse_down)
      {
        bool validlayer=false;
        while (!validlayer)
        {
          CurrentLayer=wrap(CurrentLayer-1,0,6);
          if ((CurrentLayer==0)||(Map.CurrScr()->layermap[CurrentLayer-1]))
          {
            validlayer=true;
          }
        }
        refresh(rMENU);
      }
    }
    if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_ha*16)-1))
    {
      if(draw_mode == dm_alias)
      {
        select_comboa();
        //      if(gui_mouse_x()>=256 && gui_mouse_y()>=16)
        if(isinRect(gui_mouse_x(),gui_mouse_y(),combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_h*16)-1))
        {
          //reset_combo_animations();
          comboa_cnt = combo_apos;
          onEditComboAlias();
          //setup_combo_animations();
          //        refresh(rALL);
          redraw|=rALL;
        }
      }
    }
    mouse_down = true;
  }
  else if(gui_mouse_b()&4)  //not sure what to do here yet
  {
  }
  if (mouse_z!=0)
  {
    int z=abs(mouse_z);
    if(draw_mode != dm_alias)
    {
      if (key[KEY_ALT]||key[KEY_ALTGR])
      {
        z*=combolist_h;
      }
      if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_h*16)-1))
      {
        if (mouse_z<0) //scroll down
        {
          if(First<(MAXCOMBOS-(combolist_w*combolist_h)))
          {
            First+=min((MAXCOMBOS-combolist_w)-First,combolist_w*z);
            redraw|=rALL;
          }
        }
        else //scroll up
        {
          if(First>0)
          {
            First-=min(First,combolist_w*z);
            //          refresh(rCOMBOS);
            redraw|=rALL;
          }
        }
      }
    }
    else
    {
      if (key[KEY_ALT]||key[KEY_ALTGR])
      {
        z*=20;
      }
      if(isinRect(x,y,combolist_x,combolist_y,combolist_x+(combolist_w*16)-1,combolist_y+(combolist_ha*16)-1))
      {
        if (mouse_z<0) //scroll down
        {
          if(combo_alistpos<(MAXCOMBOALIASES-(combolist_w*combolist_ha)))
          {
            combo_alistpos+=min((MAXCOMBOALIASES-combolist_w)-combo_alistpos,combolist_w*z);
            redraw|=rALL;
          }
        }
        else //scroll up
        {
          if(combo_alistpos>0)
          {
            combo_alistpos-=min(combo_alistpos,combolist_w*z);
            //          refresh(rCOMBOS);
            redraw|=rALL;
          }
        }
      }
    }
    if(isinRect(x,y,minimap_x,minimap_y+8,minimap_x+63,minimap_y+8+35))
    {
      for (int i=0; i<z; ++i)
      {
        if (mouse_z>0)
        {
          onIncMap();
        }
        else
        {
          onDecMap();
        }
      }

    }
    else if(isinRect(x,y,panel_1_x,panel_1_y,panel_1_x+191,panel_1_y+47))
      {
        for (int i=0; i<z; ++i)
        {
          if (mouse_z>0)
          {
            onPgUp();
          }
          else
          {
            onPgDn();
          }
        }
      }
      position_mouse_z(0);
  }
}

static DIALOG showpal_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   68,   272,  119,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "View Palette" },
  { jwin_frame_proc,   30,   76+16,   260,  68,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_bitmap_proc,     32,   76+18,   256,  64,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_button_proc,     130,  144+18,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
  { NULL }
};

int onShowPal()
{
  BITMAP *palbmp = create_bitmap_ex(8,256,64);
  if(!palbmp)
    return D_O_K;
  showpal_dlg[0].dp2=lfont;
  for(int i=0; i<256; i++)
    rectfill(palbmp,(i&31)<<3,(i&0xE0)>>2,((i&31)<<3)+7,((i&0xE0)>>2)+7,i);
  showpal_dlg[2].dp=palbmp;
  zc_popup_dialog(showpal_dlg,2);
  destroy_bitmap(palbmp);
  return D_O_K;
}

static DIALOG testgui_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   68,   272,  119,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "GUI Color Editor" },
  { jwin_guitest_proc,   30,   76+16,   260,  68,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { jwin_button_proc,     130,  144+18,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
  { NULL }
};

int onTestGUI()
{
  testgui_dlg[0].dp2=lfont;
  zc_popup_dialog(testgui_dlg,2);
  return D_O_K;
}

static DIALOG csetfix_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        72,   80,   176+1,  96+1,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "CSet Fix" },
  { d_dummy_proc,         160,  88,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_radio_proc,      104+22,  108,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full Screen" },
  { jwin_radio_proc,      104+22,  118+2,  80+1,   8+1,    vc(14),  vc(1),  0,       D_SELECTED, 0,             0,       (void *) "Dungeon Floor" },
  { d_dummy_proc,         120,  128,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      104+22,  128+4,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "All Layers" },
  { jwin_button_proc,     90,   152,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  152,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int onCSetFix()
{
  restore_mouse();
  csetfix_dlg[0].dp2=lfont;
  int s=2,x2=14,y2=9;

  if(zc_popup_dialog(csetfix_dlg,-1)==6)
  {
    Map.Ugo();

    if(csetfix_dlg[2].flags&D_SELECTED)
    {
      s=0; x2=16; y2=11;
    }

    if(csetfix_dlg[5].flags&D_SELECTED)
    {
      /*
        int drawmap, drawscr;
        if (CurrentLayer==0)
        {
        drawmap=Map.getCurrMap();
        drawscr=Map.getCurrScr();
        }
        else
        {
        drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
        drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
        if (drawmap<0)
        {
        return;
        }
        }

        saved=false;
        Map.Ugo();

        if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID))
        {
        Map.CurrScr()->valid|=mVALID;
        Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
        Map.setcolor(Color);
        }
        for(int i=0; i<176; i++)
        {
        Map.AbsoluteScr(drawmap, drawscr)->data[i]=Combo;
        Map.AbsoluteScr(drawmap, drawscr)->cset[i]=CSet;
        }
        refresh(rMAP+rSCRMAP);
        */
    }

    for(int y=s; y<y2; y++)
    {
      for(int x=s; x<x2; x++)
      {
        Map.CurrScr()->cset[(y<<4)+x] = CSet;
      }
    }
    refresh(rMAP);
    saved = false;
  }
  return D_O_K;
}

static DIALOG template_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 72,   80,   176+1,  96+1,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Dungeon Template" },
  { jwin_frame_proc,   178,  102+3,  20,  20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_combo_proc,      180,  104+3,  16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //  { d_bitmap_proc,     180,  104,  16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_radio_proc,      104+33,  108+3,  64+1,   8+1,    vc(14),  vc(1),  0,       D_SELECTED, 0,             0,       (void *) "Floor:" },
  { jwin_radio_proc,      104+33,  128+3,  64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Floor" },
  { jwin_button_proc,     90,   152,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  152,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int onTemplate()
{
  static bool donethis=false;
  if (!donethis||!(key[KEY_LSHIFT]||key[KEY_RSHIFT]))
  {
    template_dlg[2].d1=Combo;
    template_dlg[2].fg=CSet;
    donethis=true;
  }
  restore_mouse();
  if(Map.getCurrScr()==TEMPLATE)
    return D_O_K;

  //  BITMAP *floor_bmp = create_bitmap_ex(8,16,16);
  //  if(!floor_bmp) return D_O_K;
  template_dlg[0].dp2=lfont;
  //  put_combo(floor_bmp,0,0,Combo,CSet,0,0);
  //  template_dlg[2].dp=floor_bmp;
  if(zc_popup_dialog(template_dlg,-1)==5)
  {
    saved=false;
    Map.Ugo();
    Map.Template((template_dlg[3].flags==D_SELECTED) ? template_dlg[2].d1 : -1, template_dlg[2].fg);
    refresh(rMAP+rSCRMAP);
  }
  //  destroy_bitmap(floor_bmp);
  return D_O_K;
}

static DIALOG cpage_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 72,   20,   176+1,  212+1,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
  { d_ctext_proc,      160,  28,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Combo Page" },
  { jwin_button_proc,     90,   182,  61,   21,   vc(14),  vc(1),  's',     D_EXIT,     0,             0,       (void *) "&Set" },
  { jwin_button_proc,     170,  182,  61,   21,   vc(14),  vc(1),  'c',     D_EXIT,     0,             0,       (void *) "&Cancel" },
  { jwin_button_proc,     90,   210,  61,   21,   vc(14),  vc(1),  'a',     D_EXIT,     0,             0,       (void *) "Set &All" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  // 6
  { jwin_radio_proc,       76,  44,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "0"   },
  { jwin_radio_proc,       76,  52,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1"   },
  { jwin_radio_proc,       76,  60,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2"   },
  { jwin_radio_proc,       76,  68,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3"   },
  { jwin_radio_proc,       76,  76,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4"   },
  { jwin_radio_proc,       76,  84,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "5"   },
  { jwin_radio_proc,       76,  92,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "6"   },
  { jwin_radio_proc,       76, 100,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "7"   },
  { jwin_radio_proc,       76, 108,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "8"   },
  { jwin_radio_proc,       76, 116,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "9"   },
  { jwin_radio_proc,       76, 124,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "10"  },
  { jwin_radio_proc,       76, 132,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "11"  },
  { jwin_radio_proc,       76, 140,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "12"  },
  { jwin_radio_proc,       76, 148,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "13"  },
  { jwin_radio_proc,       76, 156,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "14"  },
  { jwin_radio_proc,       76, 164,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "15"  },
  { jwin_radio_proc,      120,  44,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "16"  },
  { jwin_radio_proc,      120,  52,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "17"  },
  { jwin_radio_proc,      120,  60,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "18"  },
  { jwin_radio_proc,      120,  68,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "19"  },
  { jwin_radio_proc,      120,  76,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "20"  },
  { jwin_radio_proc,      120,  84,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "21"  },
  { jwin_radio_proc,      120,  92,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "22"  },
  { jwin_radio_proc,      120, 100,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "23"  },
  { jwin_radio_proc,      120, 108,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "24"  },
  { jwin_radio_proc,      120, 116,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "25"  },
  { jwin_radio_proc,      120, 124,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "26"  },
  { jwin_radio_proc,      120, 132,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "27"  },
  { jwin_radio_proc,      120, 140,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "28"  },
  { jwin_radio_proc,      120, 148,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "29"  },
  { jwin_radio_proc,      120, 156,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "30"  },
  { jwin_radio_proc,      120, 164,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "31"  },
  { jwin_radio_proc,      164,  44,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "32"  },
  { jwin_radio_proc,      164,  52,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "33"  },
  { jwin_radio_proc,      164,  60,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "34"  },
  { jwin_radio_proc,      164,  68,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "35"  },
  { jwin_radio_proc,      164,  76,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "36"  },
  { jwin_radio_proc,      164,  84,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "37"  },
  { jwin_radio_proc,      164,  92,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "38"  },
  { jwin_radio_proc,      164, 100,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "39"  },
  { jwin_radio_proc,      164, 108,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "40"  },
  { jwin_radio_proc,      164, 116,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "41"  },
  { jwin_radio_proc,      164, 124,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "42"  },
  { jwin_radio_proc,      164, 132,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "43"  },
  { jwin_radio_proc,      164, 140,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "44"  },
  { jwin_radio_proc,      164, 148,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "45"  },
  { jwin_radio_proc,      164, 156,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "46"  },
  { jwin_radio_proc,      164, 164,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "47"  },
  { jwin_radio_proc,      208,  44,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "48"  },
  { jwin_radio_proc,      208,  52,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "49"  },
  { jwin_radio_proc,      208,  60,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "50"  },
  { jwin_radio_proc,      208,  68,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "51"  },
  { jwin_radio_proc,      208,  76,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "52"  },
  { jwin_radio_proc,      208,  84,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "53"  },
  { jwin_radio_proc,      208,  92,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "54"  },
  { jwin_radio_proc,      208, 100,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "55"  },
  { jwin_radio_proc,      208, 108,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "56"  },
  { jwin_radio_proc,      208, 116,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "57"  },
  { jwin_radio_proc,      208, 124,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "58"  },
  { jwin_radio_proc,      208, 132,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "59"  },
  { jwin_radio_proc,      208, 140,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "60"  },
  { jwin_radio_proc,      208, 148,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "61"  },
  { jwin_radio_proc,      208, 156,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "62"  },
  { jwin_radio_proc,      208, 164,   33,   9,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "63"  },
  { NULL }
};

int onComboPage()
{
  for(int i=0; i<64; i++)
    cpage_dlg[i+6].flags = Map.CurrScr()->old_cpage==i?D_SELECTED:0;

  int ret = zc_popup_dialog(cpage_dlg,3);

  int p = 0;
  for(int i=0; i<64; i++)

    if(cpage_dlg[i+6].flags==D_SELECTED)
      p=i;

    if(ret==2)
  {
    saved=false;
    Map.CurrScr()->old_cpage = p;
  }
  if(ret==4 && jwin_alert("Confirm Overwrite","Set all combo pages","on this map?",NULL,"&Yes","&No",'y','n',lfont)==1)
  {
    saved=false;
    for(int i=0; i<=TEMPLATE; i++)
      Map.Scr(i)->old_cpage = p;
  }
  refresh(rALL);
  return D_O_K;
}

int d_sel_scombo_proc(int msg, DIALOG *d, int c)
{
  switch(msg)
  {
    case MSG_CLICK:
    while(gui_mouse_b())
    {
      int x = min( max(gui_mouse_x() - d->x,0)>>4, 15);
      int y = min( max(gui_mouse_y() - d->y,0)&0xF0, 160);
      if(x+y != d->d1)
      {
        d->d1 = x+y;
        custom_vsync();
        scare_mouse();
        d_sel_scombo_proc(MSG_DRAW,d,0);
        unscare_mouse();
      }
    }
    break;

    case MSG_DRAW:
    {
      blit((BITMAP*)(d->dp),screen,0,0,d->x,d->y,d->w,d->h);
      int x = d->x + (((d->d1)&15)<<4);
      int y = d->y + ((d->d1)&0xF0);
      rect(screen,x,y,x+15,y+15,vc(15));
    }
    break;
  }

  return D_O_K;
}

static DIALOG sel_scombo_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 24,   16,   272+1,  216+1,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
  { d_sel_scombo_proc, 32,   24,   256,  176,  0,       0,      0,       0,          0,             0,       NULL },
  { jwin_button_proc,     90,   208,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  208,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

void select_scombo(int &pos)
{
  go();
  Map.draw_template2(screen2,0,0);
  sel_scombo_dlg[1].dp = screen2;
  sel_scombo_dlg[1].d1 = pos;

  while(gui_mouse_b());

  if(zc_do_dialog(sel_scombo_dlg,3)==2)
    pos = sel_scombo_dlg[1].d1;

  comeback();
}

//char *flaglist(int index, int *list_size);
//int select_data(char *prompt,int index,char *(proc)(int,int*), FONT *title_font);

int select_flag(int &f)
{
  int ret=select_data("Flag Type",f,flaglist,lfont);
  if (ret>=0)
  {
    f=ret;
    return true;
  }
  return false;
}

int d_scombo_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_CLICK:
    {
      int c=d->d1;
      int cs=d->fg;
      int f=d->d2;
      if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
      {
        while (gui_mouse_b());
        if(select_flag(f))
        {
          d->d2=f;

        }
      }
	  else if(key[KEY_LSHIFT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->d1++;
		  if(d->d1>=MAXCOMBOS) d->d1=0;
		}
		else if(gui_mouse_b()&2)
		{
		  d->d1--;
		  if(d->d1<0) d->d1=MAXCOMBOS-1;
		}
	  }
	  else if(key[KEY_RSHIFT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->fg++;
		  if(d->fg>11) d->fg=0;
		}
		else if(gui_mouse_b()&2)
		{
		  d->fg--;
		  if(d->fg<0) d->fg=11;
		}
	  }
	  else if(key[KEY_ALT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->d1 = Combo;
		  d->fg = CSet;
		}
	  }
      else
      {
        if (select_combo_2(c, cs))
        {
          d->d1=c;
          d->fg=cs;
        }
      }
      return D_REDRAW;
    }
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,16,16);
    if(buf)
    {
      clear_bitmap(buf);
      if(d->d1)
        putcombo(buf,0,0,d->d1,d->fg);
      if(Flags&cFLAGS)
        put_flags(buf,0,0,d->d1,d->fg,cFLAGS,d->d2);
      blit(buf,screen,0,0,d->x,d->y,d->w,d->h);
      destroy_bitmap(buf);
    }
    break;
  }
  return D_O_K;
}

int d_scombo2_proc(int msg, DIALOG *d, int c)
{
  switch(msg)
  {
    case MSG_CLICK:
    if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
      select_scombo(d->d1);
    }
    else
    {
      select_scombo(d->d1);
    }
    scare_mouse();
    d_scombo_proc(MSG_DRAW,d,0);
    unscare_mouse();
    break;


    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,16,16);
    if(buf)
    {
      clear_bitmap(buf);
      Map.draw_secret2(buf,d->d1);
      blit(buf,screen,0,0,d->x,d->y,16,16);
      destroy_bitmap(buf);
    }
    break;
  }

  return D_O_K;
}

int onSecretF();

static int secret_burn_list[] =
{
  // dialog control number
  4, 5, 6, 7, 48, 49, 50, 51, 92, 93, 94, 95, -1
};

static int secret_arrow_list[] =
{
  // dialog control number
  8, 9, 10, 52, 53, 54, 96, 97, 98, -1
};

static int secret_bomb_list[] =
{
  // dialog control number
  11, 12, 55, 56, 99, 100, -1
};

static int secret_boomerang_list[] =
{
  // dialog control number
  13, 14, 15, 57, 58, 59, 101, 102, 103, -1
};

static int secret_magic_list[] =
{
  // dialog control number
  16, 17, 60, 61, 104, 105, -1
};

static int secret_sword_list[] =
{
  // dialog control number
  18, 19, 20, 21, 22, 23, 24, 25, 62, 63, 64, 65, 66, 67, 68, 69, 106, 107, 108, 109, 110, 111, 112, 113, -1
};

static int secret_misc_list[] =
{
  // dialog control number
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, -1
};

static TABPANEL secret_tabs[] =
{
  // (text)
  { "Burn",       D_SELECTED,   secret_burn_list },
  { "Arrow",       0,           secret_arrow_list },
  { "Bomb",        0,           secret_bomb_list },
  { "Boomerang",   0,           secret_boomerang_list },
  { "Magic",       0,           secret_magic_list },
  { "Sword",       0,           secret_sword_list },
  { "Misc",        0,           secret_misc_list },
  { NULL }
};

static DIALOG secret_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,   10,   18,   281+20,  212,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Secret Combos" },
  { jwin_tab_proc,   24-8,  43,  281+8,  161-5,   0,       0,      0,       0,             0,       0,       (void *) secret_tabs, NULL, (void *)secret_dlg },
  { jwin_button_proc,     90,  210-5,   61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170, 210-5,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  // 4
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Blue Candle" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Red Candle" },
  { jwin_text_proc,        22,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Wand Fire" },
  { jwin_text_proc,        22,  71+66,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Din's Fire" },
  //8
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Wooden Arrow" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Silver Arrow" },
  { jwin_text_proc,        22,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Golden Arrow" },
  //11
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Bomb" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Super Bomb" },
  //13
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Wooden Boomerang" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Magic Boomerang" },
  { jwin_text_proc,        22,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Fire Boomerang" },
  //16
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Wand Magic" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Reflected Magic" },
  //18
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Sword" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "White Sword" },
  { jwin_text_proc,        22,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Magic Sword" },
  { jwin_text_proc,        22,  71+66,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Master Sword" },
  { jwin_text_proc,        22+148,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Sword Beam" },
  { jwin_text_proc,        22+148,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "White Sword Beam" },
  { jwin_text_proc,        22+148,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Magic Sword Beam" },
  { jwin_text_proc,        22+148,  71+66,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Master Sword Beam" },
  //26
  { jwin_text_proc,        22,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Stairs" },
  { jwin_text_proc,        22,  71+22,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Reflected Fireball" },
  { jwin_text_proc,        22,  71+44,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Hookshot" },
  { jwin_text_proc,        22,  71+66,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Wand" },
  { jwin_text_proc,        22,  71+88,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Hammer" },
  { jwin_text_proc,        22,  71+110,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Any Weapon" },
  //32
  { jwin_ctext_proc,    221,  71,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Flags 16-31" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 02" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 03" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 04" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 05" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 06" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 07" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 08" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 09" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 10" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 11" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 12" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 13" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 14" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 15" },
  { d_dummy_proc,      0,   0,       0,    0,    0,       0,      0,       0,          FR_DEEP,        0,       (void *) "Secret Combo 16" },
  //48 (burn)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //52 (arrow)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //55 (bomb)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //57 (boomerang)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //60 (magic)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //62 (sword)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   266,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   266,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   266,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   266,  65+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //70 (misc)
  { jwin_frame_proc,   118,  65,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+88,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   118,  65+110,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  //76
  { jwin_frame_proc,   178,  87,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   200,  87,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   222,  87,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   244,  87,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   178,  87+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   200,  87+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   222,  87+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   244,  87+22,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   178,  87+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   200,  87+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   222,  87+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   244,  87+44,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   178,  87+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   200,  87+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   222,  87+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   244,  87+66,   20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },

  //92 (burn)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //96 (arrow)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //99 (bomb)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //101 (boomerang)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //104 (magic)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //106 (sword)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120+148,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120+148,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120+148,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120+148,  67+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //114 (misc)
  { d_scombo_proc,     120,  67,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+88,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     120,  67+110,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //120
  { d_scombo_proc,     180,  89,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     202,  89,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     224,  89,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     246,  89,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     180,  89+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     202,  89+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     224,  89+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     246,  89+22,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     180,  89+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     202,  89+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     224,  89+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     246,  89+44,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     180,  89+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     202,  89+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     224,  89+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  { d_scombo_proc,     246,  89+66,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
  //136
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,     'f',      0,          0,             0,       (void *) onSecretF },
  { NULL }
};

int onSecretF()
{
  Flags^=cFLAGS;
  object_message(secret_dlg+1, MSG_DRAW, 0);
  return D_O_K;
}


int onSecretCombo()
{
  secret_dlg[0].dp2=lfont;

  mapscr *s;
  if (CurrentLayer==0)
  {
    s=Map.CurrScr();
  }
  else
  {
    //   s=TheMaps[(Map.CurrScr()->layermap[CurrentLayer-1]-1)*MAPSCRS+(Map.CurrScr()->layerscreen[CurrentLayer-1])];
    s=Map.AbsoluteScr((Map.CurrScr()->layermap[CurrentLayer-1]-1), (Map.CurrScr()->layerscreen[CurrentLayer-1]));
  }

  secret_dlg[92].d1 = s->secretcombo[sBCANDLE];
  secret_dlg[92].fg = s->secretcset[sBCANDLE];
  secret_dlg[92].d2 = s->secretflag[sBCANDLE];

  secret_dlg[93].d1 = s->secretcombo[sRCANDLE];
  secret_dlg[93].fg = s->secretcset[sRCANDLE];
  secret_dlg[93].d2 = s->secretflag[sRCANDLE];

  secret_dlg[94].d1 = s->secretcombo[sWANDFIRE];
  secret_dlg[94].fg = s->secretcset[sWANDFIRE];
  secret_dlg[94].d2 = s->secretflag[sWANDFIRE];

  secret_dlg[95].d1 = s->secretcombo[sDINSFIRE];
  secret_dlg[95].fg = s->secretcset[sDINSFIRE];
  secret_dlg[95].d2 = s->secretflag[sDINSFIRE];

  secret_dlg[96].d1 = s->secretcombo[sARROW];
  secret_dlg[96].fg = s->secretcset[sARROW];
  secret_dlg[96].d2 = s->secretflag[sARROW];

  secret_dlg[97].d1 = s->secretcombo[sSARROW];
  secret_dlg[97].fg = s->secretcset[sSARROW];
  secret_dlg[97].d2 = s->secretflag[sSARROW];

  secret_dlg[98].d1 = s->secretcombo[sGARROW];
  secret_dlg[98].fg = s->secretcset[sGARROW];
  secret_dlg[98].d2 = s->secretflag[sGARROW];

  secret_dlg[99].d1 = s->secretcombo[sBOMB];
  secret_dlg[99].fg = s->secretcset[sBOMB];
  secret_dlg[99].d2 = s->secretflag[sBOMB];

  secret_dlg[100].d1 = s->secretcombo[sSBOMB];
  secret_dlg[100].fg = s->secretcset[sSBOMB];
  secret_dlg[100].d2 = s->secretflag[sSBOMB];

  for(int i=0; i<3; i++)
  {
    secret_dlg[101+i].d1 = s->secretcombo[sBRANG+i];
    secret_dlg[101+i].fg = s->secretcset[sBRANG+i];
    secret_dlg[101+i].d2 = s->secretflag[sBRANG+i];
  }

  for(int i=0; i<2; i++)
  {
    secret_dlg[104+i].d1 = s->secretcombo[sWANDMAGIC+i];
    secret_dlg[104+i].fg = s->secretcset[sWANDMAGIC+i];
    secret_dlg[104+i].d2 = s->secretflag[sWANDMAGIC+i];
  }

  for(int i=0; i<8; i++)
  {
    secret_dlg[106+i].d1 = s->secretcombo[sSWORD+i];
    secret_dlg[106+i].fg = s->secretcset[sSWORD+i];
    secret_dlg[106+i].d2 = s->secretflag[sSWORD+i];
  }

  secret_dlg[114].d1 = s->secretcombo[sSTAIRS];
  secret_dlg[114].fg = s->secretcset[sSTAIRS];
  secret_dlg[114].d2 = s->secretflag[sSTAIRS];

  secret_dlg[115].d1 = s->secretcombo[sREFFIREBALL];
  secret_dlg[115].fg = s->secretcset[sREFFIREBALL];
  secret_dlg[115].d2 = s->secretflag[sREFFIREBALL];

  for(int i=0; i<4; i++)
  {
    secret_dlg[116+i].d1 = s->secretcombo[sHOOKSHOT+i];
    secret_dlg[116+i].fg = s->secretcset[sHOOKSHOT+i];
    secret_dlg[116+i].d2 = s->secretflag[sHOOKSHOT+i];
  }

  for(int i=0; i<16; i++)
  {
    secret_dlg[120+i].d1 = s->secretcombo[sSECRET01+i];
    secret_dlg[120+i].fg = s->secretcset[sSECRET01+i];
    secret_dlg[120+i].d2 = s->secretflag[sSECRET01+i];
  }

  go();
  if(zc_do_dialog(secret_dlg,3) == 2)
  {
    saved = false;
    s->secretcombo[sBCANDLE] = secret_dlg[92].d1;
    s->secretcset[sBCANDLE] = secret_dlg[92].fg;
    s->secretflag[sBCANDLE] = secret_dlg[92].d2;

    s->secretcombo[sRCANDLE] = secret_dlg[93].d1;
    s->secretcset[sRCANDLE] = secret_dlg[93].fg;
    s->secretflag[sRCANDLE] = secret_dlg[93].d2;

    s->secretcombo[sWANDFIRE] = secret_dlg[94].d1;
    s->secretcset[sWANDFIRE] = secret_dlg[94].fg;
    s->secretflag[sWANDFIRE] = secret_dlg[94].d2;

    s->secretcombo[sDINSFIRE] = secret_dlg[95].d1;
    s->secretcset[sDINSFIRE] = secret_dlg[95].fg;
    s->secretflag[sDINSFIRE] = secret_dlg[95].d2;

    s->secretcombo[sARROW] = secret_dlg[96].d1;
    s->secretcset[sARROW] = secret_dlg[96].fg;
    s->secretflag[sARROW] = secret_dlg[96].d2;

    s->secretcombo[sSARROW] = secret_dlg[97].d1;
    s->secretcset[sSARROW] = secret_dlg[97].fg;
    s->secretflag[sSARROW] = secret_dlg[97].d2;

    s->secretcombo[sGARROW] = secret_dlg[98].d1;
    s->secretcset[sGARROW] = secret_dlg[98].fg;
    s->secretflag[sGARROW] = secret_dlg[98].d2;

    s->secretcombo[sBOMB] = secret_dlg[99].d1;
    s->secretcset[sBOMB] = secret_dlg[99].fg;
    s->secretflag[sBOMB] = secret_dlg[99].d2;

    s->secretcombo[sSBOMB] = secret_dlg[100].d1;
    s->secretcset[sSBOMB] = secret_dlg[100].fg;
    s->secretflag[sSBOMB] = secret_dlg[100].d2;

    for(int i=0; i<3; i++)
    {
      s->secretcombo[sBRANG+i] = secret_dlg[101+i].d1;
      s->secretcset[sBRANG+i] = secret_dlg[101+i].fg;
      s->secretflag[sBRANG+i] = secret_dlg[101+i].d2;
    }

    for(int i=0; i<2; i++)
    {
      s->secretcombo[sWANDMAGIC+i] = secret_dlg[104+i].d1;
      s->secretcset[sWANDMAGIC+i] = secret_dlg[104+i].fg;
      s->secretflag[sWANDMAGIC+i] = secret_dlg[104+i].d2;
    }

    for(int i=0; i<8; i++)
    {
      s->secretcombo[sSWORD+i] = secret_dlg[106+i].d1;
      s->secretcset[sSWORD+i] = secret_dlg[106+i].fg;
      s->secretflag[sSWORD+i] = secret_dlg[106+i].d2;
    }

    s->secretcombo[sSTAIRS] = secret_dlg[114].d1;
    s->secretcset[sSTAIRS] = secret_dlg[114].fg;
    s->secretflag[sSTAIRS] = secret_dlg[114].d2;

    s->secretcombo[sREFFIREBALL] = secret_dlg[115].d1;
    s->secretcset[sREFFIREBALL] = secret_dlg[115].fg;
    s->secretflag[sREFFIREBALL] = secret_dlg[115].d2;

    for(int i=0; i<4; i++)
    {
      s->secretcombo[sHOOKSHOT+i] = secret_dlg[116+i].d1;
      s->secretcset[sHOOKSHOT+i] = secret_dlg[116+i].fg;
      s->secretflag[sHOOKSHOT+i] = secret_dlg[116+i].d2;
    }

    for(int i=0; i<16; i++)
    {
      s->secretcombo[sSECRET01+i] = secret_dlg[120+i].d1;
      s->secretcset[sSECRET01+i] = secret_dlg[120+i].fg;
      s->secretflag[sSECRET01+i] = secret_dlg[120+i].d2;
    }

  }
  comeback();
  return D_O_K;
}

static DIALOG under_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 72,   60,   176+1,  120+1,  vc(14),  vc(1),  0,       D_EXIT,        0,             0,       (void *) "Under Combo" },
  { jwin_frame_proc,   112,  92,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_bitmap_proc,     114,  94,   16,   16,   0,       0,      0,       0,             0,             0,       NULL },
  { jwin_frame_proc,   192,  92,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_bitmap_proc,     194,  94,   16,   16,   0,       0,      0,       0,             0,             0,       NULL },
  { jwin_button_proc,     90,   124,  61,   21,   vc(14),  vc(1),  's',     D_EXIT,     0,             0,       (void *) "&Set" },
  { jwin_button_proc,     170,  124,  61,   21,   vc(14),  vc(1),  'c',     D_EXIT,     0,             0,       (void *) "&Cancel" },
  { jwin_button_proc,     90,   152,  61,   21,   vc(14),  vc(1),  'a',     D_EXIT,     0,             0,       (void *) "Set &All" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { jwin_ctext_proc,       122,  84,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "New" },
  { jwin_ctext_proc,       202,  84,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Current" },
  { NULL }
};

int onUnderCombo()
{
  BITMAP *cur = create_bitmap_ex(8,16,16);
  BITMAP *sel = create_bitmap_ex(8,16,16);
  if(!cur || !sel)
    return D_O_K;

  put_combo(cur,0,0,Map.CurrScr()->undercombo,Map.CurrScr()->undercset,0,0);
  put_combo(sel,0,0,Combo,CSet,0,0);

  under_dlg[0].dp2 = lfont;
  under_dlg[2].dp = sel;
  under_dlg[4].dp = cur;
  int ret = zc_popup_dialog(under_dlg,-1);
  if(ret==5)
  {
    saved=false;
    Map.CurrScr()->undercombo = Combo;
    Map.CurrScr()->undercset = CSet;
  }
  if(ret==7 && jwin_alert("Confirm Overwrite","Set all under combos","on this map?",NULL,"&Yes","&No",'y','n',lfont)==1)
  {
    saved=false;
    for(int i=0; i<128; i++)
    {
      Map.Scr(i)->undercombo = Combo;
      Map.Scr(i)->undercset = CSet;
    }
  }

  destroy_bitmap(sel);
  destroy_bitmap(cur);
  return D_O_K;
}

static DIALOG list_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_list_proc,       72-12-4,   60+4,   176+24+8,  92+3,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  163,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

static DIALOG ilist_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_abclist_proc,       72-12-4,   60+4,   176+24+8,  92+3,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  163,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { NULL }
};

static DIALOG wlist_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_abclist_proc,       72-12-4,   60+4,   176+24+8,  92+3,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  163,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { NULL }
};

static DIALOG rlist_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_abclist_proc,       72-12-4,   60+4,   176+24+8,  92+3,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  163,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};



/*
  typedef struct item_struct {
  char *s;
  int i;
  } item_struct;
  */
item_struct bii[iMax];
int bii_cnt=-1;

void build_bii_list(bool usenone)
{
  int start=bii_cnt=0;
  if (usenone)
  {
    bii[0].s = "(None)";
    bii[0].i = 0;
    bii_cnt=start=1;
  }

  for(int i=start; i<iMax; i++)
  {
    bii[bii_cnt].s = item_string[i];
    bii[bii_cnt].i = i;
    ++bii_cnt;
  }

  for(int i=start; i<bii_cnt-1; i++)
  {
    for(int j=i+1; j<bii_cnt; j++)
    {
      if(stricmp(bii[i].s,bii[j].s)>0)
      {
        swap(bii[i],bii[j]);
      }
    }
  }
}

char *itemlist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = bii_cnt;
    return NULL;
  }
  return bii[index].s;
}

int select_item(char *prompt,int item,int allow_edit)
{
  ilist_dlg[3].dp=allow_edit?(void *)"Edit":(void *)"OK";
  ilist_dlg[4].dp=allow_edit?(void *)"Done":(void *)"Cancel";
  if(bii_cnt==-1)
  {
    build_bii_list(true);
  }

  int index=0;

  for(int j=0; j<bii_cnt; j++)
  {
    if(bii[j].i == item)
    {
      index=j;
    }
  }

  ilist_dlg[0].dp=prompt;
  ilist_dlg[0].dp2=lfont;
  ilist_dlg[2].d1=index;
  ilist_dlg[2].dp=(void *) itemlist;
  int ret=zc_popup_dialog(ilist_dlg,2);
  if (ret==0||ret==4)
  {
    position_mouse_z(0);
    return -1;
  }
  index = ilist_dlg[2].d1;
  position_mouse_z(0);
  return bii[index].i;
}

weapon_struct biw[wMAX];
int biw_cnt=-1;

void build_biw_list()
{
  int start=biw_cnt=0;

  for(int i=start; i<wMAX; i++)


  {
    biw[biw_cnt].s = weapon_string[i];
    biw[biw_cnt].i = i;
    ++biw_cnt;
  }

  for(int i=start; i<biw_cnt-1; i++)
    for(int j=i+1; j<biw_cnt; j++)
      if(stricmp(biw[i].s,biw[j].s)>0)
        swap(biw[i],biw[j]);

}

char *weaponlist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = biw_cnt;
    return NULL;
  }
  return biw[index].s;
}

int select_weapon(char *prompt,int weapon)
{
  if(biw_cnt==-1)
    build_biw_list();

  int index=0;

  for(int j=0; j<biw_cnt; j++)
  {
    if(biw[j].i == weapon)
    {
      index=j;
    }
  }

  wlist_dlg[0].dp=prompt;
  wlist_dlg[0].dp2=lfont;
  wlist_dlg[2].d1=index;
  wlist_dlg[2].dp=(void *) weaponlist;
  int ret=zc_popup_dialog(wlist_dlg,2);
  if(ret==0||ret==4)
  {
    position_mouse_z(0);
    return -1;
  }
  index = wlist_dlg[2].d1;
  position_mouse_z(0);
  return biw[index].i;
}






//asdfasdfasdf

item_struct bir[rMAX];
int bir_cnt=-1;

void build_bir_list()
{
  bir_cnt=0;

  for(int i=0; i<rMAX; i++)
  {
    if(roomtype_string[i][0]!='-')
    {
      bir[bir_cnt].s = roomtype_string[i];
      bir[bir_cnt].i = i;
      ++bir_cnt;
    }
  }

  for(int i=0; i<bir_cnt-1; i++)
  {
    for(int j=i+1; j<bir_cnt; j++)
    {
      if(stricmp(bir[i].s,bir[j].s)>0)
      {
        swap(bir[i],bir[j]);
      }
    }
  }
}

char *roomlist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = bir_cnt;
    return NULL;
  }
  return bir[index].s;
}

int select_room(char *prompt,int room)
{
  if(bir_cnt==-1)
  {
    build_bir_list();
  }

  int index=0;

  for(int j=0; j<bir_cnt; j++)
  {
    if(bir[j].i == room)
    {
      index=j;
    }
  }

  rlist_dlg[0].dp=prompt;
  rlist_dlg[0].dp2=lfont;
  rlist_dlg[2].d1=index;
  rlist_dlg[2].dp=(void *) roomlist;
  int ret=zc_popup_dialog(rlist_dlg,2);
  if (ret==0||ret==4)
  {
    position_mouse_z(0);
    return -1;
  }
  index = rlist_dlg[2].d1;
  position_mouse_z(0);
  return bir[index].i;
}

//adsfasdfasdfasdfasdf


int select_data(char *prompt,int index,char *(proc)(int,int*), FONT *title_font)
{
  if(proc==NULL)
    return -1;
  list_dlg[0].dp=prompt;
  list_dlg[0].dp2=title_font;
  list_dlg[2].d1=index;
  list_dlg[2].dp=(void *) proc;
  int ret=zc_popup_dialog(list_dlg,2);
  if(ret==0||ret==4)
  {
    position_mouse_z(0);
    return -1;
  }
  return list_dlg[2].d1;
}

int select_data(char *prompt,int index,char *(proc)(int,int*),char *b1, char *b2, FONT *title_font)
{

  if(proc==NULL)
    return -1;
  list_dlg[0].dp=prompt;
  list_dlg[0].dp2=title_font;
  list_dlg[2].d1=index;
  list_dlg[2].dp=(void *) proc;
  list_dlg[3].dp=b1;
  list_dlg[4].dp=b2;
  int ret = zc_popup_dialog(list_dlg,2);
  list_dlg[3].dp=(void *) "OK";
  list_dlg[4].dp=(void *) "Cancel";
  if(ret==0||ret==4)
  {
    position_mouse_z(0);
    return -1;
  }
  position_mouse_z(0);
  return list_dlg[2].d1;
}

static int edit_scrdata1[] =
{
  6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,36,37,-1
};

static int edit_scrdata2[] =
{
  28,29,30,31,32,33,34,35,62,63,64,65,66,67,68,69,70,71,72,73,
  74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,
  93,94,97,98,-1
};

static int edit_scrdata3[] =
{
  38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,-1
};

static int edit_scrdata4[] =
{
  95,96,-1
};

static TABPANEL scrdata_tabs[] = {
  { "Screen Flags 1", D_SELECTED, edit_scrdata1 },
  { "Screen Flags 2", 0, edit_scrdata3 },
  { "S. Data 1", 0, edit_scrdata2 },
  { "S. Data 2", 0, edit_scrdata4 },
  { NULL }
};

static DIALOG scrdata_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,       8,    48-29,   304+1,  156+1+38+7+10, vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Screen Data" },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  // 2
  { jwin_button_proc,     90,   176+30,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  176+30,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { jwin_tab_proc,        13,   44,   295,  147+10,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) scrdata_tabs, NULL, (void *)scrdata_dlg },
  // 6
  { jwin_check_proc,      17,   68,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Block->Shutters" },
  { jwin_check_proc,      17,   78,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enemies->Item" },
  { jwin_check_proc,      17,   88,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Dark Room" },
  { jwin_check_proc,      17,   98,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Boss Roar SFX" },

  { jwin_check_proc,      17,   108,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Whistle->Stairs" },
  { jwin_check_proc,      17,   118,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Use Ladder" },
  { jwin_check_proc,      17,   128,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Use Maze Path" },
  { jwin_check_proc,      17,   138,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Ocean Surf SFX" },

  { jwin_check_proc,      17,   148,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Secret SFX" },
  { jwin_check_proc,      17,   158,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Roar=Gasp Style" },
  { jwin_check_proc,      17,   168,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Floating Traps" },
  { jwin_check_proc,      17,   178,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enemies->Secret" },
  //18
  { jwin_check_proc,      167,  68,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Hold Up Item" },
  { jwin_check_proc,      167,  78,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Roar=Dodongo" },
  { jwin_check_proc,      167,  88,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Invisible Enemies" },
  { jwin_check_proc,      167,  98,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Invisible Link" },

  { jwin_check_proc,      167,  108,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "No Subscreen" },
  { jwin_check_proc,      167,  118,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Full-Screen Warp" },
  { jwin_check_proc,      167,  128,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "No Secret Sounds" },
  { jwin_check_proc,      167,  138,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enemies Always Return" },
  { jwin_check_proc,      167,  148,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Direct Timed Warp" },
  { jwin_check_proc,      167,  158,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Scrts Disable Time Wrp" },
  // 28
  { jwin_text_proc,       17,   182-22,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Timed Warp Tics:" },
  //  { jwin_edit_proc,       94+2,  178-6,  48,      16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { d_ticsedit_proc,       17+2,  178-6,  36,      16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  //  { d_ticsedit_proc,      68-12-3,  24+25+20+20+18+118-38+18,   36,   16,    vc(11),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,      17+2+36+1,  178-6+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,          17,   68,     200,    8,    vc(14),   vc(1),      0,      0,          0,             0,       (void *) "Room State Carry Over:" },
  { jwin_text_proc,          17,   88,     72,    8,    vc(14),   vc(1),      0,      0,          0,             0,       (void *) "Next Map:" },
  { jwin_text_proc,          17,   106,     96,    8,   vc(14),   vc(1),      0,      0,          0,             0,       (void *) "Next Screen:" },
  { jwin_droplist_proc,      90,   84,   54,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) nextmaplist },
//  { jwin_edit_proc,          90,    84,    32-6,   16,  vc(12),   vc(1),  0,       0,          3,             0,   NULL },
  { jwin_droplist_proc,      90,   102,   54,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) nslist },
 // { jwin_edit_proc,       17,   114,   32-6,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // { d_hexedit_proc,      97,   102,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },

  { jwin_check_proc,      167,  168,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Permanent Enemies->Secret" },
  { jwin_check_proc,      167,  178,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Direct Auto-warp" },
  //38
  { jwin_check_proc,      17,  68,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Use as Save Screen(SP)" },
  { jwin_check_proc,      17,  78,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Use as Save Screen(Auto)" },
  { jwin_check_proc,      17,  88,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Random Timed Warp" },
  { jwin_check_proc,      17,  98,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Dmg Combos Ignore Boots" },
  { jwin_check_proc,      17,  108,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Direct Sensitive Warp" },
  { jwin_check_proc,      17,  118,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Secrets are Temporary" },
  //44
  { jwin_check_proc,      17,  128,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Toggle diving" },
  { jwin_check_proc,      17,  138,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Treat as Cave Room" },
  { jwin_check_proc,      17,  148,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Treat as Dungeon Room" },
  { jwin_check_proc,      17,  158,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "All Triggers->Perm Scrt" },
  { jwin_check_proc,      17,  168,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Continue Here" },
  { jwin_check_proc,      17,  178,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Don't Continue Here" },
  //50
  { jwin_check_proc,      167,  68,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "All Triggers->16-31" },
  { jwin_check_proc,      167,  78,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Toggle Ring Affect Dmg" },
  { jwin_check_proc,      167,  88,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "FF Combos Wrap Around" },
  { jwin_check_proc,      167,  98,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "No FFC carryover" },
  { jwin_check_proc,      167,  108,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Layer 3 Is Background" },
  { jwin_check_proc,      167,  118,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Layer 2 Is Background" },
  //56
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dummy_proc,      160,  56-24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  //62
  { jwin_ctext_proc,       225,   68,  180,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Reset     /   No Carry Over" },
  { jwin_ctext_proc,       225,   78,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Secrets" },
  { jwin_ctext_proc,       225,   88,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Item" },
  { jwin_ctext_proc,       225,   98,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Special Item" },
  { jwin_ctext_proc,       225,   108,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Lock Block" },
  { jwin_ctext_proc,       225,   118,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss Lock Block" },
  { jwin_ctext_proc,       225,   128,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Chest" },
  { jwin_ctext_proc,       225,   138,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Locked Chest" },
  { jwin_ctext_proc,       225,   148,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss Locked Chest" },
  { jwin_ctext_proc,       225,   168,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Door Down(D)" },
  { jwin_ctext_proc,       225,   178,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Door Left(D)" },
  { jwin_ctext_proc,       225,   188,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Door Right(D)" },
  //74
  { jwin_check_proc,      160,  78,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  88,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  98,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  108,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  118,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  128,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  138,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL},
  { jwin_check_proc,      160,  148,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  168,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  178,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      160,  188,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  78,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  88,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  98,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  108,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  118,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  128,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  138,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,      280,  148,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  //93
  { jwin_text_proc,      17,  130,   80,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Screen Midi:" },
  { jwin_droplist_proc,   17,   138,   133,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) screenmidilist },
  { jwin_text_proc,          17,   68,     200,    8,    vc(14),   vc(1),      0,      0,          0,             0,       (void *) "Damage Combo Sensitivity:" },
  { jwin_edit_proc,      17,   84,   16,   16,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },
  //97
  { jwin_ctext_proc,       225,   158,  140,    8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Door Up(D)" },
  { jwin_check_proc,      160,  158,  8+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },

  { NULL }
};

int onScrData()
{
  restore_mouse();
  char timedstring[6];
 // char nmapstring[4];
 // char nscrstring[3];
  char csensstring[2];
  char tics_secs_str[80];
  sprintf(tics_secs_str, "=0.00 seconds");
  byte f = Map.CurrScr()->flags;
  scrdata_dlg[0].dp2=lfont;
  sprintf(timedstring,"%d",Map.CurrScr()->timedwarptics);
//  sprintf(nmapstring,"%d",(int)Map.CurrScr()->nextmap);
 // sprintf(nscrstring,"%x",(int)Map.CurrScr()->nextscr);
  sprintf(csensstring,"%d",(int)Map.CurrScr()->csensitive);
  for(int i=0; i<8; i++)
  {
    scrdata_dlg[i+6].flags = (f&1) ? D_SELECTED : 0;
    f>>=1;
  }

  f = Map.CurrScr()->flags2 >> 4;
  for(int i=0; i<4; i++)
  {
    scrdata_dlg[i+14].flags = (f&1) ? D_SELECTED : 0;
    f>>=1;
  }

  f = Map.CurrScr()->flags3;
  for(int i=0; i<8; i++)
  {
    scrdata_dlg[i+18].flags = (f&1) ? D_SELECTED : 0;
    f>>=1;
  }
  f = Map.CurrScr()->flags4;
  scrdata_dlg[26].flags = (f&4) ? D_SELECTED : 0;
  scrdata_dlg[27].flags = (f&8) ? D_SELECTED : 0;
  scrdata_dlg[29].dp=timedstring;
  scrdata_dlg[30].dp=tics_secs_str;
 // scrdata_dlg[34].dp=nmapstring;
  scrdata_dlg[34].d1 = (Map.CurrScr()->nextmap);
 // scrdata_dlg[35].dp=nscrstring;
  scrdata_dlg[35].d1 = (Map.CurrScr()->nextscr);
  scrdata_dlg[96].dp=csensstring;
  scrdata_dlg[36].flags = (f&16) ? D_SELECTED : 0;
  //scrdata_dlg[37].flags = (f&32) ? D_SELECTED : 0;
  scrdata_dlg[38].flags = (f&64) ? D_SELECTED : 0;
  scrdata_dlg[39].flags = (f&128) ? D_SELECTED : 0;
  f = Map.CurrScr()->flags5;
  scrdata_dlg[40].flags = (f&1) ? D_SELECTED : 0;
  scrdata_dlg[41].flags = (f&2) ? D_SELECTED : 0;
  scrdata_dlg[37].flags = (f&4) ? D_SELECTED : 0;
  scrdata_dlg[42].flags = (f&8) ? D_SELECTED : 0;
  scrdata_dlg[43].flags = (f&16) ? D_SELECTED : 0;
  scrdata_dlg[44].flags = (f&64) ? D_SELECTED : 0;
  scrdata_dlg[53].flags = (f&128) ? D_SELECTED : 0;
  f = Map.CurrScr()->flags6;
  scrdata_dlg[45].flags = (f&1) ? D_SELECTED : 0;
  scrdata_dlg[46].flags = (f&2) ? D_SELECTED : 0;
  scrdata_dlg[47].flags = (f&4) ? D_SELECTED : 0;
  scrdata_dlg[48].flags = (f&8) ? D_SELECTED : 0;
  scrdata_dlg[49].flags = (f&16) ? D_SELECTED : 0;
  scrdata_dlg[50].flags = (f&32) ? D_SELECTED : 0;
  scrdata_dlg[51].flags = (f&64) ? D_SELECTED : 0;
  scrdata_dlg[52].flags = (f&128) ? D_SELECTED : 0;
  f = Map.CurrScr()->flags7;
  scrdata_dlg[54].flags = (f&1) ? D_SELECTED : 0;
  scrdata_dlg[55].flags = (f&2) ? D_SELECTED : 0;


  word g = Map.CurrScr()->noreset;
  scrdata_dlg[74].flags = (g&mSECRET) ? D_SELECTED : 0;
  scrdata_dlg[75].flags = (g&mITEM) ? D_SELECTED : 0;
  scrdata_dlg[76].flags = (g&mBELOW) ? D_SELECTED : 0;
  scrdata_dlg[77].flags = (g&mLOCKBLOCK) ? D_SELECTED : 0;
  scrdata_dlg[78].flags = (g&mBOSSLOCKBLOCK) ? D_SELECTED : 0;
  scrdata_dlg[79].flags = (g&mCHEST) ? D_SELECTED : 0;
  scrdata_dlg[80].flags = (g&mLOCKEDCHEST) ? D_SELECTED : 0;
  scrdata_dlg[81].flags = (g&mBOSSCHEST) ? D_SELECTED : 0;
  scrdata_dlg[82].flags = (g&mDOOR_DOWN) ? D_SELECTED : 0;
  scrdata_dlg[83].flags = (g&mDOOR_LEFT) ? D_SELECTED : 0;
  scrdata_dlg[84].flags = (g&mDOOR_RIGHT) ? D_SELECTED : 0;
  scrdata_dlg[98].flags = (g&mDOOR_UP) ? D_SELECTED : 0;
  g = Map.CurrScr()->nocarry;
  scrdata_dlg[85].flags = (g&mSECRET) ? D_SELECTED : 0;
  scrdata_dlg[86].flags = (g&mITEM) ? D_SELECTED : 0;
  scrdata_dlg[87].flags = (g&mBELOW) ? D_SELECTED : 0;
  scrdata_dlg[88].flags = (g&mLOCKBLOCK) ? D_SELECTED : 0;
  scrdata_dlg[89].flags = (g&mBOSSLOCKBLOCK) ? D_SELECTED : 0;
  scrdata_dlg[90].flags = (g&mCHEST) ? D_SELECTED : 0;
  scrdata_dlg[91].flags = (g&mLOCKEDCHEST) ? D_SELECTED : 0;
  scrdata_dlg[92].flags = (g&mBOSSCHEST) ? D_SELECTED : 0;

  scrdata_dlg[94].d1 = (Map.CurrScr()->screen_midi>=0)?(Map.CurrScr()->screen_midi+1):(-(Map.CurrScr()->screen_midi+1));

  if(zc_popup_dialog(scrdata_dlg,-1)==2)
  {
    f=0;
    for(int i=7; i>=0; i--)
    {
      f<<=1;
      f |= scrdata_dlg[i+6].flags == D_SELECTED ? 1:0;
    }
    Map.CurrScr()->flags = f;

    f=0;
    for(int i=3; i>=0; i--)
    {
      f<<=1;
      f |= scrdata_dlg[i+14].flags == D_SELECTED ? 1:0;
    }
    Map.CurrScr()->flags2 &= 0x0F;
    Map.CurrScr()->flags2 |= f<<4;

    f=0;
    for(int i=8; i>=0; i--)
    {
      f<<=1;
      f |= scrdata_dlg[i+18].flags == D_SELECTED ? 1:0;
    }
    Map.CurrScr()->flags3 = f;
    f=0;
    f |= scrdata_dlg[26].flags == D_SELECTED ? 4:0;
    f |= scrdata_dlg[27].flags == D_SELECTED ? 8:0;
    f |= scrdata_dlg[36].flags == D_SELECTED ? 16:0;
    //f |= scrdata_dlg[37].flags == D_SELECTED ? 32:0;
    f |= scrdata_dlg[38].flags == D_SELECTED ? 64:0;
    f |= scrdata_dlg[39].flags == D_SELECTED ? 128:0;
    Map.CurrScr()->flags4 = f;
    f=0;
    f |= scrdata_dlg[40].flags == D_SELECTED ? 1:0;
    f |= scrdata_dlg[41].flags == D_SELECTED ? 2:0;
    f |= scrdata_dlg[37].flags == D_SELECTED ? 4:0;
    f |= scrdata_dlg[42].flags == D_SELECTED ? 8:0;
    f |= scrdata_dlg[43].flags == D_SELECTED ? 16:0;
    f |= scrdata_dlg[44].flags == D_SELECTED ? 64:0;
	f |= scrdata_dlg[53].flags == D_SELECTED ? 128:0;
    Map.CurrScr()->flags5 = f;
    f=0;
    f |= scrdata_dlg[45].flags == D_SELECTED ? 1:0;
    f |= scrdata_dlg[46].flags == D_SELECTED ? 2:0;
    f |= scrdata_dlg[47].flags == D_SELECTED ? 4:0;
	f |= scrdata_dlg[48].flags == D_SELECTED ? 8:0;
	f |= scrdata_dlg[49].flags == D_SELECTED ? 16:0;
	f |= scrdata_dlg[50].flags == D_SELECTED ? 32:0;
	f |= scrdata_dlg[51].flags == D_SELECTED ? 64:0;
	f |= scrdata_dlg[52].flags == D_SELECTED ? 128:0;
    Map.CurrScr()->flags6 = f;
   	f=0;
	f |= scrdata_dlg[54].flags == D_SELECTED ? 1:0;
	f |= scrdata_dlg[55].flags == D_SELECTED ? 2:0;
	Map.CurrScr()->flags7 = f;


    word g=0;
    g |= scrdata_dlg[74].flags == D_SELECTED ? mSECRET:0;
    g |= scrdata_dlg[75].flags == D_SELECTED ? mITEM:0;
    g |= scrdata_dlg[76].flags == D_SELECTED ? mBELOW:0;
    g |= scrdata_dlg[77].flags == D_SELECTED ? mLOCKBLOCK:0;
    g |= scrdata_dlg[78].flags == D_SELECTED ? mBOSSLOCKBLOCK:0;
    g |= scrdata_dlg[79].flags == D_SELECTED ? mCHEST:0;
    g |= scrdata_dlg[80].flags == D_SELECTED ? mLOCKEDCHEST:0;
    g |= scrdata_dlg[81].flags == D_SELECTED ? mBOSSCHEST:0;
    g |= scrdata_dlg[82].flags == D_SELECTED ? mDOOR_DOWN:0;
    g |= scrdata_dlg[83].flags == D_SELECTED ? mDOOR_LEFT:0;
    g |= scrdata_dlg[84].flags == D_SELECTED ? mDOOR_RIGHT:0;
	g |= scrdata_dlg[98].flags == D_SELECTED ? mDOOR_UP:0;
    Map.CurrScr()->noreset = g;
    g=0;
    g |= scrdata_dlg[85].flags == D_SELECTED ? mSECRET:0;
    g |= scrdata_dlg[86].flags == D_SELECTED ? mITEM:0;
    g |= scrdata_dlg[87].flags == D_SELECTED ? mBELOW:0;
    g |= scrdata_dlg[88].flags == D_SELECTED ? mLOCKBLOCK:0;
    g |= scrdata_dlg[89].flags == D_SELECTED ? mBOSSLOCKBLOCK:0;
    g |= scrdata_dlg[90].flags == D_SELECTED ? mCHEST:0;
    g |= scrdata_dlg[91].flags == D_SELECTED ? mLOCKEDCHEST:0;
    g |= scrdata_dlg[92].flags == D_SELECTED ? mBOSSCHEST:0;
    Map.CurrScr()->nocarry = g;

    Map.CurrScr()->screen_midi = (scrdata_dlg[94].d1>1)?(scrdata_dlg[94].d1-1):(-(scrdata_dlg[94].d1+1));
    Map.CurrScr()->nextmap = scrdata_dlg[34].d1;
    Map.CurrScr()->nextscr = scrdata_dlg[35].d1;
	
    refresh(rMAP+rSCRMAP+rMENU);
    Map.CurrScr()->timedwarptics=atoi(timedstring);
    //Z_message("%s\n",nmapstring);
    //Z_message("%s\n",nscrstring);
    //Map.CurrScr()->nextmap=(atoi(nmapstring)<=255?max(0,atoi(nmapstring)):Map.CurrScr()->nextmap);
   // Map.CurrScr()->nextscr=(xtoi(nscrstring)<128?max(0,xtoi(nscrstring)):Map.CurrScr()->nextscr);
	Map.CurrScr()->csensitive=(atoi(csensstring)<=8?max(1,atoi(csensstring)):Map.CurrScr()->csensitive);
    saved=false;
  }
  return D_O_K;
}

char *nslist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,MAXSCREENS);
    sprintf(ns_string, " %02X", index);
    return ns_string;
  }
  *list_size=MAXSCREENS;
  return NULL;
}

char *flaglist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=MAXFLAGS)
      index=MAXFLAGS-1;
    return flag_string[index];
  }
  *list_size=MAXFLAGS;
  return NULL;
}

char *roomslist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=MAXROOMTYPES)
      index=MAXROOMTYPES-1;
    return roomtype_string[index];
  }
  *list_size=MAXROOMTYPES;
  return NULL;
}

/*
char *guyslist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index >= zqMAXGUYS)
      index = zqMAXGUYS-1;
    return guy_string[index];
  }
  *list_size = zqMAXGUYS;
  return NULL;
}
*/

char *msgslist(int index, int *list_size)
{
  static char buf[80];
  if(index>=0)
  {
    if(index>=msg_count)
    {
      index=msg_count-1;
    }
    memcpy(buf,MsgString(index),80);
    return strip_extra_spaces(buf);
  }
  *list_size=msg_count;
  return NULL;
}

static char number_str_buf[32];
int number_list_size=1;
bool number_list_zero=false;

char *numberlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,number_list_size-1);
    sprintf(number_str_buf,"%d",index+(number_list_zero?0:1));
    return number_str_buf;
  }
  *list_size=number_list_size;
  return NULL;
}

static char dmap_str_buf[37];
int dmap_list_size=1;
bool dmap_list_zero=true;

char *dmaplist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,dmap_list_size-1);
    sprintf(dmap_str_buf,"%3d-%s",index+(dmap_list_zero?0:1), DMaps[index].name);
    return dmap_str_buf;
  }
  *list_size=dmap_list_size;
  return NULL;
}

char *hexnumlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,number_list_size-1);
    sprintf(number_str_buf,"%X",index+(number_list_zero?0:1));
    return number_str_buf;
  }
  *list_size=number_list_size;
  return NULL;
}

char *maplist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,MAXMAPS2-1);
    sprintf(number_str_buf,"%d",index+1);
    return number_str_buf;
  }
  *list_size=MAXMAPS2;
  return NULL;
}

char *gotomaplist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,map_count-1);
    sprintf(number_str_buf,"%d",index+1);
    return number_str_buf;
  }
  *list_size = map_count;
  return NULL;
}

char *nextmaplist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,map_count);
    sprintf(number_str_buf,"%3d",index);
    return number_str_buf;
  }
  *list_size = map_count+1;
  return NULL;
}

char *midilist(int index, int *list_size)
{
  if(index>=0)

  {
    bound(index,0,MAXCUSTOMMIDIS_ZQ-1);
    return midi_string[index];
  }
  *list_size=MAXCUSTOMMIDIS_ZQ;
  return NULL;
}

char *screenmidilist(int index, int *list_size)
{
  if(index>=0)

  {
    bound(index,0,MAXCUSTOMMIDIS_ZQ);
    return screen_midi_string[index];
  }
  *list_size=MAXCUSTOMMIDIS_ZQ+1;
  return NULL;
}

char *custommidilist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,MAXCUSTOMMIDIS_ZQ-1);
    sprintf(number_str_buf,"%3d - %s",index+(number_list_zero?0:1),customMIDIs[index].midi?customMIDIs[index].title:"(Empty)" );
    return number_str_buf;
  }
  *list_size=number_list_size;
  return NULL;
}

char *enhancedmusiclist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,MAXMUSIC-1);
    sprintf(number_str_buf,"%3d - %s",index+(number_list_zero?0:1),enhancedMusic[index].filename[0]?enhancedMusic[index].title:"(Empty)" );
    return number_str_buf;
  }
  *list_size=number_list_size;
  return NULL;
}

char *levelnumlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,255);
    sprintf(number_str_buf,"%.2X - %s",index,palnames[index]);
    return number_str_buf;
  }
  *list_size=MAXLEVELS;
  return NULL;
}

int onMapCount()
{
  int oldmapcount=map_count-1;
  int ret = select_data("Number of Maps",map_count-1,maplist, lfont);
  if((ret >= 0) && (ret!=oldmapcount))
  {
    saved = false;
    setMapCount2(ret+1,true);
  }
  refresh(rMAP+rSCRMAP+rMENU);
  return D_O_K;
}

int onGotoMap()
{
  int ret = select_data("Goto Map",Map.getCurrMap(),gotomaplist,lfont);
  if(ret >= 0)
    Map.setCurrMap(ret);
  refresh(rALL);
  return D_O_K;
}

int onFlags()
{
  restore_mouse();
  int ret=select_data("Flag Type",Flag,flaglist,lfont);
  position_mouse_z(0);
  if(ret>=0)
  {
    Flag=ret;
    setFlagColor();
    refresh(rMENU);
    doflags();
  }
  return D_O_K;
}

static DIALOG usedcombo_list_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,      (void *) "Combos Used" },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_textbox_proc,       72-12,   60+4,   176+24+1,  92+4,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,      NULL },
  { jwin_button_proc,     130,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { NULL }
};



int onUsedCombos()
{
  restore_mouse();
  usedcombo_list_dlg[0].dp2=lfont;

  int usedcombos[7][300][2];
  char combolist_text[65536];
  char temptext[80];

  int drawmap=Map.getCurrMap();
  int drawscr=Map.getCurrScr();
  int counter[7];
  for (int layer=0; layer<7; ++layer)
  {
    counter[layer]=0;
    if(layer==0)
    {
      drawmap=Map.getCurrMap();
      drawscr=Map.getCurrScr();
    }
    else
    {
      if(Map.AbsoluteScr(Map.getCurrMap(), Map.getCurrScr())->layermap[layer-1]>0)
      {
        drawmap=Map.AbsoluteScr(Map.getCurrMap(), Map.getCurrScr())->layermap[layer-1]-1;
        drawscr=Map.AbsoluteScr(Map.getCurrMap(), Map.getCurrScr())->layerscreen[layer-1];
      }
      else
      {
        continue;
      }
    }
    usedcombos[layer][0][0]=Map.AbsoluteScr(drawmap, drawscr)->data[0];
    usedcombos[layer][0][1]=1;
    counter[layer]=1;
    for (int i=1; i<176; ++i)
    {
      bool used=false;
      for (int j=0; j<counter[layer]; ++j)
      {
        if (usedcombos[layer][j][0]==Map.AbsoluteScr(drawmap, drawscr)->data[i])
        {
          ++usedcombos[layer][j][1];
          used=true;
          break;
        }
      }
      if (!used)
      {
        usedcombos[layer][counter[layer]][0]=Map.AbsoluteScr(drawmap, drawscr)->data[i];
        usedcombos[layer][counter[layer]][1]=1;
        ++counter[layer];
      }
    }
    for(int i=0; i<counter[layer]-1; i++)
    {
      for(int j=i+1; j<counter[layer]; j++)
      {
        if(usedcombos[layer][i][0]>usedcombos[layer][j][0])
        {
          swap(usedcombos[layer][i][0],usedcombos[layer][j][0]);
          swap(usedcombos[layer][i][1],usedcombos[layer][j][1]);
        }
      }
    }
  }

  sprintf(combolist_text, " ");
  for (int layer=0; layer<7; ++layer)
  {
    if(counter[layer]>0)
    {
      if(layer>0)
      {
        strcat(combolist_text, "\n");
      }
      sprintf(temptext, "Combos on layer %d\n-----------------\n", layer);
      strcat(combolist_text, temptext);

      for(int i=0; i<counter[layer]; i++)
      {
        if ((i<counter[layer]-1) && (((usedcombos[layer][i][1]==usedcombos[layer][i+1][1]&&(usedcombos[layer][i][0]+1==usedcombos[layer][i+1][0])) && ((i==0) || ((usedcombos[layer][i][1]!=usedcombos[layer][i-1][1])||((usedcombos[layer][i][0]-1!=usedcombos[layer][i-1][0])))))))
        {
          sprintf(temptext, "%5d ", usedcombos[layer][i][0]);
          strcat(combolist_text, temptext);
        }
        else if (((i>0) && (((usedcombos[layer][i][1]==usedcombos[layer][i-1][1])&&((usedcombos[layer][i][0]-1==usedcombos[layer][i-1][0]))) && ((i==counter[layer]-1) || ((usedcombos[layer][i][1]!=usedcombos[layer][i+1][1])||((usedcombos[layer][i][0]+1!=usedcombos[layer][i+1][0])))))))
          {
            sprintf(temptext, "- %5d (%3d)\n", usedcombos[layer][i][0],usedcombos[layer][i][1]);
            strcat(combolist_text, temptext);
          }
          else if (((i==0) && ((usedcombos[layer][i][1]!=usedcombos[layer][i+1][1])||((usedcombos[layer][i][0]+1!=usedcombos[layer][i+1][0]))))||
                   ((i==counter[layer]-1) && ((usedcombos[layer][i][1]!=usedcombos[layer][i-1][1])||((usedcombos[layer][i][0]-1!=usedcombos[layer][i-1][0]))))||
                   ((i>0) && (i<counter[layer]-1) && ((usedcombos[layer][i][1]!=usedcombos[layer][i+1][1])||((usedcombos[layer][i][0]+1!=usedcombos[layer][i+1][0]))) && ((usedcombos[layer][i][1]!=usedcombos[layer][i-1][1])||((usedcombos[layer][i][0]-1!=usedcombos[layer][i-1][0])))))
            {
              sprintf(temptext, "    %5d     (%3d)\n", usedcombos[layer][i][0],usedcombos[layer][i][1]);
              strcat(combolist_text, temptext);
            }
      }
    }
  }
  strcat(combolist_text, "\n");
  usedcombo_list_dlg[2].dp=combolist_text;
  usedcombo_list_dlg[2].d2=0;
  zc_popup_dialog(usedcombo_list_dlg,2);
  position_mouse_z(0);
  return D_O_K;
}

int onItem()
{
  restore_mouse();
  build_bii_list(true);
  int ret=select_item("Select Item",Map.CurrScr()->item,0);
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->item=ret;
  }
  refresh(rMAP+rMENU);
  return D_O_K;
}

int onRType()
{
  if(prv_mode)
  {
    Map.set_prvscr(Map.get_prv_map(), Map.get_prv_scr());
    Map.set_prvcmb(0);
    return D_O_K;
  }
  restore_mouse();
  build_bir_list();
  int ret=select_room("Select Room Type",Map.CurrScr()->room);
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->room=ret;
  }
  int c=Map.CurrScr()->catchall;
  switch(Map.CurrScr()->room)
  {
    case 1:
    Map.CurrScr()->catchall=bound(c,0,ITEMCNT-1);
    break;
    // etc...
  }
  refresh(rMENU);

  return D_O_K;
}

/*
int onGuy()
{
  restore_mouse();
  int ret=select_data("Select Guy",Map.CurrScr()->guy,guyslist,lfont);
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->guy=ret;
  }
  refresh(rMENU);
  return D_O_K;
}
*/

int onGuy()
{
  restore_mouse();
  build_big_list(true);
  int ret=select_guy("Select Guy",Map.CurrScr()->guy);
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->guy=ret;
  }
  refresh(rMAP+rMENU);
  return D_O_K;
}

int onString()
{
  if(prv_mode)
  {
    Map.prv_secrets(false);
    refresh(rALL);
    return D_O_K;
  }
  restore_mouse();
  int ret=select_data("Select Message String",Map.CurrScr()->str,msgslist,lfont);
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->str=ret;
  }
  refresh(rMENU);
  return D_O_K;
}

int onEndString()
{
  int ret=select_data("Select Ending String",misc.endstring,msgslist,lfont);
  if(ret>=0)
  {
    saved=false;
    misc.endstring=ret;
  }
  refresh(rMENU);
  return D_O_K;
}

int onCatchall()
{
  if(prv_mode)
  {
    Map.set_prvadvance(1);
    return D_O_K;
  }
  if(data_menu[11].flags==D_DISABLED)
  {
    return D_O_K;
  }

  restore_mouse();
  int ret=-1;
  int rtype=Map.CurrScr()->room;

  switch(rtype)
  {
    case 1:
    build_bii_list(false);
    ret=select_item("Select Special Item",Map.CurrScr()->catchall,0);
    if(ret>=0)
    {
      saved=false;
      Map.CurrScr()->catchall=ret;
    }
    break;
    default:
    char buf[80]="Enter ";
    strcat(buf,catchall_string[rtype]);
    ret=getnumber(buf,Map.CurrScr()->catchall);
    break;
  }
  if(ret>=0)
  {
    saved=false;
    Map.CurrScr()->catchall=ret;
  }
  refresh(rMENU);
  return D_O_K;
}

static DIALOG screen_pal_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      60-12,   40,   200-16,  72,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select Palette" },
  { d_dummy_proc,       160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_droplist_proc, 72-12,   60+4,   161,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) levelnumlist },
  { jwin_button_proc,   70,   87,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   150,  87,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};
//  return list_dlg[2].d1;

int onScreenPalette()
{
  restore_mouse();
  screen_pal_dlg[0].dp2=lfont;
  screen_pal_dlg[2].d1=Map.getcolor();
  if(zc_popup_dialog(screen_pal_dlg,2)==3)
  {
    saved=false;
    Map.setcolor(screen_pal_dlg[2].d1);
    refresh(rALL);
  }
  return D_O_K;
}

int onDecScrPal()
{
  restore_mouse();
  int c=Map.getcolor();
  c+=255;
  c=c%256;
  Map.setcolor(c);
  refresh(rALL);
  return D_O_K;
}

int onIncScrPal()
{
  restore_mouse();
  int c=Map.getcolor();
  c+=1;
  c=c%256;
  Map.setcolor(c);
  refresh(rALL);
  return D_O_K;
}

bool edit_ins_mode=true;

void put_msg_str(char *s,int x,int y,int fg,int bg,int pos)
{
  int i=0;
  // text_mode(bg);
  for(int dy=0; dy<3; dy++)
    for(int dx=0; dx<24; dx++)
    {
      if(edit_ins_mode)
      {
        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),fg,bg,"%c",*(s+i));
      }
      else
      {
        //     text_mode(i==pos?vc(15):bg);

        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,i==pos?vc(15):bg,"%c",*(s+i));
      }
      ++i;
    }
  if(edit_ins_mode&&pos>-1)
  {
    //   text_mode(-1);
    textprintf_ex(screen,zfont,x+((pos%24)<<3),y+((pos/24)<<3),vc(15),-1,"_");
  }
}

int d_msg_edit_proc(int msg,DIALOG *d,int c)

{
  char *s=(char*)(d->dp);
  switch(msg)
  {
    case MSG_WANTFOCUS:
    return D_WANTFOCUS;
    case MSG_CLICK:
    d->d2=((gui_mouse_x()-d->x)>>3)+((gui_mouse_y()-d->y)>>3)*24;
    bound(d->d2,0,71);
    scare_mouse();
    put_msg_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    unscare_mouse();
    while(gui_mouse_b());
    break;
    case MSG_DRAW:
    if (!(d->flags & D_GOTFOCUS))
    {
      d->d2=-1;
    }
    put_msg_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    break;
    case MSG_CHAR:
    bool used=false;
    int k=c>>8;
    switch(k)
    {
      case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
      case KEY_HOME:  d->d2-=d->d2%24; used=true; break;
      case KEY_END:   d->d2-=d->d2%24; d->d2+=23; used=true; break;
      case KEY_UP:    if(d->d2>=24) d->d2-=24; used=true; break;
      case KEY_DOWN:  if(d->d2<48) d->d2+=24; used=true; break;
      case KEY_LEFT:  if(d->d2>0) --d->d2; used=true; break;
      case KEY_RIGHT: if(d->d2<71) ++d->d2; used=true; break;
      case KEY_BACKSPACE:
      if(d->d2>0)
        --d->d2;
      case KEY_DEL:
      strcpy(s+d->d2,s+d->d2+1);
      s[71]=' ';
      s[72]=0;
      used=true;
      break;
      default:
      if(isprint(c&255))
      {
        if(edit_ins_mode)
        {
          for(int i=71; i>d->d2; i--)
            s[i]=s[i-1];
        }
        s[d->d2]=c&255;
        if(d->d2<71)
          ++d->d2;
        used=true;
      }
    }
    scare_mouse();
    put_msg_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    unscare_mouse();
    return used?D_USED_CHAR:D_O_K;
  }
  return D_O_K;
}

char msgbuf[73];

int d_nlist_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_droplist_proc(msg,d,c);
  switch(msg)
  {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:
    scare_mouse();
    //    text_mode(d->bg);
    textprintf_ex(screen,font,d->x - 48,d->y + 4,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%5d",d->d1);
    unscare_mouse();
  }
  return ret;
}

int d_nilist_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_droplist_proc(msg,d,c);
  switch(msg)
  {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:
    scare_mouse();
    //    text_mode(d->bg);
    textprintf_ex(screen,font,d->x - 48,d->y + 4,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%5d",bii[d->d1].i);
    unscare_mouse();
  }
  return ret;
}

static DIALOG editmsg_dlg[] =
{

  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 44,   55,   233,  119+2,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_box_proc,        63,   91,     193, 25,  vc(0),  vc(0),  0,       0,          0,             0,       NULL },
  { jwin_frame_proc,        61,  89,   197,  29,   vc(14),  vc(1),  0,       0,          FR_DEEP,             0,       NULL },

  { d_msg_edit_proc,   64,   92,   193,  25,   jwin_pal[jcTEXTBG],  jwin_pal[jcTEXTFG],  0,       0,          0,             0,       msgbuf },
  { jwin_text_proc,       52,   121,  168,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Next string:" },
  { d_nlist_proc,      100,  131,  168,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       NULL },
  { jwin_button_proc,     90,   149+2,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  149+2,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_ctext_proc,       68,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "1" },
  { jwin_ctext_proc,       76,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "2" },
  { jwin_ctext_proc,       84,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "3" },
  { jwin_ctext_proc,       92,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "4" },
  { jwin_ctext_proc,       100,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "5" },
  { jwin_ctext_proc,       108,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "6" },
  { jwin_ctext_proc,       116,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "7" },
  { jwin_ctext_proc,       124,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "8" },
  { jwin_ctext_proc,       132,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "9" },
  { jwin_ctext_proc,       140,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "0" },
  { jwin_ctext_proc,       148,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "-" },
  { jwin_ctext_proc,       156,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "+" },
  { jwin_ctext_proc,       164,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "+" },
  { jwin_ctext_proc,       172,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "-" },
  { jwin_ctext_proc,       180,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "0" },
  { jwin_ctext_proc,       188,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "9" },
  { jwin_ctext_proc,       196,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "8" },
  { jwin_ctext_proc,       204,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "7" },
  { jwin_ctext_proc,       212,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "6" },
  { jwin_ctext_proc,       220,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "5" },
  { jwin_ctext_proc,       228,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "4" },
  { jwin_ctext_proc,       236,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "3" },
  { jwin_ctext_proc,       244,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "2" },
  { jwin_ctext_proc,       252,   81,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "1" },
  { d_keyboard_proc,         0,    0,     0,  0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { NULL }
};

void editmsg(int index)
{
  char setitle[80];
  sprintf(setitle, "String Editor (%d)", index);
  char *si=MsgStrings[index].s;
  for(int i=0; i<72; i++)
  {
    msgbuf[i] = index<msg_count ? (*si ? *si : ' ') : ' ';
    ++si;
  }
  msgbuf[72]=0;
  editmsg_dlg[0].dp=setitle;
  editmsg_dlg[0].dp2=lfont;
  editmsg_dlg[3].d2=0;
  editmsg_dlg[5].d1 = MsgStrings[index].nextstring;
  editmsg_dlg[5].dp = (void *) msgslist;
  // editmsg_dlg[4].flags = get_bit(&MsgStrings[index].d1,msLINKED) ? D_SELECTED : 0;

  if(zc_popup_dialog(editmsg_dlg,3)==6)
  {
    saved=false;
    strcpy(MsgStrings[index].s,msgbuf);
    MsgStrings[index].nextstring = editmsg_dlg[5].d1;
    if(index==msg_count)
      ++msg_count;
  }
}

int strlist_del();


static DIALOG strlist_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     60-12,   39,   200+24,  148+21,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Edit Message String" },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_list_proc,       72-12,   60+4,   176+24+1,  92+3,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *)msgslist },
  { jwin_button_proc,     90,   163+20,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  163+20,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       (void *) strlist_del },
  { jwin_text_proc,       60,   161+4,   128,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Message More X, Y: " },
  { jwin_edit_proc,       156,  161,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,       204-20,  161,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { NULL }
};

int strlist_del()
{
  if(strlist_dlg[2].d1>0 && strlist_dlg[2].d1<msg_count-1)
    return D_CLOSE;
  return D_O_K;
}

void fix_string(word &str, byte index)
{
  if(str == index)
    str = 0;
  else if(str > index)
      --str;
}

int onStrings()
{
  strlist_dlg[0].dp2=lfont;
  int index=0;
  char msgmore_xstring[5], msgmore_ystring[5];
  int morex=zinit.msg_more_x;
  int morey=zinit.msg_more_y;
  sprintf(msgmore_xstring, "%d", zinit.msg_more_x);
  sprintf(msgmore_ystring, "%d", zinit.msg_more_y);

  char tempbuf[50];
  sprintf(tempbuf, "0");
  while(index!=-1)
  {
    bool hasroom=false;
    if(msg_count<MAXMSGS)
    {
      hasroom=true;
      strcpy(MsgStrings[msg_count++].s,"<New String>");
    }

    strlist_dlg[7].dp=msgmore_xstring;
    strlist_dlg[8].dp=msgmore_ystring;

    int ret=zc_popup_dialog(strlist_dlg,2);
    index=strlist_dlg[2].d1;

    int doedit=false;

    switch(ret)
    {
      case 2:
      case 3: doedit=true; break;
      case 0:
      case 4:
      index=-1;
      zinit.msg_more_x=atoi(msgmore_xstring);
      zinit.msg_more_y=atoi(msgmore_ystring);
      if (morex!=zinit.msg_more_x||morey!=zinit.msg_more_y)
      {
        saved=false;
      }
      break;
      case 5:
      char buf[73], shortbuf[73];
      memset(buf, 0, 73);
      memset(shortbuf, 0, 73);
      strncpy(buf,MsgString(index),72);
      strip_extra_spaces(buf);
      shorten_string(shortbuf, buf, font, 72, 288);
      if(jwin_alert("Confirm Delete","Delete this message string?"," ",shortbuf,"Yes","No",'y',27,lfont)==1)
      {
        saved=false;
        for(int i=index; i<MAXMSGS-1; i++)
          MsgStrings[i]=MsgStrings[i+1];
        reset_msgstr(MAXMSGS-1);
        --msg_count;
        int sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;

        for(int s=0; s<sc; s++)                           //room strings
        {
          fix_string(TheMaps[s].str, index);
        }
        for(int i=0; i<16; i++)                           //info strings
        {
          for(int j=0; j<3; j++)
          {
            fix_string(misc.info[i].str[j], index);
          }
        }
        fix_string(misc.endstring, index);              //ending string

        refresh(rMENU);
        break;
      }
    }
    if(hasroom)
      strcpy(MsgStrings[--msg_count].s,"            ");

    if(index>0 && doedit)
    {
      editmsg(index);
      refresh(rMENU);
    }
  }
  return D_O_K;
}


int onStrings2()
{
  strlist_dlg[0].dp2=lfont;
  int index=0;
  char msgmore_xstring[5], msgmore_ystring[5];
  int morex=zinit.msg_more_x;
  int morey=zinit.msg_more_y;
  sprintf(msgmore_xstring, "%d", zinit.msg_more_x);
  sprintf(msgmore_ystring, "%d", zinit.msg_more_y);

  char tempbuf[50];
  sprintf(tempbuf, "0");
  while(index!=-1)
  {
    bool hasroom=false;
    if(msg_count<MAXMSGS)
    {
      hasroom=true;
      strcpy(MsgStrings[msg_count++].s,"<New String>");
    }

    strlist_dlg[7].dp=msgmore_xstring;
    strlist_dlg[8].dp=msgmore_ystring;

    int ret=zc_popup_dialog(strlist_dlg,2);
    index=strlist_dlg[2].d1;

    int doedit=false;

    switch(ret)
    {
      case 2:
      case 3: doedit=true; break;
      case 4:
      index=-1; //time to exit
      zinit.msg_more_x=atoi(msgmore_xstring);
      zinit.msg_more_y=atoi(msgmore_ystring);
      if (morex!=zinit.msg_more_x||morey!=zinit.msg_more_y)
      {
        saved=false;
      }
      break;
      case 5:
      char stripped_string[73];
      strncpy(stripped_string,MsgString(index),72);
      strip_extra_spaces(stripped_string);

      if(jwin_alert("Confirm Delete","Delete this message string?",NULL,stripped_string,"Yes","No",'y',27,lfont)==1)
      {
        saved=false;
        for(int i=index; i<MAXMSGS-1; i++)
          MsgStrings[i]=MsgStrings[i+1];
        reset_msgstr(MAXMSGS-1);
        --msg_count;
        int sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;

        for(int s=0; s<sc; s++)                           //room strings
        {
          fix_string(TheMaps[s].str, index);
        }
        for(int i=0; i<16; i++)                           //info strings
        {
          for(int j=0; j<3; j++)
          {
            fix_string(misc.info[i].str[j], index);
          }
        }
        fix_string(misc.endstring, index);              //ending string

        //refresh(rMENU);
      }
      break;
    }
    //    if(hasroom)
    //      strcpy(MsgStrings[--msg_count].s,"            ");

    if(index>0 && doedit)
    {
      if(hasroom)
        strcpy(MsgStrings[--msg_count].s,"            ");
      editmsg(index);
    }
  }
  refresh(rMENU);
  return D_O_K;
}

/**********************************/
//        Triforce Pieces         //
/**********************************/

static byte triframe_points[9*4] =
{
  0,2,2,0,  2,0,4,2,  0,2,4,2,  1,1,3,1,  2,0,2,2,
  1,1,1,2,  1,1,2,2,  3,1,3,2,  3,1,2,2
};

int d_tri_frame_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    int x[5],y[3];

    x[0]=d->x;
    x[1]=d->x+(d->w>>2);
    x[2]=d->x+(d->w>>1);
    x[3]=d->x+(d->w>>1)+(d->w>>2);
    x[4]=d->x+d->w;
    y[0]=d->y;
    y[1]=d->y+(d->h>>1);
    y[2]=d->y+d->h;

    byte *p = triframe_points;
    for(int i=0; i<9; i++)
    {
      line(screen,x[*p],y[*(p+1)],x[*(p+2)],y[*(p+3)],d->fg);
      p+=4;
    }
  }
  return D_O_K;
}

int d_tri_edit_proc(int msg,DIALOG *d,int c)
{
  jwin_button_proc(msg,d,c);
  if(msg==MSG_CLICK)
  {
    int v = getnumber("Piece Number",d->d1);
    scare_mouse();
    if(v>=0)
    {
      bound(v,1,8);
      if(v!=d->d1)
      {
        DIALOG *tp = d - d->d2;
        for(int i=0; i<8; i++)
        {
          if(tp->d1==v)
          {
            tp->d1 = d->d1;
            ((char*)(tp->dp))[0] = d->d1+'0';
            jwin_button_proc(MSG_DRAW,tp,0);
          }
          ++tp;
        }
        d->d1 = v;
        ((char*)(d->dp))[0] = v+'0';
      }
    }
    d->flags = 0;
    jwin_button_proc(MSG_DRAW,d,0);
    unscare_mouse();
  }
  return D_O_K;
}

static DIALOG tp_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     56,   32,   208,  160,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Triforce Pieces" },
  { d_dummy_proc,      160,  40,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_tri_frame_proc,  64,   56,   192,    96,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL },
  // 3
  { d_tri_edit_proc,   138,  82,   17,   17,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1" },
  { d_tri_edit_proc,   166,  82,   17,   17,   vc(14),  vc(1),  0,       0,          0,             1,       (void *) "2" },
  { d_tri_edit_proc,   90,   130,  17,   17,   vc(14),  vc(1),  0,       0,          0,             2,       (void *) "3" },
  { d_tri_edit_proc,   214,  130,  17,   17,   vc(14),  vc(1),  0,       0,          0,             3,       (void *) "4" },
  // 7
  { d_tri_edit_proc,   138,  110,  17,   17,   vc(14),  vc(1),  0,       0,          0,             4,       (void *) "5" },
  { d_tri_edit_proc,   118,  130,  17,   17,   vc(14),  vc(1),  0,       0,          0,             5,       (void *) "6" },
  { d_tri_edit_proc,   166,  110,  17,   17,   vc(14),  vc(1),  0,       0,          0,             6,       (void *) "7" },
  { d_tri_edit_proc,   186,  130,  17,   17,   vc(14),  vc(1),  0,       0,          0,             7,       (void *) "8" },
  // 11
  { jwin_button_proc,     90,   166,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  166,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int onTriPieces()
{
  tp_dlg[0].dp2=lfont;
  char temptext[8][2];
  for(int i=0; i<8; i++)
  {
    tp_dlg[i+3].d1 = misc.triforce[i];
    //    ((char*)(tp_dlg[i+3].dp))[0] = misc.triforce[i]+'0';
    sprintf(temptext[i], "%d", misc.triforce[i]);
    tp_dlg[i+3].dp=temptext[i];
  }

  if(zc_popup_dialog(tp_dlg,-1) == 11)
  {
    saved=false;
    for(int i=0; i<8; i++)
      misc.triforce[i] = tp_dlg[i+3].d1;
  }
  return D_O_K;
}


/**********************************/
/***********  onDMaps  ************/
/**********************************/

int d_maptile_proc(int msg,DIALOG *d,int c);
bool small_dmap=false;

static DIALOG dmapmaps_dlg[] =
{

  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 4,    18,   313,  217,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Custom DMap Map Styles" },
  { d_dummy_proc,      160,  22,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_button_proc,     93,   208,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     168,  208,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_ctext_proc,      160,  38,    0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Minimaps" },
  { d_ctext_proc,      112,  46,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Before Map" },
  { d_ctext_proc,      208,  46,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "After Map" },

  { d_ctext_proc,      162,  110,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Large Maps" },
  { d_ctext_proc,      80,   118,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Before Map" },
  { d_ctext_proc,      240,  118,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "After Map" },
  // 5
  { d_maptile_proc,    72,   54,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
  { d_maptile_proc,    168,  54,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
  { d_maptile_proc,    8,    126,  144,  80,   0,       0,      0,       0,          0,             0,       NULL },
  { d_maptile_proc,    168,  126,  144,  80,   0,       0,      0,       0,          0,             0,       NULL },
  // 11
  { NULL }
};

int d_hexedit_proc(int msg,DIALOG *d,int c)
{
  return jwin_hexedit_proc(msg,d,c);
}

int xtoi(char *hexstr)
{
  int val=0;
  while(isxdigit(*hexstr))
  {
    val<<=4;
    if(*hexstr<='9')
      val += *hexstr-'0';
    else val+= ((*hexstr)|0x20)-'a'+10;
    ++hexstr;
  }
  return val;
}

void drawgrid(BITMAP *dest,int x,int y,int grid,int fg,int bg,int div)
{
  rectfill(dest,x-1,y-1,x+63,y+3,div);
  for(int dx=0; dx<64; dx+=8)
  {
    if(grid&0x80)
      rectfill(dest,x+dx,y,x+dx+6,y+2,fg);
    else
      rectfill(dest,x+dx,y,x+dx+6,y+2,bg);
    grid<<=1;
  }
}

void drawovergrid(BITMAP *dest,int x,int y,int grid,int color,int div)
{
  rectfill(dest,x-1,y-1,x+63,y+3,div);
  for(int dx=0; dx<64; dx+=4)
  {
    rectfill(dest,x+dx,y,x+dx+2,y+2,color);
    grid<<=1;
  }
}

void drawgrid(BITMAP *dest,int x,int y,int w, int h, int tw, int th, int *grid,int fg,int bg,int div)
{
  rectfill(dest,x,y,x+(8*8),y+(1*4),div);
  for (int dy=0; dy<h; dy++)
  {
    for(int dx=0; dx<64; dx+=8)
    {
      if(grid[0]&0x80)
        rectfill(dest,x+dx,y,x+dx+6,y+2,fg);
      else
        rectfill(dest,x+dx,y,x+dx+6,y+2,bg);
      grid[0]<<=1;
    }
  }
}

void drawgrid_s(BITMAP *dest,int x,int y,int grid,int fg,int bg,int div)
{
  rectfill(dest,x-1,y-1,x+63,y+3,div);
  for(int dx=0; dx<64; dx+=8)
  {
    rectfill(dest,x+dx,y,x+dx+6,y+2,bg);
    if(grid&0x80)
      rectfill(dest,x+dx+2,y,x+dx+4,y+2,fg);
    grid<<=1;
  }
}

void drawdmap(int dmap)
{
  int c;

  switch((DMaps[dmap].type&dmfTYPE))
  {
    case dmDNGN:
    for(int y=1; y<33; y+=4)
      drawgrid(dmapbmp_small,1,y,DMaps[dmap].grid[y>>2],vc(1),vc(0),vc(0));
    c=DMaps[dmap].compass;
    //  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,dvc(2*4));
    rectfill(dmapbmp_small,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(4));
    c=DMaps[dmap].cont;
    rectfill(dmapbmp_small,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
    break;

    case dmOVERW:
    //      clear_to_color(dmapbmp_small,vc(7));
    for(int y=1; y<33; y+=4)
      drawovergrid(dmapbmp_small,1,y,DMaps[dmap].grid[y>>2],vc(7),vc(0));
    c=DMaps[dmap].cont;
    rectfill(dmapbmp_small,(c&15)*4+1,(c>>4)*4+1,(c&15)*4+3,(c>>4)*4+3,vc(10));
    break;

    case dmCAVE:
    for(int y=1; y<33; y+=4)
      drawgrid(dmapbmp_small,1,y,DMaps[dmap].grid[y>>2],vc(8),vc(0),vc(0));
    c=DMaps[dmap].cont;
    rectfill(dmapbmp_small,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
    break;

    case dmBSOVERW:
    for(int y=1; y<33; y+=4)
      //    drawgrid_s(dmapbmp,1,y,DMaps[dmap].grid[y>>2],dvc(2*4),dvc(2*3),dvc(3+4));
      drawgrid_s(dmapbmp_small,1,y,DMaps[dmap].grid[y>>2],vc(4),vc(6),vc(14));
    c=DMaps[dmap].cont;
    rectfill(dmapbmp_small,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
    break;
  }
}

int d_dmaplist_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    drawdmap(d->d1);
    int *xy = (int*)(d->dp3);
    if(xy[0]>-1000&&xy[1]>-1000)
    {
      jwin_draw_frame(screen,d->x+xy[0]-2,d->y+xy[1]-2,69,37,FR_DEEP);
      blit(dmapbmp_small,screen,0,0,d->x+xy[0],d->y+xy[1],65,33);
    }
    if(xy[2]>-1000&&xy[3]>-1000)
    {
      textprintf_ex(screen,font,d->x+xy[2],d->y+xy[3],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Map: %-3d",DMaps[d->d1].map+1);
    }
    if(xy[4]>-1000&&xy[5]>-1000)
    {
      textprintf_ex(screen,font,d->x+xy[4],d->y+xy[5],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Level: %-3d",DMaps[d->d1].level);
    }
  }
  return jwin_list_proc(msg,d,c);
}

int d_dropdmaplist_proc(int msg,DIALOG *d,int c)
{
  int d1 = d->d1;
  int ret = jwin_droplist_proc(msg,d,c);
  if(msg==MSG_DRAW || d->d1!=d1)
  {
    scare_mouse();
    drawdmap(d->d1);
    int *xy = (int*)(d->dp3);
    if(xy[0]>-1000&&xy[1]>-1000)
    {
      jwin_draw_frame(screen,d->x+xy[0]-2,d->y+xy[1]-2,69,37,FR_DEEP);
      blit(dmapbmp_small,screen,0,0,d->x+xy[0],d->y+xy[1],65,33);
    }

    if(xy[2]>-1000&&xy[3]>-1000)
    {
      //      textprintf_ex(screen,pfont,d->x+xy[2],d->y+xy[3],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Map: %d",DMaps[d->d1].map+1);
      textprintf_right_ex(screen,spfont,d->x+xy[2]+29,d->y+xy[3],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"   %d",DMaps[d->d1].map+1);
      textprintf_ex(screen,spfont,d->x+xy[2],d->y+xy[3],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Map:");
    }
    if(xy[4]>-1000&&xy[5]>-1000)
    {
      //      textprintf_ex(screen,pfont,d->x+xy[4],d->y+xy[5],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Level:%2d",DMaps[d->d1].level);
      textprintf_right_ex(screen,spfont,d->x+xy[4]+34,d->y+xy[5],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"   %d",DMaps[d->d1].level);
      textprintf_ex(screen,spfont,d->x+xy[4],d->y+xy[5],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Level:");
    }
    unscare_mouse();
  }
  return ret;
}

int d_dropdmaptypelist_proc(int msg,DIALOG *d,int c)
{
  int d1 = d->d1;
  int ret = jwin_droplist_proc(msg,d,c);
  if(msg==MSG_DRAW || d->d1!=d1)
  {
    scare_mouse();
    small_dmap=(d->d1!=dmOVERW);
    object_message(d-3, MSG_DRAW, 0);
    (d-2)->flags&=~D_DISABLED;
    (d-2)->flags|=small_dmap?0:D_DISABLED;
    object_message(d-2, MSG_DRAW, 0);
    (d+35)->d1=small_dmap;
    object_message(d+35, MSG_DRAW, 0);
    unscare_mouse();
  }
  return ret;
}

int d_grid_proc(int msg,DIALOG *d,int c)
{
  //d->d1=1;
  int frame_thickness=2;
  int button_thickness=2;
  int header_width=4;
  int header_height=6;
  int cols=d->d1?8:16;
  int col_width=d->d1?14:7;
  switch(msg)
  {
    case MSG_DRAW:
    {
      int x=d->x;
      int y=d->y;
      int j=0, k=0;
      rectfill(screen,x,y,x+d->w-1,y+header_height-1,jwin_pal[jcBOX]);
      for (j=0; j<8; ++j)
      {
        textprintf_ex(screen,spfont,x,y+header_height+frame_thickness+1+(j*7),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",j);
      }
      for (j=0; j<cols; ++j)
      {
        textprintf_ex(screen,spfont,x+header_width+frame_thickness+((col_width+1)/2)-(header_width/2)+(j*col_width),y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%X",j);
      }
      jwin_draw_frame(screen, x+header_width, y+header_height, 116, 60, FR_DEEP);
      for (j=0; j<8; ++j)
      {
        for (k=0; k<cols; ++k)
        {
          jwin_draw_frame(screen, x+header_width+(k*col_width)+frame_thickness, y+header_height+(j*7)+frame_thickness, col_width, 7, get_bit((byte *)d->dp,8*j+k)||!d->d1?FR_MEDDARK:FR_BOX);
          rectfill(screen, x+header_width+(k*col_width)+frame_thickness+button_thickness, y+header_height+(j*7)+frame_thickness+button_thickness, x+header_width+(k*col_width)+frame_thickness+col_width-button_thickness-1, y+header_height+(j*7)+frame_thickness+7-button_thickness-1, get_bit((byte *)d->dp,8*j+k)&&d->d1?jwin_pal[jcBOXFG]:jwin_pal[jcBOX]);
        }
      }
    }
    break;
    case MSG_CLICK:
    {
      int x=(gui_mouse_x()-(d->x)-frame_thickness-header_width)/col_width;
      int y=(gui_mouse_y()-(d->y)-frame_thickness-header_height)/7;
      if(y>=0 && y<8 && x>=0 && x<cols)
      {
        if (key[KEY_ALT]||key[KEY_ALTGR])
        {
          sprintf((char*)((d+2)->dp),"%d%X",y,x);
          object_message((d+2), MSG_DRAW, 0);
        }
        if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
        {
          sprintf((char*)((d+4)->dp),"%d%X",y,x);
          object_message((d+4), MSG_DRAW, 0);
        }
        if (!(key[KEY_ALT]||key[KEY_ALTGR]||key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL]))
        {
          set_bit((byte *)d->dp,8*y+x,!get_bit((byte *)d->dp,8*y+x));
        }
      }
      scare_mouse();
      object_message(d, MSG_DRAW, 0);
      unscare_mouse();
      while(gui_mouse_b());
    }
    break;
  }
  return D_O_K;
}

void drawxmap(int map,int xoff,bool large)
{
  //  clear_to_color(dmapbmp_large,CSET(0)+2);
  int cols=large?8:16;
  int col_width=large?14:7;
  int dot_width=large?4:3;
  int dot_offset=large?5:2;
  clear_to_color(dmapbmp_large,jwin_pal[jcBOX]);
  for(int y=0; y<8; y++)
  {
    for(int x=0; x<cols; x++)
    {
      if(!large||(x+xoff>=0 && x+xoff<=15))
      {
        mapscr *scr = TheMaps + map*MAPSCRS + y*16+x+(large?xoff:0);
        rectfill(dmapbmp_large,x*col_width,y*7,x*col_width+col_width-1,y*7+6,scr->valid&mVALID ? lc1((scr->color)&15) : 0);
        if(scr->valid&mVALID && ((scr->color)&15)>0)
        {
          rectfill(dmapbmp_large,x*col_width+dot_offset,y*7+2,x*col_width+dot_offset+dot_width-1,y*7+2+2,lc2((scr->color)&15));
        }
      }
    }
  }
}

int d_xmaplist_proc(int msg,DIALOG *d,int c)
{
  int d1 = d->d1;
  int ret = jwin_droplist_proc(msg,d,c);
  if(msg==MSG_DRAW || d->d1!=d1)
  {
    scare_mouse();
    int *xy = (int*)(d->dp3);
    xy[0]=d->d1;
    drawxmap(xy[0],xy[1],small_dmap);
    if(xy[2]||xy[3])
    {
      int frame_thickness=2;
      int header_width=4;
      int header_height=6;
      int cols=small_dmap?8:16;
      int col_width=small_dmap?14:7;
      int x=d->x+xy[2];
      int y=d->y+xy[3];
      int j=0;

      rectfill(screen,x,y-header_height-frame_thickness,x+116-1,y-1,jwin_pal[jcBOX]);
      for (j=0; j<8; ++j)
      {
        textprintf_ex(screen,spfont,x-header_width-frame_thickness,y+1+(j*7),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",j);
      }
      for (j=0; j<cols; ++j)
      {
        textprintf_ex(screen,spfont,x+((col_width+1)/2)-(header_width/2)+(j*col_width),y-header_height-frame_thickness,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%X",j);
      }

      jwin_draw_frame(screen, x-frame_thickness, y-frame_thickness, 116, 60, FR_DEEP);
      blit(dmapbmp_large,screen,0,0,x,y,112,56);
    }
    //slider is disabled if
    (d+1)->flags&=~D_DISABLED;
    (d+1)->flags|=small_dmap?0:D_DISABLED;
    object_message(d+1, MSG_DRAW, 0);
    unscare_mouse();
  }
  return ret;
}

int xmapspecs[4] = {0,0,2,20+6};

int onXslider(void *dp3,int d2)
{
  int *x=(int *)dp3;
  int *y=x+1;
  xmapspecs[1]=d2-7;
  bound(xmapspecs[1],-7,15);
  drawxmap(xmapspecs[0],xmapspecs[1],small_dmap);
  scare_mouse();
  blit(dmapbmp_large,screen,0,0,(*x)+xmapspecs[2],(*y)+xmapspecs[3],112,56);
  unscare_mouse();
  return D_O_K;
}

char *dmaptype_str[dmMAX] = { "Dungeon","Overworld","Cave","BS-Overworld" };

char *typelist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,dmMAX-1);
    return dmaptype_str[index];
  }
  *list_size=dmMAX;
  return NULL;
}

void put_title_str(char *s,int x,int y,int fg,int bg,int pos,int lines,int cpl)
{
  int i=0;
  // text_mode(bg);
  for(int dy=0; dy<lines; dy++)
    for(int dx=0; dx<cpl; dx++)
    {
      if(edit_ins_mode)
      {
        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),fg,bg,"%c",*(s+i));
      }
      else
      {
        //     text_mode(i==pos?vc(15):bg);

        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,i==pos?vc(15):bg,"%c",*(s+i));
      }

      ++i;
    }
  if(edit_ins_mode&&pos>-1)
  {
    //   text_mode(-1);
    textprintf_ex(screen,zfont,x+((pos%cpl)<<3),y+((pos/cpl)<<3),vc(15),-1,"_");
  }
}

int d_title_edit_proc(int msg,DIALOG *d,int c)
{
  char *s=(char*)(d->dp);
  switch(msg)
  {
    case MSG_WANTFOCUS:
    return D_WANTFOCUS;
    case MSG_CLICK:
    d->d2=((gui_mouse_x()-d->x)>>3)+((gui_mouse_y()-d->y)>>3)*10;
    bound(d->d2,0,19);
    scare_mouse();
    put_title_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2,2,10);
    unscare_mouse();
    while(gui_mouse_b());
    break;
    case MSG_DRAW:
    if (!(d->flags & D_GOTFOCUS))
    {
      d->d2=-1;
    }
    put_title_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2,2,10);
    break;
    case MSG_CHAR:
    bool used=false;
    int k=c>>8;
    switch(k)
    {
      case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
      case KEY_HOME:  d->d2-=d->d2%10; used=true; break;
      case KEY_END:   d->d2-=d->d2%10; d->d2+=9; used=true; break;
      case KEY_UP:    if(d->d2>=10) d->d2-=10; used=true; break;
      case KEY_DOWN:  if(d->d2<10) d->d2+=10; used=true; break;
      case KEY_LEFT:  if(d->d2>0) --d->d2; used=true; break;
      case KEY_RIGHT: if(d->d2<19) ++d->d2; used=true; break;
      case KEY_BACKSPACE:
      if(d->d2>0)
        --d->d2;
      case KEY_DEL:
      strcpy(s+d->d2,s+d->d2+1);
      s[19]=' ';
      s[20]=0;
      used=true;
      break;
      default:
      if(isprint(c&255))
      {
        if(edit_ins_mode)
        {
          for(int i=19; i>d->d2; i--)
            s[i]=s[i-1];
        }
        s[d->d2]=c&255;
        if(d->d2<19)
          ++d->d2;
        used=true;
      }
    }
    scare_mouse();
    put_title_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2,2,10);
    unscare_mouse();
    return used?D_USED_CHAR:D_O_K;
  }
  return D_O_K;
}

void put_intro_str(char *s,int x,int y,int fg,int bg,int pos)
{
  int i=0;
  // text_mode(bg);
  for(int dy=0; dy<3; dy++)
    for(int dx=0; dx<24; dx++)
    {
      if(edit_ins_mode)
      {
        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),fg,bg,"%c",*(s+i));
      }
      else
      {
        //     text_mode(i==pos?vc(15):bg);
        textprintf_ex(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,i==pos?vc(15):bg,"%c",*(s+i));
      }
      ++i;
    }
  if(edit_ins_mode&&pos>-1)
  {
    //   text_mode(-1);
    textprintf_ex(screen,zfont,x+((pos%24)<<3),y+((pos/24)<<3),vc(15),-1,"_");
  }
}

int d_intro_edit_proc(int msg,DIALOG *d,int c)
{
  char *s=(char*)(d->dp);
  switch(msg)
  {
    case MSG_WANTFOCUS:
    return D_WANTFOCUS;
    case MSG_CLICK:
    d->d2=((gui_mouse_x()-d->x)>>3)+((gui_mouse_y()-d->y)>>3)*24;
    bound(d->d2,0,71);
    scare_mouse();
    put_intro_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    unscare_mouse();
    while(gui_mouse_b());
    break;
    case MSG_DRAW:

    if (!(d->flags & D_GOTFOCUS))
    {
      d->d2=-1;

    }
    put_intro_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    break;
    case MSG_CHAR:
    bool used=false;
    int k=c>>8;
    switch(k)
    {
      case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
      case KEY_HOME:  d->d2-=d->d2%24; used=true; break;
      case KEY_END:   d->d2-=d->d2%24; d->d2+=23; used=true; break;
      case KEY_UP:    if(d->d2>=24) d->d2-=24; used=true; break;
      case KEY_DOWN:  if(d->d2<48) d->d2+=24; used=true; break;
      case KEY_LEFT:  if(d->d2>0) --d->d2; used=true; break;
      case KEY_RIGHT: if(d->d2<71) ++d->d2; used=true; break;
      case KEY_BACKSPACE:
      if(d->d2>0)
        --d->d2;
      case KEY_DEL:
      strcpy(s+d->d2,s+d->d2+1);
      s[71]=' ';
      s[72]=0;
      used=true;
      break;
      default:
      if(isprint(c&255))
      {
        if(edit_ins_mode)
        {
          for(int i=71; i>d->d2; i--)
            s[i]=s[i-1];
        }
        s[d->d2]=c&255;
        if(d->d2<71)
          ++d->d2;
        used=true;
      }
    }
    scare_mouse();
    put_intro_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2);
    unscare_mouse();
    return used?D_USED_CHAR:D_O_K;
  }
  return D_O_K;
}

char dmap_title[21];
char dmap_name[33];
char dmap_intro[73];


static int editdmap_mechanics_list[] =
{
  // dialog control number
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1
};


static int editdmap_appearance_list[] =
{
  // dialog control number
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, -1
};

static int editdmap_music_list[] =
{
  // dialog control number
  82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, -1
};

static int editdmap_subscreenmaps_list[] =
{
  // dialog control number
  6, -1
};

static TABPANEL editdmap_tabs[] =
{
  // (text)
  { "Mechanics",      D_SELECTED, editdmap_mechanics_list },
  { "Appearance",     0,          editdmap_appearance_list },
  { "Music",          0,          editdmap_music_list },
  { "Subscreen Maps", 0,          editdmap_subscreenmaps_list },
  { NULL }
};

static int editdmapmap_before_list[] =
{
  // dialog control number
  7, 8, 9, 10, 11, 12, -1
};

static int editdmapmap_after_list[] =
{
  // dialog control number
  13, 14, 15, 16, 17, 18, -1
};

static TABPANEL editdmapmap_tabs[] =
{
  // (text)
  { "Before Map",  D_SELECTED,  editdmapmap_before_list },
  { "After Map",   0,           editdmapmap_after_list },
  { NULL }
};

int dmap_tracks=0;
static char dmap_track_number_str_buf[32];
char *dmaptracknumlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,255);
    sprintf(dmap_track_number_str_buf,"%02d",index+1);
    return dmap_track_number_str_buf;
  }
  *list_size=dmap_tracks;
  return NULL;
}

extern char *subscreenlist_a(int index, int *list_size);
extern char *subscreenlist_b(int index, int *list_size);


static DIALOG editdmap_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,        4,    15,   312,  221,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_EXIT,          0,             0,       (void *) "DMap Editor" },
  { jwin_button_proc,     93,   211,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     168,  211,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_text_proc,       14,   44,  48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Name: " },
  { jwin_edit_proc,       44,   40,  168,  16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          20,            0,       NULL },
  //5
  { jwin_tab_proc,        10,   60,  300,  144,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],      0,      0,          0,             0,       (void *) editdmap_tabs, NULL, (void *)editdmap_dlg },
  { jwin_tab_proc,        14,   80,  292,  116,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],      0,      0,          0,             0,       (void *) editdmapmap_tabs, NULL, (void *)editdmap_dlg },
  { jwin_ctext_proc,      77,   102, 0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Small" },
  { jwin_frame_proc,      35,   110,  84,  52,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { d_maptile_proc,       37,   112,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
  //10
  { jwin_ctext_proc,      211,   102, 0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Large" },
  { jwin_frame_proc,      137,   110,  148,  84,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { d_maptile_proc,       139,    112,  144,  80,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_ctext_proc,      77,   102, 0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Small" },
  { jwin_frame_proc,      35,   110,  84,  52,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  //15
  { d_maptile_proc,       37,   112,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_ctext_proc,      211,   102, 0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Large" },
  { jwin_frame_proc,      137,   110,  148,  84,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { d_maptile_proc,       139,    112,  144,  80,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_text_proc,       16,   84,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Map:" },
  //20
  { d_xmaplist_proc,      40,   80,   54,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) gotomaplist, NULL, xmapspecs},
  { jwin_slider_proc,     42,   166,  111,   10,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          22,            0,       NULL, (void *) onXslider },
  { jwin_text_proc,       107,  84,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type: " },
  { d_dropdmaptypelist_proc,   136,  80,   99,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) typelist },
  { jwin_text_proc,       247,   84,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Level:" },
  //25
  { jwin_edit_proc,       278,   80,   26,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          3,             0,       NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //30
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //35
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //40
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //45
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //50
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //55
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //  { d_grid_proc,          164,  104,  116,  60,   0,       0,      0,       0,          0,             0,       NULL },
  { d_grid_proc,          166,   98,  124,  66,   0,       0,      0,       0,          0,             0,       NULL },
  { jwin_text_proc,       166,  170,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Compass: 0x" },
  //60
  { jwin_edit_proc,       222,   166,   21,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       NULL },
  { jwin_text_proc,       166,  188,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Continue: 0x" },
  { jwin_edit_proc,       222,   184,   21,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       NULL },
  { jwin_check_proc,      80,   188,   113,  9,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       (void *) "Continue here" },
  { jwin_text_proc,       16,   84,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Color:" },
  //65
  { jwin_droplist_proc,   46,   80,   161,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) levelnumlist },
  { jwin_ctext_proc,      59,   100,   0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "DMap Title" },
  { jwin_frame_proc,      17,   108,  84,   20,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { d_title_edit_proc,    19,   110,  80,   16,   jwin_pal[jcTEXTBG],  jwin_pal[jcTEXTFG],  0,       0,          0,             0,       (void *) dmap_title },
  { jwin_ctext_proc,      205,  100,   0,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "DMap Intro" },
  //70
  { jwin_frame_proc,      107,  108,  196,  28,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { d_intro_edit_proc,    109,  110,  192,  24,   jwin_pal[jcTEXTBG],  jwin_pal[jcTEXTFG],  0,       0,          0,             0,       (void *) dmap_intro },
  { jwin_frame_proc,      16,   142,   223,  44,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  { jwin_text_proc,       24,   139,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) " Subscreens " },
  { jwin_text_proc,       20,   152,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Active:" },
  //75
  { jwin_droplist_proc,   61,   148,   174,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) subscreenlist_a },
  { jwin_text_proc,       20,   170,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Passive:" },
  { jwin_droplist_proc,   61,   166,   174,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) subscreenlist_b },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //80
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { jwin_text_proc,       16,   84,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Midi:" },
  { jwin_droplist_proc,   39,   80,   153,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) midilist },
  { jwin_frame_proc,      16,   101,   176,  68,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  //85
  { jwin_text_proc,       24,   98,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) " Alternate Music " },
  { jwin_frame_proc,      20,   107,  168,  16,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL },
  { jwin_text_proc,       23,   111,  162,  16,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       20,   129,   48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Track:" },
  { jwin_droplist_proc,   54,   125,   134,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) dmaptracknumlist },
  //90
  { jwin_button_proc,     35,   144,  61,  21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  13,      D_EXIT,     0,             0,       (void *) "Load" },
  { jwin_button_proc,     112,  144,  61,  21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  13,      D_EXIT,     0,             0,       (void *) "Clear" },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  //95
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { d_dummy_proc,          0,    0,    0,    0,   0,          0,   0,       0,          0,             0,        NULL },
  { NULL }
};

void editdmap(int index)
{
  char levelstr[4], compassstr[4], contstr[4], tmusicstr[56], dmapnumstr[60];
  char *tmfname;
  byte gridstring[8];
  static int xy[2];
  sprintf(levelstr,"%d",DMaps[index].level);
  sprintf(dmapnumstr,"Edit DMap (%d)",index);
  sprintf(compassstr,"%02X",DMaps[index].compass);
  sprintf(contstr,"%02X",DMaps[index].cont);
  sprintf(dmap_title,"%s",DMaps[index].title);
  sprintf(dmap_name,"%s",DMaps[index].name);
  sprintf(dmap_intro,"%s",DMaps[index].intro);
  sprintf(tmusicstr,"%s",DMaps[index].tmusic);
/*
  if (zinit.subscreen<ssdtBSZELDA)
  {
    editdmap_dlg[11].x=editdmap_dlg[17].x=editdmap_dlg[0].x+133;
    editdmap_dlg[11].w=editdmap_dlg[17].w=148;
  }
  else
  {
    editdmap_dlg[11].x=editdmap_dlg[17].x=editdmap_dlg[0].x+149;
    editdmap_dlg[11].w=editdmap_dlg[17].w=116;
  }
  editdmap_dlg[12].x=editdmap_dlg[18].x=editdmap_dlg[11].x+2;
  editdmap_dlg[12].w=editdmap_dlg[18].w=editdmap_dlg[11].w-4;
*/
  editdmap_dlg[0].dp=dmapnumstr;
  editdmap_dlg[0].dp2=lfont;
  editdmap_dlg[4].dp=dmap_name;
  editdmap_dlg[9].d1 = DMaps[index].minimap_1_tile;
  editdmap_dlg[9].fg = DMaps[index].minimap_1_cset;
  editdmap_dlg[12].d1 = DMaps[index].largemap_1_tile;
  editdmap_dlg[12].fg = DMaps[index].largemap_1_cset;
  editdmap_dlg[15].d1 = DMaps[index].minimap_2_tile;
  editdmap_dlg[15].fg = DMaps[index].minimap_2_cset;
  editdmap_dlg[18].d1 = DMaps[index].largemap_2_tile;
  editdmap_dlg[18].fg = DMaps[index].largemap_2_cset;
  editdmap_dlg[20].d1=(DMaps[index].map>(map_count-1))?0:DMaps[index].map;
  xy[0]=editdmap_dlg[20].x;
  xy[1]=editdmap_dlg[20].y;
  editdmap_dlg[21].dp3=xy;
  xmapspecs[1]=DMaps[index].xoff;
  editdmap_dlg[21].d2=DMaps[index].xoff+7;
  editdmap_dlg[23].d1=(DMaps[index].type&dmfTYPE);
  editdmap_dlg[25].dp=levelstr;
  for(int i=0; i<32; i++)
  {
    editdmap_dlg[26+i].dp2=spfont;
  }
  for(int i=0; i<8; i++)
  {
    for(int j=0; j<8; j++)
    {
      set_bit(gridstring,8*i+j,get_bit((byte *)(DMaps[index].grid+i),7-j));
    }
  }
  editdmap_dlg[58].dp=gridstring;
  editdmap_dlg[60].dp=compassstr;
  editdmap_dlg[62].dp=contstr;
  editdmap_dlg[63].flags = (DMaps[index].type&dmfCONTINUE) ? D_SELECTED : 0;
  editdmap_dlg[65].d1=DMaps[index].color;
  editdmap_dlg[75].d1=DMaps[index].active_subscreen;
  editdmap_dlg[77].d1=DMaps[index].passive_subscreen;
  editdmap_dlg[83].d1=DMaps[index].midi;
  editdmap_dlg[87].dp=tmusicstr;
  dmap_tracks=0;
  ZCMUSIC *tempdmapzcmusic = (ZCMUSIC*)zcmusic_load_file(tmusicstr);
  if (tempdmapzcmusic!=NULL)
  {
    dmap_tracks=zcmusic_get_tracks(tempdmapzcmusic);
    dmap_tracks=(dmap_tracks<2)?0:dmap_tracks;
  }
  zcmusic_unload_file(tempdmapzcmusic);
  editdmap_dlg[89].flags=(dmap_tracks<2)?D_DISABLED:0;
  editdmap_dlg[89].d1=vbound(DMaps[index].tmusictrack,0,dmap_tracks-1);

  int ret=-1;
  while (ret!=0&&ret!=1&&ret!=2)
  {
    ret=zc_popup_dialog(editdmap_dlg,-1);
    switch (ret)
    {
      case 90:                                              //grab a filename for tracker music
      {
        if (getname("Load DMAP Music",(char*)zcmusic_types,tmusicpath,false))
        {
          strcpy(tmusicpath,temppath);
          tmfname=get_filename(tmusicpath);
          if (strlen(tmfname)>55)
          {
            jwin_alert("Error","Filename too long","(>55 characters",NULL,"O&K",NULL,'k',0,lfont);
            temppath[0]=0;
          }
          else
          {
            sprintf(tmusicstr,"%s",tmfname);
            editdmap_dlg[87].dp=tmusicstr;
            dmap_tracks=0;
            ZCMUSIC *tempdmapzcmusic = (ZCMUSIC*)zcmusic_load_file(tmusicstr);
            if (tempdmapzcmusic!=NULL)
            {
              dmap_tracks=zcmusic_get_tracks(tempdmapzcmusic);
              dmap_tracks=(dmap_tracks<2)?0:dmap_tracks;
            }
            zcmusic_unload_file(tempdmapzcmusic);
            editdmap_dlg[89].flags=(dmap_tracks<2)?D_DISABLED:0;
            editdmap_dlg[89].d1=0;
          }
        }
        //shnarf
      }
      break;
      case 91:                                              //clear tracker music
        memset(tmusicstr, 0, 56);
        editdmap_dlg[89].flags=D_DISABLED;
        editdmap_dlg[89].d1=0;
        break;
    }
  }
  if(ret==1)
  {
    saved=false;
    sprintf(DMaps[index].name,"%s",dmap_name);
    DMaps[index].minimap_1_tile = editdmap_dlg[9].d1;
    DMaps[index].minimap_1_cset = editdmap_dlg[9].fg;
    DMaps[index].largemap_1_tile = editdmap_dlg[12].d1;
    DMaps[index].largemap_1_cset = editdmap_dlg[12].fg;
    DMaps[index].minimap_2_tile = editdmap_dlg[15].d1;
    DMaps[index].minimap_2_cset = editdmap_dlg[15].fg;
    DMaps[index].largemap_2_tile = editdmap_dlg[18].d1;
    DMaps[index].largemap_2_cset = editdmap_dlg[18].fg;
    DMaps[index].map = (editdmap_dlg[20].d1>(map_count-1))?0:editdmap_dlg[20].d1;
    DMaps[index].xoff = xmapspecs[1];
    DMaps[index].type=editdmap_dlg[23].d1|((editdmap_dlg[63].flags & D_SELECTED)?dmfCONTINUE:0);
    DMaps[index].level=vbound(atoi(levelstr),0,MAXLEVELS-1);
    for(int i=0; i<8; i++)
    {
      for(int j=0; j<8; j++)
      {
        set_bit((byte *)(DMaps[index].grid+i),7-j,get_bit(gridstring,8*i+j));
      }
    }

    DMaps[index].compass = xtoi(compassstr);
    DMaps[index].cont = xtoi(contstr);
    DMaps[index].color = editdmap_dlg[65].d1;
    DMaps[index].active_subscreen=editdmap_dlg[75].d1;
    DMaps[index].passive_subscreen=editdmap_dlg[77].d1;
    DMaps[index].midi = editdmap_dlg[83].d1;
    sprintf(DMaps[index].tmusic, "%s", tmusicstr);
    sprintf(DMaps[index].title,"%s",dmap_title);
    sprintf(DMaps[index].intro,"%s",dmap_intro);
    DMaps[index].tmusictrack = editdmap_dlg[89].d1;
  }
}

//int selectdmapxy[6] = {90,142,164,150,164,160};
int selectdmapxy[6] = {90-46,142-50,164-46,150-50,164-46,160-50};

static DIALOG selectdmap_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 72-44,   56-30,   176+88+1,  120+60+1,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select DMap" },
  { d_dummy_proc,      160,  63-30,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_dmaplist_proc,   46,   50,   64+72+88+1,   60+24+1+2,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *) dmaplist, NULL, selectdmapxy },
  { jwin_button_proc,     90,   152+30,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  152+30,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, (void *) close_dlg },
  { NULL }
};

int onDmaps()
{
  int ret;
  char buf[40];
  dmap_list_size=MAXDMAPS;
  number_list_zero=true;
  selectdmap_dlg[0].dp2=lfont;

  ret=zc_popup_dialog(selectdmap_dlg,2);
  while(ret!=4&&ret!=0)
  {
    int d=selectdmap_dlg[2].d1;
    if(ret==5)
    {
      sprintf(buf,"Delete dmap %d?",d);
      if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
      {
        reset_dmap(d);
        saved=false;
      }
    }
    else
    {
      editdmap(d);
    }
    ret=zc_popup_dialog(selectdmap_dlg,2);
  }
  return D_O_K;
}

/*******************************/
/**********  onMidis  **********/
/*******************************/

static DIALOG editmidi_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   20,   273,  189,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "MIDI Specs" },
  { d_dummy_proc,      160,  56,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  // 2
  { jwin_text_proc,       56,   94-16,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "MIDI:" },
  { jwin_text_proc,       104,  94-16,   48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       56,   114,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name:" },
  { jwin_edit_proc,       104,  114-4,  160,  16,   vc(12),  vc(1),  0,       0,          19,            0,       NULL },
  { jwin_text_proc,       56,   124-4+12,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Volume:" },
  { jwin_edit_proc,       120,  124-4+12-4,  32,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 8
  { jwin_check_proc,      176,  124+12-4,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Loop" },
  // 9
  { jwin_button_proc,     50,   72-24,   57,   21,   vc(14),  vc(1),  'l',     D_EXIT,     0,             0,       (void *) "&Load" },
  { jwin_button_proc,     116,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8D" },
  { jwin_button_proc,     156,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B" },
  { jwin_button_proc,     196,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B\x8B" },
  { jwin_button_proc,     236,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B\x8B\x8B" },
  // 14
  { jwin_text_proc,       56,   134+4+12,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Start:" },
  { jwin_edit_proc,       112,  134+12,  32,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,       176,  134+12+4,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Loop Start:" },
  { jwin_edit_proc,       240,  134+12,  40,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,       176,  144+12+12,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Loop End:" },
  { jwin_edit_proc,       240,  144+12+12-4,  40,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  // 20
  { jwin_text_proc,       176,  94-16,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Position:" },
  { jwin_text_proc,       217,  94-16,   32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       176,  104-8,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Length:" },
  { jwin_text_proc,       216,  104-8,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       56,   104-8,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Time:" },
  { jwin_text_proc,       104,  104-8,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  // 26
  { jwin_button_proc,     90,   160+12+12,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  160+12+12,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};


void edit_midi(int i)
{
  int ret,loop,volume;
  long start,loop_start,loop_end;

  char title[20];
  char volume_str[8];
  char start_str[16];
  char loop_start_str[16];
  char loop_end_str[16];
  char len_str[16];
  char pos_str[16];


  MIDI *midi = customMIDIs[i].midi;
  get_midi_info(midi,&Midi_Info);

  volume = customMIDIs[i].volume;
  loop = customMIDIs[i].loop;
  start = customMIDIs[i].start;
  loop_start = customMIDIs[i].loop_start;
  loop_end = customMIDIs[i].loop_end;

  strcpy(title,customMIDIs[i].title);

  editmidi_dlg[0].dp2=lfont;
  do
  {
    sprintf(volume_str,"%d",volume);
    sprintf(start_str,"%ld",start);
    sprintf(loop_start_str,"%ld",loop_start);
    sprintf(loop_end_str,"%ld",loop_end);
    sprintf(len_str,"%d",Midi_Info.len_beats);
    sprintf(pos_str,"%ld",midi_pos);

    editmidi_dlg[3].dp = midi?(void *) "Loaded":(void *) "Empty";
    editmidi_dlg[5].dp = title;
    editmidi_dlg[7].dp = volume_str;
    editmidi_dlg[8].flags = loop?D_SELECTED:0;
    editmidi_dlg[10].flags =
      editmidi_dlg[11].flags =
      editmidi_dlg[12].flags =
      editmidi_dlg[13].flags = (midi==NULL)?D_DISABLED:D_EXIT;
    editmidi_dlg[15].dp = start_str;
    editmidi_dlg[17].dp = loop_start_str;
    editmidi_dlg[19].dp = loop_end_str;
    editmidi_dlg[21].dp = pos_str;
    editmidi_dlg[23].dp = len_str;
    editmidi_dlg[25].dp = timestr(Midi_Info.len_sec);

    DIALOG_PLAYER *p = init_dialog(editmidi_dlg,-1);
    while(update_dialog(p))
    {
      custom_vsync();
      scare_mouse();
      //      text_mode(vc(1));
      textprintf_ex(screen,font,editmidi_dlg[0].x+193,editmidi_dlg[0].y+58,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",midi_pos);
      unscare_mouse();
    }
    ret = shutdown_dialog(p);

    loop = editmidi_dlg[8].flags?1:0;
    volume = vbound(atoi(volume_str),0,255);

    start = vbound(atol(start_str),0,max(Midi_Info.len_beats-4,0));
    loop_start = vbound(atol(loop_start_str),-1,max(Midi_Info.len_beats-4,-1));
    loop_end = vbound(atol(loop_end_str),-1,Midi_Info.len_beats);
    if(loop_end>0)
    {
      loop_end = vbound(loop_end,max(loop_start+4,start+4),Midi_Info.len_beats);
    }

    switch(ret)
    {
      case 9:
      if(getname("Load MIDI","mid",temppath,true))
      {
        stop_midi();
        if(midi!=NULL && midi!=customMIDIs[i].midi)
        {
          destroy_midi(midi);
        }
        if(!(midi=load_midi(temppath)))
        {
          jwin_alert("Error","Error loading MIDI:",temppath,NULL,"Dang",NULL,13,27,lfont);
        }
        else
        {
          char *t = get_filename(temppath);
          int i=0;
          for( ; i<19 && t[i]!=0 && t[i]!='.'; i++)
          {
            title[i]=t[i];
          }
          title[i]=0;
        }
        get_midi_info(midi,&Midi_Info);
      }
      break;

      case 10:
      stop_midi();
      break;

      case 12:
      if(midi_pos>0)
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
        if(midi_loop_end<0)
        {
          pos = min(pos+16,Midi_Info.len_beats);
        }
        else
        {
          pos = min(pos+16,midi_loop_end);
        }
        if(pos>0)
        {
          midi_seek(pos);
        }
        break;
      }
      // else play it...

      case 13:
      if(midi_pos>0)
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
        if(midi_loop_end<0)
        {
          pos = min(pos+64,Midi_Info.len_beats);
        }

        else
        {
          pos = min(pos+64,midi_loop_end);
        }
        if(pos>0)
        {
          midi_seek(pos);
        }
        break;
      }
      // else play it...

      case 11:
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_seek(pos<0?start:pos);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
      }
      break;
    }
  } while(ret<26&&ret!=0);

  stop_midi();

  if(ret==26)
  {
    strcpy(customMIDIs[i].title,title);
    customMIDIs[i].volume = volume;
    customMIDIs[i].loop = loop;
    customMIDIs[i].start = start;
    customMIDIs[i].loop_start = loop_start;
    customMIDIs[i].loop_end = loop_end;
    if(midi!=customMIDIs[i].midi)
    {
      if(customMIDIs[i].midi)
        destroy_midi(customMIDIs[i].midi);
      customMIDIs[i].midi = midi;
    }
    saved=false;
  }
  if((ret==27||ret==0) && midi!=customMIDIs[i].midi)
  {
    if(midi)
    {
      destroy_midi(midi);
    }
  }
}

int d_midilist_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    int i = d->d1;
    int x = d->x+d->w+8;
    int y = d->y+4;

    textout_right_ex(screen,font,"Volume:",x+51,y+8+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop:",x+51,y+16+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Start:",x+51,y+24+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop Start:",x+51,y+32+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop End:",x+51,y+40+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);

    textprintf_ex(screen,font,x+56,y+8+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-3d",customMIDIs[i].volume);
    textprintf_ex(screen,font,x+56,y+16+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",customMIDIs[i].loop?"On ":"Off");
    textprintf_ex(screen,font,x+56,y+24+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].start);
    textprintf_ex(screen,font,x+56,y+32+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].loop_start);
    textprintf_ex(screen,font,x+56,y+40+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].loop_end);
  }
  return jwin_list_proc(msg,d,c);
}

static DIALOG selectmidi_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   20,   273,  189,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select MIDI" },
  { d_dummy_proc,      160,  56,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_midilist_proc,   31,   44,   164,   (1+16)*8,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *) custommidilist },
  { jwin_button_proc,     90,   160+12+12,  61,   21,   vc(14),  vc(1),  13,     D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  160+12+12,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, (void *) close_dlg },
  { NULL }
};

int onMidis()
{
  stopMusic();
  int ret;
  char buf[40];
  number_list_size=MAXCUSTOMMIDIS;
  number_list_zero=false;
  strcpy(temppath,midipath);
  selectmidi_dlg[0].dp2=lfont;
  go();
  ret=zc_do_dialog(selectmidi_dlg,2);
  while(ret!=4&&ret!=0)
  {
    int d=selectmidi_dlg[2].d1;
    if(ret==5)
    {
      sprintf(buf,"Delete MIDI %d?",d+1);
      if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
      {
        reset_midi(customMIDIs+d);
        saved=false;
      }
    }
    else
    {
      edit_midi(d);
    }
    ret=zc_do_dialog(selectmidi_dlg,2);
  }
  comeback();
  return D_O_K;
}

/*******************************/
/******  onEnhancedMusic  ******/
/*******************************/

static DIALOG editmusic_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   20,   273,  189,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "MIDI Specs" },
  { d_dummy_proc,      160,  56,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  // 2
  { jwin_text_proc,       56,   94-16,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "MIDI:" },
  { jwin_text_proc,       104,  94-16,   48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       56,   114,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name:" },
  { jwin_edit_proc,       104,  114-4,  160,  16,   vc(12),  vc(1),  0,       0,          19,            0,       NULL },
  { jwin_text_proc,       56,   124-4+12,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Volume:" },
  { jwin_edit_proc,       120,  124-4+12-4,  32,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 8
  { jwin_check_proc,      176,  124+12-4,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Loop" },
  // 9
  { jwin_button_proc,     50,   72-24,   57,   21,   vc(14),  vc(1),  'l',     D_EXIT,     0,             0,       (void *) "&Load" },
  { jwin_button_proc,     116,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8D" },
  { jwin_button_proc,     156,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B" },
  { jwin_button_proc,     196,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B\x8B" },
  { jwin_button_proc,     236,  72-24,   33,   21,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "\x8B\x8B\x8B" },
  // 14
  { jwin_text_proc,       56,   134+4+12,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Start:" },
  { jwin_edit_proc,       112,  134+12,  32,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,       176,  134+12+4,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Loop Start:" },
  { jwin_edit_proc,       240,  134+12,  40,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,       176,  144+12+12,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Loop End:" },
  { jwin_edit_proc,       240,  144+12+12-4,  40,   16,   vc(12),  vc(1),  0,       0,          5,             0,       NULL },
  // 20
  { jwin_text_proc,       176,  94-16,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Position:" },
  { jwin_text_proc,       217,  94-16,   32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       176,  104-8,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Length:" },
  { jwin_text_proc,       216,  104-8,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       56,   104-8,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Time:" },
  { jwin_text_proc,       104,  104-8,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  // 26
  { jwin_button_proc,     90,   160+12+12,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  160+12+12,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};


void edit_music(int i)
{
  int ret,loop,volume;
  long start,loop_start,loop_end;

  char title[20];
  char volume_str[8];
  char start_str[16];
  char loop_start_str[16];
  char loop_end_str[16];
  char len_str[16];
  char pos_str[16];


  MIDI *midi = customMIDIs[i].midi;
  get_midi_info(midi,&Midi_Info);

  volume = customMIDIs[i].volume;
  loop = customMIDIs[i].loop;
  start = customMIDIs[i].start;
  loop_start = customMIDIs[i].loop_start;
  loop_end = customMIDIs[i].loop_end;

  strcpy(title,customMIDIs[i].title);

  editmidi_dlg[0].dp2=lfont;
  do
  {
    sprintf(volume_str,"%d",volume);
    sprintf(start_str,"%ld",start);
    sprintf(loop_start_str,"%ld",loop_start);
    sprintf(loop_end_str,"%ld",loop_end);
    sprintf(len_str,"%d",Midi_Info.len_beats);
    sprintf(pos_str,"%ld",midi_pos);

    editmidi_dlg[3].dp = midi?(void *) "Loaded":(void *) "Empty";
    editmidi_dlg[5].dp = title;
    editmidi_dlg[7].dp = volume_str;
    editmidi_dlg[8].flags = loop?D_SELECTED:0;
    editmidi_dlg[10].flags =
      editmidi_dlg[11].flags =
      editmidi_dlg[12].flags =
      editmidi_dlg[13].flags = (midi==NULL)?D_DISABLED:D_EXIT;
    editmidi_dlg[15].dp = start_str;
    editmidi_dlg[17].dp = loop_start_str;
    editmidi_dlg[19].dp = loop_end_str;
    editmidi_dlg[21].dp = pos_str;
    editmidi_dlg[23].dp = len_str;
    editmidi_dlg[25].dp = timestr(Midi_Info.len_sec);

    DIALOG_PLAYER *p = init_dialog(editmidi_dlg,-1);
    while(update_dialog(p))
    {
      custom_vsync();
      scare_mouse();
      //      text_mode(vc(1));
      textprintf_ex(screen,font,editmidi_dlg[0].x+193,editmidi_dlg[0].y+58,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",midi_pos);
      unscare_mouse();
    }
    ret = shutdown_dialog(p);

    loop = editmidi_dlg[8].flags?1:0;
    volume = vbound(atoi(volume_str),0,255);

    start = vbound(atol(start_str),0,max(Midi_Info.len_beats-4,0));
    loop_start = vbound(atol(loop_start_str),-1,max(Midi_Info.len_beats-4,-1));
    loop_end = vbound(atol(loop_end_str),-1,Midi_Info.len_beats);
    if(loop_end>0)
    {
      loop_end = vbound(loop_end,max(loop_start+4,start+4),Midi_Info.len_beats);
    }

    switch(ret)
    {
      case 9:
      if(getname("Load MIDI","mid",temppath,true))
      {
        stop_midi();
        if(midi!=NULL && midi!=customMIDIs[i].midi)
        {
          destroy_midi(midi);
        }
        if(!(midi=load_midi(temppath)))
        {
          jwin_alert("Error","Error loading MIDI:",temppath,NULL,"Dang",NULL,13,27,lfont);
        }
        else
        {
          char *t = get_filename(temppath);
          int i=0;
          for( ; i<19 && t[i]!=0 && t[i]!='.'; i++)
          {
            title[i]=t[i];
          }
          title[i]=0;
        }
        get_midi_info(midi,&Midi_Info);
      }
      break;

      case 10:
      stop_midi();
      break;

      case 12:
      if(midi_pos>0)
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
        if(midi_loop_end<0)
        {
          pos = min(pos+16,Midi_Info.len_beats);
        }
        else
        {
          pos = min(pos+16,midi_loop_end);
        }
        if(pos>0)
        {
          midi_seek(pos);
        }
        break;
      }
      // else play it...

      case 13:
      if(midi_pos>0)
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
        if(midi_loop_end<0)
        {
          pos = min(pos+64,Midi_Info.len_beats);
        }

        else
        {
          pos = min(pos+64,midi_loop_end);
        }
        if(pos>0)
        {
          midi_seek(pos);
        }
        break;
      }
      // else play it...

      case 11:
      {
        int pos=midi_pos;
        stop_midi();
        midi_loop_end = -1;
        midi_loop_start = -1;
        play_midi(midi,loop);
        set_volume(-1,volume);
        midi_seek(pos<0?start:pos);
        midi_loop_end = loop_end;
        midi_loop_start = loop_start;
      }
      break;
    }
  } while(ret<26&&ret!=0);

  stop_midi();

  if(ret==26)
  {
    strcpy(customMIDIs[i].title,title);
    customMIDIs[i].volume = volume;
    customMIDIs[i].loop = loop;
    customMIDIs[i].start = start;
    customMIDIs[i].loop_start = loop_start;
    customMIDIs[i].loop_end = loop_end;
    if(midi!=customMIDIs[i].midi)
    {
      if(customMIDIs[i].midi)
        destroy_midi(customMIDIs[i].midi);
      customMIDIs[i].midi = midi;
    }
    saved=false;
  }
  if((ret==27||ret==0) && midi!=customMIDIs[i].midi)
  {
    if(midi)
    {
      destroy_midi(midi);
    }
  }
}

int d_musiclist_proc(int msg,DIALOG *d,int c)
{
  /*
    if(msg==MSG_DRAW)
    {
    int i = d->d1;
    int x = d->x+d->w+8;
    int y = d->y+4;

    textout_right_ex(screen,font,"Volume:",x+51,y+8+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop:",x+51,y+16+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Start:",x+51,y+24+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop Start:",x+51,y+32+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    textout_right_ex(screen,font,"Loop End:",x+51,y+40+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX]);

    textprintf_ex(screen,font,x+56,y+8+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-3d",customMIDIs[i].volume);
    textprintf_ex(screen,font,x+56,y+16+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",customMIDIs[i].loop?"On ":"Off");
    textprintf_ex(screen,font,x+56,y+24+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].start);
    textprintf_ex(screen,font,x+56,y+32+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].loop_start);
    textprintf_ex(screen,font,x+56,y+40+5,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%-5ld",customMIDIs[i].loop_end);
    }
    */
  return jwin_list_proc(msg,d,c);
}

static DIALOG selectmusic_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     24,   20,   273,  189,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select Enhanced Music" },
  { d_dummy_proc,      160,  56,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { d_musiclist_proc,   31,   44,   164,   (1+16)*8,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *) enhancedmusiclist },
  { jwin_button_proc,     90,   160+12+12,  61,   21,   vc(14),  vc(1),  13,     D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  160+12+12,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, (void *) close_dlg },
  { NULL }
};

int onEnhancedMusic()
{
  stopMusic();
  int ret;
  char buf[40];
  number_list_size=MAXCUSTOMMIDIS;
  number_list_zero=false;
  strcpy(temppath,midipath);
  selectmusic_dlg[0].dp2=lfont;
  go();
  ret=zc_do_dialog(selectmusic_dlg,2);
  while(ret!=4&&ret!=0)
  {
    int d=selectmusic_dlg[2].d1;
    if(ret==5)
    {
      sprintf(buf,"Delete MIDI %d?",d+1);
      if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
      {
        reset_midi(customMIDIs+d);
        saved=false;
      }
    }
    else
    {
      edit_midi(d);
    }
    ret=zc_do_dialog(selectmusic_dlg,2);
  }
  comeback();
  return D_O_K;
}



/*******************************/
/****  onTest/onTestOptions  ***/
/*******************************/

int onTest()
{

#ifdef ALLEGRO_DOS
  PALETTE temppal;
  get_palette(temppal);
  remove_mouse();
  go();
  set_gfx_mode(GFX_TEXT,80,50,0,0);
  system("zelda.exe");
  set_gfx_mode(mode,zq_screen_w,zq_screen_h,0,0);
  comeback();
  install_mouse();
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  set_palette(temppal);
  show_mouse(screen);
  unscare_mouse();
  refresh(rALL);
#elif defined(ALLEGRO_WINDOWS)
  system("zelda.exe");
#endif
  return D_O_K;
}

int onTestOptions()
{

  return D_O_K;
}

/*******************************/
/**********  onWarp  ***********/
/*******************************/

char *warptypelist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=MAXWARPTYPES)
      index=MAXWARPTYPES-1;
    return warptype_string[index];
  }
  *list_size=MAXWARPTYPES;
  //  *list_size=6;
  return NULL;
}

char *warpeffectlist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=MAXWARPEFFECTS)
      index=MAXWARPEFFECTS-1;
    return warpeffect_string[index];
  }
  *list_size=MAXWARPEFFECTS;
  return NULL;
}

//int warpdmapxy[6] = {188,131,188,111,188,120};
//int warpdmapxy[6] = {188-68,131-93,188-68,111-93,188-68,120-93};
int warpdmapxy[6] = {188-68,131-93,188-68,111-93,188-68,120-93};
//int warpdmapxy[6] = {2,25,0,17,36,17};

static int warp1_list[] =
{
  2,3,4,5,6,7,8,9,10,11,12,13,53,54,-1
};

static int warp2_list[] =
{
  17,18,19,20,21,22,23,24,25,26,27,28,55,56,-1
};

static int warp3_list[] =
{
  29,30,31,32,33,34,35,36,37,38,39,40,57,58,-1
};

static int warp4_list[] =
{
  41,42,43,44,45,46,47,48,49,50,51,52,59,60,-1
};

static TABPANEL warp_tabs[] =
{
  // (text)
  { "A", D_SELECTED, warp1_list },
  { "B", 0,          warp2_list },
  { "C", 0,          warp3_list },
  { "D", 0,          warp4_list },

  { NULL }
};

static char warpr_buf[10];
char *warprlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,3);
    sprintf(warpr_buf,"%c",index+0x41);
    return warpr_buf;
  }
  *list_size=4;
  return NULL;
}

int d_wflag_proc(int msg,DIALOG *d,int c);


static DIALOG warp_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     9,   44,   302,  183,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { jwin_tab_proc,        15,   68,   290,  130,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) warp_tabs, NULL, (void *)warp_dlg },
  //{ d_dummy_proc,      160,  75,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,    70,   99,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
  { jwin_text_proc,    38,   117,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
  { jwin_text_proc,    87,   135,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
  { jwin_text_proc,    87,   151,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
  { jwin_frame_proc,   137,  153,  30,   30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  // 7
  { jwin_droplist_proc,   100,  95,   193,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warptypelist },
  { d_dropdmaplist_proc,   68,  113,   225,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, warpdmapxy },
  { jwin_hexedit_proc,    136,  131,  24,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 10
  { d_wflag_proc,      143,  150,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      143,  178,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      134,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      162,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  // 14
  { jwin_button_proc,     70,   202,  41,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     130,  202,  41,   21,   vc(14),  vc(1),  'g',     D_EXIT,     0,             0,       (void *) "&Go" },
  { jwin_button_proc,     190,  202,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  //17
  { jwin_text_proc,    70,   99,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
  { jwin_text_proc,    38,   117,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
  { jwin_text_proc,    87,   135,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
  { jwin_text_proc,    87,   151,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
  { jwin_frame_proc,   137,  153,  30,   30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  // 22
  { jwin_droplist_proc,   100,  95,   193,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warptypelist },
  { d_dropdmaplist_proc,   68,  113,   225,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, warpdmapxy },
  { jwin_hexedit_proc,    136,  131,  24,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 25
  { d_wflag_proc,      143,  150,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      143,  178,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      134,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      162,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  //29
  { jwin_text_proc,    70,   99,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
  { jwin_text_proc,    38,   117,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
  { jwin_text_proc,    87,   135,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
  { jwin_text_proc,    87,   151,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
  { jwin_frame_proc,   137,  153,  30,   30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  // 34
  { jwin_droplist_proc,   100,  95,   193,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warptypelist },
  { d_dropdmaplist_proc,   68,  113,   225,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, warpdmapxy },
  { jwin_hexedit_proc,    136,  131,  24,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 37
  { d_wflag_proc,      143,  150,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      143,  178,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      134,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      162,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  //41
  { jwin_text_proc,    70,   99,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
  { jwin_text_proc,    38,   117,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
  { jwin_text_proc,    87,   135,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
  { jwin_text_proc,    87,   151,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
  { jwin_frame_proc,   137,  153,  30,   30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL },
  // 46
  { jwin_droplist_proc,   100,  95,   193,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warptypelist },
  { d_dropdmaplist_proc,   68,  113,   225,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, warpdmapxy },
  { jwin_hexedit_proc,    136,  131,  24,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 49
  { d_wflag_proc,      143,  150,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      143,  178,  18,   8,    vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      134,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { d_wflag_proc,      162,  159,  8,    18,   vc(4),  vc(0),  0,       0,          1,             0,       NULL },
  { jwin_text_proc,        38,   167,   100,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use Warp Return:" },
  { jwin_droplist_proc,    83,   177,   50,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) warprlist },
  { jwin_text_proc,        38,   167,   100,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use Warp Return:" },
  { jwin_droplist_proc,    83,   177,   50,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) warprlist },
  { jwin_text_proc,        38,   167,   100,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use Warp Return:" },
  { jwin_droplist_proc,    83,   177,   50,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) warprlist },
  { jwin_text_proc,        38,   167,   100,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use Warp Return:" },
  { jwin_droplist_proc,    83,   177,   50,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) warprlist },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};

int d_wflag_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_DRAW:
    {
      int c=(d->flags&D_SELECTED)?d->fg:d->bg;
      /*if(!(d->d2&0x80))
        {
        c=d->bg;
        }*/
      if (d->d1==1)
      {
        jwin_draw_frame(screen,d->x,d->y,d->w,d->h, FR_DEEP);
        rectfill(screen,d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3,c);
        if(d->flags&D_SELECTED)
        {
          int e=d->d2&3;
          if(d->w>d->h)
            textprintf_centre_ex(screen,font,d->x+(d->w/2),d->y,jwin_pal[jcBOXFG],-1,"%c",e+0x41);
          else
            textprintf_centre_ex(screen,font,d->x+(d->w/2),d->y+(d->h/2)-4,jwin_pal[jcBOXFG],-1,"%c",e+0x41);
        }

      }
      else
      {
        rectfill(screen,d->x, d->y, d->x+d->w-1, d->y+d->h-1,c);
      }
    }
    break;
    case MSG_CLICK:
    {
      if(d->d1==1)
      {
        if(!(d->flags&D_SELECTED))
        {
          d->flags|=D_SELECTED;
          d->d2&=0x80;
          int g;
          if(d==&warp_dlg[10]||d==&warp_dlg[25]||d==&warp_dlg[37]||d==&warp_dlg[49]) g=0;
          else if(d==&warp_dlg[11]||d==&warp_dlg[26]||d==&warp_dlg[38]||d==&warp_dlg[50]) g=1;
            else if(d==&warp_dlg[12]||d==&warp_dlg[27]||d==&warp_dlg[39]||d==&warp_dlg[51]) g=2;
              else g=3;
          warp_dlg[10+g].flags = d->flags; warp_dlg[10+g].d2 = d->d2;
          warp_dlg[25+g].flags = d->flags; warp_dlg[25+g].d2 = d->d2;
          warp_dlg[37+g].flags = d->flags; warp_dlg[37+g].d2 = d->d2;
          warp_dlg[49+g].flags = d->flags; warp_dlg[49+g].d2 = d->d2;
        }
        else
        {
          if((d->d2&3)==3)
          {
            d->flags^=D_SELECTED;
            d->d2&=0x80;
            int g;
            if(d==&warp_dlg[10]||d==&warp_dlg[25]||d==&warp_dlg[37]||d==&warp_dlg[49]) g=0;
            else if(d==&warp_dlg[11]||d==&warp_dlg[26]||d==&warp_dlg[38]||d==&warp_dlg[50]) g=1;
              else if(d==&warp_dlg[12]||d==&warp_dlg[27]||d==&warp_dlg[39]||d==&warp_dlg[51]) g=2;
                else g=3;
            warp_dlg[10+g].flags = d->flags; warp_dlg[10+g].d2 = d->d2;
            warp_dlg[25+g].flags = d->flags; warp_dlg[25+g].d2 = d->d2;
            warp_dlg[37+g].flags = d->flags; warp_dlg[37+g].d2 = d->d2;
            warp_dlg[49+g].flags = d->flags; warp_dlg[49+g].d2 = d->d2;
          }
          else
          {
            int f=d->d2&3;
            d->d2&=0x80;
            f++;
            d->d2|=f;
            int g;
            if(d==&warp_dlg[10]||d==&warp_dlg[25]||d==&warp_dlg[37]||d==&warp_dlg[49]) g=0;
            else if(d==&warp_dlg[11]||d==&warp_dlg[26]||d==&warp_dlg[38]||d==&warp_dlg[50]) g=1;
              else if(d==&warp_dlg[12]||d==&warp_dlg[27]||d==&warp_dlg[39]||d==&warp_dlg[51]) g=2;
                else g=3;
            warp_dlg[10+g].flags = d->flags; warp_dlg[10+g].d2 = d->d2;
            warp_dlg[25+g].flags = d->flags; warp_dlg[25+g].d2 = d->d2;
            warp_dlg[37+g].flags = d->flags; warp_dlg[37+g].d2 = d->d2;
            warp_dlg[49+g].flags = d->flags; warp_dlg[49+g].d2 = d->d2;
          }
        }
      }
      else
      {
        d->flags^=D_SELECTED;
      }
      int c=(d->flags&D_SELECTED)?d->fg:d->bg;
      /*if(!(d->d2&0x80))
        {
        c=d->bg;
        }*/
      scare_mouse();
      if (d->d1==1)
      {
        jwin_draw_frame(screen,d->x,d->y,d->w,d->h, FR_DEEP);
        rectfill(screen,d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3,c);
        if(d->flags&D_SELECTED)
        {
          int e=d->d2&3;
          if(d->w>d->h)
            textprintf_centre_ex(screen,font,d->x+(d->w/2),d->y,jwin_pal[jcBOXFG],-1,"%c",e+0x41);
          else
            textprintf_centre_ex(screen,font,d->x+(d->w/2),d->y+(d->h/2)-4,jwin_pal[jcBOXFG],-1,"%c",e+0x41);
        }
      }
      else
      {
        rectfill(screen,d->x, d->y, d->x+d->w-1, d->y+d->h-1,c);
      }
      unscare_mouse();
      while(gui_mouse_b());
    }
    break;
  }
  return D_O_K;
}

static int north_side_warp_list[] =
{
  // dialog control number
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, -1
};

static int south_side_warp_list[] =
{
  // dialog control number
  80, -1
};

static int west_side_warp_list[] =
{
  // dialog control number
  80, -1
};

static int east_side_warp_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_1_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_2_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_3_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_4_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_5_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_6_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_7_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_8_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_9_list[] =
{
  // dialog control number
  80, -1
};

/*
static int tile_warp_list[] =
{
  // dialog control number
  80, -1
};

static int side_warp_list[] =
{
  // dialog control number
  6, -1
};

static int item_warp_list[] =
{
  // dialog control number
  80, -1
};

static int wind_warp_list[] =
{
  // dialog control number
  7, -1
};

static int special_warp_list[] =
{
  // dialog control number
  80, -1
};

static int timed_warp_list[] =
{
  // dialog control number
  80, -1
};*/


static TABPANEL side_warp_tabs[] =
{
  // (text)
  { "North",       D_SELECTED,  north_side_warp_list },
  { "South",       0,           south_side_warp_list },
  { "West",        0,           west_side_warp_list },
  { "East",        0,           east_side_warp_list },
  { NULL }
};

static TABPANEL wind_warp_tabs[] =
{
  // (text)
  { "1",       D_SELECTED,  wind_warp_1_list },
  { "2",       0,           wind_warp_2_list },
  { "3",       0,           wind_warp_3_list },
  { "4",       0,           wind_warp_4_list },
  { "5",       0,           wind_warp_5_list },
  { "6",       0,           wind_warp_6_list },
  { "7",       0,           wind_warp_7_list },
  { "8",       0,           wind_warp_8_list },
  { "9",       0,           wind_warp_9_list },
  { NULL }
};


/*static TABPANEL warp_tabs[] =
  {
  // (text)
  { "Tile",        D_SELECTED,  tile_warp_list },
  { "Side",        0,           side_warp_list },
  { "Item",        0,           item_warp_list },
  { "Wind",        0,           wind_warp_list },
  { "Special",     0,           special_warp_list },
  { "Timed",       0,           timed_warp_list },
  { NULL }
  };*/


int d_dmapscrsel_proc(int msg,DIALOG *d,int c)
{
  int ret = D_O_K;
  switch(msg)
  {
    case MSG_CLICK:
    sprintf((char*)((d+2)->dp),"%X%X",vbound((gui_mouse_y()-d->y)/4,0,7),vbound((gui_mouse_x()-d->x)/(((DMaps[(d-1)->d1].type&dmfTYPE)==1)?4:8),0,(((DMaps[(d-1)->d1].type&dmfTYPE)==1)?15:7)));
    object_message(d+2, MSG_DRAW, 0);
    break;
  }
  return ret;
}

int warpdestsel_x=-1;
int warpdestsel_y=-1;
int warpdestmap=-1;
int warpdestscr=-1;

int d_warpdestsel_proc(int msg,DIALOG *d,int c)
{
  int ret=D_O_K;
  static BITMAP *bmp=create_bitmap_ex(8,256,176);
  static bool inrect=false;
  static bool mousedown=false;
  switch(msg)
  {
    case MSG_START:
    loadlvlpal(Map.AbsoluteScr(warpdestmap,warpdestscr)->color);
    rebuild_trans_table();
    break;
    case MSG_DRAW:
    {
      scare_mouse();
      jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
      if(AnimationOn||CycleOn)
      {
        if (AnimationOn)
        {
          animate_combos();
        }
        if(CycleOn)
        {
          cycle_palette();
        }
      }
      animate_coords();
      Map.draw(bmp, 0, 0, 0, warpdestmap, warpdestscr);
      blit(icon_bmp[ICON_BMP_WARPDEST][coord_frame], bmp, 0, 0, Map.AbsoluteScr(warpdestmap,warpdestscr)->warparrivalx, Map.AbsoluteScr(warpdestmap,warpdestscr)->warparrivaly, 16, 16);
      int px=((gui_mouse_x()-d->x-2)&0xF8);
      int py=((gui_mouse_y()-d->y-2)&0xF8);
      if (isinRect(gui_mouse_x(), gui_mouse_y(), d->x+2,d->y+2,d->x+256+1,d->y+176+1))
      {
        if(gui_mouse_b())
        {
          if (!mousedown||!inrect)
          {
            set_mouse_sprite(mouse_bmp[MOUSE_BMP_BLANK][0]);
            zq_set_mouse_range(d->x+2, d->y+2, d->x+256+1, d->y+176+1);
          }
          rect(bmp, px, py, px+15, py+15, vc(15));
          warpdestsel_x=px;
          warpdestsel_y=py;
          mousedown=true;
        }
        else
        {
          if (mousedown||!inrect)
          {
            zq_set_mouse_range(0,0,zq_screen_w-1,zq_screen_h-1);
            set_mouse_sprite(mouse_bmp[MOUSE_BMP_POINT_BOX][0]);
          }
          mousedown=false;
        }
        inrect=true;
      }
      else
      {
        set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
        inrect=false;
      }
      blit(bmp, screen, 0, 0, d->x+2, d->y+2, 256, 176);
      unscare_mouse();
    }
    break;
    case MSG_VSYNC:
    d->flags|=D_DIRTY;
    break;
    case MSG_END:
    loadlvlpal(Map.CurrScr()->color);
    rebuild_trans_table();
    break;
  }
  return ret;
}

int d_vsync_proc(int msg,DIALOG *d,int c)
{
  static clock_t tics;
  switch(msg)
  {
    case MSG_START:
    tics=clock()+(CLOCKS_PER_SEC/60);
    break;
    case MSG_IDLE:
    if (clock()>tics)
    {
      tics=clock()+(CLOCKS_PER_SEC/60);
      broadcast_dialog_message(MSG_VSYNC, c);
    }
    break;
  }
  return D_O_K;
}


static DIALOG warpdestsel_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      0,                 0,    320-23, 234,    vc(14),  vc(1),    0,       D_EXIT,     0,             0,       (void *) "Select Warp Destination" },
  { jwin_button_proc,   6,                 207,  61,     21,     vc(0),   vc(11),   13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   6+60+4,            207,  61+32,  21,     vc(0),   vc(11),   0,       D_EXIT,     0,             0,       (void *) "Use Warp Square" },
  { jwin_button_proc,   6+60+4+92+4,       207,  61,     21,     vc(0),   vc(11),   0,       D_EXIT,     0,             0,       (void *) "Use Origin" },
  { jwin_button_proc,   6+60+4+92+4+60+4,  207,  61,     21,     vc(0),   vc(11),   27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,    0,                 0,    0,      0,      0,       0,        0,       0,          KEY_F1,        0,       (void *) onHelp },
  { d_warpdestsel_proc, 6+13,              23,   256+4,  176+4,  0,       0,        0,       0,          0,             0,       NULL },
  { d_vsync_proc,       0,                 0,    0,      0,      0,       0,        0,       0,          0,             0,       NULL },
  { NULL }
};

int d_warpbutton_proc(int msg,DIALOG *d,int c)
{
  int ret=jwin_button_proc(msg,d,c);
  if (ret==D_EXIT)
  {
    warpdestsel_dlg[0].dp2=lfont;
    warpdestmap=DMaps[(d-4)->d1].map;
    warpdestscr=DMaps[(d-4)->d1].xoff+xtoi((char*)((d-1)->dp));
    ret=zc_popup_dialog(warpdestsel_dlg,-1);
    switch (ret)
    {
      case 1:
      d->d1=warpdestsel_x;
      d->d2=warpdestsel_y;
      sprintf((char *)d->dp, "at: %dx%d", warpdestsel_x, warpdestsel_y);
      break;
      case 2:
      d->d1=-1;
      d->d2=-1;
      sprintf((char *)d->dp, "at: warp square");
      break;
      case 3:
      d->d1=-2;
      d->d2=-2;
      sprintf((char *)d->dp, "at: origin");
      break;
      default:
      break;
    }
    d->flags|=D_DIRTY;
  }
  return ret?D_O_K:D_O_K;
}


int jwin_minibutton_proc(int msg,DIALOG *d,int c)
{
  switch (msg)
  {
    case MSG_DRAW:
    jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, false);
    return D_O_K;
    break;
  }
  return jwin_button_proc(msg,d,c);
}

int d_triggerbutton_proc(int msg,DIALOG *d,int c)
{
  static BITMAP *dummy=create_bitmap_ex(8, 1, 1);
  switch(msg)
  {
    case MSG_START:
    d->w=gui_textout_ln(dummy, (unsigned char *)d->dp, 0, 0, jwin_pal[jcMEDDARK], -1, 0)+4;
    d->h=text_height(font)+5;
    break;
    case MSG_GOTFOCUS:
    d->flags&=~D_GOTFOCUS;
    break;

  }
  return jwin_minibutton_proc(msg,d,c);
}

int d_alltriggerbutton_proc(int msg,DIALOG *d,int c)
{
  DIALOG *temp_d;
  int ret=d_triggerbutton_proc(msg,d,c);
  switch(msg)
  {
    case MSG_CLICK:
    temp_d=d-1;
    while(temp_d->proc==d_triggerbutton_proc)
    {
      temp_d->flags&=~D_SELECTED;
      temp_d->flags|=D_DIRTY;
      if(d->flags&D_SELECTED)
      {
        temp_d->flags|=D_SELECTED;
      }
      --temp_d;
    }
    break;
  }
  return ret;
}

int d_ticsedit_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_edit_proc(msg,d,c);
  if(msg==MSG_DRAW)
  {
    scare_mouse();
    int tics=vbound(atoi((char*)d->dp),0,65535);
    int mins=tics/(60*60);
    tics=tics-(mins*60*60);
    int secs=tics/60;
    tics=tics-(secs*60);
    tics=tics*100/60;
    if(mins>0)
    {
      sprintf((char*)(d+1)->dp,"=%d:%02d.%02d      ",mins, secs, tics);
    }
    else
    {
      sprintf((char*)(d+1)->dp,"=%d.%02d seconds ",secs, tics);
    }
    object_message(d+1,MSG_DRAW,c);
    unscare_mouse();
  }

  return ret;
}



static DIALOG warp_dlg2[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      13,   24,  320,  240,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "Edit Warps" },
  { jwin_button_proc,   70+13,  188+51,   41,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,  130+13,  188+51,   41,   21,   vc(14),  vc(1),  'g',     D_EXIT,     0,             0,       (void *) "&Go" },
  { jwin_button_proc,  190+13,  188+51,   61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,     0,    0,    0,    0,   0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  //5
  { jwin_tab_proc,    13+6,  24+25,  295+13,  189-56+51,   0,       0,      0,       0,             0,       0,       (void *) warp_tabs, NULL, (void *)warp_dlg },
  { jwin_tab_proc,    13+6+4,  24+25+20,  295+13-8,  189-56-25+51,   0,       0,      0,       0,             0,       0,       (void *) side_warp_tabs, NULL, (void *)warp_dlg },
  { jwin_tab_proc,    13+6+4,  24+25+20,  295+13-8,  189-56-25+51,   0,       0,      0,       0,             0,       0,       (void *) wind_warp_tabs, NULL, (void *)warp_dlg },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  //10
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  //15
  { jwin_text_proc,      13+6+4+4,  24+25+20+20+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
  { jwin_text_proc,      13+6+4+4,  24+25+20+20+4+18,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
  { jwin_droplist_proc,   56,  24+25+20+20,   181-70,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warptypelist },
  { d_dropdmaplist_proc,   68-12,  24+25+20+20+18,   222,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, warpdmapxy },
  { d_dmapscrsel_proc,   58,     172-42+2,   65,  33,    vc(14),  vc(5),  0,       0,          1,             0,       NULL },
  //20
  { jwin_text_proc,      116+13,  24+25+20+20+18+23+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
  { jwin_edit_proc,      116+13+50,  24+25+20+20+18+23,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL },
  { d_warpbutton_proc,   204,  24+25+20+20+18+21,   90,   21,   vc(14),  vc(1),  0,     D_EXIT,     -1,             -1,       NULL },
  { jwin_check_proc,     116+13,  24+25+20+20+18+23+4+11,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enabled" },
  { jwin_check_proc,     116+13,  24+25+20+20+18+23+4+11+11,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Full Screen" },
  //25
  { jwin_text_proc,      13+6+4+4,  24+25+20+20+18+100-38+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Out Effect:" },
  { jwin_text_proc,      13+6+4+4,  24+25+20+20+18+118-38+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "In Effect:" },
  { jwin_droplist_proc,   68-12+30-5,  24+25+20+20+18+100-38,   222-78-7,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warpeffectlist },
  { jwin_droplist_proc,   68-12+30-5,  24+25+20+20+18+118-38,   222-78-7,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) warpeffectlist },
  { jwin_text_proc,      13+6+4+4,  24+25+20+20+18+118-38+18+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Tics:" },
  //30
  { d_ticsedit_proc,      68-12-3,  24+25+20+20+18+118-38+18,   36,   16,    vc(11),  vc(1),  0,       0,          5,             0,       NULL },
  { jwin_text_proc,      68-12-3+36+1,  24+25+20+20+18+118-38+18+4,   0,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_frame_proc,   203+21,    24+25+20+20+18+100-38+3-16,   93+2,   70-6,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    207+21,    24+25+20+20+18+100-38-16,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Triggers " },
  { d_triggerbutton_proc,     203+21+4,   24+25+20+20+18+100-38+10-16,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Up" },
  //35
  { d_triggerbutton_proc,     203+21+4+14,   24+25+20+20+18+100-38+10-16,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Down" },
  { d_triggerbutton_proc,     203+21+4+39,   24+25+20+20+18+100-38+10-16,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Left" },
  { d_triggerbutton_proc,     203+21+4+62,   24+25+20+20+18+100-38+10-16,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Right" },
  { d_triggerbutton_proc,     203+21+4+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "A" },
  { d_triggerbutton_proc,     203+21+4+10-1+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "B" },
  //40
  { d_triggerbutton_proc,     203+21+4+20-2+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "X" },
  { d_triggerbutton_proc,     203+21+4+30-2+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Y" },
  { d_triggerbutton_proc,     203+21+4+40-2+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "L" },
  { d_triggerbutton_proc,     203+21+4+50-3+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "R" },
  { d_triggerbutton_proc,     203+21+4+60-4+6,   24+25+20+20+18+100-38+10-16+13,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Map" },
  //45
  { d_triggerbutton_proc,     203+21+4+15,   24+25+20+20+18+100-38+10-16+26,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Select" },
  { d_triggerbutton_proc,     203+21+4+30+15,   24+25+20+20+18+100-38+10-16+26,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Start" },
  { d_alltriggerbutton_proc,     203+21+4+15+22,   24+25+20+20+18+100-38+10-16+40,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "All" },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  //35
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  //35
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  //35
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { d_dummy_proc,        0,    0,    0,    0,      0,        0,     0,          0,       0,          0,       NULL },
  { NULL }
};


int onWarps()
{
  restore_mouse();
  char warpscr[80], warpdest[80];
  char tics_str[80];
  char tics_secs_str[80];
  sprintf(warpscr, "00");
  sprintf(warpdest, "at: warp square");
  sprintf(tics_str, "0");
  sprintf(tics_secs_str, "=0.00 seconds");
  warp_dlg[0].dp2=lfont;
  warp_dlg[21].dp=warpscr;
  warp_dlg[22].dp=warpdest;
  warp_dlg[30].dp=tics_str;
  warp_dlg[31].dp=tics_secs_str;
  warp_dlg[34].dp2=pfont;
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;


  scare_mouse();
  go();
  clear_to_color(screen,0);
  unscare_mouse();

  int ret=zc_popup_dialog(warp_dlg,-1);

  scare_mouse();
  comeback();
  unscare_mouse();

  return ret?D_O_K:D_O_K;
}


int onTileWarp()
{
  int tempx5=warp_dlg[5].x;
  int tempx6=warp_dlg[6].x;
  int tempx10=warp_dlg[10].x;
  int tempx11=warp_dlg[11].x;
  int tempx12=warp_dlg[12].x;
  int tempx13=warp_dlg[13].x;

  int tempx20=warp_dlg[20].x;
  int tempx21=warp_dlg[21].x;
  int tempx25=warp_dlg[25].x;
  int tempx26=warp_dlg[26].x;
  int tempx27=warp_dlg[27].x;
  int tempx28=warp_dlg[28].x;

  int tempx32=warp_dlg[32].x;
  int tempx33=warp_dlg[33].x;
  int tempx37=warp_dlg[37].x;
  int tempx38=warp_dlg[38].x;
  int tempx39=warp_dlg[39].x;
  int tempx40=warp_dlg[40].x;

  int tempx44=warp_dlg[44].x;
  int tempx45=warp_dlg[45].x;
  int tempx49=warp_dlg[49].x;
  int tempx50=warp_dlg[50].x;
  int tempx51=warp_dlg[51].x;
  int tempx52=warp_dlg[52].x;
  restore_mouse();
  warp_dlg[0].dp=(void *) "Tile Warp";
  warp_dlg[0].dp2=lfont;
  warp_dlg[5].x = SCREEN_W+10;
  warp_dlg[6].x = SCREEN_W+10;
  warp_dlg[10].x = SCREEN_W+10;
  warp_dlg[11].x = SCREEN_W+10;
  warp_dlg[12].x = SCREEN_W+10;
  warp_dlg[13].x = SCREEN_W+10;
  warp_dlg[20].x = SCREEN_W+10;
  warp_dlg[21].x = SCREEN_W+10;
  warp_dlg[25].x = SCREEN_W+10;
  warp_dlg[26].x = SCREEN_W+10;
  warp_dlg[27].x = SCREEN_W+10;
  warp_dlg[28].x = SCREEN_W+10;
  warp_dlg[32].x = SCREEN_W+10;
  warp_dlg[33].x = SCREEN_W+10;
  warp_dlg[37].x = SCREEN_W+10;
  warp_dlg[38].x = SCREEN_W+10;
  warp_dlg[39].x = SCREEN_W+10;
  warp_dlg[40].x = SCREEN_W+10;
  warp_dlg[44].x = SCREEN_W+10;
  warp_dlg[45].x = SCREEN_W+10;
  warp_dlg[49].x = SCREEN_W+10;
  warp_dlg[50].x = SCREEN_W+10;
  warp_dlg[51].x = SCREEN_W+10;
  warp_dlg[52].x = SCREEN_W+10;
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0;
    warp_dlg[25+i].d2 = 0;
    warp_dlg[37+i].d2 = 0;
    warp_dlg[49+i].d2 = 0;
  }

  char buf[10];
  char buf2[10];
  char buf3[10];
  char buf4[10];
  sprintf(buf,"%02X",Map.CurrScr()->tilewarpscr[0]);
  warp_dlg[7].d1=Map.CurrScr()->tilewarptype[0];
  warp_dlg[8].d1=Map.CurrScr()->tilewarpdmap[0];
  warp_dlg[9].dp=buf;

  sprintf(buf2,"%02X",Map.CurrScr()->tilewarpscr[1]);
  warp_dlg[22].d1=Map.CurrScr()->tilewarptype[1];
  warp_dlg[23].d1=Map.CurrScr()->tilewarpdmap[1];
  warp_dlg[24].dp=buf2;

  sprintf(buf3,"%02X",Map.CurrScr()->tilewarpscr[2]);
  warp_dlg[34].d1=Map.CurrScr()->tilewarptype[2];
  warp_dlg[35].d1=Map.CurrScr()->tilewarpdmap[2];
  warp_dlg[36].dp=buf3;

  sprintf(buf4,"%02X",Map.CurrScr()->tilewarpscr[3]);
  warp_dlg[46].d1=Map.CurrScr()->tilewarptype[3];
  warp_dlg[47].d1=Map.CurrScr()->tilewarpdmap[3];
  warp_dlg[48].dp=buf4;

  int j=Map.CurrScr()->warpreturnc;
  warp_dlg[54].d1=(j&3);
  warp_dlg[56].d1=((j>>2)&3);
  warp_dlg[58].d1=((j>>4)&3);
  warp_dlg[60].d1=((j>>6)&3);
  warp_dlg[2].fg=vc(14);
  warp_dlg[17].fg=vc(14);
  warp_dlg[29].fg=vc(14);
  warp_dlg[41].fg=vc(14);
  //warp_dlg[5].fg=vc(7);
  //for(int i=0; i<4; i++)
  //warp_dlg[10+i].d2 = 0;
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  int ret=zc_popup_dialog(warp_dlg,-1);
  if(ret==14 || ret==15)
  {
    saved=false;
    Map.CurrScr()->tilewarpscr[0] = xtoi(buf);
    Map.CurrScr()->tilewarptype[0] = warp_dlg[7].d1;
    Map.CurrScr()->tilewarpdmap[0] = warp_dlg[8].d1;
    Map.CurrScr()->tilewarpscr[1] = xtoi(buf2);
    Map.CurrScr()->tilewarptype[1] = warp_dlg[22].d1;
    Map.CurrScr()->tilewarpdmap[1] = warp_dlg[23].d1;
    Map.CurrScr()->tilewarpscr[2] = xtoi(buf3);
    Map.CurrScr()->tilewarptype[2] = warp_dlg[34].d1;
    Map.CurrScr()->tilewarpdmap[2] = warp_dlg[35].d1;
    Map.CurrScr()->tilewarpscr[3] = xtoi(buf4);
    Map.CurrScr()->tilewarptype[3] = warp_dlg[46].d1;
    Map.CurrScr()->tilewarpdmap[3] = warp_dlg[47].d1;

    j=0;
    j|=warp_dlg[60].d1; j<<=2;
    j|=warp_dlg[58].d1; j<<=2;
    j|=warp_dlg[56].d1; j<<=2;
    j|=warp_dlg[54].d1;
    Map.CurrScr()->warpreturnc = j;
    refresh(rMENU);

  }
  if(ret==15)
  {
    int index=0;
    if(warp_tabs[0].flags & D_SELECTED) index = 0;
    if(warp_tabs[1].flags & D_SELECTED) index = 1;
    if(warp_tabs[2].flags & D_SELECTED) index = 2;
    if(warp_tabs[3].flags & D_SELECTED) index = 3;
    Map.dowarp(0,index);
    refresh(rALL);
  }

  warp_dlg[5].x = tempx5;
  warp_dlg[6].x = tempx6;
  warp_dlg[10].x = tempx10;
  warp_dlg[11].x = tempx11;
  warp_dlg[12].x = tempx12;
  warp_dlg[13].x = tempx13;

  warp_dlg[20].x = tempx20;
  warp_dlg[21].x = tempx21;
  warp_dlg[25].x = tempx25;
  warp_dlg[26].x = tempx26;
  warp_dlg[27].x = tempx27;
  warp_dlg[28].x = tempx28;

  warp_dlg[32].x = tempx32;
  warp_dlg[33].x = tempx33;
  warp_dlg[37].x = tempx37;
  warp_dlg[38].x = tempx38;
  warp_dlg[39].x = tempx39;
  warp_dlg[40].x = tempx40;

  warp_dlg[44].x = tempx44;
  warp_dlg[45].x = tempx45;
  warp_dlg[49].x = tempx49;
  warp_dlg[50].x = tempx50;
  warp_dlg[51].x = tempx51;
  warp_dlg[52].x = tempx52;
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0x80;
    warp_dlg[25+i].d2 = 0x80;
    warp_dlg[37+i].d2 = 0x80;
    warp_dlg[49+i].d2 = 0x80;
  }

  return D_O_K;
}

int onSideWarp()
{
  restore_mouse();
  warp_dlg[0].dp=(void *) "Side Warp";
  warp_dlg[0].dp2=lfont;
  warp_dlg[7].flags = 0;
  warp_dlg[22].flags = 0;
  warp_dlg[34].flags = 0;
  warp_dlg[46].flags = 0;

  char buf[10]; char buf2[10]; char buf3[10]; char buf4[10];
  sprintf(buf,"%02X",Map.CurrScr()->sidewarpscr[0]);
  warp_dlg[7].d1=Map.CurrScr()->sidewarptype[0];
  warp_dlg[8].d1=Map.CurrScr()->sidewarpdmap[0];
  warp_dlg[9].dp=buf;

  sprintf(buf2,"%02X",Map.CurrScr()->sidewarpscr[1]);
  warp_dlg[22].d1=Map.CurrScr()->sidewarptype[1];
  warp_dlg[23].d1=Map.CurrScr()->sidewarpdmap[1];
  warp_dlg[24].dp=buf2;

  sprintf(buf3,"%02X",Map.CurrScr()->sidewarpscr[2]);
  warp_dlg[34].d1=Map.CurrScr()->sidewarptype[2];
  warp_dlg[35].d1=Map.CurrScr()->sidewarpdmap[2];
  warp_dlg[36].dp=buf3;

  sprintf(buf4,"%02X",Map.CurrScr()->sidewarpscr[3]);
  warp_dlg[46].d1=Map.CurrScr()->sidewarptype[3];
  warp_dlg[47].d1=Map.CurrScr()->sidewarpdmap[3];
  warp_dlg[48].dp=buf4;

  int j=Map.CurrScr()->warpreturnc;
  warp_dlg[54].d1=(j&3);
  warp_dlg[56].d1=((j>>2)&3);
  warp_dlg[58].d1=((j>>4)&3);
  warp_dlg[60].d1=((j>>6)&3);

  warp_dlg[2].fg=warp_dlg[5].fg=vc(14);
  warp_dlg[17].fg=warp_dlg[20].fg=vc(14);
  warp_dlg[29].fg=warp_dlg[32].fg=vc(14);
  warp_dlg[41].fg=warp_dlg[44].fg=vc(14);
  byte f=Map.CurrScr()->flags2;
  byte h=Map.CurrScr()->sidewarpindex;
  byte g=f&240;
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0x80;
    warp_dlg[25+i].d2 = 0x80;
    warp_dlg[37+i].d2 = 0x80;
    warp_dlg[49+i].d2 = 0x80;
    if(f&1)
    {
      warp_dlg[10+i].flags = D_SELECTED ;
      warp_dlg[10+i].d2 |= h&3;
      warp_dlg[25+i].flags = D_SELECTED ;
      warp_dlg[25+i].d2 |= h&3;
      warp_dlg[37+i].flags = D_SELECTED ;
      warp_dlg[37+i].d2 |= h&3;
      warp_dlg[49+i].flags = D_SELECTED ;
      warp_dlg[49+i].d2 |= h&3;
    }
    else
    {
      warp_dlg[10+i].flags = 0;
      warp_dlg[25+i].flags = 0;
      warp_dlg[37+i].flags = 0;
      warp_dlg[49+i].flags = 0;
    }
    f>>=1;
    h>>=2;
  }
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  int ret=zc_popup_dialog(warp_dlg,-1);
  if(ret==14 || ret==15)
  {
    saved=false;
    Map.CurrScr()->sidewarpscr[0] = xtoi(buf);
    Map.CurrScr()->sidewarptype[0] = warp_dlg[7].d1;
    Map.CurrScr()->sidewarpdmap[0] = warp_dlg[8].d1;
    Map.CurrScr()->sidewarpscr[1] = xtoi(buf2);
    Map.CurrScr()->sidewarptype[1] = warp_dlg[22].d1;
    Map.CurrScr()->sidewarpdmap[1] = warp_dlg[23].d1;
    Map.CurrScr()->sidewarpscr[2] = xtoi(buf3);
    Map.CurrScr()->sidewarptype[2] = warp_dlg[34].d1;
    Map.CurrScr()->sidewarpdmap[2] = warp_dlg[35].d1;
    Map.CurrScr()->sidewarpscr[3] = xtoi(buf4);
    Map.CurrScr()->sidewarptype[3] = warp_dlg[46].d1;
    Map.CurrScr()->sidewarpdmap[3] = warp_dlg[47].d1;
    f=0;
    h=0;
    for(int i=3; i>=0; i--)
    {
      f<<=1;
      h<<=2;
      //f |= warp_dlg[49+i].flags&D_SELECTED ? 1 : 0;
      //f |= warp_dlg[37+i].flags&D_SELECTED ? 1 : 0;
      //f |= warp_dlg[25+i].flags&D_SELECTED ? 1 : 0;
      f |= warp_dlg[10+i].flags&D_SELECTED ? 1 : 0;
      int t=0;
      /*if(warp_dlg[10+i].flags&D_SELECTED) t=0;
        else if(warp_dlg[25+i].flags&D_SELECTED) t=1;
        else if(warp_dlg[37+i].flags&D_SELECTED) t=2;
        else if(warp_dlg[49+i].flags&D_SELECTED) t=3;*/
      t=warp_dlg[10+i].d2&3;
      h|=t;
    }
    f+=g;
    Map.CurrScr()->flags2 = f;
    Map.CurrScr()->sidewarpindex = h;

    j=0;
    j|=warp_dlg[60].d1; j<<=2;
    j|=warp_dlg[58].d1; j<<=2;
    j|=warp_dlg[56].d1; j<<=2;
    j|=warp_dlg[54].d1;
    Map.CurrScr()->warpreturnc = j;
    refresh(rMENU);
  }
  if(ret==15)
  {
    int index=0;
    if(warp_tabs[0].flags & D_SELECTED) index = 0;
    if(warp_tabs[1].flags & D_SELECTED) index = 1;
    if(warp_tabs[2].flags & D_SELECTED) index = 2;
    if(warp_tabs[3].flags & D_SELECTED) index = 3;
    Map.dowarp(1,index);
    refresh(rALL);
  }
  return D_O_K;
}

void EditWindWarp(int level)
{
  char caption[40],buf[10];
  restore_mouse();
  int tempx5=warp_dlg[5].x;
  int tempx6=warp_dlg[6].x;
  int tempx10=warp_dlg[10].x;
  int tempx11=warp_dlg[11].x;
  int tempx12=warp_dlg[12].x;
  int tempx13=warp_dlg[13].x;

  int tempx[44];
  for(int m=17;m<61;m++)
  {
    tempx[m-17]=warp_dlg[m].x;
    warp_dlg[m].x = SCREEN_W+10;
  }

  /*int tempx20=warp_dlg[20].x;
    int tempx21=warp_dlg[21].x;
    int tempx25=warp_dlg[25].x;
    int tempx26=warp_dlg[26].x;
    int tempx27=warp_dlg[27].x;
    int tempx28=warp_dlg[28].x;

    int tempx32=warp_dlg[32].x;
    int tempx33=warp_dlg[33].x;
    int tempx37=warp_dlg[37].x;
    int tempx38=warp_dlg[38].x;
    int tempx39=warp_dlg[39].x;
    int tempx40=warp_dlg[40].x;

    int tempx44=warp_dlg[44].x;
    int tempx45=warp_dlg[45].x;
    int tempx49=warp_dlg[49].x;
    int tempx50=warp_dlg[50].x;
    int tempx51=warp_dlg[51].x;
    int tempx52=warp_dlg[52].x;*/

  warp_dlg[5].x = SCREEN_W+10;
  warp_dlg[6].x = SCREEN_W+10;
  warp_dlg[10].x = SCREEN_W+10;
  warp_dlg[11].x = SCREEN_W+10;
  warp_dlg[12].x = SCREEN_W+10;
  warp_dlg[13].x = SCREEN_W+10;
  /*warp_dlg[20].x = SCREEN_W+10;
    warp_dlg[21].x = SCREEN_W+10;
    warp_dlg[25].x = SCREEN_W+10;
    warp_dlg[26].x = SCREEN_W+10;
    warp_dlg[27].x = SCREEN_W+10;
    warp_dlg[28].x = SCREEN_W+10;
    warp_dlg[32].x = SCREEN_W+10;
    warp_dlg[33].x = SCREEN_W+10;
    warp_dlg[37].x = SCREEN_W+10;
    warp_dlg[38].x = SCREEN_W+10;
    warp_dlg[39].x = SCREEN_W+10;
    warp_dlg[40].x = SCREEN_W+10;
    warp_dlg[44].x = SCREEN_W+10;
    warp_dlg[45].x = SCREEN_W+10;
    warp_dlg[49].x = SCREEN_W+10;
    warp_dlg[50].x = SCREEN_W+10;
    warp_dlg[51].x = SCREEN_W+10;
    warp_dlg[52].x = SCREEN_W+10;*/
  sprintf(caption,"Whirlwind Warp %d",level+1);
  warp_dlg[0].dp = caption;
  warp_dlg[0].dp2=lfont;
  warp_dlg[1].dp = NULL;
  warp_dlg[1].dp3 = NULL;
  sprintf(buf,"%02X",misc.wind[level].scr);
  warp_dlg[8].d1=misc.wind[level].dmap;
  warp_dlg[9].dp=buf;
  warp_dlg[24].dp=buf;
  warp_dlg[36].dp=buf;
  warp_dlg[48].dp=buf;
  //  warp_dlg[2].fg=warp_dlg[5].fg=vc(7);
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0;
  }
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  int ret=zc_popup_dialog(warp_dlg,-1);
  if(ret==14 || ret==15)
  {
    saved=false;
    misc.wind[level].dmap = warp_dlg[8].d1;
    misc.wind[level].scr = xtoi(buf);
  }
  if(ret==15)
  {
    Map.dowarp(6+level, 0);
    refresh(rALL);
  }
  warp_dlg[5].x = tempx5;
  warp_dlg[6].x = tempx6;
  warp_dlg[10].x = tempx10;
  warp_dlg[11].x = tempx11;
  warp_dlg[12].x = tempx12;
  warp_dlg[13].x = tempx13;

  for(int m=17;m<61;m++)
  {
    warp_dlg[m].x=tempx[m-17];
  }
  /*warp_dlg[20].x = tempx20;
    warp_dlg[21].x = tempx21;
    warp_dlg[25].x = tempx25;
    warp_dlg[26].x = tempx26;
    warp_dlg[27].x = tempx27;
    warp_dlg[28].x = tempx28;

    warp_dlg[32].x = tempx32;
    warp_dlg[33].x = tempx33;
    warp_dlg[37].x = tempx37;
    warp_dlg[38].x = tempx38;
    warp_dlg[39].x = tempx39;
    warp_dlg[40].x = tempx40;

    warp_dlg[44].x = tempx44;
    warp_dlg[45].x = tempx45;
    warp_dlg[49].x = tempx49;
    warp_dlg[50].x = tempx50;
    warp_dlg[51].x = tempx51;
    warp_dlg[52].x = tempx52;*/
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0x80;
    warp_dlg[25+i].d2 = 0x80;
    warp_dlg[37+i].d2 = 0x80;
    warp_dlg[49+i].d2 = 0x80;
  }
  warp_dlg[1].dp = (void *) warp_tabs;
  warp_dlg[1].dp3 = (void *)warp_dlg;
}

int onWhistle()
{
  number_list_size = 9;
  number_list_zero = false;
  int index = select_data("Whistle Warps",0,numberlist,"Edit","Done",lfont);


  while(index!=-1)
  {
    EditWindWarp(index);
    number_list_size = 9;
    number_list_zero = false;
    index = select_data("Whistle Warps",index,numberlist,"Edit","Done",lfont);
  }

  return D_O_K;
}

/*******************************/
/*********** onPath ************/
/*******************************/

char *dirlist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>3)
      index=3;
    return dirstr[index];
  }
  *list_size=4;
  return NULL;
}


static DIALOG path_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      80,   57,   161,  144,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Maze Path" },
  { d_dummy_proc,        160,  71,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,       94,   88,   192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1st" },
  { jwin_text_proc,       94,   106,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2nd" },
  { jwin_text_proc,       94,   124,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3rd" },
  { jwin_text_proc,       94,   142,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4th" },
  { jwin_text_proc,       94,   160,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Exit" },
  { jwin_droplist_proc,   140,  84,   80+1,   16,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dirlist },
  { jwin_droplist_proc,   140,  102,   80+1,   16,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dirlist },
  { jwin_droplist_proc,   140,  120,  80+1,   16,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dirlist },
  { jwin_droplist_proc,   140,  138,  80+1,   16,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dirlist },
  { jwin_droplist_proc,   140,  156,  80+1,   16,   jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dirlist },
  { jwin_button_proc,     90,   176,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  176,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};

int onPath()
{
  restore_mouse();
  path_dlg[0].dp2=lfont;
  for(int i=0; i<4; i++)
    path_dlg[i+7].d1 = Map.CurrScr()->path[i];
  path_dlg[11].d1 = Map.CurrScr()->exitdir;
  int ret=zc_popup_dialog(path_dlg,7);
  if(ret==12)
  {
    saved=false;
    for(int i=0; i<4; i++)
      Map.CurrScr()->path[i] = path_dlg[i+7].d1;

    Map.CurrScr()->exitdir = path_dlg[11].d1;
  }
  refresh(rMAP+rMENU);
  return D_O_K;
}

/********************************/
/********* onInfoTypes **********/
/********************************/

static DIALOG editinfo_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)                 (bg)                  (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      56,   30,  208,  184,  vc(14),              vc(1),                  0,      D_EXIT,     0,             0,       NULL },
  { d_dummy_proc,      160,   48,    0,    8,  vc(15),              vc(1),                  0,           0,     0,             0,       NULL },
  { jwin_text_proc,     80,   60,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "1st" },
  { jwin_text_proc,     80,  106,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "2nd" },
  { jwin_text_proc,     80,  152,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "3rd" },
  { jwin_text_proc,    112,   60,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  { jwin_text_proc,    112,  106,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  { jwin_text_proc,    112,  152,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  // 8
  { jwin_edit_proc,    142,   56,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nlist_proc,      112,   74,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  { jwin_edit_proc,    142,  102,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nlist_proc,      112,  120,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  { jwin_edit_proc,    142,  148,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nlist_proc,      112,  166,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  // 14
  { jwin_button_proc,   90,  188,   61,   21,  vc(14),              vc(1),                 13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  188,   61,   21,  vc(14),              vc(1),                 27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

void EditInfoType(int index)
{
  char ps1[4],ps2[4],ps3[4];
  char caption[40];

  sprintf(caption,"Info Data %d",index);
  editinfo_dlg[0].dp = caption;
  editinfo_dlg[0].dp2 = lfont;

  sprintf(ps1,"%d",misc.info[index].price[0]);
  sprintf(ps2,"%d",misc.info[index].price[1]);
  sprintf(ps3,"%d",misc.info[index].price[2]);
  editinfo_dlg[8].dp  = ps1;
  editinfo_dlg[10].dp = ps2;
  editinfo_dlg[12].dp = ps3;
  editinfo_dlg[9].d1  = misc.info[index].str[0];
  editinfo_dlg[11].d1 = misc.info[index].str[1];
  editinfo_dlg[13].d1 = misc.info[index].str[2];
  editinfo_dlg[9].dp  =
    editinfo_dlg[11].dp =
    editinfo_dlg[13].dp = (void *) msgslist;

  int ret = zc_popup_dialog(editinfo_dlg,-1);

  if(ret==14)
  {
    saved=false;
    misc.info[index].price[0] = atoi(ps1);
    misc.info[index].price[1] = atoi(ps2);
    misc.info[index].price[2] = atoi(ps3);
    misc.info[index].str[0] = editinfo_dlg[9].d1;
    misc.info[index].str[1] = editinfo_dlg[11].d1;
    misc.info[index].str[2] = editinfo_dlg[13].d1;
    //filter all the 0 strings to the end (yeah, bubble sort; sue me)
    word swaptmp;
    for (int j=0; j<3-1; j++)
    {
      for (int k=0; k<2-j; k++)
      {
        if (misc.info[index].str[k]==0)
        {
          swaptmp = misc.info[index].str[k];
          misc.info[index].str[k] = misc.info[index].str[k+1];
          misc.info[index].str[k+1] = swaptmp;
          swaptmp = misc.info[index].price[k];
          misc.info[index].price[k] = misc.info[index].price[k+1];
          misc.info[index].price[k+1] = swaptmp;
        }
      }
    }
  }
}

int onInfoTypes()
{
  number_list_size = 16;
  number_list_zero = true;

  int index = select_data("Info Types",0,numberlist,"Edit","Done",lfont);

  while(index!=-1)
  {
    EditInfoType(index);

    index = select_data("Info Types",index,numberlist,"Edit","Done",lfont);
  }

  return D_O_K;
}

/********************************/
/********* onShopTypes **********/
/********************************/

static DIALOG editshop_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)                 (bg)                  (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      56,   30,  208,  184,  vc(14),              vc(1),                  0,      D_EXIT,     0,             0,       NULL },
  { d_dummy_proc,      160,   48,    0,    8,  vc(15),              vc(1),                  0,           0,     0,             0,       NULL },
  { jwin_text_proc,     80,   60,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "1st" },
  { jwin_text_proc,     80,  106,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "2nd" },
  { jwin_text_proc,     80,  152,   48,    8,  vc(7),               vc(1),                  0,           0,     0,             0,       (void *) "3rd" },
  { jwin_text_proc,    112,   60,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  { jwin_text_proc,    112,  106,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  { jwin_text_proc,    112,  152,   88,    8,  vc(14),              vc(1),                  0,           0,     0,             0,       (void *) "Price:" },
  // 8
  { jwin_edit_proc,    142,   56,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nilist_proc,     112,   74,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  { jwin_edit_proc,    142,  102,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nilist_proc,     112,  120,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  { jwin_edit_proc,    142,  148,   32,   16,  vc(12),              vc(1),                  0,           0,     3,             0,       NULL },
  { d_nilist_proc,     112,  166,  137,   16,  jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],     0,           0,     0,             0,       NULL },
  // 14

  { jwin_button_proc,   90,  188,   61,   21,  vc(14),              vc(1),                 13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  188,   61,   21,  vc(14),              vc(1),                 27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

void EditShopType(int index)
{

  build_bii_list(true);
  char ps1[4],ps2[4],ps3[4];
  char caption[40];

  sprintf(caption,"Shop Data %d",index);
  editshop_dlg[0].dp = caption;
  editshop_dlg[0].dp2=lfont;

  sprintf(ps1,"%d",misc.shop[index].price[0]);
  sprintf(ps2,"%d",misc.shop[index].price[1]);
  sprintf(ps3,"%d",misc.shop[index].price[2]);
  editshop_dlg[8].dp  = ps1;
  editshop_dlg[10].dp = ps2;
  editshop_dlg[12].dp = ps3;

  editshop_dlg[9].dp  = (void *) itemlist;
  for(int j=0; j<bii_cnt; j++)
    if(bii[j].i == misc.shop[index].item[0])
      editshop_dlg[9].d1  = j;
    editshop_dlg[11].dp  = (void *) itemlist;
  for(int j=0; j<bii_cnt; j++)
    if(bii[j].i == misc.shop[index].item[1])
      editshop_dlg[11].d1  = j;
    editshop_dlg[13].dp  = (void *) itemlist;
  for(int j=0; j<bii_cnt; j++)
    if(bii[j].i == misc.shop[index].item[2])
      editshop_dlg[13].d1  = j;

    int ret = zc_popup_dialog(editshop_dlg,-1);

  if(ret==14)
  {
    saved=false;
    misc.shop[index].price[0] = atoi(ps1);
    misc.shop[index].price[1] = atoi(ps2);
    misc.shop[index].price[2] = atoi(ps3);

    misc.shop[index].item[0] = bii[editshop_dlg[9].d1].i;
    misc.shop[index].item[1] = bii[editshop_dlg[11].d1].i;
    misc.shop[index].item[2] = bii[editshop_dlg[13].d1].i;
    //filter all the 0 items to the end (yeah, bubble sort; sue me)
    word swaptmp;
    for (int j=0; j<3-1; j++)
    {
      for (int k=0; k<2-j; k++)
      {
        if (misc.shop[index].item[k]==0)
        {
          swaptmp = misc.shop[index].item[k];
          misc.shop[index].item[k] = misc.shop[index].item[k+1];
          misc.shop[index].item[k+1] = swaptmp;
          swaptmp = misc.shop[index].price[k];
          misc.shop[index].price[k] = misc.shop[index].price[k+1];
          misc.shop[index].price[k+1] = swaptmp;
        }
      }
    }
  }
}

int onShopTypes()
{
  number_list_size = 16;
  number_list_zero = true;

  int index = select_data("Shop Types",0,numberlist,"Edit","Done",lfont);

  while(index!=-1)
  {
    EditShopType(index);
    index = select_data("Shop Types",index,numberlist,"Edit","Done",lfont);
  }

  return D_O_K;
}

/********************************/
/********* onWarpRings **********/
/********************************/

int curr_ring;

void EditWarpRingScr(int ring,int index)
{
  char caption[40],buf[10];
  restore_mouse();
  int tempx5=warp_dlg[5].x;
  int tempx6=warp_dlg[6].x;
  int tempx10=warp_dlg[10].x;
  int tempx11=warp_dlg[11].x;
  int tempx12=warp_dlg[12].x;
  int tempx13=warp_dlg[13].x;

  int tempx[44];
  for(int m=17;m<61;m++)
  {
    tempx[m-17]=warp_dlg[m].x;
    warp_dlg[m].x = SCREEN_W+10;
  }
  /*int tempx20=warp_dlg[20].x;
    int tempx21=warp_dlg[21].x;
    int tempx25=warp_dlg[25].x;
    int tempx26=warp_dlg[26].x;
    int tempx27=warp_dlg[27].x;
    int tempx28=warp_dlg[28].x;

    int tempx32=warp_dlg[32].x;
    int tempx33=warp_dlg[33].x;
    int tempx37=warp_dlg[37].x;
    int tempx38=warp_dlg[38].x;
    int tempx39=warp_dlg[39].x;
    int tempx40=warp_dlg[40].x;

    int tempx44=warp_dlg[44].x;
    int tempx45=warp_dlg[45].x;
    int tempx49=warp_dlg[49].x;
    int tempx50=warp_dlg[50].x;
    int tempx51=warp_dlg[51].x;
    int tempx52=warp_dlg[52].x;*/

  warp_dlg[5].x = SCREEN_W+10;
  warp_dlg[6].x = SCREEN_W+10;
  warp_dlg[10].x = SCREEN_W+10;
  warp_dlg[11].x = SCREEN_W+10;
  warp_dlg[12].x = SCREEN_W+10;
  warp_dlg[13].x = SCREEN_W+10;
  /*warp_dlg[20].x = SCREEN_W+10;
    warp_dlg[21].x = SCREEN_W+10;
    warp_dlg[25].x = SCREEN_W+10;
    warp_dlg[26].x = SCREEN_W+10;
    warp_dlg[27].x = SCREEN_W+10;
    warp_dlg[28].x = SCREEN_W+10;
    warp_dlg[32].x = SCREEN_W+10;
    warp_dlg[33].x = SCREEN_W+10;
    warp_dlg[37].x = SCREEN_W+10;
    warp_dlg[38].x = SCREEN_W+10;
    warp_dlg[39].x = SCREEN_W+10;
    warp_dlg[40].x = SCREEN_W+10;
    warp_dlg[44].x = SCREEN_W+10;
    warp_dlg[45].x = SCREEN_W+10;
    warp_dlg[49].x = SCREEN_W+10;
    warp_dlg[50].x = SCREEN_W+10;
    warp_dlg[51].x = SCREEN_W+10;
    warp_dlg[52].x = SCREEN_W+10;*/
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0;
    warp_dlg[25+i].d2 = 0;
    warp_dlg[37+i].d2 = 0;
    warp_dlg[49+i].d2 = 0;
  }

  sprintf(caption,"Ring %d  Warp %d",ring,index+1);
  warp_dlg[0].dp = (void *)caption;
  warp_dlg[0].dp2=lfont;

  warp_dlg[1].dp = NULL;
  warp_dlg[1].dp3 = NULL;

  sprintf(buf,"%02X",misc.warp[ring].scr[index]);
  warp_dlg[8].d1=misc.warp[ring].dmap[index];
  warp_dlg[9].dp=buf;
  warp_dlg[24].dp=buf;
  warp_dlg[36].dp=buf;
  warp_dlg[48].dp=buf;
  warp_dlg[2].fg=warp_dlg[5].fg=vc(7);
  for(int i=0; i<4; i++)
    warp_dlg[10+i].d2 = 0;
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  int ret=zc_popup_dialog(warp_dlg,-1);
  if(ret==14 || ret==15)
  {
    saved=false;
    misc.warp[ring].dmap[index] = warp_dlg[8].d1;
    misc.warp[ring].scr[index] = xtoi(buf);
  }
  if(ret==15)
  {
    Map.dowarp2(ring,index);
    refresh(rALL);
  }

  warp_dlg[5].x = tempx5;
  warp_dlg[6].x = tempx6;
  warp_dlg[10].x = tempx10;
  warp_dlg[11].x = tempx11;
  warp_dlg[12].x = tempx12;
  warp_dlg[13].x = tempx13;

  for(int m=17;m<61;m++)
  {
    warp_dlg[m].x=tempx[m-17];
  }

  /*warp_dlg[20].x = tempx20;
    warp_dlg[21].x = tempx21;
    warp_dlg[25].x = tempx25;
    warp_dlg[26].x = tempx26;
    warp_dlg[27].x = tempx27;
    warp_dlg[28].x = tempx28;

    warp_dlg[32].x = tempx32;
    warp_dlg[33].x = tempx33;
    warp_dlg[37].x = tempx37;
    warp_dlg[38].x = tempx38;
    warp_dlg[39].x = tempx39;
    warp_dlg[40].x = tempx40;

    warp_dlg[44].x = tempx44;
    warp_dlg[45].x = tempx45;
    warp_dlg[49].x = tempx49;
    warp_dlg[50].x = tempx50;
    warp_dlg[51].x = tempx51;
    warp_dlg[52].x = tempx52;*/
  for(int i=0; i<4; i++)
  {
    warp_dlg[10+i].d2 = 0x80;
    warp_dlg[25+i].d2 = 0x80;
    warp_dlg[37+i].d2 = 0x80;
    warp_dlg[49+i].d2 = 0x80;
  }
  warp_dlg[1].dp = (void *) warp_tabs;
  warp_dlg[1].dp3 = (void *)warp_dlg;

}

int d_warplist_proc(int msg,DIALOG *d,int c)
{

  if(msg==MSG_DRAW)
  {
    int *xy = (int*)(d->dp3);
    int ring = curr_ring;
    int dmap = misc.warp[ring].dmap[d->d1];

    drawdmap(dmap);
    if(xy[0]||xy[1])
    {
      jwin_draw_frame(screen,xy[0]-2,xy[1]-2, 69, 37, FR_DEEP);
      blit(dmapbmp_small,screen,0,0,xy[0],xy[1],65,33);
    }
    if(xy[2]||xy[3])
    {
      textprintf_ex(screen,font,xy[2],xy[3],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Map: %d",DMaps[dmap].map+1);
    }
    if(xy[4]||xy[5])
    {
      textprintf_ex(screen,font,xy[4],xy[5],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Level:%2d",DMaps[dmap].level);
    }
    if(xy[6]||xy[7])
    {
      textprintf_ex(screen,font,xy[6],xy[7],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Scr: 0x%02X",misc.warp[ring].scr[d->d1]);
    }
  }
  return jwin_list_proc(msg,d,c);
}

int d_wclist_proc(int msg,DIALOG *d,int c)
{
  int d1 = d->d1;
  int ret = jwin_droplist_proc(msg,d,c);
  misc.warp[curr_ring].size=d->d1+3;
  if(d->d1 != d1)
    return D_CLOSE;
  return ret;
}

char *wclist(int index, int *list_size)
{
  static char buf[2];
  if(index>=0)
  {
    if(index>5)
      index=5;
    sprintf(buf,"%d",index+3);
    return buf;
  }
  *list_size=6;
  return NULL;
}

int warpringdmapxy[8] = {160,116,160,90,160,102,160,154};

static DIALOG warpring_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      64,   40,   193,  152,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
  { d_dummy_proc,      160,   48,     0,    8,   vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_text_proc,     80,   73,    48,    8,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Count:" },
  { d_wclist_proc,     136,   69,    48,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) wclist },
  // 4
  { d_warplist_proc,    80,   90,    65,   71,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *) numberlist, NULL, warpringdmapxy },
  { jwin_button_proc,   90,  167,    61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,  170,  167,    61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_keyboard_proc,     0,    0,     0,    0,        0,    0,    0,       0,       KEY_F1,           0,       (void *) onHelp },
  { NULL }
};

int select_warp()
{
  misc.warp[curr_ring].size = vbound(misc.warp[curr_ring].size,3,8);
  number_list_zero = false;

  int ret=4;
  do
  {
    number_list_size = misc.warp[curr_ring].size;
    warpring_dlg[3].d1 = misc.warp[curr_ring].size-3;
    ret = zc_popup_dialog(warpring_dlg,ret);
  } while(ret==3);

  if(ret==6 || ret==0)
  {
    return -1;
  }
  return warpring_dlg[4].d1;
}

void EditWarpRing(int ring)
{
  char buf[40];
  sprintf(buf,"Ring %d Warps",ring);
  warpring_dlg[0].dp = buf;
  warpring_dlg[0].dp2 = lfont;
  curr_ring = ring;

  int index = select_warp();

  while(index!=-1)
  {
    EditWarpRingScr(ring,index);
    index = select_warp();
  }
}

int onWarpRings()
{
  number_list_size = 8;
  number_list_zero = true;

  int index = select_data("Warp Rings",0,numberlist,"Edit","Done",lfont);

  while(index!=-1)
  {
    EditWarpRing(index);
    number_list_size = 8;
    number_list_zero = true;
    index = select_data("Warp Rings",index,numberlist,"Edit","Done",lfont);
  }

  return D_O_K;
}

/********************************/
/********** onEnemies ***********/
/********************************/


char *pattern_list(int index, int *list_size)
{

  if(index<0)
  {
    *list_size = MAXPATTERNS;
    return NULL;
  }
  return pattern_string[index];
}

static DIALOG pattern_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 72,   64,   176+1,  88+1,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Enemy Pattern" },
  { d_dummy_proc,      160,  72,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_list_proc,       80,   94,   160+1,  24,   jwin_pal[jcTEXTFG],jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       (void *) pattern_list },
  // 3
  { jwin_button_proc,     90,   128,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  128,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int onPattern()
{
  byte p=Map.CurrScr()->pattern;
  pattern_dlg[0].dp2 = lfont;
  pattern_dlg[2].d1  = p;
  if(zc_popup_dialog(pattern_dlg,2) < 4)
  {
    saved=false;
    Map.CurrScr()->pattern = pattern_dlg[2].d1;
  }
  refresh(rMENU);
  return D_O_K;
}

static DIALOG enemyflags_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     72,   48,   176+1,  128+8+1,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Enemy Flags" },
  { d_dummy_proc,      160,  56,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  // 2
  { jwin_check_proc,      82+30,   72+2,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Zora" },
  { jwin_check_proc,      82+30,   80+2+2,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Corner traps" },
  { jwin_check_proc,      82+30,   88+4+2,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Center traps" },
  { jwin_check_proc,      82+30,   96+6+2,   160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Falling rocks" },
  { jwin_check_proc,      82+30,   104+8+2,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Statues shoot fire" },
  { jwin_check_proc,      82+30,   112+10+2,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Has ring leader" },
  { jwin_check_proc,      82+30,   120+12+2,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enemy carries item" },
  { jwin_check_proc,      82+30,   128+14+2,  160+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Dungeon Boss" },
  // 10
  { jwin_button_proc,     90,   144+16,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  144+16,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};

int onEnemyFlags()
{
  restore_mouse();
  enemyflags_dlg[0].dp2=lfont;
  byte f=Map.CurrScr()->enemyflags;
  for(int i=0; i<=7; i++)
  {
    enemyflags_dlg[i+2].flags = f&1?D_SELECTED:0;
    f>>=1;
  }
  if(zc_popup_dialog(enemyflags_dlg,-1)==10)
  {
    saved=false;
    f=0;
    for(int i=7; i>=0; i--)
    {
      f<<=1;
      f |= enemyflags_dlg[i+2].flags==D_SELECTED ? 1:0;
    }
    Map.CurrScr()->enemyflags=f;
  }
  refresh(rMENU);
  return D_O_K;
}

char *enemy_viewer(int index, int *list_size)
{
  if(index<0)
  {
    *list_size=10;

    return NULL;
  }
  int guy=Map.CurrScr()->enemy[index];
  return guy>=eOCTO1S ? guy_string[guy] : (char *) "(None)";
}

/*
  typedef struct enemy_struct {
  char *s;
  int i;
  } enemy_struct;
  */
enemy_struct bie[eMAXGUYS];
enemy_struct ce[100];
int enemy_type=0,bie_cnt=-1,ce_cnt;

enemy_struct big[zqMAXGUYS];
enemy_struct cg[100];
int guy_type=0,big_cnt=-1,cg_cnt;

void build_bie_list(bool hide)
{
  bie[0].s = "(None)";
  bie[0].i = 0;
  bie_cnt=1;

  for(int i=eOCTO1S; i<eMAXGUYS; i++)
  {
    if(guy_string[i][strlen(guy_string[i])-1]!=' ' || !hide)
    {
      bie[bie_cnt].s = guy_string[i];
      bie[bie_cnt].i = i;
      ++bie_cnt;
    }
  }

  for(int i=0; i<bie_cnt-1; i++)
  {
    for(int j=i+1; j<bie_cnt; j++)
    {
      if(strcmp(bie[i].s,bie[j].s)>0)
      {
        swap(bie[i],bie[j]);
      }
    }
  }
}

void build_big_list(bool hide)
{
  big[0].s = "(None)";
  big[0].i = 0;
  big_cnt=1;

  for(int i=gABEI; i<gDUMMY1; i++)
  {
    if(guy_string[i][strlen(guy_string[i])-1]!=' ' || !hide)
    {
      big[big_cnt].s = guy_string[i];
      big[big_cnt].i = i;
      ++big_cnt;
    }
  }

  for(int i=0; i<big_cnt-1; i++)
  {
    for(int j=i+1; j<big_cnt; j++)
    {
      if(strcmp(big[i].s,big[j].s)>0)
      {
        swap(big[i],big[j]);
      }
    }
  }
}

char *enemylist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = enemy_type ? ce_cnt : bie_cnt;
    return NULL;
  }
  return enemy_type ? ce[index].s : bie[index].s;
}

char *guylist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = guy_type ? cg_cnt : big_cnt;
    return NULL;
  }
  return guy_type ? cg[index].s : big[index].s;
}

static DIALOG elist_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     50,   40,   220,  145,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_abclist_proc,    62,   68,   196,  88,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   160,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  160,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

static DIALOG glist_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     50,   40,   220,  145,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_abclist_proc,    62,   68,   196,  88,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,       NULL },
  { jwin_button_proc,     90,   160,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  160,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int select_enemy(char *prompt,int enemy)
{
  //  if(bie_cnt==-1)
  {
    build_bie_list(true);
  }

  int index=0;

  for(int j=0; j<bie_cnt; j++)
  {
    if(bie[j].i == enemy)
    {
      index=j;
    }
  }

  elist_dlg[0].dp=prompt;
  elist_dlg[0].dp2=lfont;
  elist_dlg[2].d1=index;
  elist_dlg[2].dp=(void *) enemylist;

  int ret=zc_popup_dialog(elist_dlg,2);
  if(ret==0||ret==4)
  {
    return -1;
  }


  index = elist_dlg[2].d1;
  return bie[index].i;
}

int select_guy(char *prompt,int guy)
{
  //  if(bie_cnt==-1)
  {
    build_big_list(true);
  }

  int index=0;

  for(int j=0; j<big_cnt; j++)
  {
    if(big[j].i == guy)
    {
      index=j;
    }
  }

  glist_dlg[0].dp=prompt;
  glist_dlg[0].dp2=lfont;
  glist_dlg[2].d1=index;
  glist_dlg[2].dp=(void *) guylist;

  int ret=zc_popup_dialog(glist_dlg,2);
  if(ret==0||ret==4)
  {
    return -1;
  }


  index = glist_dlg[2].d1;
  return big[index].i;
}

unsigned char check[2] = { '\x81',0 };

static DIALOG enemy_dlg[] =
{
  /* (dialog proc)     (x)   (y)    (w)   (h)    (fg)                (bg)                   (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      40,   32,   240,  177,   vc(14),              vc(1),                 0,       D_EXIT,     0,             0,       (void *) "Enemies" },
  { d_dummy_proc,      160,   40,     0,    8,   vc(15),              vc(1),                 0,       0,          0,             0,       NULL },
  { jwin_list_proc,     52,   56,   216,   88,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],    0,       D_EXIT,     0,             0,       (void *) enemy_viewer },
  { jwin_button_proc,   52,  152,   109,   21,   vc(14),              vc(1),               'e',       D_EXIT,     0,             0,       (void *) "Paste &Enemies" },
  { d_dummy_proc,      188,  156,    64,    8,   vc(11),              vc(1),                 0,       0,          0,             0,       NULL },
  { jwin_button_proc,  167,  152,    42,   21,   vc(14),              vc(1),               'f',       D_EXIT,     0,             0,       (void *) "&Flags" },
  { jwin_button_proc,  215,  152,    53,   21,   vc(14),              vc(1),               'p',       D_EXIT,     0,             0,       (void *) "&Pattern" },
  { d_keyboard_proc,     0,    0,     0,    0,   0,                   0,                   'c',       0,          0,             0,       (void *) close_dlg },
  { d_keyboard_proc,     0,    0,     0,    0,   0,                   0,                   'v',       0,          0,             0,       (void *) close_dlg },
  { d_keyboard_proc,     0,    0,     0,    0,   0,                   0,                     0,       0,          KEY_DEL,       0,       (void *) close_dlg },
  // 10
  { jwin_button_proc,   90,  178,    61,   21,   vc(14),              vc(1),               13,        D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,  170,  178,    61,   21,   vc(14),              vc(1),               27,        D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,     0,    0,     0,    0,   0,                   0,                   27,        0,          0,             0,       (void *) close_dlg },
  { jwin_text_proc,     44,  240,     8,    8,   vc(14),              vc(1),               0,         0,          0,             0,       (void *) check },
  { d_keyboard_proc,     0,    0,     0,    0,   0,                   0,                   0,         0,          KEY_F1,        0,       (void *) onHelp },

  { NULL }
};

int onEnemies()
{
  word oldenemy[10];
  memcpy(oldenemy,Map.CurrScr()->enemy,10*sizeof(word));
  restore_mouse();
  char buf[24] = " ";
  int ret;
  int copy=-1;

  enemy_dlg[0].dp2=lfont;
  if(Map.CanPaste())
  {
    enemy_dlg[3].flags=D_EXIT;
    sprintf(buf,"Past&e (from %d:%02X)",(Map.CopyScr()>>8)+1,Map.CopyScr()&255);
  }
  else
  {
    enemy_dlg[3].flags=D_DISABLED;
    sprintf(buf,"Past&e from screen");
  }
  enemy_dlg[3].dp=buf;
  enemy_dlg[2].d1=0;

  do

  {
    if(copy==-1)
    {
      enemy_dlg[13].y=240;
    }
    else
    {
      enemy_dlg[13].y=(copy<<3)+58;
    }
    ret = zc_do_dialog(enemy_dlg,2);
    switch(ret)
    {
      case 2:
      {
        int i = enemy_dlg[2].d1;
        int enemy = Map.CurrScr()->enemy[i];
        enemy = select_enemy("Select Enemy",enemy);
        if(enemy>=0)
        {
          saved=false;
          Map.CurrScr()->enemy[i] = enemy;
        }
      } break;

      case 3:
      saved=false;
      Map.PasteEnemies();
      break;
      case 5:
      onEnemyFlags();
      break;
      case 6:
      onPattern();
      break;
      case 7: copy = enemy_dlg[2].d1; break;
      case 8:
      saved=false;
      if(copy>=0)
        Map.CurrScr()->enemy[enemy_dlg[2].d1] = Map.CurrScr()->enemy[copy];
      break;
      case 9:
      saved=false;
      Map.CurrScr()->enemy[enemy_dlg[2].d1] = 0;
      break;
      case 0:
      case 11: //cancel
      memcpy(Map.CurrScr()->enemy,oldenemy,10*sizeof(word));
      break;
    }
  } while(ret<10&&ret!=0);
  refresh(rALL);
  return D_O_K;
}

/*******************************/
/********** onHeader ***********/
/*******************************/

char author[65],title[65],password[32];

int d_showedit_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_edit_proc(msg,d,c);
  if(msg==MSG_DRAW)
  {
    scare_mouse();
    (d+1)->proc(MSG_DRAW,d+1,0);
    unscare_mouse();
  }

  return ret;
}

static DIALOG header_dlg[] =
{
  /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)                (bg)              (key)    (flags)       (d1)           (d2)     (dp) */
  { jwin_win_proc,        20,   16,   280,  199,  vc(14),             vc(1),            0,       D_EXIT,        0,             0,       (void *) "Quest Header" },
  { d_dummy_proc,         0,    0,     0,     8,  vc(15),             vc(1),            0,       0,             0,             0,       NULL },
  { jwin_text_proc,       26,   40,    96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "ZQ Version:" },
  { jwin_text_proc,       103,  40,    96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_DISABLED,    0,             0,       NULL },
  { jwin_text_proc,       26,   71-16,    96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "Quest Number:" },
  { jwin_edit_proc,       100,  68-16,    32,   16,  vc(12),             vc(1),            0,       0,             2,             0,       NULL },
  { jwin_text_proc,       26,   87-16,    96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "Quest Ver:" },
  { jwin_edit_proc,       100,  84-16,    80,   16,  vc(12),             vc(1),            0,       0,             8,             0,       NULL },
  { jwin_text_proc,       26,   103-16,   96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "Min. Ver:" },
  { jwin_edit_proc,       100,  100-16,   80,   16,  vc(12),             vc(1),            0,       0,             8,             0,       NULL },
  // 10
  { jwin_text_proc,       26,   119-16,   96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "Title:" },
  { d_showedit_proc,      66,   116-16,   80,   16,  vc(12),             vc(1),            0,       0,            64,             0,       title },
  { jwin_textbox_proc,    26,   132-16,   120,  40,  vc(11),  vc(1),  0,       0,          64,            0,       title },
  { jwin_text_proc,       174,  119-16,   96,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Author:" },
  { d_showedit_proc,      214,  116-16,   80,   16,    vc(12),  vc(1),  0,       0,          64,            0,       author },
  { jwin_textbox_proc,    174,  132-16,   120,  40,   vc(11),  vc(1),  0,       0,          64,            0,       author },
  // 16
  { jwin_button_proc,     110,  198-32,  101,   21,   vc(14),             vc(1),            13,      D_EXIT,        0,             0,       (void *) "Change Password" },
  { jwin_button_proc,     90,   198-8,  61,   21,   vc(14),             vc(1),            13,      D_EXIT,        0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  198-8,  61,   21,   vc(14),             vc(1),            27,      D_EXIT,        0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,      0,    0,    0,    0,    0,                  0,                0,       0,             KEY_F1,        0,       (void *) onHelp },
  { NULL }
};

static DIALOG password_dlg[] =
{
  /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)                (bg)              (key)    (flags)       (d1)           (d2)     (dp) */
  { jwin_win_proc,        0,    0,   300,  111-32,  vc(14),             vc(1),            0,       D_EXIT,        0,             0,       (void *) "Set Password" },
  { d_dummy_proc,         0,    0,     0,     8,  vc(15),             vc(1),            0,       0,             0,             0,       NULL },
  { jwin_text_proc,       6,   30,   96,    8,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,             0,             0,       (void *) "Enter new password:" },
  { jwin_edit_proc,       104,  26,   190,   16,  vc(12),             vc(1),            0,       0,             255,             0,       NULL },
  { jwin_check_proc,      6,   42,   128+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Save key file" },
  { jwin_button_proc,     80,   54,  61,   21,   vc(14),             vc(1),            13,      D_EXIT,        0,             0,       (void *) "OK" },
  { jwin_button_proc,    160,   54,  61,   21,   vc(14),             vc(1),            27,      D_EXIT,        0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,      0,    0,    0,    0,    0,                  0,                0,       0,             KEY_F1,        0,       (void *) onHelp },
  { NULL }
};



int onHeader()
{
  char zver_str[10],q_num[8],version[10],minver[10];

  sprintf(zver_str,"%d.%02X (%d)",header.zelda_version>>8,header.zelda_version&0xFF,header.build);
  sprintf(q_num,"%d",header.quest_number);
  strcpy(version,header.version);
  strcpy(minver,header.minver);
  strcpy(author,header.author);
  strcpy(title,header.title);

  header_dlg[0].dp2 = lfont;
  header_dlg[3].dp = zver_str;
  header_dlg[5].dp = q_num;
  header_dlg[7].dp = version;
  header_dlg[9].dp = minver;

  password_dlg[0].dp2=lfont;
  char pwd[256];
  memset(pwd,0,256);
  password_dlg[3].dp=pwd;
  password_dlg[4].flags=header.use_keyfile?D_SELECTED:0;

  int ret;
  do
  {
    ret=zc_popup_dialog(header_dlg,-1);
    if (ret==16)
    {
      ret=zc_popup_dialog(password_dlg,-1);
      if (ret==5)
      {
        header.use_keyfile=password_dlg[4].flags&D_SELECTED?1:0;
        set_questpwd(pwd,header.use_keyfile!=0);
      }
      ret=16;
    }
  } while (ret==16);



  if(ret==17)
  {
    saved=false;
    header.quest_number=atoi(q_num);
    strcpy(header.author,author);
    strcpy(header.title,title);
    strcpy(header.version,version);
    strcpy(header.minver,minver);
  }
  return D_O_K;
}


static ZCHEATS tmpcheats;


static DIALOG cheats_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      32,   44,   256,  142,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Cheat Codes" },
  { d_dummy_proc,       160,  65,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  // 2
  { jwin_button_proc,     90,   160,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  160,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,       0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  // 5
  { jwin_check_proc,      104,  72,   0,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Enable Cheats" },
  { jwin_text_proc,       40,   72,   0,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Lvl  Code" },
  { jwin_text_proc,       48,   90,   8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "1" },
  { jwin_text_proc,       48,   108,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "2" },
  { jwin_text_proc,       48,   126,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "3" },
  { jwin_text_proc,       48,   144,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "4" },
  // 11
  { jwin_edit_proc,       61,   86,   210,  16,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[0] },
  { jwin_edit_proc,       61,   104,  210,  16,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[1] },
  { jwin_edit_proc,       61,   122,  210,  16,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[2] },
  { jwin_edit_proc,       61,   140,  210,  16,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[3] },
  { NULL }
};

int onCheats()
{
  cheats_dlg[0].dp2=lfont;
  tmpcheats = zcheats;
  cheats_dlg[5].flags = zcheats.flags ? D_SELECTED : 0;

  int ret = zc_popup_dialog(cheats_dlg, 3);

  if(ret == 2)
  {
    saved = false;
    zcheats = tmpcheats;
    zcheats.flags = (cheats_dlg[5].flags & D_SELECTED) ? 1 : 0;
  }

  return D_O_K;
}

static int item_props_1_list[] =
{
  // dialog control number
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1
};

static int item_props_2_list[] =
{
  // dialog control number
  32, 33, 34, 35, 36, 37, 38, 39, -1
};

static int item_props_3_list[] =
{
  // dialog control number
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1
};

static TABPANEL item_props_tabs[] =
{
  // (text)
  { "Sword",       D_SELECTED,  item_props_1_list },
  { "Hookshot",       0,        item_props_2_list },
  { "Recovery",       0,        item_props_3_list },
  { NULL }
};


static DIALOG itemprop_dlg[] =
{
  /* (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        0,   0,   284,  167+3+8+7+17,    vc(14),   vc(1),      0,      D_EXIT,          0,             0,       (void *) "Item Properties" },
  { jwin_tab_proc,        4,   24,  276,  151-5,   0,       0,      0,       0,             0,       0,       (void *) item_props_tabs, NULL, (void *)itemprop_dlg },
  { jwin_button_proc,    71,  175,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   152,  175,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,      0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  // 5
  { jwin_ctext_proc,    106,   45,   40,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Sword" },
  { jwin_ctext_proc,    106,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Hearts" },
  { jwin_text_proc,      14,   69,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wooden: " },
  { jwin_text_proc,      14,   87,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "White: " },
  { jwin_text_proc,      14,  105,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Magic: " },
  { jwin_text_proc,      14,  123,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Master: " },
  { jwin_edit_proc,      95,   65,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      95,   83,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      95,  101,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      95,  119,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 15
  { jwin_ctext_proc,    166,   45,   40,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Beam" },
  { jwin_ctext_proc,    166,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Hearts" },
  { jwin_edit_proc,     153,   65,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,   83,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,  101,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,  119,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 21
  { jwin_ctext_proc,    190,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "%" },
  { jwin_check_proc,    185,   69,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,   87,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,  105,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,  123,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  // 26
  { jwin_ctext_proc,    240,   45,   40,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Beam" },
  { jwin_ctext_proc,    240,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Power %" },
  { jwin_edit_proc,     227,   65,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     227,   83,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     227,  101,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     227,  119,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 32
  { jwin_ctext_proc,    106,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Length" },
  { jwin_text_proc,      14,   69,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Hookshot: " },
  { jwin_text_proc,      14,   87,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Longshot: " },
  { jwin_edit_proc,      95,   65,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      95,   83,   21,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  // 37
  { jwin_ctext_proc,    166,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Links" },
  { jwin_edit_proc,     153,   65,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,   83,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 40
  { jwin_ctext_proc,    106,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Hearts" },
  { jwin_text_proc,      14,   69,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Moving Fairy: " },
  { jwin_text_proc,      14,   87,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Stationary Fairy: " },
  { jwin_text_proc,      14,  105,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Blue Potion: " },
  { jwin_text_proc,      14,  123,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Red Potion: " },
  { jwin_edit_proc,      93,   65,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,      93,   83,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,      93,  101,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,      93,  119,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 49
  { jwin_ctext_proc,    130,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "%" },
  { jwin_check_proc,    125,   69,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    125,   87,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    125,  105,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    125,  123,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  // 54
  { jwin_ctext_proc,    166,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Magic" },
  { jwin_edit_proc,     153,   65,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,   83,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,  101,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,     153,  119,   26,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  // 59
  { jwin_ctext_proc,    190,   55,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "%" },
  { jwin_check_proc,    185,   69,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,   87,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,  105,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { jwin_check_proc,    185,  123,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL },
  { NULL }
};

int onItemProps()
{
  itemprop_dlg[0].dp2=lfont;

  char swordheartstring[4][5];
  char beamheartstring[4][5];
  char beampowerstring[4][5];
  char hookshotlinksstring[2][5];
  char hookshotlengthstring[2][5];
  char fairyheartsstring[2][5];
  char potionheartsstring[2][5];
  char fairymagicstring[2][5];
  char potionmagicstring[2][5];

  for (int x=0; x<4; x++)
  {
    sprintf(swordheartstring[x], "%d", zinit.sword_hearts[x]);
  }
  for (int x=0; x<4; x++)
  {
    sprintf(beamheartstring[x], "%d", zinit.beam_hearts[x]);
  }
  for (int x=0; x<4; x++)
  {
    sprintf(beampowerstring[x], "%d", zinit.beam_power[x]);
  }
  sprintf(hookshotlinksstring[0], "%d", zinit.hookshot_links);
  sprintf(hookshotlinksstring[1], "%d", zinit.longshot_links);
  sprintf(hookshotlengthstring[0], "%d", zinit.hookshot_length);
  sprintf(hookshotlengthstring[1], "%d", zinit.longshot_length);
  sprintf(fairyheartsstring[0], "%d", zinit.moving_fairy_hearts);
  sprintf(fairyheartsstring[1], "%d", zinit.stationary_fairy_hearts);
  sprintf(potionheartsstring[0], "%d", zinit.blue_potion_hearts);
  sprintf(potionheartsstring[1], "%d", zinit.red_potion_hearts);
  sprintf(fairymagicstring[0], "%d", zinit.moving_fairy_magic);
  sprintf(fairymagicstring[1], "%d", zinit.stationary_fairy_magic);
  sprintf(potionmagicstring[0], "%d", zinit.blue_potion_magic);
  sprintf(potionmagicstring[1], "%d", zinit.red_potion_magic);


  for (int x=0; x<4; x++)
  {
    itemprop_dlg[11+x].dp=swordheartstring[x];
  }
  for (int x=0; x<4; x++)
  {
    itemprop_dlg[17+x].dp=beamheartstring[x];
  }
  for(int x=0; x<idBP_MAX; x++)
  {
    itemprop_dlg[22+x].flags = get_bit(&(zinit.beam_percent),x) ? D_SELECTED : 0;
  }
  for (int x=0; x<4; x++)
  {
    itemprop_dlg[28+x].dp=beampowerstring[x];
  }
  itemprop_dlg[35].dp=hookshotlengthstring[0];
  itemprop_dlg[36].dp=hookshotlengthstring[1];
  itemprop_dlg[38].dp=hookshotlinksstring[0];
  itemprop_dlg[39].dp=hookshotlinksstring[1];

  itemprop_dlg[45].dp=fairyheartsstring[0];
  itemprop_dlg[46].dp=fairyheartsstring[1];
  itemprop_dlg[47].dp=potionheartsstring[0];
  itemprop_dlg[48].dp=potionheartsstring[1];
  itemprop_dlg[50].flags=zinit.moving_fairy_heart_percent ? D_SELECTED : 0;
  itemprop_dlg[51].flags=zinit.stationary_fairy_heart_percent ? D_SELECTED : 0;
  itemprop_dlg[52].flags=zinit.blue_potion_heart_percent ? D_SELECTED : 0;
  itemprop_dlg[53].flags=zinit.red_potion_heart_percent ? D_SELECTED : 0;

  itemprop_dlg[55].dp=fairymagicstring[0];
  itemprop_dlg[56].dp=fairymagicstring[1];
  itemprop_dlg[57].dp=potionmagicstring[0];
  itemprop_dlg[58].dp=potionmagicstring[1];
  itemprop_dlg[60].flags=zinit.moving_fairy_magic_percent ? D_SELECTED : 0;
  itemprop_dlg[61].flags=zinit.stationary_fairy_magic_percent ? D_SELECTED : 0;
  itemprop_dlg[62].flags=zinit.blue_potion_magic_percent ? D_SELECTED : 0;
  itemprop_dlg[63].flags=zinit.red_potion_magic_percent ? D_SELECTED : 0;

  int ret = zc_popup_dialog(itemprop_dlg,3);

  if (ret==2)
  {
    for (int x=0; x<4; x++)
    {
      zinit.sword_hearts[x]=atoi(swordheartstring[x]);
    }
    for (int x=0; x<4; x++)
    {
      zinit.beam_hearts[x]=atoi(beamheartstring[x]);
    }
    for(int x=0; x<idBP_MAX; x++)
    {
      set_bit(&(zinit.beam_percent),x,itemprop_dlg[22+x].flags);
    }
    for (int x=0; x<4; x++)
    {
      zinit.beam_power[x]=atoi(beampowerstring[x]);
    }
    zinit.hookshot_links=atoi(hookshotlinksstring[0]);
    zinit.longshot_links=atoi(hookshotlinksstring[1]);
    zinit.hookshot_length=atoi(hookshotlengthstring[0]);
    zinit.longshot_length=atoi(hookshotlengthstring[1]);
    zinit.moving_fairy_hearts=atoi(fairyheartsstring[0]);
    zinit.stationary_fairy_hearts=atoi(fairyheartsstring[1]);
    zinit.blue_potion_hearts=atoi(potionheartsstring[0]);
    zinit.red_potion_hearts=atoi(potionheartsstring[1]);
    zinit.moving_fairy_magic=atoi(fairymagicstring[0]);
    zinit.stationary_fairy_magic=atoi(fairymagicstring[1]);
    zinit.blue_potion_magic=atoi(potionmagicstring[0]);
    zinit.red_potion_magic=atoi(potionmagicstring[1]);
    zinit.moving_fairy_heart_percent=itemprop_dlg[50].flags&D_SELECTED ? 1 : 0;
    zinit.stationary_fairy_heart_percent=itemprop_dlg[51].flags&D_SELECTED ? 1 : 0;
    zinit.blue_potion_heart_percent=itemprop_dlg[52].flags&D_SELECTED ? 1 : 0;
    zinit.red_potion_heart_percent=itemprop_dlg[53].flags&D_SELECTED ? 1 : 0;
    zinit.moving_fairy_magic_percent=itemprop_dlg[60].flags&D_SELECTED ? 1 : 0;
    zinit.stationary_fairy_magic_percent=itemprop_dlg[61].flags&D_SELECTED ? 1 : 0;
    zinit.blue_potion_magic_percent=itemprop_dlg[62].flags&D_SELECTED ? 1 : 0;
    zinit.red_potion_magic_percent=itemprop_dlg[63].flags&D_SELECTED ? 1 : 0;
    saved=false;
  }
  return D_O_K;
}

char *subscrtype_str[ssdtMAX+1] = { "Original","New Subscreen","Revision 2","BS Zelda Original","BS Zelda Modified","BS Zelda Enhanced","BS Zelda Complete","Zelda 3","Custom" };

char *subscrtypelist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,ssdtMAX);
    return subscrtype_str[index];
  }
  *list_size=ssdtMAX+1;
  return NULL;
}

static DIALOG subscreen_type_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     83,   32,   154,  70,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Subscreen Type" },
  { jwin_button_proc,     89,  77,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  77,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_droplist_proc,   107-8,  57,   106+15,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) subscrtypelist },
  { NULL }
};

int onSubscreen()
{
  int tempsubscreen=zinit.subscreen;
  subscreen_type_dlg[0].dp2=lfont;
  subscreen_type_dlg[3].d1=zinit.subscreen;
  int ret = zc_popup_dialog(subscreen_type_dlg,2);
  if (ret==1)
  {
    if (subscreen_type_dlg[3].d1!=tempsubscreen)
    {
      zinit.subscreen=subscreen_type_dlg[3].d1;
      if (zinit.subscreen!=ssdtMAX) //custom
      {
        if (tempsubscreen==ssdtMAX)
        {
          if(jwin_alert("Reset Custom Susbscreens","This will delete all of your custom subscreens!","Proceed?",NULL,"&OK","&Cancel",13,27,lfont)==2)
          {
            return D_O_K;
          }
        }
        reset_subscreens();
        setupsubscreens();
      }
      saved=false;
    }
  }
  return D_O_K;
}


// custom items and guys
#include "zq_custom.h"
bool do_x_button(BITMAP *dest, int x, int y)
{
  bool over=false;
  while(gui_mouse_b())
  {
    custom_vsync();
    if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+15,y+13))
    {
      if(!over)
      {
        scare_mouse();
        draw_x_button(screen, SCREEN_W - 21, 5, D_SELECTED);
        unscare_mouse();
        over=true;
      }
    }
    else
    {
      if(over)
      {
        scare_mouse();
        draw_x_button(screen, SCREEN_W - 21, 5, 0);
        unscare_mouse();
        over=false;
      }
    }
  }
  return over;
}


int d_dummy_proc(int msg,DIALOG *d,int c) { return D_O_K; }

int d_maptile_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_CLICK:
    if(select_tile(d->d1,d->d2,1,d->fg,true))
      return D_REDRAW;
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,d->w,d->h);
    if(buf)
    {
      clear_bitmap(buf);
      for(int y=0; y<d->h; y+=16)
        for(int x=0; x<d->w; x+=16)
        {
          if(d->d1)
            puttile16(buf,d->d1+(y>>4)*20+(x>>4),x,y,d->fg,0);
        }
      blit(buf,screen,0,0,d->x,d->y,d->w,d->h);
      destroy_bitmap(buf);
    }
  }
  return D_O_K;
}

static int last_combo=0;
static int last_cset=0;
static combo_alias temp_aliases[MAXCOMBOALIASES];

static char comboa_str_buf[32];

int d_comboalist_proc(int msg,DIALOG *d,int c)
{
  int d1 = d->d1;
  int ret = jwin_droplist_proc(msg,d,c);
  comboa_cnt = d->d1;
  if (d1!=d->d1)
  {
    set_comboaradio(temp_aliases[comboa_cnt].layermask);
    return D_REDRAW;
  }

  return ret;
}

char *comboalist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,255);
    sprintf(comboa_str_buf,"%d",index);
    return comboa_str_buf;
  }
  *list_size=MAXCOMBOALIASES;
  return NULL;
}

extern int scheme[jcMAX];

int d_comboa_proc(int msg,DIALOG *d,int c)
{
  combo_alias *combo;
  combo = &temp_aliases[comboa_cnt];
  int position;
  int cur_layer, temp_layer;
  int lay_count=0;

  int cx1=(gui_mouse_x()-d->x-(120-(combo->width*8)));
  int cy1=(gui_mouse_y()-d->y-(80-(combo->height*8)));
  int cx=cx1/16;
  int cy=cy1/16;

  int co,cs;

  switch(msg)
  {
    case MSG_CLICK:
    Z_message("click (%d, %d) (%d, %d)\n", cx1, cy1, cx, cy);
    if ((cx>combo->width)||(cx1<0))
      return D_O_K;
    if ((cy>combo->height)||(cy1<0))
      return D_O_K;

    for(int j=0;j<layer_cnt;j++)
    {
      if(combo->layermask&(1<<j))
        lay_count++;
    }

    position=(lay_count)*(combo->width+1)*(combo->height+1);
    position+=(cy*(combo->width+1))+cx;

    if(key[KEY_LSHIFT])
    {
      combo->combos[position] = 0;
      combo->csets[position] = 0;
      while (gui_mouse_b());
      return D_REDRAW;
    }

    co=combo->combos[position];
    cs=combo->csets[position];

    if((co==0)||(key[KEY_ZC_LCONTROL]))
    {
      co=last_combo;
      cs=last_cset;
    }
    if((select_combo_2(co,cs)))
    {
      last_combo = co;
      last_cset = cs;

      combo->combos[position]=co;
      combo->csets[position]=cs;
    }
    return D_REDRAW;
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,d->w,d->h);
    if(buf)
    {
      clear_bitmap(buf);
      for(int z=0; z<=comboa_lmasktotal(combo->layermask);z++)
      {
        int k=0;
        cur_layer=0;
        temp_layer=combo->layermask;
        while((temp_layer!=0)&&(k<z))
        {
          if(temp_layer&1)
          {
            k++;
          }
          cur_layer++;
          temp_layer = temp_layer>>1;
        }
        for(int y=0; (y<d->h)&&((y/16)<=combo->height); y+=16)
        {
          for(int x=0; (x<d->w)&&((x/16)<=combo->width); x+=16)
          {
            int cpos = (z*(combo->width+1)*(combo->height+1))+(((y/16)*(combo->width+1))+(x/16));
            if(combo->combos[cpos])
            {
              if(!((d-1)->flags&D_SELECTED)||(cur_layer==layer_cnt))
              {
                if(z==0)
                {
                  puttile16(buf,combobuf[combo->combos[cpos]].tile,x,y,combo->csets[cpos],combobuf[combo->combos[cpos]].flip);
                }
                else
                {
                  overtile16(buf,combobuf[combo->combos[cpos]].tile,x,y,combo->csets[cpos],combobuf[combo->combos[cpos]].flip);
                }
              }
            }
          }
        }
      }
      rectfill(screen, d->x-2,d->y-2,d->x+256+2,d->y+176+2,jwin_pal[jcBOX]);
      blit(buf,screen,0,0,120-(combo->width*8)+d->x,80-(combo->height*8)+d->y,(combo->width+1)*16,(combo->height+1)*16);
      (d-11)->w = (combo->width+1)*16+2+2;
      (d-11)->h = (combo->height+1)*16+2+2;
      (d-11)->x = 120-(combo->width*8)+4;
      (d-11)->y = 80-(combo->height*8)+25;
      object_message((d-11),MSG_DRAW,0);

      destroy_bitmap(buf);
    }
    break;
  }
  return D_O_K;
}

void draw_combo_alias_thumbnail(BITMAP *dest, combo_alias *combo, int x, int y)
{
  if (!combo->combo)
  {
    int cur_layer, temp_layer;

    int cw=combo->width+1;
    int ch=combo->height+1;
    int dw=cw<<4;
    int dh=ch<<4;
    int sw=16, sh=16, sx=0, sy=0;

    if (cw<ch)
    {
      sw=(cw<<4)/ch;
      sx=(16-sw)>>1;
    }
    else
    {
      sh=(ch<<4)/cw;
      sy=(16-sh)>>1;
    }
    BITMAP *buf = create_bitmap_ex(8,dw,dh);
    BITMAP *buf2 = create_bitmap_ex(8, 16, 16);
    clear_bitmap(buf);
    clear_bitmap(buf2);
    if(buf&&(combo->width>0||combo->height>0||combo->combos[0]>0))
    {
      clear_bitmap(buf);
      for(int z=0; z<=comboa_lmasktotal(combo->layermask);z++)
      {
        int k=0;
        cur_layer=0;
        temp_layer=combo->layermask;
        while((temp_layer!=0)&&(k<z))
        {
          if(temp_layer&1)
          {
            k++;
          }
          cur_layer++;
          temp_layer = temp_layer>>1;
        }
        for(int y=0; (y<dh)&&((y>>4)<=combo->height); y+=16)
        {
          for(int x=0; (x<dw)&&((x>>4)<=combo->width); x+=16)
          {
            int cpos = (z*(combo->width+1)*(combo->height+1))+(((y/16)*(combo->width+1))+(x/16));
            if(combo->combos[cpos])
            {
              if(z==0)
              {
                puttile16(buf,combobuf[combo->combos[cpos]].tile,x,y,combo->csets[cpos],combobuf[combo->combos[cpos]].flip);
              }
              else
              {
                overtile16(buf,combobuf[combo->combos[cpos]].tile,x,y,combo->csets[cpos],combobuf[combo->combos[cpos]].flip);
              }
            }
          }
        }
      }
      stretch_blit(buf, buf2, 0, 0, cw<<4, ch<<4, sx, sy, sw, sh);
      blit(buf2, dest, 0, 0, x, y, 16, 16);
      destroy_bitmap(buf);
      destroy_bitmap(buf2);
    }
    else
    {
      rectfill(dest,x,y,x+15,y+15,0);
      rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
    }
  }
  else
  {
    if (combobuf[combo->combo].tile>0)
    {
      put_combo(dest,x, y, combo->combo, combo->cset,0,0);
    }
    else
    {
      rectfill(dest,x,y,x+15,y+15,0);
      rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
    }
  }
}

int d_comboat_proc(int msg,DIALOG *d,int c)
{
  switch (msg)
  {
    case MSG_CLICK:
    {
      int c;
      int cs;
      c=temp_aliases[comboa_cnt].combo;
      cs=temp_aliases[comboa_cnt].cset;
      if (gui_mouse_b()&2) //right mouse button
      {
        if (c==0&&cs==0&&!(gui_mouse_b()&1))
        {
          return D_O_K;
        }
        temp_aliases[comboa_cnt].combo=0;
        temp_aliases[comboa_cnt].cset=0;
      }
      if (gui_mouse_b()&1) //left mouse button
      {
        if(select_combo_2(c, cs))
        {
          temp_aliases[comboa_cnt].combo=c;
          temp_aliases[comboa_cnt].cset=cs;
        }
        return D_REDRAW;
      }
      else
      {
        return D_REDRAWME;
      }
    }
    break;
    case MSG_DRAW:
    draw_combo_alias_thumbnail(screen, &temp_aliases[comboa_cnt], d->x, d->y);
    break;
    default:
    break;
  }
  return D_O_K;

  return d_combo_proc(msg,d,c);
}

int d_comboa_radio_proc(int msg,DIALOG *d,int c);

static DIALOG newcomboa_dlg[] =
{
  /* (dialog proc)     (x)   (y)    (w)   (h)    (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
  { jwin_win_proc,         0,    0,   200,  161,   vc(14),   vc(1),       0,     D_EXIT,       0,             0,        (void *) "Combo Alias Properties" },
  { jwin_button_proc,     27,   130,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     112,  130,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_text_proc,     24,   34,   80,   8,       0,       0,      0,       0,          0,             0,       (void *) "Alias Width"},
  { jwin_text_proc,     24,   52,   80,   8,       0,       0,      0,       0,          0,             0,       (void *) "Alias Height"},
  { jwin_text_proc,     24,   70,   100,   8,       0,       0,      0,       0,          0,             0,       (void *) "Layers to Draw On:"},
  { jwin_edit_proc,      104,   30,   28-6,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      122,   48,   28-6,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     24,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "1" },
  { jwin_check_proc,     50,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "2" },
  { jwin_check_proc,     76,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "3" },
  { jwin_check_proc,     102,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "4" },
  { jwin_check_proc,     128,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "5" },
  { jwin_check_proc,     154,   86,   24,   9,    vc(12),  vc(1),  0,       0,          1,             0,       (void *) "6" },
  {NULL}
};

int onNewComboAlias()
{
  combo_alias *combo;
  combo = &temp_aliases[comboa_cnt];

  char cwidth[3];
  char cheight[3];
  word temp_combos[16*11*7];
  byte temp_csets[16*11*7];
  sprintf(cwidth, "%d", combo->width+1);
  sprintf(cheight, "%d", combo->height+1);
  int old_count = (comboa_lmasktotal(combo->layermask)+1)*(combo->width+1)*(combo->height+1);
  int old_width=combo->width;
  int old_height=combo->height;
  int oldlayer=combo->layermask;
  for(int i=0;i<old_count;i++)
  {
    temp_csets[i] = combo->csets[i];
    temp_combos[i] = combo->combos[i];
  }
  newcomboa_dlg[0].dp2 = lfont;
  newcomboa_dlg[6].dp = cwidth;
  newcomboa_dlg[7].dp = cheight;
  newcomboa_dlg[8].flags = (combo->layermask&1)? D_SELECTED : 0;
  newcomboa_dlg[9].flags = (combo->layermask&2)? D_SELECTED : 0;
  newcomboa_dlg[10].flags = (combo->layermask&4)? D_SELECTED : 0;
  newcomboa_dlg[11].flags = (combo->layermask&8)? D_SELECTED : 0;
  newcomboa_dlg[12].flags = (combo->layermask&16)? D_SELECTED : 0;
  newcomboa_dlg[13].flags = (combo->layermask&32)? D_SELECTED : 0;

  int ret = zc_popup_dialog(newcomboa_dlg,-1);

  if(ret==1)
  {
    combo->width = ((atoi(cwidth)-1)<16)?max(0,(atoi(cwidth)-1)):15;
    combo->height = ((atoi(cheight)-1)<11)?max(0,(atoi(cheight)-1)):10;
    combo->layermask=0;
    combo->layermask |= (newcomboa_dlg[8].flags&D_SELECTED)?1:0;
    combo->layermask |= (newcomboa_dlg[9].flags&D_SELECTED)?2:0;
    combo->layermask |= (newcomboa_dlg[10].flags&D_SELECTED)?4:0;
    combo->layermask |= (newcomboa_dlg[11].flags&D_SELECTED)?8:0;
    combo->layermask |= (newcomboa_dlg[12].flags&D_SELECTED)?16:0;
    combo->layermask |= (newcomboa_dlg[13].flags&D_SELECTED)?32:0;

    int new_count = (comboa_lmasktotal(combo->layermask)+1)*(combo->width+1)*(combo->height+1);
    if(combo->combos != NULL)
    {
      delete[] combo->combos;
    }
    if(combo->csets != NULL)
    {
      delete[] combo->csets;
    }
    combo->combos = new word[new_count];
    combo->csets = new byte[new_count];

    int j=1;
    int old_size=(old_width+1)*(old_height+1);
    int new_start[7] =
    {
      0,
      ((combo->width+1)*(combo->height+1)*(1)),
      ((combo->width+1)*(combo->height+1)*(2)),
      ((combo->width+1)*(combo->height+1)*(3)),
      ((combo->width+1)*(combo->height+1)*(4)),
      ((combo->width+1)*(combo->height+1)*(5)),
      ((combo->width+1)*(combo->height+1)*(6))
    };
    int new_layers[6] = {0,0,0,0,0,0};
    int temp_layer = combo->layermask;
    int temp_old = oldlayer;
    int old_layers[6] = {0,0,0,0,0,0};
    int k=1;
    for(int i=0;(i<6)&&(temp_layer!=0);j++,temp_layer>>=1,temp_old>>=1)
    {
      if(temp_layer&1)
      {
        new_layers[i] = j;
        //if(oldlayer&(1<<(j-1))) old_layers[i] = k++;
        i++;
      }
      if(temp_old&1)
      {
        if(temp_layer&1)
        {
          old_layers[i-1] = k;
        }
        k++;
      }
    }

    for(int i=0;i<new_count;i++)
    {
      if(i>=new_start[6])
      {
        //oldl=oldlayer>>(new_layers[5]-1);
        j=i-new_start[6];
        if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[5]-1))))
        {
          combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[5])];
          combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[5])];
        }
        else
        {
          combo->combos[i] = 0;
          combo->csets[i] = 0;
        }
      }
      else if(i>=new_start[5])
        {
          //oldl=oldlayer>>(new_layers[4]-1);
          j=i-new_start[5];
          if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[4]-1))))
          {
            combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[4])];
            combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[4])];
          }
          else
          {
            combo->combos[i] = 0;
            combo->csets[i] = 0;
          }
        }
        else if(i>=new_start[4])
          {
            //oldl=oldlayer>>(new_layers[3]-1);
            j=i-new_start[4];
            if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[3]-1))))
            {
              combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[3])];
              combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[3])];
            }
            else
            {
              combo->combos[i] = 0;
              combo->csets[i] = 0;
            }
          }
          else if(i>=new_start[3])
            {
              //oldl=oldlayer>>(new_layers[2]-1);
              j=i-new_start[3];
              if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[2]-1))))
              {
                combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[2])];
                combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[2])];
              }
              else
              {
                combo->combos[i] = 0;
                combo->csets[i] = 0;
              }
            }
            else if(i>=new_start[2])
              {
                //oldl=oldlayer>>(new_layers[1]-1);
                j=i-new_start[2];
                if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[1]-1))))
                {
                  combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[1])];
                  combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[1])];
                }
                else
                {
                  combo->combos[i] = 0;
                  combo->csets[i] = 0;
                }
              }
              else if(i>=new_start[1])
                {
                  //oldl=oldlayer>>(new_layers[0]-1);
                  j=i-new_start[1];
                  if(((j/(combo->width+1))<=old_height)&&((j%(combo->width+1))<=old_width)&&(oldlayer&(1<<(new_layers[0]-1))))
                  {
                    combo->combos[i] = temp_combos[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[0])];
                    combo->csets[i] = temp_csets[((j%(combo->width+1))+((old_width+1)*(j/(combo->width+1))))+(old_size*old_layers[0])];
                  }
                  else
                  {
                    combo->combos[i] = 0;
                    combo->csets[i] = 0;
                  }
                }
                else if(i>=new_start[0])
                  {
                    if(((i/(combo->width+1))<=old_height)&&((i%(combo->width+1))<=old_width))
                    {
                      combo->combos[i] = temp_combos[(i%(combo->width+1))+((old_width+1)*(i/(combo->width+1)))];
                      combo->csets[i] = temp_csets[(i%(combo->width+1))+((old_width+1)*(i/(combo->width+1)))];
                    }
                    else
                    {
                      combo->combos[i] = 0;
                      combo->csets[i] = 0;
                    }
                  }
    }
    set_comboaradio(combo->layermask);
  }
  return ret;
}

int d_comboabutton_proc(int msg, DIALOG *d, int c)
{
  int down=0;
  int selected=(d->flags&D_SELECTED)?1:0;
  int last_draw;

  switch (msg)
  {

    case MSG_DRAW:
    jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
    break;

    case MSG_WANTFOCUS:
    return D_WANTFOCUS;

    case MSG_KEY:
    /* close dialog? */
    onNewComboAlias();
    return D_REDRAW;

    /* or just toggle */
    d->flags ^= D_SELECTED;
    scare_mouse();
    object_message(d, MSG_DRAW, 0);
    unscare_mouse();
    break;

    case MSG_CLICK:
    last_draw = 0;

    /* track the mouse until it is released */
    while (gui_mouse_b())
    {
      down = mouse_in_rect(d->x, d->y, d->w, d->h);

      /* redraw? */
      if (last_draw != down)
      {
        if(down != selected)
          d->flags |= D_SELECTED;
        else
          d->flags &= ~D_SELECTED;
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        unscare_mouse();
        last_draw = down;
      }

      /* let other objects continue to animate */
      broadcast_dialog_message(MSG_IDLE, 0);
    }

    /* redraw in normal state */
    if(down)
    {
      if(d->flags&D_EXIT)
      {
        d->flags &= ~D_SELECTED;
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        unscare_mouse();
      }
    }

    /* should we close the dialog? */
    if(down) {
      onNewComboAlias();
      return D_REDRAW;
    }
    break;
  }

  return D_O_K;
}

int d_comboacheck_proc(int msg, DIALOG *d, int c)
{
  int temp = d->flags&D_SELECTED;
  int ret=jwin_check_proc(msg,d,c);
  if(temp != (d->flags&D_SELECTED))
  {
    return D_REDRAW;
  }
  return ret;
}

static DIALOG editcomboa_dlg[] =
{
  /* (dialog proc)     (x)   (y)    (w)   (h)   (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
  { jwin_win_proc,        0,    0,  320,  240,  vc(14),   vc(1),      0,       D_EXIT,     0,             0,       (void *) "Combo Alias Edit" },
  { jwin_button_proc,     148,  212,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     232,  212,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_frame_proc,      4+121,   28+81,   1,   1,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { d_comboabutton_proc,   25,  212,  101,   21,   vc(14),  vc(1),  'p',     D_EXIT,     0,             0,       (void *) "&Properties" },
  { d_comboalist_proc,    266,   25,   50,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,     0,             0,      (void *) comboalist },
  { d_comboa_radio_proc,  285,   44,  30,   8+1,    vc(14),  vc(1),  0,       D_SELECTED,          0,             0,       (void *) "0" },
  { d_comboa_radio_proc,  285,   54,  30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1" },
  { d_comboa_radio_proc,  285,   64,  30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2" },
  { d_comboa_radio_proc,  285,   74,  30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3" },
  { d_comboa_radio_proc,  285,   84,   30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4" },
  { d_comboa_radio_proc,  285,   94,   30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "5" },
  { d_comboa_radio_proc,  285,   104,  30,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "6" },
  //  { d_comboacheck_proc,     278,   160,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,      NULL },
  { d_comboacheck_proc,     285,   164,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,      NULL },
  { d_comboa_proc,         6,    27,  256,  176,  0,   0,      0,       0,     0,             0,       NULL },
  //  { jwin_ctext_proc,     283,   172,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "View"},
  //  { jwin_ctext_proc,     283,   182,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Current"},
  //  { jwin_ctext_proc,     283,   192,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Layer"},
  { jwin_ctext_proc,     290,   176,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Only Show"},
  { jwin_ctext_proc,     290,   186,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Current"},
  { jwin_ctext_proc,     290,   196,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Layer"},
  { jwin_ctext_proc,     290,   122,   27,   8,   0,        0,       0,      0,        0,              0,      (void *) "Thumbnail"},
  { jwin_frame_proc,     280,   132,   20,   20,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { d_comboat_proc,      282,   134,   16,   16,  0,   0,      0,       0,     0,             0,       NULL },
  { NULL }
};

int d_comboa_radio_proc(int msg,DIALOG *d,int c)
{
  int temp = layer_cnt;
  int ret = jwin_radio_proc(msg,d,c);
  if (editcomboa_dlg[6].flags&D_SELECTED) layer_cnt=0;
  else if (editcomboa_dlg[7].flags&D_SELECTED) layer_cnt=1;
    else if (editcomboa_dlg[8].flags&D_SELECTED) layer_cnt=2;
      else if (editcomboa_dlg[9].flags&D_SELECTED) layer_cnt=3;
        else if (editcomboa_dlg[10].flags&D_SELECTED) layer_cnt=4;
          else if (editcomboa_dlg[11].flags&D_SELECTED) layer_cnt=5;
            else if (editcomboa_dlg[12].flags&D_SELECTED) layer_cnt=6;
              if (temp != layer_cnt)
  {
    return D_REDRAW;
  }

  return ret;
}

int set_comboaradio(byte layermask)
{
  if(editcomboa_dlg[7].flags&D_SELECTED) editcomboa_dlg[7].flags &= ~D_SELECTED;
  if(editcomboa_dlg[8].flags&D_SELECTED) editcomboa_dlg[8].flags &= ~D_SELECTED;
  if(editcomboa_dlg[9].flags&D_SELECTED) editcomboa_dlg[9].flags &= ~D_SELECTED;
  if(editcomboa_dlg[10].flags&D_SELECTED) editcomboa_dlg[10].flags &= ~D_SELECTED;
  if(editcomboa_dlg[11].flags&D_SELECTED) editcomboa_dlg[11].flags &= ~D_SELECTED;
  if(editcomboa_dlg[12].flags&D_SELECTED) editcomboa_dlg[12].flags &= ~D_SELECTED;
  if(!(layermask&1)) editcomboa_dlg[7].flags |= D_DISABLED;
  else editcomboa_dlg[7].flags &= ~D_DISABLED;
  if(!(layermask&2)) editcomboa_dlg[8].flags |= D_DISABLED;
  else editcomboa_dlg[8].flags &= ~D_DISABLED;
  if(!(layermask&4)) editcomboa_dlg[9].flags |= D_DISABLED;
  else editcomboa_dlg[9].flags &= ~D_DISABLED;
  if(!(layermask&8)) editcomboa_dlg[10].flags |= D_DISABLED;
  else editcomboa_dlg[10].flags &= ~D_DISABLED;
  if(!(layermask&16)) editcomboa_dlg[11].flags |= D_DISABLED;
  else editcomboa_dlg[11].flags &= ~D_DISABLED;
  if(!(layermask&32)) editcomboa_dlg[12].flags |= D_DISABLED;
  else editcomboa_dlg[12].flags &= ~D_DISABLED;

  editcomboa_dlg[6].flags |= D_SELECTED;
  layer_cnt=0;
  return 1;
}

int onEditComboAlias()
{
  reset_combo_animations();
  for(int i=0;i<MAXCOMBOALIASES;i++)
  {
    if(temp_aliases[i].combos != NULL)
    {
      delete[] temp_aliases[i].combos;
    }
    if(temp_aliases[i].csets != NULL)
    {
      delete[] temp_aliases[i].csets;
    }
    temp_aliases[i].width=combo_aliases[i].width;
    temp_aliases[i].height=combo_aliases[i].height;
    temp_aliases[i].layermask=combo_aliases[i].layermask;
    int tcount = (comboa_lmasktotal(temp_aliases[i].layermask)+1)*(temp_aliases[i].width+1)*(temp_aliases[i].height+1);
    temp_aliases[i].combos = new word[tcount];
    temp_aliases[i].csets = new byte[tcount];
    for(int j=0;j<tcount;j++)
    {
      temp_aliases[i].combos[j] = combo_aliases[i].combos[j];
      temp_aliases[i].csets[j] = combo_aliases[i].csets[j];
    }
    temp_aliases[i].combo=combo_aliases[i].combo;
    temp_aliases[i].cset=combo_aliases[i].cset;
    //memcpy(temp_aliases[i].combos,combo_aliases[i].combos,sizeof(word)*tcount);
    //memcpy(temp_aliases[i].csets,combo_aliases[i].csets,sizeof(byte)*tcount);
  }
  editcomboa_dlg[0].dp2 = lfont;
  set_comboaradio(temp_aliases[comboa_cnt].layermask);
  editcomboa_dlg[5].d1 = comboa_cnt;

  int ret=zc_popup_dialog(editcomboa_dlg,-1);

  if(ret==1)
  {
    saved=false;
    for(int i=0;i<MAXCOMBOALIASES;i++)
    {
      if(combo_aliases[i].combos != NULL)
      {
        delete[] combo_aliases[i].combos;
      }
      if(combo_aliases[i].csets != NULL)
      {
        delete[] combo_aliases[i].csets;
      }
      combo_aliases[i].width=temp_aliases[i].width;
      combo_aliases[i].height=temp_aliases[i].height;
      combo_aliases[i].layermask=temp_aliases[i].layermask;
      int tcount = (comboa_lmasktotal(combo_aliases[i].layermask)+1)*(combo_aliases[i].width+1)*(combo_aliases[i].height+1);
      combo_aliases[i].combos = new word[tcount];
      combo_aliases[i].csets = new byte[tcount];
      for(int j=0;j<tcount;j++)
      {
        combo_aliases[i].combos[j] = temp_aliases[i].combos[j];
        combo_aliases[i].csets[j] = temp_aliases[i].csets[j];
      }
      combo_aliases[i].combo=temp_aliases[i].combo;
      combo_aliases[i].cset=temp_aliases[i].cset;
      //memcpy(combo_aliases[i].combos,temp_aliases[i].combos,sizeof(word)*tcount);
      //memcpy(combo_aliases[i].csets,temp_aliases[i].csets,sizeof(byte)*tcount);
    }
  }
  setup_combo_animations();
  return D_O_K;
}

static char ffcombo_str_buf[32];
static char fflink_str_buf[32];

BITMAP* ffcur;

char *ffcombolist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,31);
	sprintf(ffcombo_str_buf,"%d",index+1);
    return ffcombo_str_buf;
  }
  *list_size=32;
  return NULL;
}

char *fflinklist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,32);
	if(index)
	sprintf(fflink_str_buf,"%d",index);
	else sprintf(fflink_str_buf,"(None)");
    return fflink_str_buf;
  }
  *list_size=33;
  return NULL;
}

int d_ffcombolist_proc(int msg,DIALOG *d,int c);

static DIALOG ffcombo_sel_dlg[] =
{
	{ jwin_win_proc,        0,    0,  200,   159,  vc(14),   vc(1),      0,       D_EXIT,     0,             0,       (void *) "Choose Combo" },
	{ jwin_button_proc,     35,   132,   61,   21,  vc(14),   vc(1),     13,       D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,    104,   132,   61,   21,  vc(14),   vc(1),     27,       D_EXIT,     0,             0,       (void *) "Done" },
	{ d_ffcombolist_proc,  11,   25,   49,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) ffcombolist },
	{ jwin_frame_proc,   68,  23,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_bitmap_proc,     70,  25,   16,   16,   0,       0,      0,       0,             0,             0,       NULL },
	{ NULL }
};

int d_ffcombolist_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_droplist_proc(msg,d,c);
  int d1 = d->d1;
  int x=ffcombo_sel_dlg[0].x;
  int y=ffcombo_sel_dlg[0].y;
  FONT *tempfont=spfont;
  int x2=text_length(tempfont, "Move Delay:")+4;
  
  switch(msg)
  {
    case MSG_DRAW:
      if(!ffcur) return D_O_K;
      putcombo(ffcur,0,0,Map.CurrScr()->ffdata[d1],Map.CurrScr()->ffcset[d1]);
      object_message(&ffcombo_sel_dlg[5],MSG_DRAW,0);
      
      rectfill(screen,x+68,y+45,x+196,y+127,jwin_pal[jcBOX]);

      textprintf_ex(screen,tempfont,x+68,y+45,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Combo #:");
      textprintf_ex(screen,tempfont,x+68+x2,y+45,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",Map.CurrScr()->ffdata[d1]);

      textprintf_ex(screen,tempfont,x+68,y+51,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"CSet #:");
      textprintf_ex(screen,tempfont,x+68+x2,y+51,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",Map.CurrScr()->ffcset[d1]);

      textprintf_ex(screen,tempfont,x+68,y+57,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"X Pos:");
      textprintf_ex(screen,tempfont,x+68+x2,y+57,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffx[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+63,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Y Pos:");
      textprintf_ex(screen,tempfont,x+68+x2,y+63,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffy[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+69,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"X Speed:");
      textprintf_ex(screen,tempfont,x+68+x2,y+69,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffxdelta[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+75,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Y Speed:");
      textprintf_ex(screen,tempfont,x+68+x2,y+75,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffydelta[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+81,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"X Accel:");
      textprintf_ex(screen,tempfont,x+68+x2,y+81,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffxdelta2[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+87,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Y Accel:");
      textprintf_ex(screen,tempfont,x+68+x2,y+87,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%.4f",Map.CurrScr()->ffydelta2[d1]/10000.0);

      textprintf_ex(screen,tempfont,x+68,y+93,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Parent:");
      textprintf_ex(screen,tempfont,x+68+x2,y+93,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",Map.CurrScr()->fflink[d1]);

      textprintf_ex(screen,tempfont,x+68,y+99,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Move Delay:");
      textprintf_ex(screen,tempfont,x+68+x2,y+99,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",Map.CurrScr()->ffdelay[d1]);

      textprintf_ex(screen,tempfont,x+68,y+105,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Combo W:");
      textprintf_ex(screen,tempfont,x+68+x2,y+105,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",(Map.CurrScr()->ffwidth[d1]&63)+1);

      textprintf_ex(screen,tempfont,x+68,y+111,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Combo H:");
      textprintf_ex(screen,tempfont,x+68+x2,y+111,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",(Map.CurrScr()->ffheight[d1]&63)+1);

      textprintf_ex(screen,tempfont,x+68,y+117,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Tile W:");
      textprintf_ex(screen,tempfont,x+68+x2,y+117,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",(Map.CurrScr()->ffwidth[d1]>>6)+1);
      
      textprintf_ex(screen,tempfont,x+68,y+123,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Tile H:");
      textprintf_ex(screen,tempfont,x+68+x2,y+123,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",(Map.CurrScr()->ffheight[d1]>>6)+1);
      
      break;
  }
  return ret;
}

int onEditFFCombo(int);

int onSelectFFCombo()
{
	ffcombo_sel_dlg[0].dp2 = lfont;
	ffcombo_sel_dlg[3].d1 = ff_combo;
	ffcur = create_bitmap_ex(8,16,16);
	if(!ffcur) return D_O_K;
	putcombo(ffcur,0,0,Map.CurrScr()->ffdata[ff_combo],Map.CurrScr()->ffcset[ff_combo]);
	ffcombo_sel_dlg[5].dp = ffcur;
	int ret=zc_popup_dialog(ffcombo_sel_dlg,0);
	while(ret==1)
	{
		ff_combo = ffcombo_sel_dlg[3].d1;
		onEditFFCombo(ff_combo);
		ret=zc_popup_dialog(ffcombo_sel_dlg,0);
	}
	return D_O_K;
}

static int ffcombo_data_list[] =
{
  4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,54,55,-1
};

static int ffcombo_flag_list[] =
{
  31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,-1
};

static TABPANEL ffcombo_tabs[] =
{
  // (text)
  { "Data",       D_SELECTED,   ffcombo_data_list },
  { "Flags",       0,           ffcombo_flag_list },
  { NULL }
};

char *ffscriptlist(int index, int *list_size);

static DIALOG ffcombo_dlg[] =
{
    /* (dialog proc)     (x)   (y)    (w)   (h)   (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
	{ jwin_win_proc,        0,    0,  240,   215,   vc(14),   vc(1),   0,       D_EXIT,     0,             0,       (void *) "Edit Freeform Combo      " },
	{ jwin_tab_proc,   10,  23,  220,  165,   0,       0,      0,       0,             0,       0,       (void *) ffcombo_tabs, NULL, (void *)ffcombo_dlg },
	{ jwin_button_proc,    30+10,  171+20,   61,    21,   vc(14),   vc(1),   13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   100+10,  171+20,   61,    21,   vc(14),   vc(1),   27,      D_EXIT,     0,             0,       (void *) "Cancel" },
	// 4
	{ jwin_ctext_proc,    126+10,   25+20,   70,    36,   0,        0,       0,       D_DISABLED, 0,             0,       (void *) "COMBO" },
	{ jwin_frame_proc,    116+10,   31+20,   20,    20,   0,        0,       0,       0,          FR_DEEP,       0,       NULL },
	{ d_combo_proc,       118+10,   33+20,   16,    16,   0,        0,       0,       0,          0,             0,       NULL },
  // 7
	{ jwin_text_proc,       6+10,   29+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Link to:" },
	{ jwin_text_proc,       6+10,   47+20,   36,    36,   0,        0,       0,       0,          0,             0,       (void *) "X Pos:" },
	{ jwin_text_proc,       6+10,   65+20,   36,    36,   0,        0,       0,       0,          0,             0,       (void *) "Y Pos:" },
	{ jwin_text_proc,       6+10,   83+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "X Speed:" },
	{ jwin_text_proc,       6+10,  101+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Y Speed:" },
	{ jwin_text_proc,       6+10,  119+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "X Accel:" },
	{ jwin_text_proc,       6+10,  137+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Y Accel:" },
	{ jwin_text_proc,       6+10,  155+20,   70,    12,   0,        0,       0,       0,          0,             0,       (void *) "A. Delay:" },
	// 15
	{ jwin_droplist_proc,  50+10,   25+20,   60,    16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],   0,       0,          1,             0,       (void *) fflinklist },
  { jwin_edit_proc,      50+10,   43+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,   61+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,   79+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,   97+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,  115+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,  133+20,   56,    16,   vc(12),   vc(1),   0,       0,          9,             0,       NULL },
	{ jwin_edit_proc,      50+10,  151+20,   40,    16,   vc(12),   vc(1),   0,       0,          4,             0,       NULL },
	//23
	{ jwin_text_proc,       112+10,  65+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Combo W:" },
	{ jwin_text_proc,       112+10,  83+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Combo H:" },
	{ jwin_text_proc,       112+10,  101+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Tile W:" },
	{ jwin_text_proc,       112+10,  119+20,   70,    36,   0,        0,       0,       0,          0,             0,       (void *) "Tile H:" },
	//27
	{ jwin_edit_proc,      156+10,  61+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL },
	{ jwin_edit_proc,      156+10,  79+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL },
	{ jwin_edit_proc,      156+10,  97+20,   24,    16,   vc(12),   vc(1),   0,       0,          1,             0,       NULL },
	{ jwin_edit_proc,      156+10,  115+20,   24,    16,   vc(12),   vc(1),   0,       0,          1,             0,       NULL },
	//31
	{ jwin_text_proc,       6+10,  25+20,   160,    36,   0,        0,       0,       0,          0,             0,       (void *) "Flags (Normal):" },
	{ jwin_text_proc,       6+10,  75+20,   160,    36,   0,        0,       0,       0,          0,             0,       (void *) "Flags (FF Change specific)" },
	//33
	{ jwin_check_proc,      6+10,  35+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Draw Over" },
	{ jwin_check_proc,      6+10,  45+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Translucent" },
    { jwin_check_proc,      6+10,  55+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Carry-Over" },
	{ jwin_check_proc,      6+10,  65+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Stationary" },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	//41
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	{ d_dummy_proc,      6+10,  25+20,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
	//49
	{ jwin_check_proc,      6+10,  85+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Swap Next" },
	{ jwin_check_proc,      6+10,  95+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Swap Prev" },
	{ jwin_check_proc,      6+10,  105+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Chng Next" },
	{ jwin_check_proc,      6+10,  115+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Chng Prev" },
	{ jwin_check_proc,      70+10,  85+20,  80+1,  8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Chng to This" },
	{ jwin_text_proc,       112+10,  151+20,   70,    12,   0,        0,       0,       0,          0,             0,       (void *) "Script:" },
	{ jwin_droplist_proc,  156+10,   151+20,   60,    16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],   0,       0,          1,             0,       (void *) ffscriptlist },
	{ NULL }
};


char *strip_decimals(char *string)
{
  int len=strlen(string);
  char *src=(char *)malloc(len+1);
  char *tmpsrc=src;
  memcpy(src,string,len+1);
  memset(src,0,len+1);

  for (unsigned int i=0; string[i]&&i<=strlen(string); i++)
  {
    *tmpsrc=string[i];
    if (*tmpsrc=='.')
    {
      while(string[i+1]=='.'&&i<=strlen(string))
      {
        i++;
      }
    }
    tmpsrc++;
  }
  memcpy(string,src,len);
  return string;
}

char *clean_numeric_string(char *string)
{
  bool found_sign=false;
  bool found_decimal=false;
  int len=strlen(string);
  char *src=(char *)malloc(len+1);
  char *tmpsrc=src;
  memcpy(src,string,len+1);
  memset(src,0,len+1);
  // strip out non-numerical characters
  for (unsigned int i=0; string[i]&&i<=strlen(string); i++)
  {
    *tmpsrc=string[i];
    if (*tmpsrc!='.'&&*tmpsrc!='-'&&*tmpsrc!='+'&&!isdigit(*tmpsrc))
    {
      while(*tmpsrc!='.'&&*tmpsrc!='-'&&*tmpsrc!='+'&&!isdigit(*tmpsrc))
      {
        i++;
      }
    }
    tmpsrc++;
  }
  len=strlen(src);
  char *src2=(char *)malloc(len+1);
  tmpsrc=src2;
  memcpy(src,src2,len+1);
  memset(src2,0,len+1);
  // second purge
  for (unsigned int i=0; src[i]&&i<=strlen(src); i++)
  {
    *tmpsrc=src[i];
    if (*tmpsrc=='-'||*tmpsrc=='+')
    {
      if (found_sign||found_decimal)
      {
        while(*tmpsrc=='-'||*tmpsrc=='+')
        {
          i++;
        }
      }
      found_sign=true;
    }
    if (*tmpsrc=='.')
    {
      if (found_decimal)
      {
        while(*tmpsrc=='.')
        {
          i++;
        }
      }
      found_decimal=true;
    }
    tmpsrc++;
  }
  sprintf(string, "%s", src2);
  return string;
}


long ffparse(char *string)
{
  //return int(atof(string)*10000);

  //this function below isn't working too well yet
  //clean_numeric_string(string);

  //if no decimal point, ascii to int conversion
  char *ptr=strchr(string, '.');
  if (!ptr)
  {
    return atoi(string)*10000;
  }

  long ret=0;
  char *tempstring1;
  tempstring1=(char *)malloc(strlen(string)+5);
  sprintf(tempstring1, string);
  for (int i=0; i<4; ++i)
  {
    tempstring1[strlen(string)+i]='0';
  }
  ptr=strchr(tempstring1, '.');
  *ptr=0;
  ret=atoi(tempstring1)*10000;
  
  ++ptr;
  char *ptr2=ptr;
  ptr2+=4;
  *ptr2=0;
  
  if(ret<0)
	  ret-=atoi(ptr);
  else ret+=atoi(ptr);

  return ret;
}

static char ffscript_str_buf[32];

char *ffscriptlist(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,255);
	sprintf(ffscript_str_buf,"%d",index);
    return ffscript_str_buf;
  }
  *list_size=256;
  return NULL;
}

static char ffscript_str_buf2[32];

char *ffscriptlist2(int index, int *list_size)
{
  if(index>=0)
  {
    bound(index,0,254);
	sprintf(ffscript_str_buf2,"%d",index+1);
    return ffscript_str_buf2;
  }
  *list_size=255;
  return NULL;
}

static DIALOG ffscript_sel_dlg[] =
{
	{ jwin_win_proc,        0,    0,  200,   159,  vc(14),   vc(1),      0,       D_EXIT,     0,             0,       (void *) "Choose Script Number" },
	{ jwin_button_proc,     35,   132,   61,   21,  vc(14),   vc(1),     13,       D_EXIT,     0,             0,       (void *) "Save" },
  { jwin_button_proc,    104,   132,   61,   21,  vc(14),   vc(1),     27,       D_EXIT,     0,             0,       (void *) "Done" },
	{ jwin_droplist_proc,  11,   25,   49,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) ffscriptlist2 },
	{ jwin_frame_proc,   68,  23,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
	{ NULL }
};

int set_argument(char *argbuf, int script, int com, int argument)
{
	long *arg; 
	if(argument) arg = &(ffscripts[script][com].arg2);
	else arg = &(ffscripts[script][com].arg1);
	if(strcmp(argbuf,"d0")==0)
	{
		*arg = D0;
	}
	else if(strcmp(argbuf,"d1")==0)
	{
		*arg = D1;
	}
	else if(strcmp(argbuf,"d2")==0)
	{
		*arg = D2;
	}
	else if(strcmp(argbuf,"d3")==0)
	{
		*arg = D3;
	}
	else if(strcmp(argbuf,"d4")==0)
	{
		*arg = D4;
	}
	else if(strcmp(argbuf,"d5")==0)
	{
		*arg = D5;
	}
	else if(strcmp(argbuf,"d6")==0)
	{
		*arg = D6;
	}
	else if(strcmp(argbuf,"d7")==0)
	{
		*arg = D7;
	}
	else if(strcmp(argbuf,"a1")==0)
	{
		*arg = A1;
	}
	else if(strcmp(argbuf,"a2")==0)
	{
		*arg = A2;
	}
	else if(strcmp(argbuf,"data")==0)
	{
		*arg = DATA;
	}
	else if(strcmp(argbuf,"cset")==0)
	{
		*arg = FCSET;
	}
	else if(strcmp(argbuf,"delay")==0)
	{
		*arg = DELAY;
	}
	else if(strcmp(argbuf,"x")==0)
	{
		*arg = FX;
	}
	else if(strcmp(argbuf,"y")==0)
	{
		*arg = FY;
	}
	else if(strcmp(argbuf,"xd")==0)
	{
		*arg = XD;
	}
	else if(strcmp(argbuf,"yd")==0)
	{
		*arg = YD;
	}
	else if(strcmp(argbuf,"xd2")==0)
	{
		*arg = XD2;
	}
	else if(strcmp(argbuf,"yd2")==0)
	{
		*arg = YD2;
	}
	else if(strcmp(argbuf,"lx")==0)
	{
		*arg = LX;
	}
	else if(strcmp(argbuf,"ly")==0)
	{
		*arg = LY;
	}
	else
	{
		for(int i=0;i<176;i++)
		{
			char buf[7];
			sprintf(buf, "cd%d", i);
			if(strcmp(argbuf,buf)==0)
			{
				*arg = COMBOD(i);
				return 1;
			}
			sprintf(buf, "cc%d", i);
			if(strcmp(argbuf,buf)==0)
			{
				*arg = COMBOC(i);
				return 1;
			}
			sprintf(buf, "cf%d", i);
			if(strcmp(argbuf,buf)==0)
			{
				*arg = COMBOF(i);
				return 1;
			}
		}
		return 0;
	}
	return 1;
}

bool ffcheck(char *arg)
{

	for(int i=0;i<0x100;i++)
	{
		if(arg[i]!='\0')
		{
			if(i==0) 
			{
				if(arg[i]!='-' && arg[i]!='.' && !(arg[i] >= 48 && arg[i] <=57))
					return false;
			}
			else
			{
				if(arg[i]!='.' && !(arg[i] >= 48 && arg[i] <=57))
					return false;
			}
		}
		else
		{
			i=0x100;
		}
	}
	return true;
}
			
char labels[1000][80];
int lines[1000];
int numlines;

int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, int script, int com)
{
	if(strcmp(combuf,"SETR")==0)
	{
		ffscripts[script][com].command = SETR;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"SETV")==0)
	{
		ffscripts[script][com].command = SETV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"ADDR")==0)
	{
		ffscripts[script][com].command = ADDR;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"ADDV")==0)
	{
		ffscripts[script][com].command = ADDV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"SUBR")==0)
	{
		ffscripts[script][com].command = SUBR;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"SUBV")==0)
	{
		ffscripts[script][com].command = SUBV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"MULTR")==0)
	{
		ffscripts[script][com].command = MULTR;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"MULTV")==0)
	{
		ffscripts[script][com].command = MULTV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"DIVR")==0)
	{
		ffscripts[script][com].command = DIVR;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"DIVV")==0)
	{
		ffscripts[script][com].command = DIVV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"COMPARER")==0)
	{
		ffscripts[script][com].command = COMPARER;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"COMPAREV")==0)
	{
		ffscripts[script][com].command = COMPAREV;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!ffcheck(arg2buf)) return 0;
		ffscripts[script][com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
	}
	else if(strcmp(combuf,"WAITFRAME")==0)
	{
		ffscripts[script][com].command = WAITFRAME;
		ffscripts[script][com].arg1 = 0;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"GOTO")==0)
	{
		ffscripts[script][com].command = GOTO;
		bool nomatch = true;
		for(int i=0;i<numlines;i++)
		{
			if(strcmp(arg1buf,labels[i])==0)
			{
				ffscripts[script][com].arg1 = lines[i];
				nomatch = false;
				i=numlines;
			}
		}
		if(nomatch)
			ffscripts[script][com].arg1 = atoi(arg1buf)-1;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"CHECKTRIG")==0)
	{
		ffscripts[script][com].command = CHECKTRIG;
		ffscripts[script][com].arg1 = 0;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"WARP")==0)
	{
		ffscripts[script][com].command = WARP;
		ffscripts[script][com].arg1 = atoi(arg1buf);
		ffscripts[script][com].arg2 = atoi(arg2buf);
	}
	else if(strcmp(combuf,"GOTOTRUE")==0)
	{
		ffscripts[script][com].command = GOTOTRUE;
		bool nomatch = true;
		for(int i=0;i<numlines;i++)
		{
			if(strcmp(arg1buf,labels[i])==0)
			{
				ffscripts[script][com].arg1 = lines[i];
				nomatch = false;
				i=numlines;
			}
		}
		if(nomatch)
		ffscripts[script][com].arg1 = atoi(arg1buf)-1;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"GOTOFALSE")==0)
	{
		ffscripts[script][com].command = GOTOFALSE;
		bool nomatch = true;
		for(int i=0;i<numlines;i++)
		{
			if(strcmp(arg1buf,labels[i])==0)
			{
				ffscripts[script][com].arg1 = lines[i];
				nomatch = false;
				i=numlines;
			}
		}
		if(nomatch)
		ffscripts[script][com].arg1 = atoi(arg1buf)-1;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"GOTOMORE")==0)
	{
		ffscripts[script][com].command = GOTOMORE;
		bool nomatch = true;
		for(int i=0;i<numlines;i++)
		{
			if(strcmp(arg1buf,labels[i])==0)
			{
				ffscripts[script][com].arg1 = lines[i];
				nomatch = false;
				i=numlines;
			}
		}
		if(nomatch)
		ffscripts[script][com].arg1 = atoi(arg1buf)-1;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"GOTOLESS")==0)
	{
		ffscripts[script][com].command = GOTOLESS;
		bool nomatch = true;
		for(int i=0;i<numlines;i++)
		{
			if(strcmp(arg1buf,labels[i])==0)
			{
				ffscripts[script][com].arg1 = lines[i];
				nomatch = false;
				i=numlines;
			}
		}
		if(nomatch)
		ffscripts[script][com].arg1 = atoi(arg1buf)-1;
		ffscripts[script][com].arg2 = 0;
	}
	else if(strcmp(combuf,"LOAD1")==0)
	{
		ffscripts[script][com].command = LOAD1;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"LOAD2")==0)
	{
		ffscripts[script][com].command = LOAD2;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"SETA1")==0)
	{
		ffscripts[script][com].command = SETA1;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"SETA2")==0)
	{
		ffscripts[script][com].command = SETA2;
		if(!set_argument(arg1buf, script, com, 0))
			return 0;
		if(!set_argument(arg2buf, script, com, 1))
			return 0;
	}
	else if(strcmp(combuf,"QUIT")==0)
	{
		ffscripts[script][com].command = QUIT;
		ffscripts[script][com].arg1 = 0;
		ffscripts[script][com].arg2 = 0;
	}
	else
	{
		return 0;
	}
	return 1;
}

int parse_script(int script)
{
  if(!getname("Import Script (.txt)","txt",datapath,false))
    return D_O_K;
  saved=false;
  FILE *fscript = fopen(temppath,"rb");
  char *buffer = new char[0x400];
  char *combuf = new char[0x100];
  char *arg1buf = new char[0x100];
  char *arg2buf = new char[0x100];
  bool stop=false;
  numlines = 0;
  for(int i=0;i<1000;i++)
  {
	  buffer[0]=0;
      for(int j=0;j<0x400;j++)
      {
        char temp;
        temp = getc(fscript);
        if(feof(fscript)) {stop=true; j=0x400; ungetc(temp,fscript);}
        else
        {
          ungetc(temp,fscript);
          buffer[j] = getc(fscript);
          if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
          {
            if(buffer[j] == '\n')
            {
              buffer[j] = '\0';
              j=0x400;
            }
            else
            {
              while(getc(fscript)!='\n');
              buffer[j] = '\0';
                j=0x400;
            }
          }
        }
      }
	  int k=0;
	  if(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
	  {
        while(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0') 
		{
			labels[numlines][k] = buffer[k];
			k++;
		}
		labels[numlines][k] = '\0';
		lines[numlines] = i;
		numlines++;
	  }
  }

  fseek(fscript, 0, SEEK_SET);
  stop = false;

  for(int i=0;i<1000;i++)
  {
    if(stop)
    {
      ffscripts[script][i].command = 0xFFFF;
      i=1000;
    }
    else
    {
/*
      sprintf(buffer, "");
      sprintf(combuf, "");
      sprintf(arg1buf, "");
      sprintf(arg2buf, "");
*/
      buffer[0]=0;
      combuf[0]=0;
      arg1buf[0]=0;
      arg2buf[0]=0;
      for(int j=0;j<0x400;j++)
      {
        char temp;
        temp = getc(fscript);
        if(feof(fscript)) {stop=true; j=0x400; ungetc(temp,fscript);}
        else
        {
          ungetc(temp,fscript);
          buffer[j] = getc(fscript);
          if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
          {
            if(buffer[j] == '\n')
            {
              buffer[j] = '\0';
              j=0x400;
            }
            else
            {
              while(getc(fscript)!='\n');
              buffer[j] = '\0';
                j=0x400;
            }
          }
        }
      }
      int k=0, l=0;
	  while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0') k++;
      while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0')  k++;
      while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        combuf[l] = buffer[k]; k++; l++;
      }
      combuf[l] = '\0';
      l=0;
      while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0') k++;
      while(buffer[k] != ',' && buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        arg1buf[l] = buffer[k]; k++; l++;
      }
      arg1buf[l] = '\0';
      l=0;
      while((buffer[k] == ' ' || buffer[k] == '\t' || buffer[k] == ',') && buffer[k] != '\0') k++;
      while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        arg2buf[l] = buffer[k]; k++; l++;
      }
      arg2buf[l] = '\0';
      if(!(parse_script_section(combuf, arg1buf, arg2buf, script, i)))
      {
        char buf[80],name[13];
        extract_name(temppath,name,FILENAME8_3);
        sprintf(buf,"Unable to parse instruction %d from script %s",i+1,name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
        stop=true;
        ffscripts[script][0].command = 0xFFFF;
	  }
    }
  }
  if(!stop)
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Script %s has been parsed",name);
    jwin_alert("Success",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  delete [] buffer;
  delete [] combuf;
  delete [] arg1buf;
  delete [] arg2buf;
  fclose(fscript);
  return D_O_K;
}

int onImportFFScript()
{
	ffscript_sel_dlg[0].dp2 = lfont;
	ffscript_sel_dlg[3].d1 = 0;

	int ret=zc_popup_dialog(ffscript_sel_dlg,0);
	if(ret==1)
	{
		parse_script(ffscript_sel_dlg[3].d1+1);
	}
	return D_O_K;
}

int onEditFFCombo(int ffcombo)
{
  char xystring[8][10];
  char wstring[4][10];
  sprintf(xystring[0],"%.4f",Map.CurrScr()->ffx[ffcombo]/10000.0);
  sprintf(xystring[1],"%.4f",Map.CurrScr()->ffy[ffcombo]/10000.0);
  sprintf(xystring[2],"%.4f",Map.CurrScr()->ffxdelta[ffcombo]/10000.0);
  sprintf(xystring[3],"%.4f",Map.CurrScr()->ffydelta[ffcombo]/10000.0);
  sprintf(xystring[4],"%.4f",Map.CurrScr()->ffxdelta2[ffcombo]/10000.0);
  sprintf(xystring[5],"%.4f",Map.CurrScr()->ffydelta2[ffcombo]/10000.0);
  //sprintf(xystring[6],"%d",Map.CurrScr()->fflink[ffcombo]);
  sprintf(xystring[6],"%d",Map.CurrScr()->ffdelay[ffcombo]);

  sprintf(wstring[0],"%d",(Map.CurrScr()->ffwidth[ffcombo]&63)+1);
  sprintf(wstring[1],"%d",(Map.CurrScr()->ffheight[ffcombo]&63)+1);
  sprintf(wstring[2],"%d",(Map.CurrScr()->ffwidth[ffcombo]>>6)+1);
  sprintf(wstring[3],"%d",(Map.CurrScr()->ffheight[ffcombo]>>6)+1);

  char title[80];
  sprintf(title,"Edit Freeform Combo (#%d)", ffcombo+1);
  ffcombo_dlg[0].dp2 = lfont;
  ffcombo_dlg[0].dp = title;
  ffcombo_dlg[4].dp2 = spfont;

  ffcombo_dlg[6].d1 = Map.CurrScr()->ffdata[ffcombo];
  ffcombo_dlg[6].fg = Map.CurrScr()->ffcset[ffcombo];

  ffcombo_dlg[15].d1 = Map.CurrScr()->fflink[ffcombo];
  ffcombo_dlg[16].dp = xystring[0];
  ffcombo_dlg[17].dp = xystring[1];
  ffcombo_dlg[18].dp = xystring[2];
  ffcombo_dlg[19].dp = xystring[3];
  ffcombo_dlg[20].dp = xystring[4];
  ffcombo_dlg[21].dp = xystring[5];
  ffcombo_dlg[22].dp = xystring[6];

  ffcombo_dlg[27].dp = wstring[0];
  ffcombo_dlg[28].dp = wstring[1];
  ffcombo_dlg[29].dp = wstring[2];
  ffcombo_dlg[30].dp = wstring[3];
  ffcombo_dlg[55].d1 = Map.CurrScr()->ffscript[ffcombo];

  int f=Map.CurrScr()->ffflags[ffcombo];
  ffcombo_dlg[33].flags = (f&ffOVERLAY) ? D_SELECTED : 0;
  ffcombo_dlg[34].flags = (f&ffTRANS) ? D_SELECTED : 0;
  ffcombo_dlg[35].flags = (f&ffCARRYOVER) ? D_SELECTED : 0;
  ffcombo_dlg[35].flags = (f&ffSTATIONARY) ? D_SELECTED : 0;
  ffcombo_dlg[49].flags = (f&ffSWAPNEXT) ? D_SELECTED : 0;
  ffcombo_dlg[50].flags = (f&ffSWAPPREV) ? D_SELECTED : 0;
  ffcombo_dlg[51].flags = (f&ffCHANGENEXT) ? D_SELECTED : 0;
  ffcombo_dlg[52].flags = (f&ffCHANGEPREV) ? D_SELECTED : 0;
  ffcombo_dlg[53].flags = (f&ffCHANGETHIS) ? D_SELECTED : 0;

  int ret=zc_popup_dialog(ffcombo_dlg,0);
  if(ret==2)
  {
	  saved=false;
    Map.CurrScr()->ffdata[ffcombo] = ffcombo_dlg[6].d1;
    Map.CurrScr()->ffcset[ffcombo] = ffcombo_dlg[6].fg;
    Map.CurrScr()->fflink[ffcombo] = ffcombo_dlg[15].d1;
    Map.CurrScr()->ffx[ffcombo] = vbound(ffparse(xystring[0]),-320000, 2880000);
    Map.CurrScr()->ffy[ffcombo] = vbound(ffparse(xystring[1]),-320000, 2080000);
    Map.CurrScr()->ffxdelta[ffcombo] = vbound(ffparse(xystring[2]),-1280000, 1280000);
    Map.CurrScr()->ffydelta[ffcombo] = vbound(ffparse(xystring[3]),-1280000, 1280000);
    Map.CurrScr()->ffxdelta2[ffcombo] = vbound(ffparse(xystring[4]),-1280000, 1280000);
    Map.CurrScr()->ffydelta2[ffcombo] = vbound(ffparse(xystring[5]),-1280000, 1280000);
    Map.CurrScr()->ffdelay[ffcombo] = atoi(xystring[6])<10000?max(0,atoi(xystring[6])):9999;
	Map.CurrScr()->ffscript[ffcombo] = ffcombo_dlg[55].d1;

	int cw = atoi(wstring[0])<65?max(1,atoi(wstring[0])):64;
	int ch = atoi(wstring[1])<65?max(1,atoi(wstring[1])):64;
	int tw = atoi(wstring[2])<5?max(1,atoi(wstring[2])):4;
	int th = atoi(wstring[3])<5?max(1,atoi(wstring[3])):4;
	Map.CurrScr()->ffwidth[ffcombo] = (cw-1)+((tw-1)<<6);
	Map.CurrScr()->ffheight[ffcombo] = (ch-1)+((th-1)<<6);

	f=0;
	f |= (ffcombo_dlg[33].flags&D_SELECTED) ? ffOVERLAY : 0;
	f |= (ffcombo_dlg[34].flags&D_SELECTED) ? ffTRANS : 0;
	f |= (ffcombo_dlg[35].flags&D_SELECTED) ? ffCARRYOVER : 0;
	f |= (ffcombo_dlg[35].flags&D_SELECTED) ? ffSTATIONARY : 0;
	f |= (ffcombo_dlg[49].flags&D_SELECTED) ? ffSWAPNEXT : 0;
	f |= (ffcombo_dlg[50].flags&D_SELECTED) ? ffSWAPPREV : 0;
	f |= (ffcombo_dlg[51].flags&D_SELECTED) ? ffCHANGENEXT : 0;
	f |= (ffcombo_dlg[52].flags&D_SELECTED) ? ffCHANGEPREV : 0;
	f |= (ffcombo_dlg[53].flags&D_SELECTED) ? ffCHANGETHIS : 0;
	Map.CurrScr()->ffflags[ffcombo] = f;

    if(Map.CurrScr()->ffdata[ffcombo]!=0)
    {
      Map.CurrScr()->numff|=(1<<ffcombo);
    }
    else
    {
      Map.CurrScr()->numff&=~(1<<ffcombo);
    }
  }

  return D_O_K;
}



static DIALOG mapstyles_dlg[] =
{
  /* (dialog proc)     (x)   (y)    (w)   (h)   (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
  { jwin_win_proc,        0,    0,  307,  186,  vc(14),   vc(1),      0,       D_EXIT,     0,             0,       (void *) "Map Styles and Colors" },
  { jwin_ctext_proc,     24,   34,   36,   36,       0,       0,      0,       0,          0,             0,       (void *) "Frame" }, //frame
  { jwin_ctext_proc,     68,   26,   20,   20,       0,       0,      0,       0,          0,             0,       (void *) "Triforce" }, //triforce fragment
  { jwin_ctext_proc,     68,   34,   20,   20,       0,       0,      0,       0,          0,             0,       (void *) "Fragment" }, //triforce fragment
  { jwin_ctext_proc,    152,   26,  100,   52,       0,       0,      0,       0,          0,             0,       (void *) "Triforce" }, //triforce frame
  { jwin_ctext_proc,    152,   34,  100,   52,       0,       0,      0,       0,          0,             0,       (void *) "Frame" }, //triforce frame
  { jwin_ctext_proc,    260,   34,   84,   52,       0,       0,      0,       0,          0,             0,       (void *) "Overworld Map" }, //overworld map
  { jwin_ctext_proc,     24,   82,   20,   20,       0,       0,      0,       0,          0,             0,       (void *) "Heart" }, //heart container piece
  { jwin_ctext_proc,     24,   90,   20,   20,       0,       0,      0,       0,          0,             0,       (void *) "Container" }, //heart container piece
  { jwin_ctext_proc,     24,   98,   20,   20,       0,       0,      0,       0,          0,             0,       (void *) "Piece" }, //heart container piece
  { jwin_ctext_proc,    260,   98,   84,   52,       0,       0,      0,       0,          0,             0,       (void *) "Dungeon Map" }, //dungeon map
  // 11
  { jwin_frame_proc,      6,   42,   36,   36,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //frame
  //  { jwin_frame_proc,     50,   42,   36,   52,       0,       0,      0,       0,          FR_DEEP,       0,       NULL }, //bs triforce fragment
  { jwin_frame_proc,     58,   42,   20,   20,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //normal triforce fragment
  //  { jwin_frame_proc,     94,   42,  116,  116,       0,       0,      0,       0,          FR_DEEP,       0,       NULL }, //bs triforce frame
  { jwin_frame_proc,    102,   42,  100,   52,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //normaltriforce frame
  { jwin_frame_proc,    218,   42,   84,   52,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //overworld map
  { jwin_frame_proc,     14,  106,   20,   20,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //heart container piece
  { jwin_frame_proc,    218,  106,   84,   52,       0,       0,      0,       0,          FR_DEEP,       0,       NULL },             //dungeon map
  // 17
  { d_maptile_proc,       8,   44,   32,   32,       0,       0,      0,       0,          0,             0,       NULL },             //frame
  //  { d_maptile_proc,      52,   44,   32,   48,       0,       0,      0,       0,          0,             0,       NULL }, //bs triforce fragment
  { d_maptile_proc,      60,   44,   16,   16,       0,       0,      0,       0,          0,             0,       NULL },             //normal triforce fragment
  //  { d_maptile_proc,      96,   44,  112,  112,       0,       0,      0,       0,          0,             0,       NULL }, //bs triforce frame
  { d_maptile_proc,     104,   44,   96,   48,       0,       0,      0,       0,          0,             0,       NULL },             //normal triforce frame
  { d_maptile_proc,     220,   44,   80,   48,       0,       0,      0,       0,          0,             0,       NULL },             //overworld map
  { d_maptile_proc,      16,  108,   16,   16,       0,       0,      0,       0,          0,             0,       NULL },             //heart container piece
  { d_maptile_proc,     220,  108,   80,   48,       0,       0,      0,       0,          0,             0,       NULL },             //dungeon map
  // 23
  { jwin_button_proc,    83,  162,   61,   21,  vc(14),   vc(1),     13,       D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,   163,  162,   61,   21,  vc(14),   vc(1),     27,       D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int onMapStyles()
{
/*
  if (zinit.subscreen<ssdtBSZELDA)
  {
    mapstyles_dlg[12].x=mapstyles_dlg[0].x+58;
    mapstyles_dlg[12].w=20;
    mapstyles_dlg[12].h=20;
    mapstyles_dlg[13].x=mapstyles_dlg[0].x+102;
    mapstyles_dlg[13].w=100;
    mapstyles_dlg[13].h=52;
  }
  else
*/
  {
    mapstyles_dlg[12].x=mapstyles_dlg[0].x+50;
    mapstyles_dlg[12].w=36;
    mapstyles_dlg[12].h=52;
    mapstyles_dlg[13].x=mapstyles_dlg[0].x+94;
    mapstyles_dlg[13].w=116;
    mapstyles_dlg[13].h=116;
  }
  mapstyles_dlg[18].x=mapstyles_dlg[12].x+2;
  mapstyles_dlg[18].w=mapstyles_dlg[12].w-4;
  mapstyles_dlg[18].h=mapstyles_dlg[12].h-4;
  mapstyles_dlg[19].x=mapstyles_dlg[13].x+2;
  mapstyles_dlg[19].w=mapstyles_dlg[13].w-4;
  mapstyles_dlg[19].h=mapstyles_dlg[13].h-4;

  mapstyles_dlg[0].dp2 = lfont;
  mapstyles_dlg[17].d1  = misc.colors.blueframe_tile;
  mapstyles_dlg[17].fg  = misc.colors.blueframe_cset;
  mapstyles_dlg[18].d1  = misc.colors.triforce_tile;
  mapstyles_dlg[18].fg  = misc.colors.triforce_cset;
  mapstyles_dlg[19].d1  = misc.colors.triframe_tile;
  mapstyles_dlg[19].fg  = misc.colors.triframe_cset;
  mapstyles_dlg[20].d1  = misc.colors.overworld_map_tile;
  mapstyles_dlg[20].fg  = misc.colors.overworld_map_cset;
  mapstyles_dlg[21].d1 = misc.colors.HCpieces_tile;
  mapstyles_dlg[21].fg = misc.colors.HCpieces_cset;
  mapstyles_dlg[22].d1  = misc.colors.dungeon_map_tile;
  mapstyles_dlg[22].fg  = misc.colors.dungeon_map_cset;

  go();
  int ret = zc_do_dialog(mapstyles_dlg,-1);
  comeback();

  if(ret==23)
  {
    misc.colors.blueframe_tile     = mapstyles_dlg[17].d1;
    misc.colors.blueframe_cset     = mapstyles_dlg[17].fg;
    misc.colors.triforce_tile      = mapstyles_dlg[18].d1;
    misc.colors.triforce_cset      = mapstyles_dlg[18].fg;
    misc.colors.triframe_tile      = mapstyles_dlg[19].d1;
    misc.colors.triframe_cset      = mapstyles_dlg[19].fg;
    misc.colors.overworld_map_tile = mapstyles_dlg[20].d1;
    misc.colors.overworld_map_cset = mapstyles_dlg[20].fg;
    misc.colors.HCpieces_tile      = mapstyles_dlg[21].d1;
    misc.colors.HCpieces_cset      = mapstyles_dlg[21].fg;
    misc.colors.dungeon_map_tile   = mapstyles_dlg[22].d1;
    misc.colors.dungeon_map_cset   = mapstyles_dlg[22].fg;
    saved=false;
  }
  return D_O_K;
}

int d_misccolors_old_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    textout_ex(screen,font,"0123456789ABCDEF",d->x+8,d->y,d->fg,d->bg);
    textout_ex(screen,font,"0",d->x,d->y+8,d->fg,d->bg);
    textout_ex(screen,font,"1",d->x,d->y+16,d->fg,d->bg);
    textout_ex(screen,font,"5",d->x,d->y+24,d->fg,d->bg);
    for(int i=0; i<32; i++)
    {
      int px = d->x+((i&15)<<3)+8;
      int py = d->y+((i>>4)<<3)+8;
      rectfill(screen,px,py,px+7,py+7,i);
    }
    for(int i=0; i<16; i++)
    {
      int px = d->x+(i<<3)+8;
      rectfill(screen,px,d->y+24,px+7,d->y+31,i+80);
    }
  }
  return D_O_K;
}

int hexclicked=-1;

int d_misccolors_hexedit_proc(int msg,DIALOG *d,int c)
{
  switch (msg)
  {
    case MSG_GOTFOCUS:
    hexclicked=(int)(d->dp3)+20;
    break;
    case MSG_LOSTFOCUS:
    hexclicked=-1;
    break;
  }
  return d_hexedit_proc(msg,d,c);
}


int d_misccolors_proc(int msg,DIALOG *d,int c);

static int misccolor1_list[] =
{
  // dialog control number
  4, 5, 6, 7, 8, 20, 21, 22, 23, 24, 36, 37, 38, 39, 40, -1
};

static int misccolor2_list[] =
{
  // dialog control number
  9, 10, 11, 12, 13, 25, 26, 27, 28, 29, 41, 42, 43, 44, 45, -1
};

static int misccolor3_list[] =
{
  // dialog control number
  14, 15, 16, 17, 18, 30, 31, 32, 33, 34, 46, 47, 48, 49, 50, -1
};

static int misccolor4_list[] =
{
  19, 35, 51, -1
};

static TABPANEL misccolor_tabs[] =
{
  // (text)
  { "1",  D_SELECTED,  misccolor1_list },
  { "2",  0,           misccolor2_list },
  { "3",  0,           misccolor3_list },
  { "4",  0,           misccolor4_list },
  { NULL }
};

int d_misccolors_tab_proc(int msg,DIALOG *d,int c);

static DIALOG misccolors_dlg[] =
{
  // (dialog proc)        (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,        2,   21,    316,  197-23,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Misc Colors" },
  //  { jwin_frame_proc,      98-84+1+2,   52+8-6+4,   132,  100,  vc(15),  vc(1),  0,       0,          FR_DEEP,             0,       NULL },
  { d_dummy_proc,         0,           0,          1,      1,      0,       0,      0,       0,          0,             0,       NULL },
  { d_misccolors_proc,    92-84+1+2,   44+8-6+4,   128+8,  96+8,   vc(9),   vc(1),  0,       0,          0,             0,       NULL },
  //3
  { d_misccolors_tab_proc,  150+14-2+10-15,   60-14,  150-10+15,  144-20-10,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],      0,      0,          0,             0,       (void *) misccolor_tabs, NULL, (void *)misccolors_dlg },
  //4
  { jwin_text_proc,       215-25-12-15,    76-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Text:" },
  { jwin_text_proc,       215-25-12-15,    94-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Caption:" },
  { jwin_text_proc,       215-25-12-15,    112-4,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Overworld Background:" },
  { jwin_text_proc,       215-25-12-15,    130-4,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon Background:" },
  { jwin_text_proc,       215-25-12-15,    148-4,    0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon Foreground:" },
  { jwin_text_proc,       215-25-12-15,   76-4,      0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Cave Foreground:" },
  { jwin_text_proc,       215-25-12-15,   94-4,      0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "BS Dark:" },
  { jwin_text_proc,       215-25-12-15,   112-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "BS Goal:" },
  { jwin_text_proc,       215-25-12-15,   130-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Compass Light:" },
  { jwin_text_proc,       215-25-12-15,   148-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Compass Dark:" },
  { jwin_text_proc,       215-25-12-15,   76-4,      0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Subscreen Background:" },
  { jwin_text_proc,       215-25-12-15,   94-4,      0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Subscreen Shadow:" },
  { jwin_text_proc,       215-25-12-15,   112-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Triforce Frame:" },
  { jwin_text_proc,       215-25-12-15,   130-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Big Map Background:" },
  { jwin_text_proc,       215-25-12-15,   148-4,     0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Big Map Foreground:" },
  { jwin_text_proc,       215-25-12-15,   76-4,      0,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Link's Position:" },

  //20
  { d_misccolors_hexedit_proc,       294-25+14+2,   76-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)0 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   94-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)1 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   112-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)2 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   130-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)3 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   148-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)4 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   76-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)5 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   94-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)6 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   112-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)7 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   130-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)8 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   148-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)9 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   76-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)10 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   94-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)11 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   112-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)12 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   130-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)13 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   148-8,   21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)14 },
  { d_misccolors_hexedit_proc,       294-25+14+2,   76-8,    21,   16,    vc(11),  vc(1),  0,       0,          2,             0,       NULL, NULL, (void *)15 },

  //36
  { jwin_text_proc,       283-25+14+2,    76-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,    94-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,    112-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,    130-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,    148-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   76-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   94-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   112-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   130-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   148-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   76-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   94-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   112-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   130-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   148-4,    0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },
  { jwin_text_proc,       283-25+14+2,   76-4,     0,    8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "0x" },

  //52
  { jwin_button_proc,     90,   190-20,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  190-20,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },

  { NULL }
};

int d_misccolors_tab_proc(int msg,DIALOG *d,int c)
{

  switch (msg)
  {
    case MSG_WANTFOCUS:
    return D_WANTFOCUS;
    break;
  }

  return jwin_tab_proc(msg,d,c);
}


int d_misccolors_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_CLICK:
    if (hexclicked!=-1)
    {
      int color_col=vbound(((gui_mouse_x()-d->x-8)/8),0,15);
      int color_row=vbound(((gui_mouse_y()-d->y-10)/8),0,11);
      sprintf((char*)misccolors_dlg[hexclicked].dp,"%X%X",color_row,color_col);
      object_message(misccolors_dlg+hexclicked,MSG_DRAW,0);
    }
    break;
    case MSG_DRAW:
    for (int i=0; i<10; i++)
    {
      textprintf_centre_ex(screen,font,d->x+8+4+(i*8),d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX], "%d", i);
    }
    for (int i=0; i<6; i++)
    {
      textprintf_centre_ex(screen,font,d->x+8+4+((10+i)*8),d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX], "%c", i+'A');
    }
    for (int i=0; i<10; i++)
    {
      textprintf_right_ex(screen,font,d->x+6,d->y+(i*8)+10,jwin_pal[jcBOXFG],jwin_pal[jcBOX], "%d", i);
    }
    for (int i=0; i<2; i++)
    {
      textprintf_right_ex(screen,font,d->x+6,d->y+((i+10)*8)+10,jwin_pal[jcBOXFG],jwin_pal[jcBOX], "%c", i+'A');
    }
    jwin_draw_frame(screen,d->x+6,d->y+8,132,100,FR_DEEP);
    for(int i=0; i<192; i++)
    {
      int px = d->x+((i&15)<<3)+8;
      int py = d->y+((i>>4)<<3)+8+2;
      rectfill(screen,px,py,px+7,py+7,i);
    }
    break;
  }
  return D_O_K;
}


int onMiscColors()
{
  char buf[16][3];
  byte *si = &(misc.colors.text);
  misccolors_dlg[0].dp2=lfont;
  for(int i=0; i<16; i++)
  {
    sprintf(buf[i],"%02X",*(si++));
    misccolors_dlg[i+20].dp = buf[i];
  }

  if(zc_popup_dialog(misccolors_dlg,0)==52)
  {
    saved=false;
    si = &(misc.colors.text);
    for(int i=0; i<16; i++)
    {
      *si = xtoi(buf[i]);
      ++si;
    }
  }
  return D_O_K;
}

// ****  Palette cycling  ****

static int palclk[3];
static int palpos[3];

void reset_pal_cycling()
{
  for(int i=0; i<3; i++)
    palclk[i]=palpos[i]=0;
}

void cycle_palette()
{
  //  if(!(Map.CurrScr()->valid&mVALID))
  //    return;

  int level = Map.CurrScr()->color;
  bool refreshpal = false;

  for(int i=0; i<3; i++)
  {
    palcycle c = misc.cycles[level][i];
    if(c.count&0xF0)
    {
      if(++palclk[i] >= c.speed)
      {
        palclk[i]=0;
        if(++palpos[i] >= (c.count>>4))
          palpos[i]=0;

        byte *si = colordata + CSET(level*pdLEVEL+poFADE1+1+palpos[i])*3;

        si += (c.first&15)*3;

        for(int col=c.first&15; col<=(c.count&15); col++)
        {
          RAMpal[CSET(c.first>>4)+col] = _RGB(si);
          si+=3;
        }

        refreshpal = true;
      }
    }
  }

  if(refreshpal) {
    rebuild_trans_table();
    set_palette_range(RAMpal,0,192,false);
  }
}


/********************/
/******  Help  ******/
/********************/

editbox_data temp_eb_data;

static DIALOG help_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
//  { jwin_textbox_proc,    4,   2+21,   320-8,  240-6-21,  0,       0,      0,       0,          0,        0,        NULL },
  { jwin_win_proc,        0,   0,   320,  240,  0,       vc(15), 0,      D_EXIT,       0,          0,        (void *) "ZQuest Help" },
  { jwin_frame_proc,   4,   23,   320-8,  240-27,   0,       0,      0,       0,             FR_DEEP,       0,       NULL },
  { d_editbox_proc,    6,   25,   320-12,  240-6-25,  0,       0,      0,       0/*D_SELECTED*/,          0,        0,        &temp_eb_data },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC,  (void *) close_dlg },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_F12,  (void *) onSnapshot },
  { NULL }
};

void doHelp(int bg,int fg)
{
  //  help_dlg[1].dp = helpbuf;
  //  help_dlg[1].bg = bg;
  //  help_dlg[1].fg = fg;
  help_dlg[2].dp=&temp_eb_data;
  //  help_dlg[0].bg = bg;
  //  help_dlg[0].fg = fg;
  ((editbox_data*) (help_dlg[2].dp))->text = &helpbuf;
  ((editbox_data*) (help_dlg[2].dp))->font = font;
  ((editbox_data*) (help_dlg[2].dp))->selfg=bg;
  ((editbox_data*) (help_dlg[2].dp))->selbg=fg;
  ((editbox_data*) (help_dlg[2].dp))->wrapping=eb_wrap_word;
  ((editbox_data*) (help_dlg[2].dp))->hsbarstyle=eb_scrollbar_off;
  //((editbox_data*) (help_dlg[1].dp))->clipboard=testcb;
  help_dlg[0].dp2= lfont;
  help_dlg[2].bg = bg;
  //help_dlg[1].fg = fg;
  //  help_dlg[1].dp = helpbuf;
  /*
    {
    char **text;
    int showcursor;
    int lines;
    int currtextline;
    int list_width;
    int currxpos;
    int fakexpos;
    int xofs;
    int yofs;
    int maxchars;
    int maxlines;
    int wrapping;
    int insertmode;
    int currchar;
    int tabdisplaystyle;
    int crlfdisplaystyle;
    int newcrlftype;
    int vsbarstyle;
    int hsbarstyle;
    FONT *font;
    char *clipboard;
    int clipboardsize;
    int defaulttabsize;
    int tabunits;
    int customtabs;
    int *customtabpos;
    int numchars;
    int selstart;
    int selend;
    int selfg;
    int selbg;
    int postpaste_dontmove;
    } editbox_data;
    */
  zc_popup_dialog(help_dlg,2);
}

int onHelp()
{
  restore_mouse();
  doHelp(vc(15),vc(0));
  return D_O_K;
}

static DIALOG layerdata_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     16-12,   20+32,   288+1+24,  200+1-32-16,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Layer Data" },
  { jwin_button_proc,     170,  180,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,     90,   180,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  // 3
  { jwin_rtext_proc,       72,   88,    40,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Map:" },
  { jwin_rtext_proc,       72,   88+18,    48,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Screen:" },
  { jwin_rtext_proc,       72,   88+36,    56,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Transparent:" },
  { jwin_ctext_proc,       89,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "1" },
  { jwin_ctext_proc,       89+40,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "2" },
  { jwin_ctext_proc,       89+80,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "3" },
  { jwin_ctext_proc,       89+120,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "4" },
  { jwin_ctext_proc,       89+160,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "5" },
  { jwin_ctext_proc,       89+200,  76,   8,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "6" },

  //12
  { jwin_edit_proc,      76,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76,   76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  { jwin_edit_proc,      76+40,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76+40,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76+40,  76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  { jwin_edit_proc,      76+80,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76+80,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76+80,  76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  { jwin_edit_proc,      76+120,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76+120,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76+120,  76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  { jwin_edit_proc,      76+160,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76+160,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76+160,  76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  { jwin_edit_proc,      76+200,   76+8,   32-6,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { d_hexedit_proc,      76+200,   76+18+8,   24-3,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_check_proc,     76+200,  76+40+8,   17,   9,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },

  //30
  { jwin_button_proc,     76,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     76+40,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     76+80,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     76+120,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     76+160,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     76+200,  76+40+18+8,  30,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Auto" },

  { NULL }

};

int edit_layers(mapscr* tempscr)
{
  char buf[6][2][8];
  layerdata_dlg[0].dp2 = lfont;
  for (int x=0; x<6; x++)
  {
    sprintf(buf[x][0],"%d",tempscr->layermap[x]);
    sprintf(buf[x][1],"%02X",tempscr->layerscreen[x]);
  }
  for (int x=0; x<6; x++)
  {
    for (int y=0; y<2; y++)
    {
      layerdata_dlg[(x*3)+y+12].dp = buf[x][y];
    }
  }
  for (int x=0; x<6; x++)
  {
    layerdata_dlg[(x*3)+2+12].flags = (tempscr->layeropacity[x]<255) ? D_SELECTED : 0;
  }

  int ret=zc_popup_dialog(layerdata_dlg,0);
  if(ret>=2)
  {
    for (int x=0; x<6; x++)
    {

      tempscr->layermap[x]=atoi(buf[x][0]);
      if (tempscr->layermap[x]>map_count)
      {
        tempscr->layermap[x]=0;
      }
      tempscr->layerscreen[x]=xtoi(buf[x][1]);

      //      tempscr->layeropacity[x]=layerdata_dlg[(x*9)+8+19].flags & D_SELECTED ? 128:255;
      tempscr->layeropacity[x]=layerdata_dlg[(x*3)+2+12].flags & D_SELECTED ? 128:255;
    }
    //  } else if (ret>72&&ret<79) {
    //    return (ret-72);
  }
  return ret;
}

static DIALOG autolayer_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        64,   32+48,   192+1,  184+1-64-28,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Autolayer Setup" },
  { jwin_text_proc,       76,   56+48,   136,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Map for layer ?: " },
  { jwin_edit_proc,       212,  56+48,   32,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_check_proc,      76,   56+18+48,   153,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Overwrite current" },

  //5
  { jwin_button_proc,     90,   188-40,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  188-40,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};

void autolayer(mapscr* tempscr, int layer, int al[6][3])
{
  char tbuf[80],mlayer[80];
  autolayer_dlg[0].dp2=lfont;
  sprintf(tbuf, "Map for layer %d: ", layer+1);
  autolayer_dlg[1].dp=tbuf;
  sprintf(mlayer, "%d", tempscr->layermap[layer]);
  autolayer_dlg[2].dp=mlayer;
  int ret=zc_popup_dialog(autolayer_dlg,0);
  if (ret==4)
  {
    int lmap=vbound(atoi(mlayer),0,Map.getMapCount());
    al[layer][0]=lmap;
    tempscr->layermap[layer]=lmap;
    tempscr->layerscreen[layer]=Map.getCurrScr();
    al[layer][1]=autolayer_dlg[3].flags & D_SELECTED?1:0;
    al[layer][2]=1;
  }
}

int findblankcombo()
{
  for (int i=0; i<MAXCOMBOS; i++)
  {

    if (!combobuf[i].flip&&!combobuf[i].walk&&!combobuf[i].type&&
        !combobuf[i].csets&&!combobuf[i].frames&&!combobuf[i].speed&&
        !combobuf[i].nextcombo&&!combobuf[i].nextcset&&
        blank_tile_table[combobuf[i].tile])
    {
      return i;
    }
  }
  return 0;
}

int onLayers()
{
  mapscr tempscr=*Map.CurrScr();
  int blankcombo=findblankcombo();
  int al[6][3];                                             //autolayer[layer][0=map, 1=overwrite current][go]
  for (int i=0; i<6; i++)
  {
    al[i][0]=tempscr.layermap[i];
    al[i][1]=0;
    al[i][2]=0;
  }
  int ret;
  do
  {
    ret=edit_layers(&tempscr);
    if (ret>2)                                              //autolayer button
    {
      autolayer(&tempscr, ret-30, al);
    }
  } while (ret>2);                                          //autolayer button
  if (ret==2)                                               //OK
  {
    Map.Ugo();
    saved=false;
    TheMaps[Map.getCurrMap()*MAPSCRS+Map.getCurrScr()]=tempscr;
    for (int i=0; i<6; i++)
    {
      int tm=tempscr.layermap[i]-1;
      if (tm!=al[i][0]-1)
      {
        al[i][2]=0;
      }
      int ts=tempscr.layerscreen[i];
      if (tm>0)
      {
        if (!(TheMaps[tm*MAPSCRS+ts].valid&mVALID))
        {
          TheMaps[tm*MAPSCRS+ts].valid=mVALID+mVERSION;
          for (int k=0; k<176; k++)
          {
            TheMaps[tm*MAPSCRS+ts].data[k]=blankcombo;
          }
        }
      }
      if (al[i][2]>0)
      {
        for (int j=0; j<128; j++)
        {
          if ((TheMaps[Map.getCurrMap()*MAPSCRS+j].layermap[i]==0) || (al[i][1]))
          {
            if (TheMaps[Map.getCurrMap()*MAPSCRS+j].layermap[i]==0)
            {
            }
            if ((TheMaps[Map.getCurrMap()*MAPSCRS+j].layermap[i]==0) && (al[i][1]))
            {
            }
            if (al[i][1])
            {
            }
            TheMaps[Map.getCurrMap()*MAPSCRS+j].layermap[i]=al[i][0];
            TheMaps[Map.getCurrMap()*MAPSCRS+j].layerscreen[i]=al[i][0]?j:0;
            if (al[i][0])
            {
              if (!(TheMaps[(al[i][0]-1)*MAPSCRS+j].valid&mVALID))
              {
                TheMaps[(al[i][0]-1)*MAPSCRS+j].valid=mVALID+mVERSION;
                for (int k=0; k<176; k++)
                {
                  TheMaps[(al[i][0]-1)*MAPSCRS+j].data[k]=blankcombo;
                }
              }
            }
          }
        }
      }
    }
  }
  return D_O_K;
}


char *itoa(int i)
{
  static char itoaret[500];
  sprintf(itoaret, "%d", i);
  return itoaret;
}

void fps_callback()
{
  lastfps=framecnt;
  framecnt=0;
}

END_OF_FUNCTION(fps_callback)

  //unsigned int col_diff[3*128];
  /*
    void bestfit_init(void)
    {
    int i;

    for (i=1; i<64; i++)

    {
    int k = i * i;
    col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
    col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
    col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
    }
    }
    */
  void create_rgb_table2(RGB_MAP *table, AL_CONST PALETTE pal, void (*callback)(int pos))
{
#define UNUSED 65535
#define LAST 65532

  /* macro add adds to single linked list */
#define add(i)    (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          (first != LAST ? (next[last] = (i)) : (first = (i))), \
                                          (last = (i))) : 0)

  /* same but w/o checking for first element */
#define add1(i)   (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          next[last] = (i), \
                                          (last = (i))) : 0)
  /* calculates distance between two colors */
#define dist(a1, a2, a3, b1, b2, b3) \
          (col_diff[ ((a2) - (b2)) & 0x7F] + \
           (col_diff + 128)[((a1) - (b1)) & 0x7F] + \
           (col_diff + 256)[((a3) - (b3)) & 0x7F])

  /* converts r,g,b to position in array and back */
#define pos(r, g, b) \
          (((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))

#define depos(pal, r, g, b) \
          ((b) = ((pal) & 31) * 2, \
           (g) = (((pal) >> 5) & 31) * 2, \
           (r) = (((pal) >> 10) & 31) * 2)

  /* is current color better than pal1? */
#define better(r1, g1, b1, pal1) \
          (((int)dist((r1), (g1), (b1), \
                      (pal1).r, (pal1).g, (pal1).b)) > (int)dist2)

  /* checking of position */
#define dopos(rp, gp, bp, ts) \
          if ((rp > -1 || r > 0) && (rp < 1 || r < 61) && \
              (gp > -1 || g > 0) && (gp < 1 || g < 61) && \
              (bp > -1 || b > 0) && (bp < 1 || b < 61)) \
        {                     \
          i = first + rp * 32 * 32 + gp * 32 + bp; \
          if (!data[i])       \
          {                   \
            data[i] = val;    \
            add1(i);          \
          }                   \
          else if ((ts) && (data[i] != val)) \
            {                 \
              dist2 = (rp ? (col_diff+128)[(r+2*rp-pal[val].r) & 0x7F] : r2) + \
                (gp ? (col_diff    )[(g+2*gp-pal[val].g) & 0x7F] : g2) + \
                (bp ? (col_diff+256)[(b+2*bp-pal[val].b) & 0x7F] : b2); \
              if (better((r+2*rp), (g+2*gp), (b+2*bp), pal[data[i]])) \
              {               \
                data[i] = val; \
                add1(i);      \
              }               \
            }                 \
        }

  int i, curr, r, g, b, val, dist2;
  unsigned int r2, g2, b2;
  unsigned short next[32*32*32];
  unsigned char *data;
  int first = LAST;
  int last = LAST;
  int count = 0;
  int cbcount = 0;

#define AVERAGE_COUNT   18000
  if (col_diff[1] == 0)
    bestfit_init();

  memset(next, 255, sizeof(next));
  memset(table->data, 0, sizeof(char)*32*32*32);


  data = (unsigned char *)table->data;

  /* add starting seeds for floodfill */
  for (i=1; i<PAL_SIZE; i++)
  {
    curr = pos(pal[i].r, pal[i].g, pal[i].b);
    if (next[curr] == UNUSED)
    {
      data[curr] = i;
      add(curr);
    }
  }

  /* main floodfill: two versions of loop for faster growing in blue axis */
  //   while (first != LAST) {
  while (first < LAST)
  {
    depos(first, r, g, b);

    /* calculate distance of current color */
    val = data[first];
    r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
    g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
    b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

    /* try to grow to all directions */
    dopos( 0, 0, 1, 1);
    dopos( 0, 0,-1, 1);
    dopos( 1, 0, 0, 1);
    dopos(-1, 0, 0, 1);
    dopos( 0, 1, 0, 1);
    dopos( 0,-1, 0, 1);

    /* faster growing of blue direction */
    if ((b > 0) && (data[first-1] == val))
    {
      b -= 2;
      first--;
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

      dopos(-1, 0, 0, 0);
      dopos( 1, 0, 0, 0);
      dopos( 0,-1, 0, 0);
      dopos( 0, 1, 0, 0);

      first++;
    }

    /* get next from list */
    i = first;
    first = next[first];
    next[i] = UNUSED;

    /* second version of loop */
    //      if (first != LAST) {
    if (first < LAST)
    {

      depos(first, r, g, b);

      val = data[first];
      r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
      g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

      dopos( 0, 0, 1, 1);
      dopos( 0, 0,-1, 1);
      dopos( 1, 0, 0, 1);
      dopos(-1, 0, 0, 1);
      dopos( 0, 1, 0, 1);
      dopos( 0,-1, 0, 1);

      if ((b < 61) && (data[first + 1] == val))
      {
        b += 2;
        first++;
        b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7f];

        dopos(-1, 0, 0, 0);
        dopos( 1, 0, 0, 0);
        dopos( 0,-1, 0, 0);
        dopos( 0, 1, 0, 0);

        first--;
      }

      i = first;
      first = next[first];
      next[i] = UNUSED;
    }

    count++;
    if (count == (cbcount+1)*AVERAGE_COUNT/256)
    {
      if (cbcount < 256)
      {
        if (callback)
          callback(cbcount);
        cbcount++;
      }
    }

  }

  /* only the transparent (pink) color can be mapped to index 0 */
  if ((pal[0].r == 63) && (pal[0].g == 0) && (pal[0].b == 63))
    table->data[31][0][31] = 0;

  if (callback)
    while (cbcount < 256)
      callback(cbcount++);
}

void rebuild_trans_table()
{
  create_rgb_table2(&zq_rgb_table, RAMpal, NULL);
  create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128, NULL);
}

int isFullScreen()
{
  return !is_windowed_mode();
}

void hit_close_button()
{
  close_button_quit=true;
  return;
}

/********************/
/******  MAIN  ******/
/********************/

/*
  enum { jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
  jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
  jcMAX };

  enum { light gray, white, off-white, gray, dark gray, black,
  jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
  jcMAX };
  */

/*
  static int jwin_pal[jcMAX] =
  {
  vc(11),vc(15),vc(4),vc(7),vc(6),vc(0),
  192,223,vc(14),vc(15),vc(0),vc(1),vc(14)
  };
  */
void custom_vsync()
{
  ++framecnt;
  if(prv_mode) {
    if(Map.get_prvtime()) {
      Map.set_prvtime(Map.get_prvtime()-1);
      if(!Map.get_prvtime())
      {
        prv_warp=1;
      }
    }
  }

  while(!myvsync) rest(1);
  myvsync=0;
  if(Vsync)
  {
    //vsync();
  }
}

void music_pause()
{
  zcmusic_pause(zcmusic, ZCM_PAUSE);
  midi_pause();
}

void music_resume()
{
  zcmusic_pause(zcmusic, ZCM_RESUME);
  midi_resume();
}

void PopulateInitDialog();
int get_currdmap()
{
  return zinit.start_dmap;
}

int get_dlevel()
{
  return DMaps[zinit.start_dmap].level;
}

int get_currscr()
{
  return Map.getCurrScr();
}

int get_homescr()
{
  return DMaps[zinit.start_dmap].cont;
}

int current_item(int item_type, bool consecutive)
{
  if (item_type==itype_shield)
  {
    return 2;
  }
  return 1;
}

bool can_use_item(int item_type, int item)
{
  return true;
}

bool has_item(int item_type, int it)
{
  return true;
}

int get_bmaps(int si)
{
  return 255;
}

word get_gamedata_maxlife()
{
  return ((get_bit(quest_rules,qr_24HC))?24:16)*HP_PER_HEART;
}

word get_gamedata_life()
{
  return ((get_bit(quest_rules,qr_24HC))?24:16)*HP_PER_HEART;
}

byte get_gamedata_magicdrainrate()
{
  return get_bit(zinit.misc,idM_DOUBLEMAGIC)?1:2;
}

word get_gamedata_maxmagic()
{
  return zinit.max_magic*MAGICPERBLOCK;
}

word get_gamedata_magic()
{
  return zinit.magic*MAGICPERBLOCK;
}

byte get_gamedata_cheat()
{
  return false;
}

byte get_gamedata_HCpieces()
{
  return zinit.hcp;
}

byte get_gamedata_sbombs()
{
  return zinit.super_bombs;
}

byte get_gamedata_bombs()
{
  return zinit.bombs;
}

byte get_gamedata_keys()
{
  return zinit.keys;
}

byte get_gamedata_lkeys()
{
  return 0;
}

byte get_gamedata_timevalid()
{
  return true;
}

dword get_gamedata_time()
{
//  return 9672950;
  return 2717760;//12:34:56
}

word get_gamedata_arrows()
{
  return 99; //zinit.arrows;
}

word get_gamedata_rupies()
{
  return zinit.rupies;
}

bool no_subscreen()
{
  return false;
}

int Awpn=0, Bwpn=0, Bpos=0;
sprite_list Sitems;

int main(int argc,char **argv)
{
  switch (IS_BETA)
  {
    case -1:
    Z_title("ZQuest %s Alpha (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case 1:
    Z_title("ZQuest %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case 0:
    Z_title("ZQuest %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
  }

  PopulateInitDialog();

  memrequested+=sizeof(zcmidi)*MAXCUSTOMMIDIS;
  Z_message("Allocating MIDI buffer (%s)... ", byte_conversion2(sizeof(zcmidi)*MAXCUSTOMMIDIS,memrequested,-1,-1));
  customMIDIs = (zcmidi*)malloc(sizeof(zcmidi)*MAXCUSTOMMIDIS);
  if(!customMIDIs)
  {
    Z_error("Error");
  }
  Z_message("OK\n");                                      // Allocating MIDI buffer...

  memrequested+=sizeof(emusic)*MAXMUSIC;
  Z_message("Allocating Enhanced Music buffer (%s)... ", byte_conversion2(sizeof(emusic)*MAXMUSIC,memrequested,-1,-1));
  enhancedMusic = (emusic*)malloc(sizeof(emusic)*MAXMUSIC);
  if(!enhancedMusic)
  {
    Z_error("Error");
  }
  Z_message("OK\n");                                      // Allocating Enhanced Music buffer...

  get_qst_buffers();

  memrequested+=sizeof(newcombo)*MAXCOMBOS;
  Z_message("Allocating combo undo buffer (%s)... ", byte_conversion2(sizeof(newcombo)*MAXCOMBOS,memrequested,-1,-1));
  undocombobuf = (newcombo*)malloc(sizeof(newcombo)*MAXCOMBOS);
  if(!undocombobuf)
  {
    Z_error("Error");
  }
  Z_message("OK\n");                                      // Allocating combo undo buffer...

  memrequested+=(NEWMAXTILES*sizeof(tiledata));
  Z_message("Allocating new undo tile buffer (%s)... ", byte_conversion2(NEWMAXTILES*sizeof(tiledata),memrequested,-1,-1));
  if(!(newundotilebuf=(tiledata*)malloc(NEWMAXTILES*sizeof(tiledata))))
    Z_error("Error");
  memset(newundotilebuf, 0, NEWMAXTILES*sizeof(tiledata));
  Z_message("OK\n");                                        // Allocating new tile buffer...

  Z_message("Resetting new tile buffer...");
  newtilebuf = (tiledata*)malloc(NEWMAXTILES*sizeof(tiledata));
  for(int j=0;j<NEWMAXTILES;j++)
    newtilebuf[j].data=NULL;
  Z_message("OK\n");

  /*
    memrequested+=NEWTILE_SIZE2;
    Z_message("Allocating tile undo buffer (%s)... ", byte_conversion2(NEWTILE_SIZE2,memrequested,-1,-1));
    undotilebuf = (byte*)malloc(NEWTILE_SIZE2);
    if(!undotilebuf)
    {
    Z_error("Error");
    }
    Z_message("OK\n");                                      // Allocating tile undo buffer...
    */

  memrequested+=(2048*5);
  Z_message("Allocating file path buffers (%s)... ", byte_conversion2(2048*5,memrequested,-1,-1));
  filepath=(char*)malloc(2048);
  temppath=(char*)malloc(2048);
  datapath=(char*)malloc(2048);
  midipath=(char*)malloc(2048);
  imagepath=(char*)malloc(2048);
  tmusicpath=(char*)malloc(2048);
  if(!filepath || !datapath || !temppath || !imagepath || !midipath || !tmusicpath)
  {
    Z_error("Error");
  }
  Z_message("OK\n");                                      // Allocating file path buffers...

  //char srandtemp[1];
  //srandtemp[0]=srandtemp[0]+0;
  srand(time(0));


  set_uformat(U_ASCII);
  Z_message("Initializing Allegro... ");

  allegro_init();
  three_finger_flag=false;
  register_bitmap_file_type("GIF",  load_gif, save_gif);
  jpgalleg_init();
  loadpng_init();
  
  set_config_file("ag.cfg");

  if(install_timer() < 0)
  {
    Z_error(allegro_error);
  }

  if(install_keyboard() < 0)
  {
    Z_error(allegro_error);
  }

  if(install_mouse() < 0)
  {
    Z_error(allegro_error);
  }
  enable_hardware_cursor();

  LOCK_VARIABLE(lastfps);

  LOCK_VARIABLE(framecnt);
  LOCK_FUNCTION(fps_callback);
  if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
    Z_error("couldn't allocate timer");

  LOCK_VARIABLE(myvsync);
  LOCK_FUNCTION(myvsync_callback);
  if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60)))
    Z_error("couldn't allocate timer");

  LOCK_VARIABLE(dclick_status);
  LOCK_VARIABLE(dclick_time);
  lock_dclick_function();
  install_int(dclick_check, 20);

  set_gfx_mode(GFX_TEXT,80,50,0,0);

  Z_message("OK\n");                                      // Initializing Allegro...

  Z_message("Loading data files:\n");

  resolve_password(datapwd);
  packfile_password(datapwd);

  sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);

  Z_message("Fonts.Dat...");
  if((fontsdata=load_datafile("fonts.dat"))==NULL)
  {
    Z_error("failed");
  }
  if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,23))
  {
    Z_error("\nIncompatible version of fonts.dat.\nPlease upgrade to %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);
  }
  Z_message("OK\n");

  sprintf(qstdat_sig,"QST.Dat %s Build %d",VerStr(QSTDAT_VERSION), QSTDAT_BUILD);

  Z_message("QST.Dat...");
  PACKFILE *f=pack_fopen("qst.dat#_SIGNATURE", F_READ_PACKED);
  if (!f)
  {
    Z_error("failed");
  }

  Z_message("ZQuest.Dat...");
  if(!(zcdata=load_datafile("zquest.dat")))
  {
    Z_error("failed");
  }
  datafile_str="zquest.dat";
  Z_message("OK\n");


  char qstdat_read_sig[52];
  memset(qstdat_read_sig, 0, 52);
  int pos=0;
  while (!pack_feof(f))
  {
    if(!p_getc(&(qstdat_read_sig[pos++]),f,true))
    {
      pack_fclose(f);
      Z_error("failed");
    }
  }
  pack_fclose(f);
  if(strncmp(qstdat_read_sig,qstdat_sig,21))
  {
    Z_error("\nIncompatible version of qst.dat.\nPlease upgrade to %s Build %d",VerStr(QSTDAT_VERSION), QSTDAT_BUILD);
  }
  Z_message("OK\n");


  packfile_password(NULL);

  font = (FONT*)fontsdata[FONT_GUI_PROP].dat;
  pfont = (FONT*)fontsdata[FONT_8xPROP_THIN].dat;
  lfont = (FONT*)fontsdata[FONT_LARGEPROP].dat;
  lfont_l = (FONT*)fontsdata[FONT_LARGEPROP_L].dat;
  zfont = (FONT*)fontsdata[FONT_NES].dat;
  z3font = (FONT*)fontsdata[FONT_Z3].dat;
  z3smallfont = (FONT*)fontsdata[FONT_Z3SMALL].dat;
  mfont = (FONT*)fontsdata[FONT_MATRIX].dat;
  ztfont = (FONT*)fontsdata[FONT_ZTIME].dat;
  sfont = (FONT*)fontsdata[FONT_6x6].dat;
  spfont = (FONT*)fontsdata[FONT_6xPROP].dat;
  ssfont1 = (FONT*)fontsdata[FONT_SUBSCREEN1].dat;
  ssfont2 = (FONT*)fontsdata[FONT_SUBSCREEN2].dat;
  ssfont3 = (FONT*)fontsdata[FONT_SUBSCREEN3].dat;
  ssfont4 = (FONT*)fontsdata[FONT_SUBSCREEN4].dat;
  gbzfont = (FONT*)fontsdata[FONT_GBZELDA].dat;
  goronfont = (FONT*)fontsdata[FONT_GORON].dat;
  zoranfont = (FONT*)fontsdata[FONT_ZORAN].dat;
  hylian1font = (FONT*)fontsdata[FONT_HYLIAN1].dat;
  hylian2font = (FONT*)fontsdata[FONT_HYLIAN2].dat;
  hylian3font = (FONT*)fontsdata[FONT_HYLIAN3].dat;
  hylian4font = (FONT*)fontsdata[FONT_HYLIAN4].dat;

  for(int i=0; i<MAXCUSTOMMIDIS; i++)
  {
    customMIDIs[i].midi=NULL;
    midi_string[i+4]=customMIDIs[i].title;
  }

  for(int i=0; i<MAXCUSTOMMIDIS; i++)
  {
    customMIDIs[i].midi=NULL;
    screen_midi_string[i+5]=customMIDIs[i].title;
  }

  for(int i=0; i<4; i++)
  {
    for(int j=0;j<MAXSUBSCREENITEMS;j++)
    {
      memset(&custom_subscreen[i].objects[j],0,sizeof(subscreen_object));
    }
  }
  for(int i=0;i<256;i++)
  {
	  for(int j=0;j<1000;j++)
	  {
		  ffscripts[i][j].command = 0xFFFF;
	  }
  }

  int helpsize = file_size("zquest.txt");
  if(helpsize==0)
  {
    Z_error("zquest.txt not found");
  }

  helpbuf = (char*)malloc(helpsize<65536?65536:helpsize*2+1);
  if(!helpbuf)
  {
    Z_error("Error allocating help buffer");
  }

  if(!readfile("zquest.txt",helpbuf,helpsize))
  {
    Z_error("Error loading zquest.txt");
  }

  helpbuf[helpsize]=0;

  Z_message("OK\n");                                      // loading data files...

  init_qts();

  filepath[0]=temppath[0]=0;
#ifdef ALLEGRO_MACOSX
  char *default_path="../";
  sprintf(filepath, "../");
  sprintf(temppath, "../");
#else
  char *default_path="";
#endif
  strcpy(datapath,get_config_string("zquest",data_path_name,default_path));
  strcpy(midipath,get_config_string("zquest",midi_path_name,default_path));
  strcpy(imagepath,get_config_string("zquest",image_path_name,default_path));
  strcpy(tmusicpath,get_config_string("zquest",tmusic_path_name,default_path));
  chop_path(datapath);
  chop_path(midipath);
  chop_path(imagepath);
  chop_path(tmusicpath);

  MouseScroll         = get_config_int("zquest","mouse_scroll",0);
  TileProtection      = get_config_int("zquest","tile_protection",1);
  ShowGrid            = get_config_int("zquest","show_grid",0);
  GridColor           = get_config_int("zquest","grid_color",15);
  SnapshotFormat      = get_config_int("zquest","snapshot_format",0);
  SavePaths           = get_config_int("zquest","save_paths",1);
  CycleOn             = get_config_int("zquest","cycle_on",1);
  Vsync               = get_config_int("zquest","vsync",1)!=0;
  ShowFPS             = get_config_int("zquest","showfps",0)!=0;
  ComboBrush          = get_config_int("zquest","combo_brush",0);
  BrushPosition       = get_config_int("zquest","brush_position",0);
  FloatBrush          = get_config_int("zquest","float_brush",0);

  OpenLastQuest       = get_config_int("zquest","open_last_quest",0);
  ShowMisalignments   = get_config_int("zquest","show_misalignments",0);
  AnimationOn         = get_config_int("zquest","animation_on",1);
  AutoBackup          = get_config_int("zquest","auto_backup",0);
  OverwriteProtection = get_config_int("zquest","overwrite_prevention",0)!=0;
  ImportMapBias       = get_config_int("zquest","import_map_bias",0);
  midi_strict         = get_config_int("zquest","strict_midi",1);
  // 1 <= zcmusic_bufsz <= 128
  zcmusic_bufsz = vbound(get_config_int("zquest","zqmusic_bufsz",64),1,128);

  int tempvalue       = get_config_int("zquest","layer_mask",-1);
  LayerMask[0]=byte(tempvalue&0xFF);
  LayerMask[1]=byte((tempvalue>>8)&0xFF);

  for (int x=0; x<7; x++)
  {
    LayerMaskInt[x]=get_bit(LayerMask,x);
  }
  DuplicateAction[0] = get_config_int("zquest","normal_duplicate_action",2);
  DuplicateAction[1] = get_config_int("zquest","horizontal_duplicate_action",0);
  DuplicateAction[2] = get_config_int("zquest","vertical_duplicate_action",0);
  DuplicateAction[3] = get_config_int("zquest","both_duplicate_action",0);
  LeechUpdate = get_config_int("zquest","leech_update",500);
  LeechUpdateTiles = get_config_int("zquest","leech_update_tiles",1);
  OnlyCheckNewTilesForDuplicates = get_config_int("zquest","only_check_new_tiles_for_duplicates",0);
  gui_colorset = get_config_int("zquest","gui_colorset",0);

  if (OpenLastQuest&&!used_switch(argc,argv,"-new"))
  {
    strcpy(filepath,get_config_string("zquest",last_quest_name,""));
  }

  if (used_switch(argc,argv,"-large"))
  {
    blackout_color=8;
    zq_screen_w=800;
    zq_screen_h=600;
    minimap_x=3;
    minimap_y=556;
    minimap_w=55;
    minimap_h=43;
    combolist_w=4;
    combolist_h=24;
    combolist_ha=20;
    combolist_x=672;
    combolist_y=16;
    mapscreen_x=0;
    mapscreen_y=16;
    mapscreensize=2;
    showedges=1;
    showallpanels=0;
    panel_1_x=64;
    panel_1_y=552;
    panel_1_w=198;
    panel_1_h=48;
    panel_2_x=64;
    panel_2_y=552;
    panel_2_w=198;
    panel_2_h=48;
    panel_3_x=64;
    panel_3_y=552;
    panel_3_w=198;
    panel_3_h=48;
    panel_4_x=64;
    panel_4_y=552;
    panel_4_w=198;
    panel_4_h=48;
    panel_5_x=64;
    panel_5_y=552;
    panel_5_w=198;
    panel_5_h=48;
    panel_6_x=64;
    panel_6_y=552;
    panel_6_w=198;
    panel_6_h=48;
    panel_7_x=64;
    panel_7_y=552;
    panel_7_w=198;
    panel_7_h=48;
    panel_8_x=64;
    panel_8_y=552;
    panel_8_w=198;
    panel_8_h=48;
    panel_9_x=64;
    panel_9_y=552;
    panel_9_w=198;
    panel_9_h=48;
    showpanelcomboscrollers=1;
    comboscroller_x=panel_1_x+panel_1_w-15;
    comboscroller_y=panel_1_y+9;
    comboscroller_w=11;
    comboscroller_h=11;
  }
  else
  {
    blackout_color=0;
    zq_screen_w=320;
    zq_screen_h=240;
    minimap_x=3;
    minimap_y=195;
    minimap_w=55;
    minimap_h=43;
    combolist_w=4;
    combolist_h=14;
    combolist_ha=10;
    combolist_x=256;
    combolist_y=16;
    mapscreen_x=0;
    mapscreen_y=16;
    mapscreensize=1;
    showedges=0;
    showallpanels=0;
    panel_1_x=58;
    panel_1_y=192;
    panel_1_w=198;
    panel_1_h=48;
    panel_2_x=58;
    panel_2_y=192;
    panel_2_w=198;
    panel_2_h=48;
    panel_3_x=58;
    panel_3_y=192;
    panel_3_w=198;
    panel_3_h=48;
    panel_4_x=58;
    panel_4_y=192;
    panel_4_w=198;
    panel_4_h=48;
    panel_5_x=58;
    panel_5_y=192;
    panel_5_w=198;
    panel_5_h=48;
    panel_6_x=58;
    panel_6_y=192;
    panel_6_w=198;
    panel_6_h=48;
    panel_7_x=58;
    panel_7_y=192;
    panel_7_w=198;
    panel_7_h=48;
    panel_8_x=58;
    panel_8_y=192;
    panel_8_w=198;
    panel_8_h=48;
    panel_9_x=58;
    panel_9_y=192;
    panel_9_w=198;
    panel_9_h=48;
    showpanelcomboscrollers=1;
    comboscroller_x=panel_1_x+panel_1_w-15;
    comboscroller_y=panel_1_y+9;
    comboscroller_w=11;
    comboscroller_h=11;
  }

  if(used_switch(argc,argv,"-d"))
  {
    resolve_password(zquestpwd);
    set_debug(!strcmp(zquestpwd,get_config_string("zquest","debug_this","")));
  }

  char qtnametitle[20];
  char qtpathtitle[20];
  for (int x=1; x<MAXQTS; x++)
  {
    sprintf(qtnametitle, qtname_name, x);
    sprintf(qtpathtitle, qtpath_name, x);
    strcpy(QuestTemplates[x].name,get_config_string("zquest",qtnametitle,""));
    strcpy(QuestTemplates[x].path,get_config_string("zquest",qtpathtitle,""));
    if (QuestTemplates[x].name[0]==0)
    {
      qt_count=x;
      break;
    }
  }

  build_bie_list(true);

  Z_message("Initializing sound driver... ");
  if(used_switch(argc,argv,"-s"))
  {
    Z_message("skipped\n");
  }
  else
  {
    if(install_sound(DIGI_AUTODETECT,DIGI_AUTODETECT,NULL))
    {

      Z_message("Sound driver not available.  Sound disabled.\n");
    }
    else
    {
      Z_message("OK\n");
    }
  }

  if(used_switch(argc,argv,"-q"))
  {
    Z_message("-q switch used, quitting program.\n");
    exit(0);
  }

  zcmusic_init();

  set_color_depth(8);
  set_close_button_callback((void (*)()) hit_close_button);

#ifndef ALLEGRO_DOS
  zq_scale = get_config_int("zquest","scale",1);
  scale_arg = used_switch(argc,argv,"-scale");
  if(scale_arg && (argc>(scale_arg+1)))
  {
    scale_arg = atoi(argv[scale_arg+1]);
    if (scale_arg == 0)
    {
      scale_arg = 1;
    }
    zq_scale=scale_arg;
  }
  else
  {
    scale_arg = zq_scale;
  }
  zqwin_set_scale(scale_arg);
#endif

  if(used_switch(argc,argv,"-fullscreen"))
  {
    tempmode = GFX_AUTODETECT_FULLSCREEN;
  }
  else if(used_switch(argc,argv,"-windowed"))
    {
      tempmode=GFX_AUTODETECT_WINDOWED;
    }

    if (tempmode==GFX_AUTODETECT_FULLSCREEN)
  {
#ifdef ALLEGRO_MACOSX
    scale_arg=2;
#else
    if (scale_arg>2)
    {
      scale_arg=1;
    }
#endif
    zqwin_set_scale(scale_arg);
  }

  if(set_gfx_mode(tempmode,zq_screen_w,zq_screen_h,0,0)!=0)
  {
    Z_error(allegro_error);
  }
  else
  {
    Z_message("OK\n");
  }

  position_mouse(zq_screen_w/2,zq_screen_h/2);

  center_zq_class_dialogs();
  center_zq_custom_dialogs();
  center_zq_files_dialogs();
  center_zq_subscreen_dialogs();
  center_zq_tiles_dialogs();
  center_zquest_dialogs();

  screen2 = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
  tmp_scr = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
  menu1 = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
  menu3 = create_bitmap_ex(8,zq_screen_w,zq_screen_h);
  mapscreenbmp = create_bitmap_ex(8,16*(showedges?18:16),16*(showedges?13:11));
  combo_bmp = create_bitmap_ex(8,16,16);
  dmapbmp_small = create_bitmap_ex(8,65,33);
  dmapbmp_large = create_bitmap_ex(8,113,57);
  brushbmp = create_bitmap_ex(8,256, 176);
  brushscreen = create_bitmap_ex(8,256, 176);
  if(!screen2 || !tmp_scr || !menu1 || !menu3 || !combo_bmp || !dmapbmp_large || !dmapbmp_large || !brushbmp || !brushscreen)// || !brushshadowbmp )
  {
    allegro_exit();
    Z_message("Error creating bitmaps\n");
    return 1;
  }

  set_palette((RGB*)zcdata[PAL_ZQUEST].dat);
  get_palette(RAMpal);

  switch (gui_colorset)
  {
    /*
      enum
      {
      jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
      jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
      jcMAX
      };
      */
    case 1:  //Windows 98
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
      RAMpal[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
      RAMpal[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
      RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

      byte palrstart=  0*63/255, palrend=166*63/255,
        palgstart=  0*63/255, palgend=202*63/255,
        palbstart=128*63/255, palbend=240*63/255,
        paldivs=7;
      for(int i=0; i<paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(3);
      jwin_pal[jcLIGHT]  =dvc(5);
      jwin_pal[jcMEDLT]  =dvc(4);
      jwin_pal[jcMEDDARK]=dvc(2);
      jwin_pal[jcDARK]   =dvc(1);
      jwin_pal[jcBOXFG]  =dvc(1);
      jwin_pal[jcTITLEL] =dvc(9);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(7);
      jwin_pal[jcTEXTBG] =dvc(5);
      jwin_pal[jcTEXTFG] =dvc(1);
      jwin_pal[jcSELBG]  =dvc(8);
      jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
    case 2:  //Windows 99
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
      RAMpal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
      RAMpal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
      RAMpal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
      RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal[dvc(9)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

      byte palrstart=  0*63/255, palrend=166*63/255,
        palgstart=  0*63/255, palgend=202*63/255,
        palbstart=128*63/255, palbend=240*63/255,
        paldivs=6;
      for(int i=0; i<paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(4);
      jwin_pal[jcLIGHT]  =dvc(6);
      jwin_pal[jcMEDLT]  =dvc(5);
      jwin_pal[jcMEDDARK]=dvc(3);
      jwin_pal[jcDARK]   =dvc(2);
      jwin_pal[jcBOXFG]  =dvc(1);
      jwin_pal[jcTITLEL] =dvc(10);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(8);
      jwin_pal[jcTEXTBG] =dvc(6);
      jwin_pal[jcTEXTFG] =dvc(1);
      jwin_pal[jcSELBG]  =dvc(9);
      jwin_pal[jcSELFG]  =dvc(7);
    }
    break;
    case 3:  //Windows 2000 Blue
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB( 16*63/255,  15*63/255, 116*63/255);
      RAMpal[dvc(3)] = _RGB( 82*63/255,  80*63/255, 182*63/255);
      RAMpal[dvc(4)] = _RGB(162*63/255, 158*63/255, 250*63/255);
      RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 127*63/255);
      RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255,  63*63/255);
      RAMpal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

      byte palrstart=  0*63/255, palrend=162*63/255,
        palgstart=  0*63/255, palgend=158*63/255,
        palbstart= 80*63/255, palbend=250*63/255,
        paldivs=7;
      for(int i=0; i<paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(4);
      jwin_pal[jcLIGHT]  =dvc(5);
      jwin_pal[jcMEDLT]  =dvc(4);
      jwin_pal[jcMEDDARK]=dvc(3);
      jwin_pal[jcDARK]   =dvc(2);
      jwin_pal[jcBOXFG]  =dvc(1);
      jwin_pal[jcTITLEL] =dvc(9);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(7);
      jwin_pal[jcTEXTBG] =dvc(5);
      jwin_pal[jcTEXTFG] =dvc(1);
      jwin_pal[jcSELBG]  =dvc(8);
      jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
    case 687:  //Windows 2000 Gold (6-87 was the North American release date of LoZ)
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  43*63/255);
      RAMpal[dvc(3)] = _RGB(128*63/255, 128*63/255,  85*63/255);
      RAMpal[dvc(4)] = _RGB(192*63/255, 192*63/255, 128*63/255);
      RAMpal[dvc(5)] = _RGB(223*63/255, 223*63/255, 149*63/255);
      RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal[dvc(9)] = _RGB( 80*63/255,  80*63/255,   0*63/255);

      byte palrstart=128*63/255, palrend=240*63/255,
        palgstart=128*63/255, palgend=202*63/255,
        palbstart=  0*63/255, palbend=166*63/255,
        paldivs=6;
      for(int i=0; i<paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(4);
      jwin_pal[jcLIGHT]  =dvc(6);
      jwin_pal[jcMEDLT]  =dvc(5);
      jwin_pal[jcMEDDARK]=dvc(3);
      jwin_pal[jcDARK]   =dvc(2);
      jwin_pal[jcBOXFG]  =dvc(1);
      jwin_pal[jcTITLEL] =dvc(10);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(8);
      jwin_pal[jcTEXTBG] =dvc(6);
      jwin_pal[jcTEXTFG] =dvc(1);
      jwin_pal[jcSELBG]  =dvc(9);
      jwin_pal[jcSELFG]  =dvc(7);
    }
    break;
    case 4104:  //Windows 2000 Easter (4-1-04 is April Fools Day, the date of this release)
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
      RAMpal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
      RAMpal[dvc(4)] = _RGB(252*63/255, 186*63/255, 188*63/255);
      RAMpal[dvc(5)] = _RGB(254*63/255, 238*63/255, 238*63/255);
      RAMpal[dvc(6)] = _RGB(244*63/255, 243*63/255, 161*63/255);
      RAMpal[dvc(7)] = _RGB(120*63/255, 173*63/255, 189*63/255);
      RAMpal[dvc(8)] = _RGB(220*63/255, 183*63/255, 227*63/255);

      byte palrstart=244*63/255, palrend=220*63/255,
        palgstart=243*63/255, palgend=183*63/255,
        palbstart=161*63/255, palbend=227*63/255,
        paldivs=7;
      for(int i=0; i < paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(4);
      jwin_pal[jcLIGHT]  =dvc(5);
      jwin_pal[jcMEDLT]  =dvc(4);
      jwin_pal[jcMEDDARK]=dvc(3);
      jwin_pal[jcDARK]   =dvc(2);
      jwin_pal[jcBOXFG]  =dvc(7);
      jwin_pal[jcTITLEL] =dvc(9);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(7);
      jwin_pal[jcTEXTBG] =dvc(5);
      jwin_pal[jcTEXTFG] =dvc(7);
      jwin_pal[jcSELBG]  =dvc(8);
      jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
    default:  //Windows 2000
    {
      RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
      RAMpal[dvc(2)] = _RGB( 66*63/255,  65*63/255,  66*63/255);
      RAMpal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
      RAMpal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
      RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
      RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
      RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
      RAMpal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

      byte palrstart= 10*63/255, palrend=166*63/255,
        palgstart= 36*63/255, palgend=202*63/255,
        palbstart=106*63/255, palbend=240*63/255,
        paldivs=7;
      for(int i=0; i<paldivs; i++)
      {
        RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
        RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
      }

      jwin_pal[jcBOX]    =dvc(4);
      jwin_pal[jcLIGHT]  =dvc(5);
      jwin_pal[jcMEDLT]  =dvc(4);
      jwin_pal[jcMEDDARK]=dvc(3);
      jwin_pal[jcDARK]   =dvc(2);
      jwin_pal[jcBOXFG]  =dvc(1);
      jwin_pal[jcTITLEL] =dvc(9);
      jwin_pal[jcTITLER] =dvc(15);
      jwin_pal[jcTITLEFG]=dvc(7);
      jwin_pal[jcTEXTBG] =dvc(5);
      jwin_pal[jcTEXTFG] =dvc(1);
      jwin_pal[jcSELBG]  =dvc(8);
      jwin_pal[jcSELFG]  =dvc(6);
    }
    break;
  }

  gui_bg_color=jwin_pal[jcBOX];
  gui_fg_color=jwin_pal[jcBOXFG];
  gui_mg_color=jwin_pal[jcMEDDARK];

  jwin_set_colors(jwin_pal);
  set_palette(RAMpal);
  clear_to_color(screen,vc(0));

  //clear the midis (to keep loadquest from crashing by trying to destroy a garbage midi)
  for (int i=0; i<MAXCUSTOMMIDIS_ZQ; ++i)
  {
    customMIDIs[i].midi=NULL;
  }
  if(argc>1 && argv[1][0]!='-')
  {
    replace_extension(temppath,argv[1],"qst",2047);
    if(load_quest(temppath,true)==0)
    {
      strcpy(filepath,temppath);
      refresh(rALL);
    }
  }
  else if (OpenLastQuest&&exists(filepath)&&!used_switch(argc,argv,"-new"))
    {
      if(load_quest(filepath,true)==0)
      {
        refresh(rALL);
      }
      else
      {
        filepath[0]=temppath[0]=0;
      }
    }
    else
    {
      init_quest();
    }

  for (int x=0; x<MAXITEMS; x++)
  {
    lens_hint_item[x][0]=0;
    lens_hint_item[x][1]=0;
  }
  for (int x=0; x<MAXWPNS; x++)
  {
    lens_hint_weapon[x][0]=0;
    lens_hint_weapon[x][1]=0;
  }

  load_mice();
  load_icons();
  load_selections();
  load_arrows();
  clear_to_color(menu1,vc(0));
  refresh(rALL);
  DIALOG_PLAYER *player=init_dialog(dialogs,-1);
  gui_mouse_focus=0;

  get_palette(RAMpal);

  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  show_mouse(screen);
  rgb_map = &zq_rgb_table;

  Map.setCurrMap(zinit.last_map);
  Map.setCurrScr(zinit.last_screen);
  //  setup_combo_animations();
  refresh(rALL);
  brush_width_menu[0].flags=D_SELECTED;
  brush_height_menu[0].flags=D_SELECTED;
  fill_menu[1].flags=D_SELECTED;

  rebuild_trans_table();

  set_display_switch_mode(SWITCH_BACKGROUND);
  set_display_switch_callback(SWITCH_OUT, music_pause);
  set_display_switch_callback(SWITCH_IN, music_resume);

  quit=!update_dialog(player);
  //clear_keybuf();
  etc_menu[11].flags=D_DISABLED;
  fix_drawing_mode_menu();
  while(!quit)
  {
    ++alignment_arrow_timer;
    if (alignment_arrow_timer>63)
    {
      alignment_arrow_timer=0;
    }

    if(strcmp(catchall_string[Map.CurrScr()->room]," "))
    {
      static char ca_menu_str[40];
      sprintf(ca_menu_str,"%s\tA",catchall_string[Map.CurrScr()->room]);
      data_menu[11].text=ca_menu_str;
      data_menu[11].flags=0;
    }
    else
    {
      data_menu[11].text="Catch All\tA";
      data_menu[11].flags=D_DISABLED;
    }

    file_menu[2].flags =
    file_menu[5].flags = (saved | disable_saving|OverwriteProtection) ? D_DISABLED : 0;
    file_menu[6].flags = disable_saving ? D_DISABLED : 0;
    edit_menu[0].flags = Map.CanUndo() ? 0 : D_DISABLED;
    edit_menu[2].flags =
      edit_menu[3].flags =
      edit_menu[4].flags =
	  edit_menu[5].flags =
      paste_menu[0].flags =
      paste_menu[1].flags =
	  paste_item_menu[0].flags =
	  paste_item_menu[1].flags =
	  paste_item_menu[2].flags =
	  paste_item_menu[3].flags =
	  paste_item_menu[4].flags =
	  paste_item_menu[5].flags =
	  paste_item_menu[6].flags =
	  paste_item_menu[7].flags =
	  paste_item_menu[8].flags =
	  paste_item_menu[9].flags =
	  paste_item_menu[10].flags = Map.CanPaste() ? 0 : D_DISABLED;
    edit_menu[1].flags =
      edit_menu[6].flags = (Map.CurrScr()->valid&mVALID) ? 0 : D_DISABLED;

    tool_menu[0].flags =
      tool_menu[1].flags = (Map.getCurrScr()<TEMPLATE) ? 0 : D_DISABLED;

    defs_menu[1].flags = 0;

    etc_menu[4].flags=(isFullScreen()==1)?D_SELECTED:0;

    quit = !update_dialog(player);
    //clear_keybuf();
    if (close_button_quit)
    {
      close_button_quit=false;
      if (onExit()==D_CLOSE)
      {
        quit=true;
      }
    }

  }

#ifndef ALLEGRO_DOS
  zqwin_set_scale(1);
#endif

  chop_path(datapath);
  chop_path(midipath);
  chop_path(imagepath);
  chop_path(tmusicpath);
  set_config_string("zquest",data_path_name,datapath);
  set_config_string("zquest",midi_path_name,midipath);
  set_config_string("zquest",image_path_name,imagepath);
  set_config_string("zquest",tmusic_path_name,tmusicpath);
  set_config_string("zquest",last_quest_name,filepath);
  set_config_int("zquest","mouse_scroll",MouseScroll);
  set_config_int("zquest","tile_protection",TileProtection);
  set_config_int("zquest","show_grid",ShowGrid);
  set_config_int("zquest","grid_color",GridColor);
  set_config_int("zquest","snapshot_format",SnapshotFormat);
  set_config_int("zquest","save_paths",SavePaths);
  set_config_int("zquest","cycle_on",CycleOn);
  set_config_int("zquest","vsync",Vsync);
  set_config_int("zquest","showfps",ShowFPS);
  set_config_int("zquest","combo_brush",ComboBrush);
  set_config_int("zquest","brush_position",BrushPosition);
  set_config_int("zquest","float_brush",FloatBrush);
  set_config_int("zquest","open_last_quest",OpenLastQuest);
  set_config_int("zquest","show_misalignments",ShowMisalignments);
  set_config_int("zquest","scale",zq_scale);

  set_config_int("zquest","animation_on",AnimationOn);
  set_config_int("zquest","auto_backup",AutoBackup);
  set_config_int("zquest","overwrite_prevention",OverwriteProtection);
  set_config_int("zquest","strict_midi",midi_strict);
  set_config_int("zquest","import_map_bias",ImportMapBias);

  set_config_int("zquest","zqmusic_bufsz",zcmusic_bufsz);


  for (int x=0; x<7; x++)
  {
    set_bit(LayerMask,x, LayerMaskInt[x]);
  }
  tempvalue=LayerMask[0]+(LayerMask[1]<<8);
  set_config_int("zquest","layer_mask",tempvalue);
  set_config_int("zquest","normal_duplicate_action",DuplicateAction[0]);
  set_config_int("zquest","horizontal_duplicate_action",DuplicateAction[1]);
  set_config_int("zquest","vertical_duplicate_action",DuplicateAction[2]);
  set_config_int("zquest","both_duplicate_action",DuplicateAction[3]);
  set_config_int("zquest","leech_update",LeechUpdate);
  set_config_int("zquest","leech_update_tiles",LeechUpdateTiles);
  set_config_int("zquest","only_check_new_tiles_for_duplicates",OnlyCheckNewTilesForDuplicates);
  set_config_int("zquest","gui_colorset",gui_colorset);

  for (int x=1; x<qt_count+1; x++)
  {
    sprintf(qtnametitle, qtname_name, x);
    sprintf(qtpathtitle, qtpath_name, x);
    if (QuestTemplates[x].path[0]!=0)
    {
      set_config_string("zquest",qtnametitle,QuestTemplates[x].name);
      set_config_string("zquest",qtpathtitle,QuestTemplates[x].path);
    }
    else
    {
      break;
    }
  }

  set_palette(black_palette);
  stop_midi();
  unload_datafile(zcdata);
  destroy_bitmap(screen2);
  destroy_bitmap(tmp_scr);
  destroy_bitmap(menu1);
  destroy_bitmap(menu3);
  destroy_bitmap(mapscreenbmp);
  destroy_bitmap(combo_bmp);
  destroy_bitmap(dmapbmp_small);
  destroy_bitmap(dmapbmp_large);
  destroy_bitmap(brushbmp);
  destroy_bitmap(brushscreen);
  for(int i=0;i<MAXCOMBOALIASES;i++)
  {
    if(combo_aliases[i].combos != NULL)
    {
      delete[] combo_aliases[i].combos;
    }
    if(combo_aliases[i].csets != NULL)
    {
      delete[] combo_aliases[i].csets;
    }
    if(temp_aliases[i].combos != NULL)
    {
      delete[] temp_aliases[i].combos;
    }
    if(temp_aliases[i].csets != NULL)
    {
      delete[] temp_aliases[i].csets;
    }
  }
  for(int i=0; i<4; i++)
  {
    for(int j=0;j<MAXSUBSCREENITEMS;j++)
    {
      switch(custom_subscreen[i].objects[j].type)
      {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
        if(custom_subscreen[i].objects[j].dp1 != NULL) delete[] (char *)custom_subscreen[i].objects[j].dp1;
      }
    }
  }
  return 0;
  memset(qtpathtitle,0,10);
}

END_OF_MAIN()

void center_zquest_dialogs()
{
  jwin_center_dialog(autolayer_dlg);
  jwin_center_dialog(cheats_dlg);
  jwin_center_dialog(cpage_dlg);
  center_zq_cset_dialogs();
  jwin_center_dialog(change_track_dlg);
  jwin_center_dialog(csetfix_dlg);
  jwin_center_dialog(dmapmaps_dlg);
  center_zq_door_dialogs();
  jwin_center_dialog(editcomboa_dlg);
  jwin_center_dialog(editdmap_dlg);
  jwin_center_dialog(editinfo_dlg);
  jwin_center_dialog(editmidi_dlg);
  jwin_center_dialog(editmsg_dlg);
  jwin_center_dialog(editmusic_dlg);
  jwin_center_dialog(editshop_dlg);
  jwin_center_dialog(elist_dlg);
  jwin_center_dialog(enemy_dlg);
  jwin_center_dialog(enemyflags_dlg);
  jwin_center_dialog(ffcombo_dlg);
  jwin_center_dialog(ffcombo_sel_dlg);
  jwin_center_dialog(getnum_dlg);
  jwin_center_dialog(header_dlg);
  jwin_center_dialog(help_dlg);
  jwin_center_dialog(ilist_dlg);
  jwin_center_dialog(itemprop_dlg);
  center_zq_init_dialog();
  jwin_center_dialog(layerdata_dlg);
  jwin_center_dialog(list_dlg);
  jwin_center_dialog(loadmap_dlg);
  jwin_center_dialog(mapstyles_dlg);
  jwin_center_dialog(misccolors_dlg);
  jwin_center_dialog(newcomboa_dlg);
  jwin_center_dialog(options_dlg);
  jwin_center_dialog(path_dlg);
  jwin_center_dialog(password_dlg);
  jwin_center_dialog(pattern_dlg);
  center_zq_rules_dialog();
  jwin_center_dialog(scrdata_dlg);
  jwin_center_dialog(screen_pal_dlg);
  jwin_center_dialog(secret_dlg);
  jwin_center_dialog(sel_scombo_dlg);
  jwin_center_dialog(selectdmap_dlg);
  jwin_center_dialog(selectmidi_dlg);
  jwin_center_dialog(selectmusic_dlg);
  jwin_center_dialog(showpal_dlg);
  jwin_center_dialog(strlist_dlg);
  jwin_center_dialog(subscreen_type_dlg);
  jwin_center_dialog(template_dlg);
  jwin_center_dialog(tp_dlg);
  jwin_center_dialog(under_dlg);
  jwin_center_dialog(warp_dlg);
  jwin_center_dialog(warpdestsel_dlg);
  jwin_center_dialog(warpring_dlg);
  jwin_center_dialog(wlist_dlg);
}


void animate_coords()
{
  coord_frame=(coord_timer>>3)&3;
  if (++coord_timer>=(1<<5))
  {
    coord_timer=0;
  }
}

static int help_pos=0;

static char *help_list[] =
{
  "PgUp/PgDn - Scroll through hotkey list",
  "Esc/Enter - Exit Preview Mode",
  "R - Restore screen to original form",
  "C - Trigger combo cycling On/Off",
  "S - Trigger secret combos",
  "E - Trigger secrets 16-31 when",
  "    Enemies->Secrets defined",
  "Q/W - These still work",
  "F - Show/hide flags",
  "P - Pause everything",
  "A - Advance frame-by-frame",
  "1/2 - Trigger tile, side warp",
  "      respectively",
  "3 - Enable timed warps"
};

void do_animations()
{
  if(AnimationOn||CycleOn)
  {
    if (AnimationOn)
    {
      animate_combos();
	  update_freeform_combos();
    }
    if(CycleOn)
    {
      cycle_palette();
    }
  }
  animate_coords();
  custom_vsync();
  refresh(rALL);
  if(prv_mode)
  {
    //Put in help areas
    textprintf_ex(menu1,font,panel_8_x+1,panel_8_y+3,vc(0),-1,"%s",help_list[help_pos]);
    textprintf_ex(menu1,font,panel_8_x+1,panel_8_y+8+3,vc(0),-1,"%s",help_list[help_pos+1]);
    textprintf_ex(menu1,font,panel_8_x+1,panel_8_y+16+3,vc(0),-1,"%s",help_list[help_pos+2]);
    textprintf_ex(menu1,font,panel_8_x+1,panel_8_y+24+3,vc(0),-1,"%s",help_list[help_pos+3]);
    textprintf_ex(menu1,font,panel_8_x+1,panel_8_y+32+3,vc(0),-1,"%s",help_list[help_pos+4]);
  }
}



int d_nbmenu_proc(int msg,DIALOG *d,int c)
{
  static int ret=D_O_K;
  domouse();
  do_animations();

  //  if (msg!=MSG_IDLE)
  if (msg==MSG_GOTMOUSE||msg==MSG_XCHAR)
    //  if (0)
  {
    ComboBrushPause=1;
    refresh(rMAP);
    ComboBrushPause=0;
    restore_mouse();
  }
  //YIELD();
  rest(4);
  ret = jwin_menu_proc(msg,d,c);
  return ret;
}

void dopreview()
{
  //set_mouse_sprite(mouse_bmp[MOUSE_BMP_FLAG][0]);
  refresh(rMAP);

  while(!(gui_mouse_b()&2))
  {
    //ret = jwin_menu_proc(msg,d,c);
    if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ESC:
        case KEY_ENTER:
        goto finished;
        break;
        case KEY_F:
        Flags^=cFLAGS;
        refresh(rMAP);
        break;
        case KEY_R:
        onRType();
        break;
        case KEY_S:
        onString();
        break;
        case KEY_E:
        Map.prv_secrets(true);
        refresh(rALL);
        break;
        case KEY_C:
        onCopy();
        break;
        case KEY_A:
        onCatchall();
        break;
        case KEY_P:
        onP();
        break;
        case KEY_1:
        Map.prv_dowarp(0,0);
        prv_warp=0;
        break;
        case KEY_2:
        Map.prv_dowarp(1,0);
        prv_warp=0;
        break;
        case KEY_3:
        if(prv_twon)
        {
          prv_twon=0;
          Map.set_prvtime(0);
          prv_warp=0;
        }
        else
        {
          Map.set_prvtime(Map.get_prvscr()->timedwarptics);
          prv_twon=1;
        }
        break;
        case KEY_W:
        onW();
        break;
        case KEY_Q:
        onQ();
        break;
        case KEY_PGUP:
        help_pos--;
        if(help_pos<0)
        {
          help_pos=0;
        }
        break;
        case KEY_PGDN:
        help_pos++;
        if(help_pos>9)
        {
          help_pos=9;
        }
        break;
        /*
          case KEY_ASTERISK:
          onTimes();
          break;
          case KEY_SLASH_PAD:
          onDivide();
          break;
          case KEY_UP:
          onUp();
          break;
          case KEY_DOWN:
          onDown();
          break;
          case KEY_LEFT:
          onLeft();
          break;
          case KEY_RIGHT:
          onRight();
          break;
          case KEY_PGUP:
          onPgUp();
          break;
          case KEY_PGDN:
          onPgDn();
          break;
          case KEY_COMMA:
          onDecMap();
          break;
          case KEY_STOP:
          onIncMap();
          break;
          */
      }
    }

    if(prv_warp)
    {
      Map.prv_dowarp(1,0);
      prv_warp=0;
    }
    if(Map.get_prvfreeze())
    {
      if(Map.get_prvadvance())
      {
        do_animations();
        Map.set_prvadvance(0);
      }
    }
    else
    {
      do_animations();
      Map.set_prvadvance(0);
    }
    refresh(rMAP);

  }

finished:
  //Flags=of;
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
  prv_mode=0;
  Map.set_prvcmb(0);
  Map.set_prvadvance(0);
  Map.set_prvfreeze(0);
  Map.set_prvtime(0);
  prv_warp=0;
  loadlvlpal(Map.getcolor());
  rebuild_trans_table();
  refresh(rMAP+rMENU);
  while(gui_mouse_b());
}


int onZQVidMode()
{
  char str_a[80], str_b[80], str_c[80];
  str_a[0]=0;
  str_b[0]=0;
  str_c[0]=0;
  int mode=gfx_driver->id;
#ifdef ALLEGRO_DOS
  switch(mode)
  {
    case GFX_MODEX:                    sprintf(str_a,"VGA Mode X"); break;
    case GFX_VESA1:                    sprintf(str_a,"VESA 1.x"); break;
    case GFX_VESA2B:                   sprintf(str_a,"VESA2 Banked"); break;
    case GFX_VESA2L:                   sprintf(str_a,"VESA2 Linear"); break;
    case GFX_VESA3:                    sprintf(str_a,"VESA3"); break;
    case GFX_AUTODETECT_WINDOWED:      sprintf(str_a,"Autodetect Windowed"); break;
    case GFX_AUTODETECT_FULLSCREEN:    sprintf(str_a,"Autodetect Fullscreen"); break;
    default:                           sprintf(str_a,"Unknown... ?"); break;
  }
#elif defined(ALLEGRO_WINDOWS)
  switch(mode)
  {
    case GFX_DIRECTX:                  sprintf(str_a,"DirectX Hardware Accelerated"); break;
    case GFX_DIRECTX_SOFT:             sprintf(str_a,"DirectX Software Accelerated"); break;
    case GFX_DIRECTX_SAFE:             sprintf(str_a,"DirectX Safe"); break;
    case GFX_DIRECTX_WIN:              sprintf(str_a,"DirectX Windowed"); break;
    case GFX_GDI:                      sprintf(str_a,"GDI"); break;
    case GFX_AUTODETECT_WINDOWED:      sprintf(str_a,"Autodetect Windowed"); break;
    case GFX_AUTODETECT_FULLSCREEN:    sprintf(str_a,"Autodetect Fullscreen"); break;
    default:                           sprintf(str_a,"Unknown... ?"); break;
  }
#elif defined(ALLEGRO_MACOSX)
  switch(mode)
  {
    case GFX_SAFE:                     sprintf(str_a,"MacOS X Safe"); break;
    case GFX_QUARTZ_FULLSCREEN:        sprintf(str_a,"MacOS X Fullscreen Quartz"); break;
    case GFX_QUARTZ_WINDOW:            sprintf(str_a,"MacOS X Windowed Quartz"); break;
    case GFX_AUTODETECT_WINDOWED:      sprintf(str_a,"Autodetect Windowed"); break;
    case GFX_AUTODETECT_FULLSCREEN:    sprintf(str_a,"Autodetect Fullscreen"); break;
    default:                           sprintf(str_a,"Unknown... ?"); break;
  }
#elif defined(ALLEGRO_LINUX)
  switch(mode)
  {
    case GFX_AUTODETECT_WINDOWED:      sprintf(str_a,"Autodetect Windowed"); break;
    case GFX_AUTODETECT_FULLSCREEN:    sprintf(str_a,"Autodetect Fullscreen"); break;
    default:                           sprintf(str_a,"Unknown... ?"); break;
  }
#elif defined(ALLEGRO_GP2X)
  switch(mode)
  {
    case GFX_AUTODETECT_WINDOWED:      sprintf(str_a,"Autodetect Windowed"); break;
    case GFX_AUTODETECT_FULLSCREEN:    sprintf(str_a,"Autodetect Fullscreen"); break;
    default:                           sprintf(str_a,"Unknown... ?"); break;
  }
#endif
  sprintf(str_c,"%dx%d 8-bit",zq_screen_w*zq_scale,zq_screen_h*zq_scale);
  jwin_alert("Video Mode",str_a,str_b,str_c,"OK",NULL,13,27,lfont);
  return D_O_K;
}

bool is_zquest()
{
  return true;
}

/* end */
