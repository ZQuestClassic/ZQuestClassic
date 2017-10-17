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

#include "items.h"

int fairy_cnt=0;

item::~item()
{
  if(id==iFairyMoving && misc>0)
    killfairy(misc);
}

bool item::animate(int index)
{
  if((++clk)>=0x8000)
    clk=0x7000;
  if(flash)
  {
    cs = itemsbuf[id].csets;
    if(frame&8)
      cs >>= 4;
    else
      cs &= 15;
  }
  if(anim)
  {
    int spd = itemsbuf[id].speed;
    if(aframe==0)
      spd *= itemsbuf[id].delay+1;

    if(++aclk >= spd)
    {
      aclk=0;
      if(++aframe >= itemsbuf[id].frames)
        aframe=0;

    }
    tile = itemsbuf[id].tile + aframe;
  }

  if(id==iFairyMoving)
  {
    movefairy(x,y,misc);
  }

  if(fadeclk==0)
    return true;
  if(pickup&ipTIMER)
    if(++clk2 == 512)
      return true;
  return false;
}

void item::draw(BITMAP *dest)
{
  if(pickup&ipNODRAW)
    return;

  if(!(pickup&ipFADE) || fadeclk<0 || fadeclk&1)
    if(clk2>32 || (clk2&2)==0 || id==iFairyMoving || id==iFairyStill)
      sprite::draw(dest);
}

item::item(fix X,fix Y,int i,int p,int c) : sprite()
{
  x=X; y=Y; id=i; pickup=p; clk=c;
  misc=clk2=0;
  aframe=aclk=0;
  anim=flash=twohand=false;

  if(id<0 || id>=iMax)
    return;

  tile = itemsbuf[id].tile;
  cs = itemsbuf[id].csets&15;

  if(itemsbuf[id].misc&1)
    flash=true;
  if(itemsbuf[id].misc&2)
    twohand=true;

  anim = itemsbuf[id].frames>0;

  if(pickup&ipBIGRANGE)
  {
    hxofs=-8;
    hxsz=17;
    hyofs=-4;
    hysz=20;
  }
  else if(pickup&ipBIGTRI)
  {
    hxofs=-8;
    hxsz=28;
    hyofs=-4;
    hysz=20;
  }
  else
  {
    hxsz=1;
    hyofs=4;
    hysz=12;
  }

  if(id==iFairyMoving)
  {
    misc = ++fairy_cnt;
    addfairy(x, y, misc);
  }
}

// easy way to draw an item

void putitem(BITMAP *dest,int x,int y,int item_id)
{
  item temp((fix)x,(fix)y,item_id,0,0);
  temp.yofs=0;
  temp.animate(0);
  temp.draw(dest);
}

void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash)
{
  item temp((fix)x,(fix)y,item_id,0,0);
  temp.yofs=0;
  temp.aclk=aclk;
  temp.aframe=aframe;
  if (flash)
  {
    temp.flash=flash;
  }
  temp.animate(0);
  temp.draw(dest);
  aclk=temp.aclk;
  aframe=temp.aframe;
}

/*** end of sprite.cc ***/
