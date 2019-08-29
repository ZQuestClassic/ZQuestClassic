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

#include "zdefs.h"

void movefairy(fix &x,fix &y,int misc)
{
  //these are here to bypass compiler warnings about unused arguments
  x=x;
  y=y;
  misc=misc;

  return;
}

void killfairy(int misc)
{
  //these are here to bypass compiler warnings about unused arguments
  misc=misc;

  return;
}

void addfairy(fix x, fix y, int id)
{
  //these are here to bypass compiler warnings about unused arguments
  x=x;
  y=y;
  id=id;

  return;
}

/*** end of sprite.cc ***/
