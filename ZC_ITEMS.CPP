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

#include "zelda.h"
#include "guys.h"
#include "zdefs.h"
#include "maps.h"

/*
  void movefairy(fix &x,fix &y,int misc) {
  return;
  }

  void killfairy(int misc) {
  return;
  }
  */
void addfairy(fix x, fix y, int id)
{
  addenemy(x,y,eITEMFAIRY,id);
  sfx(WAV_SCALE);
  movefairy(x,y,id);
}

bool can_drop(fix x, fix y)
{
  return !_walkflag(x,y+16,0);
}

bool is_side_view()
{
  return (tmpscr->flags7&fSIDEVIEW)!=0;
}
/*** end of sprite.cc ***/
