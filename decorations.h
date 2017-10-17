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

#ifndef _DECORATIONS_H_
#define _DECORATIONS_H_

/***************************************/
/*******  Decoration Base Class  *******/
/***************************************/

class decoration : public sprite
{
public:
  decoration(fix X,fix Y,int Id,int Clk);
  virtual ~decoration();
};

/*******************************/
/*******   Decorations   *******/
/*******************************/

class dBushLeaves : public decoration
{
public:
  int ft[4][8][3];                                        //[leaf][frame][x/y/flip]
  int ox, oy;
  dBushLeaves(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dFlowerClippings : public decoration
{
public:
  int ft[4][8][3];                                        //[leaf][frame][x/y/flip]
  int ox, oy;
  dFlowerClippings(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dGrassClippings : public decoration
{
public:
  int ft[3][4][4];                                        //[leaf][frame][x/y/flip/size(0=large, 1=small)]
  int ox, oy;
  dGrassClippings(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dHammerSmack : public decoration
{
public:
  int ft[2][4][3];                                        //[leaf][frame][x/y/icon(0=spark, 1=star)]
  int ox, oy, wpnid;
  dHammerSmack(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dTallGrass : public decoration
{
public:
  dTallGrass(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dRipples : public decoration
{
public:
  dRipples(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dHover : public decoration
{
  int wpnid;
public:
  dHover(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void draw(BITMAP *dest);
};

class dNayrusLoveShield : public decoration
{
public:
  dNayrusLoveShield(fix X,fix Y,int Id,int Clk);
  virtual bool animate(int index);
  virtual void realdraw(BITMAP *dest, int draw_what);
  virtual void draw(BITMAP *dest);
  virtual void draw2(BITMAP *dest);
};
#endif
/*** end of sprite.cc ***/
 
