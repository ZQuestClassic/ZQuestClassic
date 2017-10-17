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
extern tiledata *newtilebuf, *grabtilebuf;
extern newcombo *combobuf;
extern word animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos;
extern word animated_combo_table2[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table24[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos2;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool used_tile_table[NEWMAXTILES];                   //keeps track of used tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tile quarters

// in tiles.cc
extern byte unpackbuf[UNPACKSIZE];
extern const char *tileformat_string[tfMax];
extern comboclass   *combo_class_buf;

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
bool isonline(long x1, long y1, long x2, long y2, long x3, long y3);
void reset_tile(tiledata *buf, int t, int format);
//void clear_tile(tiledata *buf, word tile);
void clear_tiles(tiledata *buf);
void overlay_tile(tiledata *buf,int dest,int src,int cs,bool backwards);
bool copy_tile(tiledata *buf, int src, int dest, bool swap);
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
void overcomboblock(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h);
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
void overcomboblocktranslucent(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h, int opacity);

int tilesize(byte format);
int comboa_lmasktotal(byte layermask);
#endif                                                      // _ZC_TILES_H_
 
