//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  link.cc
//
//  Link's class: LinkClass
//  Handles a lot of game play stuff as well as Link's
//  movement, attacking, etc.
//
//--------------------------------------------------------

#ifndef _LINK_H_
#define _LINK_H_

#include "zc_alleg.h"
#include "zcmusic.h"
#include "zdefs.h"
#include "zelda.h"
#include "maps.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "aglogo.h"
#include "zsys.h"
#include "qst.h"
#include "matrix.h"
#include "jwin.h"
#include "jwinfsel.h"
//#include "save_gif.h"
#include "sprite.h"
#include "zc_custom.h"
#include "subscr.h"

extern movingblock mblock2;                                 //mblock[4]?
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;

extern const byte lsteps[8];

enum { did_fairy=1, did_candle=2, did_whistle=4, did_magic=8, did_glove=16, did_all=32 };

enum actiontype
{
  none, walking, attacking, freeze, landhold1, landhold2,
  rafting, gothit, inwind, scrolling, won, swimming, hopping,
  swimhit, waterhold1, waterhold2, casting, climbcovertop,
  climbcoverbottom, dying, drowning, climbing
};

typedef struct tilesequence
{
  word tile;
  byte frames;                                              // animation frame count
  byte speed;                                               // animation speed
  byte exp;                                                 // not used
} tilesequence;

#define SWORDCHARGEFRAME 20
#define SWORDTAPFRAME SWORDCHARGEFRAME-8
#define HAMMERCHARGEFRAME 11
#define WANDCHARGEFRAME 12

class LinkClass : public sprite
{
  bool autostep,superman,refilling,inlikelike,inwallm,tapping;
  int ladderx,laddery,
      warpx,warpy,
      pushing,  //incremental time spent pushing.
      jumping,  //incremental time spent airborne.
      charging, //incremental time spent charging.
      spins,    //1 + number of sword spins left to do. 0 = not spinning.
      fairyclk, //fairy circle timeout.
      refillclk,//life refill timeout.
      drunkclk, //intoxication timeout.
      NayrusLoveShieldClk, // Nayru's Love timeout.
      hoverclk, //hover boots timeout.
      hclk,     //damage blinking timeout.
      holdclk,  //"hold up item" timeout.
      holditem, //item being held.
      attackclk,//attack timeout.
      attack,   //current attack.
      swordclk, //curse timeout.
      didstuff, //played the whistle? used the blue candle?
      blowcnt,  //number of times whistle blown.
      drownclk; //drowning timeout.
  byte skipstep,lstep,hopclk,diveclk,whirlwind;
  byte hitdir,ladderdir,lastdir[3],ladderstart;
  int stepoutindex, stepoutwr;
  int shiftdir;
  int sdir;
  int holddir;
  int landswim;
  bool ilswim;
  bool walkable;
  actiontype action;
  int hshandle_id, hshead_id, itemclk;
  byte conveyor_flags;
  fix climb_cover_x, climb_cover_y;
  fix entry_x, entry_y;
  bool dontdraw;
  // Methods below here.
  void movelink();
  void move(int d);
  void hitlink(int hit);
  int  nextcombo(int cx,int cy,int cdir);
  int  nextflag(int cx,int cy,int cdir, bool comboflag);
  bool nextcombo_wf(int d);
  bool nextcombo_solid(int d);
  int  lookahead(int destscr = -1, int d=up);
  int  lookaheadflag(int destscr = -1, int d=up);
  void checkhit();
  void checkdamagecombos(int dx1, int dx2, int dy1, int dy2);
  void checkscroll();
  void checkspecial();
  void checkspecial2(int *ls);
  void checkspecial3();
  void checkpushblock();
  void checkbosslockblock();
  void checklockblock();
  void checkswordtap();
  void checkchest();
  void checklockedchest();
  void checkbosschest();
  void checktouchblk();
  void checklocked();
  void checkitems();
  bool startwpn(int wpn);
  bool doattack();
  bool can_attack();
  void do_rafting();
  void do_hopping();
  bool walkflag(int wx,int wy,int cnt,byte d);
  bool checkmaze(mapscr *scr);
  void scrollscr(int dir,int destscr = -1, int destdmap = -1);
  void scrollscr2(int dir,int destscr = -1, int destdmap = -1);
  void walkdown();
  void walkup();
  void walkdown2();
  void walkup2();
  void exitcave();
  void stepout();
  void masked_draw(BITMAP *dest);
  void getTriforce(int id);
  void checkstab();
  void fairycircle(int type);
  void StartRefill();
  int  EwpnHit();
  int  LwpnHit();
  void gameover();
  void ganon_intro();
  void saved_Zelda();
  void check_slash_block(int bx, int by);
  void check_wand_block(int bx, int by);
  void check_pound_block(int bx, int by);
  void check_conveyor();
  bool check_cheat_warp();
  bool sideviewhammerpound();

public:

  int DrunkClock();
  LinkClass();
  void init();
  virtual void drawshadow(BITMAP* dest, bool translucent);
  virtual void draw(BITMAP* dest);
  virtual bool animate(int index);
  bool dowarp(int type, int index);

  void linkstep();
  void stepforward(int steps, bool adjust);
  void draw_under(BITMAP* dest);

  // called by ALLOFF()
  void resetflags(bool all);
  void Freeze();
  void unfreeze();
  void Drown();
  void beatlikelike();
  fix  getX();
  fix  getY();
  fix  getZ();
  int  getFall();
  fix  getXOfs();
  fix  getYOfs();
  void setXOfs(int newxofs);
  void setYOfs(int newyofs);
  int  getHXOfs();
  int  getHYOfs();
  int  getHXSz();
  int  getHYSz();
  fix  getClimbCoverX();
  fix  getClimbCoverY();
  void setX(int new_x);
  void setY(int new_y);
  void setZ(int new_Z);
  void setFall(int new_fall);
  void setClimbCoverX(int new_x);
  void setClimbCoverY(int new_y);
  int  getLStep();
  int  getCharging();
  void setCharging(int new_charging);
  bool isCharged();
  int  getAttackClk();
  int  getSwordClk();
  int  getItemClk();
  void  setSwordClk(int newclk);
  void  setItemClk(int newclk);
  fix  getModifiedX();
  fix  getModifiedY();
  int  getDir();
  int  getClk();
  int  getPushing();
  void reset_hookshot();
  void reset_ladder();
  bool refill();
  void Catch();
  bool getClock();
  void setClock(bool state);
  int  getAction();
  void setAction(actiontype new_action);
  bool isDiving();
  bool isSwimming();
  void setDontDraw(bool new_dontdraw);
  void setHClk(int newhclk);
  int getHClk();
  void setNayrusLoveShieldClk(int newclk);
  int getNayrusLoveShieldClk();
  int getHoverClk();
  bool ffwarp;
  bool ffpit;
};

bool isRaftFlag(int flag);
void do_lens();
int touchcombo(int x,int y);
extern bool did_secret;
int selectWlevel(int d);
bool edge_of_dmap(int side);
void computeMaxArrows();

/************************************/
/********  More Items Code  *********/
/************************************/

int Bweapon(int pos);
void selectWpn(int xstep, int ystep, bool b);
bool isWpnPressed(int wpn);
bool canget(int id);
void dospecialmoney(int index);
void getitem(int id, bool nosound=false);
void getdraggeditem(int j);
void red_shift();
void slide_in_color(int color);
#endif
/*** end of link.cc ***/
