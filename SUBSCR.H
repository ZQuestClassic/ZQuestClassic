//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef _SUBSCR_H_
#define _SUBSCR_H_

#include "sprite.h"
#include "items.h"

extern bool show_subscreen_dmap_dots;
extern bool show_subscreen_numbers;
extern bool show_subscreen_items;
extern bool show_subscreen_life;

void blueframe(BITMAP *dest,int x,int y,int xsize,int ysize);
void drawgrid(BITMAP *dest,int c1,int c2);
void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void drawdmap(BITMAP *dest);
void lifemeter(BITMAP *dest,int x,int y);
void magicmeter(BITMAP *dest,int x,int y);
void putxnum(BITMAP *dest,int x,int y,int num);
inline void putdot(BITMAP *dest,int x,int y,int c)
{
  rectfill(dest,x,y,x+2,y+2,c);
}

/****  Subscr items code  ****/

extern item *Bitem, *Aitem;
extern int   Bid, Aid;
const byte tripiece[8*3] =
{
  112,112,0, 128,112,1, 96,128,0, 144,128,1,
  112,128,2, 112,128,1, 128,128,3, 128,128,0
};
const byte bmap[16*2] =
{
  0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,
  0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0
};
const int fringe[8] = { 6,2,4,7,6,8,7,5 };
const byte bmaptiles[8*5] =
{
  0, 1, 2, 3, 2, 3, 3, 4,
  20,21,22,23,22,23,23,24,
  20,21,22,23,22,23,23,24,
  20,21,22,23,22,23,23,24,
  40,41,42,43,42,43,43,44
};

void reset_subscr_items();
void update_subscr_items();
int stripspaces(char *source, char *target, int stop);
void putsubscr(BITMAP *dest,int x,int y,bool showtime=false);
void puttriframe(BITMAP *dest);
void markBmap(int dir);
void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void putBmap(BITMAP *dest);
void load_Sitems();
void put_topsubscr();
void update_topsubscr(int y);
void dosubscr();
#endif

/*** end of subscr.cc ***/
