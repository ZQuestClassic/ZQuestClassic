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

#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "sprite.h"
#include "zdefs.h"

extern int fairy_cnt;
void movefairy(fix &x,fix &y,int misc);
void killfairy(int misc);
void addenemy(int x,int y,int id,int clk);
void addfairy(fix x, fix y, int misc);

class item : public sprite
{
  public:
    int pickup,clk2;
    int aclk,aframe;
    bool flash,twohand,anim;
    item(fix X,fix Y,int i,int p,int c);
    virtual ~item();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

// easy way to draw an item
void putitem(BITMAP *dest,int x,int y,int item_id);
void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash);
#endif
/*** end of sprite.cc ***/
