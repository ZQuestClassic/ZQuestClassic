//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  tiles.h
//
//  Tile drawing routines for ZC.
//
//  !! Don't use them on "screen"; use memory BITMAPs only.
//
//--------------------------------------------------------

#ifndef _ZC_TILES_H_
#define _ZC_TILES_H_

#define UNPACKSIZE 256

#include "zc_alleg.h"
#include "zdefs.h"

//extern byte *tilebuf;
extern newcombo *combobuf;

// in tiles.cc
extern byte unpackbuf[UNPACKSIZE];
extern char *tileformat_string[tfMax];

void register_blank_tiles();
word count_tiles(tiledata *buf);
word count_combos();
void setup_combo_animations();
void reset_combo_animation(int c);
void reset_combo_animations();
void setup_combo_animations2();
void reset_combo_animation2(int c);
void reset_combo_animations2();
void animate_combos();
bool isonline(float x1, float y1, float x2, float y2, float x3, float y3);
void clear_tile(tiledata *buf, word tile);
void clear_tiles(tiledata *buf);
void unpack_tile(tiledata *buf, int tile, int flip, bool force);

void pack_tile(tiledata *buf, byte *src,int tile);
void pack_tiledata(byte *dest, byte *src, byte format);
void pack_tiles(byte *buf);
int rotate_value(int flip);

void puttile8(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void oldputtile8(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void overtile8(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void puttile16(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void oldputtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void overtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip);

void putblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask);
void oldputblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask);
void overblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask);

int combo_tile(newcombo c, int x, int y);
int combo_tile(int cmbdat, int x, int y);

void putcombo(BITMAP* dest,int x,int y,int cmbdat,int cset);
void oldputcombo(BITMAP* dest,int x,int y,int cmbdat,int cset);
void overcombo(BITMAP* dest,int x,int y,int cmbdat,int cset);
void overcombo2(BITMAP* dest,int x,int y,int cmbdat,int cset);

void puttiletranslucent8(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity);
void overtiletranslucent8(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity);
void puttiletranslucent16(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity);
void overtiletranslucent16(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity);
void overtilecloaked16(BITMAP* dest,int tile,int x,int y,int flip);

void putblocktranslucent8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask,int opacity);
void overblocktranslucent8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask,int opacity);

void putcombotranslucent(BITMAP* dest,int x,int y,int cmbdat,int cset,int opacity);
void overcombotranslucent(BITMAP* dest,int x,int y,int cmbdat,int cset,int opacity);

int tilesize(byte format);
int comboa_lmasktotal(byte layermask);
#endif                                                      // _ZC_TILES_H_
