//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include "zdefs.h"
#include "sprite.h"

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

extern byte boomframe[16];
extern byte bszboomflip[4];

class weapon : public sprite
{
  void seekLink();
  void seekEnemy(int j);

  public:
    int power,type,dead,clk2,misc2,ignorecombo;
    int dragging;
    fix step;
    bool bounce, ignoreLink;
    word flash,wid,aframe,csclk;

    weapon(fix X,fix Y,int Id,int Type,int pow,int Dir);
    void LOADGFX(int wpn);
    bool Dead();
    bool clip();
    virtual bool animate(int index);
    virtual void onhit(bool clipped);
    virtual void onhit(bool clipped, int special, int linkdir);
    // override hit detection to check for invicibility, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int txsz,int tysz);
    virtual void draw(BITMAP *dest);
};

void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe);
#endif
/*** end of sprite.cc ***/
