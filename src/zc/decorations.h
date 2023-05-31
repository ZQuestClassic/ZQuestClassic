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

#include "zfix.h"
#include "sprite.h"
/***************************************/
/*******  Decoration Base Class  *******/
/***************************************/

class decoration : public sprite
{
public:
	decoration(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	int32_t the_deco_sprite; 
	virtual ~decoration();
};

/*******************************/
/*******   Decorations   *******/
/*******************************/

class comboSprite : public decoration
{
public:
    int32_t timer;
    bool initialized;
	int32_t tframes, spd;
    comboSprite(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
    virtual bool animate(int32_t index);
    virtual void draw(BITMAP *dest);
    virtual void draw2(BITMAP *dest);
	virtual void realdraw(BITMAP *dest, int32_t draw_what);
	
};

class dBushLeaves : public decoration
{
public:
	static int32_t ft[4][8][3]; //[leaf][frame][x/y/flip]
	int32_t ox, oy;
	dBushLeaves(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dFlowerClippings : public decoration
{
public:
	static int32_t ft[4][8][3]; //[leaf][frame][x/y/flip]
	int32_t ox, oy;
	dFlowerClippings(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dGrassClippings : public decoration
{
public:
	static int32_t ft[3][4][4]; //[leaf][frame][x/y/flip/size(0=large, 1=small)]
	int32_t ox, oy;
	dGrassClippings(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dHammerSmack : public decoration
{
public:
	static int32_t ft[2][4][3]; //[leaf][frame][x/y/icon(0=spark, 1=star)]
	int32_t ox, oy, wpnid;
	dHammerSmack(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dTallGrass : public decoration
{
public:
	dTallGrass(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dRipples : public decoration
{
public:
	dRipples(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dHover : public decoration
{
	int32_t wpnid;
public:
	dHover(zfix X,zfix Y,int32_t Id,int32_t Clk, int32_t wpnSpr=0);
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
};

class dDivineProtectionShield : public decoration
{
public:
	dDivineProtectionShield(zfix X,zfix Y,int32_t Id,int32_t Clk);
	virtual bool animate(int32_t index);
	virtual void realdraw(BITMAP *dest, int32_t draw_what);
	virtual void draw(BITMAP *dest);
	virtual void draw2(BITMAP *dest);
};
#endif
