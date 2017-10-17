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

#include "zc_alleg.h"
#include "zdefs.h"

extern byte *tilebuf;
extern newcombo *combobuf;

// in tiles.cc
extern byte unpackbuf[256];

void register_blank_tiles();
word count_tiles(byte *buf);
word count_combos();
void setup_combo_animations();
void reset_combo_animations();
void animate_combos();
void clear_tiles(byte *buf);
//void unpack_tile(int tile);
void unpack_tile(int tile, int flip, bool force);

void pack_tile(byte *src,int tile);
void pack_tiles(byte *buf);
int rotate_value(int flip);

void puttile8(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void overtile8(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void puttile16(BITMAP* dest,int tile,int x,int y,int cset,int flip);
void overtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip);

void putblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask);
void overblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask);

void putcombo(BITMAP* dest,int x,int y,int cmbdat,int cset);
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
#endif                                                      // _ZC_TILES_H_
