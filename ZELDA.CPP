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
//#include <dir.h>
#include <ctype.h>

#include "zc_alleg.h"
#include <png.h>
#include <pngconf.h>
#include <loadpng.h>
#include <jpgalleg.h>

#include "zcmusic.h"
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
#include "save_gif.h"
#include "load_gif.h" // not really needed; we're just saving GIF files in ZC.
#include "fontsdat.h"
#include "particles.h"
#include "gamedata.h"
#include "ffscript.h"
#include "zc_init.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif


ZCMUSIC *zcmusic = NULL;
zinitdata zinit;
int colordepth;
int db=0;
//zinitdata  zinit;
int detail_int[10];                                         //temporary holder for things you want to detail
int lens_hint_item[MAXITEMS][2];                            //aclk, aframe
int lens_hint_weapon[MAXWPNS][5];                           //aclk, aframe, dir, x, y
int strike_hint_counter=0;
int strike_hint_timer=0;
int strike_hint;
int slot_arg, slot_arg2;
char *SAVE_FILE = "zc.sav";

int passive_subscreen_height=56;
int playing_field_offset=56;
int passive_subscreen_offset=0;

void playLevelMusic();

volatile int logic_counter=0;
bool drawit=false;
bool trip=false;
void update_logic_counter()
{
  ++logic_counter;
} END_OF_FUNCTION(update_logic_counter)

  int onHelp()
{
  //  restore_mouse();
  //  doHelp(vc(15),vc(0));
  return D_O_K;
}
int d_dropdmaplist_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  msg=msg;
  d=d;
  c=c;

  return D_O_K;
}

char *dmaplist(int index, int *list_size)
{
  //these are here to bypass compiler warnings about unused arguments
  index=index;
  list_size=list_size;

  return NULL;
}
int startdmapxy[6] = {0,0,0,0,0,0};

/**********************************/
/******** Global Variables ********/
/**********************************/

int curr_tb_page=0;
bool triplebuffer_not_available=false;

RGB_MAP rgb_table;
COLOR_MAP trans_table;

BITMAP     *framebuf, *scrollbuf, *tmp_bmp, *tmp_scr, *screen2, *fps_undo, *msgdisplaybuf, *pricesdisplaybuf, *tb_page[3], *real_screen, *temp_buf, *temp_buf2;
DATAFILE   *data, *sfxdata, *fontsdata, *mididata;
FONT       *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gbzfont,
           *goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font;
PALETTE    RAMpal;
byte       *colordata, *trashbuf;
//byte       *tilebuf;
tiledata   *newtilebuf, *grabtilebuf;
newcombo   *combobuf;
itemdata   *itemsbuf;
wpndata    *wpnsbuf;
comboclass *combo_class_buf;
guydata    *guysbuf;
ZCHEATS    zcheats;
byte       use_cheats;
byte       use_tiles;
char       palnames[MAXLEVELS][PALNAMESIZE];
word animated_combo_table[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table4[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos;
word animated_combo_table2[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table24[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos2;
bool blank_tile_table[NEWMAXTILES];                         //keeps track of blank tiles
bool blank_tile_quarters_table[NEWMAXTILES*4];              //keeps track of blank tiles
bool ewind_restart=false;
word     msgclk, msgstr, msgpos, msg_count;
word     door_combo_set_count;
word     introclk, intropos, dmapmsgclk, linkedmsgclk;
short    lensclk, lenscnt;
int    Bpos;
byte screengrid[22];
bool halt=false;
bool screenscrolling=false;
bool close_button_quit=false;
PALETTE tempbombpal;
bool usebombpal;

int readsize, writesize;

combo_alias combo_aliases[MAXCOMBOALIASES];  //Temporarily here to ZC can compile. All memory from this is freed after loading the quest file.

SAMPLE customsfxdata[WAV_COUNT];
int sfxdat=1;

int jwin_pal[jcMAX];
int gui_colorset=0;

int homescr,currscr,frame=0,currmap=0,dlevel,warpscr,worldscr;
int newscr_clk=0,opendoors=0,currdmap=0,fadeclk=-1,currgame=0,listpos=0;
int lastentrance=0,lastentrance_dmap=0,prices[3][2],loadside, Bwpn, Awpn;
int digi_volume,midi_volume,currmidi,wand_x,wand_y,wand_z,hasitem,whistleclk,pan_style;
int Akey,Bkey,Skey,Lkey,Rkey,Pkey,Abtn,Bbtn,Sbtn,Mbtn,Lbtn,Rbtn,Pbtn,Quit=0;
int DUkey, DDkey, DLkey, DRkey, ss_after, ss_speed, ss_density;
int arrow_x, arrow_y, arrow_z, brang_x, brang_y, brang_z;
int hs_startx, hs_starty, hs_xdist, hs_ydist, clockclk, clock_zoras;
int cheat_goto_dmap=0, cheat_goto_screen=0, swordhearts[4], currcset;
int gfc, gfc2, pitx, pity, refill_what, refill_why, heart_beep_timer=0, new_enemy_tile_start=1580;
int nets=1580, magictype=mgc_none, magiccastclk, castx, casty, df_x, df_y, nl1_x, nl1_y, nl2_x, nl2_y;
int magicdrainclk=0, conveyclk=3, memrequested=0;
float avgfps=0;
dword fps_secs=0;
bool do_cheat_goto=false;
int checkx, checky;
int loadlast=0;
int skipcont=0;
int skipicon=0;

bool show_layer_0=true, show_layer_1=true, show_layer_2=true, show_layer_3=true, show_layer_4=true, show_layer_5=true, show_layer_6=true, show_layer_over=true, show_layer_push=true, show_sprites=true;


bool nosecretsounds=false;

bool Throttlefps, Paused=false, Advance=false, ShowFPS, Showpal=false;
bool Playing, FrameSkip=false, TransLayers;
bool __debug=false,debug_enabled;
bool refreshpal,blockpath,wand_dead,loaded_guys,freeze_guys,
  loaded_enemies,drawguys,details=false,DXtitle,watch;
bool darkroom=false,BSZ,COOLSCROLL;                         //,NEWSUBSCR;
bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,Mdown,LBdown,RBdown,Pdown,
  SystemKeys=true,NESquit,volkeys,useCD=false,boughtsomething=false,
  fixed_door=false, hookshot_used=false, hookshot_frozen=false,
  pull_link=false, add_chainlink=false, del_chainlink=false, hs_fix=false,
  cheat_superman=false, gofast=false, checklink=true, didpit=false, heart_beep=true,
  pausenow=false, castnext=false, add_df1asparkle, add_df1bsparkle, add_nl1asparkle, add_nl1bsparkle, add_nl2asparkle, add_nl2bsparkle,
  is_on_conveyor, activated_timed_warp=false;

int  add_asparkle=0, add_bsparkle=0;
int SnapshotFormat;

char   zeldadat_sig[52];
char   sfxdat_sig[52];
char   fontsdat_sig[52];
char   cheat_goto_dmap_str[4];
char   cheat_goto_screen_str[3];
short  visited[6];
byte   guygrid[176];
mapscr tmpscr[2];
mapscr tmpscr2[6];
mapscr tmpscr3[6];
gamedata *game=NULL;
ffscript *ffscripts[512];
ffscript *itemscripts[256];
ffscript *guyscripts[256];
ffscript *wpnscripts[256];
ffscript *globalscripts[3];
ffscript *linkscripts[3];
ffscript *screenscripts[256];

extern byte global_ffc;
extern byte global_item;
extern byte global_guy;
extern byte global_lwpn;
extern byte global_ewpn;
extern byte global_scr;
extern byte global_itemclass;
extern byte global_lwpnclass;
extern byte global_ewpnclass;
extern byte global_guyclass;

extern long g_d[8];
extern dword g_scriptflag;
extern long g_stack[256];
extern byte g_sp;
extern word g_pc;
extern word g_doscript;

//movingblock mblock2; //mblock[4]?
//LinkClass   Link;

int resx,resy,scrx,scry;
bool sbig;                                                  // big screen
bool scanlines;                                             //do scanlines if sbig==1
bool toogam=false;

int cheat=0;                                                // 0 = none; 1,2,3,4 = cheat level

int mouse_down;                                             // used to hold the last reading of 'gui_mouse_b()' status
int idle_count, active_count;


// quest file data
zquestheader QHeader;
byte         quest_rules[QUESTRULES_SIZE];
byte         midi_flags[MIDIFLAGS_SIZE];
byte         music_flags[MUSICFLAGS_SIZE];
word         map_count;
MsgStr       *MsgStrings;
DoorComboSet *DoorComboSets;
dmap         *DMaps;
miscQdata    QMisc;
mapscr       *TheMaps;
byte	     *quest_file;
dword		 quest_map_pos[MAPSCRS*MAXMAPS2];
long		ffstack[32][256];

char     *qstpath=NULL;
char     *qstdir=NULL;
gamedata *saves=NULL;

volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync=0;

/**********************************/
/*********** Misc Data ************/
/**********************************/

const char startguy[8] = {-13,-13,-13,-14,-15,-18,-21,-40};
const char gambledat[12*6] =
{
  20,-10,-10, 20,-10,-10, 20,-40,-10, 20,-10,-40,
  50,-10,-10, 50,-10,-10, 50,-40,-10, 50,-10,-40,
  -10,20,-10, -10,20,-10, -40,20,-10, -10,20,-40,
  -10,50,-10, -10,50,-10, -40,50,-10, -10,50,-40,
  -10,-10,20, -10,-10,20, -10,-40,20, -40,-10,20,
  -10,-10,50, -10,-10,50, -10,-40,50, -40,-10,50
};
const byte stx[4][9] = {
  { 48, 80, 80, 96,112,144,160,160,192},
  { 48, 80, 80, 96,128,144,160,160,192},
  { 80, 80,128,128,160,160,192,192,208},
  { 32, 48, 48, 80, 80,112,112,160,160}
};
const byte sty[4][9] = {
  {112, 64,128, 96, 80, 96, 64,128,112},
  { 48, 32, 96, 64, 80, 64, 32, 96, 48},
  { 32,128, 64, 96, 64, 96, 48,112, 80},
  { 80, 48,112, 64, 96, 64, 96, 32,128}
};

const byte ten_rupies_x[10] = {120,112,128,96,112,128,144,112,128,120};
const byte ten_rupies_y[10] = {49,65,65,81,81,81,81,97,97,113};

zcmidi tunes[MAXMIDIS] =
{
  // (title)                (s) (ls) (le) (l) (vol) (midi)
  { "Zelda 1 - Dungeon",     0,  -1,  -1,  1,  176,  NULL },
  { "Zelda 1 - Ending",      0, 129, 225,  1,  160,  NULL },
  { "Zelda 1 - Game Over",   0,  -1,  -1,  1,  224,  NULL },
  { "Zelda 1 - Level 9",     0,  -1,  -1,  1,  255,  NULL },
  { "Zelda 1 - Overworld",   0,  17,  -1,  1,  208,  NULL },
  { "Zelda 1 - Title",       0,  -1,  -1,  0,  168,  NULL },
  { "Zelda 1 - Triforce",    0,  -1,  -1,  0,  168,  NULL },
};

emusic enhancedMusic[MAXMUSIC];

void dointro ()
{
  if (game->visited[currdmap]!=1)
  {
    dmapmsgclk=0;
    game->visited[currdmap]=1;
    introclk=intropos=0;
  }
}

bool bad_version(int version)
{
  // minimum zquest version allowed for any quest file
  if(version < 0x183)
    return true;

  return false;
}

extern char *weapon_string[];
extern char *item_string[];


/**********************************/
/******* Other Source Files *******/
/**********************************/

bool get_debug()
{
  //return false; //remove later
  return __debug;
}

void set_debug(bool d)
{
  __debug=d;
  return;
}


void hit_close_button()
{
  close_button_quit=true;
  return;
}


bool blockmoving;
#include "sprite.h"
movingblock mblock2;                                        //mblock[4]?

sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;

#include "zc_custom.h"
#include "link.h"
LinkClass   Link;

#include "maps.h"
#include "subscr.h"
#include "guys.h"

#include "title.h"
#include "ending.h"

#include "zc_sys.h"

void addLwpn(int x,int y,int z,int id,int type,int power,int dir)
{
  Lwpns.add(new weapon((fix)x,(fix)y,(fix)z,id,type,power,dir));
}

void ALLOFF()
{
  clear_bitmap(msgdisplaybuf);
  set_clip_state(msgdisplaybuf, 1);
  clear_bitmap(pricesdisplaybuf);
  set_clip_state(pricesdisplaybuf, 1);
  if(items.idCount(iPile))
  {
    loadlvlpal(DMaps[currdmap].color);
  }
  items.clear();
  guys.clear();
  Lwpns.clear();
  Ewpns.clear();
  chainlinks.clear();
  decorations.clear();
  if (Link.getNayrusLoveShieldClk())
  {
    Link.setNayrusLoveShieldClk(Link.getNayrusLoveShieldClk());
  }
  Link.resetflags(false);
  Link.reset_hookshot();
  linkedmsgclk=0;
  add_asparkle=0;
  add_bsparkle=0;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  add_nl1asparkle=false;
  add_nl1bsparkle=false;
  add_nl2asparkle=false;
  add_nl2bsparkle=false;
  //  for(int i=0; i<1; i++)
  mblock2.clk=0;
  msgstr=0;
  fadeclk=-1;

  lensclk = lenscnt = 0;
  drawguys=Udown=Ddown=Ldown=Rdown=Adown=Bdown=Sdown=true;
  if (watch && !cheat_superman)
  {
    Link.setClock(false);
  }
  //  if(watch)
  //    Link.setClock(false);
  watch=freeze_guys=loaded_guys=loaded_enemies=wand_dead=blockpath=false;
  stop_sfx(WAV_BRANG);
  for(int i=0; i<176; i++)
    guygrid[i]=0;
  sle_clk=0;
  blockmoving=false;
  fairy_cnt=0;
  if (usebombpal)
  {
    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
    refreshpal=true;
    usebombpal=false;
  }
}
void centerLink() {Link.setX(120);Link.setY(80);}
fix  LinkX()   { return Link.getX(); }
fix  LinkY()   { return Link.getY(); }
fix  LinkZ()   { return Link.getZ(); }
int  LinkHClk() { return Link.getHClk(); }
int  LinkNayrusLoveShieldClk() { return Link.getNayrusLoveShieldClk(); }
int  LinkHoverClk() { return Link.getHoverClk(); }
int  LinkLStep() { return Link.getLStep(); }
fix  GuyX(int j)   { return guys.getX(j); }
fix  GuyY(int j)   { return guys.getY(j); }
int  GuyID(int j)   { return guys.getID(j); }
int  GuyMisc(int j)   { return guys.getMisc(j); }
bool  GuySuperman(int j)
{
  if ((j>=guys.Count())||(j<0))
  {
    return true;
  }
  return ((enemy*)guys.spr(j))->superman !=0;
}

int  GuyCount()   { return guys.Count(); }
void StunGuy(int j)   { ((enemy*)guys.spr(j))->stunclk=160; }

fix LinkModifiedX()   { return Link.getModifiedX(); }
fix LinkModifiedY()   { return Link.getModifiedY(); }
int LinkDir() { return Link.getDir(); }
void add_grenade(int wx, int wy, int wz, int size)
{
  if (size)
  {
    Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wSBomb,0,16*DAMAGE_MULTIPLIER,LinkDir()));
    Lwpns.spr(Lwpns.Count()-1)->id=wSBomb;
  }
  else
  {
    Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wBomb,0,4*DAMAGE_MULTIPLIER,LinkDir()));
    Lwpns.spr(Lwpns.Count()-1)->id=wBomb;
  }
  Lwpns.spr(Lwpns.Count()-1)->clk=41;
}

fix distance(int x1, int y1, int x2, int y2)

{
  return (fix)sqrt(pow((double)abs(x1-x2),2)+pow((double)abs(y1-y2),2));
}

bool getClock() { return Link.getClock(); }
void setClock(bool state) { Link.setClock(state); }
void CatchBrang() { Link.Catch(); }

/**************************/
/***** Main Game Code *****/
/**************************/

int load_quest(gamedata *g, bool report)
{
  //  packfile_password(datapwd);
  chop_path(qstpath);
  char *tempdir="";
#ifndef ALLEGRO_MACOSX
  tempdir=qstdir;
#endif

  if(get_gamedata_quest(g)<255)
  {
    sprintf(qstpath,"%s%s.qst",tempdir,ordinal(get_gamedata_quest(g)));
  }
  else
  {
    if (is_relative_filename(g->qstpath))
    {
      sprintf(qstpath,"%s%s",qstdir,g->qstpath);
    }
    else
    {
      sprintf(qstpath,"%s", g->qstpath);
    }
  }

  packfile_password(datapwd);
  byte skip_flags[4];
  for (int i=0; i<4; ++i)
  {
    skip_flags[i]=0;
  }
  int ret = loadquest(qstpath,&QHeader,&QMisc,tunes+ZC_MIDI_COUNT,false,true,true,true,skip_flags);
  packfile_password(NULL);

  /*
    for(int j=0;j<MAXCOMBOALIASES;j++) {
    if(combo_aliases[j].combos != NULL)
    {
    delete[] combo_aliases[j].combos;
    }
    if(combo_aliases[j].csets != NULL)
    {
    delete[] combo_aliases[j].csets;
    }
    }
    */

  if(!g->title[0] || !get_gamedata_hasplayed(g))
  {
    strcpy(g->version,QHeader.version);
    strcpy(g->title,QHeader.title);
  }
  else
  {
    if(!ret && strcmp(g->title,QHeader.title))
    {
      ret = qe_match;
    }
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
    jwin_alert("File error",buf1,buf2,qstpath,"OK",NULL,13,27,lfont);
    game_pal();
  }

  //  packfile_password(NULL);
  return ret;
}

int init_game()
{
  //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
  didpit=false;
  Link.unfreeze();
  Link.reset_hookshot();
  Link.reset_ladder();
  linkedmsgclk=0;
  blockmoving=false;
  add_asparkle=0;
  add_bsparkle=0;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  add_nl1asparkle=false;
  add_nl1bsparkle=false;
  add_nl2asparkle=false;
  add_nl2bsparkle=false;
  gofast=false;
  srand(time(0));

  // confuse the cheaters by moving the game data to a random location
  if (game!=NULL)
  {
    free(game);
  }
  char *dummy=(char *)malloc(rand()%64000);
  game=(gamedata *)malloc(sizeof(gamedata));
  //  game=&realgame;
  free(dummy);


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

  // copy saved data to RAM data
  //  realgame = saves[currgame];
  //  memcpy(&realgame,saves+currgame,sizeof(gamedata));
  memcpy(game,saves+currgame,sizeof(gamedata));

  packfile_password(datapwd);
  if(load_quest(game))
  {
    Quit=qERROR;
    packfile_password(NULL);
    return 1;
  }
  packfile_password(NULL);

  cheat=0;

  char keyfilename[256];
  replace_extension(keyfilename, qstpath, "key", 255);
  bool gotfromkey=false;
  if (exists(keyfilename))
  {
    char password[32], pwd[32];
    PACKFILE *fp = pack_fopen(keyfilename, F_READ);
    char msg[80];
    memset(msg,0,80);
    pfread(msg, 80, fp,true);
    if (strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
    {
      short ver;
      byte  bld;
      p_igetw(&ver,fp,true);
      p_getc(&bld,fp,true);
      memset(password,0,32);
      pfread(password, 30, fp,true);
      /*
        get-questpwd(&QHeader, pwd);
        if (strcmp(pwd,password)==0)
        {
        gotfromkey=true;
        }
        */
      gotfromkey=check_questpwd(&QHeader, password);
      memset(password,0,32);
      memset(pwd,0,32);
    }
    pack_fclose(fp);
  }

  if (gotfromkey)
  {
    cheat=4;
  }

  BSZ = get_bit(quest_rules,qr_BSZELDA)!=0;
  //setuplinktiles(zinit.linkanimationstyle);
  COOLSCROLL = get_bit(quest_rules,qr_COOLSCROLL)!=0;
  //  NEWSUBSCR = get_bit(quest_rules,qr_NEWSUBSCR);

  //  homescr = currscr = DMaps[0].cont;
  //  currdmap = warpscr = worldscr=0;
  if(!get_gamedata_hasplayed(game))
  {
    set_gamedata_continue_dmap(game, zinit.start_dmap);
  }
  currdmap = warpscr = worldscr=get_gamedata_continue_dmap(game);


  if (get_gamedata_continue_scrn(game) >= 0x80)
  {
    //if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW || QHeader.zelda_version <= 0x190)
    if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW)
    {
      homescr = currscr = DMaps[currdmap].cont;
    }
    else
    {
      homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
    }
  }
  else
  {
    homescr = currscr = get_gamedata_continue_scrn(game);
  }
  lastentrance = currscr;
  lastentrance_dmap = currdmap;
  currmap = DMaps[currdmap].map;
  dlevel = DMaps[currdmap].level;
  sle_x=sle_y=newscr_clk=opendoors=Bwpn=Bpos=0;
  activated_timed_warp=false;
  fadeclk=-1;
  game->maps[(currmap<<7)+currscr] |= mVISITED;              // mark as visited

  for(int i=0; i<6; i++)
  {
    visited[i]=-1;
  }
  game->lvlitems[9]&=~liBOSS;

  ALLOFF();
  whistleclk=-1;
  clockclk=0;
  currcset=DMaps[currdmap].color;
  darkroom=false;
  memset(tmpscr,0,sizeof(mapscr)*2);
//loadscr(0,currscr,up);
  loadscr(0,currscr,-1);
  putscr(scrollbuf,0,0,&tmpscr[0]);
  Link.init();
  Link.resetflags(true);

  copy_pal((RGB*)data[PAL_GUI].dat,RAMpal);
  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);

  if (!get_gamedata_hasplayed(game))
  {
		resetItems(game,&zinit);
  }

  for (int x=0; x<4; x++)
  {
    swordhearts[x]=zinit.sword_hearts[x];
  }


  if (!get_gamedata_hasplayed(game))
  {
    set_gamedata_life(game, zinit.start_heart*HP_PER_HEART);
  }
  else
  {
    if(get_bit(zinit.misc,idM_CONTPERCENT))
    {
      set_gamedata_life(game, ((get_gamedata_maxlife(game)*zinit.cont_heart/100)/HP_PER_HEART)*HP_PER_HEART);
    }
    else
    {
      set_gamedata_life(game, zinit.cont_heart*HP_PER_HEART);
    }
  }
  set_gamedata_hasplayed(game, 1);

  if(get_bit(quest_rules,qr_CONTFULL))
    set_gamedata_life(game, get_gamedata_maxlife(game));
  /*
    else
    game->life=3*HP_PER_HEART;
    */

  //int type = (DMaps[currdmap].type&dmfTYPE);

  update_subscreens();

  load_Sitems(&QMisc);
  selectBwpn(0,0);
  selectAwpn(0);
  reset_subscr_items();

  Link.setDontDraw(false);
  show_subscreen_dmap_dots=true;
  show_subscreen_items=true;
  show_subscreen_numbers=true;
  show_subscreen_life=true;

  //  for(int i=0; i<128; i++)
  //    key[i]=0;


  Playing=true;
  lighting(2,Link.getDir());

  map_bkgsfx();
  openscreen();
  show_subscreen_numbers=true;
  show_subscreen_life=true;
  dointro();
  loadguys();

  if(isdungeon() && currdmap>0)
  {
    Link.stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
  }

  if(!Quit)
    //play_DmapMusic();
    playLevelMusic();

  for(int i=0;i<8;i++)
  {
    g_d[i]=0;
  }
  for(int i=0;i<256;i++)
  {
    g_stack[i]=0;
  }
  g_scriptflag=0;
  g_pc=0;
  g_sp=0;
  g_doscript=1;
  global_itemclass=0;
  global_ewpnclass=0;
  global_lwpnclass=0;
  global_guyclass=0;
  global_ffc=0;
  global_item=0;
  global_lwpn=0;
  global_ewpn=0;
  global_guy=0;
  run_script(0, 0, SCRIPT_GLOBAL);
  for(int i=0;i<8;i++)
  {
    g_d[i]=0;
  }
  for(int i=0;i<256;i++)
  {
    g_stack[i]=0;
  }
  g_scriptflag=0;
  g_pc=0;
  g_sp=0;
  g_doscript=1;
  global_itemclass=0;
  global_ewpnclass=0;
  global_lwpnclass=0;
  global_guyclass=0;
  global_ffc=0;
  global_item=0;
  global_lwpn=0;
  global_ewpn=0;
  global_guy=0;

  return 0;
}

int cont_game()
{
  //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
  didpit=false;
  Link.unfreeze();
  Link.reset_hookshot();
  Link.reset_ladder();
  linkedmsgclk=0;
  blockmoving=0;
  add_asparkle=0;
  add_bsparkle=0;
  add_df1asparkle=false;
  add_df1bsparkle=false;
  add_nl1asparkle=false;
  add_nl1bsparkle=false;
  add_nl2asparkle=false;
  add_nl2bsparkle=false;
  /*
    if(DMaps[currdmap].cont >= 0x80)
    {
    homescr = currscr = DMaps[0].cont;
    currdmap = warpscr = worldscr=0;
    currmap = DMaps[0].map;
    dlevel = DMaps[0].level;
    }
    */
  currdmap = lastentrance_dmap;
  homescr = currscr = lastentrance;
  currmap = DMaps[currdmap].map;
  dlevel = DMaps[currdmap].level;

  for(int i=0; i<6; i++)
  {
    visited[i]=-1;
  }
  if(dlevel==0)
  {
    game->lvlitems[9]&=~liBOSS;
  }

  ALLOFF();
  whistleclk=-1;
  currcset=DMaps[currdmap].color;
  darkroom=false;
  memset(tmpscr,0,sizeof(mapscr)*2);
//loadscr(0,currscr,up);
  loadscr(0,currscr,-1);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);

  Link.init();

  if(get_bit(zinit.misc,idM_CONTPERCENT))
  {
    set_gamedata_life(game, ((get_gamedata_maxlife(game)*zinit.cont_heart/100)/HP_PER_HEART)*HP_PER_HEART);
  }
  else
  {
    set_gamedata_life(game, zinit.cont_heart*HP_PER_HEART);
  }

  if(get_bit(quest_rules,qr_CONTFULL))
    set_gamedata_life(game, get_gamedata_maxlife(game));
  /*
    else
    game->life=3*HP_PER_HEART;
    */

  //  for(int i=0; i<128; i++)
  //    key[i]=0;

  update_subscreens();
  Playing=true;
  lighting(2,Link.getDir());
  map_bkgsfx();
  openscreen();
  show_subscreen_numbers=true;
  show_subscreen_life=true;
  loadguys();

  if(!Quit)
  {
    //play_DmapMusic();
    playLevelMusic();
    if(isdungeon())
      Link.stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
    newscr_clk=frame;
    activated_timed_warp=false;
  }
  for(int i=0;i<8;i++)
  {
    g_d[i]=0;
  }
  for(int i=0;i<256;i++)
  {
    g_stack[i]=0;
  }
  g_scriptflag=0;
  g_pc=0;
  g_sp=0;
  g_doscript=1;
  global_itemclass=0;
  global_ewpnclass=0;
  global_lwpnclass=0;
  global_guyclass=0;
  global_ffc=0;
  global_item=0;
  global_lwpn=0;
  global_ewpn=0;
  global_guy=0;
  return 0;
}

void restart_level()
{
  blackscr(16,true);
  if(dlevel)
  {
    currdmap = lastentrance_dmap;
    homescr = currscr = lastentrance;
  }
  else
  {
    if ((DMaps[currdmap].type&dmfTYPE)==dmOVERW)
    {
      homescr = currscr = DMaps[currdmap].cont;
    }
    else
    {
      homescr = currscr = DMaps[currdmap].cont + DMaps[currdmap].xoff;
    }
  }

  currmap = DMaps[currdmap].map;
  dlevel = DMaps[currdmap].level;
  for(int i=0; i<6; i++)
    visited[i]=-1;

  ALLOFF();
  whistleclk=-1;
  darkroom=false;
  memset(tmpscr,0,sizeof(mapscr)*2);
//loadscr(0,currscr,up);
  loadscr(0,currscr,-1);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
  Link.init();
  lighting(2,Link.getDir());
//  map_bkgsfx();
  openscreen();
  map_bkgsfx();

  show_subscreen_numbers=true;
  show_subscreen_life=true;
  loadguys();

  if(!Quit)
  {
    //play_DmapMusic();
    playLevelMusic();
    if(isdungeon())
      Link.stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
    newscr_clk=frame;
    activated_timed_warp=false;
  }
}


void putintro()
{
  if (!stricmp("                                                                        ", DMaps[currdmap].intro))
  {
    introclk=intropos=72;
    return;
  }

  if(intropos>=72)
    return;

  if((cBbtn())&&(get_bit(quest_rules,qr_ALLOWMSGBYPASS)))
  {
    //finish writing out the string
    for (;intropos<72;++intropos)
    {
      textprintf_ex(msgdisplaybuf,zfont,((intropos%24)<<3)+32,((intropos/24)<<3)+40,CSET(0)+1,-1,
                    "%c",DMaps[currdmap].intro[intropos]);
    }
  }

  if(((introclk++)%6<5)&&((!cAbtn())||(!get_bit(quest_rules,qr_ALLOWFASTMSG))))
    return;

  dmapmsgclk=51;
  if(intropos == 0)
  {
    while(DMaps[currdmap].intro[intropos]==' ')
      ++intropos;
  }

  sfx(WAV_MSG);


  //using the clip value to indicate the bitmap is "dirty"
  //rather than add yet another global variable
  set_clip_state(msgdisplaybuf, 0);
  textprintf_ex(msgdisplaybuf,zfont,((intropos%24)<<3)+32,((intropos/24)<<3)+40,CSET(0)+1,-1,
                "%c",DMaps[currdmap].intro[intropos]);

  ++intropos;

  if(DMaps[currdmap].intro[intropos]==' ' && DMaps[currdmap].intro[intropos+1]==' ')
    while(DMaps[currdmap].intro[intropos]==' ')
      ++intropos;

  if(intropos>=72)
  {
    //   Link.unfreeze();
    dmapmsgclk=50;
  }
}

//static char *dirstr[4] = {"Up","Down","Left","Right"};
//static char *dirstr[32] = {"U","D","L","R"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "};

//use detail_int[x] for global detail info
void show_details()
{
  if(details)
  {
    textprintf_ex(framebuf,font,-3,-5,WHITE,BLACK,"%-4d",whistleclk);
    textprintf_ex(framebuf,font,0,8,WHITE,BLACK,"dlvl:%-2d dngn:%d", dlevel, isdungeon());
    textprintf_ex(framebuf,font,0,176,WHITE,BLACK,"%ld %s",get_gamedata_time(game),time_str_long(get_gamedata_time(game)));

//    for(int i=0; i<guys.Count(); i++)
//    for(int i=0; i<Ewpns.Count(); i++)
//    for(int i=0; i<items.Count(); i++)
    {
//      textprintf_ex(framebuf,font,200,(i<<3)+16,WHITE,BLACK,"%3d",items.spr(i)->id);
//      textprintf_ex(framebuf,font,160,(i<<3)+16,WHITE,BLACK,"%3d %3d %3d %3d",((weapon*)Ewpns.spr(i))->id, ((weapon*)Ewpns.spr(i))->tile, ((weapon*)Ewpns.spr(i))->aframe, wpnsbuf[((weapon*)Ewpns.spr(i))->id].frames);
//      textprintf_ex(framebuf,font,100,(i<<3)+16,WHITE,BLACK,"%3d %3d %3d %3d %3d %3d",(int)((enemy*)guys.spr(i))->x, (int)((enemy*)guys.spr(i))->y, ((enemy*)guys.spr(i))->hxofs, ((enemy*)guys.spr(i))->hyofs, ((enemy*)guys.spr(i))->hxsz, ((enemy*)guys.spr(i))->hysz);//,dirstr[((enemy*)guys.spr(i))->dir]);//, dirstr[((enemy*)guys.spr(i))->clk3]);
    }

    /*
      for(int i=0; i<Ewpns.Count(); i++)
      {
      sprite *s=Ewpns.spr(i);
      textprintf_ex(framebuf,font,100,(i<<3)+16,WHITE,BLACK,"%3d>%3d %3d>%3d %3d<%3d %3d<%3d ",
      int(Link.getX()+0+16), int(s->x+s->hxofs),  int(Link.getY()+0+16), int(s->y+s->hyofs),
      int(Link.getX()+0), int(s->x+s->hxofs+s->hxsz), int(Link.getY()+0), int(s->y+s->hyofs+s->hysz));
      }
      */
    //    textprintf_ex(framebuf,font,200,16,WHITE,BLACK,"yofs=%3d",detail_int[0]);
  }
}

void do_magic_casting()
{
  static int tempx, tempy;
  static byte linktilebuf[256];
  int ltile=0; int lflip=0;
  bool shieldModify=true;
  switch (magictype)
  {
    case mgc_none:
    magiccastclk=0;
    break;
    case mgc_dinsfire:
    {
      int flamemax=32;
      if (magiccastclk==0)
      {
        Lwpns.add(new weapon(LinkX(),LinkY(),LinkZ(),wPhantom,pDINSFIREROCKET,0,up));
        weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        //          Link.tile=(BSZ)?32:29;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        casty=Link.getY();
      }
      if (magiccastclk==64)
      {
        Lwpns.add(new weapon((fix)LinkX(),(fix)(-32),(fix)LinkZ(),wPhantom,pDINSFIREROCKETRETURN,0,down));
        weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        //          Link.tile=29;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        castnext=false;
      }
      if (castnext)
      {
        //          Link.tile=4;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        for (int flamecounter=((-1)*(flamemax/2))+1; flamecounter<((flamemax/2)+1); flamecounter++)
        {
          Lwpns.add(new weapon((fix)LinkX(),(fix)LinkY(),(fix)LinkZ(),wFire,3,8*DAMAGE_MULTIPLIER,0));
          weapon *w = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
          w->step=2;
          w->angular=true;
          w->angle=(flamecounter*PI/(flamemax/2));
        }
        castnext=false;
        magiccastclk=128;
      }

      /*
        */
      if ((magiccastclk++)==226)
      {
        magictype=mgc_none;
      }
    }
    break;
    case mgc_faroreswind:
    {
      if (magiccastclk==0)
      {
        linktile(&ltile, &lflip, ls_stab, down, zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          ltile+=item_tile_mod(shieldModify);
        }
        unpack_tile(newtilebuf, ltile, lflip, true);
        memcpy(linktilebuf, unpackbuf, 256);
        tempx=Link.getX();
        tempy=Link.getY();
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_pound, down, zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
      }
      if (magiccastclk>=0&&magiccastclk<64)
      {
        Link.setX(tempx+((rand()%3)-1));
        Link.setY(tempy+((rand()%3)-1));
      }
      if (magiccastclk==64)
      {
        Link.setX(tempx);
        Link.setY(tempy);
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_stab, down, zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
      }
      if (magiccastclk==96)
      {
        Link.setDontDraw(true);
        for (int i=0; i<16; ++i)
        {
          for (int j=0; j<16; ++j)
          {
            if(linktilebuf[i*16+j])
            {
              particles.add(new pFaroresWindDust(Link.getX()+j, Link.getY()-Link.getZ()+i, 5, 6, linktilebuf[i*16+j], rand()%96));
              int k=particles.Count()-1;
              particle *p = (particle*)(particles.spr(k));
              p->angular=true;
              p->angle=rand();
              p->step=(((double)j)/8);
              p->yofs=Link.getYOfs();
            }
          }
        }
      }
      if ((magiccastclk++)==226)
      {
        //attackclk=0;
        restart_level();
        //xofs=0;
        //action=none;
        magictype=mgc_none;
        Link.setDontDraw(false);
      }
    }
    break;
    case mgc_nayruslove:
    {
      if (magiccastclk==0)
      {
        Lwpns.add(new weapon(LinkX(),LinkY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET1,0,left));
        weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        Lwpns.add(new weapon(LinkX(),LinkY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET2,0,right));
        w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        //          Link.tile=(BSZ)?32:29;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        castx=Link.getX();
      }
      if (magiccastclk==64)
      {
        int d=max(LinkX(),256-LinkX())+32;
        Lwpns.add(new weapon((fix)(LinkX()-d),(fix)LinkY(),(fix)LinkZ(),wPhantom,pNAYRUSLOVEROCKETRETURN1,0,right));
        weapon *w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        Lwpns.add(new weapon((fix)(LinkX()+d),(fix)LinkY(),(fix)LinkZ(),wPhantom,pNAYRUSLOVEROCKETRETURN2,0,left));
        w1 = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
        w1->step=4;
        //          Link.tile=29;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_cast, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        castnext=false;
      }
      if (castnext)
      {
        //          Link.tile=4;
        linktile(&Link.tile, &Link.flip, &Link.extend, ls_landhold2, Link.getDir(), zinit.linkanimationstyle);
        if (get_bit(quest_rules,qr_EXPANDEDLTM))
        {
          Link.tile+=item_tile_mod(shieldModify);
        }
        Link.setNayrusLoveShieldClk(512);
        castnext=false;
        magiccastclk=128;
      }

      /*
        */
      if ((magiccastclk++)==160)
      {
        magictype=mgc_none;
      }
    }
    break;
    default:
    magiccastclk=0;
    break;
  }
}

void update_hookshot()
{
  int hs_x, hs_y, hs_z,hs_dx, hs_dy;
  bool check_hs=false;
  int dist_bx, dist_by, hs_w;
  chainlinks.animate();
  //  char tempbuf[80];
  //  char tempbuf2[80];

  //find out where the head is and make it
  //easy to reference
  if (Lwpns.idFirst(wHookshot)>-1)
  {
    check_hs=true;
  }
  if (check_hs)
  {
    hs_x=Lwpns.spr(Lwpns.idFirst(wHookshot))->x;
    hs_y=Lwpns.spr(Lwpns.idFirst(wHookshot))->y;
    hs_z=Lwpns.spr(Lwpns.idFirst(wHookshot))->z;
    hs_dx=hs_x-hs_startx;
    hs_dy=hs_y-hs_starty;
    //extending
    if (((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==0)
    {
      int maxchainlinks=(current_item(itype_hookshot)==2)?zinit.longshot_links:zinit.hookshot_links;
      if (chainlinks.Count()<maxchainlinks)          //extending chain
      {
        if (abs(hs_dx)>=hs_xdist+8)
        {
          hs_xdist=abs(hs_x-hs_startx);
          chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,Link.getDir()));
        }
        else if (abs(hs_dy)>=hs_ydist+8)
          {
            hs_ydist=abs(hs_y-hs_starty);
            chainlinks.add(new weapon((fix)hs_x, (fix)hs_y, (fix)hs_z,wHSChain, 0,0,Link.getDir()));
          }
      }                                                     //stretching chain
      else
      {
        dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
        dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
        hs_w=8;
        if (hs_dx<0)
        {
          dist_bx=0-dist_bx;
          hs_w=-8;
        }
        if (hs_dy<0)
        {
          dist_by=0-dist_by;
          hs_w=-8;
        }
        for (int counter=0; counter<chainlinks.Count(); counter++)
        {
          if (Link.getDir()>down)                           //chain is moving horizontally
          {
            chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
          }
          else
          {
            chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
          }
        }
      }
    }                                                       //retracting
    else if (((weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)))->misc==1)
      {
        dist_bx=(abs(hs_dx)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
        dist_by=(abs(hs_dy)-(8*chainlinks.Count()))/(chainlinks.Count()+1);
        hs_w=8;
        if (hs_dx<0)
        {
          dist_bx=0-dist_bx;
          hs_w=-8;
        }
        if (hs_dy<0)
        {
          dist_by=0-dist_by;
          hs_w=-8;
        }
        if (Link.getDir()>down)                               //chain is moving horizontally
        {
          if (abs(hs_dx)-(8*chainlinks.Count())>0)            //chain is stretched
          {
            for (int counter=0; counter<chainlinks.Count(); counter++)
            {
              chainlinks.spr(counter)->x=hs_startx+hs_w+dist_bx+(counter*(hs_w+dist_bx));
            }
          }
          else
          {
            if (abs(hs_x-hs_startx)<=hs_xdist-8)
            {
              hs_xdist=abs(hs_x-hs_startx);
              if (pull_link==false)
              {
                chainlinks.del(chainlinks.idLast(wHSChain));
              }
              else
              {
                chainlinks.del(chainlinks.idFirst(wHSChain));
              }
            }
          }
        }                                                     //chain is moving vertically
        else
        {
          if (abs(hs_dy)-(8*chainlinks.Count())>0)            //chain is stretched
          {
            for (int counter=0; counter<chainlinks.Count(); counter++)
            {
              chainlinks.spr(counter)->y=hs_starty+hs_w+dist_by+(counter*(hs_w+dist_by));
            }
          }
          else
          {
            if (abs(hs_y-hs_starty)<=hs_ydist-8)
            {
              hs_ydist=abs(hs_y-hs_starty);
              if (pull_link==false)
              {
                chainlinks.del(chainlinks.idLast(wHSChain));
              }
              else
              {
                chainlinks.del(chainlinks.idFirst(wHSChain));
              }
            }
          }
        }
      }
  }
}

void game_loop()
{
  //  walkflagx=0; walkflagy=0;
  if(fadeclk>=0)
  {
    if(fadeclk==0 && currscr<128)
      blockpath=false;
    --fadeclk;
  }
  bool freeze = false;
  bool freezeff = false;
  for(int i=0;i<176;i++)
  {
    if(combobuf[tmpscr->data[i]].type == cSCREENFREEZE) freeze=true;
    if(combobuf[tmpscr->data[i]].type == cSCREENFREEZEFF) freezeff=true;
  }
  animate_combos();
  load_control_state();

  if(!freezeff)
  {
    update_freeform_combos();
  }
  if(!freeze)
  {
    run_script(1, 0, SCRIPT_GLOBAL);

    mblock2.animate(0);
    items.animate();
    items.check_conveyor();
    guys.animate();
    roaming_item();
    dragging_item();
    Ewpns.animate();
    checklink=true;
    for(int i=0; i<(gofast?8:1); i++)
    {
      if(Link.animate(0))
      {
        if(!Quit)
        {
          Quit=qGAMEOVER;
        }
        return;
      }
      checklink=false;
    }
    do_magic_casting();
    Lwpns.animate();
    decorations.animate();
    particles.animate();
    update_hookshot();
    if (conveyclk<=0)
    {
      conveyclk=3;
    }
    --conveyclk;
    check_collisions();
    dryuplake();
    cycle_palette();
    nosecretsounds=(tmpscr->flags3&fNOSECRETSOUND)!=0;
  }
  draw_screen(tmpscr, 0, 0);
  //  v This is in draw_screen now
  //  put_passive_subscr(framebuf,0,0);
  if (linkedmsgclk==1)
  {
    //4 is for iwMore
    if (wpnsbuf[iwMore].tile!=0)
    {
      putweapon(framebuf,zinit.msg_more_x, zinit.msg_more_y+playing_field_offset, wPhantom, 4, up, lens_hint_weapon[wPhantom][0], lens_hint_weapon[wPhantom][1]);
    }
  }

  if(!freeze)
  {
    putintro();
  }

  if (dmapmsgclk>0)
  {
    Link.Freeze();
    if (dmapmsgclk<=50)
    {
      --dmapmsgclk;
    }
  }
  if (dmapmsgclk==1)
  {
    //    if (!tmpscr[currscr>=128?1:0].str)
    if (!msgstr)
    {
      //these are to cancel out any keys that Link may
      //be pressing so he doesn't attack at the end of
      //a message if he was scrolling through it quickly.
      rAbtn();
      rBbtn();

      Link.unfreeze();
    }
    dmapmsgclk=0;
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    //    clear_bitmap(pricesdisplaybuf);
  }

  if(!freeze)
  {
    if (!dmapmsgclk)
    {
      putmsg();
    }
    domoney();
    domagic();
    if(lensclk)
    {
      draw_lens_over();
      --lensclk;
    }
  }

  //  putpixel(framebuf, walkflagx, walkflagy+playing_field_offset, vc(int(rand()%16)));
}

int get_currdmap()
{
  return currdmap;
}

int get_dlevel()
{
  return dlevel;
}

int get_currscr()
{
  return currscr;
}

int get_homescr()
{
  return homescr;
}

int get_bmaps(int si)
{
  return game->bmaps[si];
}

word get_gamedata_maxlife()
{
  return get_gamedata_maxlife(game);
}

word get_gamedata_life()
{
  return get_gamedata_life(game);
}

byte get_gamedata_magicdrainrate()
{
  return get_gamedata_magicdrainrate(game);
}

word get_gamedata_maxmagic()
{
  return get_gamedata_maxmagic(game);
}

word get_gamedata_magic()
{
  return get_gamedata_magic(game);
}

void set_gamedata_maxlife(word l)
{
  set_gamedata_maxlife(game,l);
}

void set_gamedata_life(word l)
{
  set_gamedata_life(game,l);
}

void set_gamedata_maxmagic(word l)
{
  set_gamedata_maxmagic(game,l);
}

void set_gamedata_magic(word l)
{
  set_gamedata_magic(game,l);
}

byte get_gamedata_cheat()
{
  return get_gamedata_cheat(game);
}

byte get_gamedata_HCpieces()
{
  return get_gamedata_HCpieces(game);
}

byte get_gamedata_sbombs()
{
  return get_gamedata_sbombs(game);
}

byte get_gamedata_bombs()
{
  return get_gamedata_bombs(game);
}

byte get_gamedata_keys()
{
  return get_gamedata_keys(game);
}

byte get_gamedata_lkeys()
{
  return game->lvlkeys[get_dlevel()];
}

byte get_gamedata_timevalid()
{
  return get_gamedata_timevalid(game);
}

dword get_gamedata_time()
{
  return get_gamedata_time(game);
}

word get_gamedata_arrows()
{
  return get_gamedata_arrows(game);
}

word get_gamedata_rupies()
{
  return get_gamedata_rupies(game);
}

bool no_subscreen()
{
  return (tmpscr->flags3&fNOSUBSCR)!=0;
}

/**************************/
/********** Main **********/
/**************************/

char cfg_error[] =
  "\"ag.cfg\" not found. Please run \"agsetup.exe\"";

void PopulateInitDialog();

bool is_zquest()
{
  return false;
}

int main(int argc, char* argv[])
{

  switch (IS_BETA)
  {
    case -1:
    Z_title("Zelda Classic %s Alpha (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case 1:
    Z_title("Zelda Classic %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case 0:
    Z_title("Zelda Classic %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
  }

  //turn on MSVC memory checks
  //this should be interesting...
#ifdef _MSC_VER
#ifdef _DEBUG
  int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  flag |= (_CRTDBG_ALLOC_MEM_DF);
  _CrtSetDbgFlag(flag);
#endif
#endif

  //  Z_title("Zelda Classic %s",VerStr(ZELDA_VERSION));

  // allocate quest data buffers

  PopulateInitDialog();

  memrequested+=4096;
  Z_message("Allocating quest path buffers (%s)...", byte_conversion2(4096,memrequested,-1,-1));
  qstdir = (char*)malloc(2048);
  qstpath = (char*)malloc(2048);
  if(!qstdir||!qstpath)
  {
    Z_error("Error");
  }
  qstdir[0] = 0;
  qstpath[0] = 0;

#ifdef ALLEGRO_MACOSX
  sprintf(qstdir, "../");
  sprintf(qstpath, "../");
#endif

  Z_message("OK\n");
  get_qst_buffers();

  // initialize Allegro

  Z_message("Initializing Allegro... ");
  allegro_init();
  three_finger_flag=false;
  //atexit(&dumb_exit);
  //dumb_register_stdfiles();

  register_bitmap_file_type("GIF",  load_gif, save_gif);
  jpgalleg_init();
  loadpng_init();

  set_config_file("ag.cfg");

  if(install_timer() < 0)
    Z_error(allegro_error);

  if(install_keyboard() < 0)
    Z_error(allegro_error);

  if(install_mouse() < 0)
    Z_error(allegro_error);

  if(install_joystick(JOY_TYPE_AUTODETECT) < 0)
    Z_error(allegro_error);

  LOCK_VARIABLE(logic_counter);
  LOCK_FUNCTION(update_logic_counter);
  install_int_ex(update_logic_counter, BPS_TO_TIMER(60));

  Z_init_timers();
  Z_message("OK\n");

  // allocate bitmap buffers
  // check for the ag.cfg file and 1st.qst
  Z_message("Checking Files... ");
  if(!exists("1st.qst"))
  {
    Z_error("\"1st.qst\" not found.");
  }
  if(!exists("2nd.qst"))
  {
    Z_error("\"2nd.qst\" not found.");
  }
  if(!exists("3rd.qst"))
  {
    Z_error("\"3rd.qst\" not found.");
  }
  Z_message("OK\n");


  Z_message("Allocating bitmap buffers... ");
  if(used_switch(argc,argv,"-16bit"))
  {
    set_color_depth(16);
  }
  else
  {
    set_color_depth(8);
  }
  framebuf  = create_bitmap_ex(8,256,224);
  temp_buf  = create_bitmap_ex(8,256,224);
  temp_buf2 = create_bitmap_ex(8,256,224);
  scrollbuf = create_bitmap_ex(8,512,406);
  screen2   = create_bitmap_ex(8,320,240);
  tmp_scr   = create_bitmap_ex(8,320,240);
  tmp_bmp   = create_bitmap_ex(8,32,32);
  fps_undo  = create_bitmap_ex(8,64,16);
  msgdisplaybuf = create_bitmap_ex(8,256, 176);
  pricesdisplaybuf = create_bitmap_ex(8,256, 176);
  if(!framebuf || !scrollbuf || !tmp_bmp || !fps_undo || !tmp_scr
     || !screen2 || !msgdisplaybuf || !pricesdisplaybuf)
    Z_error("Error");

  clear_bitmap(scrollbuf);
  clear_bitmap(framebuf);
  clear_bitmap(msgdisplaybuf);
  set_clip_state(msgdisplaybuf, 1);
  clear_bitmap(pricesdisplaybuf);
  set_clip_state(pricesdisplaybuf, 1);
  Z_message("OK\n");

  // load game configurations

  load_game_configs();
  zcmusic_init();

  //  int mode = VidMode;                                       // from config file
  int tempmode=GFX_AUTODETECT;
  int res_arg = used_switch(argc,argv,"-res");

  if(used_switch(argc,argv,"-v0")) Throttlefps=false;
  if(used_switch(argc,argv,"-v1")) Throttlefps=true;
  /*
    if(used_switch(argc,argv,"-16bit")) {
    colordepth=16;
    } else {
    colordepth=8;
    }
    set_color_depth(colordepth);
    */
  resolve_password(zeldapwd);
  debug_enabled = used_switch(argc,argv,"-d") && !strcmp(get_config_string("zeldadx","debug",""),zeldapwd);
  set_debug(debug_enabled);

  skipicon = used_switch(argc,argv,"-quickload");

  int load_save=0;

  load_save = used_switch(argc,argv,"-load");
  load_save = load_save?(argc>load_save+1)?atoi(argv[load_save+1]):0:0;

  if(!(used_switch(	argc,argv,"-load")))
    if(used_switch(argc,argv,"-loadlast"))
      load_save = loadlast;

    slot_arg=used_switch(argc,argv,"-slot");
  if (!(argc>(slot_arg+1)))
  {
    slot_arg=0;
  }
  else
  {
    slot_arg2=atoi(argv[slot_arg+1]);
  }
  int fast_start = debug_enabled || used_switch(argc,argv,"-fast") || load_save || (slot_arg && (argc>(slot_arg+1)));
  int save_arg=used_switch(argc,argv,"-savefile");
  if(save_arg&&(argc>(save_arg+1)))
  {
    SAVE_FILE=(char *)malloc(2048);
    sprintf(SAVE_FILE, "%s", argv[save_arg+1]);
  }



  // load the data file

  resolve_password(datapwd);
  packfile_password(datapwd);



  Z_message("Loading data files:\n");
  set_color_conversion(COLORCONV_NONE);

  sprintf(zeldadat_sig,"Zelda.Dat %s Build %d",VerStr(ZELDADAT_VERSION), ZELDADAT_BUILD);
  sprintf(sfxdat_sig,"SFX.Dat %s Build %d",VerStr(SFXDAT_VERSION), SFXDAT_BUILD);
  sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);

  Z_message("Zelda.Dat...");
  if((data=load_datafile("zelda.dat"))==NULL)
  {
    Z_error("failed");
  }
  if(strncmp((char*)data[0].dat,zeldadat_sig,23))
  {
    Z_error("\nIncompatible version of zelda.dat.\nPlease upgrade to %s Build %d",VerStr(ZELDADAT_VERSION), ZELDADAT_BUILD);
  }
  Z_message("OK\n");

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

  packfile_password(NULL);

  Z_message("SFX.Dat...");
  if((sfxdata=load_datafile("sfx.dat"))==NULL)
  {
    Z_error("failed");
  }
  if(strncmp((char*)sfxdata[0].dat,sfxdat_sig,21))
  {
    Z_error("\nIncompatible version of sfx.dat.\nPlease upgrade to %s Build %d",VerStr(SFXDAT_VERSION), SFXDAT_BUILD);
  }
  Z_message("OK\n");

  mididata = (DATAFILE*)data[ZC_MIDI].dat;

  set_uformat(U_ASCII);
  deffont = font;
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
  sfont2 = (FONT*)fontsdata[FONT_6x4].dat;
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

  for(int i=0; i<4; i++)
  {
    for(int j=0;j<MAXSUBSCREENITEMS;j++)
    {
      memset(&custom_subscreen[i].objects[j],0,sizeof(subscreen_object));
    }
  }
  for(int i=0; i<WAV_COUNT; i++)
  {
     customsfxdata[i].data=NULL;
  }
  for(int i=0;i<WPNCNT;i++)
  {
    weapon_string[i] = new char[64];
  }
  for(int i=0;i<ITEMCNT;i++)
  {
    item_string[i] = new char[64];
  }
  for(int i=0;i<512;i++)
  {
    ffscripts[i] = new ffscript[1];
    ffscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<256;i++)
  {
    itemscripts[i] = new ffscript[1];
    itemscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<256;i++)
  {
    guyscripts[i] = new ffscript[1];
    guyscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<256;i++)
  {
    wpnscripts[i] = new ffscript[1];
    wpnscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<256;i++)
  {
    screenscripts[i] = new ffscript[1];
    screenscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<3;i++)
  {
    globalscripts[i] = new ffscript[1];
    globalscripts[i][0].command = 0xFFFF;
  }
  for(int i=0;i<3;i++)
  {
    linkscripts[i] = new ffscript[1];
    linkscripts[i][0].command = 0xFFFF;
  }

  // load saved games
  Z_message("Loading saved games... ");
  if(load_savedgames()!=0)
  {
    Z_error("Insufficient memory");
  }

  Z_message("OK\n");

  // initialize sound driver

  Z_message("Initializing sound driver... ");
  if(used_switch(argc,argv,"-s") || used_switch(argc,argv,"-nosound"))
  {
    Z_message("skipped\n");
  }
  else
  {
    if(install_sound(DIGI_AUTODETECT,DIGI_AUTODETECT,NULL))
    {
      //      Z_error(allegro_error);
      Z_message("Sound driver not available.  Sound disabled.\n");
    }
    else
    {
      Z_message("OK\n");
    }
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

  if(res_arg && (argc>(res_arg+2)))
  {
    resx = atoi(argv[res_arg+1]);
    resy = atoi(argv[res_arg+2]);
    sbig = (argc>(res_arg+3))? stricmp(argv[res_arg+3],"big")==0 : 0;
    //    mode = GFX_AUTODETECT;
  }
  request_refresh_rate(60);

  if(used_switch(argc,argv,"-fullscreen"))
  {
    al_trace("Used switch: -fullscreen\n");
    tempmode = GFX_AUTODETECT_FULLSCREEN;
  }
  else if(used_switch(argc,argv,"-windowed"))
  {
    al_trace("Used switch: -windowed\n");
    tempmode=GFX_AUTODETECT_WINDOWED;
  }

  if(!game_vid_mode(tempmode,250))
  {
    if (tempmode==GFX_AUTODETECT_FULLSCREEN&&resx==320&&resy==240&&sbig==0)
    {
      sbig=1;
      resx=640;
      resy=480;
      if(!game_vid_mode(tempmode,250))
      {
        Z_error(allegro_error);
      }
    }
    else
    {
      Z_error(allegro_error);
    }
  }

  real_screen=screen;

  /* if triple buffering isn't available, try to enable it */
  if (!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER))
  {
    enable_triple_buffer();
  }

  /* if that didn't work, give up */
  if (!(gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)) {
    triplebuffer_not_available = TRUE;
  }

  if(!triplebuffer_not_available)
  {
    for (int i=0; i<3; ++i)
    {
      tb_page[i]=create_video_bitmap(SCREEN_W, SCREEN_H);
    }
    for (int i=0; i<3; ++i)
    {
      if(!tb_page[i])
      {
        triplebuffer_not_available=true;
      }
      else
      {
        clear_bitmap(tb_page[i]);
      }
    }
    if (triplebuffer_not_available)
    {
      for (int i=0; i<3; ++i)
      {
        destroy_bitmap(tb_page[i]);
      }
    }
  }

  Z_message("Triplebuffer %savailable\n", triplebuffer_not_available?"not ":"");

  set_close_button_callback((void (*)()) hit_close_button);
  set_window_title("Zelda Classic");

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
  reset_items(true, &QHeader);

  Quit = fast_start ? qQUIT : qRESET;

  rgb_map = &rgb_table;

  // set up an initial game save slot (for the list_saves function)
  game=(gamedata *)malloc(sizeof(gamedata));

  while(Quit!=qEXIT)
  {
    toogam = false;
    //    packfile_password(datapwd);
    titlescreen(load_save);
    load_save=0;
    //    packfile_password(NULL);
    setup_combo_animations();
  setup_combo_animations2();
    while(!Quit)
    {
      drawit=true;
      game_loop();
      advanceframe();
    }
    tmpscr->flags3=0;
    Playing=Paused=false;
    switch(Quit)
    {
      case qQUIT:
      case qGAMEOVER:
      {
        Link.setDontDraw(false);
        show_subscreen_dmap_dots=true;
        show_subscreen_numbers=true;
        show_subscreen_items=true;
        show_subscreen_life=true;
        introclk=intropos=0;
		  for(int i=0;i<8;i++)
        {
          g_d[i]=0;
        }
        for(int i=0;i<256;i++)
        {
          g_stack[i]=0;
        }
        g_scriptflag=0;
        g_pc=0;
        g_sp=0;
        g_doscript=1;
        global_itemclass=0;
        global_ewpnclass=0;
         global_lwpnclass=0;
         global_guyclass=0;
         global_ffc=0;
        global_item=0;
        global_lwpn=0;
        global_ewpn=0;
        global_guy=0;
        run_script(2, 0, SCRIPT_GLOBAL);
        if(!skipcont&&!get_bit(quest_rules,qr_NOCONTINUE)) game_over(get_bit(quest_rules,qr_NOSAVE));
        skipcont = 0;
      } break;
      case qWON:
      {
        Link.setDontDraw(false);
        show_subscreen_dmap_dots=true;
        show_subscreen_numbers=true;
        show_subscreen_items=true;
        show_subscreen_life=true;
        for(int i=0;i<8;i++)
        {
          g_d[i]=0;
        }
        for(int i=0;i<256;i++)
        {
          g_stack[i]=0;
        }
        g_scriptflag=0;
        g_pc=0;
        g_sp=0;
        g_doscript=1;
        global_itemclass=0;
        global_ewpnclass=0;
        global_lwpnclass=0;
        global_guyclass=0;
        global_ffc=0;
        global_item=0;
        global_lwpn=0;
        global_ewpn=0;
        global_guy=0;
        run_script(2, 0, SCRIPT_GLOBAL);
        ending();
      } break;

    }
    kill_sfx();
    music_stop();
    clear_to_color(screen,BLACK);
  }

  // clean up

  music_stop();
  kill_sfx();
  set_gfx_mode(GFX_TEXT,80,25,0,0);
  rest(250);

quick_quit:

  save_savedgames();
  save_game_configs();
  //  if(useCD)
  //    cd_exit();
  quit_game();
  return 0;
}

void quit_game()
{
  unload_datafile(data);
  //  if(mappic)
  //    destroy_bitmap(mappic);
  destroy_bitmap(framebuf);
  destroy_bitmap(scrollbuf);
  destroy_bitmap(tmp_scr);
  destroy_bitmap(screen2);
  destroy_bitmap(tmp_bmp);
  destroy_bitmap(fps_undo);
  set_clip_state(msgdisplaybuf, 1);
  destroy_bitmap(msgdisplaybuf);
  set_clip_state(pricesdisplaybuf, 1);
  destroy_bitmap(pricesdisplaybuf);
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
  zcmusic_exit();
  for(int i=0; i<WAV_COUNT; i++)
  {
    if(customsfxdata[i].data!=NULL)
    {
//      delete [] customsfxdata[i].data;
      free(customsfxdata[i].data);
    }
  }
  for(int i=0;i<WPNCNT;i++)
  {
    delete [] weapon_string[i];
  }
  for(int i=0;i<ITEMCNT;i++)
  {
    delete [] item_string[i];
  }
  for(int i=0;i<512;i++)
  {
    if(ffscripts[i]!=NULL) delete [] ffscripts[i];
  }
  for(int i=0;i<256;i++)
  {
    if(itemscripts[i]!=NULL) delete [] itemscripts[i];
  }
  for(int i=0;i<256;i++)
  {
    if(guyscripts[i]!=NULL) delete [] guyscripts[i];
  }
  for(int i=0;i<256;i++)
  {
    if(wpnscripts[i]!=NULL) delete [] wpnscripts[i];
  }
  for(int i=0;i<256;i++)
  {
    if(screenscripts[i]!=NULL) delete [] screenscripts[i];
  }
  for(int i=0;i<3;i++)
  {
    if(globalscripts[i]!=NULL) delete [] globalscripts[i];
  }
  for(int i=0;i<3;i++)
  {
    if(linkscripts[i]!=NULL) delete [] linkscripts[i];
  }
  //  dumb_exit();
  Z_message("Armageddon Games web site: http://www.armageddongames.com\n");
  Z_message("Zelda Classic web site: http://www.zeldaclassic.com\n");

  allegro_exit();
}

END_OF_MAIN()

  /*** end of zelda.cc ***/
