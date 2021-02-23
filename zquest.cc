//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------


#define  INTERNAL_VERSION  0xA721
#define  _ZQUEST_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>

#include <allegro.h>

BEGIN_GFX_DRIVER_LIST
 GFX_DRIVER_VESA3
 GFX_DRIVER_VESA2L
 GFX_DRIVER_VESA2B
 GFX_DRIVER_VESA1
 GFX_DRIVER_MODEX
END_GFX_DRIVER_LIST

BEGIN_COLOR_DEPTH_LIST
 COLOR_DEPTH_8
END_COLOR_DEPTH_LIST

#include <jpeg.h>
#include <load_gif.h>
#include "jwin.h"

#include "zquest.h"

// the following are used by both zelda.cc and zquest.cc
#include "zdefs.h"
#include "tiles.h"
#include "colors.h"
#include "qst.h"
#include "zsys.h"

#include "midi.cc"
#include "sprite.cc"
#include "zgpdat.h"

int CSET_SIZE = 16;
int CSET_SHFT = 4;
#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14


enum { m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_layers,
       m_menucount };

#define MAXMICE 14
#define MAXARROWS 8
#define SHADOW_DEPTH 2

char *datafile_str;
DATAFILE *data=NULL, *zgpdata=NULL;
MIDI *song=NULL;
FONT *zfont,*sfont;
BITMAP *menu,*menu2,*combo_bmp,*screen2,*mouse_bmp[MAXMICE],*dmapbmp;
BITMAP *arrow_bmp[MAXARROWS],*brushbmp, *brushshadowbmp, *brushscreen;
byte *tilebuf=NULL,*colordata=NULL;
newcombo *combobuf;
itemdata *itemsbuf;
wpndata  *wpnsbuf;
guydata  *guysbuf;
newcombo curr_combo;
PALETTE RAMpal;
midi_info Midi_Info;
int  Flip=0,Combo=0,CPage=0,CSet=2,First=0;
int  Flags=0,Flag=1,menutype=m_block;
int MouseScroll, SavePaths, CycleOn, Vsync, ShowFPS;
int ComboBrush;  //show the brush under the mouse
int BrushPosition;  //top left, middle, bottom right, etc.
int FloatBrush; //makes the combo brush float a few pixels up and left
                //complete with shadow
int OpenLastQuest; //makes the program reopen the quest that was
                   //open at the time you quit
int ShowMisalignments; //makes the program display arrows over combos that are
                       //not aligned with the next screen.
int AnimationOn; //animate the combos in zquest?
int BrushWidth=1, BrushHeight=1;
bool quit=false,saved=true,debug=false,usetiles=false;
byte LayerMask[2]; //determines which layers are on or off.  0-15
int LayerMaskInt[7];
int CurrentLayer=0;
int DuplicateAction[4];
int OnlyCheckNewTilesForDuplicates;
/*
, HorizontalDuplicateAction;
int VerticalDuplicateAction, BothDuplicateAction;
*/
int LeechUpdate;
byte Color;

int fill_type=0;

char *filepath,*temppath,*midipath,*datapath,*imagepath;
char *helpbuf;

int rules_page=1, rules_pages=4;
int init_page=1, init_pages=7;
bool rules_done=false;
bool init_done=false;

// quest data
zquestheader header;
miscQdata    misc;
mapscr       *TheMaps;
dmap         *DMaps;
MsgStr       *MsgStrings;
music        *customMIDIs;
ZCHEATS      zcheats;
zinitdata    zinit;
char         palnames[256][17];

byte         *undotilebuf;
newcombo     *undocombobuf;
word animated_combo_table[MAXCOMBOS][2]; //[0]=original tile, [1]=clock

// qst.cc helpers
void init_tiles()
{
  clear_tiles();
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoread=(word*)tmpptr2; //check number of tiles
  byte *src=(byte*)(tilestoread+1); //skip past number of tiles
  //copy tile data
  memcpy(tilebuf, src, (dword)(*tilestoread)*(dword)SINGLE_TILE_SIZE);
  return;
}

bool init_colordata()
{
  if(colordata==NULL)
    return false;
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  memcpy(colordata, tmpptr6, newpsTOTAL); //read the palette info
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  memcpy(misc.cycles, tmpptr6, sizeof(palcycle)*256*3); //read cycling info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  memcpy(palnames, tmpptr6, MAXLEVELS*PALNAMESIZE); //read palette names
//  loadlvlpal(Color);
  return true;
}


void init_combos()
{
  clear_combos();
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  memcpy(combobuf, tmpptr5, (*combostoread)*sizeof(newcombo));
  setup_combo_animations();
  return;
}

void crash() {
  int a=0;
  a=a;
//  a=a/a;
  return;
}

void crash2() {
  alert("That wasn't very nice.","Do not try to crack this again.","You will not like what happens.","OK",NULL,13,27);
  int a=0;
  a=a;
//  a=a/a;
  return;
}

void crash3() {
  alert("Final warning.","This program contains a Trojan.","Do NOT try to crack it again.","OK",NULL,13,27);
  int a=0;
  a=a;
//  a=a/a;
  return;
}

void crash4() {
  alert("37 random files have been erased.","BTW, you cracked this for nothing.","This version CAN'T save.", "OK",NULL,13,27);
  int a=0;
  a=a;
//  a=a/a;
  return;
}

bool bad_version(zquestheader *header)
{
  if(header->zelda_version < 0x170)
    return true;

  return false;
}


#include "zq_misc.cc"


static MENU import_menu[] =
{ 
   { "Map",             onImport_Map,            NULL },
   { "DMaps",           onImport_DMaps,          NULL },
   { "Tiles",           onImport_Tiles,          NULL },
   { "Palettes",        onImport_Pals,           NULL },
   { "String Table",    onImport_Msgs,           NULL },
   { "Combo Table",     onImport_Combos,         NULL },
   { "Graphics Pack",   onImport_ZGP,            NULL },
   { "Unencoded Quest", onImport_UnencodedQuest, NULL },
   { NULL }
};

static MENU export_menu[] =
{ 
   { "Map",             onExport_Map,       NULL },
   { "DMaps",           onExport_DMaps,     NULL },
   { "Tiles",           onExport_Tiles,     NULL },
   { "Palettes",        onExport_Pals,      NULL },
   { "String Table",    onExport_Msgs,      NULL },
   { "Combo Table",     onExport_Combos,    NULL },
   { "Graphics Pack",   onExport_ZGP,       NULL },
   { "Unencoded Quest", onExport_UnencodedQuest, NULL },
   { NULL }
};


static MENU file_menu[] =
{ 
   { "&New",            onNew,       NULL },
   { "&Open\tF3",       onOpen,      NULL },
   { "&Save\tF2",       onSave,      NULL },
   { "Save &as...",     onSaveAs,    NULL },
   { "" },
   { "&Import\t ",   NULL,        import_menu },
   { "&Export\t ",   NULL,        export_menu },
   { "" },
   { "E&xit\tESC",      onExit,      NULL },
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
   { "Shop types",        onShopTypes,  NULL },
   { "Info types",        onInfoTypes,  NULL },
   { "Warp rings",        onWarpRings,  NULL },
   { "Whistle warps",     onWhistle,    NULL },
   { "Triforce pieces",   onTriPieces,  NULL },
   { "Game icons",        onIcons,      NULL },
   { "Misc colors",       onMiscColors, NULL },
   { "Map styles",        onMapStyles,  NULL },
   { "End String",        onEndString,  NULL },
   { NULL }
};

static MENU spr_menu[] =
{ 
   { "Items",             onCustomItems,  NULL },
   { "Weapons/Misc",      onCustomWpns,   NULL },
//   { "Guys/Enemies",      onCustomGuys,   NULL },
   { NULL }
};


static MENU colors_menu[] =
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

static MENU quest_menu[] =
{ 
   { "Maps\t ",      NULL,          maps_menu },
   { "Palettes\t ",  NULL,          colors_menu },
   { "Misc Data\t ", NULL,          misc_menu },
   { "Sprites\t ",   NULL,          spr_menu },
   { "" },
   { "&Header",         onHeader,      NULL },
   { "&Rules",          onRules,       NULL },
//   { "R&ules2",         onRules2,     NULL },
   { "Ch&eats",         onCheats,      NULL },
   { "&Strings",        onStrings,     NULL },
   { "&DMaps",          onDmaps,       NULL },
   { "&Combos",         onCombos,      NULL },
   { "&Tiles",          onTiles,       NULL },
   { "&MIDIs",          onMidis,       NULL },
   { "&Init Data",      onInit,        NULL },
   { "" },
//   { "Test",            onTest,        NULL },
//   { "Test Options",    onTestOptions, NULL },
//   { "" },
   { "Defaults\t ",  NULL,          defs_menu },
   { NULL }
};

static MENU paste_menu[] =
{ 
   { "Paste To All",      onPasteToAll,      NULL },
   { "Paste All To All",  onPasteAllToAll,   NULL },
   { NULL }
};

static MENU edit_menu[] =
{ 
   { "&Undo\tU",            onUndo,            NULL },
   { "&Copy\tC",            onCopy,            NULL },
   { "&Paste\tV",           onPaste,           NULL },
   { "Paste All",           onPasteAll,        NULL },
   { "&Adv. Paste\t ",   NULL,              paste_menu },
   { "&Delete\tDel",        onDelete,          NULL },
   { "" },
   { "Delete &Map",         onDeleteMap,       NULL },
   { NULL }
};

static MENU tool_menu[] =
{ 
   { "&Template\tF5",      onTemplate,   NULL },
   { "&Doors\tF6",         onDoors,      NULL },
   { "&Color set fix\tF7", onCSetFix,    NULL },
   { "&Flags\tF8",         onFlags,      NULL },
   { "" },
   { "Combo &Page",        onComboPage,  NULL },
   { "&View Palette",      onShowPal,    NULL },
   { "Re-apply Template",  onReTemplate, NULL },
   { NULL }
};


static MENU data_menu[] =
{ 
   { "&Screen Flags\tF9",   onScrFlags,       NULL },
   { "&Tile Warp\tF10",     onWarp,           NULL },
   { "&Side Warp\tF11",     onWarp2,          NULL },
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
   { "&Help...",            onHelp,     NULL },
   { "&About...",           onAbout,    NULL },
   { "&Options...",         onOptions,  NULL },
   { "" },
   { "&View pic...\tP",     onViewPic,  NULL },
   { "View &map...",        onViewMap,  NULL },
   { "" },
   { "&Lost woods",         playTune,   NULL },
   { "&Play MIDI",          playMIDI,   NULL },
   { "&Stop tunes",         stopMIDI,   NULL },
   { "" },
   { "&Take Snapshot",      onSnapshot, NULL },
   { NULL }
};

static MENU the_menu[] =
{ 
   { "&File",          NULL,            file_menu },
   { "&Quest",         NULL,            quest_menu },
   { "&Edit",          NULL,            edit_menu },
   { "&Tools",         NULL,            tool_menu },
   { "&Data",          NULL,            data_menu },
   { "Et&c",           NULL,            etc_menu },
   { NULL }
};

int onB() {
  char buf[80];
  sprintf(buf,"%d", CurrentLayer);
  alert(buf,NULL,NULL,"OK",NULL,13,27);
  return D_O_K;
}

static DIALOG dialogs[] =
{
 // still unused:  bhjknotxy`
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
 { d_menu_proc,       0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       the_menu },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0,       onSave },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F3,   0,       onOpen },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F5,   0,       onTemplate },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0,       onDoors },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0,       onCSetFix },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0,       onFlags },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F9,   0,       onScrFlags },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F10,  0,       onWarp },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F11,  0,       onWarp2 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0,       onEnemyFlags },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    27,      0,       0,        0,       onExit },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'd',     0,       0,        0,       onD },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'w',     0,       0,        0,       onW },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'f',     0,       0,        0,       onF },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'q',     0,       0,        0,       onQ },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'l',     0,       0,        0,       onL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'm',     0,       0,        0,       onM },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '0',     0,       0,        0,       on0 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '1',     0,       0,        0,       on1 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '2',     0,       0,        0,       on2 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '3',     0,       0,        0,       on3 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '4',     0,       0,        0,       on4 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '5',     0,       0,        0,       on5 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '6',     0,       0,        0,       on6 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '7',     0,       0,        0,       on7 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '8',     0,       0,        0,       on8 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '9',     0,       0,        0,       on9 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    ')',     0,       0,        0,       on10 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '!',     0,       0,        0,       on11 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '@',     0,       0,        0,       on12 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '#',     0,       0,        0,       on13 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '$',     0,       0,        0,       on14 },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_UP,   0,       onUp },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_DOWN, 0,       onDown },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_LEFT, 0,       onLeft },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_RIGHT,0,       onRight },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PGUP, 0,       onPgUp },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PGDN, 0,       onPgDn },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_PLUS_PAD,0,    onPlus },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_MINUS_PAD,0,   onMinus },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_ASTERISK,0,    onTimes },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_SLASH_PAD,0,      onDivide },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'u',     0,       0,        0,       onUndo },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'c',     0,       0,        0,       onCopy },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'v',     0,       0,        0,       onPaste },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'e',     0,       0,        0,       onEnemies },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'i',     0,       0,        0,       onItem },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    's',     0,       0,        0,       onString },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'g',     0,       0,        0,       onGuy },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'r',     0,       0,        0,       onRType },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'a',     0,       0,        0,       onCatchall },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'p',     0,       0,        0,       onViewPic },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    ',',     0,       0,        0,       onDecMap },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'z',     0,       0,        0,       onSnapshot },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '.',     0,       0,        0,       onIncMap },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_SPACE,0,       onSpacebar },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_DEL,  0,       onDelete },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F4,   0,       onScreenPalette },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'b',     0,       0,        0,       onB },
 { NULL,              0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       NULL }
};



static DIALOG getnum_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 80,   80,   160,  64,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       112,  104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Value:" },
 { d_edit_proc,       168,  104,  48,   8,    vc(12),  vc(1),  0,       0,          6,             0,       NULL },
 { d_button_proc,     90,   120,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  120,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


int getnumber(char *prompt,int initialval)
{
 char buf[20];
 sprintf(buf,"%d",initialval);
 getnum_dlg[1].dp=prompt;
 getnum_dlg[3].dp=buf;
 if(popup_dialog(getnum_dlg,3)==4)
   return atoi(buf);
 return -1;
}

int gethexnumber(char *prompt,int initialval)
{
 char buf[20];
 sprintf(buf,"%X",initialval);
 getnum_dlg[1].dp=prompt;
 getnum_dlg[3].dp=buf;
 if(popup_dialog(getnum_dlg,3)==4)
   return xtoi(buf);
 return -1;
}


#include "zq_class.cc"



/***********************/
/*** dialog handlers ***/
/***********************/

/*
 { d_shadow_box_proc, 56,   20,   208,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Rules" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 1" },
// 3
 { d_button_proc,     170,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     62,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     240,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Solid Blocks         " },
*/


static DIALOG options_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 80,   72-24,   160,  112+48,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  80-24,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "ZQuest Options" },
 { d_button_proc,     90,   160+24,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160+24,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_check_proc,      96,   96-24,   128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Mouse scroll  " },
 { d_check_proc,      96,   104-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Save paths    " },
 { d_check_proc,      96,   112-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Palette cycle " },
 { d_check_proc,      96,   120-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Vsync         " },
 { d_check_proc,      96,   128-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Show FPS      " },
 { d_check_proc,      96,   136-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Combo Brush   " },
 { d_check_proc,      96,   144-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Floating Brush" },
 { d_check_proc,      96,   152-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Reload Last   " },
 { d_check_proc,      96,   160-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Show Misaligns" },
 { d_check_proc,      96,   168-24,  128,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Animate Combos" },
 { NULL }
};

int onOptions()
{

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
  if(popup_dialog(options_dlg,-1) == 2)
  {
    MouseScroll        = options_dlg[4].flags & D_SELECTED ? 1 : 0;
    SavePaths          = options_dlg[5].flags & D_SELECTED ? 1 : 0;
    CycleOn            = options_dlg[6].flags & D_SELECTED ? 1 : 0;
    Vsync              = options_dlg[7].flags & D_SELECTED ? 1 : 0;
    ShowFPS            = options_dlg[8].flags & D_SELECTED ? 1 : 0;
    ComboBrush         = options_dlg[9].flags & D_SELECTED ? 1 : 0;
    FloatBrush         = options_dlg[10].flags & D_SELECTED ? 1 : 0;
    OpenLastQuest      = options_dlg[11].flags & D_SELECTED ? 1 : 0;
    ShowMisalignments  = options_dlg[12].flags & D_SELECTED ? 1 : 0;
    AnimationOn        = options_dlg[13].flags & D_SELECTED ? 1 : 0;
  }
  setup_combo_animations();
  refresh(rALL);
  comeback();
  return D_O_K;
}

int onReTemplate()
{
 if(alert("Apply template to","all screens on this map?",NULL,"&Yes","&No",'y','n')==1)
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

int onCopy()
{
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

int onDelete()
{
 if(Map.CurrScr()->valid&mVALID) {
  if(alert("Delete this screen?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
   Map.Ugo();
   Map.clearscr(Map.getCurrScr());
   refresh(rALL);
   }
  }
 return D_O_K;
}

int onDeleteMap()
{
 if(alert("Clear this entire map?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
   Map.clearmap();
   refresh(rALL);
   }
 return D_O_K;
}


int onD()
{
 Map.CurrScr()->flags^=4;
 refresh(rMAP+rMENU);
 return D_O_K;
}

int onIncMap()
{
 int m=Map.getCurrMap();
 Map.setCurrMap(m+1>=header.map_count?0:m+1);
 refresh(rALL);
 return D_O_K;
}

int onDecMap()
{
 int m=Map.getCurrMap();
 Map.setCurrMap((m-1<0)?header.map_count-1:min(m-1,header.map_count-1));
 refresh(rALL);
 return D_O_K;
}


int onDefault_Pals()
{
  if(alert("Reset all palette data?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    init_colordata();
    refresh_pal();
    }
  return D_O_K;
}

int onDefault_Combos()
{
  if(alert("Reset combo data?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
/*
    for(int i=0; i<1024; i++)
      combobuf[i]=((newcombo*)data[DAT_COMBO].dat)[i+2];
*/
    init_combos();
    refresh(rALL);
    }

  return D_O_K;
}

int onDefault_Sprites()
{
  if(alert("Reset sprite data?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    reset_items();
    reset_wpns();
    }
  return D_O_K;
}

int onDefault_Tiles()
{
  if(alert("Reset all tiles?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    reset_tiles();
    refresh(rALL);
    }
  return D_O_K;
}

int onDefault_MapStyles()
{
  if(alert("Reset all map styles?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    reset_mapstyles();
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

int onLeft()  { Map.scroll(2); refresh(rALL); return D_O_K; }
int onRight() { Map.scroll(3); refresh(rALL); return D_O_K; }
int onUp()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(0);
    refresh(rALL);
  }
  else if(First>0)
  {
    First-=4;
    refresh(rCOMBOS);
  }
  return D_O_K;
}
int onDown()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    Map.scroll(1);
    refresh(rALL);
  }
  else if(First<200)
  {
    First+=4;
    refresh(rCOMBOS);
  }
  return D_O_K;
}
int onPgUp()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    menutype=wrap(menutype-1,0,m_menucount-1);
    refresh(rMENU);
  }
  else if(First>0)
  {
    First-=min(First,52);
    refresh(rCOMBOS);
  }
  return D_O_K;
}
int onPgDn()
{
  if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT])
  {
    menutype=wrap(menutype+1,0,m_menucount-1);
    refresh(rMENU);
  }
  else if(First<200)
  {
    First+=min(200-First,52);
    refresh(rCOMBOS);
  }
  return D_O_K;
}



bool getname(char *prompt,char *ext,char *def,bool usefilename)
{
 strcpy(temppath,def);

 if(!usefilename) {
  int i=strlen(temppath);
  while(i>=0 && temppath[i]!='\\' && temppath[i]!='/')
   temppath[i--]=0;
  }

 go();
 int ret = file_select(prompt,temppath,ext);
 comeback();
 return ret!=0;
}


bool getname_nogo(char *prompt,char *ext,char *def,bool usefilename)
{
 strcpy(temppath,def);

 if(!usefilename) {
  int i=strlen(temppath);
  while(i>=0 && temppath[i]!='\\' && temppath[i]!='/')
   temppath[i--]=0;
  }

 int ret = file_select(prompt,temppath,ext);
 return ret!=0;
}





int playMIDI()
{
  if(getname("Load MIDI","mid",midipath,false)) {
   strcpy(midipath,temppath);
   if((song=load_midi(midipath))!=NULL)
    if(play_midi(song,true)==0) {
     etc_menu[7].flags=0;
     etc_menu[8].flags=D_SELECTED;
     }
   }
  return D_O_K;
}

int playTune()
{
  if(play_midi((MIDI*)data[MID_LOSTWOODS].dat,true)==0) {
   etc_menu[7].flags=D_SELECTED;
   etc_menu[8].flags=0;
   }
  return D_O_K;
}

int stopMIDI()
{
  stop_midi();
  etc_menu[7].flags = etc_menu[8].flags = 0;
  return D_O_K;
}


#include "zq_files.cc"



//  +----------+
//  |          |
//  | View Pic |
//  |          |
//  |          |
//  |          |
//  +----------+


BITMAP *pic=NULL;
PALETTE picpal;
int  px=0,py=0,pblack,pwhite;
double scale=1.0;
bool vp_showpal=true, vp_showsize=true, vp_center=true;


inline int pal_sum(RGB p) { return p.r + p.g + p.b; }


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

void draw_bw_mouse(int white)
{
  blit(mouse_bmp[0],mouse_bmp[12],0,0,0,0,16,16);
  for(int y=0; y<16; y++)
   for(int x=0; x<16; x++)
    if(getpixel(mouse_bmp[12],x,y)!=0)
     putpixel(mouse_bmp[12],x,y,white);
}


int load_the_pic()
{
  int gotit = getname("View Image (bmp,pcx,jpg,gif)","bmp;pcx;jpg;jpe;jpeg;gif",imagepath,true);
  if(!gotit)
    return 1;
  strcpy(imagepath,temppath);

  if(pic)
    destroy_bitmap(pic);

  if(   !stricmp(get_extension(imagepath),"jpg")
     || !stricmp(get_extension(imagepath),"jpeg")
     || !stricmp(get_extension(imagepath),"jpe")
    )
    pic = load_jpeg(imagepath,picpal);
  else
    pic = load_bitmap(imagepath,picpal);

  if(!pic)
  {
    alert("Error loading image:",imagepath,NULL,"OK",NULL,13,27);
    return 2;
  }

  get_bw(picpal,pblack,pwhite);
  draw_bw_mouse(pwhite);
  gui_bg_color = pblack;
  gui_fg_color = pwhite;

  if(vp_center)
    px=py=0;
  else
  {
    px=pic->w-320;
    py=pic->h-240;
  }
  return 0;
}


int onViewPic()
{
  BITMAP *buf;
  bool done=false, redraw=true;

  int bg = gui_bg_color;
  int fg = gui_fg_color;

  if(!pic && load_the_pic())
    return D_O_K;

  gui_bg_color = pblack;
  gui_fg_color = pwhite;

  buf = create_bitmap(320,240);
  if(!buf)
  {
    alert("Error creating temp bitmap",NULL,NULL,"OK",NULL,13,27);
    gui_bg_color = bg;
    gui_fg_color = fg;
    return D_O_K;
  }

  go();
  scare_mouse();
  clear(screen);
  set_palette(picpal);
  set_mouse_sprite(mouse_bmp[12]);


  do {
    if(redraw)
    {
      clear_to_color(buf,pblack);
      stretch_blit(pic,buf,0,0,pic->w,pic->h,
          int(320+(px-pic->w)*scale)/2,int(240+(py-pic->h)*scale)/2,
          int(pic->w*scale),int(pic->h*scale));

      if(vp_showpal)
        for(int i=0; i<256; i++)
          rectfill(buf,((i&15)<<2)+256,((i>>4)<<2)+176,((i&15)<<2)+259,((i>>4)<<2)+179,i);
      if(vp_showsize)
      {
        text_mode(pblack);
        textprintf(buf,font,0,232,pwhite,"%dx%d %1.2f%%",pic->w,pic->h,scale);
      }
      vsync();
      blit(buf,screen,0,0,0,0,320,240);
      redraw=false;
    }

    int step = 4;
    if(scale < 1.0)
      step = int(4.0/scale);
    if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      step <<= 2;
    if(key[KEY_LCONTROL] || key[KEY_RCONTROL])
      step = 1;

    if(key[KEY_UP])    { py+=step; redraw=true; }
    if(key[KEY_DOWN])  { py-=step; redraw=true; }
    if(key[KEY_LEFT])  { px+=step; redraw=true; }
    if(key[KEY_RIGHT]) { px-=step; redraw=true; }

    if(keypressed() && !redraw)
      switch(readkey()>>8) {
      case KEY_PGUP:  scale*=0.95; if(scale<0.1) scale=0.1; redraw=true; break;
      case KEY_PGDN:  scale/=0.95; if(scale>5.0) scale=5.0; redraw=true; break;
      case KEY_HOME:  scale/=2.0; if(scale<0.1) scale=0.1; redraw=true; break;
      case KEY_END:   scale*=2.0; if(scale>5.0) scale=5.0; redraw=true; break;
      case KEY_TILDE: scale=0.5; redraw=true; break;
      case KEY_Z:     px=pic->w-320; py=pic->h-240; vp_center=false; redraw=true; break;
      case KEY_1:     scale=1.0; redraw=true; break;
      case KEY_2:     scale=2.0; redraw=true; break;
      case KEY_3:     scale=3.0; redraw=true; break;
      case KEY_4:     scale=4.0; redraw=true; break;
      case KEY_5:     scale=5.0; redraw=true; break;
      case KEY_C:     px=py=0; redraw=vp_center=true; break;
      case KEY_S:     vp_showsize = !vp_showsize; redraw=true; break;
      case KEY_D:     vp_showpal = !vp_showpal; redraw=true; break;
      case KEY_P:
      case KEY_ESC:   done=true; break;
      case KEY_SPACE:
        if(load_the_pic()==2)
          done=true;
        else
        {
          redraw=true;
          gui_bg_color = pblack;
          gui_fg_color = pwhite;
          scale=1.0;
          set_palette(picpal);
        }
        break;
      }
  } while(!done);

  gui_bg_color = bg;
  gui_fg_color = fg;
  destroy_bitmap(buf);
  set_palette(RAMpal);
  set_mouse_sprite(mouse_bmp[0]);
  unscare_mouse();
  comeback();
  return D_O_K;
}



static DIALOG loadmap_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 48,   72,   224,  112,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  80,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "View Map" },
 { d_text_proc,       80,   96,   96,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Res - Size" },
// 3
 { d_radio_proc,      64,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1/4 - 352KB" },
 { d_radio_proc,      64,   112,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1/2 - 1.4MB" },
 { d_radio_proc,      64,   120,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full- 5.5MB" },
 { d_text_proc,       192,  96,   96,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Options" },
// 7
 { d_check_proc,      192,  104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Walk  " },
 { d_check_proc,      192,  112,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flags " },
 { d_check_proc,      192,  120,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Dark  " },
 { d_check_proc,      192,  128,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Items " },
// 11
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int load_the_map()
{
  static int res = 1;
  static int flags = cDEBUG;

  loadmap_dlg[3].flags  = (res==2) ? D_SELECTED : 0;
  loadmap_dlg[4].flags  = (res==1) ? D_SELECTED : 0;
  loadmap_dlg[5].flags  = (res==0) ? D_SELECTED : 0;
  loadmap_dlg[7].flags  = (flags&cWALK)   ? D_SELECTED : 0;
  loadmap_dlg[8].flags  = (flags&cFLAGS)  ? D_SELECTED : 0;
  loadmap_dlg[9].flags  = (flags&cNODARK) ? 0 : D_SELECTED;
  loadmap_dlg[10].flags = (flags&cNOITEM) ? 0 : D_SELECTED;

  if(popup_dialog(loadmap_dlg,11) != 11)
    return 1;

  flags = cDEBUG;
  if(loadmap_dlg[3].flags&D_SELECTED)  res=2;
  if(loadmap_dlg[4].flags&D_SELECTED)  res=1;
  if(loadmap_dlg[5].flags&D_SELECTED)  res=0;
  if(loadmap_dlg[7].flags&D_SELECTED)  flags|=cWALK;
  if(loadmap_dlg[8].flags&D_SELECTED)  flags|=cFLAGS;
  if(!(loadmap_dlg[9].flags&D_SELECTED))  flags|=cNODARK;
  if(!(loadmap_dlg[10].flags&D_SELECTED)) flags|=cNOITEM;


  if(pic)
    destroy_bitmap(pic);

  pic = create_bitmap((256*16)>>res,(176*8)>>res);

  if(!pic)
  {
    alert("Error creating bitmap.",NULL,NULL,"OK",NULL,13,27);
    return 2;
  }

  for(int y=0; y<8; y++)
    for(int x=0; x<16; x++)
    {
      Map.draw(screen2, 0, 0, flags, y*16+x);
      stretch_blit(screen2, pic, 0, 0, 256, 176,
                   x<<(8-res), (y*176)>>res, 256>>res,176>>res);
    }

  picpal = RAMpal;
  vp_showpal = false;
  get_bw(picpal,pblack,pwhite);
  px = py = 0;
  scale = 0.125*(1<<res);
  imagepath[0] = 0;
  return 0;
}



int onViewMap()
{
  if(load_the_map()==0)
    onViewPic();
  return D_O_K;
}

static char *dirstr[4] = {"up","down","left","right"};
char _pathstr[25]="up,up,up,up";

char *pathstr(byte path[])
{
  sprintf(_pathstr,"%s,%s,%s,%s",dirstr[path[0]],dirstr[path[1]],
   dirstr[path[2]],dirstr[path[3]]);
  return _pathstr;
}



void refresh(int flags)
{
 CPage = Map.CurrScr()->cpage;

 if(flags&rCLEAR)
  clear_to_color(menu,vc(0));

 if(flags&rMAP)
 {
  Map.setCurrScr(Map.getCurrScr());  // to update palette
  Map.draw(menu,0,16,Flags);
  if (ShowMisalignments) {
  }
 }

 if(flags&rSCRMAP)
 {
  text_mode(vc(0));
  if(Map.getCurrMap()<MAXMAPS)
  {
    for(int i=0; i<MAPSCRS; i++)
    {
     rectfill(menu,(i&15)*4,(i/16)*4+204,(i&15)*4+3,(i/16)*4+207,
       Map.Scr(i)->valid&mVALID ? lc1((Map.Scr(i)->color)&15) : vc(0));
     if(Map.Scr(i)->valid&mVALID&&((Map.Scr(i)->color)&15)>0)
       putpixel(menu,(i&15)*4+1,(i/16)*4+205,lc2((Map.Scr(i)->color)&15));
    }
    int s=Map.getCurrScr();
    rect(menu,(s&15)*4,(s/16)*4+204,(s&15)*4+3,(s/16)*4+207,vc(15));
    textprintf(menu,font,8,196,vc(11),"%-3d  %02X",Map.getCurrMap()+1,s);
    textout(menu,font,"M",0,196,vc(15));
    textout(menu,font,"S",40,196,vc(15));
  }
 }

 if(flags&rCOMBOS)
 {
   int drawmap, drawscr;
   drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
   drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
   for(int i=0; i<56; i++) {
     put_combo(menu,(i&3)*16+256,(i/4)*16+16,(CSet<<8)+i+First,0,
       (CurrentLayer==0)?CPage:Map.AbsoluteScr(drawmap, drawscr)->cpage,0);
   }
 }

 if(flags&rCOMBO) {
   int drawmap, drawscr;
   drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
   drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
   put_combo(menu,304,0,(CSet<<8)+Combo,0,
     (CurrentLayer==0)?CPage:Map.AbsoluteScr(drawmap, drawscr)->cpage,0);
 }

 if(flags&rMENU)
 {
  rectfill(menu,64,192,255,239,dvc(9));
  rect(menu,64,192,255,239,vc(15));
  rect(menu,241,194,253,237,vc(1));
  rect(menu,241,216,253,237,vc(1));
  text_mode(-1);
  set_clip(menu,65,193,254,238);
  mapscr *scr=Map.CurrScr();
  textprintf(menu,font,246,195,vc(9),"%d",menutype+1);
  switch(menutype)
  {
   case m_block: {
    char name[13];
    extract_name(filepath,name,FILENAME8__);
    textprintf(menu,font,72,200,vc(7),"File:");
    textprintf(menu,font,120,200,vc(11),"%s",name);
    textprintf(menu,font,72,216,vc(7),"Combo Type:");
    textprintf(menu,font,72,224,vc(11),"%s",combotype_string[combobuf[Combo+(CPage<<8)].type]);
    put_combo(menu,192,200,(CSet<<8)+Combo,cWALK,CPage,0);
    put_combo(menu,216,200,(CSet<<8)+Combo,0,CPage,0);
    textprintf_centre(menu,sfont,224,216,vc(15),"%dx%d", BrushWidth, BrushHeight);
    textprintf(menu,font,208,224,vc(11),"%3d",Combo);
    textprintf(menu,font,192,224,vc(11),"%d:",CPage);
    textprintf(menu,font,240,208,vc(11),"%d",CSet);
    } break;
   case m_coords:
    draw_sprite(menu,mouse_bmp[4],80,198);
    draw_sprite(menu,mouse_bmp[5],112,198);
    draw_sprite(menu,mouse_bmp[6],144,198);
    draw_sprite(menu,mouse_bmp[8],176,198);
    draw_sprite(menu,mouse_bmp[7],208,198);
    textprintf(menu,font,73,218,vc(11),"%3d %3d %3d %3d %3d",scr->itemx,scr->stairx,scr->warpx,scr->warpx2,Flag);
    textprintf(menu,font,73,226,vc(11),"%3d %3d %3d %3d",scr->itemy,scr->stairy,scr->warpy,scr->warpy2);
    break;
   case m_flags:
    {
    byte f=scr->flags;
    byte wf=scr->flags2;
    byte f3=scr->flags3;
    textprintf(menu,font,64,198,vc(7),"       DHETGSOMLWRDIBI");
    textprintf(menu,font,64,206,vc(7),"Flags:");
    textprintf(menu,font,120,206,vc(11),"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
      bit(f3,1),bit(f3,0),bit(wf,7),bit(wf,6),bit(wf,5),bit(wf,4),
      bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0),
      bit(f3,2));

    f=scr->enemyflags;
    textprintf(menu,font,72,218,vc(7),"       BILFR24Z P:");
    textprintf(menu,font,72,226,vc(7),"Enemy:");
    textprintf(menu,font,128,226,vc(11),"%d%d%d%d%d%d%d%d %s",
      bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0),
      short_pattern_string[(Map.CurrScr()->pattern)]);
//      Map.CurrScr()->pattern == 0 ? "random" : "sides");
    }
    break;
   case m_guy:
    {
    textprintf(menu,font,71,200,vc(7),"Guy:");
    textprintf(menu,font,71,208,vc(7),"Str:");
    textprintf(menu,font,71,216,vc(7),"Room:");
    textprintf(menu,font,111,200,vc(11),"%s",guy_string[scr->guy]);
    textprintf(menu,font,111,208,vc(11),"%s",MsgString(scr->str));
    textprintf(menu,font,119,216,vc(11),"%s",roomtype_string[scr->room]);
    int rtype=scr->room;
    if(strcmp(catchall_string[rtype]," ")) {
      textprintf(menu,font,71,224,vc(7),"%s:",catchall_string[rtype]);
      int xofs=text_length(font,catchall_string[rtype])+16;
      switch(rtype) {
       case 1:
        textprintf(menu,font,71+xofs,224,vc(11),"%s",item_string[scr->catchall]);
        break;
       default:
        textprintf(menu,font,71+xofs,224,vc(11),"%d",scr->catchall);
       }
      }
    }
    break;
   case m_warp:
    textprintf(menu,font,71,200,vc(7),"Tile Warp:");
    textprintf(menu,font,71,208,vc(7),"Type:");
    textprintf(menu,font,71,216,vc(7),"Side Warp:");
    textprintf(menu,font,71,224,vc(7),"Type:");
    textprintf(menu,font,159,200,vc(11),"%d:%02X",Map.CurrScr()->warpdmap,scr->warpscr);
    textprintf(menu,font,119,208,vc(11),"%s",warptype_string[scr->warptype]);
    textprintf(menu,font,159,216,vc(11),"%d:%02X",Map.CurrScr()->warpdmap2,scr->warpscr2);
    textprintf(menu,font,119,224,vc(11),"%s",warptype_string[scr->warptype2]);
    break;
   case m_misc: {
    textprintf(menu,font,71,206,vc(7),"Triggers:");
    byte wf=scr->flags2;
    textprintf(menu,font,151,198,vc(7),"RLBT");
    textprintf(menu,font,151,206,vc(11),"%d%d%d%d",
      bit(wf,3),bit(wf,2),bit(wf,1),bit(wf,0));

    textprintf(menu,font,71,218,vc(7),"Path:");
    textprintf(menu,font,71,226,vc(7),"Exit dir:");
    textprintf(menu,font,119,218,vc(11),"%s",scr->flags&64?pathstr(scr->path):"(none)");
    textprintf(menu,font,151,226,vc(11),"%s",scr->flags&64?dirstr[scr->exitdir]:"(none)");
    }
    break;
   case m_layers: {
    textprintf(menu,font,136,194,vc(7),"Layers");
    textprintf(menu,font,74,203,vc(7),"0");
    draw_checkbox(menu,73,212,dvc(9),vc(14), LayerMaskInt[0]);
    textprintf(menu,font,99,203,vc(7),"1");
    draw_checkbox(menu,98,212,dvc(9),vc(14), LayerMaskInt[1]);
    textprintf(menu,font,124,203,vc(7),"2");
    draw_checkbox(menu,123,212,dvc(9),vc(14), LayerMaskInt[2]);
    textprintf(menu,font,149,203,vc(7),"3");
    draw_checkbox(menu,148,212,dvc(9),vc(14), LayerMaskInt[3]);
    textprintf(menu,font,174,203,vc(7),"4");
    draw_checkbox(menu,173,212,dvc(9),vc(14), LayerMaskInt[4]);
    textprintf(menu,font,199,203,vc(7),"5");
    draw_checkbox(menu,198,212,dvc(9),vc(14), LayerMaskInt[5]);
    textprintf(menu,font,224,203,vc(7),"6");
    draw_checkbox(menu,223,212,dvc(9),vc(14), LayerMaskInt[6]);
    draw_layerradio(menu,73,222,dvc(9),vc(14), CurrentLayer);
    }
    break;
  } //switch(menutype)
  set_clip(menu,0,0,319,239);
 } //if(flags&rMENU)

 scare_mouse();
 if(flags&rCLEAR)
   blit(menu,screen,0,0,0,0,320,240);
 else {
   blit(menu,screen,0,16,0,16,320,224);
   if(flags&rCOMBO)
     blit(menu,screen,304,0,304,0,16,16);
   }
 if(ComboBrush && isinRect(mouse_x,mouse_y,0,16,255,191)) {
   set_mouse_sprite(mouse_bmp[13]);
   int mx=mouse_x/16*16;
   int my=mouse_y/16*16;
   if (FloatBrush) {
     clear(brushscreen);
     blit(brushbmp, brushscreen, 0, 0, mx-2, my-18, 16, 16);
     masked_blit(brushscreen, screen, 0, 0, 0, 16, 256, 192);
     //shadow
     for(int i=0; i<SHADOW_DEPTH; i++)
      for(int j=0; j<BrushHeight*16; j++)
       if((((i^j)&1)==1) && (my+j)<192)
        putpixel(screen,mx+i+16-SHADOW_DEPTH,my+j,vc(0));
     for(int i=0; i<BrushWidth*16; i++)
      for(int j=0; j<SHADOW_DEPTH; j++)
       if((((i^j)&1)==1) && (mx+i)<256)
        putpixel(screen,mx+i,my+j+16-SHADOW_DEPTH,vc(0));
   } else {
     clear(brushscreen);
     blit(brushbmp, brushscreen, 0, 0, mx, my-16, 16, 16);
     masked_blit(brushscreen, screen, 0, 0, 0, 16, 256, 192);
   }
 }
 unscare_mouse();
 SCRFIX();
}


void select_scr()
{
  if(Map.getCurrMap()>=MAXMAPS)
   return;

  while(mouse_b) {
   int x=mouse_x; if(x>63) x=63;
   int y=mouse_y; if(y<204) y=204;
   int s=(((y-204)>>2)<<4)+(x>>2);
   if(s>=MAPSCRS)
     s-=16;
   if(s!=Map.getCurrScr()) {
     Map.setCurrScr(s);
     refresh(rALL);
     }
   }
}


void select_combo()
{
  while(mouse_b)
  {
   int x=mouse_x; if(x<256) x=256;
   int y=mouse_y; if(y<16) y=16;
   Combo=(((y-16)>>4)<<2)+((x-256)>>4)+First;
   vsync();
   refresh(rMENU+rCOMBO);
  }
}


void draw()
{
 saved=false;
 Map.Ugo();
 int drawmap, drawscr;
 if (CurrentLayer==0) {
   drawmap=Map.getCurrMap();
   drawscr=Map.getCurrScr();
 } else {
   drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
   drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
   if (drawmap<0) {
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
 while(mouse_b)
 {
  int x=mouse_x;
  int y=mouse_y;
  if(isinRect(x,y,0,16,255,191))
  {
   word dat = Map.AbsoluteScr(drawmap, drawscr)->data[((y-16)&0xF0)+(x>>4)];
   dat &= 0xF800;
   dat |= (CSet<<8)+(Combo&0xFF);
   Map.AbsoluteScr(drawmap, drawscr)->data[((y-16)&0xF0)+(x>>4)]=dat;
   vsync();
   refresh(rMAP);
  }
 }
}

/*
void draw()
{
 saved=false;
 Map.Ugo();
 if(!(Map.CurrScr()->valid&mVALID))
 {
   Map.CurrScr()->valid|=mVALID;
   Map.setcolor(Color);
 }
 refresh(rMAP+rSCRMAP);
 while(mouse_b)
 {
  int x=mouse_x;
  int y=mouse_y;
  if(isinRect(x,y,0,16,255,191))
  {
   word dat = Map.CurrScr()->data[((y-16)&0xF0)+(x>>4)];
   dat &= 0xF800;
   dat |= (CSet<<8)+(Combo&0xFF);
   Map.CurrScr()->data[((y-16)&0xF0)+(x>>4)]=dat;
   vsync();
   refresh(rMAP);
  }
 }
}
*/

void replace(int c)
{
  saved=false;
  Map.Ugo();
  int drawmap, drawscr;
  if (CurrentLayer==0) {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  } else {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0) {
      return;
    }
  }

  int target = Map.AbsoluteScr(drawmap, drawscr)->data[c]&0x7FF;

  for(int i=0; i<176; i++)
    if((Map.AbsoluteScr(drawmap, drawscr)->data[i]&0x7FF)==target)
    {
      Map.AbsoluteScr(drawmap, drawscr)->data[i]&=0xF800;
      Map.AbsoluteScr(drawmap, drawscr)->data[i]|=(CSet<<8)+(Combo&0xFF);
    }
  refresh(rMAP);
}


void draw_block(int start,int w,int h)
{
 saved=false;
 Map.Ugo();
  int drawmap, drawscr;
  if (CurrentLayer==0) {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  } else {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0) {
      return;
    }
  }

 if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
   Map.CurrScr()->valid|=mVALID;
   Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
   Map.setcolor(Color);
   }
 for(int y=0; y<h && (y<<4)+start < 176; y++)
   for(int x=0; x<w && (start&15)+x < 16; x++)
   {
     word dat = Map.AbsoluteScr(drawmap, drawscr)->data[start+(y<<4)+x];
     dat &= 0xF800;
     dat |= (CSet<<8)+((Combo+(y<<2)+x)&0xFF);
     Map.AbsoluteScr(drawmap, drawscr)->data[start+(y<<4)+x]=dat;
   }
 refresh(rMAP+rSCRMAP);
}

void fill(mapscr* fillscr, int target, int sx, int sy, int dir, int diagonal)
{
    if ((fillscr->data[((sy<<4)+sx)]&0x7FF)!=target)
      return;
     fillscr->data[((sy<<4)+sx)]&=0xF800;
     fillscr->data[((sy<<4)+sx)]|=(CSet<<8)+(Combo&0xFF);
    if((sy>0) && (dir!=down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx)]&0x7FF)==target))
      fill(fillscr, target, sx, sy-1, up, diagonal);
    if((sy<10) && (dir!=up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx)]&0x7FF)==target))
      fill(fillscr, target, sx, sy+1, down, diagonal);
    if((sx>0) && (dir!=right))// && ((Map.CurrScr()->data[((sy<<4)+sx-1)]&0x7FF)==target))
      fill(fillscr, target, sx-1, sy, left, diagonal);
    if((sx<15) && (dir!=left))// && ((Map.CurrScr()->data[((sy<<4)+sx+1)]&0x7FF)==target))
      fill(fillscr, target, sx+1, sy, right, diagonal);
    if (diagonal==1) {
        if((sy>0) && (sx>0) && (dir!=r_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx-1)]&0x7FF)==target))
          fill(fillscr, target, sx-1, sy-1, l_up, diagonal);
        if((sy<10) && (sx<15) && (dir!=l_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx+1)]&0x7FF)==target))
          fill(fillscr, target, sx+1, sy+1, r_down, diagonal);
        if((sx>0) && (sy<10) && (dir!=r_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx-1)]&0x7FF)==target))
          fill(fillscr, target, sx-1, sy+1, l_down, diagonal);
        if((sx<15) && (sy>0) && (dir!=l_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx+1)]&0x7FF)==target))
          fill(fillscr, target, sx+1, sy-1, r_up, diagonal);
    }

}

void fill2(mapscr* fillscr, int target, int sx, int sy, int dir, int diagonal)
{
    if((fillscr->data[((sy<<4)+sx)]&0x7FF)==target)
      return;
     fillscr->data[((sy<<4)+sx)]&=0xF800;
     fillscr->data[((sy<<4)+sx)]|=(CSet<<8)+(Combo&0xFF);
    if((sy>0) && (dir!=down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx)]&0x7FF)!=target))
      fill2(fillscr, target, sx, sy-1, up, diagonal);
    if((sy<10) && (dir!=up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx)]&0x7FF)!=target))
      fill2(fillscr, target, sx, sy+1, down, diagonal);
    if((sx>0) && (dir!=right))// && ((Map.CurrScr()->data[((sy<<4)+sx-1)]&0x7FF)!=target))
      fill2(fillscr, target, sx-1, sy, left, diagonal);
    if((sx<15) && (dir!=left))// && ((Map.CurrScr()->data[((sy<<4)+sx+1)]&0x7FF)!=target))
      fill2(fillscr, target, sx+1, sy, right, diagonal);
    if (diagonal==1) {
        if((sy>0) && (sx>0) && (dir!=r_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx-1)]&0x7FF)!=target))
          fill2(fillscr, target, sx-1, sy-1, l_up, diagonal);
        if((sy<10) && (sx<15) && (dir!=l_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx+1)]&0x7FF)!=target))
          fill2(fillscr, target, sx+1, sy+1, r_down, diagonal);
        if((sx>0) && (sy<10) && (dir!=r_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx-1)]&0x7FF)!=target))
          fill2(fillscr, target, sx-1, sy+1, l_down, diagonal);
        if((sx<15) && (sy>0) && (dir!=l_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx+1)]&0x7FF)!=target))
          fill2(fillscr, target, sx+1, sy-1, r_up, diagonal);
    }
/*
    if((sy>0) && (dir!=down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx)]&0x7FF)!=target))
      fill2(target, sx, sy-1, up, diagonal);
    if((sy<11) && (dir!=up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx)]&0x7FF)!=target))
      fill2(target, sx, sy+1, down, diagonal);
    if((sx>0) && (dir!=right))// && ((Map.CurrScr()->data[((sy<<4)+sx-1)]&0x7FF)!=target))
      fill2(target, sx-1, sy, left, diagonal);
    if((sx<16) && (dir!=left))// && ((Map.CurrScr()->data[((sy<<4)+sx+1)]&0x7FF)!=target))
      fill2(target, sx+1, sy, right, diagonal);
    if (diagonal==1) {
        if((sy>0) && (sx>0) && (dir!=r_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx-1)]&0x7FF)!=target))
          fill2(target, sx-1, sy-1, l_up, diagonal);
        if((sy<11) && (sx<16) && (dir!=l_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx+1)]&0x7FF)!=target))
          fill2(target, sx+1, sy+1, r_down, diagonal);
        if((sx>0) && (sy<11) && (dir!=r_up))// && ((Map.CurrScr()->data[(((sy+1)<<4)+sx-1)]&0x7FF)!=target))
          fill2(target, sx-1, sy+1, l_down, diagonal);
        if((sx<16) && (sy>0) && (dir!=l_down))// && ((Map.CurrScr()->data[(((sy-1)<<4)+sx+1)]&0x7FF)!=target))
          fill2(target, sx+1, sy-1, r_up, diagonal);
    }
*/
}

int d_wflag_proc(int msg,DIALOG *d,int c)
{
 switch(msg)
 {
 case MSG_DRAW: {
   int c=(d->flags&D_SELECTED)?d->fg:d->bg;
   if(d->d2==0)
     c=d->bg;
   rectfill(screen,d->x, d->y, d->x+d->w-1, d->y+d->h-1,c);
   }
   break;
 case MSG_CLICK: {
   d->flags^=D_SELECTED;
   int c=(d->flags&D_SELECTED)?d->fg:d->bg;
   if(d->d2==0)
     c=d->bg;
   scare_mouse();
   rectfill(screen,d->x, d->y, d->x+d->w-1, d->y+d->h-1,c);
   unscare_mouse();
   while(mouse_b);
   }
   break;
 }
 return D_O_K;
}


#include "zq_tiles.cc" // tile and combo code




/**************************/
/*****     Mouse      *****/
/**************************/



void doxypos(byte &px,byte &py,int color,int mask)
{
 scare_mouse();
 rectfill(screen,px,py+16,px+15,py+31,vc(color));
 set_mouse_sprite(mouse_bmp[1]);
 unscare_mouse();
 bool canedit=false;
 bool done=false;
 while(!done && !(mouse_b&2)) {
  int x=mouse_x;
  int y=mouse_y;
  if(!mouse_b)
    canedit=true;
  if(canedit && mouse_b==1 && isinRect(x,y,0,16,255,191)) {
    scare_mouse();
    set_mouse_range(0,16,247,183);
    blit(screen,menu,0,0,0,0,320,240);
    while(mouse_b==1) {
      vsync();
      x=mouse_x;
      y=mouse_y;
      blit(menu,screen,0,16,0,16,256,176);
      draw_sprite(screen,mouse_bmp[3],x&mask,y&mask);
      }
    if(mouse_b==0) {
      px=byte(x&mask);
      py=byte((y&mask)-16);
      Map.draw(menu,0,16,Flags);
      rectfill(menu,px,py+16,px+15,py+31,vc(color));
      }
    blit(menu,screen,0,16,0,16,256,176);
    set_mouse_range(0,0,319,239);
    unscare_mouse();
    done=true;
    }
  if(keypressed()) {
    switch(readkey()>>8) {
     case KEY_ESC:
     case KEY_ENTER:
       goto finished;
     }
    }
  }

 finished:
 set_mouse_sprite(mouse_bmp[0]);
 refresh(rMAP+rMENU);
 while(mouse_b);
}



void doflags()
{
 set_mouse_sprite(mouse_bmp[2]);
 int of=Flags;
 Flags=cFLAGS;
 refresh(rMAP);

 bool canedit=false;
 while(!(mouse_b&2)) {
  int x=mouse_x;
  int y=mouse_y;
  if(!mouse_b)
    canedit=true;
  if(canedit && mouse_b==1 && isinRect(x,y,0,16,255,191)) {
    saved=false;
    Map.CurrScr()->data[(x>>4)+((y-16)&0xF0)]&=0x07FF;
    Map.CurrScr()->sflag[(x>>4)+((y-16)&0xF0)]=Flag;
    refresh(rMAP);
    }
  if(keypressed()) {
    switch(readkey()>>8) {
     case KEY_ESC:
     case KEY_ENTER:     goto finished;
     case KEY_ASTERISK:  onTimes();  break;
     case KEY_SLASH_PAD:    onDivide(); break;
     case KEY_UP:        onUp();     break;
     case KEY_DOWN:      onDown();   break;
     case KEY_LEFT:      onLeft();   break;
     case KEY_RIGHT:     onRight();  break;
     }
    }
  }

 finished:
 Flags=of;
 set_mouse_sprite(mouse_bmp[0]);
 refresh(rMAP+rMENU);
 while(mouse_b);
}



void peek(int c)
{
  char buf1[40],buf2[60],buf3[40];
  int drawmap, drawscr;
  if (CurrentLayer==0) {
    drawmap=Map.getCurrMap();
    drawscr=Map.getCurrScr();
  } else {
    drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
    drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
    if (drawmap<0) {
      return;
    }
  }
  sprintf(buf1,"Pos:  %-3d  Combo: %-3d", c, Map.AbsoluteScr(drawmap, drawscr)->data[c]&0xFF);
  sprintf(buf2,"CSet: %-3d  Flags: %-3d", (Map.AbsoluteScr(drawmap, drawscr)->data[c]>>8)&7, Map.CurrScr()->sflag[c]);
  sprintf(buf3,"Combo type: %-9s",combotype_string[combobuf[(Map.AbsoluteScr(drawmap, drawscr)->data[c]&0xFF)+(Map.AbsoluteScr(drawmap, drawscr)->cpage<<8)].type]);
  alert(buf1,buf2,buf3,"OK",NULL,13,27);
}

void set_brush_width(int width);
void set_brush_height(int height);

int set_brush_width_1() {set_brush_width(1); return D_O_K;};
int set_brush_width_2() {set_brush_width(2); return D_O_K;};
int set_brush_width_3() {set_brush_width(3); return D_O_K;};
int set_brush_width_4() {set_brush_width(4); return D_O_K;};
int set_brush_width_5() {set_brush_width(5); return D_O_K;};
int set_brush_width_6() {set_brush_width(6); return D_O_K;};
int set_brush_width_7() {set_brush_width(7); return D_O_K;};
int set_brush_width_8() {set_brush_width(8); return D_O_K;};
int set_brush_width_9() {set_brush_width(9); return D_O_K;};
int set_brush_width_10() {set_brush_width(10); return D_O_K;};
int set_brush_width_11() {set_brush_width(11); return D_O_K;};
int set_brush_width_12() {set_brush_width(12); return D_O_K;};
int set_brush_width_13() {set_brush_width(13); return D_O_K;};
int set_brush_width_14() {set_brush_width(14); return D_O_K;};
int set_brush_width_15() {set_brush_width(15); return D_O_K;};
int set_brush_width_16() {set_brush_width(16); return D_O_K;};

int set_brush_height_1() {set_brush_height(1); return D_O_K;};
int set_brush_height_2() {set_brush_height(2); return D_O_K;};
int set_brush_height_3() {set_brush_height(3); return D_O_K;};
int set_brush_height_4() {set_brush_height(4); return D_O_K;};
int set_brush_height_5() {set_brush_height(5); return D_O_K;};
int set_brush_height_6() {set_brush_height(6); return D_O_K;};
int set_brush_height_7() {set_brush_height(7); return D_O_K;};
int set_brush_height_8() {set_brush_height(8); return D_O_K;};
int set_brush_height_9() {set_brush_height(9); return D_O_K;};
int set_brush_height_10() {set_brush_height(10); return D_O_K;};
int set_brush_height_11() {set_brush_height(11); return D_O_K;};

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


void flood() {
 saved=false;
 Map.Ugo();
// int start=0, w=0, h=0;
 int drawmap, drawscr;
 if (CurrentLayer==0) {
   drawmap=Map.getCurrMap();
   drawscr=Map.getCurrScr();
 } else {
  drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
  drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
  if (drawmap<0) {
    return;
  }
 }
 if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
   Map.CurrScr()->valid|=mVALID;
   Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
   Map.setcolor(Color);
   }
/* for(int y=0; y<h && (y<<4)+start < 176; y++)
   for(int x=0; x<w && (start&15)+x < 16; x++)
*/
  for(int i=0; i<176; i++)
   {
     word dat = Map.AbsoluteScr(drawmap, drawscr)->data[i];
     dat &= 0xF800;
     dat |= (CSet<<8)+(Combo&0xFF);
     Map.AbsoluteScr(drawmap, drawscr)->data[i]=dat;
   }
 refresh(rMAP+rSCRMAP);
}

void fill_4() {
    int drawmap, drawscr;
    if (CurrentLayer==0) {
      drawmap=Map.getCurrMap();
      drawscr=Map.getCurrScr();
    } else {
     drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
     drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      if (drawmap<0) {
        return;
      }
    }
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    if ((Map.AbsoluteScr(drawmap,drawscr)->data[(by<<4)+bx]&0x7FF)!=dat) {
      saved=false;
      Map.Ugo();
      if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
        Map.CurrScr()->valid|=mVALID;
        Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
        Map.setcolor(Color);
      }
      fill(Map.AbsoluteScr(drawmap, drawscr),
       (Map.AbsoluteScr(drawmap, drawscr)->data[(by<<4)+bx]&0x7FF), bx, by, 255, 0);
      refresh(rMAP+rSCRMAP);
    }
};

void fill_8() {
    int drawmap, drawscr;
    if (CurrentLayer==0) {
      drawmap=Map.getCurrMap();
      drawscr=Map.getCurrScr();
    } else {
     drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
     drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      if (drawmap<0) {
        return;
      }
    }
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    if ((Map.AbsoluteScr(drawmap,drawscr)->data[(by<<4)+bx]&0x7FF)!=dat) {
      saved=false;
      Map.Ugo();
      if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
        Map.CurrScr()->valid|=mVALID;
        Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
        Map.setcolor(Color);
      }
      fill(Map.AbsoluteScr(drawmap, drawscr),
       (Map.AbsoluteScr(drawmap, drawscr)->data[(by<<4)+bx]&0x7FF), bx, by, 255, 1);
      refresh(rMAP+rSCRMAP);
    }
};

/*
void fill_8() {
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    if ((Map.CurrScr()->data[(by<<4)+bx]&0x7FF)!=dat) {
      saved=false;
      Map.Ugo();
      if(!(Map.CurrScr()->valid&mVALID)) {
          Map.CurrScr()->valid|=mVALID;
          Map.setcolor(Color);
      }
      fill((Map.CurrScr()->data[(by<<4)+bx]&0x7FF), bx, by, 255, 1);
      refresh(rMAP+rSCRMAP);
    }
};
*/

void fill2_4() {
    int drawmap, drawscr;
    if (CurrentLayer==0) {
      drawmap=Map.getCurrMap();
      drawscr=Map.getCurrScr();
    } else {
     drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
     drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      if (drawmap<0) {
        return;
      }
    }
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    saved=false;
    Map.Ugo();
    if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
      Map.CurrScr()->valid|=mVALID;
      Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
      Map.setcolor(Color);
    }
    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    fill2(Map.AbsoluteScr(drawmap, drawscr), dat, bx, by, 255, 0);
    refresh(rMAP+rSCRMAP);
};

void fill2_8() {
    int drawmap, drawscr;
    if (CurrentLayer==0) {
      drawmap=Map.getCurrMap();
      drawscr=Map.getCurrScr();
    } else {
     drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
     drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      if (drawmap<0) {
        return;
      }
    }
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    saved=false;
    Map.Ugo();
    if(!(Map.AbsoluteScr(drawmap, drawscr)->valid&mVALID)) {
      Map.CurrScr()->valid|=mVALID;
      Map.AbsoluteScr(drawmap, drawscr)->valid|=mVALID;
      Map.setcolor(Color);
    }
    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    fill2(Map.AbsoluteScr(drawmap, drawscr), dat, bx, by, 255, 1);
    refresh(rMAP+rSCRMAP);
};

/*
void fill2_8() {
    int x=mouse_x;
    int y=mouse_y;
    int by= (((y&0xF0)-16)>>4);
    int bx= (x>>4);
    word dat;

    saved=false;
    Map.Ugo();
    if(!(Map.CurrScr()->valid&mVALID)) {
        Map.CurrScr()->valid|=mVALID;
        Map.setcolor(Color);
    }
    dat &= 0xF800;
    dat |= (CSet<<8)+(Combo&0xFF);
    fill2(dat, bx, by, 255, 1);
    refresh(rMAP+rSCRMAP);
};
*/
static MENU fill_menu[] =
{ 
   { "Flood",                   set_flood,   NULL },
   { "Fill (4-way)",            set_fill_4,  NULL },
   { "Fill (8-way)",            set_fill_8,  NULL },
   { "Fill2 (4-way)",           set_fill2_4, NULL },
   { "Fill2 (8-way)",           set_fill2_8, NULL },
   { NULL }
};

int set_flood() {
  for (int x=0; x<5; x++) {
    fill_menu[x].flags=0;
  }
  fill_menu[0].flags=D_SELECTED;
  fill_type=0;
  return D_O_K;
};

int set_fill_4() {
  for (int x=0; x<5; x++) {
    fill_menu[x].flags=0;
  }
  fill_menu[1].flags=D_SELECTED;
  fill_type=1;
  return D_O_K;
};

int set_fill_8() {
  for (int x=0; x<5; x++) {
    fill_menu[x].flags=0;
  }
  fill_menu[2].flags=D_SELECTED;
  fill_type=2;
  return D_O_K;
};

int set_fill2_4() {
  for (int x=0; x<5; x++) {
    fill_menu[x].flags=0;
  }
  fill_menu[3].flags=D_SELECTED;
  fill_type=3;
  return D_O_K;
};

int set_fill2_8() {
  for (int x=0; x<5; x++) {
    fill_menu[x].flags=0;
  }
  fill_menu[4].flags=D_SELECTED;
  fill_type=4;
  return D_O_K;
};



static MENU draw_rc_menu[] =
{ 
   { "Draw 1x2 Block",          NULL,  NULL },
   { "Draw 2x1 Block",          NULL,  NULL },
   { "Draw 2x2 Block",          NULL,  NULL },
   { "Draw 4x2 Block",          NULL,  NULL },
   { "" },
   { "Set Brush Width\t ",   NULL,  brush_width_menu },
   { "Set Brush Height\t ",  NULL,  brush_height_menu },
   { "" },
   { "Set Fill type\t ",     NULL,  fill_menu },
   { "" },
   { "Replace",                 NULL,  NULL },
   { "Properties",              NULL,  NULL },
   { NULL }
};

/*
static MENU draw_rc_menu[] =
{ 
   { "Draw 1x2 Block",    NULL,  NULL },
   { "Draw 2x1 Block",    NULL,  NULL },
   { "Draw 2x2 Block",    NULL,  NULL },
   { "Draw 4x2 Block",    NULL,  NULL },
   { "" },
   { "Flood",             NULL,  NULL },
   { "Fill (4-way)",      NULL,  NULL },
   { "Fill (8-way)",      NULL,  NULL },
   { "Fill2 (4-way)",     NULL,  NULL },
   { "Fill2 (8-way)",     NULL,  NULL },
   { "" },
   { "Replace",           NULL,  NULL },
   { "Properties",        NULL,  NULL },
   { NULL }
};
*/


void set_brush_width(int width) {
  for (int x=0; x<16; x++) {
    brush_width_menu[x].flags=0;
  }
  BrushWidth=width;
  brush_width_menu[width-1].flags=D_SELECTED;
  refresh(rALL);
}

void set_brush_height(int height) {
  for (int x=0; x<11; x++) {
    brush_height_menu[x].flags=0;
  }
  BrushHeight=height;
  brush_height_menu[height-1].flags=D_SELECTED;
  refresh(rALL);
}



void domouse()
{
 static bool mouse_down = false;
 static int scrolldelay = 0;

 int x=mouse_x;
 int y=mouse_y;
 int redraw=0;

 put_combo(brushbmp,0,0,(CSet<<8)+Combo,0,CPage,0);

 if(!isinRect(mouse_x,mouse_y,0,16,255,191)) {
   scare_mouse();
   set_mouse_sprite(mouse_bmp[0]);
   unscare_mouse();
 }

 scrolldelay++;
 if(MouseScroll && (x>=256) && (key[KEY_LSHIFT] || key[KEY_RSHIFT] || (scrolldelay&3)==0) )
 {
   if(x>=300 && y<16 && First)
   {
     First-=4;
     redraw|=rCOMBOS;
   }
   if(y==239 && First<256-56)
   {
     First+=4;
     redraw|=rCOMBOS;
   }
 }

 if(mouse_b==0)
 {
   mouse_down = false;
 }
 else if(mouse_b&1)
 {
   if(isinRect(x,y,0,16,255,191))
     if (!((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))) {
         draw();
     } else {
         switch (fill_type) {
             case 0: flood();   break;
             case 1: fill_4();  break;
             case 2: fill_8();  break;
             case 3: fill2_4(); break;
             case 4: fill2_8(); break;
         }
     }
   if(isinRect(x,y,0,204,63,239))
     select_scr();
   if(menutype==m_coords)
   {
     if(isinRect(x,y,80,198,95,213))
       doxypos(Map.CurrScr()->itemx,Map.CurrScr()->itemy,11,0xF8);
     if(isinRect(x,y,112,198,127,213))
       doxypos(Map.CurrScr()->stairx,Map.CurrScr()->stairy,14,0xF0);
     if(isinRect(x,y,144,198,159,213))
       doxypos(Map.CurrScr()->warpx,Map.CurrScr()->warpy,9,0xF8);
     if(isinRect(x,y,176,198,191,213))
       doxypos(Map.CurrScr()->warpx2,Map.CurrScr()->warpy2,10,0xF8);
     if(isinRect(x,y,208,198,223,213))
       doflags();
   } else if (menutype==m_layers) {
     if(isinRect(x, y, 73,212,81,220)) {
       do_checkbox(menu,73,212,dvc(9),vc(14), LayerMaskInt[0]);
     }
     if(isinRect(x, y, 98,212,106,220)) {
       do_checkbox(menu,98,212,dvc(9),vc(14), LayerMaskInt[1]);
     }
     if(isinRect(x, y, 123,212,131,220)) {
       do_checkbox(menu,123,212,dvc(9),vc(14), LayerMaskInt[2]);
     }
     if(isinRect(x, y, 148,212,156,220)) {
       do_checkbox(menu,148,212,dvc(9),vc(14), LayerMaskInt[3]);
     }
     if(isinRect(x, y, 173,212,181,220)) {
       do_checkbox(menu,173,212,dvc(9),vc(14), LayerMaskInt[4]);
     }
     if(isinRect(x, y, 198,212,206,220)) {
       do_checkbox(menu,198,212,dvc(9),vc(14), LayerMaskInt[5]);
     }
     if(isinRect(x, y, 223,212,231,220)) {
       do_checkbox(menu,223,212,dvc(9),vc(14), LayerMaskInt[6]);
     }
     if(isinRect(x, y, 73, 222, (6*25)+73+8,231)) {
       do_layerradio(menu,73,222,dvc(9),vc(14), CurrentLayer);
     }
     redraw|=rMENU;
   }
   if(isinRect(x,y,241,194,253,215) && First>0 && !mouse_down)
   {
     First-=min(First,52);
     redraw|=rCOMBOS;
   }
   if(isinRect(x,y,241,216,253,237) && First<200 && !mouse_down)
   {
     First+=min(200-First,52);
     redraw|=rCOMBOS;
   }
   if(x>=256 && y>=16)
     select_combo();
   mouse_down = true;
 }
 else if(mouse_b&2)
 {
   if(isinRect(x,y,0,16,255,191))
   {
     scare_mouse();
     set_mouse_sprite(mouse_bmp[0]);
     unscare_mouse();
     int b = ((y&0xF0)-16)+(x>>4);
     int m = popup_menu(draw_rc_menu,x,y);
/*
     int by= (((y&0xF0)-16)>>4);
     int bx= (x>>4);
     word dat;
*/
     switch(m)
     {
     case 0: draw_block(b,1,2);                 break;
     case 1: draw_block(b,2,1);                 break;
     case 2: draw_block(b,2,2);                 break;
     case 3: draw_block(b,4,2);                 break;
/*     case 8: flood();                           break;
     case 9: dat &= 0xF800;
             dat |= (CSet<<8)+(Combo&0xFF);
             if ((Map.CurrScr()->data[(by<<4)+bx]&0x7FF)!=dat) {
               saved=false;
               Map.Ugo();
               if(!(Map.CurrScr()->valid&mVALID)) {
                   Map.CurrScr()->valid|=mVALID;
                   Map.setcolor(Color);
               }
               fill((Map.CurrScr()->data[(by<<4)+bx]&0x7FF), bx, by, 255, 0);
               refresh(rMAP+rSCRMAP);
             } break;
     case 10: dat &= 0xF800;
             dat |= (CSet<<8)+(Combo&0xFF);
             if ((Map.CurrScr()->data[(by<<4)+bx]&0x7FF)!=dat) {
               saved=false;
               Map.Ugo();
               if(!(Map.CurrScr()->valid&mVALID)) {
                   Map.CurrScr()->valid|=mVALID;
                   Map.setcolor(Color);
               }
               fill((Map.CurrScr()->data[(by<<4)+bx]&0x7FF), bx, by, 255, 1);
               refresh(rMAP+rSCRMAP);
             } break;
     case 11: saved=false;
             Map.Ugo();
             if(!(Map.CurrScr()->valid&mVALID)) {
                 Map.CurrScr()->valid|=mVALID;
                 Map.setcolor(Color);
             }
             dat &= 0xF800;
             dat |= (CSet<<8)+(Combo&0xFF);
             fill2(dat, bx, by, 255, 0);
             refresh(rMAP+rSCRMAP); break;
     case 12: saved=false;
             Map.Ugo();
             if(!(Map.CurrScr()->valid&mVALID)) {
                 Map.CurrScr()->valid|=mVALID;
                 Map.setcolor(Color);
             }
             dat &= 0xF800;
             dat |= (CSet<<8)+(Combo&0xFF);
             fill2(dat, bx, by, 255, 1);
             refresh(rMAP+rSCRMAP); break;
*/
     case 10: replace(b);                        break;
     case 11: peek(b);                           break;

     }
   }
   else if(x>=256 && y>=16)
   {
     select_combo();
     if(mouse_x>=256 && mouse_y>=16)
     {
       reset_combo_animations();
       edit_combo(((mouse_y&0xF0)>>2)-4+((mouse_x-256)>>4)+First+(CPage<<8),true,CSet);
       setup_combo_animations();
       refresh(rALL);
     }
   }
   mouse_down = true;
 }

 if(redraw)
  refresh(redraw);
}


static DIALOG showpal_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 28,   72,   264,  96,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_bitmap_proc,     32,   76,   256,  64,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     130,  144,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
 { NULL }
};


int onShowPal()
{
 BITMAP *palbmp = create_bitmap(256,64);
 if(!palbmp)
   return D_O_K;
 for(int i=0; i<256; i++)
  rectfill(palbmp,(i&31)<<3,(i&0xE0)>>2,((i&31)<<3)+7,((i&0xE0)>>2)+7,i);
 showpal_dlg[1].dp=palbmp;
 popup_dialog(showpal_dlg,2);
 destroy_bitmap(palbmp);
 return D_O_K;
}


static DIALOG csetfix_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   80,   176,  96,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "CSet Fix" },
 { d_radio_proc,      104,  108,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Full Screen  " },
 { d_radio_proc,      104,  128,  64,   8,    vc(14),  vc(1),  0,       D_SELECTED, 0,             0,       (void *) "Dungeon Floor" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int onCSetFix()
{
  int s=2,x2=14,y2=9;

  if(popup_dialog(csetfix_dlg,-1)==4)
  {
    Map.Ugo();

    if(csetfix_dlg[2].flags&D_SELECTED)
    { s=0; x2=16; y2=11; }

    for(int y=s; y<y2; y++)
      for(int x=s; x<x2; x++)
      {
        Map.CurrScr()->data[(y<<4)+x] &= 0xF8FF;
        Map.CurrScr()->data[(y<<4)+x] += CSet<<8;
      }
    refresh(rMAP);
    saved = false;
  }
  return D_O_K;
}



static DIALOG template_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   80,   176,  96,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_bitmap_proc,     180,  104,  16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon Template" },
 { d_radio_proc,      104,  108,  64,   8,    vc(14),  vc(1),  0,       D_SELECTED, 0,             0,       (void *) "Floor:" },
 { d_radio_proc,      104,  128,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Floor" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


int onTemplate()
{
 if(Map.getCurrScr()==TEMPLATE)
   return D_O_K;

 BITMAP *floor_bmp = create_bitmap(16,16);
 if(!floor_bmp) return D_O_K;
 put_combo(floor_bmp,0,0,(CSet<<8)+Combo,0,CPage,0);
 template_dlg[1].dp=floor_bmp;
 if(popup_dialog(template_dlg,-1)==5) {
   saved=false;
   Map.Ugo();
   Map.Template((template_dlg[3].flags==D_SELECTED) ? (CSet<<8)+Combo : -1);
   refresh(rMAP+rSCRMAP);
   }
 destroy_bitmap(floor_bmp);
 return D_O_K;
}


static DIALOG cpage_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   20,   176,  212,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  28,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Combo Page" },
 { d_button_proc,     90,   182,  60,   16,   vc(14),  vc(1),  's',     D_EXIT,     0,             0,       (void *) "&Set" },
 { d_button_proc,     170,  182,  60,   16,   vc(14),  vc(1),  'c',     D_EXIT,     0,             0,       (void *) "&Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  'a',     D_EXIT,     0,             0,       (void *) "Set &All" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 6
 { d_radio_proc,       76,  44,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "0"   },
 { d_radio_proc,       76,  52,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1"   },
 { d_radio_proc,       76,  60,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2"   },
 { d_radio_proc,       76,  68,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3"   },
 { d_radio_proc,       76,  76,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4"   },
 { d_radio_proc,       76,  84,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "5"   },
 { d_radio_proc,       76,  92,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "6"   },
 { d_radio_proc,       76, 100,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "7"   },
 { d_radio_proc,       76, 108,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "8"   },
 { d_radio_proc,       76, 116,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "9"   },
 { d_radio_proc,       76, 124,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "10"  },
 { d_radio_proc,       76, 132,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "11"  },
 { d_radio_proc,       76, 140,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "12"  },
 { d_radio_proc,       76, 148,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "13"  },
 { d_radio_proc,       76, 156,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "14"  },
 { d_radio_proc,       76, 164,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "15"  },
 { d_radio_proc,      120,  44,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "16"  },
 { d_radio_proc,      120,  52,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "17"  },
 { d_radio_proc,      120,  60,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "18"  },
 { d_radio_proc,      120,  68,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "19"  },
 { d_radio_proc,      120,  76,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "20"  },
 { d_radio_proc,      120,  84,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "21"  },
 { d_radio_proc,      120,  92,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "22"  },
 { d_radio_proc,      120, 100,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "23"  },
 { d_radio_proc,      120, 108,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "24"  },
 { d_radio_proc,      120, 116,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "25"  },
 { d_radio_proc,      120, 124,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "26"  },
 { d_radio_proc,      120, 132,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "27"  },
 { d_radio_proc,      120, 140,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "28"  },
 { d_radio_proc,      120, 148,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "29"  },
 { d_radio_proc,      120, 156,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "30"  },
 { d_radio_proc,      120, 164,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "31"  },
 { d_radio_proc,      164,  44,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "32"  },
 { d_radio_proc,      164,  52,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "33"  },
 { d_radio_proc,      164,  60,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "34"  },
 { d_radio_proc,      164,  68,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "35"  },
 { d_radio_proc,      164,  76,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "36"  },
 { d_radio_proc,      164,  84,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "37"  },
 { d_radio_proc,      164,  92,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "38"  },
 { d_radio_proc,      164, 100,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "39"  },
 { d_radio_proc,      164, 108,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "40"  },
 { d_radio_proc,      164, 116,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "41"  },
 { d_radio_proc,      164, 124,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "42"  },
 { d_radio_proc,      164, 132,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "43"  },
 { d_radio_proc,      164, 140,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "44"  },
 { d_radio_proc,      164, 148,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "45"  },
 { d_radio_proc,      164, 156,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "46"  },
 { d_radio_proc,      164, 164,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "47"  },
 { d_radio_proc,      208,  44,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "48"  },
 { d_radio_proc,      208,  52,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "49"  },
 { d_radio_proc,      208,  60,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "50"  },
 { d_radio_proc,      208,  68,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "51"  },
 { d_radio_proc,      208,  76,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "52"  },
 { d_radio_proc,      208,  84,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "53"  },
 { d_radio_proc,      208,  92,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "54"  },
 { d_radio_proc,      208, 100,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "55"  },
 { d_radio_proc,      208, 108,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "56"  },
 { d_radio_proc,      208, 116,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "57"  },
 { d_radio_proc,      208, 124,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "58"  },
 { d_radio_proc,      208, 132,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "59"  },
 { d_radio_proc,      208, 140,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "60"  },
 { d_radio_proc,      208, 148,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "61"  },
 { d_radio_proc,      208, 156,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "62"  },
 { d_radio_proc,      208, 164,   32,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "63"  },
 { NULL }
};


int onComboPage()
{
 for(int i=0; i<64; i++)
   cpage_dlg[i+6].flags = Map.CurrScr()->cpage==i?D_SELECTED:0;

 int ret = popup_dialog(cpage_dlg,3);

 int p = 0;
 for(int i=0; i<64; i++)
   if(cpage_dlg[i+6].flags==D_SELECTED)
     p=i;

 if(ret==2) {
   saved=false;
   Map.CurrScr()->cpage = p;
   }
 if(ret==4 && alert("Set all combo pages","on this map?",NULL,"&Yes","&No",'y','n')==1) {
   saved=false;
   for(int i=0; i<=TEMPLATE; i++)
     Map.Scr(i)->cpage = p;
   }
 refresh(rALL);
 return D_O_K;
}


int d_sel_scombo_proc(int msg, DIALOG *d, int c)
{
  switch(msg)
  {
  case MSG_CLICK:
    while(mouse_b)
    {
      int x = min( max(mouse_x - d->x,0)>>4, 15);
      int y = min( max(mouse_y - d->y,0)&0xF0, 160);
      if(x+y != d->d1)
      {
        d->d1 = x+y;
        vsync();
        scare_mouse();
        d_sel_scombo_proc(MSG_DRAW,d,0);
        unscare_mouse();
      }
    }
    break;

  case MSG_DRAW: {
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
 { d_shadow_box_proc, 24,   16,   272,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_sel_scombo_proc, 32,   24,   256,  176,  0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     90,   208,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  208,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


void select_scombo(int &pos)
{
  go();
  Map.draw_template(screen2,0,0);
  sel_scombo_dlg[1].dp = screen2;
  sel_scombo_dlg[1].d1 = pos;

  while(mouse_b);

  if(do_dialog(sel_scombo_dlg,3)==2)
    pos = sel_scombo_dlg[1].d1;

  comeback();
}



int d_scombo_proc(int msg, DIALOG *d, int c)
{
  switch(msg)
  {
  case MSG_CLICK:
    select_scombo(d->d1);
    scare_mouse();
    d_scombo_proc(MSG_DRAW,d,0);
    unscare_mouse();
    break;

  case MSG_DRAW:
    BITMAP *buf = create_bitmap(16,16);
    if(buf)
    {
      clear(buf);
      Map.draw_secret(buf,d->d1);
      blit(buf,screen,0,0,d->x,d->y,16,16);
      destroy_bitmap(buf);
    }
    break;
  }

  return D_O_K;
}



static DIALOG secret_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60,   48,   200,  148,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Secret Combos" },
 { d_button_proc,     90,  170,   60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170, 170,   60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
// 4
 { d_scombo_proc,     120,  84,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     120, 104,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     120, 124,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     120, 144,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },

 { d_scombo_proc,     160,  84,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     180,  84,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     200,  84,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     220,  84,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },

 { d_scombo_proc,     160, 104,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     180, 104,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     200, 104,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     220, 104,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },

 { d_scombo_proc,     160, 124,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     180, 124,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     200, 124,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     220, 124,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },

 { d_scombo_proc,     160, 144,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     180, 144,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     200, 144,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_scombo_proc,     220, 144,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },

 { d_text_proc,       170,  72,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "16 - 31" },
 { d_text_proc,        72,  88,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Burn" },
 { d_text_proc,        72, 108,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Arrow" },
 { d_text_proc,        72, 128,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Bomb" },
 { d_text_proc,        72, 148,   16,   16,   vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Stair" },

 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
 { NULL }
};


int onSecretCombo()
{
 for(int i=0; i<20; i++)
   secret_dlg[i+4].d1 = Map.CurrScr()->secret[i];

 go();
 if(do_dialog(secret_dlg,3) == 2)
 {
   saved = false;
   for(int i=0; i<20; i++)
     Map.CurrScr()->secret[i] = secret_dlg[i+4].d1;
 }
 comeback();
 return D_O_K;
}




static DIALOG under_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   60,   176,  120,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  68,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Under Combo" },
 { d_bitmap_proc,     114,  94,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_bitmap_proc,     194,  94,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     90,   124,  60,   16,   vc(14),  vc(1),  's',     D_EXIT,     0,             0,       (void *) "&Set" },
 { d_button_proc,     170,  124,  60,   16,   vc(14),  vc(1),  'c',     D_EXIT,     0,             0,       (void *) "&Cancel" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  'a',     D_EXIT,     0,             0,       (void *) "Set &All" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
 { d_text_proc,       110,  84,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "New     Current" },
 { NULL }
};


int onUnderCombo()
{
 BITMAP *cur = create_bitmap(16,16);
 BITMAP *sel = create_bitmap(16,16);
 if(!cur || !sel)
   return D_O_K;

 put_combo(cur,0,0,Map.CurrScr()->under,0,CPage,0);
 put_combo(sel,0,0,(CSet<<8)+Combo,0,CPage,0);

 under_dlg[2].dp = sel;
 under_dlg[3].dp = cur;
 int ret = popup_dialog(under_dlg,-1);
 if(ret==4) {
   saved=false;
   Map.CurrScr()->under = (CSet<<8)+Combo;
   }
 if(ret==6 && alert("Set all under combos","on this map?",NULL,"&Yes","&No",'y','n')==1) {
   saved=false;
   for(int i=0; i<128; i++)
     Map.Scr(i)->under = (CSet<<8)+Combo;
   }

 destroy_bitmap(sel);
 destroy_bitmap(cur);
 return D_O_K;
}





static DIALOG list_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60-12,   40,   200+24,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_list_proc,       72-12,   60,   176+24,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};

static DIALOG ilist_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60,   40,   200,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_abclist_proc,    72,   60,   176,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


typedef struct item_struct {
  char *s;
  int i;
} item_struct;

item_struct bii[iMax];
int bii_cnt=-1;

void build_bii_list()
{
  bii[0].s = "(none)";
  bii[0].i = 0;
  bii_cnt=1;

  for(int i=1; i<iMax; i++)
  {
    bii[bii_cnt].s = item_string[i];
    bii[bii_cnt].i = i;
    bii_cnt++;
  }

  for(int i=1; i<bii_cnt-1; i++)
    for(int j=i+1; j<bii_cnt; j++)
      if(stricmp(bii[i].s,bii[j].s)>0)
        swap(bii[i],bii[j]);

}

char *itemlist(int index, int *list_size)
{
 if(index<0) {
   *list_size = bii_cnt;
   return NULL;
   }
 return bii[index].s;
}



int select_item(char *prompt,int item)
{
 if(bii_cnt==-1)
   build_bii_list();

 int index=0;

 for(int j=0; j<bii_cnt; j++)
   if(bii[j].i == item)
     index=j;

 ilist_dlg[1].dp=prompt;
 ilist_dlg[2].d1=index;
 ilist_dlg[2].dp=itemlist;
 if(popup_dialog(ilist_dlg,2)==4)
   return -1;
 index = ilist_dlg[2].d1;
 return bii[index].i;
}


int select_data(char *prompt,int index,char *(proc)(int,int*))
{
 if(proc==NULL)
   return -1;
 list_dlg[1].dp=prompt;
 list_dlg[2].d1=index;
 list_dlg[2].dp=proc;
 if(popup_dialog(list_dlg,2)==4)
   return -1;
 return list_dlg[2].d1;
}


int select_data(char *prompt,int index,char *(proc)(int,int*),char *b1, char *b2)
{
 if(proc==NULL)
   return -1;
 list_dlg[1].dp=prompt;
 list_dlg[2].d1=index;
 list_dlg[2].dp=proc;
 list_dlg[3].dp=b1;
 list_dlg[4].dp=b2;
 int ret = popup_dialog(list_dlg,2);
 list_dlg[3].dp=(void *) "OK";
 list_dlg[4].dp=(void *) "Cancel";
 if(ret==4)
   return -1;
 return list_dlg[2].d1;
}


char *doors_string[7]={"wall","passage","locked","shutter","bombable","walk thru","1-way shutter"};

char *doorslist(int index, int *list_size)
{
 if(index>=0) {
   if(index>6)
    index=6;
   return doors_string[index];
   }
 *list_size=7;
 return NULL;
}


void edit_door(int side)
{
 int index=Map.CurrScr()->door[side];
 switch(index) {
   case 0:
   case 1:
   case 2: break;
   case 4: index=3; break;
   case 6: index=4; break;
   case 8: index=5; break;
   case 14: index=6; break;
   default: index=0;
   }
 int ret=select_data("Select Door Type",index,doorslist);
 if(ret!=-1) {
   switch(ret) {
     case 0:
     case 1:
     case 2: index=ret; break;
     case 3: index=4; break;
     case 4: index=6; break;
     case 5: index=8; break;
     case 6: index=14; break;
     default: index=0;
     }
   saved=false;
   Map.Ugo();
   Map.putdoor(side,index);
   refresh(rMAP);
   }
}



static DIALOG door_select_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 48,   48,   160,  112,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      128,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Select Door" },
 { d_button_proc,     118,  74,   20,   16,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
 { d_button_proc,     118,  110,  20,   16,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
 { d_button_proc,     82,   90,   16,   20,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
 { d_button_proc,     158,  90,   16,   20,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
 { d_button_proc,     150,  132,  48,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { NULL }
};


int onDoors()
{
 if(Map.getCurrScr()==TEMPLATE)
   return D_O_K;
 if(Map.getCurrMap()>=MAXMAPS)
   return D_O_K;

 int ret=0;
 do {
   ret = popup_dialog(door_select_dlg,-1);
   if(ret<6)
     edit_door(ret-2);
 } while(ret!=6);
 return D_O_K;
}



static DIALOG scrflags_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 8,    48,   304,  156,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Screen Flags" },
// 2
 { d_check_proc,      16,   72,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Block->Shutters " },
 { d_check_proc,      16,   80,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enemies->Item   " },
 { d_check_proc,      16,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Dark room       " },
 { d_check_proc,      16,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss roar SFX   " },

 { d_check_proc,      16,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Whistle->Stairs " },
 { d_check_proc,      16,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use ladder      " },
 { d_check_proc,      16,   120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Use maze path   " },
 { d_check_proc,      16,   128,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Ocean surf SFX  " },

 { d_check_proc,      16,   136,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Secret SFX      " },
 { d_check_proc,      16,   144,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Roar=Gasp style " },
 { d_check_proc,      16,   152,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Floating Traps  " },
 { d_check_proc,      16,   160,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enemies->Secret " },

 { d_check_proc,      168,  72,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Hold up item    " },
 { d_check_proc,      168,  80,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Roar=Dodongo    " },
 { d_check_proc,      168,  88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Invis. Enemies  " },

// 17
 { d_button_proc,     90,   176,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     170,  176,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
 { NULL }
};


int onScrFlags()
{
 byte f = Map.CurrScr()->flags;
 for(int i=0; i<8; i++)
 {
   scrflags_dlg[i+2].flags = (f&1) ? D_SELECTED : 0;
   f>>=1;
 }

 f = Map.CurrScr()->flags2 >> 4;
 for(int i=0; i<4; i++)
 {
   scrflags_dlg[i+10].flags = (f&1) ? D_SELECTED : 0;
   f>>=1;
 }

 f = Map.CurrScr()->flags3;
 for(int i=0; i<3; i++)
 {
   scrflags_dlg[i+14].flags = (f&1) ? D_SELECTED : 0;
   f>>=1;
 }


 if(popup_dialog(scrflags_dlg,-1)==17)
 {
   f=0;
   for(int i=7; i>=0; i--)
   {
     f<<=1;
     f |= scrflags_dlg[i+2].flags == D_SELECTED ? 1:0;
   }
   Map.CurrScr()->flags = f;

   f=0;
   for(int i=3; i>=0; i--)
   {
     f<<=1;
     f |= scrflags_dlg[i+10].flags == D_SELECTED ? 1:0;
   }
   Map.CurrScr()->flags2 &= 0x0F;
   Map.CurrScr()->flags2 |= f<<4;

   f=0;
   for(int i=2; i>=0; i--)
   {
     f<<=1;
     f |= scrflags_dlg[i+14].flags == D_SELECTED ? 1:0;
   }
   Map.CurrScr()->flags3 = f;

   refresh(rMAP+rSCRMAP+rMENU);
 }
 return D_O_K;
}



char *flaglist(int index, int *list_size)
{
 if(index>=0) {
   if(index>=MAXFLAGS)
    index=MAXFLAGS-1;
   return flag_string[index];
   }
 *list_size=MAXFLAGS;
 return NULL;
}


char *itemslist(int index, int *list_size)
{
 if(index >= 0)
 {
   if(index >= ITEMCNT)
     index = ITEMCNT-1;
   return item_string[index];
 }
 *list_size = ITEMCNT;
 return NULL;
}

char *weaponslist(int index, int *list_size)
{
 if(index >= 0)
 {
   if(index >= WPNCNT)
     index = WPNCNT-1;
   return weapon_string[index];
 }
 *list_size = WPNCNT;
 return NULL;
}

char *roomslist(int index, int *list_size)
{
 if(index>=0) {
   if(index>=MAXROOMTYPES)
    index=MAXROOMTYPES-1;
   return roomtype_string[index];
   }
 *list_size=MAXROOMTYPES;
 return NULL;
}

char *guyslist(int index, int *list_size)
{
 if(index>=0) {
   if(index >= zqMAXGUYS)
    index = zqMAXGUYS-1;
   return guy_string[index];
   }
 *list_size = zqMAXGUYS;
 return NULL;
}

char *msgslist(int index, int *list_size)
{
 if(index>=0) {
   if(index>=msg_count)
    index=msg_count-1;
   return MsgString(index);
   }
 *list_size=msg_count;
 return NULL;
}

static char number_str_buf[32];
int number_list_size=1;
bool number_list_zero=false;

char *numberlist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,number_list_size-1);
   sprintf(number_str_buf,"%d",index+(number_list_zero?0:1));
   return number_str_buf;
   }
 *list_size=number_list_size;
 return NULL;
}

static char dmap_str_buf[37];
int dmap_list_size=1;
bool dmap_list_zero=false;


char *dmaplist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,dmap_list_size-1);
   sprintf(dmap_str_buf,"%3d-%s",index+(dmap_list_zero?0:1), DMaps[index].name);
   return dmap_str_buf;
   }
 *list_size=dmap_list_size;
 return NULL;
}


char *hexnumlist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,number_list_size-1);
   sprintf(number_str_buf,"%X",index+(number_list_zero?0:1));
   return number_str_buf;
   }
 *list_size=number_list_size;
 return NULL;
}


char *maplist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,MAXMAPS-1);
   sprintf(number_str_buf,"%d",index+1);
   return number_str_buf;
   }
 *list_size=MAXMAPS;
 return NULL;
}


char *gotomaplist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,header.map_count-1);
   sprintf(number_str_buf,"%d",index+1);
   return number_str_buf;
   }
 *list_size = header.map_count;
 return NULL;
}


char *midilist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,MAXMIDIS_ZQ-1);
   return midi_string[index];
   }
 *list_size=MAXMIDIS_ZQ;
 return NULL;
}

char *levelnumlist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,255);
   sprintf(number_str_buf,"%.2X - %s",index,palnames[index]);
   return number_str_buf;
   }
 *list_size=MAXLEVELS;
 return NULL;
}


int onMapCount()
{
 int ret = select_data("Number of Maps",header.map_count-1,maplist);
 if(ret >= 0)
 {
   saved = false;
   setMapCount(ret+1);
 }
 refresh(rMAP+rSCRMAP+rMENU);
 return D_O_K;
}


int onGotoMap()
{
 int ret = select_data("Goto Map",Map.getCurrMap(),gotomaplist);
 if(ret >= 0)
   Map.setCurrMap(ret);
 refresh(rALL);
 return D_O_K;
}


int onFlags()
{
 int ret=select_data("Flag Type",Flag,flaglist);
 if(ret>=0)
 {
   Flag=ret;
   setFlagColor();
   refresh(rMENU);
   doflags();
 }
 return D_O_K;
}



int onItem()
{
 build_bii_list();
 int ret=select_item("Select Item",Map.CurrScr()->item);
 if(ret>=0) {
   saved=false;
   Map.CurrScr()->item=ret;
   }
 refresh(rMAP+rMENU);
 return D_O_K;
}

int onRType()
{
 int ret=select_data("Select Room Type",Map.CurrScr()->room,roomslist);
 if(ret>=0) {
   saved=false;
   Map.CurrScr()->room=ret;
   }
 int c=Map.CurrScr()->catchall;
 switch(Map.CurrScr()->room) {
  case 1: Map.CurrScr()->catchall=bound(c,0,ITEMCNT-1); break;
  // etc...
  }
 refresh(rMENU);
 return D_O_K;
}

int onGuy()
{
 int ret=select_data("Select Guy",Map.CurrScr()->guy,guyslist);
 if(ret>=0) {
   saved=false;
   Map.CurrScr()->guy=ret;
   }
 refresh(rMENU);
 return D_O_K;
}


int onString()
{
 int ret=select_data("Select Message String",Map.CurrScr()->str,msgslist);
 if(ret>=0) {
   saved=false;
   Map.CurrScr()->str=ret;
   }
 refresh(rMENU);
 return D_O_K;
}

int onEndString()
{
 int ret=select_data("Select Ending String",misc.endstring,msgslist);
 if(ret>=0) {
   saved=false;
   misc.endstring=ret;
   }
 refresh(rMENU);
 return D_O_K;
}


int onCatchall()
{
 if(data_menu[9].flags==D_DISABLED)
   return D_O_K;

 int ret=-1;
 int rtype=Map.CurrScr()->room;
 switch(rtype) {
  case 1:
   ret=select_data("Select Special Item",Map.CurrScr()->catchall,itemslist);
   break;
  default:
   char buf[80]="Enter ";
   strcat(buf,catchall_string[rtype]);
   ret=getnumber(buf,Map.CurrScr()->catchall);
   break;
  }
 if(ret>=0) {
   saved=false;
   Map.CurrScr()->catchall=ret;
   }
 refresh(rMENU);
 return D_O_K;
}

int onScreenPalette()
{
 int ret=gethexnumber("Screen Palette",Map.getcolor());
 bound(ret,0,255);
 if(ret>=0) {
   saved=false;
   Map.setcolor(ret);
   }
 refresh(rALL);
 return D_O_K;
}





bool edit_ins_mode=true;

void put_msg_str(char *s,int x,int y,int fg,int bg,int pos)
{
 int i=0;
 text_mode(bg);
 for(int dy=0; dy<3; dy++)
  for(int dx=0; dx<24; dx++) {
   if(edit_ins_mode) {
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),fg,"%c",*(s+i));
     }
   else {
     text_mode(i==pos?vc(15):bg);
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,"%c",*(s+i));
     }
   i++;
   }
 if(edit_ins_mode) {
   text_mode(-1);
   textprintf(screen,zfont,x+((pos%24)<<3),y+((pos/24)<<3),vc(15),"_");
   }
}


int d_msg_edit_proc(int msg,DIALOG *d,int c)
{
 char *s=(char*)(d->dp);
 switch(msg) {
 case MSG_WANTFOCUS:
   return D_WANTFOCUS;
 case MSG_CLICK:
   d->d2=((mouse_x-d->x)>>3)+((mouse_y-d->y)>>3)*24;
   bound(d->d2,0,71);
   scare_mouse();
   put_msg_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   unscare_mouse();
   while(mouse_b);
   break;
 case MSG_DRAW:
   put_msg_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   break;
 case MSG_CHAR:
   bool used=false;
   int k=c>>8;
   switch(k) {
   case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
   case KEY_HOME:  d->d2-=d->d2%24; used=true; break;
   case KEY_END:   d->d2-=d->d2%24; d->d2+=23; used=true; break;
   case KEY_UP:    if(d->d2>=24) d->d2-=24; used=true; break;
   case KEY_DOWN:  if(d->d2<48) d->d2+=24; used=true; break;
   case KEY_LEFT:  if(d->d2>0) d->d2--; used=true; break;
   case KEY_RIGHT: if(d->d2<71) d->d2++; used=true; break;
   case KEY_BACKSPACE:
     if(d->d2>0)
       d->d2--;
   case KEY_DEL:
     strcpy(s+d->d2,s+d->d2+1);
     s[71]=' ';
     s[72]=0;
     used=true;
     break;
   default:
     if(isprint(c&255)) {
       if(edit_ins_mode) {
         for(int i=71; i>d->d2; i--)
           s[i]=s[i-1];
         }
       s[d->d2]=c&255;
       if(d->d2<71)
         d->d2++;
       used=true;
     }
   }
   scare_mouse();
   put_msg_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   unscare_mouse();
   return used?D_USED_CHAR:D_O_K;
 }
 return D_O_K;
}

char msgbuf[73];

static DIALOG editmsg_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   68,   208,  96,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  75,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "String Editor" },
 { d_box_proc,        62,   98,   195,  27,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_msg_edit_proc,   64,   100,  192,  24,   vc(12),  vc(1),  0,       0,          0,             0,       msgbuf },
 { d_button_proc,     90,   140,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  140,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_text_proc,       64,   90,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       (void *) "123456789  ++  987654321" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


void editmsg(int index)
{
 char *si=MsgStrings[index].s;
 for(int i=0; i<72; i++) {
  msgbuf[i] = index<msg_count ? (*si ? *si : ' ') : ' ';
  si++;
  }
 msgbuf[72]=0;
 editmsg_dlg[3].d2=0;

 if(popup_dialog(editmsg_dlg,3)==4) {
   saved=false;
   strcpy(MsgStrings[index].s,msgbuf);
   if(index==msg_count)
     msg_count++;
   }
}

int strlist_del();

static DIALOG strlist_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   40,   192,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Edit Message String" },
 { d_list_proc,       80,   60,   160,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       msgslist },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       strlist_del },
 { NULL }
};

int strlist_del()
{
 if(strlist_dlg[2].d1>0 && strlist_dlg[2].d1<msg_count-1)
   return D_CLOSE;
 return D_O_K;
}


void fix_string(byte &str, byte index)
{
  if(str == index)
    str = 0;
  else if(str > index)
    str--;
}


int onStrings()
{
 int index=0;
 while(index!=-1) {
   bool hasroom=false;
   if(msg_count<MAXMSGS) {
     hasroom=true;
     strcpy(MsgStrings[msg_count++].s,"<new string>");
     }

   int ret=popup_dialog(strlist_dlg,2);
   index=strlist_dlg[2].d1;

   int doedit=false;

   switch(ret) {
    case 2:
    case 3: doedit=true; break;
    case 4: index=-1; break;
    case 5:
      char buf[25];
      strncpy(buf,MsgString(index),24);
      if(alert("Delete this message string?",buf,NULL,"Yes","No",'y',27)==1)
      {
        saved=false;
        for(int i=index; i<MAXMSGS-1; i++)
          MsgStrings[i]=MsgStrings[i+1];
        reset_msgstr(MAXMSGS-1);
        msg_count--;
        int sc = vbound(header.map_count,0,MAXMAPS)*MAPSCRS;

        for(int s=0; s<sc; s++)
          fix_string(TheMaps[s].str, index);
        for(int i=0; i<16; i++)
          for(int j=0; j<3; j++)
            fix_string(misc.info[i].str[j], index);

        refresh(rMENU);
      }
    }
   if(hasroom)
     strcpy(MsgStrings[--msg_count].s,"            ");

   if(index>0 && doedit) {
     editmsg(index);
     refresh(rMENU);
     }
   }
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
  d_button_proc(msg,d,c);
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
            d_button_proc(MSG_DRAW,tp,0);
          }
          tp++;
        }
        d->d1 = v;
        ((char*)(d->dp))[0] = v+'0';
      }
    }
    d->flags = 0;
    d_button_proc(MSG_DRAW,d,0);
    unscare_mouse();
  }
  return D_O_K;
}



static DIALOG tp_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   32,   208,  160,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Triforce Pieces" },
 { d_tri_frame_proc,  64,   56,   192,  96,   vc(11),  vc(1),  0,       0,          0,             0,       NULL },
// 3
 { d_tri_edit_proc,   138,  82,   16,   16,   vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_tri_edit_proc,   166,  82,   16,   16,   vc(14),  vc(1),  0,       0,          0,             1,       (void *) "2" },
 { d_tri_edit_proc,   90,   130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             2,       (void *) "3" },
 { d_tri_edit_proc,   214,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             3,       (void *) "4" },
// 7
 { d_tri_edit_proc,   138,  110,  16,   16,   vc(14),  vc(1),  0,       0,          0,             4,       (void *) "5" },
 { d_tri_edit_proc,   118,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             5,       (void *) "6" },
 { d_tri_edit_proc,   166,  110,  16,   16,   vc(14),  vc(1),  0,       0,          0,             6,       (void *) "7" },
 { d_tri_edit_proc,   186,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             7,       (void *) "8" },
// 11
 { d_button_proc,     90,   166,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  166,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int onTriPieces()
{
  for(int i=0; i<8; i++)
  {
    tp_dlg[i+3].d1 = misc.triforce[i];
    ((char*)(tp_dlg[i+3].dp))[0] = misc.triforce[i]+'0';
  }

  if(popup_dialog(tp_dlg,-1) == 11)
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



int d_hexedit_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_CHAR)
  if(((isalpha(c&255) && !isxdigit(c&255))) || ispunct(c&255))
    return D_USED_CHAR;
 return d_edit_proc(msg,d,isalpha(c&255)?c&0xDF:c);
}

int xtoi(char *hexstr)
{
 int val=0;
 while(isxdigit(*hexstr)) {
  val<<=4;
  if(*hexstr<='9')
    val += *hexstr-'0';
  else val+= ((*hexstr)|0x20)-'a'+10;
  hexstr++;
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
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],vc(1),vc(0),0);
  c=DMaps[dmap].compass;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,dvc(8));
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmOVERW:
  clear_to_color(dmapbmp,vc(7));
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*4+1,(c>>4)*4+1,(c&15)*4+3,(c>>4)*4+3,vc(10));
  break;

 case dmCAVE:
  for(int y=1; y<33; y+=4)
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],vc(8),vc(0),0);
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmBSOVERW:
  for(int y=1; y<33; y+=4)
    drawgrid_s(dmapbmp,1,y,DMaps[dmap].grid[y>>2],dvc(8),dvc(6),dvc(7));
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;
 }
}



int d_dmaplist_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_DRAW) {
   drawdmap(d->d1);
   int *xy = (int*)(d->dp3);
   if(xy[0]||xy[1])
     blit(dmapbmp,screen,0,0,xy[0],xy[1],65,33);
   text_mode(d->bg);
   if(xy[2]||xy[3])
     textprintf(screen,font,xy[2],xy[3],vc(11),"Map: %-3d",DMaps[d->d1].map+1);
   if(xy[4]||xy[5])
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level: %-3d",DMaps[d->d1].level);
   }
 return d_list_proc(msg,d,c);
}



int d_dropdmaplist_proc(int msg,DIALOG *d,int c)
{
 int d1 = d->d1;
 int ret = d_droplist_proc(msg,d,c);
 if(msg==MSG_DRAW || d->d1!=d1) {
   drawdmap(d->d1);
   int *xy = (int*)(d->dp3);
   if(xy[0]||xy[1])
     blit(dmapbmp,screen,0,0,xy[0],xy[1],65,33);
   text_mode(d->bg);
   if(xy[2]||xy[3])
     textprintf(screen,font,xy[2],xy[3],vc(11),"Map: %d",DMaps[d->d1].map+1);
   if(xy[4]||xy[5])
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level:%2d",DMaps[d->d1].level);
   }
 return ret;
}



int d_grid_proc(int msg,DIALOG *d,int c)
{
 switch(msg) {
 case MSG_DRAW:
   drawgrid(screen,d->x,d->y,d->d1,3,0,2);
   break;
 case MSG_CLICK:
   int x=(mouse_x-(d->x))/8;
   int y=(mouse_y-(d->y))/4;
   if(y==0 && x>=0 && x<8)
     d->d1 ^= (1 << (7-x));
   scare_mouse();
   drawgrid(screen,d->x,d->y,d->d1,3,0,2);
   unscare_mouse();
   while(mouse_b);
   break;
 }
 return D_O_K;
}


void drawxmap(int map,int xoff)
{
 clear_to_color(dmapbmp,CSET(0)+2);
 for(int y=0; y<8; y++)
   for(int x=0; x<8; x++)
     if(x+xoff>=0 && x+xoff<=15) {
       mapscr *scr = TheMaps + map*MAPSCRS + y*16+x+xoff;
       rectfill(dmapbmp,x*8,y*4,x*8+7,y*4+3,scr->valid&mVALID ? lc1((scr->color)&15) : 0);
       if(scr->valid&mVALID && ((scr->color)&15)>0)
         putpixel(dmapbmp,x*8+2,y*4+1,lc2((scr->color)&15));
       }
}


int d_xmaplist_proc(int msg,DIALOG *d,int c)
{
 int d1 = d->d1;
 int ret = d_droplist_proc(msg,d,c);
 if(msg==MSG_DRAW || d->d1!=d1) {
   int *xy = (int*)(d->dp3);
   xy[0]=d->d1;
   drawxmap(xy[0],xy[1]);
   if(xy[2]||xy[3])
     blit(dmapbmp,screen,0,0,xy[2],xy[3],64,32);
   }
 return ret;
}


int xmapspecs[4] = {0,0,84,118+32};


int onXslider(void *dp3,int d2)
{
 xmapspecs[1]=d2-7;
 bound(xmapspecs[1],-7,15);
 drawxmap(xmapspecs[0],xmapspecs[1]);
 scare_mouse();
 blit(dmapbmp,screen,0,0,xmapspecs[2],xmapspecs[3],64,32);
 unscare_mouse();
 return D_O_K;
}


char *dmaptype_str[dmMAX] = { "Dngn","Overw","Cave","BS-Ow" };

char *typelist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,dmMAX-1);
   return dmaptype_str[index];
   }
 *list_size=dmMAX;
 return NULL;
}

void put_title_str(char *s,int x,int y,int fg,int bg,int pos,int lines,int cpl)
{
 int i=0;
 text_mode(bg);
 for(int dy=0; dy<lines; dy++)
  for(int dx=0; dx<cpl; dx++) {
   if(edit_ins_mode) {
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),fg,"%c",*(s+i));
     }
   else {
     text_mode(i==pos?vc(15):bg);
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,"%c",*(s+i));
     }
   i++;
   }
 if(edit_ins_mode) {
   text_mode(-1);
   textprintf(screen,zfont,x+((pos%cpl)<<3),y+((pos/cpl)<<3),vc(15),"_");
   }
}


int d_title_edit_proc(int msg,DIALOG *d,int c)
{
 char *s=(char*)(d->dp);
 switch(msg) {
 case MSG_WANTFOCUS:
   return D_WANTFOCUS;
 case MSG_CLICK:
   d->d2=((mouse_x-d->x)>>3)+((mouse_y-d->y)>>3)*10;
   bound(d->d2,0,19);
   scare_mouse();
   put_title_str(s,d->x,d->y,d->fg,d->bg,d->d2,2,10);
   unscare_mouse();
   while(mouse_b);
   break;
 case MSG_DRAW:
   put_title_str(s,d->x,d->y,d->fg,d->bg,d->d2,2,10);
   break;
 case MSG_CHAR:
   bool used=false;
   int k=c>>8;
   switch(k) {
   case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
   case KEY_HOME:  d->d2-=d->d2%10; used=true; break;
   case KEY_END:   d->d2-=d->d2%10; d->d2+=9; used=true; break;
   case KEY_UP:    if(d->d2>=10) d->d2-=10; used=true; break;
   case KEY_DOWN:  if(d->d2<10) d->d2+=10; used=true; break;
   case KEY_LEFT:  if(d->d2>0) d->d2--; used=true; break;
   case KEY_RIGHT: if(d->d2<19) d->d2++; used=true; break;
   case KEY_BACKSPACE:
     if(d->d2>0)
       d->d2--;
   case KEY_DEL:
     strcpy(s+d->d2,s+d->d2+1);
     s[19]=' ';
     s[20]=0;
     used=true;
     break;
   default:
     if(isprint(c&255)) {
       if(edit_ins_mode) {
         for(int i=19; i>d->d2; i--)
           s[i]=s[i-1];
         }
       s[d->d2]=c&255;
       if(d->d2<19)
         d->d2++;
       used=true;
     }
   }
   scare_mouse();
   put_title_str(s,d->x,d->y,d->fg,d->bg,d->d2,2,10);
   unscare_mouse();
   return used?D_USED_CHAR:D_O_K;
 }
 return D_O_K;
}


void put_intro_str(char *s,int x,int y,int fg,int bg,int pos)
{
 int i=0;
 text_mode(bg);
 for(int dy=0; dy<3; dy++)
  for(int dx=0; dx<24; dx++) {
   if(edit_ins_mode) {
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),fg,"%c",*(s+i));
     }
   else {
     text_mode(i==pos?vc(15):bg);
     textprintf(screen,zfont,x+(dx<<3),y+(dy<<3),i==pos?bg:fg,"%c",*(s+i));
     }
   i++;
   }
 if(edit_ins_mode) {
   text_mode(-1);
   textprintf(screen,zfont,x+((pos%24)<<3),y+((pos/24)<<3),vc(15),"_");
   }
}


int d_intro_edit_proc(int msg,DIALOG *d,int c)
{
 char *s=(char*)(d->dp);
 switch(msg) {
 case MSG_WANTFOCUS:
   return D_WANTFOCUS;
 case MSG_CLICK:
   d->d2=((mouse_x-d->x)>>3)+((mouse_y-d->y)>>3)*24;
   bound(d->d2,0,71);
   scare_mouse();
   put_intro_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   unscare_mouse();
   while(mouse_b);
   break;
 case MSG_DRAW:
   put_intro_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   break;
 case MSG_CHAR:
   bool used=false;
   int k=c>>8;
   switch(k) {
   case KEY_INSERT: edit_ins_mode=!edit_ins_mode; used=true; break;
   case KEY_HOME:  d->d2-=d->d2%24; used=true; break;
   case KEY_END:   d->d2-=d->d2%24; d->d2+=23; used=true; break;
   case KEY_UP:    if(d->d2>=24) d->d2-=24; used=true; break;
   case KEY_DOWN:  if(d->d2<48) d->d2+=24; used=true; break;
   case KEY_LEFT:  if(d->d2>0) d->d2--; used=true; break;
   case KEY_RIGHT: if(d->d2<71) d->d2++; used=true; break;
   case KEY_BACKSPACE:
     if(d->d2>0)
       d->d2--;
   case KEY_DEL:
     strcpy(s+d->d2,s+d->d2+1);
     s[71]=' ';
     s[72]=0;
     used=true;
     break;
   default:
     if(isprint(c&255)) {
       if(edit_ins_mode) {
         for(int i=71; i>d->d2; i--)
           s[i]=s[i-1];
         }
       s[d->d2]=c&255;
       if(d->d2<71)
         d->d2++;
       used=true;
     }
   }
   scare_mouse();
   put_intro_str(s,d->x,d->y,d->fg,d->bg,d->d2);
   unscare_mouse();
   return used?D_USED_CHAR:D_O_K;
 }
 return D_O_K;
}



char dmap_title[21];
char dmap_name[33];
char dmap_intro[73];

static DIALOG editdmap_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 4,   32-12,   311,  160+32+12,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40-12,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "DMap Editor" },
 { d_text_proc,       64-50,   56-13,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Map:" },
 { d_text_proc,       64-50,   69-13,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Midi:" },
 { d_text_proc,       64-50,   80,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Level:" },
 { d_text_proc,       64-50,   82-13,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Color:" },
// { d_text_proc,       114-8,  80,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Color:" },
 { d_text_proc,       64-50,   90-1,   72,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Compass: 0x" },
// 7
 { d_xmaplist_proc,   104-50,  54-13,   48,   11,   vc(14),  vc(1),  0,       0,          1,             0,       maplist, NULL, xmapspecs},
 { d_droplist_proc,   112-50,  67-13,   144+96,  11,   vc(14),  vc(1),  0,       0,          1,             0,       midilist },
 //level
 { d_edit_proc,       64+6,  80,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 //color
 { d_droplist_proc,   64+6,  80-13,   176+24,  11,   vc(14),  vc(1),  0,       0,          1,             0,       levelnumlist },
// { d_hexedit_proc,    64+6,  82-12,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    160-50-8,  90-1,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 12
 { d_grid_proc,       173,  119+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  123+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  127+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  131+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  135+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  139+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  143+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  147+32,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
// 20
 { d_button_proc,     90,   168+32,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  168+32,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_slider_proc,     84,   152+32,  64,   8,    vc(14),  vc(1),  0,       0,          22,            0,       NULL, onXslider },
// 23
 { d_text_proc,       64-50+180,   90,   72,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Continue: 0x" },
 { d_hexedit_proc,    160-50+180,  90,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_text_proc,       160-50+96-16,  56-13,   48+16,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type: " },
 { d_droplist_proc,   192-50+96,  54-13,   64,   11,   vc(14),  vc(1),  0,       0,          1,             0,       typelist },
// { d_check_proc,      64-50,   104,  136,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Continue here: " },
 { d_check_proc,      194,     80,  112,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Cont. here: " },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { d_ctext_proc,      106-50,  110,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap Title" },
 { d_box_proc,        64-50,   118,  82,  18,    vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_title_edit_proc, 66-50,   120,  80,  16,    vc(12),  vc(1),  0,       0,          0,             0,       dmap_title },
 { d_text_proc,       64-50,   100,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name: " },
 { d_edit_proc,       112-50,  100,  168,  8,    vc(12),  vc(1),  0,       0,          20,            0,       NULL },
 { d_ctext_proc,      106+96-50+56,  110,  80,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap Intro" },
 { d_box_proc,        64+96-50,   118,  194, 26,    vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_intro_edit_proc, 66+96-50,   120,  192, 24,    vc(12),  vc(1),  0,       0,          0,             0,       dmap_intro },
 { NULL }
};



void editdmap(int index)
{
 char levelstr[4],compassstr[4],contstr[4];
 sprintf(levelstr,"%d",DMaps[index].level);
// sprintf(colorstr,"%X",DMaps[index].color);
 sprintf(compassstr,"%02X",DMaps[index].compass);
 sprintf(contstr,"%02X",DMaps[index].cont);
 sprintf(dmap_title,"%s",DMaps[index].title);
 sprintf(dmap_name,"%s",DMaps[index].name);
 sprintf(dmap_intro,"%s",DMaps[index].intro);

 editdmap_dlg[7].d1=DMaps[index].map;
 editdmap_dlg[8].d1=DMaps[index].midi;
 editdmap_dlg[10].d1=DMaps[index].color;
 editdmap_dlg[26].d1=(DMaps[index].type&dmfTYPE);
 editdmap_dlg[9].dp=levelstr;
// editdmap_dlg[10].dp=colorstr;
 editdmap_dlg[11].dp=compassstr;
 editdmap_dlg[24].dp=contstr;
 xmapspecs[1]=DMaps[index].xoff;
 editdmap_dlg[22].d2=DMaps[index].xoff+7;
 editdmap_dlg[27].flags = (DMaps[index].type&dmfCONTINUE) ? D_SELECTED : 0;
 editdmap_dlg[33].dp=dmap_name;

 for(int i=0; i<8; i++)
   editdmap_dlg[i+12].d1=DMaps[index].grid[i];


 char *si=MsgStrings[index].s;
 for(int i=0; i<72; i++) {
  msgbuf[i] = index<msg_count ? (*si ? *si : ' ') : ' ';
  si++;
  }
 msgbuf[72]=0;
 editdmap_dlg[29].d2=0;

/*
 if(popup_dialog(editmsg_dlg,3)==4) {
   saved=false;
   strcpy(MsgStrings[index].s,msgbuf);
   if(index==msg_count)
     msg_count++;
   }
*/

 if(popup_dialog(editdmap_dlg,-1)==20)
 {
   saved=false;
   for(int i=0; i<8; i++)
     DMaps[index].grid[i] = editdmap_dlg[i+12].d1;
   DMaps[index].map=editdmap_dlg[7].d1;
   DMaps[index].midi=editdmap_dlg[8].d1;
   DMaps[index].type=editdmap_dlg[26].d1|((editdmap_dlg[27].flags & D_SELECTED)?dmfCONTINUE:0);
   DMaps[index].level=vbound(atoi(levelstr),0,MAXLEVELS-1);
   DMaps[index].color=editdmap_dlg[10].d1;
//   DMaps[index].color=xtoi(colorstr);
   DMaps[index].compass=xtoi(compassstr);
   DMaps[index].cont=xtoi(contstr);
   DMaps[index].xoff=xmapspecs[1];
   sprintf(DMaps[index].title,"%s",dmap_title);
   sprintf(DMaps[index].name,"%s",dmap_name);
   sprintf(DMaps[index].intro,"%s",dmap_intro);
 }
}


/*
int selectdmapxy[6] = {164,108,164,80,164,90};

static DIALOG selectdmap_dlg[] =
{
 // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
 { d_shadow_box_proc, 72,   56,   176,  120,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  63,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Select DMap" },
 { d_dmaplist_proc,   90,   80,   64,   60,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       dmaplist, NULL, selectdmapxy },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, close_dlg },
 { NULL }
};
*/

int selectdmapxy[6] = {164-74,108+8+24,164,80+44+24,164,90+44+24};

static DIALOG selectdmap_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72-44,   56-30,   176+88,  120+60,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  63-30,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Select DMap" },
 { d_dmaplist_proc,   90-44,   80-30,   64+72+88,   60+24,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       dmaplist, NULL, selectdmapxy },
 { d_button_proc,     90,   152+30,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
 { d_button_proc,     170,  152+30,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, close_dlg },
 { NULL }
};




int onDmaps()
{
 int ret;
 char buf[40];
 dmap_list_size=MAXDMAPS;
 number_list_zero=true;

 while((ret=popup_dialog(selectdmap_dlg,2))!=4)
 {
   int d=selectdmap_dlg[2].d1;
   if(ret==5)
   {
     sprintf(buf,"Delete dmap %d?",d);
     if(alert(buf,NULL,NULL,"&Yes","&No",'y','n')==1)
     {
       reset_dmap(d);
       saved=false;
     }
   }
   else
     editdmap(d);
 }
 return D_O_K;
}



/*******************************/
/**********  onMidis  **********/
/*******************************/


static DIALOG editmidi_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 24,   48,   272,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "MIDI Specs" },
// 2
 { d_text_proc,       56,   94,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "MIDI:" },
 { d_text_proc,       104,  94,   48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   114,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name:" },
 { d_edit_proc,       104,  114,  160,  8,    vc(12),  vc(1),  0,       0,          19,            0,       NULL },
 { d_text_proc,       56,   124,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Volume:" },
 { d_edit_proc,       120,  124,  32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
// 8
 { d_check_proc,      176,  124,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Loop:   " },
// 9
 { d_button_proc,     50,   72,   56,   16,   vc(14),  vc(1),  'l',     D_EXIT,     0,             0,       (void *) "&Load" },
 { d_button_proc,     116,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) "#" },
 { d_button_proc,     156,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) ">" },
 { d_button_proc,     196,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) ">>" },
 { d_button_proc,     236,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       (void *) ">>>" },
// 14
 { d_text_proc,       56,   134,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Start:" },
 { d_edit_proc,       112,  134,  32,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
 { d_text_proc,       176,  134,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "L.Strt:" },
 { d_edit_proc,       240,  134,  40,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
 { d_text_proc,       176,  144,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "L.End:" },
 { d_edit_proc,       240,  144,  40,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
// 20
 { d_text_proc,       176,  94,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Pos:" },
 { d_text_proc,       216,  94,   32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       176,  104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Len:" },
 { d_text_proc,       216,  104,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Time:" },
 { d_text_proc,       104,  104,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
// 26
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
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

  do {
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
      vsync();
      scare_mouse();
      text_mode(vc(1));
      textprintf(screen,font,216,94,vc(11),"%-5ld",midi_pos);
      unscare_mouse();
    }
    ret = shutdown_dialog(p);

    loop = editmidi_dlg[8].flags?1:0;
    volume = vbound(atoi(volume_str),0,255);

    start = vbound(atol(start_str),0,max(Midi_Info.len_beats-4,0));
    loop_start = vbound(atol(loop_start_str),-1,max(Midi_Info.len_beats-4,-1));
    loop_end = vbound(atol(loop_end_str),-1,Midi_Info.len_beats);
    if(loop_end>0)
      loop_end = vbound(loop_end,max(loop_start+4,start+4),Midi_Info.len_beats);

    switch(ret)
    {
    case 9:
      if(getname("Load MIDI","mid",temppath,true))
      {
        stop_midi();
        if(midi!=NULL && midi!=customMIDIs[i].midi)
          destroy_midi(midi);
        if(!(midi=load_midi(temppath)))
          alert("Error loading MIDI:",temppath,NULL,"Dang",NULL,13,27);
        else
        {
          char *t = get_filename(temppath);
          int i=0;
          for( ; i<19 && t[i]!=0 && t[i]!='.'; i++)
            title[i]=t[i];
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
          pos = min(pos+16,Midi_Info.len_beats);
        else
          pos = min(pos+16,midi_loop_end);
        if(pos>0)
          midi_seek(pos);
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
          pos = min(pos+64,Midi_Info.len_beats);
        else
          pos = min(pos+64,midi_loop_end);
        if(pos>0)
          midi_seek(pos);
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
  } while(ret<26);

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
  if(ret==27 && midi!=customMIDIs[i].midi)
  {
    if(midi)
      destroy_midi(midi);
  }
}



int d_midilist_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_DRAW) {
   int i = d->d1;
   int x = d->x+d->w+12;
   int y = d->y+2;

   text_mode(d->bg);

   textout(screen,font,"MIDI:",x,y,d->fg);
   textout(screen,font,"Name:",x,y+8,d->fg);
   textout(screen,font,"Volume:",x,y+16,d->fg);
   textout(screen,font,"Loop:",x,y+24,d->fg);
   textout(screen,font,"Start:",x,y+32,d->fg);
   textout(screen,font,"Loop Start:",x,y+40,d->fg);
   textout(screen,font,"Loop End:",x,y+48,d->fg);

   textprintf(screen,font,x+48,y,vc(11),"%s",customMIDIs[i].midi?"Loaded":"Empty ");
   textprintf(screen,font,x+48,y+8,vc(11),"%-19s",customMIDIs[i].title);
   textprintf(screen,font,x+96,y+16,vc(11),"%-3d",customMIDIs[i].volume);
   textprintf(screen,font,x+96,y+24,vc(11),"%s",customMIDIs[i].loop?"On ":"Off");
   textprintf(screen,font,x+96,y+32,vc(11),"%-5ld",customMIDIs[i].start);
   textprintf(screen,font,x+96,y+40,vc(11),"%-5ld",customMIDIs[i].loop_start);
   textprintf(screen,font,x+96,y+48,vc(11),"%-5ld",customMIDIs[i].loop_end);
   }
 return d_list_proc(msg,d,c);
}


static DIALOG selectmidi_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 24,   48,   272,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Select MIDI" },
 { d_midilist_proc,   36,   76,   44,   68,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       numberlist },
 { d_button_proc,     78,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
 { d_button_proc,     158,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, close_dlg },
 { NULL }
};



int onMidis()
{
 int ret;
 char buf[40];
 number_list_size=MAXMIDIS;
 number_list_zero=false;
 strcpy(temppath,midipath);

 go();
 while((ret=do_dialog(selectmidi_dlg,2))!=4)
 {
   int d=selectmidi_dlg[2].d1;
   if(ret==5)
   {
     sprintf(buf,"Delete MIDI %d?",d+1);
     if(alert(buf,NULL,NULL,"&Yes","&No",'y','n')==1)
     {
       reset_midi(customMIDIs+d);
       saved=false;
     }
   }
   else
     edit_midi(d);
 }
 comeback();
 return D_O_K;
}




/*******************************/
/****  onTest/onTestOptions  ***/
/*******************************/

int onTest()
{


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
 if(index>=0) {
   if(index>=MAXWARPTYPES)
    index=MAXWARPTYPES-1;
   return warptype_string[index];
   }
 *list_size=MAXWARPTYPES;
 return NULL;
}


int warpdmapxy[6] = {188,126,188,100,188,112};


static DIALOG warp_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 45-32,   48,   230+64,  136+12,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  55,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   70,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
 { d_text_proc,       56-32,   88,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
 { d_text_proc,       56,   106,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
 { d_text_proc,       56,   120,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
 { d_box_proc,        137,  122,  21,   21,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
// 7
 { d_droplist_proc,   100,  68,   160,  11,   vc(14),  vc(1),  0,       0,          0,             0,       warptypelist },
 { d_dropdmaplist_proc,100-32, 86,   288-96+32,  11,   vc(14),  vc(1),  0,       0,          0,             0,       dmaplist, NULL, warpdmapxy },
 { d_hexedit_proc,    136,  106,  24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 10
 { d_wflag_proc,      141,  121,  14,   4,    vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      141,  141,  14,   4,    vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      136,  126,  4,    14,   vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      156,  126,  4,    14,   vc(12),  vc(7),  0,       0,          0,             0,       NULL },
// 14
 { d_button_proc,     70,   160+12,  40,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     130,  160+12,  40,   16,   vc(14),  vc(1),  'g',     D_EXIT,     0,             0,       (void *) "&Go" },
 { d_button_proc,     190,  160+12,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


/*

int warpdmapxy[6] = {188,114,188,88,188,100};


static DIALOG warp_dlg[] =
{
 // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
 { d_shadow_box_proc, 45,   48,   230,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  55,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   70,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
 { d_text_proc,       56,   88,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "DMap:" },
 { d_text_proc,       56,   106,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Screen: 0x" },
 { d_text_proc,       56,   120,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Triggers:" },
 { d_box_proc,        137,  122,  21,   21,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
// 7
 { d_droplist_proc,   100,  68,   160,  11,   vc(14),  vc(1),  0,       0,          0,             0,       warptypelist },
 { d_dropdmaplist_proc,100, 86,   64,   11,   vc(14),  vc(1),  0,       0,          0,             0,       numberlist, NULL, warpdmapxy },
 { d_hexedit_proc,    136,  106,  24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 10
 { d_wflag_proc,      141,  121,  14,   4,    vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      141,  141,  14,   4,    vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      136,  126,  4,    14,   vc(12),  vc(7),  0,       0,          0,             0,       NULL },
 { d_wflag_proc,      156,  126,  4,    14,   vc(12),  vc(7),  0,       0,          0,             0,       NULL },
// 14
 { d_button_proc,     70,   160,  40,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     130,  160,  40,   16,   vc(14),  vc(1),  'g',     D_EXIT,     0,             0,       (void *) "&Go" },
 { d_button_proc,     190,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};
*/


int onWarp()
{
 warp_dlg[1].dp=(void *) "Tile Warp";
 warp_dlg[7].flags = 0;

 char buf[10];
 sprintf(buf,"%02X",Map.CurrScr()->warpscr);
 warp_dlg[7].d1=Map.CurrScr()->warptype;
 warp_dlg[8].d1=Map.CurrScr()->warpdmap;
 warp_dlg[9].dp=buf;
 warp_dlg[2].fg=vc(14);
 warp_dlg[5].fg=vc(7);
 for(int i=0; i<4; i++)
   warp_dlg[10+i].d2 = 0;
 dmap_list_size=MAXDMAPS;
 dmap_list_zero=true;
 int ret=popup_dialog(warp_dlg,-1);
 if(ret==14 || ret==15) {
   saved=false;
   Map.CurrScr()->warpscr = xtoi(buf);
   Map.CurrScr()->warptype = warp_dlg[7].d1;
   Map.CurrScr()->warpdmap = warp_dlg[8].d1;
   refresh(rMENU);
   }
 if(ret==15) {
   Map.dowarp(0);
   refresh(rALL);
   }
 return D_O_K;
}




int onWarp2()
{
 warp_dlg[1].dp=(void *) "Side Warp";
 warp_dlg[7].flags = 0;

 char buf[10];
 sprintf(buf,"%02X",Map.CurrScr()->warpscr2);
 warp_dlg[7].d1=Map.CurrScr()->warptype2;
 warp_dlg[8].d1=Map.CurrScr()->warpdmap2;
 warp_dlg[9].dp=buf;
 warp_dlg[2].fg=warp_dlg[5].fg=vc(14);
 byte f=Map.CurrScr()->flags2;
 for(int i=0; i<4; i++) {
   warp_dlg[10+i].d2 = 1;
   warp_dlg[10+i].flags = f&1 ? D_SELECTED : 0;
   f>>=1;
   }
 dmap_list_size=MAXDMAPS;
 dmap_list_zero=true;
 int ret=popup_dialog(warp_dlg,-1);
 if(ret==14 || ret==15) {
   saved=false;
   Map.CurrScr()->warpscr2 = xtoi(buf);
   Map.CurrScr()->warptype2 = warp_dlg[7].d1;
   Map.CurrScr()->warpdmap2 = warp_dlg[8].d1;
   f=0;
   for(int i=3; i>=0; i--) {
     f<<=1;
     f |= warp_dlg[10+i].flags&D_SELECTED ? 1 : 0;
     }
   Map.CurrScr()->flags2 = f;
   refresh(rMENU);
   }
 if(ret==15) {
   Map.dowarp(1);
   refresh(rALL);
   }
 return D_O_K;
}



void EditWindWarp(int level)
{
 char caption[40],buf[10];

 sprintf(caption,"Whirlwind Warp %d",level+1);
 warp_dlg[1].dp = caption;
 warp_dlg[7].flags = D_DISABLED;

 sprintf(buf,"%02X",misc.wind[level].scr);
 warp_dlg[8].d1=misc.wind[level].dmap;
 warp_dlg[9].dp=buf;
 warp_dlg[2].fg=warp_dlg[5].fg=vc(7);
 for(int i=0; i<4; i++)
   warp_dlg[10+i].d2 = 0;
 dmap_list_size=MAXDMAPS;
 dmap_list_zero=true;
 int ret=popup_dialog(warp_dlg,-1);
 if(ret==14 || ret==15) {
   saved=false;
   misc.wind[level].dmap = warp_dlg[8].d1;
   misc.wind[level].scr = xtoi(buf);
   }
 if(ret==15) {
   Map.dowarp(2+level);
   refresh(rALL);
   }
}


int onWhistle()
{
  number_list_size = 9;
  number_list_zero = false;
  int index = select_data("Whistle Warps",0,numberlist,"Edit","Done");

  while(index!=-1) {
    EditWindWarp(index);
    number_list_size = 9;
    number_list_zero = false;
    index = select_data("Whistle Warps",index,numberlist,"Edit","Done");
    }

  return D_O_K;
}



/*******************************/
/*********** onPath ************/
/*******************************/



char *dirlist(int index, int *list_size)
{
 if(index>=0) {
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
 { d_shadow_box_proc, 80,   64,   160,  120,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  71,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Maze Path" },
 { d_text_proc,       94,   86,   192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1st" },
 { d_text_proc,       94,   100,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2nd" },
 { d_text_proc,       94,   114,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3rd" },
 { d_text_proc,       94,   128,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4th" },
 { d_text_proc,       94,   142,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Exit" },
 { d_droplist_proc,   140,  84,   80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  98,   80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  112,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  126,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  140,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_button_proc,     90,   156,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  156,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};



int onPath()
{
 for(int i=0; i<4; i++)
   path_dlg[i+7].d1 = Map.CurrScr()->path[i];
 path_dlg[11].d1 = Map.CurrScr()->exitdir;
 int ret=popup_dialog(path_dlg,7);
 if(ret==12) {
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


int d_nlist_proc(int msg,DIALOG *d,int c)
{
  int ret = d_droplist_proc(msg,d,c);
  switch(msg) {
  case MSG_DRAW:
  case MSG_CHAR:
  case MSG_CLICK:
    scare_mouse();
    text_mode(d->bg);
    textprintf(screen,font,d->x - 32,d->y + 2,vc(11),"%3d",d->d1);
    unscare_mouse();
  }
  return ret;
}

static DIALOG editinfo_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   40,   192,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  48,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       72,   64,   48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       (void *) "1st" },
 { d_text_proc,       72,   96,   48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       (void *) "2nd" },
 { d_text_proc,       72,   128,  48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       (void *) "3rd" },
 { d_text_proc,       104,  64,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Price:" },
 { d_text_proc,       104,  96,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Price:" },
 { d_text_proc,       104,  128,  88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Price:" },
// 8
 { d_edit_proc,       160,  64,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  73,   136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_edit_proc,       160,  96,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  105,  136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_edit_proc,       160,  128,  32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  137,  136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
// 14
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



void EditInfoType(int index)
{
  char ps1[4],ps2[4],ps3[4];
  char caption[40];

  sprintf(caption,"Info Data %d",index);
  editinfo_dlg[1].dp = caption;

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
  editinfo_dlg[13].dp = msgslist;

  int ret = popup_dialog(editinfo_dlg,-1);

  if(ret==14) {
    saved=false;
    misc.info[index].price[0] = atoi(ps1);
    misc.info[index].price[1] = atoi(ps2);
    misc.info[index].price[2] = atoi(ps3);
    misc.info[index].str[0] = editinfo_dlg[9].d1;
    misc.info[index].str[1] = editinfo_dlg[11].d1;
    misc.info[index].str[2] = editinfo_dlg[13].d1;
    }
}


int onInfoTypes()
{
  number_list_size = 16;
  number_list_zero = true;

  int index = select_data("Info Types",0,numberlist,"Edit","Done");

  while(index!=-1) {
    EditInfoType(index);
    index = select_data("Info Types",index,numberlist,"Edit","Done");
    }

  return D_O_K;
}



/********************************/
/********* onShopTypes **********/
/********************************/


void EditShopType(int index)
{
  char ps1[4],ps2[4],ps3[4];
  char caption[40];

  sprintf(caption,"Shop Data %d",index);
  editinfo_dlg[1].dp = caption;

  sprintf(ps1,"%d",misc.shop[index].price[0]);
  sprintf(ps2,"%d",misc.shop[index].price[1]);
  sprintf(ps3,"%d",misc.shop[index].price[2]);
  editinfo_dlg[8].dp  = ps1;
  editinfo_dlg[10].dp = ps2;
  editinfo_dlg[12].dp = ps3;
  editinfo_dlg[9].d1  = misc.shop[index].item[0];
  editinfo_dlg[11].d1 = misc.shop[index].item[1];
  editinfo_dlg[13].d1 = misc.shop[index].item[2];
  editinfo_dlg[9].dp  =
  editinfo_dlg[11].dp =
  editinfo_dlg[13].dp = itemslist;

  int ret = popup_dialog(editinfo_dlg,-1);

  if(ret==14) {
    saved=false;
    misc.shop[index].price[0] = atoi(ps1);
    misc.shop[index].price[1] = atoi(ps2);
    misc.shop[index].price[2] = atoi(ps3);
    misc.shop[index].item[0] = editinfo_dlg[9].d1;
    misc.shop[index].item[1] = editinfo_dlg[11].d1;
    misc.shop[index].item[2] = editinfo_dlg[13].d1;
    }
}


int onShopTypes()
{
  number_list_size = 16;
  number_list_zero = true;

  int index = select_data("Shop Types",0,numberlist,"Edit","Done");

  while(index!=-1) {
    EditShopType(index);
    index = select_data("Shop Types",index,numberlist,"Edit","Done");
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

 sprintf(caption,"Ring %d  Warp %d",ring,index+1);
 warp_dlg[1].dp = caption;
 warp_dlg[7].flags = D_DISABLED;

 sprintf(buf,"%02X",misc.warp[ring].scr[index]);
 warp_dlg[8].d1=misc.warp[ring].dmap[index];
 warp_dlg[9].dp=buf;
 warp_dlg[2].fg=warp_dlg[5].fg=vc(7);
 for(int i=0; i<4; i++)
   warp_dlg[10+i].d2 = 0;
 dmap_list_size=MAXDMAPS;
 dmap_list_zero=true;
 int ret=popup_dialog(warp_dlg,-1);
 if(ret==14 || ret==15) {
   saved=false;
   misc.warp[ring].dmap[index] = warp_dlg[8].d1;
   misc.warp[ring].scr[index] = xtoi(buf);
   }
 if(ret==15) {
   Map.dowarp(ring,index);
   refresh(rALL);
   }
}


int d_warplist_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_DRAW) {
   int *xy = (int*)(d->dp3);
   int ring = curr_ring;
   int dmap = misc.warp[ring].dmap[d->d1];

   drawdmap(dmap);
   if(xy[0]||xy[1])
     blit(dmapbmp,screen,0,0,xy[0],xy[1],65,33);
   text_mode(d->bg);
   if(xy[2]||xy[3])
     textprintf(screen,font,xy[2],xy[3],vc(11),"Map: %d",DMaps[dmap].map+1);
   if(xy[4]||xy[5])
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level:%2d",DMaps[dmap].level);
   if(xy[6]||xy[7])
     textprintf(screen,font,xy[6],xy[7],vc(11),"Scr: 0x%02X",misc.warp[ring].scr[d->d1]);
   }
 return d_list_proc(msg,d,c);
}


int d_wclist_proc(int msg,DIALOG *d,int c)
{
 int d1 = d->d1;
 int ret = d_droplist_proc(msg,d,c);
 misc.warp[curr_ring].size=d->d1+3;
 if(d->d1 != d1)
   return D_CLOSE;
 return ret;
}


char *wclist(int index, int *list_size)
{
 static char buf[2];
 if(index>=0) {
   if(index>5)
    index=5;
   sprintf(buf,"%d",index+3);
   return buf;
   }
 *list_size=6;
 return NULL;
}


int warpringdmapxy[8] = {160,106,160,80,160,92,160,144};


static DIALOG warpring_dlg[] =
{
 // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
 { d_shadow_box_proc, 64,   40,   192,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  48,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   64,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Count:" },
 { d_wclist_proc,     136,  62,   48,   11,   vc(14),  vc(1),  0,       0,          1,             0,       wclist },
// 4
 { d_warplist_proc,   80,   80,   64,   67,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       numberlist, NULL, warpringdmapxy },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


int select_warp()
{
 misc.warp[curr_ring].size = vbound(misc.warp[curr_ring].size,3,8);
 number_list_zero = false;

 int ret=4;
 do {
   number_list_size = misc.warp[curr_ring].size;
   warpring_dlg[3].d1 = misc.warp[curr_ring].size-3;
   ret = popup_dialog(warpring_dlg,ret);
 } while(ret==3);

 if(ret==6)
   return -1;
 return warpring_dlg[4].d1;
}



void EditWarpRing(int ring)
{
  char buf[40];
  sprintf(buf,"Ring %d Warps",ring);
  warpring_dlg[1].dp = buf;
  curr_ring = ring;

  int index = select_warp();

  while(index!=-1) {
    EditWarpRingScr(ring,index);
    index = select_warp();
    }
}


int onWarpRings()
{
  number_list_size = 8;
  number_list_zero = true;

  int index = select_data("Warp Rings",0,numberlist,"Edit","Done");

  while(index!=-1) {
    EditWarpRing(index);
    number_list_size = 8;
    number_list_zero = true;
    index = select_data("Warp Rings",index,numberlist,"Edit","Done");
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
 { d_shadow_box_proc, 72,   64,   176,  88,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  72,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Enemy Pattern" },
 { d_list_proc,       80,   88,   160,  28,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       pattern_list },
// 3
 { d_button_proc,     90,   128,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     170,  128,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};

int onPattern()
{
 byte p=Map.CurrScr()->pattern;
 pattern_dlg[2].d1 = p;
 if(popup_dialog(pattern_dlg,2) < 4)
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
 { d_shadow_box_proc, 72,   48,   176,  128,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Enemy Flags" },
// 2
 { d_check_proc,      82,   72,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Zora              " },
 { d_check_proc,      82,   80,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Corner traps      " },
 { d_check_proc,      82,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Center traps      " },
 { d_check_proc,      82,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Falling rocks     " },
 { d_check_proc,      82,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Statues shoot fire" },
 { d_check_proc,      82,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Has ring leader   " },
 { d_check_proc,      82,   120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enemy carries item" },
 { d_check_proc,      82,   128,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon Boss      " },
// 10
 { d_button_proc,     90,   144,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_button_proc,     170,  144,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


int onEnemyFlags()
{
 byte f=Map.CurrScr()->enemyflags;
 for(int i=0; i<=7; i++) {
   enemyflags_dlg[i+2].flags = f&1?D_SELECTED:0;
   f>>=1;
   }
 if(popup_dialog(enemyflags_dlg,-1)==10) {
   saved=false;
   f=0;
   for(int i=7; i>=0; i--) {
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
 if(index<0) {
   *list_size=10;
   return NULL;
   }
 int guy=Map.CurrScr()->enemy[index];
 return guy>=10 ? guy_string[guy] : (char *) "(none)";
}



typedef struct enemy_struct {
  char *s;
  int i;
} enemy_struct;

enemy_struct bie[eMAXGUYS];
enemy_struct ce[100];
int enemy_type=0,bie_cnt=-1,ce_cnt;

enemy_struct big[eMAXGUYS];
enemy_struct cg[100];
int big_cnt=-1,cg_cnt;


void build_bie_list()
{
  bie[0].s = "(none)";
  bie[0].i = 0;
  bie_cnt=1;

  for(int i=10; i<eMAXGUYS; i++)
  {
    if(guy_string[i][0]!='-')
    {
      bie[bie_cnt].s = guy_string[i];
      bie[bie_cnt].i = i;
      bie_cnt++;
    }
  }

  for(int i=0; i<bie_cnt-1; i++)
    for(int j=i+1; j<bie_cnt; j++)
      if(strcmp(bie[i].s,bie[j].s)>0)
        swap(bie[i],bie[j]);

/*
  FILE *f = fopen("enemies.txt","w");

  fprintf(f,"bie_cnt: %d\n",bie_cnt);
  for(int i=0; i<bie_cnt; i++)
    fprintf(f,"%-3d %s\n",bie[i].i,bie[i].s);

  fclose(f);
*/
}

char *enemylist(int index, int *list_size)
{
 if(index<0) {
   *list_size = enemy_type ? ce_cnt : bie_cnt;
   return NULL;
   }
 return enemy_type ? ce[index].s : bie[index].s;
}



static DIALOG elist_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60,   40,   200,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_abclist_proc,    72,   60,   176,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int select_enemy(char *prompt,int enemy)
{
 if(bie_cnt==-1)
   build_bie_list();

 int index=0;

 for(int j=0; j<bie_cnt; j++)
   if(bie[j].i == enemy)
     index=j;

 elist_dlg[1].dp=prompt;
 elist_dlg[2].d1=index;
 elist_dlg[2].dp=enemylist;

 if(popup_dialog(elist_dlg,2)==4)
   return -1;

 index = elist_dlg[2].d1;
 return bie[index].i;
}



unsigned char check[2] = { ';'+128,0 };

static DIALOG enemy_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 40,   32,   240,  176,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Enemies" },
 { d_list_proc,       64,   56,   192,  84,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       enemy_viewer },
 { d_button_proc,     52,   152,  128,  16,   vc(14),  vc(1),  'e',     D_EXIT,     0,             0,       (void *) "Paste &Enemies" },
 { d_text_proc,       188,  156,  64,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_button_proc,     52,   178,  64,   16,   vc(14),  vc(1),  'f',     D_EXIT,     0,             0,       (void *) "&Flags" },
 { d_button_proc,     132,  178,  80,   16,   vc(14),  vc(1),  'p',     D_EXIT,     0,             0,       (void *) "&Pattern" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'c',     0,          0,             0,       close_dlg },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'v',     0,          0,             0,       close_dlg },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       close_dlg },
// 10
 { d_button_proc,     228,  178,  40,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      27,      0,          0,             0,       close_dlg },
 { d_text_proc,       52,   240,  8,    8,    vc(14),  vc(1),  0,       0,          0,             0,       check },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};



int onEnemies()
{
 char buf[24] = "";
 int ret;
 int copy=-1;

 if(Map.CanPaste()) {
   enemy_dlg[3].flags=D_EXIT;
   sprintf(buf,"from %d:%02X",(Map.CopyScr()>>8)+1,Map.CopyScr()&255);
   }
 else
   enemy_dlg[3].flags=D_DISABLED;
 enemy_dlg[4].dp=buf;
 enemy_dlg[2].d1=0;

 do {
   if(copy==-1)
     enemy_dlg[12].y=240;
   else
     enemy_dlg[12].y=(copy<<3)+58;
   ret = do_dialog(enemy_dlg,2);
   switch(ret) {
   case 2: {
     int i = enemy_dlg[2].d1;
     int enemy = Map.CurrScr()->enemy[i];
     enemy = select_enemy("Select Enemy",enemy);
     if(enemy>=0) {
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
   }
 } while(ret<10);
 refresh(rALL);
 return D_O_K;
}



/*******************************/
/********** onHeader ***********/
/*******************************/

char author[65],title[65],password[32];


int d_showedit_proc(int msg,DIALOG *d,int c)
{
  int ret = d_edit_proc(msg,d,c);
  if(msg==MSG_CHAR) {
    scare_mouse();
    (d+1)->proc(MSG_DRAW,d+1,0);
    unscare_mouse();
    }
  return ret;
}


static DIALOG header_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   32,   192,  184,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Header" },
 { d_text_proc,       76,   56,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "ZQ Version:" },
 { d_text_proc,       168,  56,   96,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       76,   64,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Password:" },
 { d_edit_proc,       152,  64,   96,   8,    vc(12),  vc(1),  0,       0,          24,            0,       password },
 { d_text_proc,       76,   72,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Quest Number:" },
 { d_edit_proc,       184,  72,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_text_proc,       76,   80,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Quest Ver:" },
 { d_edit_proc,       160,  80,   80,   8,    vc(12),  vc(1),  0,       0,          8,             0,       NULL },
 { d_text_proc,       76,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Min. Ver:" },
 { d_edit_proc,       160,  88,   80,   8,    vc(12),  vc(1),  0,       0,          8,             0,       NULL },
// 12
 { d_text_proc,       76,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Title:" },
 { d_showedit_proc,   128,  104,  104,  8,    vc(12),  vc(1),  0,       0,          64,            0,       title },
 { d_textbox_proc,    88,   113,  144,  24,   vc(11),  vc(1),  0,       0,          64,            0,       title },
 { d_text_proc,       76,   144,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Author:" },
 { d_showedit_proc,   136,  144,  96,   8,    vc(12),  vc(1),  0,       0,          64,            0,       author },
 { d_textbox_proc,    88,   153,  144,  24,   vc(11),  vc(1),  0,       0,          64,            0,       author },
// 18
 { d_button_proc,     90,   188,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  188,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
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
 get_questpwd(password);

 header_dlg[3].dp = zver_str;
 header_dlg[7].dp = q_num;
 header_dlg[9].dp = version;
 header_dlg[11].dp = minver;

 int ret=popup_dialog(header_dlg,-1);

 if(ret==18)
 {
   saved=false;
   header.quest_number=atoi(q_num);
   strcpy(header.author,author);
   strcpy(header.title,title);
   strcpy(header.version,version);
   strcpy(header.minver,minver);
   set_questpwd(password);
 }
 return D_O_K;
}


static ZCHEATS tmpcheats;

static DIALOG cheats_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 32,   58,   256,  128,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  65,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Cheat Codes" },
// 2
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 5
 { d_check_proc,      104,  80,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enable Cheats " },
 { d_text_proc,       40,   100,  160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Lvl  Code" },
 { d_text_proc,       48,   110,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_text_proc,       48,   120,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_text_proc,       48,   130,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_text_proc,       48,   140,  8,    8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "4" },
// 11
 { d_edit_proc,       80,   110,  192,  8,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[0] },
 { d_edit_proc,       80,   120,  192,  8,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[1] },
 { d_edit_proc,       80,   130,  192,  8,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[2] },
 { d_edit_proc,       80,   140,  192,  8,    vc(12),  vc(1),  0,       0,          40,            0,       tmpcheats.codes[3] },
 { NULL }
};


int onCheats()
{
  tmpcheats = zcheats;
  cheats_dlg[5].flags = zcheats.flags ? D_SELECTED : 0;

  int ret = popup_dialog(cheats_dlg, 3);

  if(ret == 2)
  {
    saved = false;
    zcheats = tmpcheats;
    zcheats.flags = (cheats_dlg[5].flags & D_SELECTED) ? 1 : 0;
  }

  return D_O_K;
}




static DIALOG rules_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   20,   208,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Rules" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 1" },
// 3
 { d_button_proc,     170,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     62,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     240,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Solid Blocks         " },
 { d_check_proc,      70,   60,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "TmpNoRet Disabled    " },
 { d_check_proc,      70,   70,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enemies Always Return" },
 { d_check_proc,      70,   80,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Multi-Dir Traps      " },

 { d_check_proc,      70,   90,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "BS-Zelda Animation   " },
 { d_check_proc,      70,  100,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Interpolated Fading  " },
 { d_check_proc,      70,  110,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Fade CSet 5          " },
 { d_check_proc,      70,  120,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Fast Dungeon Scroll  " },

 { d_check_proc,      70,  130,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Level-3 Pal Fix   " },
 { d_check_proc,      70,  140,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Cool Scrolling       " },
 { d_check_proc,      70,  150,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "999 Rupies From Start" },
 { d_check_proc,      70,  160,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Big Triforce Pieces  " },

 { d_check_proc,      70,  170,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "24 Heart Containers  " },
 { d_check_proc,      70,  180,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Fast Heart Refill    " },
 { d_check_proc,      70,  190,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "View Overworld Map   " },
 { d_check_proc,      70,  200,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3 or 6 Triforce Total" },
 { NULL }
};


int RulesPage_1()
{
  for(int i=0; i<qr1_MAX; i++)
    rules_dlg[i+8].flags = get_bit(header.rules,i) ? D_SELECTED : 0;

  int ret = popup_dialog(rules_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<qr1_MAX; i++)
      set_bit(header.rules,i,rules_dlg[i+8].flags);
  }

  if(ret==5) {
    rules_page--;
    if (rules_page==0) {
      rules_page=rules_pages;
    }
  }

  if(ret==6) {
    rules_page++;
    if (rules_page>rules_pages) {
      rules_page=1;
    }
  }

  if(ret<5) {
    rules_done=true;
  }
  return D_O_K;
}


static DIALOG rules2_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   20,   208,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Rules" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 2" },
// 3
 { d_button_proc,     170,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     62,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     240,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Show Time on Subscrn " },
 { d_check_proc,      70,   60,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "New Subscreen        " },
 { d_check_proc,      70,   70,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Must Kill All Bosses " },
 { d_check_proc,      70,   80,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Sprite Flicker    " },
 { d_check_proc,      70,   90,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Continue w/ Full Life" },
 { d_check_proc,      70,  100,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Raft/Ladder fix      " },
 { d_check_proc,      70,  110,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Wooden Sword beams" },
 { d_check_proc,      70,  120,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No White Sword beams " },
 { d_check_proc,      70,  130,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Magic Sword beams " },
 { d_check_proc,      70,  140,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Master Sword beams" },
 { d_check_proc,      70,  150,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Sword Beams 1/2 Power" },
 { d_check_proc,      70,  160,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Silver Arrow Sparkles" },
 { d_check_proc,      70,  170,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Golden Arrow Sparkles" },
 { d_check_proc,      70,  180,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Magic Brang Sparkles " },
 { d_check_proc,      70,  190,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Fire Brang Sparkles  " },
 { d_check_proc,      70,  200,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Flashing Death    " },
 { NULL }
};


int RulesPage_2()
{
  for(int i=0; i<qr2_MAX; i++)
    rules2_dlg[i+8].flags = get_bit(header.rules2,i) ? D_SELECTED : 0;

  int ret = popup_dialog(rules2_dlg,4);

  if(ret>3) {
    saved=false;
    for(int i=0; i<qr2_MAX; i++)
      set_bit(header.rules2,i,rules2_dlg[i+8].flags);
  }

  if(ret==5) {
    rules_page--;
    if (rules_page==0) {
      rules_page=rules_pages;
    }
  }

  if(ret==6) {
    rules_page++;
    if (rules_page>rules_pages) {
      rules_page=1;
    }
  }

  if(ret<5) {
    rules_done=true;
  }
  return D_O_K;
}

static DIALOG rules3_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   20,   208,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Rules" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 3" },
// 3
 { d_button_proc,     170,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     62,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     240,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
// { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Door Repair Fix      " },
 { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "<Reserved>           " },
 { d_check_proc,      70,   60,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Own Fire Doesn't Hurt" },
 { d_check_proc,      70,   70,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bombs Hurt           " },
 { d_check_proc,      70,   80,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Random Clocks     " },
 { d_check_proc,      70,   90,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Temporary Clocks     " },
 { d_check_proc,      70,  100,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Broken Drknut Shields" },
// { d_check_proc,      70,  110,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Under Combos      " },
 { d_check_proc,      70,  110,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Reserved             " },
 { d_check_proc,      70,  120,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Mean Placed Traps    " },
 { d_check_proc,      70,  130,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Phantom Placed Traps " },
 { d_check_proc,      70,  140,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Allow Fast Messages  " },
 { d_check_proc,      70,  150,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Linked Combos        " },
 { d_check_proc,      70,  160,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Guy Fires         " },
 { d_check_proc,      70,  170,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Heart Ring Fix       " },
 { d_check_proc,      70,  180,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "No Heart Ring        " },
 { d_check_proc,      70,  190,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Dodongo Color Fix    " },
 { d_check_proc,      70,  200,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Sword/Wand Flip Fix  " },
 { NULL }
};


int RulesPage_3()
{
  for(int i=0; i<qr3_MAX; i++)
    rules3_dlg[i+8].flags = get_bit(header.rules3,i) ? D_SELECTED : 0;

  int ret = popup_dialog(rules3_dlg,4);

  if(ret>3) {
    saved=false;
    for(int i=0; i<qr3_MAX; i++)
      set_bit(header.rules3,i,rules3_dlg[i+8].flags);
  }

  if(ret==5) {
    rules_page--;
    if (rules_page==0) {
      rules_page=rules_pages;
    }
  }

  if(ret==6) {
    rules_page++;
    if (rules_page>rules_pages) {
      rules_page=1;
    }
  }

  if(ret<5) {
    rules_done=true;
  }
  return D_O_K;
}


static DIALOG rules4_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   20,   208,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Quest Rules" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 4" },
// 3
 { d_button_proc,     170,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     90,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     62,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     240,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      70,   50,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Enable Magic         " },
 { d_check_proc,      70,   60,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wand uses Magic      " },
 { d_check_proc,      70,   70,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Candle uses Magic    " },
 { d_check_proc,      70,   80,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boots use Magic      " },
 { d_check_proc,      70,   90,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Non-bubble Medicine  " },
 { d_check_proc,      70,  100,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Non-bubble Fairies   " },
 { d_check_proc,      70,  110,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Non-bubble Triforce  " },
 { d_check_proc,      70,  120,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "New Enemy Tiles      " },
 { d_check_proc,      70,  130,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Rope 2's Don't Flash " },
 { d_check_proc,      70,  140,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bubbles Don't Flash  " },
 { d_check_proc,      70,  150,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Ghini 2's Blink      " },
 { d_check_proc,      70,  160,   184,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Weapon Animation Fix " },
 { NULL }
};

int RulesPage_4()
{
  for(int i=0; i<qr4_13; i++)
    rules4_dlg[i+8].flags = get_bit(header.rules4,i) ? D_SELECTED : 0;

  int ret = popup_dialog(rules4_dlg,4);

  if(ret>3) {
    saved=false;
    for(int i=0; i<qr4_13; i++)
      set_bit(header.rules4,i,rules4_dlg[i+8].flags);
  }

  if(ret==5) {
    rules_page--;
    if (rules_page==0) {
      rules_page=rules_pages;
    }
  }

  if(ret==6) {
    rules_page++;
    if (rules_page>rules_pages) {
      rules_page=1;
    }
  }

  if(ret<5) {
    rules_done=true;
  }
  return D_O_K;
}

int RulesPage_5()
{
  return D_O_K;
}

int RulesPage_6()
{
  return D_O_K;
}

int RulesPage_7()
{
  return D_O_K;
}

int RulesPage_8()
{
  return D_O_K;
}

int RulesPage_9()
{
  return D_O_K;
}

int RulesPage_10()
{
  return D_O_K;
}


int onRules()
{
//  while (!rules_done) {
  do {
    rules_done=false;
    switch (rules_page) {
      case 1:
        RulesPage_1();
        break;
      case 2:
        RulesPage_2();
        break;
      case 3:
        RulesPage_3();
        break;
      case 4:
        RulesPage_4();
        break;
      case 5:
        RulesPage_5();
        break;
      case 6:
        RulesPage_6();
        break;
      case 7:
        RulesPage_7();
        break;
      case 8:
        RulesPage_8();
        break;
      case 9:
        RulesPage_9();
        break;
      case 10:
        RulesPage_10();
        break;
      default:
        break;
    }
  } while (!rules_done);
//  rules_done=false;
  return D_O_K;
}

int d_line_proc(int msg, DIALOG *d, int c)
{
   if (msg==MSG_DRAW) {
      int fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
      line(screen, d->x, d->y, d->x+d->w, d->y+d->h, fg);
   }
   return D_O_K;
}


static DIALOG init1_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 1 - Equipment" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      34,   50,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Raft     " },
 { d_check_proc,      34,   60,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Ladder   " },
 { d_check_proc,      34,   70,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Book     " },
// 11
 { d_check_proc,      132,  50,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Magic Key" },
 { d_check_proc,      132,  60,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Flippers " },
 { d_check_proc,      132,  70,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boots    " },
// 14
 { d_text_proc,       230,  50,    48,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Ring" },
 { d_radio_proc,      230,  60,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "None" },
 { d_radio_proc,      230,  70,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Blue" },
 { d_radio_proc,      230,  80,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Red" },
 { d_radio_proc,      230,  90,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Gold" },
// 19
 { d_text_proc,       34,   85,    64,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Sword" },
 { d_radio_proc,      34,   95,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "None" },
 { d_radio_proc,      34,  105,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Wooden" },
 { d_radio_proc,      34,  115,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "White" },
 { d_radio_proc,      34,  125,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Magic" },
 { d_radio_proc,      34,  135,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Master" },
// 25
 { d_text_proc,       230, 105,    56,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Wallet" },
 { d_radio_proc,      230, 115,    56,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "None" },
 { d_radio_proc,      230, 125,    56,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Small" },
 { d_radio_proc,      230, 135,    56,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Large" },
// 29
 { d_text_proc,       134,  85,    64,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Shield" },
// { d_radio_proc,      134,  95,    64,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "None" },
 { d_text_proc,       134,  95,    64,  8,    vc(15),  vc(1),  0,       0,          3,             0,       (void *) " " },
 { d_radio_proc,      134, 105,    64,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Small" },
 { d_radio_proc,      134, 115,    64,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Magic" },
 { d_radio_proc,      134, 125,    64,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Mirror" },
// 34
 { d_text_proc,       34,  150,    72,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Bracelet" },
 { d_radio_proc,      34,  160,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "None" },
 { d_radio_proc,      34,  170,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "Level 1" },
 { d_radio_proc,      34,  180,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "Level 2" },
// 38
 { d_text_proc,       134, 150,    72,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Amulet" },
 { d_radio_proc,      134, 160,    72,  8,    vc(14),  vc(1),  0,       0,          5,             0,       (void *) "None" },
 { d_radio_proc,      134, 170,    72,  8,    vc(14),  vc(1),  0,       0,          5,             0,       (void *) "Level 1" },
// { d_radio_proc,      134, 180,    72,  8,    vc(14),  vc(1),  0,       0,          5,             0,       (void *) "Level 2" },
 { d_text_proc,      134, 180,    72,  8,    vc(15),  vc(1),  0,       0,          5,             0,       (void *) " " },
// 42
 { d_text_proc,       230, 150,    56,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Bow" },
 { d_radio_proc,      230, 160,    56,  8,    vc(14),  vc(1),  0,       0,          6,             0,       (void *) "None" },
 { d_radio_proc,      230, 170,    56,  8,    vc(14),  vc(1),  0,       0,          6,             0,       (void *) "Small" },
// { d_radio_proc,      230, 180,    56,  8,    vc(14),  vc(1),  0,       0,          6,             0,       (void *) "Large" },
 { d_text_proc,      230, 180,    56,  8,    vc(15),  vc(1),  0,       0,          6,             0,       (void *) " " },
 { NULL }
};

int InitPage_1()
{

  for(int i=0; i<idE_MAX; i++)
    init1_dlg[i+8].flags = get_bit(&(zinit.equipment),i) ? D_SELECTED : 0;

  //ring
  for (int i=0; i<4; i++) {
    init1_dlg[i+15].flags=0;
  }
  init1_dlg[zinit.ring+15].flags=D_SELECTED;

  //sword
  for (int i=0; i<5; i++) {
    init1_dlg[i+20].flags=0;
  }
  init1_dlg[zinit.sword+20].flags=D_SELECTED;

  //wallet
  for (int i=0; i<3; i++) {
    init1_dlg[i+26].flags=0;
  }
  init1_dlg[zinit.wallet+26].flags=D_SELECTED;

  //shield
  for (int i=0; i<4; i++) {
    init1_dlg[i+30].flags=0;
  }
  init1_dlg[zinit.shield+30].flags=D_SELECTED;

  //bracelet
  for (int i=0; i<3; i++) {
    init1_dlg[i+35].flags=0;
  }
  init1_dlg[zinit.bracelet+35].flags=D_SELECTED;

  //amulet
  for (int i=0; i<3; i++) {
    init1_dlg[i+39].flags=0;
  }
  init1_dlg[zinit.amulet+39].flags=D_SELECTED;

  //bow
  for (int i=0; i<3; i++) {
    init1_dlg[i+43].flags=0;
  }
  init1_dlg[zinit.bow+43].flags=D_SELECTED;

  int ret = popup_dialog(init1_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<idE_MAX; i++)
      set_bit(&(zinit.equipment),i,init1_dlg[i+8].flags);

    //ring
    for (int i=0; i<4; i++) {
      if (init1_dlg[i+15].flags&D_SELECTED) {
        zinit.ring=i;
      }
    }

    //sword
    for (int i=0; i<5; i++) {
      if (init1_dlg[i+20].flags&D_SELECTED) {
        zinit.sword=i;
      }
    }

    //wallet
    for (int i=0; i<3; i++) {
      if (init1_dlg[i+26].flags&D_SELECTED) {
        zinit.wallet=i;
      }
    }

    //shield
    for (int i=0; i<4; i++) {
      if (init1_dlg[i+30].flags&D_SELECTED) {
        zinit.shield=i;
      }
    }

    //bracelet
    for (int i=0; i<3; i++) {
      if (init1_dlg[i+35].flags&D_SELECTED) {
        zinit.bracelet=i;
      }
    }

    //amulet
    for (int i=0; i<3; i++) {
      if (init1_dlg[i+39].flags&D_SELECTED) {
        zinit.amulet=i;
      }
    }

    //bow
    for (int i=0; i<3; i++) {
      if (init1_dlg[i+43].flags&D_SELECTED) {
        zinit.bow=i;
      }
    }

  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}



static DIALOG init2_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 2 - Items" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_check_proc,      34,   50,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wand     " },
 { d_check_proc,      34,   60,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Letter   " },
 { d_check_proc,      34,   70,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Lens     " },
// 11
 { d_check_proc,      132,  50,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Hookshot " },
 { d_check_proc,      132,  60,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bait     " },
 { d_check_proc,      132,  70,    88,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Hammer   " },
// 14
 { d_text_proc,       230,  50,    48,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Candle" },
 { d_radio_proc,      230,  60,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "None" },
 { d_radio_proc,      230,  70,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Blue" },
 { d_radio_proc,      230,  80,    48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Red" },
// 18
 { d_text_proc,       34,   85,    64,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Boomerang" },
 { d_radio_proc,      34,   95,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "None" },
 { d_radio_proc,      34,  105,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Wooden" },
 { d_radio_proc,      34,  115,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Magic" },
 { d_radio_proc,      34,  125,    64,  8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Fire" },
// 23
 { d_text_proc,       134,  85,    64,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Arrow" },
 { d_radio_proc,      134,  95,    64,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "None" },
 { d_radio_proc,      134, 105,    64,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Wooden" },
 { d_radio_proc,      134, 115,    64,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Silver" },
 { d_radio_proc,      134, 125,    64,  8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Golden" },
// 28
 { d_text_proc,       230,  95,    56,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Potion" },
 { d_radio_proc,      230, 105,    56,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "None" },
 { d_radio_proc,      230, 115,    56,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Blue" },
 { d_radio_proc,      230, 125,    56,  8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Red" },
// 32
 { d_text_proc,       34,  140,    72,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Whistle" },
 { d_radio_proc,      34,  150,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "None" },
 { d_radio_proc,      34,  160,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "Recorder" },
// { d_radio_proc,      34,  170,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) "Ocarina" },
 { d_text_proc,       34,  170,    72,  8,    vc(14),  vc(1),  0,       0,          4,             0,       (void *) " " },
// 36
 { d_text_proc,       134, 140,    48,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Bombs:" },
 { d_edit_proc,       190, 140,    32,  8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
// 38
 { d_text_proc,       134, 150,    56,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "SBombs:" },
 { d_edit_proc,       198, 150,    32,  8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { NULL }
};

int InitPage_2()
{

  for(int i=0; i<idI_MAX; i++)
    init2_dlg[i+8].flags = get_bit(&(zinit.items),i) ? D_SELECTED : 0;

  //candle
  for (int i=0; i<3; i++) {
    init2_dlg[i+15].flags=0;
  }
  init2_dlg[zinit.candle+15].flags=D_SELECTED;

  //boomerang
  for (int i=0; i<4; i++) {
    init2_dlg[i+19].flags=0;
  }
  init2_dlg[zinit.boomerang+19].flags=D_SELECTED;

  //arrow
  for (int i=0; i<4; i++) {
    init2_dlg[i+24].flags=0;
  }
  init2_dlg[zinit.arrow+24].flags=D_SELECTED;

  //potion
  for (int i=0; i<3; i++) {
    init2_dlg[i+29].flags=0;
  }
  init2_dlg[zinit.potion+29].flags=D_SELECTED;

  //whistle
  for (int i=0; i<3; i++) {
    init2_dlg[i+33].flags=0;
  }
  init2_dlg[zinit.whistle+33].flags=D_SELECTED;

  // bombs/super bombs
  char bombstring[5];
  char sbombstring[5];
  sprintf(bombstring, "%d", zinit.bombs);
  sprintf(sbombstring, "%d", zinit.super_bombs);
  init2_dlg[37].dp=bombstring;
  init2_dlg[39].dp=sbombstring;

  int ret = popup_dialog(init2_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<idI_MAX; i++)
      set_bit(&(zinit.items),i,init2_dlg[i+8].flags);

    //candle
    for (int i=0; i<3; i++) {
      if (init2_dlg[i+15].flags&D_SELECTED) {
        zinit.candle=i;
      }
    }

    //boomerang
    for (int i=0; i<4; i++) {
      if (init2_dlg[i+19].flags&D_SELECTED) {
        zinit.boomerang=i;
      }
    }

    //arrow
    for (int i=0; i<4; i++) {
      if (init2_dlg[i+24].flags&D_SELECTED) {
        zinit.arrow=i;
      }
    }

    //potion
    for (int i=0; i<3; i++) {
      if (init2_dlg[i+29].flags&D_SELECTED) {
        zinit.potion=i;
      }
    }

    //whistle
    for (int i=0; i<3; i++) {
      if (init2_dlg[i+33].flags&D_SELECTED) {
        zinit.whistle=i;
      }
    }

    // bombs/super bombs
    zinit.bombs=atoi(bombstring);
    zinit.super_bombs=atoi(sbombstring);

  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

static DIALOG init3_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 3 - DMap Map/Compass" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_text_proc,       44,   71,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "0" },
 { d_text_proc,       44,   81,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_text_proc,       44,   91,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_text_proc,       44,  101,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_text_proc,       44,  111,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "4" },
 { d_text_proc,       44,  121,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "5" },
 { d_text_proc,       44,  131,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "6" },
 { d_text_proc,       44,  141,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "7" },
 { d_text_proc,       44,  151,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "8" },
 { d_text_proc,       44,  161,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "9" },
// 18
 { d_check_proc,      54,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
// 158
 { d_ctext_proc,      71,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "0" },
 { d_ctext_proc,     101,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "10" },
 { d_ctext_proc,     131,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "20" },
 { d_ctext_proc,     161,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "30" },
 { d_ctext_proc,     191,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "40" },
 { d_ctext_proc,     221,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "50" },
 { d_ctext_proc,     251,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "60" },
// 165
 { d_text_proc,       59,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,       89,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      119,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      149,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      179,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      209,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      239,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
// 172
 { d_line_proc,       85,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      115,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      145,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      175,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      205,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      235,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { NULL }
};

int InitPage_3()
{

  for(int i=0; i<70; i++) {
    init3_dlg[(i*2)+18].flags = get_bit(zinit.map,i) ? D_SELECTED : 0;
    init3_dlg[(i*2)+19].flags = get_bit(zinit.compass,i) ? D_SELECTED : 0;
  }


  int ret = popup_dialog(init3_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<70; i++) {
      set_bit(zinit.map,i,init3_dlg[(i*2)+18].flags);
      set_bit(zinit.compass,i,init3_dlg[(i*2)+19].flags);
    }
  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

static DIALOG init4_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 4 - DMap Map/Compass" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_text_proc,       44,   71,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "0" },
 { d_text_proc,       44,   81,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_text_proc,       44,   91,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_text_proc,       44,  101,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_text_proc,       44,  111,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "4" },
 { d_text_proc,       44,  121,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "5" },
 { d_text_proc,       44,  131,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "6" },
 { d_text_proc,       44,  141,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "7" },
 { d_text_proc,       44,  151,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "8" },
 { d_text_proc,       44,  161,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "9" },
// 18
 { d_check_proc,      54,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
// 158
 { d_ctext_proc,      71,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "70" },
 { d_ctext_proc,     101,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "80" },
 { d_ctext_proc,     131,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "90" },
 { d_ctext_proc,     161,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "100" },
 { d_ctext_proc,     191,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "110" },
 { d_ctext_proc,     221,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "120" },
 { d_ctext_proc,     251,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "130" },
// 165
 { d_text_proc,       59,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,       89,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      119,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      149,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      179,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      209,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      239,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
// 172
 { d_line_proc,       85,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      115,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      145,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      175,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      205,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      235,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { NULL }
};

int InitPage_4()
{

  for(int i=0; i<70; i++) {
    init4_dlg[(i*2)+18].flags = get_bit(zinit.map,i+70) ? D_SELECTED : 0;
    init4_dlg[(i*2)+19].flags = get_bit(zinit.compass,i+70) ? D_SELECTED : 0;
  }


  int ret = popup_dialog(init4_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<70; i++) {
      set_bit(zinit.map,i+70,init4_dlg[(i*2)+18].flags);
      set_bit(zinit.compass,i+70,init4_dlg[(i*2)+19].flags);
    }
  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

static DIALOG init5_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 5 - DMap Map/Compass" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_text_proc,       44,   71,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "0" },
 { d_text_proc,       44,   81,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_text_proc,       44,   91,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_text_proc,       44,  101,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_text_proc,       44,  111,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "4" },
 { d_text_proc,       44,  121,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "5" },
 { d_text_proc,       44,  131,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "6" },
 { d_text_proc,       44,  141,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "7" },
 { d_text_proc,       44,  151,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "8" },
 { d_text_proc,       44,  161,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "9" },
// 18
 { d_check_proc,      54,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     204,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     220,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     234,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     250,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
// 158
 { d_ctext_proc,      71,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "140" },
 { d_ctext_proc,     101,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "150" },
 { d_ctext_proc,     131,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "160" },
 { d_ctext_proc,     161,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "170" },
 { d_ctext_proc,     191,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "180" },
 { d_ctext_proc,     221,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "190" },
 { d_ctext_proc,     251,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "200" },
// 165
 { d_text_proc,       59,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,       89,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      119,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      149,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      179,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      209,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      239,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
// 172
 { d_line_proc,       85,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      115,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      145,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      175,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      205,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      235,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { NULL }
};

int InitPage_5()
{

  for(int i=0; i<70; i++) {
    init5_dlg[(i*2)+18].flags = get_bit(zinit.map,i+140) ? D_SELECTED : 0;
    init5_dlg[(i*2)+19].flags = get_bit(zinit.compass,i+140) ? D_SELECTED : 0;
  }


  int ret = popup_dialog(init5_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<70; i++) {
      set_bit(zinit.map,i+140,init5_dlg[(i*2)+18].flags);
      set_bit(zinit.compass,i+140,init5_dlg[(i*2)+19].flags);
    }
  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

static DIALOG init6_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 6 - DMap Map/Compass" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_text_proc,       44,   71,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "0" },
 { d_text_proc,       44,   81,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_text_proc,       44,   91,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_text_proc,       44,  101,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_text_proc,       44,  111,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "4" },
 { d_text_proc,       44,  121,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "5" },
 { d_text_proc,       44,  131,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "6" },
 { d_text_proc,       44,  141,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "7" },
 { d_text_proc,       44,  151,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "8" },
 { d_text_proc,       44,  161,   8,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "9" },
// 18
 { d_check_proc,      54,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      54,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      70,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,      84,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     100,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     114,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     130,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  130,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  140,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  150,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     144,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     160,  160,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   70,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   80,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,   90,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  100,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  110,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     174,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { d_check_proc,     190,  120,   12,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
// 110
 { d_ctext_proc,      71,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "210" },
 { d_ctext_proc,     101,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "220" },
 { d_ctext_proc,     131,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "230" },
 { d_ctext_proc,     161,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "240" },
 { d_ctext_proc,     191,   50,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "250" },
// 25
 { d_text_proc,       59,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,       89,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      119,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      149,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
 { d_text_proc,      179,   60,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "M/C" },
// 32
 { d_line_proc,       85,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      115,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      145,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_line_proc,      175,   50,   0,  118,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
// 38
 { NULL }
};

int InitPage_6()
{

  for(int i=0; i<46; i++) {
    init6_dlg[(i*2)+18].flags = get_bit(zinit.map,i+210) ? D_SELECTED : 0;
    init6_dlg[(i*2)+19].flags = get_bit(zinit.compass,i+210) ? D_SELECTED : 0;
  }


  int ret = popup_dialog(init6_dlg,4);

  if(ret>3)
  {
    saved=false;
    for(int i=0; i<46; i++) {
      set_bit(zinit.map,i+210,init6_dlg[(i*2)+18].flags);
      set_bit(zinit.compass,i+210,init6_dlg[(i*2)+19].flags);
    }
  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

static DIALOG init7_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 26,   20,   268,  216,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Initialization Data" },
 { d_ctext_proc,      160,  35,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Page 7 - Misc" },
// 3
 { d_button_proc,     200,  210,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     32,   27,   16,   16,   vc(14),  vc(1),  44,      D_EXIT,     0,             0,       (void *) "<" },
 { d_button_proc,     270,  27,   16,   16,   vc(14),  vc(1),  46,      D_EXIT,     0,             0,       (void *) ">" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 8
 { d_text_proc,       34,   50,  144,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Heart Containers:" },
 { d_text_proc,       34,   60,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Starting HP (hearts):" },
 { d_text_proc,       34,   70,  104,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Continue HP:" },
 { d_text_proc,       34,   80,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Max Bombs:" },
 { d_text_proc,       34,   90,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Keys:" },
 { d_text_proc,       34,  100,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Rupies:" },
// 14
 { d_edit_proc,      178,   50,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      210,   60,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      138,   70,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_edit_proc,      122,   80,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,       82,   90,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,       98,  100,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_check_proc,     172,   70,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "%" },
// 21
 { d_text_proc,       34,  115+30,   40,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "HCP's" },
 { d_radio_proc,      34,  125+30,   24,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "0" },
 { d_radio_proc,      34,  135+30,   24,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "1" },
 { d_radio_proc,      34,  145+30,   24,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2" },
 { d_radio_proc,      34,  155+30,   24,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "3" },
// 26
 { d_ctext_proc,     146-24,  115+30,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Triforce" },
 { d_check_proc,     116-24,  125+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "1" },
 { d_check_proc,     116-24,  135+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "2" },
 { d_check_proc,     116-24,  145+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "3" },
 { d_check_proc,     116-24,  155+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "4" },
 { d_check_proc,     146-24,  125+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "5" },
 { d_check_proc,     146-24,  135+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "6" },
 { d_check_proc,     146-24,  145+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "7" },
 { d_check_proc,     146-24,  155+30,   24,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "8" },
// 35
 { d_text_proc,      116+24+36,  115+30,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Sword Hearts" },
 { d_text_proc,      116+24+36,  125+30,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wooden: " },
 { d_text_proc,      116+24+36,  135+30,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "White: " },
 { d_text_proc,      116+24+36,  145+30,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Magic: " },
 { d_text_proc,      116+24+36,  155+30,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Master: " },
 { d_edit_proc,      116+24+64+36,  125+30,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      116+24+56+36,  135+30,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      116+24+56+36,  145+30,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      116+24+64+36,  155+30,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 44
 { d_text_proc,       34,   110,   88,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Max Magic: " },
 { d_edit_proc,      122,   110,   16,   8,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },
 { d_text_proc,       34,   120,   56,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Magic: " },
 { d_edit_proc,       90,   120,   16,   8,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },
 { d_text_proc,       34,   130,  104,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Double Magic " },
 { d_check_proc,     138,   130,   16,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "" },
 { NULL }
};


int InitPage_7()
{

  // bombs/super bombs
  char hcstring[5];
  char sheartstring[5];
  char cheartstring[5];
  char mbombstring[5];
  char keystring[5];
  char rupiestring[5];
  char swordheartstring[4][5];
  char magicstring[5];
  char maxmagicstring[5];
  sprintf(hcstring, "%d", zinit.hc);
  sprintf(sheartstring, "%d", zinit.start_heart);
  sprintf(cheartstring, "%d", zinit.cont_heart);
  sprintf(mbombstring, "%d", zinit.max_bombs);
  sprintf(keystring, "%d", zinit.keys);
  sprintf(rupiestring, "%d", zinit.rupies);
  sprintf(magicstring, "%d", zinit.magic);
  sprintf(maxmagicstring, "%d", zinit.maxmagic);
  for (int x=0; x<4; x++) {
    sprintf(swordheartstring[x], "%d", zinit.swordhearts[x]);
  }
  init7_dlg[14].dp=hcstring;
  init7_dlg[15].dp=sheartstring;
  init7_dlg[16].dp=cheartstring;
  init7_dlg[17].dp=mbombstring;
  init7_dlg[18].dp=keystring;
  init7_dlg[19].dp=rupiestring;
  init7_dlg[45].dp=maxmagicstring;
  init7_dlg[47].dp=magicstring;
  init7_dlg[20].flags = get_bit(zinit.misc,idM_CONTPERCENT) ? D_SELECTED : 0;
  init7_dlg[49].flags = get_bit(zinit.misc,idM_DOUBLEMAGIC) ? D_SELECTED : 0;

  // heart container pieces
  for (int i=0; i<4; i++) {
    init7_dlg[i+22].flags=0;
  }
  init7_dlg[zinit.hcp+22].flags=D_SELECTED;

  // triforce
  for(int i=0; i<8; i++) {
    init7_dlg[27].flags = get_bit(&zinit.triforce,i) ? D_SELECTED : 0;
  }

  for (int x=0; x<4; x++) {
    init7_dlg[40+x].dp=swordheartstring[x];
  }
  int ret = popup_dialog(init7_dlg,4);

  if(ret>3)
  {
    saved=false;
    zinit.hc=atoi(hcstring);
    zinit.start_heart=atoi(sheartstring);
    zinit.cont_heart=atoi(cheartstring);
    zinit.max_bombs=atoi(mbombstring);
    zinit.keys=atoi(keystring);
    zinit.rupies=atoi(rupiestring);
    zinit.maxmagic=atoi(maxmagicstring);
    zinit.magic=atoi(magicstring);
    set_bit(zinit.misc,idM_CONTPERCENT,init7_dlg[20].flags);
    set_bit(zinit.misc,idM_DOUBLEMAGIC,init7_dlg[49].flags);

    // heart container pieces
    for (int i=0; i<4; i++) {
      if (init7_dlg[i+22].flags&D_SELECTED) {
        zinit.hcp=i;
      }
    }

    // triforce
    for(int i=0; i<8; i++) {
      set_bit(&zinit.triforce,i,init7_dlg[27].flags);
    }

    for (int x=0; x<4; x++) {
      zinit.swordhearts[x]=atoi(swordheartstring[x]);
    }
  }

  if(ret==5) {
    init_page--;
    if (init_page==0) {
      init_page=init_pages;
    }
  }

  if(ret==6) {
    init_page++;
    if (init_page>init_pages) {
      init_page=1;
    }
  }

  if(ret<5) {
    init_done=true;
  }

  return D_O_K;
}

int onInit()
{
  do {
    init_done=false;
    switch (init_page) {
      case 1:
        InitPage_1();
        break;
      case 2:
        InitPage_2();
        break;
      case 3:
        InitPage_3();
        break;
      case 4:
        InitPage_4();
        break;
      case 5:
        InitPage_5();
        break;
      case 6:
        InitPage_6();
        break;
      case 7:
        InitPage_7();
        break;
/*
      case 8:
        InitPage_8();
        break;
      case 9:
        InitPage_9();
        break;
      case 10:
        InitPage_10();
        break;
*/
      default:
        break;
    }
  } while (!init_done);
  return D_O_K;
}



// custom items and guys
#include "zq_custom.cc"



/************************/
/******  onColors  ******/
/************************/

#include "gfxpal.cc"

#define edc  246
#define edi  247
#define ed0  248
#define ed1  249
#define ed7  250
#define ed11 251
#define ed14 252
#define ed15 253


void get_cset(int dataset,int row,RGB *pal)
{
  saved=false;
  for(int i=0; i<16; i++)
  {
    colordata[(CSET(dataset)+i)*3]   = pal[(row<<4)+i].r;
    colordata[(CSET(dataset)+i)*3+1] = pal[(row<<4)+i].g;
    colordata[(CSET(dataset)+i)*3+2] = pal[(row<<4)+i].b;
  }
}


void draw_edit_dataset_specs(int index,int copy)
{
  for(int i=0; i<16; i++)
    rectfill(screen,(i<<3)+96,163,(i<<3)+103,170,14*16+i);
  text_mode(ed1);
  rectfill(screen,96,173,223,200,ed1);
  textout(screen,font,"\x82",(index<<3)+96,173,ed11);
  if(copy>=0)
    textout(screen,font,"\x81",(copy<<3)+96,173,ed11);
  textprintf(screen,font,88,184,ed15,"Old: %2d - %2d %2d %2d",index,
    RAMpal[12*16+index].r,RAMpal[12*16+index].g,RAMpal[12*16+index].b);
  textprintf(screen,font,88,194,ed15,"New: %2d - %2d %2d %2d",index,
    RAMpal[14*16+index].r,RAMpal[14*16+index].g,RAMpal[14*16+index].b);
}


void init_colormixer()
{
  for(int i=0; i<128; i++) {
    RAMpal[i] = _RGB(gfx_pal+i*3);
    rectfill(screen,i+96,16,i+96,31,i);
    }

  for(int i=0; i<32; i++) {
    RAMpal[i+128] = _RGB(i<<1,i<<1,i<<1);
    rectfill(screen,80,(i<<1)+32,95,(i<<1)+33,i+128);
    }

  rect(screen,95,31,224,96,ed15);
  rect(screen,224,31,240,96,ed15);
  set_palette_range(RAMpal,0,255,false);
}


void colormixer(int color,int gray,int ratio)
{
  vsync();
  scare_mouse();

  for(int i=0; i<32; i++) {
    RAMpal[i+160] = mixRGB(gfx_pal[color*3],gfx_pal[color*3+1],
                           gfx_pal[color*3+2],gray,gray,gray,i<<1);
    rectfill(screen,225,(i<<1)+32,239,(i<<1)+33,i+160);
    }

  RAMpal[edc] = mixRGB(gfx_pal[color*3],gfx_pal[color*3+1],
                       gfx_pal[color*3+2],gray,gray,gray,ratio);
  RAMpal[edi] = invRGB(RAMpal[edc]);
  set_palette_range(RAMpal,160,255,false);

  rectfill(screen,96,32,223,95,edc);
  hline(screen,96,gray+32,223,edi);
  vline(screen,color+96,32,95,edi);
  hline(screen,225,ratio+32,239,edi);
  text_mode(ed1);
  textprintf(screen,font,104,104,ed15,"RGB - %2d %2d %2d",
             RAMpal[edc].r,RAMpal[edc].g,RAMpal[edc].b);
  unscare_mouse();
  SCRFIX();
}

int color = 0;
int gray  = 0;
int ratio = 32;

void edit_dataset(int dataset)
{
  PALETTE holdpal;
  holdpal = RAMpal;
  int index = 0;

  for(int i=240; i<256; i++)
    RAMpal[i] = ((RGB*)data[PAL_RAT].dat)[i];
  RAMpal[ed0]  = RAMpal[vc(0)];
  RAMpal[ed1]  = RAMpal[vc(1)];
  RAMpal[ed7]  = RAMpal[vc(7)];
  RAMpal[ed11] = RAMpal[vc(11)];
  RAMpal[ed14] = RAMpal[vc(14)];
  RAMpal[ed15] = RAMpal[vc(15)];

  vsync();
  scare_mouse();
  clear_to_color(screen,ed1);
  load_cset(RAMpal,12,dataset);
  load_cset(RAMpal,14,dataset);
  set_palette_range(RAMpal,0,255,false);

  init_colormixer();
  colormixer(color,gray,ratio);

  rectfill(screen,95,153,224,171,ed15);
  for(int i=0; i<16; i++)
    rectfill(screen,(i<<3)+96,154,(i<<3)+103,161,12*16+i);
  draw_edit_dataset_specs(index,-1);

  draw_button(screen,240,152,60,16,"OK",ed1,ed14);
  draw_button(screen,240,184,60,16,"Cancel",ed1,ed14);
  draw_button(screen,130,120,60,16,"Insert",ed1,ed14);

  unscare_mouse();
  while(mouse_b);
  bool bdown=false;
  int doing=0;
  int copy=-1;

  int done=0;
  do {
    bool setpal=false;
    int x=mouse_x;
    int y=mouse_y;
    if(mouse_b==1 && !bdown) {
      if(isinRect(x,y,96,32,223,95))
        doing=1;
      if(isinRect(x,y,225,32,239,95))
        doing=3;
      if(isinRect(x,y,96,153,223,180))
        doing=2;
      if(isinRect(x,y,240,152,299,167))
        if(do_button(240,152,60,16,"OK",ed1,ed14))
          done=2;
      if(isinRect(x,y,240,184,299,199))
        if(do_button(240,184,60,16,"Cancel",ed1,ed14))
          done=1;
      if(isinRect(x,y,130,120,189,135))
        if(do_button_reset(130,120,60,16,"Insert",ed1,ed14)) {
          RAMpal[14*16+index] = RAMpal[edc];
          setpal=true;
          }
      bdown=true;
      }

    if(mouse_b==1) {
      switch(doing) {
      case 1:
        color = vbound(x-96,0,127);
        gray = vbound(y-32,0,63);
        break;
      case 2:
        index = vbound((x-96)>>3,0,15);
        break;
      case 3:
        ratio = vbound(y-32,0,63);
        break;
      }
     }

    if(mouse_b==0) {
      bdown=false;
      doing=0;
      }

    if(keypressed())
    {
      int k = readkey();
      switch(k>>8)
      {
      case KEY_ESC:   done=1; break;
      case KEY_ENTER: done=2; break;
      case KEY_LEFT:  index=(index-1)&15; break;
      case KEY_RIGHT: index=(index+1)&15; break;
      case KEY_HOME:  RAMpal[14*16+index].r++; RAMpal[14*16+index].r&=0x3F; setpal=true; break;
      case KEY_END:   RAMpal[14*16+index].r--; RAMpal[14*16+index].r&=0x3F; setpal=true; break;
      case KEY_UP:    RAMpal[14*16+index].g++; RAMpal[14*16+index].g&=0x3F; setpal=true; break;
      case KEY_DOWN:  RAMpal[14*16+index].g--; RAMpal[14*16+index].g&=0x3F; setpal=true; break;
      case KEY_PGUP:  RAMpal[14*16+index].b++; RAMpal[14*16+index].b&=0x3F; setpal=true; break;
      case KEY_PGDN:  RAMpal[14*16+index].b--; RAMpal[14*16+index].b&=0x3F; setpal=true; break;
      case KEY_C:     copy=index; break;
      case KEY_V:     if(copy>=0) { RAMpal[14*16+index]=RAMpal[14*16+copy]; setpal=true; copy=-1; } break;
      case KEY_I:
      case KEY_INSERT: RAMpal[14*16+index]=RAMpal[edc]; setpal=true; break;
      case KEY_F1:    doHelp(ed0,ed7); break;
      default:
        switch(k&255)
        {
        case '4':  index=(index-1)&15; break;
        case '6':  index=(index+1)&15; break;
        case '7':  RAMpal[14*16+index].r++; RAMpal[14*16+index].r&=0x3F; setpal=true; break;
        case '1':  RAMpal[14*16+index].r--; RAMpal[14*16+index].r&=0x3F; setpal=true; break;
        case '8':  RAMpal[14*16+index].g++; RAMpal[14*16+index].g&=0x3F; setpal=true; break;
        case '2':  RAMpal[14*16+index].g--; RAMpal[14*16+index].g&=0x3F; setpal=true; break;
        case '9':  RAMpal[14*16+index].b++; RAMpal[14*16+index].b&=0x3F; setpal=true; break;
        case '3':  RAMpal[14*16+index].b--; RAMpal[14*16+index].b&=0x3F; setpal=true; break;
        }
      }
     }

    if(mouse_b && (doing==1 || doing==3))
      colormixer(color,gray,ratio);
    else {
      vsync();
      scare_mouse();
      if(setpal)
        set_palette_range(RAMpal,14*16,15*16,false);
      draw_edit_dataset_specs(index,copy);
      unscare_mouse();
      SCRFIX();
      }

  } while(!done);

  if(done==2)
    get_cset(dataset,14,RAMpal);

  RAMpal = holdpal;
  set_palette(RAMpal);
  while(mouse_b);
}


int pal_index(RGB *pal,RGB c)
{
  for(int i=0; i<256; i++)
    if(pal[i].r==c.r && pal[i].g==c.g && pal[i].b==c.b)
      return i;
  return -1;
}


void grab_dataset(int dataset)
{
  int row=0;
  static int palx=176;
  static int paly=64;

  PALETTE tmp;

  if(!pic && load_the_pic())
    return;

  int bg = gui_bg_color;
  int fg = gui_fg_color;
  gui_bg_color = pblack;
  gui_fg_color = pwhite;


  char fname[13];
  extract_name(imagepath,fname,FILENAME8_3);

  draw_bw_mouse(pwhite);

  scare_mouse();
  clear(screen);
  set_mouse_sprite(mouse_bmp[12]);
  unscare_mouse();
  set_palette(picpal);

  bool redraw=true;
  bool reload=false;
  int done=0;
  int f=0;

  do {
    int x=mouse_x;
    int y=mouse_y;

    vsync();

    if(reload)
    {
      reload=false;
      if(load_the_pic()==2)
        done=1;
      else
      {
        clear(screen);
        set_palette(picpal);
        redraw=true;
      }
    }

    if(redraw)
    {
      redraw=false;
      scare_mouse();
      clear_to_color(screen,pblack);
      blit(pic,screen,0,0,0,0,pic->w,pic->h);
      text_mode(pblack);
      textout(screen,font,fname,0,232,pwhite);
      draw_button(screen,184,224,60,16,"File",pblack,pwhite);
      draw_button(screen,260,200,60,16,"OK",pblack,pwhite);
      draw_button(screen,260,224,60,16,"Cancel",pblack,pwhite);
      unscare_mouse();
    }

    if((mouse_b&1) && isinRect(x,y,palx,paly,palx+127,paly+127))
      row=((y-paly)>>3);

    if((mouse_b&1) && isinRect(x,y,184,224,243,239))
      if(do_button(184,224,60,16,"File",pblack,pwhite))
        reload=true;

    if((mouse_b&1) && isinRect(x,y,260,200,319,215))
      if(do_button(260,200,60,16,"OK",pblack,pwhite))
        done=2;

    if((mouse_b&1) && isinRect(x,y,260,224,319,239))
      if(do_button(260,224,60,16,"Cancel",pblack,pwhite))
        done=1;


    if(keypressed()) {
      switch(readkey()>>8) {
      case KEY_ESC:   done=1; break;
      case KEY_ENTER: done=2; break;
      case KEY_SPACE: reload=true; break;
      case KEY_UP:    row=(row-1)&15; break;
      case KEY_DOWN:  row=(row+1)&15; break;
      case KEY_LEFT:
      case KEY_RIGHT:
        palx = 192-palx;
        redraw=true;
        break;
      case KEY_TAB:
        tmp = picpal;
        for(int i=0; i<16; i++)
          tmp[(row<<4)+i] = invRGB(tmp[(row<<4)+i]);
        for(int i=0; i<12; i++) {
          vsync();
          if(i&2)
            set_palette(picpal);
          else
            set_palette(tmp);
          }
        break;
      }
    }

    scare_mouse();
    for(int i=0; i<256; i++) {
      int x=((i&15)<<3)+palx;
      int y=((i>>4)<<3)+paly;
      rectfill(screen,x,y,x+7,y+7,i);
      }
    f++;
    rect(screen,palx-1,paly-1,palx+128,paly+128,pwhite);
    rect(screen,palx-1,(row<<3)+paly-1,palx+128,(row<<3)+paly+8,(f&2)?pwhite:pblack);
    unscare_mouse();
    SCRFIX();
  } while(!done);

  if(done==2)
    get_cset(dataset,row,picpal);

  gui_bg_color = bg;
  gui_fg_color = fg;

  while(mouse_b);
  scare_mouse();
  set_mouse_sprite(mouse_bmp[0]);
  unscare_mouse();
}




byte cset_hold[15][16*3];
byte cset_hold_cnt;
bool cset_ready = false;
int cset_count,cset_first;
PALETTE pal,undopal;

#define FLASH 243
byte rc[16] = {253,248,0,0,0,0,0,246,247,249,250,251,252,240,255,254};

void undo_pal()
{
  for(int i=0; i<cset_count; i++)
    get_cset(cset_first+i,i,undopal);
  undopal=pal;
}


void calc_dark(int first)
{
  undopal=pal;

  PALETTE tmp;
  tmp = black_palette;

  fade_interpolate(pal,black_palette,tmp,16,0,47);
  for(int i=0; i<48; i++)
    tmp[i+64] = tmp[i];

  fade_interpolate(pal,black_palette,tmp,32,0,47);
  for(int i=0; i<48; i++)
    tmp[i+112] = tmp[i];

  tmp[160+3] = tmp[3];

  for(int i=4; i<11; i++)
    get_cset(first+i,i,tmp);
}



static DIALOG cycle_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   48,   192,  152,  rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   160,  8,    rc[15],  rc[1],  0,       0,          0,             0,       (void *) "Palette Cycles" },
 { d_button_proc,     90,   176,  60,   16,   rc[14],  rc[1],  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  176,  60,   16,   rc[14],  rc[1],  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_text_proc,       152,  72,   96,   8,    rc[14],  rc[1],  0,       0,          0,             0,       (void *) "1   2   3" },
 { d_text_proc,       88,   88,   56,   8,    rc[11],  rc[1],  0,       0,          0,             0,       (void *) "CSet:" },
 { d_text_proc,       88,   104,  56,   8,    rc[11],  rc[1],  0,       0,          0,             0,       (void *) "First:" },
 { d_text_proc,       88,   120,  56,   8,    rc[11],  rc[1],  0,       0,          0,             0,       (void *) "Last:" },
 { d_text_proc,       88,   136,  56,   8,    rc[11],  rc[1],  0,       0,          0,             0,       (void *) "Count:" },
 { d_text_proc,       88,   152,  56,   8,    rc[11],  rc[1],  0,       0,          0,             0,       (void *) "Speed:" },
// 10
 { d_edit_proc,       152,  88,   24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    152,  104,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    152,  120,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_edit_proc,       152,  136,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },
 { d_edit_proc,       152,  152,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },

 { d_edit_proc,       184,  88,   24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    184,  104,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    184,  120,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_edit_proc,       184,  136,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },
 { d_edit_proc,       184,  152,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },

 { d_edit_proc,       216,  88,   24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    216,  104,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    216,  120,  24,   8,    rc[12],  rc[1],  0,       0,          1,             0,       NULL },
 { d_edit_proc,       216,  136,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },
 { d_edit_proc,       216,  152,  24,   8,    rc[12],  rc[1],  0,       0,          3,             0,       NULL },

 { NULL }
};


void edit_cycles(int level)
{
  char buf[15][8];

  for(int i=0; i<3; i++)
  {
    palcycle c = misc.cycles[level][i];
    sprintf(buf[i*5  ],"%d",c.first>>4);
    sprintf(buf[i*5+1],"%X",c.first&15);
    sprintf(buf[i*5+2],"%X",c.count&15);
    sprintf(buf[i*5+3],"%d",c.count>>4);
    sprintf(buf[i*5+4],"%d",c.speed);
  }

  for(int i=0; i<15; i++)
    cycle_dlg[i+10].dp = buf[i];

  if(popup_dialog(cycle_dlg,3)==2)
  {
    saved=false;
    reset_pal_cycling();
    for(int i=0; i<3; i++)
    {
      palcycle c;
      c.first =  (atoi(buf[i*5])&7)<<4;
      c.first += xtoi(buf[i*5+1])&15;
      c.count =  xtoi(buf[i*5+2])&15;
      c.count += (atoi(buf[i*5+3])&15)<<4;
      c.speed =  atoi(buf[i*5+4]);
      misc.cycles[level][i] = c;
    }
  }
}


void draw_cset_proc(DIALOG *d)
{
  int d1 = min(d->d1, d->d2);
  int d2 = max(d->d1, d->d2);

  d_bitmap_proc(MSG_DRAW,d,0);
  text_mode(d->bg);

  rect(screen,d->x,(d1<<3)+d->y,d->x+d->w-1,(d2<<3)+d->y+7,FLASH);
  int drc;
  if((d->flags & D_GOTFOCUS))
    drc = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
  else
    drc = d->bg;
  dotted_rect(d->x-1, d->y-1, d->x+d->w, d->y+d->h, drc, d->bg);
}



int d_cset_proc(int msg,DIALOG *d,int c)
{
  switch(msg) {
  case MSG_START:
    d->d2 = d->d1;
    break;

  case MSG_WANTFOCUS:
    return D_WANTFOCUS;

  case MSG_DRAW:
    draw_cset_proc(d);
    break;

  case MSG_CLICK: {
    int fc=0;
    do {
      int x=mouse_x;
      int y=mouse_y;
      if(isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
      {
        d->d2 = (y-d->y)>>3;
        if(!(key_shifts&KB_SHIFT_FLAG))
          d->d1 = d->d2;
      }
      vsync();
      scare_mouse();
      draw_cset_proc(d);
      unscare_mouse();
      ((RGB*)d->dp3)[243]=((RGB*)d->dp3)[rc[(fc++)&15]];
      set_palette_range(((RGB*)d->dp3),FLASH,FLASH,false);
    } while(mouse_b);
    }
    break;

  case MSG_CHAR: {
    int shift = (key_shifts&KB_SHIFT_FLAG);
    int k=c>>8;
    switch(k) {
     case KEY_UP:    if(d->d2>0) d->d2--;             if(!shift) d->d1 = d->d2; break;
     case KEY_DOWN:  if(d->d2<((d->h)>>3)-1) d->d2++; if(!shift) d->d1 = d->d2; break;
     case KEY_PGUP:  d->d2=0;                         if(!shift) d->d1 = d->d2; break;
     case KEY_PGDN:  d->d2=((d->h)>>3)-1;             if(!shift) d->d1 = d->d2; break;

     case KEY_C:
       cset_hold_cnt=0;
       for(int row=0; row <= abs(d->d1 - d->d2); row++)
       {
         int d1 = min(d->d1,d->d2);
         cset_hold_cnt++;
         for(int i=0; i<16*3; i++)
           cset_hold[row][i] = *(((byte*)d->dp2)+CSET(d1+row)*3+i);
       }
       cset_ready=true;
       break;

     case KEY_V:
       if(cset_ready)
       {
         undopal=pal;
         int d1 = min(d->d1,d->d2);
         for(int row=0; row<cset_hold_cnt && d1+row<cset_count; row++)
         {
           for(int i=0; i<16*3; i++)
             *(((byte*)d->dp2)+CSET(d1+row)*3+i) = cset_hold[row][i];
         }
         for(int i=0; i<cset_count; i++)
           load_cset(pal,i,cset_first+i);
         set_palette(pal);
       }
       break;

     case KEY_U:
       undo_pal();
       for(int i=0; i<cset_count; i++)
         load_cset(pal,i,cset_first+i);
       set_palette(pal);
       break;

     default:
       return D_O_K;
     }
    vsync();
    scare_mouse();
    draw_cset_proc(d);
    unscare_mouse();
    return D_USED_CHAR;
    }
  }
  return D_O_K;
}



int d_dummy_proc(int msg,DIALOG *d,int c) { return D_O_K; }

byte mainpal_csets[30]    = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
byte levelpal_csets[26]   = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
byte levelpal2_csets[26]  = { 2,3,4,9,2,0,1,2,3,4, 5, 6, 7,       15,15,15,15, 8,  9,9,9,9,9,9,9,9 };
byte spritepal_csets[30]  = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
byte spritepal2_csets[30] = { 15,16,17,18,19,20,21,22,23,24,25,26,27,28,29, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };

static DIALOG colors_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 40,   16,   240,  216,  rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  24,   152,  8,    rc[15],  rc[1],  0,       0,          0,             0,       NULL },
 { d_cset_proc,       104,  48,   256,  96,   rc[14],  rc[1],  0,       0,          0,             0,       NULL },

 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },
 { d_text_proc,       80,   0,    16,   8,    rc[14],  rc[1],  0,       0,          0,             0,       NULL },

 { d_text_proc,       104,  39,   128,  8,    rc[9],   rc[1],  0,       0,          0,             0,       (void *) "0123456789ABCDEF" },
// 19
 { d_button_proc,     60,   178,  60,   16,   rc[14],  rc[1],  'e',     D_EXIT,     0,             0,       (void *) "&Edit" },
 { d_button_proc,     130,  178,  60,   16,   rc[14],  rc[1],  'g',     D_EXIT,     0,             0,       (void *) "&Grab" },
 { d_button_proc,     200,  178,  60,   16,   rc[14],  rc[1],  0,       D_EXIT,     0,             0,       (void *) "Cycle" },
 { d_button_proc,     244,  152,  24,   16,   rc[14],  rc[1],  'u',     D_EXIT,     0,             0,       (void *) "&U" },
// 23
 { d_button_proc,     60,   204,  130,  16,   rc[14],  rc[1],  'l',     D_EXIT,     0,             0,       (void *) "&Load to CS 9" },
 { d_button_proc,     200,  204,  60,   16,   rc[14],  rc[1],  27,      D_EXIT,     0,             0,       (void *) "Done" },
 { d_text_proc,       60,   162,  48,   8,    rc[14],  rc[1],  0,       0,          0,             0,       (void *) "Name: " },
 { d_edit_proc,       108,  162, 136,   8,    rc[12],  rc[1],  0,       0,          16,            0,       NULL },
 { NULL }
};


int EditColors(char *caption,int first,int count,byte *label)
{
  char tempstuff[17];
  cset_first=first;
  cset_count=count;
  for(int i=0; i<240; i++)
    pal[i] = RAMpal[rc[1]];
  for(int i=240; i<256; i++)
    pal[i] = ((RGB*)data[PAL_RAT].dat)[i];

  go();

  BITMAP *bmp = create_bitmap(128,count*8);
  if(!bmp)
    return 0;
  for(int i=0; i<16*count; i++) {
    int x=(i&15)<<3;
    int y=(i>>4)<<3;
    rectfill(bmp,x,y,x+7,y+7,i);
    }
  colors_dlg[2].dp = bmp;
  colors_dlg[2].w  = bmp->w;
  colors_dlg[2].h  = bmp->h;
  colors_dlg[2].dp2 = colordata+CSET(first)*3;
  colors_dlg[2].dp3 = pal;
  colors_dlg[21].proc = (count==pdLEVEL) ? d_button_proc : d_dummy_proc;
  colors_dlg[21].dp   = get_bit(header.rules,qr1_FADE) ? (void *) "Cycle" : (void *) "Dark";
  colors_dlg[26].dp   =  tempstuff;
// { d_text_proc,       60,   162,  48,   8,    rc[14],  rc[1],  0,       0,          0,             0,       (void *) "Name: " },
// { d_edit_proc,       108,  162, 136,   8,    rc[12],  rc[1],  0,       0,          16,            0,       NULL },
  colors_dlg[25].x    =(count==pdLEVEL)?60:52;
  colors_dlg[25].w    =(count==pdLEVEL)?48:0;
  colors_dlg[25].dp   =(count==pdLEVEL)?(void *) "Name: ":(void *) " ";
  colors_dlg[26].x    =(count==pdLEVEL)?108:52;
  colors_dlg[26].w    =(count==pdLEVEL)?136:0;
  colors_dlg[26].d1   =(count==pdLEVEL)?16:0;
  colors_dlg[26].dp   =(count==pdLEVEL)?palnames[(first-poLEVEL)/pdLEVEL]:NULL;

  char buf[count][4];

  for(int i=0; i<15; i++) {
    if(i<count)
      sprintf(buf[i],"%2d",label[i]);
    else
      buf[i][0]=0;
    colors_dlg[3+i].dp=buf[i];
    colors_dlg[3+i].y=(i<<3)+48;
    colors_dlg[3+i].fg=rc[label[i+count]];
    }

  colors_dlg[1].dp = caption;

  for(int i=0; i<count; i++)
    load_cset(pal,i,i+first);
  undopal=pal;

  int ret=0;
  do {
    gui_fg_color = rc[14];
    gui_bg_color = rc[1];
    gui_mg_color = rc[9];

    for(int i=0; i<count; i++)
      load_cset(pal,i,i+first);
    set_palette(pal);
    scare_mouse();
    clear_to_color(screen,rc[0]);
    set_mouse_sprite((BITMAP*)data[BMP_RAT].dat);
    unscare_mouse();

    colors_dlg[19].flags =
    colors_dlg[20].flags =
    colors_dlg[23].flags = D_EXIT;

    DIALOG_PLAYER *p = init_dialog(colors_dlg,2);
    int fc=0;
    bool enable = true;

    while(update_dialog(p))
    {
      vsync();
      pal[FLASH]=pal[rc[(fc++)&15]];
      set_palette_range(pal,FLASH,FLASH,false);

      bool en = (colors_dlg[2].d1 == colors_dlg[2].d2);
      if(en!=enable)
      {
        colors_dlg[19].flags =
        colors_dlg[20].flags =
        colors_dlg[23].flags = en  ? D_EXIT : D_DISABLED;
        broadcast_dialog_message(MSG_DRAW,0);
        enable = en;
      }
    }
    ret = shutdown_dialog(p);

    if(ret==19)
    {
      undopal=pal;
      edit_dataset(first+colors_dlg[2].d2);
    }
    if(ret==20)
    {
      undopal=pal;
      grab_dataset(first+colors_dlg[2].d2);
    }
    if(ret==21)
    {
      if(!get_bit(header.rules,qr1_FADE))
        calc_dark(first);
      else
        edit_cycles((first-poLEVEL)/pdLEVEL);
    }
    if(ret==22)
      undo_pal();
  } while(ret<23);

  while(mouse_b);
  set_palette(RAMpal);
  clear_to_color(screen,vc(0));
  loadlvlpal(Color);

  if(ret==23) {
    load_cset(RAMpal,9,first+colors_dlg[2].d2);
    set_pal();
    }

  gui_fg_color = vc(14);
  gui_bg_color = vc(1);
  gui_mg_color = vc(9);
  set_mouse_sprite(mouse_bmp[0]);
  comeback();
  return int(ret==23);
}


int onColors_Main()
{
  int l9 = EditColors("Main Palette",0,pdFULL,mainpal_csets);

  // copy main to level 0
  int di = CSET(poLEVEL)*3;
  int si = CSET(2)*3;
  for(int i=0; i<CSET(3)*3; i++)
    colordata[di++] = colordata[si++];

  si = CSET(9)*3;
  for(int i=0; i<16*3; i++)
    colordata[di++] = colordata[si++];

  loadlvlpal(Color);
  if(l9)
  {
    load_cset(RAMpal,9,colors_dlg[2].d2);
    set_pal();
  }

  return D_O_K;
}

int onColors_Levels()
{
  int cycle = get_bit(header.rules,qr1_FADE);
  int index=0;
  while((index=select_data("Select Level",index,levelnumlist,"Edit","Done"))!=-1)
  {
    char buf[40];
    sprintf(buf,"Level %X Palettes",index);
    int l9 = EditColors(buf,index*pdLEVEL+poLEVEL,pdLEVEL,cycle?levelpal2_csets:levelpal_csets);
    setup_lcolors();
    if(index==0)
    { // copy level 0 to main
      int si = CSET(poLEVEL)*3;
      int di = CSET(2)*3;
      for(int i=0; i<CSET(3)*3; i++)
        colordata[di++] = colordata[si++];

      di = CSET(9)*3;
      for(int i=0; i<16*3; i++)
        colordata[di++] = colordata[si++];
    }
    loadlvlpal(Color);
    if(l9)
    {
      load_cset(RAMpal,9,index*pdLEVEL+poLEVEL+colors_dlg[2].d2);
      set_pal();
    }
  }
  return D_O_K;
}

int onColors_Sprites()
{
  int index;
  do {
    index = alert3("Select Extra Sprite Palette Set",NULL,NULL,"&1","&2","&Done",'1','2','d');
    if(index==1)
      EditColors("Extra Sprite Palettes 1",newpoSPRITE,15,spritepal_csets);
    if(index==2)
      EditColors("Extra Sprite Palettes 2",newpoSPRITE+15,15,spritepal2_csets);
  } while(index==1 || index==2);

  return D_O_K;
}



int d_maptile_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
  case MSG_START:
    d->dp3 = create_bitmap(d->w,d->h);
    break;

  case MSG_END:
    if((BITMAP*)(d->dp3))
      destroy_bitmap((BITMAP*)(d->dp3));
    break;

  case MSG_CLICK:
    if(select_tile(d->d1,d->d2,1,d->fg))
      return D_REDRAW;
    break;

  case MSG_DRAW:
    if((BITMAP*)(d->dp3))
    {
      clear((BITMAP*)(d->dp3));
      for(int y=0; y<d->h; y+=16)
        for(int x=0; x<d->w; x+=16)
        {
          if(d->d1)
            puttile16((BITMAP*)(d->dp3),d->d1+(y>>4)*20+(x>>4),x,y,d->fg,0);
          else
            rectfill((BITMAP*)(d->dp3),x+3,y+3,x+11,y+11,vc(4));
        }
      blit((BITMAP*)(d->dp3),screen,0,0,d->x,d->y,d->w,d->h);
    }
    break;
  }
  return D_O_K;
}




static DIALOG mapstyles_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 48,   24,   224,  192,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  32,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Map Styles && Colors" },
 { d_text_proc,       76,   47,   192,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Overworld    Dungeon" },
 { d_text_proc,       68,   115,  192,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Frame  Tri   Tri Frame" },
 { d_text_proc,       124,  147,  192,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "HCP" },
// 5
 { d_maptile_proc,    72,   56,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
 { d_maptile_proc,    168,  56,   80,   48,   0,       0,      0,       0,          0,             0,       NULL },
 { d_maptile_proc,    72,   124,  32,   32,   0,       0,      0,       0,          0,             0,       NULL },
 { d_maptile_proc,    128,  124,  16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_maptile_proc,    160,  124,  96,   48,   0,       0,      0,       0,          0,             0,       NULL },
 { d_maptile_proc,    128,  156,  16,   16,   0,       0,      0,       0,          0,             0,       NULL },
// 11
 { d_button_proc,     90,   184,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  184,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int onMapStyles()
{
  mapstyles_dlg[5].d1 = misc.colors.overworld_map_tile;
  mapstyles_dlg[5].fg = misc.colors.overworld_map_cset;
  mapstyles_dlg[6].d1 = misc.colors.dungeon_map_tile;
  mapstyles_dlg[6].fg = misc.colors.dungeon_map_cset;
  mapstyles_dlg[7].d1 = misc.colors.blueframe_tile;
  mapstyles_dlg[7].fg = misc.colors.blueframe_cset;
  mapstyles_dlg[8].d1 = misc.colors.triforce_tile;
  mapstyles_dlg[8].fg = misc.colors.triforce_cset;
  mapstyles_dlg[9].d1 = misc.colors.triframe_tile;
  mapstyles_dlg[9].fg = misc.colors.triframe_cset;
  mapstyles_dlg[10].d1 = misc.colors.HCpieces_tile;
  mapstyles_dlg[10].fg = misc.colors.HCpieces_cset;

  go();
  int ret = do_dialog(mapstyles_dlg,-1);
  comeback();

  if(ret==11)
  {
    misc.colors.overworld_map_tile = mapstyles_dlg[5].d1;
    misc.colors.overworld_map_cset = mapstyles_dlg[5].fg;
    misc.colors.dungeon_map_tile   = mapstyles_dlg[6].d1;
    misc.colors.dungeon_map_cset   = mapstyles_dlg[6].fg;
    misc.colors.blueframe_tile     = mapstyles_dlg[7].d1;
    misc.colors.blueframe_cset     = mapstyles_dlg[7].fg;
    misc.colors.triforce_tile      = mapstyles_dlg[8].d1;
    misc.colors.triforce_cset      = mapstyles_dlg[8].fg;
    misc.colors.triframe_tile      = mapstyles_dlg[9].d1;
    misc.colors.triframe_cset      = mapstyles_dlg[9].fg;
    misc.colors.HCpieces_tile      = mapstyles_dlg[10].d1;
    misc.colors.HCpieces_cset      = mapstyles_dlg[10].fg;
    saved=false;
  }
  return D_O_K;
}



int d_misccolors_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    textout(screen,font,"0123456789ABCDEF",d->x+8,d->y,d->fg);
    textout(screen,font,"0",d->x,d->y+8,d->fg);
    textout(screen,font,"1",d->x,d->y+16,d->fg);
    textout(screen,font,"5",d->x,d->y+24,d->fg);
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



static DIALOG misccolors_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60,   20,   200,  200,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  24,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Misc Colors" },
 { d_misccolors_proc, 92,   36,   128,  32,   vc(9),   vc(1),  0,       0,          0,             0,       NULL },

 { d_text_proc,       76,   76,   132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Text        : 0x" },
 { d_text_proc,       76,   84,   132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Caption     : 0x" },
 { d_text_proc,       76,   92,   132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Overworld BG: 0x" },
 { d_text_proc,       76,   100,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon BG  : 0x" },
 { d_text_proc,       76,   108,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Dungeon FG  : 0x" },
 { d_text_proc,       76,   116,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Cave FG     : 0x" },
 { d_text_proc,       76,   124,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "BS Dark     : 0x" },
 { d_text_proc,       76,   132,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "BS Goal     : 0x" },
 { d_text_proc,       76,   140,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Compass Lt  : 0x" },
 { d_text_proc,       76,   148,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Compass Dk  : 0x" },
 { d_text_proc,       76,   156,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Subscreen BG: 0x" },
 { d_text_proc,       76,   164,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Tri Frame   : 0x" },
 { d_text_proc,       76,   172,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Link's Pos  : 0x" },
 { d_text_proc,       76,   180,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Big map BG  : 0x" },
 { d_text_proc,       76,   188,  132,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Big map FG  : 0x" },

 //18
 { d_hexedit_proc,    204,  76,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  84,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  92,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  100,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  108,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  116,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  124,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  132,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  140,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  148,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  156,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  164,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  172,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  180,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,    204,  188,  32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },

 // 33
 { d_button_proc,     90,   200,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  200,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


int onMiscColors()
{
  char buf[15][8];
  byte *si = &(misc.colors.text);
  for(int i=0; i<15; i++)
  {
    sprintf(buf[i],"%02X",*si);
    si++;
    misccolors_dlg[i+18].dp = buf[i];
  }

  if(popup_dialog(misccolors_dlg,0)==33)
  {
    saved=false;
    si = &(misc.colors.text);
    for(int i=0; i<15; i++)
    {
      *si = xtoi(buf[i]);
      si++;
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
  if(!(Map.CurrScr()->valid&mVALID))
    return;

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

  if(refreshpal)
    set_palette_range(RAMpal,0,192,false);
}




/********************/
/******  Help  ******/
/********************/



static DIALOG help_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
 { d_textbox_proc,    -1,   -1,   321,  241,  0,       0,      0,       0,          0,        0,        NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC,  close_dlg },
 { NULL }
};




void doHelp(int bg,int fg)
{
 help_dlg[0].dp = helpbuf;
 help_dlg[0].bg = bg;
 help_dlg[0].fg = fg;

 swap(font,sfont);
 popup_dialog(help_dlg,0);
 swap(font,sfont);
}


int onHelp()
{
 doHelp(vc(0),vc(7));
 return D_O_K;
}

static DIALOG overheaddata_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 36,   20,   248,  200,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  24,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Layer Data" },

 { d_text_proc,       52,   76,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 1 Map: " },
 { d_text_proc,       52,   84,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 1 Screen: " },
 { d_text_proc,       52,   92,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 2 Map: " },
 { d_text_proc,       52,  100,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 2 Screen: " },
 { d_text_proc,       52,  108,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 3 Map: " },
 { d_text_proc,       52,  116,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 3 Screen: " },
 { d_text_proc,       52,  124,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 4 Map: " },
 { d_text_proc,       52,  132,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 4 Screen: " },
 { d_text_proc,       52,  140,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 5 Map: " },
 { d_text_proc,       52,  148,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 5 Screen: " },
 { d_text_proc,       52,  156,   104,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 6 Map: " },
 { d_text_proc,       52,  164,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Layer 6 Screen: " },

 //14
 { d_edit_proc,      156,   76,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,   84,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      156,   92,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,  100,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      156,  108,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,  116,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      156,  124,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,  132,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      156,  140,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,  148,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,      156,  156,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_hexedit_proc,   180,  164,   24,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },

 // 26
 { d_button_proc,     90,   200,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  200,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};


int onLayers()
{
  char buf[12][8];
  for (int x=0; x<6; x++) {
    sprintf(buf[x*2],"%d",Map.CurrScr()->layermap[x]);
    sprintf(buf[(x*2)+1],"%02X",Map.CurrScr()->layerscreen[x]);
  }
  for (int i=0; i<12; i++) {
    overheaddata_dlg[i+14].dp = buf[i];
  }

  if(popup_dialog(overheaddata_dlg,0)==26)
  {
    saved=false;
    for (int x=0; x<6; x++) {
      Map.CurrScr()->layermap[x]=atoi(buf[x*2]);
      Map.CurrScr()->layerscreen[x]=xtoi(buf[(x*2)+1]);
    }
  }
  return D_O_K;
}

// **** Timers ****

volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync = 0;

void myvsync_callback()
{
  myvsync++;
}
END_OF_FUNCTION(myvsync_callback);


void fps_callback()
{
  lastfps=framecnt;
  framecnt=0;
}
END_OF_FUNCTION(fps_callback);



/********************/
/******  MAIN  ******/
/********************/

int main(int argc,char **argv)
{
 if (IS_BETA) {
   Z_title("ZQuest %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
 } else {
   Z_title("ZQuest %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
 }
// Z_title("ZQuest %s",VerStr(ZELDA_VERSION));

 printf("Allocating data buffers... ");

 customMIDIs = (music*)malloc(sizeof(music)*MAXMIDIS);
 if(!customMIDIs || !get_qst_buffers())
   Z_error("Error");

 undocombobuf = (newcombo*)malloc(sizeof(newcombo)*MAXCOMBOS);
 undotilebuf = (byte*)malloc(NEWTILE_SIZE2);
 if(!undocombobuf || !undotilebuf)
   Z_error("Error");

 srand(undotilebuf[4]);

 filepath=(char*)malloc(256);
 temppath=(char*)malloc(256);
 datapath=(char*)malloc(256);
 midipath=(char*)malloc(256);
 imagepath=(char*)malloc(256);

 if(!filepath || !datapath || !temppath || !imagepath || !midipath)
   Z_error("Error");

 printf("OK\n"); // Allocating data buffers...


 printf("Initializing Allegro... ");
 set_config_file("ag.cfg");
 allegro_init();
 register_bitmap_file_type("GIF", load_gif, NULL);

 if(install_timer() < 0)
   Z_error(allegro_error);

 if(install_keyboard() < 0)
   Z_error(allegro_error);

 if(install_mouse() < 0)
   Z_error(allegro_error);

 LOCK_VARIABLE(lastfps);
 LOCK_VARIABLE(framecnt);
 LOCK_FUNCTION(fps_callback);
 if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
   Z_error("couldn't allocate timer");

 LOCK_VARIABLE(myvsync);
 LOCK_FUNCTION(myvsync_callback);
 if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60)))
   Z_error("couldn't allocate timer");

 printf("OK\n"); // Initializing Allegro...


 printf("Loading data files... ");
 if((data=load_datafile("#")))
   datafile_str="#";
 else
 {
   if(!(data=load_datafile("zquest.dat")))
     Z_error("Error loading zquest.dat");

   datafile_str="zquest.dat";
 }

 resolve_password(datapwd);
 packfile_password(datapwd);

 if((zgpdata=load_datafile("zgp.dat"))==NULL)
   Z_error("Error loading zgp.dat");

 font =(FONT*)data[FONT_GUI].dat;
 zfont=(FONT*)data[FONT_NES].dat;
 sfont=(FONT*)data[FONT_6x6].dat;


 for(int i=0; i<MAXMIDIS; i++)
 {
   customMIDIs[i].midi=NULL;
   midi_string[i+4]=customMIDIs[i].title;
 }


 int helpsize = file_size("zquest.txt");
 if(helpsize==0)
   Z_error("zquest.txt not found");

 helpbuf = (char*)malloc(helpsize+1);
 if(!helpbuf)
   Z_error("Error allocating help buffer");

 if(readfile("zquest.txt",helpbuf,helpsize)<helpsize)
   Z_error("Error loading zquest.txt");

 helpbuf[helpsize]=0;

 printf("OK\n"); // loading data files...


 filepath[0]=temppath[0]=0;
 strcpy(datapath,get_config_string("zquest","data_path",""));
 strcpy(midipath,get_config_string("zquest","midi_path",""));
 strcpy(imagepath,get_config_string("zquest","image_path",""));
 chop_path(datapath);
 chop_path(midipath);
 chop_path(imagepath);

 MouseScroll        = get_config_int("zquest","mouse_scroll",0);
 SavePaths          = get_config_int("zquest","save_paths",1);
 CycleOn            = get_config_int("zquest","cycle_on",1);
 Vsync              = get_config_int("zquest","vsync",1);
 ShowFPS            = get_config_int("zquest","showfps",0);
 ComboBrush         = get_config_int("zquest","combo_brush",0);
 BrushPosition      = get_config_int("zquest","brush_position",0);
 FloatBrush         = get_config_int("zquest","float_brush",0);
 OpenLastQuest      = get_config_int("zquest","open_last_quest",0);
 ShowMisalignments  = get_config_int("zquest","show_misalignments",0);
 AnimationOn        = get_config_int("zquest","animation_on",1);
 int tempvalue      = get_config_int("zquest","layer_mask",-1);
 LayerMask[0]=byte(tempvalue&0xFF);
 LayerMask[1]=byte((tempvalue>>8)&0xFF);
 for (int x=0; x<7; x++) {
   LayerMaskInt[x]=get_bit(LayerMask,x);
 }
 DuplicateAction[0] = get_config_int("zquest","normal_duplicate_action",2);
 DuplicateAction[1] = get_config_int("zquest","horizontal_duplicate_action",0);
 DuplicateAction[2] = get_config_int("zquest","vertical_duplicate_action",0);
 DuplicateAction[3] = get_config_int("zquest","both_duplicate_action",0);
 LeechUpdate = get_config_int("zquest","leech_update",500);
 OnlyCheckNewTilesForDuplicates = get_config_int("zquest","only_check_new_tiles_for_duplicates",0);

 if (OpenLastQuest&&!used_switch(argc,argv,"-new")) {
   strcpy(filepath,get_config_string("zquest","last_quest",""));
 }

 if(used_switch(argc,argv,"-d"))
 {
   resolve_password(zquestpwd);
   debug = !strcmp(zquestpwd,get_config_string("zquest","debug_this",""));
 }

 build_bie_list();


 printf("Initializing sound driver... ");
 if(used_switch(argc,argv,"-s"))
   printf("skipped\n");
 else
 {
    if(install_sound(DIGI_AUTODETECT,DIGI_AUTODETECT,NULL)) {
//      Z_error(allegro_error);
      printf("Sound driver not available.  Sound disabled.\n");
    } else {
      printf("OK\n");
    }
 }

 if(used_switch(argc,argv,"-q"))
 {
   printf("-q switch used, quitting program.\n");
   exit(0);
 }

 int mode = GFX_AUTODETECT;
 if(used_switch(argc,argv,"-modex"))  mode=GFX_MODEX;
 if(used_switch(argc,argv,"-vesa1"))  mode=GFX_VESA1;
 if(used_switch(argc,argv,"-vesa2b")) mode=GFX_VESA2B;
 if(used_switch(argc,argv,"-vesa2l")) mode=GFX_VESA2L;
 if(used_switch(argc,argv,"-vesa3"))  mode=GFX_VESA3;

 switch(mode)
 {
 case GFX_AUTODETECT:
 case GFX_VESA3:   if(set_gfx_mode(GFX_VESA3,320,240,0,0)==0)
                     break;
 case GFX_VESA2L:  if(set_gfx_mode(GFX_VESA2L,320,240,0,0)==0)
                     break;
 case GFX_VESA2B:  if(set_gfx_mode(GFX_VESA2B,320,240,0,0)==0)
                     break;
 case GFX_VESA1:   if(set_gfx_mode(GFX_VESA1,320,240,0,0)==0)
                     break;
 case GFX_MODEX:   if(set_gfx_mode(GFX_MODEX,320,240,0,0)==0)
                     break;
 default: Z_error(allegro_error);
 }


 screen2 = create_bitmap(320,240);
 menu = create_bitmap(320,240);
 menu2 = create_bitmap(320,240);
 combo_bmp = create_bitmap(16,16);
 dmapbmp = create_bitmap(65,33);
 brushbmp = create_bitmap(16, 16);
 brushshadowbmp = create_bitmap(16, 16);
 brushscreen = create_bitmap(256, 192);
 if(!screen2 || !menu || !menu2 || !combo_bmp || !dmapbmp || !brushbmp
             || !brushshadowbmp || !brushscreen )
 {
   allegro_exit();
   printf("Error creating bitmaps\n");
   return 1;
 }

 set_palette((RGB*)data[PAL_ZQUEST].dat);
 get_palette(RAMpal);

 fade_interpolate(RAMpal+vc(1),black_palette,RAMpal+vc(1)-32,31,0,0);
 fade_interpolate(RAMpal+vc(9),black_palette,RAMpal+vc(9)-32,31,0,0);
 fade_interpolate(RAMpal+vc(14),black_palette,RAMpal+vc(14)-32,31,0,0);
 set_palette_range(RAMpal,vc(0)-32,vc(15)-32,true);

 clear_to_color(screen,vc(0));

 init_quest();

 load_mice();
 load_arrows();
 clear_to_color(menu,vc(0));
 refresh(rALL);
 DIALOG_PLAYER *player=init_dialog(dialogs,-1);
 gui_mouse_focus=0;
 gui_mg_color=vc(9);
 gui_bg_color=vc(1);
 gui_fg_color=vc(14);
 set_mouse_sprite(mouse_bmp[0]);
 show_mouse(screen);

 quit=!update_dialog(player);

 if(argc>1 && argv[1][0]!='-')
 {
   replace_extension(temppath,argv[1],"qst",255);
   if(load_quest(temppath)==0)
   {
     strcpy(filepath,temppath);
     refresh(rALL);
   }
 } else {
   if (OpenLastQuest&&!used_switch(argc,argv,"-clear")) {
     if(load_quest(filepath)==0)
     {
       refresh(rALL);
     } else {
       filepath[0]=temppath[0]=0;
     }
   }
 }

 Map.setCurrMap(zinit.lastmap);
 Map.setCurrScr(zinit.lastscreen);
 refresh(rALL);
 setup_combo_animations();
 brush_width_menu[0].flags=D_SELECTED;
 brush_height_menu[0].flags=D_SELECTED;
 fill_menu[0].flags=D_SELECTED;

 while(!quit)
 {
   if (AnimationOn) {
     animate_combos();
//   putscr(scrollbuf,0,0,tmpscr);
     refresh(rALL);
   }

   // update the menu status
   if(strcmp(catchall_string[Map.CurrScr()->room]," "))
   {
     static char ca_menu_str[40];
     sprintf(ca_menu_str,"%s\tA",catchall_string[Map.CurrScr()->room]);
     data_menu[10].text=ca_menu_str;
     data_menu[10].flags=0;
   }
   else
   {
     data_menu[10].text="Catch All\tA";
     data_menu[10].flags=D_DISABLED;
   }

   edit_menu[0].flags = Map.CanUndo() ? 0 : D_DISABLED;
   edit_menu[2].flags =
   edit_menu[3].flags =
   edit_menu[4].flags =
   paste_menu[0].flags =
   paste_menu[1].flags = Map.CanPaste() ? 0 : D_DISABLED;
   edit_menu[1].flags =
   edit_menu[5].flags = (Map.CurrScr()->valid&mVALID) ? 0 : D_DISABLED;

   tool_menu[0].flags =
   tool_menu[1].flags = (Map.getCurrScr()<TEMPLATE) ? 0 : D_DISABLED;

   defs_menu[1].flags = usetiles ? 0 : D_DISABLED;

   if(Vsync)
     vsync();
   else
     while(!myvsync);
   myvsync=0;

   framecnt++;

   if(ShowFPS)
   {
     text_mode(vc(0));
     textprintf(screen,sfont,0,16,vc(15),"FPS:%-3d",lastfps);
   }

   if(CycleOn)
     cycle_palette();

   domouse();

   quit = !update_dialog(player);
 }

 chop_path(datapath);
 chop_path(midipath);
 chop_path(imagepath);
 set_config_string("zquest","data_path",datapath);
 set_config_string("zquest","midi_path",midipath);
 set_config_string("zquest","image_path",imagepath);
 set_config_string("zquest","last_quest",filepath);
 set_config_int("zquest","mouse_scroll",MouseScroll);
 set_config_int("zquest","save_paths",SavePaths);
 set_config_int("zquest","cycle_on",CycleOn);
 set_config_int("zquest","vsync",Vsync);
 set_config_int("zquest","showfps",ShowFPS);
 set_config_int("zquest","combo_brush",ComboBrush);
 set_config_int("zquest","brush_position",BrushPosition);
 set_config_int("zquest","float_brush",FloatBrush);
 set_config_int("zquest","open_last_quest",OpenLastQuest);
 set_config_int("zquest","show_misalignments",ShowMisalignments);
 set_config_int("zquest","animation_on",AnimationOn);
 for (int x=0; x<7; x++) {
   set_bit(LayerMask,x, LayerMaskInt[x]);
 }
 tempvalue=LayerMask[0]+(LayerMask[1]<<8);
 set_config_int("zquest","layer_mask",tempvalue);
 set_config_int("zquest","normal_duplicate_action",DuplicateAction[0]);
 set_config_int("zquest","horizontal_duplicate_action",DuplicateAction[1]);
 set_config_int("zquest","vertical_duplicate_action",DuplicateAction[2]);
 set_config_int("zquest","both_duplicate_action",DuplicateAction[3]);
 set_config_int("zquest","leech_update",LeechUpdate);
 set_config_int("zquest","only_check_new_tiles_for_duplicates",OnlyCheckNewTilesForDuplicates);


 set_palette(black_palette);
 stop_midi();
 unload_datafile(data);
 allegro_exit();
 return 0;

}



/* end */

