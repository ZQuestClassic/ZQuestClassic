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

#include "zq_misc.h"
#include "zquestdat.h"
#include "zquest.h"
#include "colors.h"
#include "qst.h"
#include "zsys.h"
#include <string.h>
#include <stdio.h>

/*
inline int popup_menu(MENU *menu,int x,int y)
{
  while(gui_mouse_b());
  return do_menu(menu,x,y);
}

inline int bit(int val,int b)
{
 return (val>>b)&1;
}

enum { ftBIN=1, ftBMP, ftTIL, ftZGP, ftQSU };

#define vc(x)  ((x)+224)    // offset to 'VGA color' x
#define dvc(x) ((x)+192)    // offset to dark 'VGA color' x
#define lc1(x) ((x)+208)    // offset to 'level bg color' x
#define lc2(x) ((x)+240)    // offset to 'level fg color' x

#define ITEMCNT   iMax
#define WPNCNT    wMAX
#define MAXROOMTYPES   rMAX
#define MAXWARPTYPES   wtMAX-1 // minus whistle
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

*/

char *imgstr[6] =
{
  "Not loaded", "Binary/ROM", "Image", "ZC Tiles",
  "ZC Tiles", "ZC Tiles"
};

int filetype(char *path)
{
  if(path==NULL || strlen(get_filename(path))==0)
    return 0;

  char ext[40];
  strcpy(ext,get_extension(path));
  strupr(ext);

  if(strcmp(ext,"BMP")==0) return ftBMP;
  if(strcmp(ext,"GIF")==0) return ftBMP;
  if(strcmp(ext,"PCX")==0) return ftBMP;
  if(strcmp(ext,"TGA")==0) return ftBMP;
  if(strcmp(ext,"TIL")==0) return ftTIL;
  if(strcmp(ext,"ZGP")==0) return ftZGP;
  if(strcmp(ext,"QSU")==0) return ftQSU;
  if(strcmp(ext,"DAT")==0) return 0;
  if(strcmp(ext,"HTM")==0) return 0;
  if(strcmp(ext,"HTML")==0) return 0;
  if(strcmp(ext,"TXT")==0) return 0;
  if(strcmp(ext,"ZIP")==0) return 0;
  return ftBIN;
}

void load_mice()
{
  for(int i=0; i<MOUSE_BMP_MAX; i++)
  {
    for(int j=0; j<2; j++)
    {
      mouse_bmp[i][j] = create_bitmap_ex(8,16,16);
      blit((BITMAP*)zcdata[BMP_MOUSE].dat,mouse_bmp[i][j],i*17+1,j*17+1,0,0,16,16);
    }
  }
}

void load_icons()
{
  for(int i=0; i<ICON_BMP_MAX; i++)
  {
    for(int j=0; j<4; j++)
    {
      icon_bmp[i][j] = create_bitmap_ex(8,16,16);
      blit((BITMAP*)zcdata[BMP_ICONS].dat,icon_bmp[i][j],i*17+1,j*17+1,0,0,16,16);
    }
  }
}

void load_selections()
{
  for(int i=0; i<2; i++)
  {
    select_bmp[i] = create_bitmap_ex(8,16,16);
    //  blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
    blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
  }
}

void load_arrows()
{
  for(int i=0; i<MAXARROWS; i++)
  {
    arrow_bmp[i] = create_bitmap_ex(8,16,16);
    blit((BITMAP*)zcdata[BMP_ARROWS].dat,arrow_bmp[i],i*17+1,1,0,0,16,16);
  }
}

void dump_pal()
{
  for(int i=0; i<256; i++)
    rectfill(screen,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

int bound(int &x,int low,int high)
{
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
}

int wrap(int x,int low,int high)
{
  if(x<low) x=high;
  if(x>high) x=low;
  return x;
}

bool readfile(char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen(path,F_READ);
  if(!f)
    return 0;
  bool good=pfread(buf,count,f,true);
  pack_fclose(f);
  return good;
}

bool writefile(char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen(path,F_WRITE);
  if(!f)
    return 0;

  bool good=pfwrite(buf,count,f);
  pack_fclose(f);
  return good;
}

/* dotted_rect: (from allegro's guiproc.c)
 *  Draws a dotted rectangle, for showing an object has the input focus.
 */
void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
  int x = ((x1+y1) & 1) ? 1 : 0;
  int c;

  /* two loops to avoid bank switches */
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
  }
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
  }

  for (c=y1+1; c<y2; c++)
  {
    putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
    putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
  }
}

RGB _RGB(byte *si)
{
  RGB x;
  x.r = si[0];
  x.g = si[1];
  x.b = si[2];
  return x;
}

RGB _RGB(int r,int g,int b)
{
  RGB x;
  x.r = r;
  x.g = g;
  x.b = b;
  return x;
}

RGB invRGB(RGB s)
{
  RGB x;
  x.r = 63-s.r;
  x.g = 63-s.g;
  x.b = 63-s.b;
  return x;
}

/*
inline RGB NESpal(int i)
{
 return _RGB(nes_pal+(i*3));
}
*/

RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
{
  RGB x;
  x.r = ( r1*(64-ratio) + r2*ratio ) >> 6;
  x.g = ( g1*(64-ratio) + g2*ratio ) >> 6;
  x.b = ( b1*(64-ratio) + b2*ratio ) >> 6;
  return x;
}

void reset_pal_cycling();
void cycle_palette();

void load_cset(RGB *pal,int cset_index,int dataset)
{
  byte *si = colordata + CSET(dataset)*3;
  for(int i=0; i<16; i++)
  {
    pal[CSET(cset_index)+i] = _RGB(si);
    si+=3;
  }
}

void set_pal()
{
  set_palette_range(RAMpal,0,192,true);
}

void loadlvlpal(int level)
{
  Color=level;
  // full pal
  for(int i=0; i<192; i++)
    RAMpal[i] = _RGB(colordata+i*3);

  // level pal
  byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  for(int i=0; i<16; i++)
  {
    RAMpal[CSET(9)+i] = _RGB(si);
    si+=3;
  }

  reset_pal_cycling();
  set_pal();
}

void loadfadepal(int dataset)
{
  byte *si = colordata + CSET(dataset)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  set_pal();
}

void setup_lcolors()
{
  for(int i=0; i<14; i++)
  {
    RAMpal[lc1(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+2)*3);
    RAMpal[lc2(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+16+1)*3);
  }
  set_palette(RAMpal);
}

void refresh_pal()
{
  loadlvlpal(Color);
  setup_lcolors();
}

char *item_string[ITEMCNT] =
{
  "Rupee (1)", "Rupee (5)", "Heart", "Bomb (Normal)", "Clock",
  "Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Shield 1 (Magic)", "Key (Normal)",
  "Candle 1 (Blue)", "Candle 2 (Red)", "Letter", "Arrow 1 (Wooden)", "Arrow 2 (Silver)",
  "Bow 1 (Short)", "Bait", "Ring 1 (Blue)", "Ring 2 (Red)", "Bracelet 1",
  "Triforce (Fragment)", "Map", "Compass", "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)",
  "Wand", "Raft", "Ladder", "Heart Container", "Potion 1 (Blue)",
  "Potion 2 (Red)", "Whistle", "Magic Book", "Key (Magic)", "Fairy (Moving)",
  "Boomerang 3 (Fire)", "Sword 4 (Master)", "Shield 2 (Mirror)", "Rupee (20)", "Rupee (50)",
  "Rupee (200)", "Wallet 1 (500)", "Wallet 2 (999)", "Dust Pile",
  "Triforce (Whole)", "Selection", "Misc 1", "Misc 2", "Bomb (Super)","Heart Container Piece",
  "Amulet 1", "Flippers", "Hookshot", "Lens of Truth", "Hammer", "Boots",
  "Bracelet 2", "Arrow 3 (Golden)", "Magic Container", "Magic Jar 1 (Small)",
  "Magic Jar 2 (Large)", "Ring 3 (Golden)", "Kill All Enemies", "Amulet 2",
  "Din's Fire", "Farore's Wind", "Nayru's Love", "Key (Boss)", "Bow 2 (Long)", "Fairy (Stationary)"
};

char *weapon_string[WPNCNT] =
{
  "Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Sword 3 (Master)",
  "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)", "Boomerang 3 (Fire)", "Bomb (Normal)",
  "Bomb (Super)", "Explosion", "Arrow 1 (Wooden)", "Arrow 2 (Silver)", "Fire", "Whirlwind",
  "Bait", "Wand", "Magic", "Fireball", "Rock", "Arrow (Enemy)", "Sword (Enemy)",
  "Magic (Enemy)", "MISC: Spawn", "MISC: Death", "MISC: Swim", "Hammer",
  "Hookshot Head", "Hookshot Chain (Horizontal)", "Hookshot Handle", "MISC: Arrow 2 (Silver) Sparkle",
  "MISC: Arrow 3 (Golden) Sparkle", "MISC: Boomerang 2 (Magic) Sparkle", "MISC: Boomerang 3 (Fire) Sparkle",
  "MISC: Hammer Impact", "Arrow 3 (Golden)", "Fire (Enemy)", "Whirlwind (Enemy)", "MISC: Magic Meter",
  "Din's Fire (Rising)", "Din's Fire (Falling)","Din's Fire Trail (Rising)", "Din's Fire Trail (Falling)", "Hookshot Chain (Vertical)", "MISC: More...",
  "MISC: Boss Marker", "MISC: Link Slash", "Sword 1 (Wooden) Slash",
  "Sword 2 (White) Slash", "Sword 3 (Magic) Slash", "Sword 4 (Master) Slash", "MISC: Shadow (Small)",
  "MISC: Shadow (Large)", "MISC: Bush Leaves", "MISC: Flower Clippings",
  "MISC: Grass Clippings", "MISC: Tall Grass", "MISC: Ripples", "MISC: NPCs",
  "Nayru's Love (Left)", "Nayru's Love (Left, Returning)","Nayru's Love Trail (Left)", "Nayru's Love Trail (Left, Returning)",
  "Nayru's Love (Right)", "Nayru's Love (Right, Returning)","Nayru's Love Trail (Right)", "Nayru's Love Trail (Right, Returning)",
  "Nayru's Love Shield (Front)", "Nayru's Love Shield (Back)"
};

char *roomtype_string[MAXROOMTYPES] =
{
  "(None)","Special Item","Pay for Info","Secret Money","Gamble",
  "Door Repair","Heart Container","Feed the Goriya","Level 9 Entrance",
  "Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupies",
  "3-Stair Warp","Ganon","Zelda", "Item Pond", "Magic Upgrade", "Learn Slash"
};

char *catchall_string[MAXROOMTYPES] =
{
  " ","Special Item","Info Type","Amount"," ","Repair Fee"," "," "," ","Shop Type",
  "Shop Type","Price ","Price"," ","Warp Ring"," "," ", "Item Pond Type", " ",
  " "
};

char *warptype_string[MAXWARPTYPES] =
{
  "Cave/Item Room","Passageway","Entrance/Exit","Scrolling Warp","Insta-Warp","Insta-Warp with Blackout","Insta-Warp with Opening Screen","Insta-Warp with Zap Effects", "Insta-Warp with Wave Effects", "Cancel Warp"
//  "Cave/Item Room","Passageway","Entrance/Exit","Scrolling Warp","Standard","Cancel Warp"," "," ", " ", " "
};

char *warpeffect_string[MAXWARPEFFECTS] =
{
  "Instant", "Circle", "Oval", "Triangle", "Super Mario All-Stars", "Curtains (Smooth)", "Curtains (Stepped)", "Mosaic",
  "Wave White", "Wave Black", "Fade White", "Fade Black", "Global Opening/Closing", "Destination Default"
};

char *combotype_string[MAXCOMBOTYPES] =
{
  "(None)","Stairs","Cave (Walk Down)","Water","Armos","Grave","Dock",
  "Undefined","Push (Wait)","Push (Heavy)","Push (Heavy, Wait)","Left Statue","Right Statue",
  "Slow Walk","Conveyor Up","Conveyor Down","Conveyor Left","Conveyor Right",
  "Swim Warp","Dive Warp","Ladder or Hookshot","Trigger (Temporary)","Trigger (Permanent)",
  "Win Game","Slash","Slash (->item)", "Push (Very Heavy)", "Push (Very Heavy, Wait)", "Pound",
  "Hookshot Grab", "-Hookshot Bridge", "Damage (1/2 Heart)", "Damage (1 Heart)", "Damage (2 hearts)", "Damage (4 Hearts)",
  "Center Statue", "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (4-Way)",
  "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Direct Warp", "Hookshot Only", "Overhead",
  "No Fly Zone", "Magic Mirror (4-Way)", "Magic Mirror (Up-Left, Down-Right)", "Magic Mirror (Up-Right, Down-Left)", "Magic Prism (3-Way)",
  "Magic Prism (4-Way)", "Magic Sponge", "Cave (Walk Up)", "Eyeball A", "Eyeball B",
  "No Jump Zone", "Bush", "Flowers", "Tall Grass", "Shallow Water", "Lock Block",
  "Lock Block (Copycat)", "Boss Lock Block", "Boss Lock Block (Copycat)", "Ladder Only", "BS Grave"
};

char *flag_string[MAXFLAGS] =
{
  " 0 (None)"," 1 Push (Vertical, Once)"," 2 Push (4-Way, Once)"," 3 Whistle"," 4 Burn 1 (Any)",
  " 5 Arrow 1 (Any)"," 6 Bomb (Any)"," 7 Fairy"," 8 Raft"," 9 Armos (->Secret)",
  "10 Armos (->Item)","11 Bomb (Super)","12 Raft Branch",
  "13 Dive for Item",
  "14 Lens Marker Flag",
  "15 Zelda",
  "16 Secret Tile 0",
  "17 Secret Tile 1",
  "18 Secret Tile 2",
  "19 Secret Tile 3",
  "20 Secret Tile 4",
  "21 Secret Tile 5",
  "22 Secret Tile 6",
  "23 Secret Tile 7",
  "24 Secret Tile 8",
  "25 Secret Tile 9",
  "26 Secret Tile 10",
  "27 Secret Tile 11",
  "28 Secret Tile 12",
  "29 Secret Tile 13",
  "30 Secret Tile 14",
  "31 Secret Tile 15",
  "32 Trap (Horizontal, Line of Sight)",
  "33 Trap (Vertical, Line of Sight)",
  "34 Trap (4-Way, Line of Sight)",
  "35 Trap (Horizontal, Constant)",
  "36 Trap (Vertical, Constant)",
  "37 Enemy 1",
  "38 Enemy 2",
  "39 Enemy 3",
  "40 Enemy 4",
  "41 Enemy 5",
  "42 Enemy 6",
  "43 Enemy 7",
  "44 Enemy 8",
  "45 Enemy 9",
  "46 Enemy 10",
  "47 Push (Horizontal, Once)",
  "48 Push (Up, Once)",
  "49 Push (Down, Once)",
  "50 Push (Left, Once)",
  "51 Push (Right, Once)",
  "52 Push (Vertical, Once, Silent)",
  "53 Push (Horizontal, Once, Silent)",
  "54 Push (4-Way, Once, Silent)",
  "55 Push (Up, Once, Silent)",
  "56 Push (Down, Once, Silent)",
  "57 Push (Left, Once, Silent)",
  "58 Push (Right, Once, Silent)",
  "59 Push (Vertical, Many, Silent)",
  "60 Push (Horizontal, Many, Silent)",
  "61 Push (4-Way, Many, Silent)",
  "62 Push (Up, Many, Silent)",
  "63 Push (Down, Many, Silent)",
  "64 Push (Left, Many, Silent)",
  "65 Push (Right, Many, Silent)",
  "66 Block Trigger",
  "67 No Push Blocks",
  "68 Boomerang 1 (Any)",
  "69 Boomerang 2 (Magic +)",
  "70 Boomerang 3 (Fire)",
  "71 Arrow 2 (Silver +)",
  "72 Arrow 3 (Golden)",
  "73 Burn 2 (Red Candle +)",
  "74 Burn 3 (Wand Fire)",
  "75 Burn 4 (Din's Fire)",
  "76 Magic (Wand)",
  "77 Magic (Reflected)",
  "78 Fireball (Reflected)",
  "79 Sword 1 (Any)",
  "80 Sword 2 (White +)",
  "81 Sword 3 (Magic +)",
  "82 Sword 4 (Master)",
  "83 Sword Beam 1 (Any)",
  "84 Sword Beam 2 (White +)",
  "85 Sword Beam 3 (Magic +)",
  "86 Sword Beam 4 (Master)",
  "87 Hookshot",
  "88 Wand",
  "89 Hammer",
  "90 Strike"
};

// eMAXGUYS is defined in zdefs.h
char *guy_string[eMAXGUYS] =
{
  "(None)","Abei","Ama","Merchant","Moblin","Fire","Fairy","Goriya","Zelda","",
  // 010
  "Octorok (Red, Slow)","Octorok (Blue, Slow)","Octorok (Red, Fast)","Octorok (Blue, Fast)","Tektite (Red)",
  // 015
  "Tektite (Blue)","Leever (Red)","Leever (Blue)","Moblin (Red)","Moblin (Black)",
  // 020
  "Lynel (Red)","Lynel (Blue)","Peahat","Zora <slide-proof>","Rock <slide-proof>",
  // 025
  "Ghini 1","Ghini 2","Armos","Keese (Blue)","Keese (Red)",
  // 030
  "Keese (Level Color)","Stalfos 1","Gel (normal)","Zol (normal)","Rope 1",
  // 035
  "Goriya (Red)","Goriya (Blue)","-trap","Wall Master","Darknut (Red)",
  // 040
  "Darknut (Blue)","Bubble","Vire (normal)","Like Like","Gibdo",
  // 045
  "Pols Voice","Wizzrobe (Red)","Wizzrobe (Blue)","Aquamentus (Facing Left)","Moldorm",
  // 050
  "Dodongo","Manhandla 1","Gleeok (1 Head)","Gleeok (2 Heads)","Gleeok (3 Heads)",
  // 055
  "Gleeok (4 Heads)","Digdogger (1 Kid)","Digdogger (3 Kids)","Digdogger Kid (1)","Digdogger Kid (2)",
  // 060
  "Digdogger Kid (3)","Digdogger Kid (4)","Gohma (Red)","Gohma (Blue)","Lanmola (Red)",
  // 065
  "Lanmola (Blue)","Patra (Big Circle)","Patra (Oval)","-Ganon","Stalfos 2",
  // 070
  "Rope 2","Bubble (Red)","Bubble (Blue)","Fire Shooter","-item fairy",
  // 075
  "Fire","Octorok (Magic)", "Darknut (Death Knight)", "Gel (Tribble)", "Zol (Tribble)",
  // 080
  "Keese (Tribble)", "Vire (Tribble)", "Darknut (Splitting)", "Aquamentus (Facing Right)", "Manhandla 2",
  // 085
  "-trap h", "-trap v", "-trap lr", "-trap ud", "Wizzrobe (Fire)",
  // 090
  "Wizzrobe (Wind)", "-ceiling master", "-floor master", "Patra (BS Zelda)", "Patra 2",
  // 095
  "Patra 3", "Bat", "Wizzrobe (Bat)", "-wizzrobe - bat2", "Gleeok (Fire, 1 Head)",
  // 100
  "Gleeok (Fire, 2 Heads)",  "Gleeok (Fire, 3 Heads)","Gleeok (Fire, 4 Heads)", "Wizzrobe (Mirror)", "Dodongo (BS Zelda)",
  // 105
  "-dodongo - fire","Trigger", "Bubble (Item, Temporary Disabling)", "Bubble (Item, Permanent Disabling)", "Bubble (Item, Re-enabling)",
  // 110
  "Stalfos 3", "Gohma 3", "Gohma 4", "-NPC 1 (Standing)", "-NPC 2 (Standing)",
  // 115
  "-NPC 3 (Standing)", "-NPC 4 (Standing)", "-NPC 5 (Standing)", "-NPC 6 (Standing)", "-NPC 1 (Walking)",
  // 120
  "-NPC 2 (Walking)", "-NPC 3 (Walking)", "-NPC 4 (Walking)", "-NPC 5 (Walking)", "-NPC 6 (Walking)"
  // 125
};

char *pattern_string[MAXPATTERNS] =
{
  "Random","Enter from Sides"
};

char *short_pattern_string[MAXPATTERNS] =
{
  "Random", "Sides"
};

char *midi_string[MAXMIDIS_ZQ] =
{
  "(None)",
  "Overworld",
  "Dungeon",
  "Level 9",
};

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

int onSpacebar()
{
  combo_cols=!combo_cols;
  return D_O_K;
}

int onSnapshot()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.bmp", ++num);
  } while(num<999 && exists(buf));

  blit(screen,screen2,0,0,0,0,zq_screen_w,zq_screen_h);
  PALETTE RAMpal2;
  get_palette(RAMpal2);
  save_bitmap(buf,screen2,RAMpal2);
  return D_O_K;
}

int gocnt=0;

void go()
{
  switch(gocnt)
  {
    case 0:
      scare_mouse();
      blit(screen,menu1,0,0,0,0,zq_screen_w,zq_screen_h);
      unscare_mouse();
      break;
    case 1:
      scare_mouse();
      blit(screen,menu3,0,0,0,0,zq_screen_w,zq_screen_h);
      unscare_mouse();
      break;
    default: return;
  }
  ++gocnt;
}

void comeback()
{
  switch(gocnt)
  {
    case 1:
      scare_mouse();
      blit(menu1,screen,0,0,0,0,zq_screen_w,zq_screen_h);
      unscare_mouse();
      break;
    case 2:
      scare_mouse();
      blit(menu3,screen,0,0,0,0,zq_screen_w,zq_screen_h);
      unscare_mouse();
      break;
    default: return;
  }
  --gocnt;
}

int checksave()
{
  if(saved)
    return 1;
  char buf[80];
  char *name = get_filename(filepath);
  if(name[0]==0)
    sprintf(buf,"Save this quest file?");
  else
    sprintf(buf,"Save changes to %s?",name);
  switch(jwin_alert3("ZQuest",buf,NULL,NULL,"&Yes","&No","Cancel",'y','n',27,lfont))
  {
    case 1:
      onSave();
      return 1;
    case 2:
      return 1;
  }
  return 0;
}

int onExit()
{
  restore_mouse();
  if(checksave()==0)
    return D_O_K;
  if(jwin_alert("ZQuest","Really want to quit?", NULL, NULL, "&Yes", "&No", 'y', 'n', lfont) == 2)
    return D_O_K;
  return D_CLOSE;
}

int onAbout()
{
  char buf1[40];
  char buf2[40];
  char buf3[40];
  if(debug)
  {
    if (IS_BETA)
    {
      sprintf(buf1,"ZQuest %s Beta (Build %d) - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
    }
    else
    {
      sprintf(buf1,"ZQuest %s (Build %d) - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
    }
    sprintf(buf2,"ZQuest Editor: %04X",INTERNAL_VERSION);
    sprintf(buf3,"This qst file: %04X",header.internal&0xFFFF);
    jwin_alert("About ZQuest",buf1,buf2,buf3,"OK", NULL, 13, 27, lfont);
  }
  else
  {
    switch (IS_BETA)
    {
      case -1:
        sprintf(buf1,"ZQuest %s Alpha (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
      case 1:
        sprintf(buf1,"ZQuest %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
      case 0:
        sprintf(buf1,"ZQuest %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
    }
    jwin_alert("About ZQuest",buf1,NULL,NULL,"OK", NULL, 13, 27, lfont);
  }
  return D_O_K;
}

int onW()
{
  Flags^=cWALK;
  refresh(rMAP+rMENU);
  return D_O_K;
}

int onF()
{
  Flags^=cFLAGS;
  refresh(rMAP);
  return D_O_K;
}

int onQ()
{
  if(Flags&cCSET)
  {
    Flags ^= cCSET;
    Flags |= cCTYPE;
  }
  else if(Flags&cCTYPE)
  {
    Flags ^= cCTYPE;
  }
  else
  {
    Flags |= cCSET;
  }

  refresh(rMAP);
  return D_O_K;
}

int onL()
{
  if(get_bit(quest_rules,qr_FADE))
  {
    int last = CSET(5)-1;

    if(get_bit(quest_rules,qr_FADECS5))
      last += 16;

    byte *si = colordata + CSET(Color*pdLEVEL+poFADE1)*3;
    for(int i=0; i<16; i++)
    {
      int light = si[0]+si[1]+si[2];
      si+=3;
      fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+i,CSET(2)+i);
    }
    fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
    set_palette(RAMpal);

    readkey();

    load_cset(RAMpal,5,5);
    loadlvlpal(Color);
  }
  else
  {
    loadfadepal(Color*pdLEVEL+poFADE3);
    readkey();
    loadlvlpal(Color);
  }
  return D_O_K;
}

int onM()
{
  return D_O_K;
}

void setFlagColor()
{
  RAMpal[dvc(0)]=RAMpal[vc(Flag%16)];
  set_palette_range(RAMpal,dvc(0),dvc(0),false);
}

int onTimes()
{
  Flag=(Flag+1);
  if (Flag==mfMAX)
  {
    Flag=0;
  }
  setFlagColor();
  refresh(rMENU);
  return D_O_K;
}

int onDivide()
{
  if (Flag==0)
  {
    Flag=mfMAX;
  }
  Flag=(Flag-1);
  setFlagColor();
  refresh(rMENU);
  return D_O_K;
}

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
