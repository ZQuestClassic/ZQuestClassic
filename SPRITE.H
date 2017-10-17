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

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "zc_alleg.h"
#include "zdefs.h"
// this code needs some patching for use in zquest.cc

extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;
extern bool     freeze_guys;
extern int fadeclk;
extern int frame;
extern bool BSZ;
extern int conveyclk;

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

class sprite
{
  public:
    fix x,y;
    int tile,shadowtile,cs,flip,clk,misc;
    fix xofs,yofs;
    int hxofs,hyofs,hxsz,hysz;
    int id,dir;
    bool angular,canfreeze;
    double angle;
    int lasthit, lasthitclk;
    int dummy_int[10];
    bool dummy_bool[10];
    int drawstyle;                                          //0=normal, 1=translucent, 2=cloaked

    sprite();
    sprite(fix X,fix Y,int T,int CS,int F,int Clk,int Yofs);
    virtual ~sprite();
    virtual void draw(BITMAP* dest);                        // main layer
    virtual void draw8(BITMAP* dest);                       // main layer
    virtual void drawcloaked(BITMAP* dest);                 // main layer
    virtual void drawshadow(BITMAP* dest, bool translucent);// main layer
    virtual void draw2(BITMAP* dest);                       // top layer for special needs
    virtual void drawcloaked2(BITMAP* dest);                // top layer for special needs
    virtual bool animate(int index);
    virtual void check_conveyor();
    int real_x(fix fx);
    int real_y(fix fy);
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int txsz,int tysz);


    virtual int hitdir(int tx,int ty,int txsz,int tysz,int dir);
    virtual void move(fix dx,fix dy);
    virtual void move(fix s);
};

/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

#define SLMAX 255

class sprite_list
{
  sprite *sprites[SLMAX];
  int count;

  public:
    sprite_list();
    void clear();
    sprite *spr(int index);
    bool swap(int a,int b);
    bool add(sprite *s);
    // removes pointer from list but doesn't delete it
    bool remove(sprite *s);
    fix getX(int j);
    fix getY(int j);
    int getID(int j);
    int getMisc(int j);
    bool del(int j);
    void draw(BITMAP* dest,bool lowfirst);
    void drawshadow(BITMAP* dest,bool translucent, bool lowfirst);
    void draw2(BITMAP* dest,bool lowfirst);
    void drawcloaked2(BITMAP* dest,bool lowfirst);
    void animate();
    void check_conveyor();
    int Count();
    int hit(sprite *s);
    int hit(int x,int y,int xsize, int ysize);
    // returns the number of sprites with matching id
    int idCount(int id, int mask);
    // returns index of first sprite with matching id, -1 if none found
    int idFirst(int id, int mask);
    // returns index of last sprite with matching id, -1 if none found
    int idLast(int id, int mask);
    // returns the number of sprites with matching id
    int idCount(int id);
    // returns index of first sprite with matching id, -1 if none found
    int idFirst(int id);
    // returns index of last sprite with matching id, -1 if none found
    int idLast(int id);
};

/**********************************/
/********** Moving Block **********/
/**********************************/

class movingblock : public sprite
{
  public:
    int bcombo;
    int oldflag;
    int endx, endy;
    bool trigger;
    byte undercset;

    movingblock();
    void push(fix bx,fix by,int d,int f);
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
};
#endif
/*** end of sprite.cc ***/
