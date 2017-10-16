/*
  zquest.cc
  Jeremy Craner, 1999
  Quest & map editor for the DJGPP version of Zelda.
*/

#define  INTERNAL_VERSION  0xA105
#define  _ZQUEST_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include "allegro.h"
#include "jpeg.h"

#include "zquest.h"

// the following are used by both zelda.cc and zquest.cc
#include "zdefs.h"
#include "colors.cc"
#include "qst.cc"
#include "midi.cc"

#define MIN16  8
bool temptile=false;
#include "tiles.cc"

#include "sprite.cc"
#include "gui.cc"


enum { m_block, m_coords, m_flags, m_guy, m_warp, m_misc, m_menucount };

#define MAXMICE 13

char *datafile_str;
DATAFILE *data=NULL;
MIDI *song=NULL;
FONT *zfont;
BITMAP *menu,*menu2,*combo_bmp,*screen2,*mouse_bmp[MAXMICE],*dmapbmp;
byte *tilebuf=NULL,*colordata=NULL;
combo *combobuf;
combo curr_combo;
PALETTE RAMpal;
midi_info Midi_Info;
short Flip=0,Combo=0,CSet=2,First=0;
int  Flags=0,Flag=1,menutype=m_block;
bool quit=false,saved=true,debug=false,usetiles=false;
byte Color;

char *filepath,*temppath,*midipath,*datapath,*imagepath;
char *helpbuf;


// quest data
zquestheader header;
miscQdata misc;
mapscr *TheMaps;
dmap   *DMaps;
MsgStr *MsgStrings;
music  *customMIDIs;



#include "zq_misc.cc"


static MENU import_menu[] =
{ 
   { "Map",             onImport_Map,     NULL },
   { "DMaps",           onImport_DMaps,   NULL },
   { "Tiles",           onImport_Tiles,   NULL },
   { "Palettes",        onImport_Pals,    NULL },
   { "String Table",    onImport_Msgs,    NULL },
   { "Combo Table",     onImport_Combos,  NULL },
   { NULL }
};

static MENU export_menu[] =
{ 
   { "Map",             onExport_Map,     NULL },
   { "DMaps",           onExport_DMaps,   NULL },
   { "Tiles",           onExport_Tiles,   NULL },
   { "Palettes",        onExport_Pals,    NULL },
   { "String Table",    onExport_Msgs,    NULL },
   { "Combo Table",     onExport_Combos,  NULL },
   { NULL }
};


static MENU file_menu[] =
{ 
   { "&New",            onNew,       NULL },
   { "&Open\tF3",       onOpen,      NULL },
   { "&Save\tF2",       onSave,      NULL },
   { "Save &as...",     onSaveAs,    NULL },
   { "" },
   { "&Import\t\x86",   NULL,        import_menu },
   { "&Export\t\x86",   NULL,        export_menu },
   { "" },
   { "E&xit\tESC",      onExit,      NULL },
   { NULL }
};

static MENU maps_menu[] =
{ 
   { "Map &1",         onMap1,        NULL },
   { "Map &2",         onMap2,        NULL },
   { "Map &3",         onMap3,        NULL },
   { "Map &4",         onMap4,        NULL },
   { "Map &5",         onMap5,        NULL },
   { "Map &6",         onMap6,        NULL },
   { "Map &7",         onMap7,        NULL },
   { "Map &8",         onMap8,        NULL },
   { "Map &9",         onMap9,        NULL },
   { "Map 1&0",        onMap10,       NULL },
   { "" },
   { "Map Count",      onMapCount,    NULL },
   { NULL }
};

static MENU misc_menu[] =
{ 
   { "Shop types",        onShopTypes,  NULL },
   { "Info types",        onInfoTypes,  NULL },
   { "Warp rings",        onWarpRings,  NULL },
   { "Whistle warps",     onWhistle,    NULL },
   { "Triforce pieces",   onTriPieces,  NULL },
   { "Map colors",        onMapColors,  NULL },
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
   { "&Map Colors",     onDefault_MapColors, NULL },
   { NULL }
};

static MENU quest_menu[] =
{ 
   { "Maps\t\x86",      NULL,         maps_menu },
   { "Palettes\t\x86",  NULL,         colors_menu },
   { "Misc Data\t\x86", NULL,         misc_menu },
   { "" },
   { "&Header",         onHeader,     NULL },
   { "&Rules",          onRules,      NULL },
   { "&Strings",        onStrings,    NULL },
   { "&DMaps",          onDmaps,      NULL },
   { "&Combos",         onCombos,     NULL },
   { "&Tiles",          onTiles,      NULL },
   { "&MIDIs",          onMidis,      NULL },
   { "" },
   { "Defaults\t\x86",  NULL,         defs_menu },
   { NULL }
};

static MENU edit_menu[] =
{ 
   { "&Undo\tU",        onUndo,      NULL },
   { "&Copy\tC",        onCopy,      NULL },
   { "&Paste\tV",       onPaste,     NULL },
   { "&Delete\tDel",    onDelete,    NULL },
   { "" },
   { "Delete &Map",     onDeleteMap, NULL },
   { NULL }
};

static MENU tool_menu[] =
{ 
   { "&Template\tF5",      onTemplate,  NULL },
   { "&Doors\tF6",         onDoors,     NULL },
   { "&Color set fix\tF7", onCSetFix,   NULL },
   { "&Flags\tF8",         onFlags,     NULL },
   { "" },
   { "View &Palette",      onShowPal,   NULL },
   { "&Edit Template",     onEditTemplate, NULL },
   { NULL }
};


static MENU data_menu[] =
{ 
   { "&Screen Flags\tF9",   onScrFlags,    NULL },
   { "&Tile Warp\tF10",     onWarp,        NULL },
   { "&Side Warp\tF11",     onWarp2,       NULL },
   { "&Enemy Flags\tF12",   onEnemyFlags,  NULL },
   { "&Path\tP",            onPath,        NULL },
   { "" },
   { "&Item\tI",            onItem,        NULL },
   { "&Guy\tG",             onGuy,         NULL },
   { "&Message String\tS",  onString,      NULL },
   { "&Room Type\tR",       onRType,       NULL },
   { "Catch All\tA",        onCatchall,    NULL, D_DISABLED },
   { "&Enemies\tE",         onEnemies,     NULL },
   { "&Under Combo",        onUnderCombo,  NULL },
   { NULL }
};


static MENU etc_menu[] =
{ 
   { "&Lost woods",         playTune,   NULL },
   { "&Play MIDI",          playMIDI,   NULL },
   { "&Stop tunes",         stopMIDI,   NULL },
   { "" },
   { "&Help...",            onHelp,     NULL },
   { "&About...",           onAbout,    NULL },
   { "View pic...",         onViewPic,  NULL },
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


static DIALOG dialogs[] =
{
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
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'l',     0,       0,        0,       onL },
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
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_SLASH2,0,      onDivide },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'u',     0,       0,        0,       onUndo },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'c',     0,       0,        0,       onCopy },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'v',     0,       0,        0,       onPaste },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'e',     0,       0,        0,       onEnemies },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'i',     0,       0,        0,       onItem },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    's',     0,       0,        0,       onString },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'g',     0,       0,        0,       onGuy },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'r',     0,       0,        0,       onRType },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'a',     0,       0,        0,       onCatchall },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    'p',     0,       0,        0,       onPath },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    ',',     0,       0,        0,       onDecMap },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    '.',     0,       0,        0,       onIncMap },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_DEL,  0,       onDelete },
 { NULL,              0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       NULL }
};



static DIALOG getnum_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 80,   80,   160,  64,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       112,  104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Value:" },
 { d_edit_proc,       168,  104,  48,   8,    vc(12),  vc(1),  0,       0,          6,             0,       NULL },
 { d_button_proc,     90,   120,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  120,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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


#include "zq_class.cc"



/***********************/
/*** dialog handlers ***/
/***********************/


int onUndo()
{
 Map.Uhuilai();
 refresh(rMAP+rSCRMAP+rMENU);
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
 refresh(rMAP+rSCRMAP+rMENU);
 return D_O_K;
}

int onDelete()
{
 if(Map.CurrScr()->valid&mVALID) {
  if(alert("Delete this screen?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
   Map.Ugo();
   Map.clearscr(Map.getCurrScr());
   refresh(rMAP+rSCRMAP+rMENU);
   }
  }
 return D_O_K;
}

int onDeleteMap()
{
 if(alert("Clear this entire map?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
   Map.clearmap();
   refresh(rMAP+rSCRMAP+rMENU);
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
 refresh(rMAP+rSCRMAP+rMENU);
 return D_O_K;
}

int onDecMap()
{
 int m=Map.getCurrMap();
 Map.setCurrMap((m-1<0)?header.map_count-1:min(m-1,header.map_count-1));
 refresh(rMAP+rSCRMAP+rMENU);
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
    for(int i=0; i<512; i++)
      combobuf[i]=((combo*)data[DAT_COMBO].dat)[i];
    }
  return D_O_K;
}

int onDefault_Tiles()
{
  if(alert("Reset all tiles?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    reset_tiles();
    }
  return D_O_K;
}

int onDefault_MapColors()
{
  if(alert("Reset all map colors?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 1) {
    saved=false;
    reset_mapcolors();
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

int onUp()    { Map.scroll(0); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onDown()  { Map.scroll(1); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onLeft()  { Map.scroll(2); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onRight() { Map.scroll(3); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onPgUp()  { menutype=wrap(menutype-1,0,m_menucount-1); refresh(rMENU); return D_O_K; }
int onPgDn()  { menutype=wrap(menutype+1,0,m_menucount-1); refresh(rMENU); return D_O_K; }


int onMap1() { Map.setCurrMap(0); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap2() { Map.setCurrMap(1); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap3() { Map.setCurrMap(2); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap4() { Map.setCurrMap(3); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap5() { Map.setCurrMap(4); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap6() { Map.setCurrMap(5); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap7() { Map.setCurrMap(6); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap8() { Map.setCurrMap(7); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap9() { Map.setCurrMap(8); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }
int onMap10() { Map.setCurrMap(9); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }

int onEditTemplate() { Map.setCurrMap(10); refresh(rMAP+rSCRMAP+rMENU); return D_O_K; }


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
     etc_menu[0].flags=0;
     etc_menu[1].flags=D_SELECTED;
     }
   }
  return D_O_K;
}

int playTune()
{
  if(play_midi((MIDI*)data[MID_LOSTWOODS].dat,true)==0) {
   etc_menu[0].flags=D_SELECTED;
   etc_menu[1].flags=0;
   }
  return D_O_K;
}

int stopMIDI()
{
  stop_midi();
  etc_menu[0].flags = etc_menu[1].flags = 0;
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

inline int pal_sum(RGB p) { return p.r + p.g + p.b; }

bool vp_showpal=true, vp_showsize=true, vp_center=true;

void get_bw(RGB *pal,int &black,int &white)
{
  black=white=0;
  for(int i=0; i<256; i++) {
    if(pal_sum(pal[i])<pal_sum(pal[black]))
      black=i;
    if(pal_sum(pal[i])>pal_sum(pal[white]))
      white=i;
    }
}


int onViewPic()
{
  BITMAP *bmp,*buf;
  PALETTE pal=black_palette;
  int  x=0,y=0,black,white;
  double scale=1.0;
  bool done=false, redraw=true;

  int gotit = getname("View Image (bmp,pcx,jpg)","bmp;pcx;jpg",imagepath,false);
  strcpy(imagepath,temppath);
  if(!gotit)
    return D_O_K;

  if(!strcmp(get_extension(imagepath),"jpg"))
    bmp = load_jpeg(imagepath,pal);
  else
    bmp = load_bitmap(imagepath,pal);

  if(!bmp) {
    alert("Error loading file",NULL,NULL,"OK",NULL,13,27);
    return D_O_K;
    }

  get_bw(pal,black,white);

  buf = create_bitmap(320,240);
  if(!buf) {
    alert("Error creating temp bitmap",NULL,NULL,"OK",NULL,13,27);
    destroy_bitmap(bmp);
    return D_O_K;
    }

  if(!vp_center) {
    x=bmp->w-320;
    y=bmp->h-240;
    }

  go();
  scare_mouse();
  set_palette(pal);

  do {
    if(redraw) {
      clear(buf);
      stretch_blit(bmp,buf,0,0,bmp->w,bmp->h,
        int(320+(x-bmp->w)*scale)/2,int(240+(y-bmp->h)*scale)/2,
        int(bmp->w*scale),int(bmp->h*scale));
      if(vp_showpal)
        for(int i=0; i<256; i++)
          rectfill(buf,((i&15)<<2)+256,((i>>4)<<2)+176,((i&15)<<2)+259,((i>>4)<<2)+179,i);
      if(vp_showsize) {
        text_mode(black);
        textprintf(buf,font,0,232,white,"%dx%d %1.2f%%",bmp->w,bmp->h,scale);
        }
      vsync();
      blit(buf,screen,0,0,0,0,320,240);
      redraw=false;
      }

    if(key[KEY_UP])    { y+=5; redraw=true; }
    if(key[KEY_DOWN])  { y-=5; redraw=true; }
    if(key[KEY_LEFT])  { x+=5; redraw=true; }
    if(key[KEY_RIGHT]) { x-=5; redraw=true; }

    if(keypressed() && !redraw)
      switch(readkey()>>8) {
      case KEY_PGUP:  if(scale>0.1) scale*=0.95; redraw=true; break;
      case KEY_PGDN:  if(scale<5.0) scale/=0.95; redraw=true; break;
      case KEY_TILDE: scale=0.5; redraw=true; break;
      case KEY_Z:     x=bmp->w-320; y=bmp->h-240; vp_center=false; // cont...
      case KEY_1:     scale=1.0; redraw=true; break;
      case KEY_2:     scale=2.0; redraw=true; break;
      case KEY_3:     scale=3.0; redraw=true; break;
      case KEY_4:     scale=4.0; redraw=true; break;
      case KEY_5:     scale=5.0; redraw=true; break;
      case KEY_C:     x=y=0; redraw=vp_center=true; break;
      case KEY_S:     vp_showsize = !vp_showsize; redraw=true; break;
      case KEY_P:     vp_showpal = !vp_showpal; redraw=true; break;
      case KEY_ESC:
      case KEY_SPACE: done=true; break;
      }
  } while(!done);

  destroy_bitmap(bmp);
  destroy_bitmap(buf);
  set_palette(RAMpal);
  unscare_mouse();
  comeback();
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
 if(flags&rCLEAR)
  clear_to_color(menu,vc(0));

 if(flags&rMAP) {
  Map.setCurrScr(Map.getCurrScr());  // to update palette
  Map.draw(menu,0,16,Flags);
  }

 if(flags&rSCRMAP) {
  text_mode(vc(0));
  if(Map.getCurrMap()<MAXMAPS) {
    for(int i=0; i<130; i++) {
     rectfill(menu,(i&15)*4,(i/16)*4+204,(i&15)*4+3,(i/16)*4+207,
       Map.Scr(i)->valid&mVALID ? lc1((Map.Scr(i)->color)&15) : vc(0));
     if(Map.Scr(i)->valid&mVALID&&((Map.Scr(i)->color)&15)>0)
       putpixel(menu,(i&15)*4+1,(i/16)*4+205,lc2((Map.Scr(i)->color)&15));
     }
    int s=Map.getCurrScr();
    rect(menu,(s&15)*4,(s/16)*4+204,(s&15)*4+3,(s/16)*4+207,vc(15));
    textprintf(menu,font,0,196,vc(15),"M:%X S:%02X",Map.getCurrMap()+1,s);
    }
  else {
    rectfill(menu,0,204,63,239,vc(0));
    textprintf(menu,font,0,196,vc(15),"Template");
    }
  }
 if(flags&rCOMBOS) {
   for(int i=0; i<56; i++)
     putcombo(menu,(i&3)*16+256,(i/4)*16+16,(CSet<<9)+i+First,0);
   }
 if(flags&rCOMBO)
   putcombo(menu,304,0,(CSet<<9)+Combo,0);

 if(flags&rMENU) {
  rectfill(menu,64,192,255,239,dvc(9));
  rect(menu,64,192,255,239,vc(15));
  text_mode(-1);
  set_clip(menu,65,193,254,238);
  mapscr *scr=Map.CurrScr();
  textprintf(menu,font,246,195,vc(9),"%d",menutype+1);
  switch(menutype) {
   case m_block: {
    char name[13];
    extract_name(filepath,name,FILENAME8__);
    textprintf(menu,font,72,200,vc(7),"File:");
    textprintf(menu,font,120,200,vc(11),"%s",name);
    textprintf(menu,font,72,216,vc(7),"Combo Type:");
    textprintf(menu,font,72,224,vc(11),"%s",combotype_string[combobuf[Combo].type]);
    putcombo(menu,192,200,(CSet<<9)+Combo,cWALK);
    putcombo(menu,216,200,(CSet<<9)+Combo,0);
    textprintf(menu,font,208,224,vc(11),"%3d",Combo);
    textprintf(menu,font,192,224,vc(11),"%d",CSet);
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
    textprintf(menu,font,72,198,vc(7),"       TGSOMLWRDIB");
    textprintf(menu,font,72,206,vc(7),"Flags:");
    textprintf(menu,font,128,206,vc(11),"%d%d%d%d%d%d%d%d%d%d%d",
      bit(wf,6),bit(wf,5),bit(wf,4),bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0));

    f=scr->enemyflags;
    textprintf(menu,font,72,218,vc(7),"       BILFR24Z P:");
    textprintf(menu,font,72,226,vc(7),"Enemy:");
    textprintf(menu,font,128,226,vc(11),"%d%d%d%d%d%d%d%d %s",
      bit(f,7),bit(f,6),bit(f,5),bit(f,4),bit(f,3),bit(f,2),bit(f,1),bit(f,0),
      Map.CurrScr()->pattern == 0 ? "random" : "sides");
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
   }
  set_clip(menu,0,0,319,239);
  }
 vsync();
 scare_mouse();
 if(flags&rCLEAR)
   blit(menu,screen,0,0,0,0,320,240);
 else {
   blit(menu,screen,0,16,0,16,320,224);
   if(flags&rCOMBO)
     blit(menu,screen,304,0,304,0,16,16);
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
   if(s>=130)
     s-=16;
   if(s!=Map.getCurrScr()) {
     Map.setCurrScr(s);
     refresh(rMAP+rSCRMAP+rMENU);
     }
   }
}


void select_combo()
{
  while(mouse_b) {
   int x=mouse_x; if(x<256) x=256;
   int y=mouse_y; if(y<16) y=16;
   Combo=(((y-16)>>4)<<2)+((x-256)>>4)+First;
   refresh(rMENU+rCOMBO);
   }
}


void draw()
{
 saved=false;
 Map.Ugo();
 if(!(Map.CurrScr()->valid&mVALID)) {
   Map.CurrScr()->valid|=mVALID;
   Map.setcolor(Color);
   }
 refresh(rMAP+rSCRMAP);
 while(mouse_b) {
  int x=mouse_x;
  int y=mouse_y;
  if(isinRect(x,y,0,16,255,191)) {
   word dat = Map.CurrScr()->data[((y-16)&0xF0)+(x>>4)];
   dat &= 0xF000;
   dat |= (CSet<<9)+(Combo&0x1FF);
   Map.CurrScr()->data[((y-16)&0xF0)+(x>>4)]=dat;
   refresh(rMAP);
   }
  }
}


int d_wflag_proc(int msg,DIALOG *d,int c)
{
 switch(msg) {
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
    Map.CurrScr()->data[(x>>4)+((y-16)&0xF0)]&=0x0FFF;
    Map.CurrScr()->data[(x>>4)+((y-16)&0xF0)]|=Flag<<12;
    refresh(rMAP);
    }
  if(keypressed()) {
    switch(readkey()>>8) {
     case KEY_ESC:
     case KEY_ENTER:    goto finished;
     case KEY_ASTERISK: onTimes();  break;
     case KEY_SLASH2:   onDivide(); break;
     case KEY_UP:       onUp();     break;
     case KEY_DOWN:     onDown();   break;
     case KEY_LEFT:     onLeft();   break;
     case KEY_RIGHT:    onRight();  break;
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
  char buf1[40],buf2[40],buf3[40];
  sprintf(buf1,"Pos:  %-3d  Combo: %-3d",c,Map.CurrScr()->data[c]&0x1FF);
  sprintf(buf2,"CSet: %-3d  Flags: %-3d",(Map.CurrScr()->data[c]&0xE00)>>9,Map.CurrScr()->data[c]>>12);
  sprintf(buf3,"Combo type: %-9s",combotype_string[combobuf[Map.CurrScr()->data[c]&0x1FF].type]);
  alert(buf1,buf2,buf3,"OK",NULL,13,27);
}


int scrolldelay=0;

void domouse()
{
 int x=mouse_x;
 int y=mouse_y;
 int redraw=0;
 scrolldelay++;
 if((x>=256) && (key[KEY_LSHIFT] || key[KEY_RSHIFT] || (scrolldelay&3)==0) ) {
   if(x>=300 && y<16 && First) {
     First-=4;
     redraw|=rCOMBOS;
     }
   if(y==239 && First<512-56) {
     First+=4;
     redraw|=rCOMBOS;
     }
   }
 if(mouse_b&1) {
   if(isinRect(x,y,0,16,255,191))
     draw();
   if(isinRect(x,y,0,204,63,239))
     select_scr();
   if(menutype==m_coords) {
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
     }
   if(x>=256 && y>=16)
     select_combo();
   }
 if(mouse_b&2) {
   if(isinRect(x,y,0,16,255,191))
     peek(((y&0xF0)-16)+(x>>4));
   if(x>=256 && y>=16) {
     select_combo();
     if(mouse_x>=256 && mouse_y>=16) {
       edit_combo(((mouse_y&0xF0)>>2)-4+((mouse_x-256)>>4)+First,true);
       refresh(rALL);
       }
     }
   }
 if(redraw)
  refresh(redraw);
 else
  vsync();
}


static DIALOG showpal_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 28,   72,   264,  96,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_bitmap_proc,     32,   76,   256,  64,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     130,  144,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "OK" },
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
 { d_ctext_proc,      160,  88,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "CSet Fix" },
 { d_radio_proc,      104,  108,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Full Screen  " },
 { d_radio_proc,      104,  128,  64,   8,    vc(14),  vc(1),  0,       D_SELECTED, 0,             0,       "Dungeon Floor" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
        Map.CurrScr()->data[(y<<4)+x] &= 0xF1FF;
        Map.CurrScr()->data[(y<<4)+x] += CSet<<9;
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
 { d_ctext_proc,      160,  88,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Dungeon Template" },
 { d_radio_proc,      104,  108,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Floor:" },
 { d_radio_proc,      104,  128,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "No Floor" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


int onTemplate()
{
 if(Map.getCurrMap()>=MAXMAPS)
   return D_O_K;
 BITMAP *floor_bmp = create_bitmap(16,16);
 if(!floor_bmp) return D_O_K;
 putcombo(floor_bmp,0,0,(CSet<<9)+Combo);
 template_dlg[1].dp=floor_bmp;
 template_dlg[3].flags=D_SELECTED;
 template_dlg[4].flags=0;
 if(popup_dialog(template_dlg,-1)==5) {
   saved=false;
   Map.Ugo();
   Map.Template((template_dlg[3].flags==D_SELECTED) ? (CSet<<9)+Combo : -1);
   refresh(rMAP+rSCRMAP);
   }
 destroy_bitmap(floor_bmp);
 return D_O_K;
}


static DIALOG under_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   60,   176,  120,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  68,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Under Combo TM" },
 { d_bitmap_proc,     114,  94,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_bitmap_proc,     194,  94,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     90,   124,  60,   16,   vc(14),  vc(1),  's',     D_EXIT,     0,             0,       "&Set" },
 { d_button_proc,     170,  124,  60,   16,   vc(14),  vc(1),  'c',     D_EXIT,     0,             0,       "&Cancel" },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  'a',     D_EXIT,     0,             0,       "Set &All" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
 { d_text_proc,       110,  84,   128,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "New     Current" },
 { NULL }
};


int onUnderCombo()
{
 BITMAP *cur = create_bitmap(16,16);
 BITMAP *sel = create_bitmap(16,16);
 if(!cur || !sel)
   return D_O_K;

 putcombo(cur,0,0,Map.CurrScr()->under);
 putcombo(sel,0,0,(CSet<<9)+Combo);

 under_dlg[2].dp = sel;
 under_dlg[3].dp = cur;
 int ret = popup_dialog(under_dlg,-1);
 if(ret==4) {
   saved=false;
   Map.CurrScr()->under = (CSet<<9)+Combo;
   }
 if(ret==6 && alert("Set all under combos","on this map?",NULL,"&Yes","&No",'y','n')==1) {
   saved=false;
   for(int i=0; i<128; i++)
     Map.Scr(i)->under = (CSet<<9)+Combo;
   }

 destroy_bitmap(sel);
 destroy_bitmap(cur);
 return D_O_K;
}




static DIALOG list_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 60,   40,   200,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_list_proc,       72,   60,   176,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


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
 list_dlg[3].dp="OK";
 list_dlg[4].dp="Cancel";
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
 { d_ctext_proc,      128,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Select Door" },
 { d_button_proc,     118,  74,   20,   16,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       " " },
 { d_button_proc,     118,  110,  20,   16,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       " " },
 { d_button_proc,     82,   90,   16,   20,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       " " },
 { d_button_proc,     158,  90,   16,   20,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       " " },
 { d_button_proc,     150,  132,  48,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Done" },
 { NULL }
};


int onDoors()
{
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
 { d_shadow_box_proc, 72,   48,   176,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Screen Flags" },
// 2
 { d_check_proc,      82,   72,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Block->Shutters   " },
 { d_check_proc,      82,   80,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Enemies->Item     " },
 { d_check_proc,      82,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Dark room         " },
 { d_check_proc,      82,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Boss roar SFX     " },
 { d_check_proc,      82,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Whistle->Stairs   " },
 { d_check_proc,      82,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Use ladder        " },
 { d_check_proc,      82,   120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Use maze path     " },
 { d_check_proc,      82,   128,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Ocean surf SFX    " },
 { d_check_proc,      82,   136,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Secret SFX        " },
 { d_check_proc,      82,   144,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Roar=Gasp style   " },
 { d_check_proc,      82,   144,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Floating Traps    " },
// 13
 { d_button_proc,     90,   156,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  156,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


int onScrFlags()
{
 byte f=Map.CurrScr()->flags;
 for(int i=0; i<=7; i++) {
   scrflags_dlg[i+2].flags = f&1?D_SELECTED:0;
   f>>=1;
   }
 scrflags_dlg[10].flags = Map.CurrScr()->flags2&16?D_SELECTED:0;
 scrflags_dlg[11].flags = Map.CurrScr()->flags2&32?D_SELECTED:0;
 scrflags_dlg[12].flags = Map.CurrScr()->flags2&64?D_SELECTED:0;

 if(popup_dialog(scrflags_dlg,-1)==13) {
   f=0;
   for(int i=7; i>=0; i--) {
     f<<=1;
     f |= scrflags_dlg[i+2].flags ==D_SELECTED ? 1:0;
     }
   Map.CurrScr()->flags=f;
   Map.CurrScr()->flags2 &= 0xCF;
   Map.CurrScr()->flags2 |= scrflags_dlg[10].flags?16:0;
   Map.CurrScr()->flags2 |= scrflags_dlg[11].flags?32:0;
   Map.CurrScr()->flags2 |= scrflags_dlg[12].flags?64:0;
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
 if(index>=0) {
   if(index>=MAXITEMS)
    index=MAXITEMS-1;
   return item_string[index];
   }
 *list_size=MAXITEMS;
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
   if(index>=MAXGUYS)
    index=MAXGUYS-1;
   return guy_string[index];
   }
 *list_size=MAXGUYS;
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


char *midilist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,MAXMIDIS_ZQ-1);
   return midi_string[index];
   }
 *list_size=MAXMIDIS_ZQ;
 return NULL;
}


int onMapCount()
{
 int ret=select_data("Number of Maps",header.map_count-1,maplist);
 if(ret>=0) {
   saved=false;
   setMapCount(ret+1);
   }
 refresh(rMAP+rSCRMAP+rMENU);
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
 int ret=select_data("Select Item",Map.CurrScr()->item,itemslist);
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
  case 1: Map.CurrScr()->catchall=bound(c,0,MAXITEMS-1); break;
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
 { d_ctext_proc,      160,  75,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "String Editor" },
 { d_box_proc,        62,   98,   195,  27,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_msg_edit_proc,   64,   100,  192,  24,   vc(12),  vc(1),  0,       0,          0,             0,       msgbuf },
 { d_button_proc,     90,   140,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  140,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_text_proc,       64,   90,   192,  8,    vc(9),   vc(1),  0,       0,          0,             0,       "123456789  ++  987654321" },
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
 { d_ctext_proc,      160,  47,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Edit Message String" },
 { d_list_proc,       80,   60,   160,  92,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       msgslist },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "Edit" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       strlist_del },
 { NULL }
};

int strlist_del()
{
 if(strlist_dlg[2].d1>0 && strlist_dlg[2].d1<msg_count-1)
   return D_CLOSE;
 return D_O_K;
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
      if(alert("Delete this message string?",buf,NULL,"Yes","No",'y',27)==1) {
        saved=false;
        for(int i=index; i<MAXMSGS-1; i++)
          MsgStrings[i]=MsgStrings[i+1];
        reset_msgstr(MAXMSGS-1);
        msg_count--;
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
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Triforce Pieces" },
 { d_tri_frame_proc,  64,   56,   192,  96,   vc(11),  vc(1),  0,       0,          0,             0,       NULL },
// 3
 { d_tri_edit_proc,   138,  82,   16,   16,   vc(14),  vc(1),  0,       0,          0,             0,       "1" },
 { d_tri_edit_proc,   166,  82,   16,   16,   vc(14),  vc(1),  0,       0,          0,             1,       "2" },
 { d_tri_edit_proc,   90,   130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             2,       "3" },
 { d_tri_edit_proc,   214,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             3,       "4" },
// 7
 { d_tri_edit_proc,   138,  110,  16,   16,   vc(14),  vc(1),  0,       0,          0,             4,       "5" },
 { d_tri_edit_proc,   118,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             5,       "6" },
 { d_tri_edit_proc,   166,  110,  16,   16,   vc(14),  vc(1),  0,       0,          0,             6,       "7" },
 { d_tri_edit_proc,   186,  130,  16,   16,   vc(14),  vc(1),  0,       0,          0,             7,       "8" },
// 11
 { d_button_proc,     90,   166,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  166,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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

 switch(DMaps[dmap].type)
 {
 case dmDNGN:
  for(int y=1; y<33; y+=4)
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],misc.colors.dngn_fg,misc.colors.dngn_bg,0);
  c=DMaps[dmap].compass;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,dvc(8));
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmOVERW:
  clear_to_color(dmapbmp,misc.colors.overw);
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*4+1,(c>>4)*4+1,(c&15)*4+3,(c>>4)*4+3,vc(10));
  break;

 case dmCAVE:
  for(int y=1; y<33; y+=4)
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],misc.colors.cave_fg,misc.colors.cave_bg,0);
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmBSDNGN:
  for(int y=1; y<33; y+=4)
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],misc.colors.bs_dk,
             misc.colors.bs_lt,misc.colors.bs_lt);
  c=DMaps[dmap].compass;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,dvc(8));
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmBSOVERW:
  for(int y=1; y<33; y+=4)
    drawgrid_s(dmapbmp,1,y,DMaps[dmap].grid[y>>2],misc.colors.bs_goal,
               misc.colors.bs_dk,misc.colors.bs_lt);
  c=DMaps[dmap].cont;
  rectfill(dmapbmp,(c&15)*8+3,(c>>4)*4+1,(c&15)*8+5,(c>>4)*4+3,vc(10));
  break;

 case dmBSCAVE:
  for(int y=1; y<33; y+=4)
    drawgrid(dmapbmp,1,y,DMaps[dmap].grid[y>>2],misc.colors.bs_dk,
             misc.colors.bs_lt,misc.colors.bs_lt);
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
     textprintf(screen,font,xy[2],xy[3],vc(11),"Map: %d",DMaps[d->d1].map+1);
   if(xy[4]||xy[5])
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level: %X",DMaps[d->d1].level);
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
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level: %X",DMaps[d->d1].level);
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
       mapscr *scr = TheMaps + map*130 + y*16+x+xoff;
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


int xmapspecs[4] = {0,0,84,118};


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


char *dmaptype_str[dmMAX] = { "Dngn","Overw","Cave","BS-D","BS-Ow","BS-C" };

char *typelist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,dmMAX-1);
   return dmaptype_str[index];
   }
 *list_size=dmMAX;
 return NULL;
}



static DIALOG editdmap_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   32,   212,  160,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "DMap Editor" },
 { d_text_proc,       72,   56,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Map:" },
 { d_text_proc,       72,   72,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Midi:" },
 { d_text_proc,       72,   88,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Level:" },
 { d_text_proc,       176,  88,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Color:" },
 { d_text_proc,       72,   96,   72,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Compass:  0x" },
// 7
 { d_xmaplist_proc,   112,  54,   40,   11,   vc(14),  vc(1),  0,       0,          1,             0,       maplist, NULL, xmapspecs},
 { d_droplist_proc,   112,  70,   144,  11,   vc(14),  vc(1),  0,       0,          1,             0,       midilist },
 { d_hexedit_proc,    128,  88,   16,   8,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    232,  88,   16,   8,    vc(12),  vc(1),  0,       0,          1,             0,       NULL },
 { d_hexedit_proc,    168,  96,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 12
 { d_grid_proc,       173,  119,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  123,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  127,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  131,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  135,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  139,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  143,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
 { d_grid_proc,       173,  147,  64,   4,    0,       0,      0,       0,          0,             0,       NULL },
// 20
 { d_button_proc,     90,   168,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  168,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_slider_proc,     84,   152,  64,   8,    vc(14),  vc(1),  0,       0,          22,            0,       NULL, onXslider },
// 23
 { d_text_proc,       72,   104,  72,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Continue: 0x" },
 { d_hexedit_proc,    168,  104,  72,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_text_proc,       160,  56,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Type" },
 { d_droplist_proc,   192,  54,   64,   11,   vc(14),  vc(1),  0,       0,          1,             0,       typelist },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};



void editdmap(int index)
{
 char levelstr[4],colorstr[4],compassstr[4],contstr[4];
 sprintf(levelstr,"%X",DMaps[index].level);
 sprintf(colorstr,"%X",DMaps[index].color);
 sprintf(compassstr,"%02X",DMaps[index].compass);
 sprintf(contstr,"%02X",DMaps[index].cont);

 editdmap_dlg[7].d1=DMaps[index].map;
 editdmap_dlg[8].d1=DMaps[index].midi;
 editdmap_dlg[26].d1=DMaps[index].type;
 editdmap_dlg[9].dp=levelstr;
 editdmap_dlg[10].dp=colorstr;
 editdmap_dlg[11].dp=compassstr;
 editdmap_dlg[24].dp=contstr;
 xmapspecs[1]=DMaps[index].xoff;
 editdmap_dlg[22].d2=DMaps[index].xoff+7;

 for(int i=0; i<8; i++)
   editdmap_dlg[i+12].d1=DMaps[index].grid[i];

 if(popup_dialog(editdmap_dlg,-1)==20) {
   saved=false;
   for(int i=0; i<8; i++)
     DMaps[index].grid[i] = editdmap_dlg[i+12].d1;
   DMaps[index].map=editdmap_dlg[7].d1;
   DMaps[index].midi=editdmap_dlg[8].d1;
   DMaps[index].type=editdmap_dlg[26].d1;
   DMaps[index].level=xtoi(levelstr);
   DMaps[index].color=xtoi(colorstr);
   DMaps[index].compass=xtoi(compassstr);
   DMaps[index].cont=xtoi(contstr);
   DMaps[index].xoff=xmapspecs[1];
   }
}



int selectdmapxy[6] = {168,108,168,80,168,90};

static DIALOG selectdmap_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   56,   176,  120,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  63,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Select DMap" },
 { d_dmaplist_proc,   90,   80,   64,   60,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       numberlist, NULL, selectdmapxy },
 { d_button_proc,     90,   152,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "Edit" },
 { d_button_proc,     170,  152,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Done" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,             KEY_DEL, close_dlg },
 { NULL }
};



int onDmaps()
{
 int ret;
 char buf[40];
 number_list_size=MAXDMAPS;
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
 { d_shadow_box_proc, 36,   48,   248,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "MIDI Specs" },
// 2
 { d_text_proc,       56,   94,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "MIDI:" },
 { d_text_proc,       104,  94,   48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   114,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Name:" },
 { d_edit_proc,       104,  114,  160,  8,    vc(12),  vc(1),  0,       0,          16,            0,       NULL },
 { d_text_proc,       56,   124,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Volume:" },
 { d_edit_proc,       120,  124,  32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
// 8
 { d_check_proc,      176,  124,  80,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Loop:   " },
// 9
 { d_button_proc,     50,   72,   56,   16,   vc(14),  vc(1),  'l',     D_EXIT,     0,             0,       "&Load" },
 { d_button_proc,     116,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       "#" },
 { d_button_proc,     156,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       ">" },
 { d_button_proc,     196,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       ">>" },
 { d_button_proc,     236,  72,   32,   16,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       ">>>" },
// 14
 { d_text_proc,       56,   134,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Start:" },
 { d_edit_proc,       112,  134,  32,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
 { d_text_proc,       176,  134,  56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "L.Strt:" },
 { d_edit_proc,       240,  134,  40,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
 { d_text_proc,       176,  144,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "L.End:" },
 { d_edit_proc,       240,  144,  40,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
// 20
 { d_text_proc,       176,  94,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Pos:" },
 { d_text_proc,       216,  94,   32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       176,  104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Len:" },
 { d_text_proc,       216,  104,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Time:" },
 { d_text_proc,       104,  104,  32,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
// 26
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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

    editmidi_dlg[3].dp = midi?"Loaded":"Empty";
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
          for( ; i<16 && t[i]!=0 && t[i]!='.'; i++)
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
   textprintf(screen,font,x+48,y+8,vc(11),"%-16s",customMIDIs[i].title);
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
 { d_shadow_box_proc, 36,   48,   248,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Select MIDI" },
 { d_midilist_proc,   48,   76,   44,   68,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       numberlist },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "Edit" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Done" },
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


int warpdmapxy[6] = {188,114,188,88,188,100};


static DIALOG warp_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 45,   48,   230,  136,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  55,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       56,   70,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Type:" },
 { d_text_proc,       56,   88,   40,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "DMap:" },
 { d_text_proc,       56,   106,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Screen: 0x" },
 { d_text_proc,       56,   120,  64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Triggers:" },
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
 { d_button_proc,     70,   160,  40,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     130,  160,  40,   16,   vc(14),  vc(1),  'g',     D_EXIT,     0,             0,       "&Go" },
 { d_button_proc,     190,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};



int onWarp()
{
 warp_dlg[1].dp="Tile Warp";
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
 number_list_size=MAXDMAPS;
 number_list_zero=true;
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
 warp_dlg[1].dp="Side Warp";
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
 number_list_size=MAXDMAPS;
 number_list_zero=true;
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
 number_list_size=MAXDMAPS;
 number_list_zero=true;
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
 { d_ctext_proc,      160,  71,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Maze Path" },
 { d_text_proc,       94,   86,   192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "1st" },
 { d_text_proc,       94,   100,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "2nd" },
 { d_text_proc,       94,   114,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "3rd" },
 { d_text_proc,       94,   128,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "4th" },
 { d_text_proc,       94,   142,  192,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Exit" },
 { d_droplist_proc,   140,  84,   80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  98,   80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  112,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  126,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_droplist_proc,   140,  140,  80,   11,   vc(14),  vc(1),  0,       0,          0,             0,       dirlist },
 { d_button_proc,     90,   156,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  156,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
 { d_text_proc,       72,   64,   48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       "1st" },
 { d_text_proc,       72,   96,   48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       "2nd" },
 { d_text_proc,       72,   128,  48,   8,    vc(7),   vc(1),  0,       0,          0,             0,       "3rd" },
 { d_text_proc,       104,  64,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Price:" },
 { d_text_proc,       104,  96,   88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Price:" },
 { d_text_proc,       104,  128,  88,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Price:" },
// 8
 { d_edit_proc,       160,  64,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  73,   136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_edit_proc,       160,  96,   32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  105,  136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_edit_proc,       160,  128,  32,   8,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
 { d_nlist_proc,      104,  137,  136,  11,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
// 14
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
 number_list_size=MAXDMAPS;
 number_list_zero=true;
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
     textprintf(screen,font,xy[4],xy[5],vc(11),"Level: %X",DMaps[dmap].level);
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
 { d_text_proc,       80,   64,   48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Count:" },
 { d_wclist_proc,     136,  62,   48,   11,   vc(14),  vc(1),  0,       0,          1,             0,       wclist },
// 4
 { d_warplist_proc,   80,   80,   64,   67,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       numberlist, NULL, warpringdmapxy },
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "Edit" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Done" },
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
 { d_ctext_proc,      160,  72,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Enemy Pattern" },
 { d_list_proc,       80,   88,   160,  28,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       pattern_list },
// 3
 { d_button_proc,     90,   128,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  128,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Enemy Flags" },
// 2
 { d_check_proc,      82,   72,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Zora              " },
 { d_check_proc,      82,   80,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Corner traps      " },
 { d_check_proc,      82,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Center traps      " },
 { d_check_proc,      82,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Falling rocks     " },
 { d_check_proc,      82,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Statues shoot fire" },
 { d_check_proc,      82,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Has ring leader   " },
 { d_check_proc,      82,   120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Enemy carries item" },
 { d_check_proc,      82,   128,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Dungeon Boss      " },
// 10
 { d_button_proc,     90,   144,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  144,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
 return guy>=10 ? guy_string[guy] : "(none)";
}



typedef struct enemy_struct {
  char *s;
  int i;
} enemy_struct;

enemy_struct bie[eMAXGUYS];
enemy_struct ce[100];
int enemy_type=0,bie_cnt=-1,ce_cnt;


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
 { d_button_proc,     90,   160,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  160,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
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
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Enemies" },
 { d_list_proc,       64,   56,   192,  84,   vc(14),  vc(1),  0,       D_EXIT,     0,             0,       enemy_viewer },
 { d_button_proc,     52,   152,  128,  16,   vc(14),  vc(1),  'e',     D_EXIT,     0,             0,       "Paste &Enemies" },
 { d_text_proc,       188,  156,  64,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_button_proc,     52,   178,  64,   16,   vc(14),  vc(1),  'f',     D_EXIT,     0,             0,       "&Flags" },
 { d_button_proc,     132,  178,  80,   16,   vc(14),  vc(1),  'p',     D_EXIT,     0,             0,       "&Pattern" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'c',     0,          0,             0,       close_dlg },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'v',     0,          0,             0,       close_dlg },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       close_dlg },
// 10
 { d_button_proc,     228,  178,  40,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
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
 { d_shadow_box_proc, 64,   32,   192,  176,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  40,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Quest Header" },
 { d_text_proc,       76,   56,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Zelda Version:" },
 { d_text_proc,       192,  56,   96,   8,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       76,   64,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Password:" },
 { d_edit_proc,       152,  64,   96,   8,    vc(12),  vc(1),  0,       0,          24,            0,       password },
 { d_text_proc,       76,   72,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Quest Number:" },
 { d_edit_proc,       184,  72,   32,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_text_proc,       76,   80,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Quest Version:" },
 { d_edit_proc,       192,  80,   56,   8,    vc(12),  vc(1),  0,       0,          8,             0,       NULL },
// 10
 { d_text_proc,       76,   96,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Title:" },
 { d_showedit_proc,   128,  96,   104,  8,    vc(12),  vc(1),  0,       0,          64,            0,       title },
 { d_textbox_proc,    88,   105,  144,  24,   vc(11),  vc(1),  0,       0,          64,            0,       title },
 { d_text_proc,       76,   136,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Author:" },
 { d_showedit_proc,   136,  136,  96,   8,    vc(12),  vc(1),  0,       0,          64,            0,       author },
 { d_textbox_proc,    88,   145,  144,  24,   vc(11),  vc(1),  0,       0,          64,            0,       author },
// 16
 { d_button_proc,     90,   180,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  180,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


int onHeader()
{
 char zver_str[10],q_num[8],version[10];

 sprintf(zver_str,"%d.%02X",header.zelda_version>>8,header.zelda_version&0xFF);
 sprintf(q_num,"%d",header.quest_number);
 strcpy(version,header.version);
 strcpy(author,header.author);
 strcpy(title,header.title);
 get_questpwd(password);

 header_dlg[3].dp=zver_str;
 header_dlg[7].dp=q_num;
 header_dlg[9].dp=version;

 int ret=popup_dialog(header_dlg,-1);

 if(ret==16)
 {
   saved=false;
   header.quest_number=atoi(q_num);
   strcpy(header.author,author);
   strcpy(header.title,title);
   strcpy(header.version,version);
   set_questpwd(password);
 }
 return D_O_K;
}



static DIALOG rules_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   24,   208,  184,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  32,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Quest Rules" },
// 2
 { d_check_proc,      70,   50,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Solid Blocks         " },
 { d_check_proc,      70,   60,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "TmpNoRet disabled    " },
 { d_check_proc,      70,   70,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Multi-Dir Traps      " },
// 5
 { d_button_proc,     90,   180,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  180,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};


int onRules()
{
  for(int i=0; i<qrMAX; i++)
    rules_dlg[i+2].flags = get_bit(header.rules,i) ? D_SELECTED : 0;

  int ret=popup_dialog(rules_dlg,-1);

  if(ret==5)
  {
    saved=false;
    for(int i=0; i<qrMAX; i++)
      set_bit(header.rules,i,rules_dlg[i+2].flags);
  }

  return D_O_K;
}



/************************/
/******  onColors  ******/
/************************/

#include "gfxpal.cc"

#define edc  247
#define edi  248
#define ed0  249
#define ed1  250
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


void draw_edit_dataset_specs(int index)
{
  for(int i=0; i<16; i++)
    rectfill(screen,(i<<3)+96,163,(i<<3)+103,170,14*16+i);
  text_mode(ed1);
  rectfill(screen,96,173,223,200,ed1);
  textout(screen,font,"\x82",(index<<3)+96,173,ed11);
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
  PALETTE holdpal = RAMpal;
  int index = 0;

  for(int i=240; i<256; i++)
    RAMpal[i] = ((RGB*)data[PAL_RAT].dat)[i];
  RAMpal[ed0]  = RAMpal[vc(0)];
  RAMpal[ed1]  = RAMpal[vc(1)];
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
  draw_edit_dataset_specs(index);

  draw_button(screen,240,152,60,16,"OK",ed1,ed14);
  draw_button(screen,240,184,60,16,"Cancel",ed1,ed14);
  draw_button(screen,130,120,60,16,"Insert",ed1,ed14);

  unscare_mouse();
  while(mouse_b);
  bool bdown=false;
  int doing=0;

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

    if(keypressed()) {
      switch(readkey()>>8) {
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
      }
     }

    if(mouse_b && (doing==1 || doing==3))
      colormixer(color,gray,ratio);
    else {
      vsync();
      scare_mouse();
      if(setpal)
        set_palette_range(RAMpal,14*16,15*16,false);
      draw_edit_dataset_specs(index);
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


int px=168;
int py=64;

void grab_dataset(int dataset)
{
  PALETTE pal = black_palette;
  PALETTE tmp;
  BITMAP *bmp;

  bmp = load_bitmap(imagepath,pal);
  if(!bmp) {
    alert("Error loading file",NULL,NULL,"OK",NULL,13,27);
    clear_to_color(screen,vc(0));
    return;
    }

  char fname[13];
  extract_name(imagepath,fname,FILENAME8_3);

  int black,white,row=0;
  get_bw(pal,black,white);

  blit(mouse_bmp[0],mouse_bmp[MAXMICE-1],0,0,0,0,16,16);
  for(int y=0; y<16; y++)
   for(int x=0; x<16; x++)
    if(getpixel(mouse_bmp[MAXMICE-1],x,y)!=0)
     putpixel(mouse_bmp[MAXMICE-1],x,y,white);

  scare_mouse();
  clear(screen);
  vsync();
  set_palette(pal);
  clear_to_color(screen,black);
  blit(bmp,screen,0,0,0,0,bmp->w,bmp->h);
  text_mode(black);
  textout(screen,font,fname,0,232,white);
  set_mouse_sprite(mouse_bmp[MAXMICE-1]);
  draw_button(screen,180,224,60,16,"OK",black,white);
  draw_button(screen,260,224,60,16,"Cancel",black,white);
  unscare_mouse();

  int done=0;
  int f=0;
  do {
    int x=mouse_x;
    int y=mouse_y;

    if((mouse_b&1) && isinRect(x,y,px,py,px+127,py+127)) {
      row=((y-py)>>3);
      }

    if((mouse_b&1) && isinRect(x,y,180,224,239,239))
      if(do_button(180,224,60,16,"OK",black,white))
        done=2;

    if((mouse_b&1) && isinRect(x,y,260,224,319,239))
      if(do_button(260,224,60,16,"Cancel",black,white))
        done=1;

    if(keypressed()) {
      switch(readkey()>>8) {
      case KEY_ESC:   done=1; break;
      case KEY_ENTER: done=2; break;
      case KEY_SPACE:
        vsync();
        scare_mouse();
        clear_to_color(screen,black);
        blit(bmp,screen,0,0,0,0,bmp->w,bmp->h);
        text_mode(black);
        textout(screen,font,fname,0,232,white);
        set_mouse_sprite(mouse_bmp[MAXMICE-1]);
        draw_button(screen,180,224,60,16,"OK",black,white);
        draw_button(screen,260,224,60,16,"Cancel",black,white);
        unscare_mouse();
        px = 192-px;
        break;
      case KEY_TAB:
        tmp = pal;
        for(int i=0; i<16; i++)
          tmp[(row<<4)+i] = invRGB(tmp[(row<<4)+i]);
        for(int i=0; i<12; i++) {
          vsync();
          if(i&2)
            set_palette(pal);
          else
            set_palette(tmp);
          }
        break;
      }
    }
    vsync();
    scare_mouse();
    for(int i=0; i<256; i++) {
      int x=((i&15)<<3)+px;
      int y=((i>>4)<<3)+py;
      rectfill(screen,x,y,x+7,y+7,i);
      }
    f++;
    rect(screen,px-1,py-1,px+128,py+128,white);
    rect(screen,px-1,(row<<3)+py-1,px+128,(row<<3)+py+8,(f&2)?white:black);
    unscare_mouse();
    SCRFIX();
  } while(!done);

  if(done==2)
    get_cset(dataset,row,pal);

  while(mouse_b);
  scare_mouse();
  set_mouse_sprite(mouse_bmp[0]);
  unscare_mouse();
  destroy_bitmap(bmp);
}




byte cset_hold[16*3];
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

  PALETTE tmp = black_palette;

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


void draw_cset_proc(DIALOG *d)
{
  d_bitmap_proc(MSG_DRAW,d,0);
  text_mode(d->bg);
  rect(screen,d->x,(d->d2<<3)+d->y,d->x+d->w-1,(d->d2<<3)+d->y+7,FLASH);
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
  case MSG_WANTFOCUS:
    return D_WANTFOCUS;
  case MSG_DRAW: draw_cset_proc(d); break;
  case MSG_CLICK: {
    int fc=0;
    do {
      int x=mouse_x;
      int y=mouse_y;
      if(isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
        d->d2 = (y-d->y)>>3;
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
    int k=c>>8;
    switch(k) {
     case KEY_UP:    if(d->d2>0) d->d2--;             break;
     case KEY_DOWN:  if(d->d2<((d->h)>>3)-1) d->d2++; break;
     case KEY_PGUP:  d->d2=0;                         break;
     case KEY_PGDN:  d->d2=((d->h)>>3)-1;             break;
     case KEY_C:
       for(int i=0; i<16*3; i++)
         cset_hold[i] = *(((byte*)d->dp2)+CSET(d->d2)*3+i);
       cset_ready=true;
       break;
     case KEY_V:
       undopal=pal;
       if(cset_ready) {
         for(int i=0; i<16*3; i++)
           *(((byte*)d->dp2)+CSET(d->d2)*3+i) = cset_hold[i];
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

byte mainpal_csets[30]   = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
byte levelpal_csets[26]  = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
byte spritepal_csets[30] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
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

 { d_text_proc,       104,  39,   128,  8,    rc[9],   rc[1],  0,       0,          0,             0,       "0123456789ABCDEF" },
// 19
 { d_button_proc,     60,   178,  60,   16,   rc[14],  rc[1],  'e',     D_EXIT,     0,             0,       "&Edit" },
 { d_button_proc,     130,  178,  60,   16,   rc[14],  rc[1],  'g',     D_EXIT,     0,             0,       "&Grab" },
 { d_button_proc,     200,  178,  60,   16,   rc[14],  rc[1],  'f',     D_EXIT,     0,             0,       "&File" },
 { d_button_proc,     244,  126,  24,   16,   rc[14],  rc[1],  'd',     D_EXIT,     0,             0,       "&D" },
 { d_button_proc,     244,  152,  24,   16,   rc[14],  rc[1],  'u',     D_EXIT,     0,             0,       "&U" },
// 24
 { d_button_proc,     60,   204,  130,  16,   rc[14],  rc[1],  'l',     D_EXIT,     0,             0,       "&Load to CS 9" },
 { d_button_proc,     200,  204,  60,   16,   rc[14],  rc[1],  27,      D_EXIT,     0,             0,       "Done" },
 { NULL }
};


int EditColors(char *caption,int first,int count,byte *label)
{
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
  colors_dlg[2].d2 = 0;
  colors_dlg[2].dp2 = colordata+CSET(first)*3;
  colors_dlg[2].dp3 = pal;
  colors_dlg[22].proc = (count==pdLEVEL) ? d_button_proc : d_dummy_proc;

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

    colors_dlg[20].flags = D_DISABLED;
    struct ffblk f;
    if(findfirst(imagepath,&f,0)==0 && filetype(imagepath)==ftBMP)
      colors_dlg[20].flags = D_EXIT;

    DIALOG_PLAYER *p = init_dialog(colors_dlg,2);
    int fc=0;
    while(update_dialog(p)) {
      vsync();
      pal[FLASH]=pal[rc[(fc++)&15]];
      set_palette_range(pal,FLASH,FLASH,false);
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
    if(ret==21) {
      if(getname_nogo("Image Files (bmp,pcx)","bmp;pcx",imagepath,true))
        strcpy(imagepath,temppath);
      }
    if(ret==22)
      calc_dark(first);
    if(ret==23)
      undo_pal();
  } while(ret<24);

  while(mouse_b);
  set_palette(RAMpal);
  clear_to_color(screen,vc(0));
  loadfullpal();
  loadlvlpal(Color);
  if(ret==24) {
    load_cset(RAMpal,9,first+colors_dlg[2].d2);
    set_pal();
    }
  gui_fg_color = vc(14);
  gui_bg_color = vc(1);
  gui_mg_color = vc(9);
  set_mouse_sprite(mouse_bmp[0]);
  comeback();
  return int(ret==24);
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
  if(l9) {
    load_cset(RAMpal,9,colors_dlg[2].d2);
    set_pal();
    }

  return D_O_K;
}

char *levelnumlist(int index, int *list_size)
{
 if(index>=0) {
   bound(index,0,14);
   sprintf(number_str_buf,"%X",index);
   switch(index) {
   case 0:  sprintf(number_str_buf,"0 - Overworld"); break;
   case 10: sprintf(number_str_buf,"A - Caves"); break;
   case 11: sprintf(number_str_buf,"B - Passageways"); break;
   }
   return number_str_buf;
   }
 *list_size=15;
 return NULL;
}


int onColors_Levels()
{
  int index=0;
  while((index=select_data("Select Level",index,levelnumlist,"Edit","Done"))!=-1) {
    char buf[40];
    sprintf(buf,"Level %X Palettes",index);
    int l9 = EditColors(buf,index*pdLEVEL+poLEVEL,pdLEVEL,levelpal_csets);
    setup_lcolors();
    if(index==0) {   // copy level 0 to main
      int si = CSET(poLEVEL)*3;
      int di = CSET(2)*3;
      for(int i=0; i<CSET(3)*3; i++)
        colordata[di++] = colordata[si++];

      di = CSET(9)*3;
      for(int i=0; i<16*3; i++)
        colordata[di++] = colordata[si++];
      }
    loadlvlpal(Color);
    if(l9) {
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
      EditColors("Extra Sprite Palettes 1",poSPRITE,15,spritepal_csets);
    if(index==2)
      EditColors("Extra Sprite Palettes 2",poSPRITE+15,15,spritepal2_csets);
  } while(index==1 || index==2);

  return D_O_K;
}


int onMapColors()
{

 return D_O_K;
}



/********************/
/******  Help  ******/
/********************/



static DIALOG help_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
 { d_textbox_proc,    -1,   -1,   321,  241,  vc(7),   vc(0),  0,       0,          0,        0,        NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC,  close_dlg },
 { NULL }
};




int onHelp()
{
 help_dlg[0].dp = helpbuf;

 FONT *oldfont = font;
 font = (FONT*)data[FONT_6x6].dat;

 popup_dialog(help_dlg,0);

 font=oldfont;

 return D_O_K;
}




/********************/
/******  MAIN  ******/
/********************/

int main(int argc,char **argv)
{
 set_config_file("ag.cfg");
 allegro_init();
 install_keyboard();
 install_timer();
 install_mouse();

 if(!(data=load_datafile("#"))) {
   if(!(data=load_datafile("zquest.dat"))) {
     allegro_exit();
     printf("Error loading zquest.dat\n");
     return 1;
   }
   else datafile_str="zquest.dat";
 }
 else datafile_str="#";

 resolve_password(datapwd);
 packfile_password(datapwd);

 font =(FONT*)data[FONT_GUI].dat;
 zfont=(FONT*)data[FONT_NES].dat;

 customMIDIs = (music*)malloc(sizeof(music)*MAXMIDIS);
 if(!customMIDIs || !get_qst_buffers()) {
   allegro_exit();
   printf("Sheesh! Couldn't allocate quest data buffers\n");
   return 1;
   }

 for(int i=0; i<MAXMIDIS; i++)
 {
   customMIDIs[i].midi=NULL;
   midi_string[i+4]=customMIDIs[i].title;
 }

 filepath=(char*)malloc(256);
 temppath=(char*)malloc(256);
 datapath=(char*)malloc(256);
 midipath=(char*)malloc(256);
 imagepath=(char*)malloc(256);

 if(!filepath || !datapath || !temppath || !imagepath || !midipath) {
   allegro_exit();
   printf("Sheesh! Couldn't allocate 1270 bytes!\n");
   return 1;
   }

 int helpsize = file_size("zquest.txt");
 if(helpsize==0) {
   allegro_exit();
   printf("ZQuest.txt not found\n");
   return 1;
   }

 helpbuf = (char*)malloc(helpsize+1);
 if(!helpbuf) {
   allegro_exit();
   printf("Sheesh! Couldn't allocate help buffer\n");
   return 1;
   }

 readfile("zquest.txt",helpbuf,helpsize);
 helpbuf[helpsize]=0;

 filepath[0]=temppath[0]=0;
 strcpy(datapath,get_config_string("zquest","fp_data",""));
 strcpy(midipath,get_config_string("zquest","fp_midi",""));
 strcpy(imagepath,get_config_string("zquest","fp_image",""));
 if(used_switch(argc,argv,"-d"))
 {
   resolve_password(zquestpwd);
   debug = !strcmp(zquestpwd,get_config_string("zquest","debug",""));
 }

 build_bie_list();

 if(!used_switch(argc,argv,"-s"))
   if(install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL)) {
     allegro_exit();
     printf("Allegro error: %s\n",allegro_error);
     return 1;
     }

 if(set_gfx_mode(GFX_VESA2L,320,240,0,0))
   if(set_gfx_mode(GFX_MODEX,320,240,0,0)) {
     allegro_exit();
     printf("Allegro error: %s\n",allegro_error);
     return 1;
     }

 screen2=create_bitmap(320,240);
 menu=create_bitmap(320,240);
 menu2=create_bitmap(320,240);
 combo_bmp=create_bitmap(16,16);
 dmapbmp=create_bitmap(65,33);
 if(!screen2 || !menu || !menu2 || !combo_bmp || !dmapbmp) {
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
 }


 while(!quit) {
  // update the menu status
  if(strcmp(catchall_string[Map.CurrScr()->room]," ")) {
    static char ca_menu_str[40];
    sprintf(ca_menu_str,"%s\tA",catchall_string[Map.CurrScr()->room]);
    data_menu[10].text=ca_menu_str;
    data_menu[10].flags=0;
    }
  else {
    data_menu[10].text="Catch All\tA";
    data_menu[10].flags=D_DISABLED;
    }
  for(int i=0; i<MAXMAPS; i++)
    maps_menu[i].flags=(i==Map.getCurrMap())?D_SELECTED:(i<header.map_count?0:D_DISABLED);
  edit_menu[0].flags = Map.CanUndo()?0:D_DISABLED;
  edit_menu[2].flags = Map.CanPaste()?0:D_DISABLED;
  edit_menu[1].flags = edit_menu[3].flags=(Map.CurrScr()->valid&mVALID)?0:D_DISABLED;
  tool_menu[2].flags = 0; //Map.isDungeon()?0:D_DISABLED;

  tool_menu[0].flags =
  tool_menu[1].flags =
  edit_menu[5].flags =
  import_menu[0].flags =
  export_menu[0].flags = (Map.getCurrMap()==MAXMAPS)?D_DISABLED:0;

  domouse();
  quit=!update_dialog(player);
  }

 set_palette(black_palette);
 stop_midi();
 unload_datafile(data);
 allegro_exit();
 return 0;
}



/* end */

