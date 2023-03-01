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

#include "base/zc_alleg.h"
#include "base/zdefs.h"

//extern byte *tilebuf;
extern tiledata *newtilebuf, *grabtilebuf;
extern newcombo *combobuf;
extern int32_t animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern int32_t animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern int32_t animated_combos;
extern int32_t animated_combo_table2[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern int32_t animated_combo_table24[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern int32_t animated_combos2;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool used_tile_table[NEWMAXTILES];                   //keeps track of used tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tile quarters

// in tiles.cc
extern byte unpackbuf[256];
extern const char *tileformat_string[tfMax];
extern comboclass   *combo_class_buf;

void register_blank_tiles();
void register_blank_tiles(int32_t max);
int32_t count_tiles(tiledata *buf);
word count_combos();
void setup_combo_animations();
void reset_combo_animation(newcombo &cmb);
void reset_combo_animation(int32_t c);
void reset_combo_animations();
void setup_combo_animations2();
void reset_combo_animation2(int32_t c);
void reset_combo_animations2();
void animate(newcombo& cdata, bool forceNextFrame = false);
void animate_combos();
bool isonline(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);
void reset_tile(tiledata *buf, int32_t t, int32_t format);
//void clear_tile(tiledata *buf, word tile);
void clear_tiles(tiledata *buf);
void overlay_tile(tiledata *buf,int32_t dest,int32_t src,int32_t cs,bool backwards);
bool copy_tile(tiledata *buf, int32_t src, int32_t dest, bool swap);
bool write_tile(tiledata *buf, BITMAP* src, int32_t dest, int32_t x, int32_t y, bool is8bit, bool overlay);
void unpack_tile(tiledata *buf, int32_t tile, int32_t flip, bool force);

void pack_tile(tiledata *buf, byte *src,int32_t tile);
void pack_tiledata(byte *dest, byte *src, byte format);
void pack_tiles(byte *buf);
int32_t rotate_value(int32_t flip);
void load_tile(byte* buf, int tile);
void load_minitile(byte* buf, int tile, int mini);
byte rotate_walk(byte v);
byte rotate_cset(byte v);

void puttile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void oldputtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void puttile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void oldputtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtileblock16(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, byte skiprows=0);

void putblock8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask);
void overblock8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask);

int32_t combo_tile(const newcombo &c, int32_t x, int32_t y);
int32_t combo_tile(int32_t cmbdat, int32_t x, int32_t y);

void putcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);
void overcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);
void overcomboblock(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h);
void overcombo2(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);

void puttiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void puttiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtilecloaked16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t flip);
void draw_cloaked_sprite(BITMAP* dest,BITMAP* src,int32_t x,int32_t y);

void putblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity);
void overblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity);

void putcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity);
void overcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity);
void overcomboblocktranslucent(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h, int32_t opacity);

bool is_valid_format(byte format);
int32_t tilesize(byte format);
int32_t comboa_lmasktotal(byte layermask);

void a5_draw_tile(int x, int y, int tile, int cs, int cs2, int flip, unsigned char alpha = 255);
#endif                                                      // _ZC_TILES_H_
 
