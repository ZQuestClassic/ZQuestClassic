//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.cc
//
//  Main code for Zelda Classic. Originally written in
//  SPHINX C--, now rewritten in DJGPP with Allegro.
//
//--------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dir.h>
#include <ctype.h>
#include <allegro.h>
//#include <libcda.h>   // for CD support

#include "zdefs.h"
#include "zelda.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "aglogo.h"
#include "zsys.h"
#include "qst.h"
#include "matrix.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "zgpdat.h"
#include <save_gif.h>

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



int db=0;



/**********************************/
/*** Classes, Definitions, Etc. ***/
/**********************************/

#include "sprite.cc"
#include "link.cc"

/**********************************/
/******** Global Variables ********/
/**********************************/

sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks;

BITMAP     *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *fps_undo, *msgdisplaybuf, *overbuf;
DATAFILE   *data, *sfxdata, *mididata, *zgpdata;
SAMPLE     wav_refill;
FONT       *zfont, *deffont, *mfont;
PALETTE    RAMpal;
byte       *tilebuf, *colordata;
newcombo   *combobuf;
itemdata   *itemsbuf;
wpndata    *wpnsbuf;
guydata    *guysbuf;
ZCHEATS    zcheats;
zinitdata  zinit;
char       palnames[256][17];
word animated_combo_table[MAXCOMBOS][2]; //[0]=original tile, [1]=clock
bool ewind_restart=false;
word     msgclk, msgstr, msgpos;
word     introclk, intropos, dmapmsgclk;
short    Bpos, lensclk, lenscnt;
byte screengrid[22];

int homescr,currscr,frame=0,currmap=0,dlevel,warpscr,worldscr;
int newscr_clk=0,opendoors=0,currdmap=0,fadeclk=-1,currgame=0,listpos=0;
int lastentrance,prices[3],loadside, Bwpn, Awpn;
int digi_volume,midi_volume,currmidi,wand_x,wand_y,hasitem,whistleclk,pan_style;
int Akey,Bkey,Skey,Lkey,Rkey,Pkey,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Quit=0;
int DUkey, DDkey, DLkey, DRkey, ss_after, ss_speed, ss_density;
int arrow_x, arrow_y, brang_x, brang_y, chainlink_x, chainlink_y;
int chainlinks_count, hs_startx, hs_starty, clockclk, clock_zoras;
int cheat_goto_map=0, cheat_goto_screen=0, swordhearts[4], currcset;
int gfc, gfc2, pitx, pity, refill_what, heart_beep_timer=0, new_enemy_tile_start=1580;
int nets=1580, magictype=mgc_none, magiccastclk, castx, casty, df_x, df_y;
int magicdrainclk=0, conveyclk=3;

bool Vsync, Paused=false, Advance=false, ShowFPS, Showpal=false, Playing;
bool refreshpal,blockpath,wand_dead,debug=false,loaded_guys,freeze_guys,
     loaded_enemies,drawguys,details=false,DXtitle,debug_enabled,watch;
bool darkroom=false,BSZ,COOLSCROLL,NEWSUBSCR;
bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown,
     SystemKeys=true,NESquit,volkeys,useCD=false,boughtsomething=false,
     add_asparkle=false, add_bsparkle=false, fixed_door=false,
     hookshot_used=false, hookshot_frozen=false, pull_link=false,
     add_chainlink=false, del_chainlink=false, hs_fix=false,
     cheat_superman=false, gofast=false, didpit=false, heart_beep=true,
     pausenow=false, castnext=false, add_df1asparkle, add_df1bsparkle,
     is_on_conveyor, was_on_conveyor;

char   sig_str[44];
char   cheat_goto_map_str[4];
char   cheat_goto_screen_str[3];
short  visited[6];
byte   guygrid[176];
mapscr tmpscr[2];
gamedata game;

movingblock mblock[4];
LinkClass   Link;

int VidMode,resx,resy,scrx,scry;
bool sbig; // big screen
bool toogam=false;

int cheat=0; // 0 = none; 1,2,3,4 = cheat level

int mouse_down;  // used to hold the last reading of 'mouse_b' status
int idle_count;


// quest file data
zquestheader QHeader;
MsgStr       *MsgStrings;
dmap         *DMaps;
miscQdata    QMisc;
mapscr       *TheMaps;

char     *qstpath=NULL;
gamedata *saves=NULL;


volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync=0;

inline void addLwpn(int x,int y,int id,int type,int power,int dir)
{
 Lwpns.add(new weapon(x,y,id,type,power,dir));
}



/**********************************/
/*********** Misc Data ************/
/**********************************/

const char startguy[8] = {-13,-13,-13,-14,-15,-18,-21,-40};
const char gambledat[12*6] ={ 20,-10,-10, 20,-10,-10, 20,-40,-10, 20,-10,-40,
	                      50,-10,-10, 50,-10,-10, 50,-40,-10, 50,-10,-40,
                              -10,20,-10, -10,20,-10, -40,20,-10, -10,20,-40,
                              -10,50,-10, -10,50,-10, -40,50,-10, -10,50,-40,
                              -10,-10,20, -10,-10,20, -10,-40,20, -40,-10,20,
                              -10,-10,50, -10,-10,50, -10,-40,50, -40,-10,50 };
const byte stx0[9] = { 48, 80, 80, 96,112,144,160,160,192};
const byte stx1[9] = { 48, 80, 80, 96,128,144,160,160,192};
const byte stx2[9] = { 80, 80,128,128,160,160,192,192,208};
const byte stx3[9] = { 32, 48, 48, 80, 80,112,112,160,160};
const byte sty0[9] = {112, 64,128, 96, 80, 96, 64,128,112};
const byte sty1[9] = { 48, 32, 96, 64, 80, 64, 32, 96, 48};
const byte sty2[9] = { 32,128, 64, 96, 64, 96, 48,112, 80};
const byte sty3[9] = { 80, 48,112, 64, 96, 64, 96, 32,128};

const byte ten_rupies_x[10] = {120,112,128,96,112,128,144,112,128,120};
const byte ten_rupies_y[10] = {49,65,65,81,81,81,81,97,97,113};


music tunes[MAXMUSIC] = {
 // (title)                (s) (ls) (le) (l) (vol) (midi)
 { "Zelda 1 - Dungeon",     0,  -1,  -1,  1,  176,  NULL },
 { "Zelda 1 - Ending",      0, 129, 225,  1,  160,  NULL },
 { "Zelda 1 - Game Over",   0,  -1,  -1,  1,  224,  NULL },
 { "Zelda 1 - Level 9",     0,  -1,  -1,  1,  255,  NULL },
 { "Zelda 1 - Overworld",   0,  17,  -1,  1,  208,  NULL },
 { "Zelda 1 - Title",       0,  -1,  -1,  0,  168,  NULL },
 { "Zelda 1 - Triforce",    0,  -1,  -1,  0,  168,  NULL },
};


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
  memcpy(QMisc.cycles, tmpptr6, sizeof(palcycle)*256*3); //read cycling info
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


/*
void init_tiles()
{
  for(dword i=0; i<NEWTILE_SIZE2/4; i++)
    ((long*)tilebuf)[i]=0;
  for(int i=0; i<OLDTILE_SIZE2/4; i++)
    ((long*)tilebuf)[i]=((long*)data[TIL_NES].dat)[i];
}

bool init_colordata()
{
  if(colordata==NULL)
    return false;

//  clear_colordata();

  byte *src=(byte*)data[DAT_PAL].dat;
  memcpy(colordata, src, newpdTOTAL*48);

  return true;
}
*/

void dointro () {
  if (game.visited[currdmap]==0) {
    dmapmsgclk=0;
    game.visited[currdmap]=1;
    introclk=intropos=0;
  }
}

bool bad_version(zquestheader *header)
{
   // minimum zquest version allowed for any quest file
   if(header->zelda_version < 0x183)
     return true;

   return false;
}


/**********************************/
/******* Other Source Files *******/
/**********************************/

#include "maps.cc"
#include "subscr.cc"
#include "guys.cc"

#include "title.cc"
#include "ending.cc"

#include "zc_sys.cc"



void ALLOFF()
{
  clear(msgdisplaybuf);
  if(items.idCount(iPile))
    loadlvlpal(DMaps[currdmap].color);
  items.clear();
  guys.clear();
  Lwpns.clear();
  Ewpns.clear();
  chainlinks.clear();
  Link.resetflags(false);
  Link.reset_hookshot();
  add_asparkle=false;
  add_bsparkle=false;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  for(int i=0; i<4; i++)
    mblock[i].clk=0;
  msgstr=0;
  fadeclk=-1;
  lensclk = lenscnt = 0;
  drawguys=Udown=Ddown=Ldown=Rdown=Adown=Bdown=Sdown=true;
  if(watch)
    Link.setClock(false);
  watch=freeze_guys=loaded_guys=loaded_enemies=wand_dead=blockpath=false;
  stop_sfx(WAV_BRANG);
  for(int i=0; i<176; i++)
    guygrid[i]=0;
  sle_clk=0;
}


fix LinkX()   { return Link.getX(); }
fix LinkY()   { return Link.getY(); }
int LinkDir() { return Link.getDir(); }

bool getClock() { return Link.getClock(); }
void setClock(bool state) { Link.setClock(state); }
void CatchBrang() { Link.Catch(); }



/**************************/
/***** Main Game Code *****/
/**************************/


int load_quest(gamedata *g, bool report=true)
{
  chop_path(qstpath);

  if(g->quest<255)
    sprintf(qstpath,"%s%s.qst",qstpath,ordinal(g->quest));
  else
    sprintf(qstpath,"%s%s",qstpath,get_filename(g->qstpath));


  int ret = loadquest(qstpath,&QHeader,&QMisc,tunes+MUSIC_COUNT);

  if(!g->title[0] || !g->hasplayed)
  {
    strcpy(g->version,QHeader.version);
    strcpy(g->title,QHeader.title);
  }
  else
  {
    if(!ret && strcmp(g->title,QHeader.title))
      ret = qe_match;
  }

  if(QHeader.minver[0])
  {
    if(strcmp(g->version,QHeader.minver) < 0)
      ret = qe_minver;
  }

  if(ret && report)
  {
    system_pal();
    char buf1[80],buf2[80];
    sprintf(buf1,"Error loading %s:",get_filename(qstpath));
    sprintf(buf2,"%s",qst_error[ret]);
    jwin_alert("File error",buf1,buf2,qstpath,"OK",NULL,13,27);
    game_pal();
  }

  return ret;
}



int init_game()
{
  didpit=false;
  dmapmsgclk=0;
  Link.unfreeze();
  Link.reset_hookshot();
  add_asparkle=false;
  add_bsparkle=false;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  gofast=false;
  srand(frame);

  // copy saved data to RAM data
  game = saves[currgame];

  if(load_quest(&game))
  {
    Quit=qERROR;
    return 1;
  }

  BSZ = get_bit(QHeader.rules,qr1_BSZELDA);
  COOLSCROLL = get_bit(QHeader.rules,qr1_COOLSCROLL);
  NEWSUBSCR = get_bit(QHeader.rules2,qr2_NEWSUBSCR);

//  homescr = currscr = DMaps[0].cont;
//  currdmap = warpscr = worldscr=0;
  currdmap = warpscr = worldscr=game.continue_dmap;
  if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW) {
    homescr = currscr = DMaps[currdmap].cont;
  } else {
    homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
  }
  currmap = DMaps[currdmap].map;
  dlevel = DMaps[currdmap].level;
  sle_x=sle_y=newscr_clk=opendoors=Bwpn=Bpos=0;
  fadeclk=-1;
  game.maps[(currmap<<7)+currscr] |= mVISITED; // mark as visited

  for(int i=0; i<6; i++)
    visited[i]=-1;
  game.lvlitems[9]&=~liBOSS;

  ALLOFF();
  whistleclk=-1;
  clockclk=0;
  currcset=DMaps[currdmap].color;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);
  Link.init();
  Link.resetflags(true);

  copy_pal((RGB*)data[PAL_GUI].dat,RAMpal);
  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);

  if (!game.hasplayed) {
    game.maxlife=zinit.hc*HP_PER_HEART;
    game.sword=zinit.sword;
    game.brang=zinit.boomerang;
    game.bombs=zinit.bombs;
    game.arrow=zinit.arrow;
    game.candle=zinit.candle;
    game.whistle=(zinit.whistle>0)?1:0;
    game.potion=zinit.potion;
    game.ring=min(zinit.ring,3); ringcolor();
    game.keys=zinit.keys;
    game.maxbombs=zinit.max_bombs;
    game.wallet=zinit.wallet;
    game.sbombs=zinit.super_bombs;
    game.HCpieces=zinit.hcp;
    game.rupies=zinit.rupies;
    game.letter=get_bit(&(zinit.items),idI_LETTER)?1:0;
    game.bait=get_bit(&(zinit.items),idI_BAIT)?1:0;
    game.wand=get_bit(&(zinit.items),idI_WAND)?1:0;
    if (zinit.bracelet>0) {
      game.misc|=iBRACELET;
      if (zinit.bracelet>1) {
        game.misc2|=iL2BRACELET;
      }
    }
    if (zinit.bow>0) {
      game.misc|=iBOW;
    }

    if (zinit.shield>1) {
      game.misc|=iSHIELD;
      if (zinit.shield>2) {
        game.misc|=iMSHIELD;
      }
    }

    game.misc|=get_bit(&(zinit.equipment),idE_RAFT)?iRAFT:0;
    game.misc|=get_bit(&(zinit.equipment),idE_LADDER)?iLADDER:0;
    game.misc|=get_bit(&(zinit.equipment),idE_BOOK)?iBOOK:0;
    game.misc|=get_bit(&(zinit.equipment),idE_KEY)?iMKEY:0;
    if (zinit.amulet>0) {
      game.misc2|=iCROSS;
    }
    game.misc2|=get_bit(&(zinit.equipment),idE_FLIPPERS)?iFLIPPERS:0;
    game.misc2|=get_bit(&(zinit.equipment),idE_BOOTS)?iBOOTS:0;

    game.misc2|=get_bit(&(zinit.items),idI_HOOKSHOT)?iHOOKSHOT:0;
    game.misc2|=get_bit(&(zinit.items),idI_LENS)?iLENS:0;
    game.misc2|=get_bit(&(zinit.items),idI_HAMMER)?iHAMMER:0;

    for (int i=0; i<MAXLEVELS; i++) {
      game.lvlitems[i]|=get_bit(zinit.map,i)?liMAP:0;
      game.lvlitems[i]|=get_bit(zinit.compass,i)?liCOMPASS:0;
    }

    game.maxmagic=zinit.maxmagic*MAGICPERBLOCK;
    game.magic=zinit.magic*MAGICPERBLOCK;
    game.magicdrainrate=get_bit(zinit.misc,idM_DOUBLEMAGIC)?1:2;

  }

  for (int x=0; x<4; x++) {
    swordhearts[x]=zinit.swordhearts[x];
  }

  if (!game.hasplayed) {
    game.life = zinit.start_heart*HP_PER_HEART;
  } else {
    if(get_bit(zinit.misc,idM_CONTPERCENT)) {
      game.life = ((game.maxlife*zinit.cont_heart/100)/HP_PER_HEART)*HP_PER_HEART;
    } else {
      game.life = zinit.cont_heart*HP_PER_HEART;
    }
  }
  game.hasplayed=1;

  if(get_bit(QHeader.rules2,qr2_CONTFULL))
    game.life = game.maxlife;
/*
  else
    game.life=3*HP_PER_HEART;
*/

  selectBwpn(0);
  selectAwpn(0);
  reset_subscr_items();

  for(int i=0; i<128; i++)
    key[i]=0;

  Playing=true;
  lighting(2);
  map_bkgsfx();
  openscreen();

  loadguys();

  if(isdungeon() && currdmap>0)
  {
    Link.stepforward(12);
  }

/*
  if(hasitem) {
    int Item=tmpscr->item;
    if(getmapflag())
      Item=0;
    if(Item) {
      if(hasitem==1)
        sfx(WAV_CLEARED);
      additem(tmpscr->itemx,tmpscr->itemy+1,Item,ipONETIME+ipBIGRANGE+
        ((Item==iTriforce || (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0));
      }
    hasitem=0;
    }
*/
  if(!Quit)
    play_DmapMidi();

  return 0;
}



int cont_game()
{
  didpit=false;
  dmapmsgclk=0;
  Link.unfreeze();
  Link.reset_hookshot();
  add_asparkle=false;
  add_bsparkle=false;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  if(DMaps[currdmap].cont >= 0x80)
  {
    homescr = currscr = DMaps[0].cont;
    currdmap = warpscr = worldscr=0;
    currmap = DMaps[0].map;
    dlevel = DMaps[0].level;
  }
  else if(dlevel)
  {
    homescr = currscr = lastentrance;
  }
  else
  {
    if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW) {
      homescr = currscr = DMaps[currdmap].cont;
    } else {
      homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
    }
  }

  for(int i=0; i<6; i++)
    visited[i]=-1;
  if(dlevel==0)
    game.lvlitems[9]&=~liBOSS;

  ALLOFF();
  whistleclk=-1;
  currcset=DMaps[currdmap].color;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);

  Link.init();

  if(get_bit(zinit.misc,idM_CONTPERCENT)) {
    game.life = ((game.maxlife*zinit.cont_heart/100)/HP_PER_HEART)*HP_PER_HEART;
  } else {
    game.life = zinit.cont_heart*HP_PER_HEART;
  }

  if(get_bit(QHeader.rules2,qr2_CONTFULL))
    game.life = game.maxlife;
/*
  else
    game.life=3*HP_PER_HEART;
*/

  for(int i=0; i<128; i++)
    key[i]=0;

  Playing=true;
  lighting(2);
  map_bkgsfx();
  openscreen();
  loadguys();

/*
  system_pal();
  jwin_alert("continue",NULL,NULL,qstpath,"OK",NULL,13,27);
  game_pal();
  if(hasitem) {
  system_pal();
  jwin_alert("continue2",NULL,NULL,qstpath,"OK",NULL,13,27);
  game_pal();
    int Item=tmpscr->item;
    if(getmapflag())
      Item=0;
    if(Item) {
      if(hasitem==1)
        sfx(WAV_CLEARED);
      additem(tmpscr->itemx,tmpscr->itemy+1,Item,ipONETIME+ipBIGRANGE+
        ((Item==iTriforce || (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0));
      }
    hasitem=0;
    }
*/
  if(!Quit)
  {
    play_DmapMidi();
    if(isdungeon())
      Link.stepforward(12);
    newscr_clk=frame;
  }
  return 0;
}


void restart_level()
{
  blackscr(16,true);
  homescr = currscr = (dlevel) ? lastentrance : DMaps[currdmap].cont;

  for(int i=0; i<6; i++)
    visited[i]=-1;

  ALLOFF();
  whistleclk=-1;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
  Link.init();
  lighting(2);
  map_bkgsfx();
  openscreen();
  loadguys();
/*
  if(hasitem) {
    int Item=tmpscr->item;
    if(getmapflag())
      Item=0;
    if(Item) {
      if(hasitem==1)
        sfx(WAV_CLEARED);
      additem(tmpscr->itemx,tmpscr->itemy+1,Item,ipONETIME+ipBIGRANGE+
        ((Item==iTriforce || (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0));
      }
    hasitem=0;
    }
*/
  if(!Quit)
  {
    play_DmapMidi();
    if(isdungeon())
      Link.stepforward(12);
    newscr_clk=frame;
  }
}

void putintro()
{
 if (!stricmp("                                                                        ", DMaps[currdmap].intro)) {
   return;
 }

 if(intropos>=72)
   return;

 if(((introclk++)%6<5)&&((!cAbtn()&&!cBbtn())||(!get_bit(QHeader.rules3,qr3_ALLOWFASTMSG))))
   return;

 dmapmsgclk=51;
 if(intropos == 0) {
   while(DMaps[currdmap].intro[intropos]==' ')
     intropos++;
   }

 sfx(WAV_MSG);

 text_mode(-1);
// textprintf(scrollbuf,zfont,((msgpos%24)<<3)+32,((msgpos/24)<<3)+40,CSET(0)+1,
//   "%c",MsgStrings[msgstr].s[msgpos]);
 textprintf(msgdisplaybuf,zfont,((intropos%24)<<3)+32,((intropos/24)<<3)+40,CSET(0)+1,
   "%c",DMaps[currdmap].intro[intropos]);
 text_mode(0);

 intropos++;

 if(DMaps[currdmap].intro[intropos]==' ' && DMaps[currdmap].intro[intropos+1]==' ')
   while(DMaps[currdmap].intro[intropos]==' ')
     intropos++;

 if(intropos>=72)
 {
//   Link.unfreeze();
   dmapmsgclk=50;

 }
}


void show_details() {
  if(details)
  {
    text_mode(BLACK);

    textprintf(framebuf,font,0,0,WHITE,"%-4d",whistleclk);
    textprintf(framebuf,font,0,8,WHITE,"dlvl:%-2d dngn:%d", dlevel, isdungeon());
    textprintf(framebuf,font,256,232,WHITE,"%ld %s",game.time,time_str_long(game.time));

    for(int i=0; i<24; i++)
    {
      if(i>=Ewpns.Count())
        textout(framebuf,font,"   ",296,(i<<3)+16,0);
      else
      {
        textprintf(framebuf,font,200,(i<<3)+16,vc(0),"%d %d %d",(((weapon*)Ewpns.spr(i))->id),(((weapon*)Ewpns.spr(i))->wid),(((weapon*)Ewpns.spr(i))->tile));
      }
    }

    for(int i=0; i<24; i++)
    {
      if(i>=Lwpns.Count())
        textout(framebuf,font,"   ",0,(i<<3)+16,0);
      else
      {
        textprintf(framebuf,font,0,(i<<3)+16,vc(0),"%d %d",(((weapon*)Lwpns.spr(i))->id),((((weapon*)Lwpns.spr(i))->ignoreLink)?1:0));
      }
    }
  }
}

void do_magic_casting() {
  switch (magictype) {
    case mgc_none:
      magiccastclk=0;
      break;
    case mgc_dinsfire:
      {
        int fm=32;
        if (magiccastclk==0) {
          Lwpns.add(new weapon(LinkX(),LinkY(),wPhantom,0,0,up));
          weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
          w1->step=4;
          Link.tile=(BSZ)?32:29;
          casty=Link.getY();
        }
        if (magiccastclk==64) {
          Lwpns.add(new weapon(LinkX(),-32,wPhantom,1,0,down));
          weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
          w1->step=4;
          Link.tile=29;
          castnext=false;
        }
        if (castnext) {
          Link.tile=4;
          for (int fc=((-1)*(fm/2))+1; fc<((fm/2)+1); fc++) {
            Lwpns.add(new weapon(LinkX(),LinkY(),wFire,3,8*DAMAGE_MULTIPLIER,0));
            weapon *w = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
            w->step=2;
            w->angular=true;
            w->angle=(fc*PI/(fm/2));
          }
          castnext=false;
          magiccastclk=128;
        }

/*
*/
        if ((magiccastclk++)==226) {
          magictype=mgc_none;
        }
      }
      break;
    default:
      magiccastclk=0;
      break;
  }
}

void game_loop()
{
  if(fadeclk>=0) {
    if(fadeclk==0 && currscr<128)
      blockpath=false;
    fadeclk--;
    }

  animate_combos();
  putscr(scrollbuf,0,0,tmpscr);
  for(int i=0; i<4; i++)
    mblock[i].animate(0);
  items.animate();
  guys.animate();
  roaming_item();
  Ewpns.animate();
  if(Link.animate(0)) {
    if(!Quit)
      Quit=qGAMEOVER;
    return;
    }
  do_magic_casting();
  Lwpns.animate();
  chainlinks.animate();
  if (conveyclk<=0) {
    conveyclk=3;
  }
  conveyclk--;
  check_collisions();

  dryuplake();
  cycle_palette();
/*
  putintro();

  if (dmapmsgclk>0) {
    Link.Freeze();
    if (dmapmsgclk<=50) {
      dmapmsgclk--;
    }
  }
  if (dmapmsgclk==1) {
    Link.unfreeze();
    dmapmsgclk=0;
    clear(msgdisplaybuf);
  }

  putmsg();
  domoney();
*/
//  putsubscr(framebuf,0,0);

  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  if(lensclk || (debug && key[KEY_L]))
    draw_lens_under();

  do_layer2(0, tmpscr, 0, 0);
  do_layer2(1, tmpscr, 0, 0);
  Link.draw_under(framebuf);
  for(int i=0; i<4; i++)
    mblock[i].draw(framebuf);
  if (Link.isSwimming()) {
    Link.draw(framebuf);
  }
  if(drawguys)
  {
    if(get_bit(QHeader.rules2,qr2_NOFLICKER) || (frame&1))
    {
      guys.draw(framebuf,true);
      chainlinks.draw(framebuf,true);
      Lwpns.draw(framebuf,true);
      Ewpns.draw(framebuf,true);
      items.draw(framebuf,true);
    }
    else
    {
      items.draw(framebuf,false);
      chainlinks.draw(framebuf,false);
      Lwpns.draw(framebuf,false);
      guys.draw(framebuf,false);
      Ewpns.draw(framebuf,false);
    }
    guys.draw2(framebuf,true);
  }
  if (!Link.isSwimming()) {
    Link.draw(framebuf);
  }

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->id == eLIKE)
      if(((eLikeLike*)guys.spr(i))->haslink)
        guys.spr(i)->draw(framebuf);
    if(guys.spr(i)->id == eWALLM)
      if(((eWallM*)guys.spr(i))->haslink)
        guys.spr(i)->draw(framebuf);
  }

  do_layer2(2, tmpscr, 0, 0);
  do_layer2(3, tmpscr, 0, 0);
  do_layer2(-1, tmpscr, 0, 0);
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id)) {
        guys.spr(i)->draw(framebuf);
    }
  }
  do_layer2(4, tmpscr, 0, 0);
  do_layer2(5, tmpscr, 0, 0);
  putsubscr(framebuf,0,0);

  putintro();

  if (dmapmsgclk>0) {
    Link.Freeze();
    if (dmapmsgclk<=50) {
      dmapmsgclk--;
    }
  }
  if (dmapmsgclk==1) {
    Link.unfreeze();
    dmapmsgclk=0;
    clear(msgdisplaybuf);
  }

  putmsg();
  domoney();
  domagic();

  masked_blit(msgdisplaybuf,framebuf,0,0,0,56,256,176);

  if(lensclk)
  {
    draw_lens_over();
    lensclk--;
  }


  advanceframe();
}



/**************************/
/********** Main **********/
/**************************/


char cfg_error[] =
"\"ag.cfg\" not found. Please run \"agsetup.exe\"";


int main(int argc, char* argv[])
{
 if (IS_BETA) {
   Z_title("Zelda Classic %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
 } else {
   Z_title("Zelda Classic %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
 }
//  Z_title("Zelda Classic %s",VerStr(ZELDA_VERSION));

  // check for the ag.cfg file and 1st.qst

  if(!exists("ag.cfg"))
    Z_error(cfg_error);

  if(!exists("1st.qst"))
    Z_error("\"1st.qst\" not found.");

  if(!exists("2nd.qst"))
    Z_error("\"2nd.qst\" not found.");

  if(!exists("3rd.qst"))
    Z_error("\"3rd.qst\" not found.");
  set_config_file("ag.cfg");


  // allocate quest data buffers

  printf("Allocating quest data buffers... ");
  qstpath = (char*)malloc(280);
  if(!qstpath || !get_qst_buffers())
    Z_error("Error");
  qstpath[0] = 0;
  printf("OK\n");


  // initialize Allegro

  printf("Initializing Allegro... ");
  three_finger_flag = FALSE;
  allegro_init();

  if(install_timer() < 0)
    Z_error(allegro_error);

  if(install_keyboard() < 0)
    Z_error(allegro_error);

  if(install_mouse() < 0)
    Z_error(allegro_error);

  if(install_joystick(JOY_TYPE_AUTODETECT) < 0)
    Z_error(allegro_error);

  Z_init_timers();
  printf("OK\n");


  // allocate bitmap buffers

  printf("Allocating bitmap buffers... ");
  framebuf  = create_bitmap(256,224);
  scrollbuf = create_bitmap(512,406);
  tmp_scr   = create_bitmap(320,240);
  tmp_bmp   = create_bitmap(32,32);
  fps_undo  = create_bitmap(64,16);
  msgdisplaybuf = create_bitmap(256, 176);
  overbuf = create_bitmap(256, 176);
  if(!framebuf || !scrollbuf || !tmp_bmp || !fps_undo || !tmp_scr
     ||!msgdisplaybuf || !overbuf)
    Z_error("Error");

  clear(scrollbuf);
  clear(framebuf);
  clear(msgdisplaybuf);
  clear(overbuf);
  printf("OK\n");


  // load game configurations

  load_game_configs();

  int mode = VidMode; // from config file
  int res_arg = used_switch(argc,argv,"-res");

  if(used_switch(argc,argv,"-v0")) Vsync=false;
  if(used_switch(argc,argv,"-v1")) Vsync=true;

  resolve_password(zeldapwd);
  debug = debug_enabled = used_switch(argc,argv,"-d")
    && !strcmp(get_config_string("zeldadx","debug",""),zeldapwd);

  bool fast_start = debug_enabled || used_switch(argc,argv,"-fast");


  // load the data file

  resolve_password(datapwd);
  packfile_password(datapwd);

  printf("Loading data file... ");
  set_color_conversion(COLORCONV_NONE);
  if((data=load_datafile("zelda.dat"))==NULL)
    Z_error("Error loading zelda.dat");

  if((zgpdata=load_datafile("zgp.dat"))==NULL)
    Z_error("Error loading zgp.dat");

  sprintf(sig_str,"Zelda Classic %s - Jeremy Craner, 1999",VerStr(DATA_VERSION));

  if(strncmp((char*)data[0].dat,sig_str,36))
    Z_error("Incompatible version of zelda.dat.\nPlease upgrade to %s",VerStr(DATA_VERSION));

  printf("OK\n");

  sfxdata = (DATAFILE*)data[WAV].dat;
  mididata = (DATAFILE*)data[MUSIC].dat;

  wav_refill = *((SAMPLE*)sfxdata[WAV_REFILL].dat);
  wav_refill.loop_start = 256;
  wav_refill.loop_end = 2048;

  deffont = font;
  font = (FONT*)data[FONT_GUI].dat;
  zfont = (FONT*)data[FONT_NES].dat;
  mfont = (FONT*)data[FONT_MATRIX].dat;
  text_mode(0);


  // load saved games

  printf("Loading saved games... ");
  if(load_savedgames()!=0)
    Z_error("Insufficient memory");

  printf("OK\n");

  // initialize sound driver

  printf("Initializing sound driver... ");
  if(used_switch(argc,argv,"-s") || used_switch(argc,argv,"-nosound"))
    printf("skipped\n");
  else
  {
    if(install_sound(DIGI_AUTODETECT,DIGI_AUTODETECT,NULL)) {
//      Z_error(allegro_error);
      printf("Sound driver not available.  Sound disabled.\n");
    } else {
      printf("OK\n");
    }
    printf("OK\n");
  }

  Z_init_sound();

  // CD player

  /*
  if(used_switch(argc,argv,"-cd"))
  {
    printf("Initializing CD player... ");
    if(cd_init())
      Z_error("Error");
    printf("OK\n");
    useCD = true;
  }
  */

  // quick quit

  if(used_switch(argc,argv,"-q"))
  {
    printf("-q switch used, quitting program.\n");
    goto quick_quit;
  }

  // set video mode

  if(res_arg && argc>res_arg+2)
  {
    resx = atoi(argv[res_arg+1]);
    resy = atoi(argv[res_arg+2]);
    sbig = stricmp(argv[res_arg+3],"big")==0;
    mode = GFX_AUTODETECT;
  }

  if(used_switch(argc,argv,"-modex"))  mode=GFX_MODEX;
  if(used_switch(argc,argv,"-vesa1"))  mode=GFX_VESA1;
  if(used_switch(argc,argv,"-vesa2b")) mode=GFX_VESA2B;
  if(used_switch(argc,argv,"-vesa2l")) mode=GFX_VESA2L;
  if(used_switch(argc,argv,"-vesa3"))  mode=GFX_VESA3;

  if(!game_vid_mode(mode,250))
    Z_error(allegro_error);

  fix_dialogs();
  gui_mouse_focus = FALSE;
  position_mouse(resx-16,resy-16);

  // AG logo

  if(!fast_start)
  {
    set_volume(240,-1);
    aglogo(tmp_scr, scrollbuf, resx, resy);
    master_volume(digi_volume,midi_volume);
  }

  // play the game

  fix_menu();

  Quit = fast_start ? qQUIT : qRESET;

  while(Quit!=qEXIT)
  {
    cheat = 0;
    toogam = false;
    titlescreen();
    cheat = 0;
    setup_combo_animations();
    while(!Quit)
      game_loop();
    Playing=Paused=false;
    switch(Quit)
    {
    case qQUIT:
    case qGAMEOVER: game_over(); break;
    case qWON:      ending(); break;
    }
    kill_sfx();
    stop_midi();
    clear_to_color(screen,BLACK);
  }

  // clean up

  stop_midi();
  kill_sfx();
  set_gfx_mode(GFX_TEXT,80,25,0,0);
  rest(250);

quick_quit:

  save_savedgames();
  save_game_configs();
//  if(useCD)
//    cd_exit();
  unload_datafile(data);
  if(mappic)
    destroy_bitmap(mappic);
  destroy_bitmap(framebuf);
  destroy_bitmap(scrollbuf);
  destroy_bitmap(tmp_scr);
  destroy_bitmap(tmp_bmp);
  destroy_bitmap(fps_undo);
  allegro_exit();
  printf("Armageddon Games web site: http://www.armageddongames.com\n");
  printf("Zelda Classic web site: http://www.zeldaclassic.com\n");
  exit(0);
}

/*** end of zelda.cc ***/

