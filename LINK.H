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
#include "save_gif.h"
#include "sprite.h"
#include "zc_custom.h"

extern movingblock mblock2;                                 //mblock[4]?
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;

extern const byte lsteps[8];

enum { did_fairy=1, did_candle=2, did_whistle=4 };

enum actiontype
{
  none, walking, attacking, freeze, holding1, holding2,
  rafting, gothit, inwind, scrolling, won, swimming, hopping,
  swimhit, swimhold1, swimhold2, casting, climbcovertop,
  climbcoverbottom, dying
};

typedef struct tilesequence
{
  word tile;
  byte frames;                                              // animation frame count
  byte speed;                                               // animation speed
  byte exp;                                                 // not used
} tilesequence;

class LinkClass : public sprite
{
  int drunkclk, NayrusLoveShieldClk;
  bool autostep,superman,refilling,inlikelike,inwallm;
  int attackclk,ladderx,laddery,pushing,fairyclk,refillclk,hclk;
  int warpx,warpy,holdclk,holditem,attack,swordclk,didstuff,blowcnt;
  byte skipstep,lstep,hopclk,diveclk,whirlwind;
  byte hitdir,ladderdir,lastdir[3];
  actiontype action;
  int hshandle_id, hshead_id, itemclk;
  byte conveyor_flags;
  fix climb_cover_x, climb_cover_y;
  bool dontdraw;
  void movelink();
  void move(int d);
  void hitlink(int hit);
  int  nextcombo(int cx,int cy,int cdir);
  int  nextflag(int cx,int cy,int cdir);
  bool nextcombo_wf();
  int  lookahead(int destscr = -1);
  int  lookaheadflag(int destscr = -1);
  void checkhit();
  void checkscroll();
  void checkspecial();
  void checkpushblock();
  void checkbosslockblock();
  void checklockblock();
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
  bool dowarp(int type);
  void exitcave();
  void stepout();
  void masked_draw(BITMAP *dest);
  void getTriforce(int id);
  void checkstab();
  void fairycircle();
  void StartRefill(int refill_why);
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

  public:

  int DrunkClock();
  LinkClass();
  void init();
  virtual void draw(BITMAP* dest);
  virtual bool animate(int index);

  void linkstep();
  void stepforward(int steps);
  void draw_under(BITMAP* dest);

  // called by ALLOFF()
  void resetflags(bool all);
  void Freeze();
  void unfreeze();
  void beatlikelike();
  fix  getX();
  fix  getY();
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
  void setClimbCoverX(int new_x);
  void setClimbCoverY(int new_y);
  int  getLStep();
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
  bool isDiving();
  bool isSwimming();
  void setDontDraw(bool new_dontdraw);
  void setHClk(int newhclk);
  int  getHClk();
  void setNayrusLoveShieldClk(int newclk);
  int  getNayrusLoveShieldClk();
};

bool isRaftFlag(int flag);
void do_lens();
int touchcombo(int x,int y);
extern bool did_secret;
int selectWlevel(int d);
bool edge_of_dmap(int side);

/************************************/
/********  More Items Code  *********/
/************************************/

int Bweapon(int pos);
void selectAwpn(int step);
void selectBwpn(int xstep, int ystep);
bool canget(int id);
void dospecialmoney(int index);
void getitem(int id);
void getdraggeditem(int j);
void setup_red_screen();
void slide_in_color(int color);
#endif
/*** end of link.cc ***/
