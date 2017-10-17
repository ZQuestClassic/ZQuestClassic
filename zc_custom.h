//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_custom.cc
//
//  Custom item, enemy, etc. for Zelda.
//
//--------------------------------------------------------

#ifndef _ZC_CUSTOM_H_
#define _ZC_CUSTOM_H_

enum
{
  ls_walk, ls_jump, ls_slash, ls_stab, ls_pound, ls_swim, ls_dive,
  ls_hold1, ls_hold2, ls_swimhold1, ls_swimhold2, ls_cast
};
enum {spr_tile, spr_flip};
enum {spr_standhold, spr_swimhold};
enum {spr_hold1, spr_hold2};
enum { las_original, las_bszelda, las_max };

extern int swimspr, slashspr, linkspr;
extern int walkspr[4][2];                                   //dir, tile/flip
extern int stabspr[4][2];                                   //dir, tile/flip
extern int slashspr2[4][2];                                 //dir, tile/flip
extern int swimspr2[4][2];                                  //dir, tile/flip
extern int divespr[4][2];                                   //dir, tile/flip
extern int poundspr[4][2];                                  //dir, tile/flip
extern int castingspr;
extern int holdspr[2][2];                                   //stand/swim, tile/flip

void linktile(int *tile, int *flip, int state, int dir, int style);
void setuplinktiles(int style);
#endif
