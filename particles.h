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

#ifndef _PARTICLES_H_
#define _PARTICLES_H_

#include "sprite.h"
#include "zdefs.h"

class particle : public sprite
{
  public:
    int layer, cset, color, timer;
    fix step;
    particle(fix X,fix Y,int layer,int cset,int color,int timer);
    virtual ~particle();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class pFaroresWindDust : public particle
{
  public:
    fix os;
    int ot;
    bool initialized;
    pFaroresWindDust(fix X,fix Y,int layer,int cset,int color,int timer);
    virtual bool animate(int index);
};



#endif
/*** end of sprite.cc ***/
