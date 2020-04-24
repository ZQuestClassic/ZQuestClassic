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
#include "zfix.h"

class particle : public sprite
{
public:
    int layer, cset, color;
    zfix step;
    particle(zfix X,zfix Y,int layer,int cset,int color);
    virtual ~particle();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};

class pFaroresWindDust : public particle
{
public:
    zfix os;
    int ot;
    int timer;
    bool initialized;
    pFaroresWindDust(zfix X,zfix Y,int layer,int cset,int color,int timer);
    virtual bool animate(int index);
};

class pTwilight : public particle
{
public:
    int delay;
    pTwilight(zfix X,zfix Y,int layer,int cset,int color, int delay);
    virtual bool animate(int index);
};




#endif
/*** end of sprite.cc ***/

