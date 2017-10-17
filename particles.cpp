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

/**********************************/
/**********  Item Class  **********/
/**********************************/

#include "particles.h"

particle::~particle()
{
}

bool particle::animate(int index)
{
  if (timer>0)
  {
    --timer;
  }
  move(step);
  return (!timer);
}

void particle::draw(BITMAP *dest)
{
  int tcs=cset;
  tcs &= 15;
  tcs <<= CSET_SHFT;
  putpixel(dest, x, y+yofs, tcs+color);
}

particle::particle(fix X,fix Y,int L,int CS,int C, int T) : sprite()
{
  x=X; y=Y; layer=L; cset=CS; color=C; timer=T;
  yofs = 54;
}


pFaroresWindDust::pFaroresWindDust(fix X,fix Y,int L,int CS,int C, int T) : particle(X,Y,L,CS,C,T)
{
  initialized=false;
}

bool pFaroresWindDust::animate(int index)
{
  if (!initialized)
  {
    os=step;
    ot=timer;
    initialized=true;
  }
  step=os*(double)timer/(double)ot;
  return particle::animate(index);
}

/*** end of sprite.cc ***/
