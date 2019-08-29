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

#include "zdefs.h"
#include "sprite.h"
#include "tiles.h"

extern bool get_debug();
extern bool halt;
extern bool show_sprites;
extern bool is_zquest();

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

sprite::sprite()
{
  x=y=ground_y=z=tile=shadowtile=cs=flip=c_clk=clk=xofs=ground_yofs=zofs=hxofs=hyofs=0;
  id=-1;
  hxsz=hysz=16;
  yofs=playing_field_offset;
  dir=down;
  angular=canfreeze=false;
  drawstyle=0;
  extend=0;
  for(int i=0;i<8;i++)
  {
    if(i<2) a[i]=0;
    d[i]=0;
  }
  scriptflag=0;
  pc=0;
  sp=0;
  for(int i=0;i<256;i++)
  {
	stack[i]=0;
  }
  ffcref=0;
  doscript=1;
  itemclass=0;
}

sprite::sprite(fix X,fix Y,int T,int CS,int F,int Clk,int Yofs):
  x(X),y(Y),tile(T),cs(CS),flip(F),clk(Clk),yofs(Yofs)
{
  hxsz=hysz=16;
  hxofs=hyofs=xofs=0;
  id=-1;
  dir=down;
  angular=canfreeze=false;
  extend=0;
  for(int i=0;i<8;i++)
  {
    if(i<2) a[i]=0;
    d[i]=0;
  }
  scriptflag=0;
  pc=0;
  sp=0;
  for(int i=0;i<256;i++)
  {
	stack[i]=0;
  }
  ffcref=0;
  doscript=1;
  itemclass=0;
}

sprite::~sprite() {}
void sprite::draw2(BITMAP* dest)                            // top layer for special needs
{
}

void sprite::drawcloaked2(BITMAP* dest)                     // top layer for special needs
{
}

bool sprite::animate(int index) { ++c_clk; return false; }
int sprite::real_x(fix fx)
{
  int rx=fx.v>>16;
  switch(dir)
  {
    case 9:
    case 13:
    if(fx.v&0xFFFF)
      ++rx;
    break;
  }
  return rx;
}

int sprite::real_y(fix fy)
{
  return fy.v>>16;
}

int sprite::real_ground_y(fix fy)
{
  return fy.v>>16;
}

int sprite::real_z(fix fz)
{
  return fz.v>>16;
}

bool sprite::hit(sprite *s)
{
  if(id<0 || s->id<0 || clk<0) return false;
  if (halt)
  {
  }
  return hit(s->x+s->hxofs,s->y+s->hyofs,s->hxsz,s->hysz);
}

bool sprite::hit(int tx,int ty,int txsz,int tysz)
{
  if(id<0 || clk<0) return false;
  return (tx+txsz>x+hxofs && ty+tysz>y+hyofs && tx<x+hxofs+hxsz
          && ty<y+hyofs+hysz);
}

int sprite::hitdir(int tx,int ty,int txsz,int tysz,int dir)
{
  int cx1=x+hxofs+(hxsz>>1);
  int cy1=y+hyofs+(hysz>>1);
  int cx2=tx+(txsz>>1);
  int cy2=ty+(tysz>>1);
  if(dir>=left && abs(cy1-cy2)<=8)
    return (cx2-cx1<0)?left:right;
  return (cy2-cy1<0)?up:down;
}

void sprite::move(fix dx,fix dy)
{
  x+=dx; y+=dy;
}

void sprite::move(fix s)
{
  if(angular)
  {
    x += cos(angle)*s;
    y += sin(angle)*s;
    return;
  }

  switch(dir)
  {
    case 8:
    case up:     y-=s; break;
    case 12:
    case down:   y+=s; break;
    case 14:
    case left:   x-=s; break;
    case 10:
    case right:  x+=s; break;
    case 15:
    case l_up:   x-=s; y-=s; break;
    case 9:
    case r_up:   x+=s; y-=s; break;
    case 13:
    case l_down: x-=s; y+=s; break;
    case 11:
    case r_down: x+=s; y+=s; break;
  }
}

void sprite::draw(BITMAP* dest)
{
  if (!show_sprites)
  {
    return;
  }
  int sz = real_z(z+zofs);
  int sx = real_x(x+xofs);
  int sy = real_y(y+yofs);

  sz+=0; //temporarily here to keep from getting "unused variable" compiler warnings.

  if(id<0)
    return;
  if(clk>=0)
  {
    switch (drawstyle)
    {
      case 0:                                               //normal
      overtile16(dest,tile,sx,sy,cs,flip);
      break;
      case 1:                                               //phantom
      overtiletranslucent16(dest,tile,sx,sy,cs,flip,128);
      break;
      case 2:                                               //cloaked
      overtilecloaked16(dest,tile,sx,sy,flip);
      break;
      case 3:												//link
      switch(extend)
      {
        BITMAP *temp;
        case 0:
        overtile16(dest,tile,sx,sy,cs,flip);
        break;
        case 1:
        temp = create_bitmap_ex(8,16,32);
        clear_bitmap(temp);
        overtile16(temp,tile-TILES_PER_ROW,0,0,cs,flip);
        overtile16(temp,tile,0,16,cs,flip);
        masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
        destroy_bitmap(temp);
        break;
        case 2:
        temp = create_bitmap_ex(8,48,32);
        clear_bitmap(temp);
        overtile16(temp,tile-TILES_PER_ROW,16,0,cs,flip);
        overtile16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip);
        overtile16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip);
        overtile16(temp,tile,16,16,cs,flip);
        overtile16(temp,tile-(flip?-1:1),0,16,cs,flip);
        overtile16(temp,tile+(flip?-1:1),32,16,cs,flip);
        masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
        destroy_bitmap(temp);
        break;
      }
      break;
    }
  }
  else
  {
    int t  = wpnsbuf[iwSpawn].tile;
    int cs = wpnsbuf[iwSpawn].csets&15;
    if(BSZ)
    {
      if(clk>=-10) ++t;
      if(clk>=-5) ++t;
    }
    else
    {
      if(clk>=-12) ++t;
      if(clk>=-6) ++t;
    }
    overtile16(dest,t,sx,sy,cs,0);
  }

  if(get_debug() && key[KEY_O] && !is_zquest())
    rectfill(dest,x+hxofs,y+playing_field_offset+hyofs,x+hxofs+hxsz-1,y+playing_field_offset+hyofs+hysz-1,vc(id));
}

void sprite::draw8(BITMAP* dest)
{
  int sx = real_x(x+xofs);
  int sy = real_y(y+yofs);

  if(id<0)
    return;
  if(clk>=0)
  {
    switch (drawstyle)
    {
      case 0:                                               //normal
      overtile8(dest,tile,sx,sy,cs,flip);
      break;
      case 1:                                               //phantom
      overtiletranslucent8(dest,tile,sx,sy,cs,flip,128);
      break;
    }
  }
}

void sprite::drawcloaked(BITMAP* dest)
{
  int sx = real_x(x+xofs);
  int sy = real_y(y+yofs);

  if(id<0)
    return;
  if(clk>=0)
  {
    overtilecloaked16(dest,tile,sx,sy,flip);
  }
  else
  {
    int t  = wpnsbuf[iwSpawn].tile;
    int cs = wpnsbuf[iwSpawn].csets&15;
    if(BSZ)
    {
      if(clk>=-10) ++t;
      if(clk>=-5) ++t;
    }
    else
    {
      if(clk>=-12) ++t;
      if(clk>=-6) ++t;
    }
    overtile16(dest,t,x,sy,cs,0);
  }

  if(get_debug() && key[KEY_O])
    rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
}

void sprite::drawshadow(BITMAP* dest,bool translucent)
{
  if (shadowtile==0)
  {
    return;
  }
  int sx = real_x(x+xofs);
  int sy = real_y(y+yofs);

  if(id<0)
    return;
  if(clk>=0)
  {
    if (translucent)
    {
      overtiletranslucent16(dest,shadowtile,sx,sy,cs,flip,128);
    }
    else
    {
      if (frame&1)
      {
        overtile16(dest,shadowtile,sx,sy,cs,flip);
      }
    }
  }
}

/*
  void sprite::check_conveyor()
  {
  if (conveyclk<=0) {
  int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
  if((ctype>=cCVUP) && (ctype<=cCVRIGHT)) {
  switch (ctype-cCVUP) {
  case up:
  if(!_walkflag(x,y+8-2,2)) {
  y=y-2;
  }
  break;
  case down:
  if(!_walkflag(x,y+15+2,2)) {
  y=y+2;
  }
  break;
  case left:
  if(!_walkflag(x-2,y+8,1)) {
  x=x-2;
  }
  break;
  case right:

  if(!_walkflag(x+15+2,y+8,1)) {
  x=x+2;
  }
  break;
  }
  }
  }
  }
  */

/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

#define SLMAX 255

//class enemy;

sprite_list::sprite_list() : count(0) {}
void sprite_list::clear() { while(count>0) del(0); }

sprite *sprite_list::spr(int index)
{
  if(index<0 || index>=count)
    return NULL;
  return sprites[index];
}

bool sprite_list::swap(int a,int b)
{
  if(a<0 || a>=count || b<0 || b>=count)
    return false;
  sprite *c = sprites[a];
  sprites[a] = sprites[b];
  sprites[b] = c;
  return true;
}

bool sprite_list::add(sprite *s)
{
  if(count>=SLMAX)
  {
    delete s;
    return false;
  }
  sprites[count++]=s;
  return true;
}

bool sprite_list::remove(sprite *s)
  // removes pointer from list but doesn't delete it
{
  int j=0;
  for( ; j<count; j++)
    if(sprites[j] == s)
      goto gotit;

    return false;

gotit:

  for(int i=j; i<count-1; i++)
    sprites[i]=sprites[i+1];

  --count;
  return true;
}

fix sprite_list::getX(int j)
{
  if ((j>=count)||(j<0))
  {
    return (fix)1000000;
  }
  return sprites[j]->x;
}

fix sprite_list::getY(int j)
{
  if ((j>=count)||(j<0))
  {
    return (fix)1000000;
  }
  return sprites[j]->y;
}

int sprite_list::getID(int j)
{
  if ((j>=count)||(j<0))
  {
    return -1;
  }
  return sprites[j]->id;
}

int sprite_list::getMisc(int j)
{
  if ((j>=count)||(j<0))
  {
    return -1;
  }
  return sprites[j]->misc;
}

bool sprite_list::del(int j)
{
  if(j<0||j>=count)
    return false;

  delete sprites[j];
  for(int i=j; i<count-1; i++)
    sprites[i]=sprites[i+1];
  --count;
  return true;
}

void sprite_list::draw(BITMAP* dest,bool lowfirst)
{
  switch(lowfirst)
  {
    case true:
    for(int i=0; i<count; i++)
    {
      sprites[i]->draw(dest);
    }
    break;
    case false:
    for(int i=count-1; i>=0; i--)
    {
      sprites[i]->draw(dest);
    }
    break;
  }
}

void sprite_list::drawshadow(BITMAP* dest,bool translucent, bool lowfirst)
{
  switch(lowfirst)
  {
    case true:
    for(int i=0; i<count; i++)
      sprites[i]->drawshadow(dest,translucent);
    break;
    case false:
    for(int i=count-1; i>=0; i--)
      sprites[i]->drawshadow(dest,translucent);
    break;
  }
}

void sprite_list::draw2(BITMAP* dest,bool lowfirst)
{
  switch(lowfirst)
  {
    case true:
    for(int i=0; i<count; i++)
      sprites[i]->draw2(dest);
    break;
    case false:
    for(int i=count-1; i>=0; i--)
      sprites[i]->draw2(dest);
    break;
  }
}

void sprite_list::drawcloaked2(BITMAP* dest,bool lowfirst)
{
  switch(lowfirst)
  {
    case true:
    for(int i=0; i<count; i++)
      sprites[i]->drawcloaked2(dest);
    break;
    case false:

    for(int i=count-1; i>=0; i--)
      sprites[i]->drawcloaked2(dest);
    break;
  }
}

void sprite_list::animate()
{
  int i=0;
  while(i<count)
  {
    if(!(freeze_guys && sprites[i]->canfreeze))
    {
      if(sprites[i]->animate(i))
      {
        del(i);
        --i;
      }
    }
    ++i;
  }
}

void sprite_list::check_conveyor()
{
  int i=0;
  while(i<count)
  {
    sprites[i]->check_conveyor();
    ++i;
  }
}

int sprite_list::Count() { return count; }

int sprite_list::hit(sprite *s)
{
  for(int i=0; i<count; i++)
    if(sprites[i]->hit(s))
      return i;
    return -1;
}

int sprite_list::hit(int x,int y,int xsize, int ysize)
{
  for(int i=0; i<count; i++)
    if(sprites[i]->hit(x,y,xsize,ysize))
      return i;
    return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id, int mask)
{
  int c=0;
  for(int i=0; i<count; i++)
  {
    if(((sprites[i]->id)&mask) == (id&mask))
    {
      ++c;
    }
  }
  return c;
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id, int mask)
{
  for(int i=0; i<count; i++)
  {
    if(((sprites[i]->id)&mask) == (id&mask))
    {
      return i;
    }
  }
  return -1;
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id, int mask)
{
  for(int i=count-1; i>=0; i--)
  {
    if(((sprites[i]->id)&mask) == (id&mask))
    {
      return i;
    }
  }
  return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id)
{
  return idCount(id,0xFFFF);
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id)
{
  return idFirst(id,0xFFFF);
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id)
{
  return idLast(id,0xFFFF);
}

/**********************************/
/********** Moving Block **********/
/**********************************/

movingblock::movingblock() : sprite()
{
  id=1;
}

void movingblock::draw(BITMAP *dest)
{
  if(clk)
  {
    //    sprite::draw(dest);
    overcombo(dest,real_x(x+xofs),real_y(y+yofs),bcombo ,cs);
  }
}

/*** end of sprite.cc ***/
