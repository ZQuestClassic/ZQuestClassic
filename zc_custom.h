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
  ls_walk, ls_slash, ls_stab, ls_pound, ls_float, ls_dive,
  ls_landhold1, ls_landhold2, ls_waterhold1, ls_waterhold2, ls_cast,
  ls_swim, ls_jump, ls_charge, ls_max
};
enum {spr_tile, spr_flip, spr_extend};
//enum {cst_tile, cst_extend};
enum {spr_landhold, spr_waterhold};
enum {spr_hold1, spr_hold2};
enum { las_original, las_bszelda, las_zelda3, las_zelda3slow, las_max };

extern int old_floatspr, old_slashspr, linkspr;
extern int walkspr[4][3];                                   //dir,                    tile/flip/extend
extern int stabspr[4][3];                                   //dir,                    tile/flip/extend
extern int slashspr[4][3];                                  //dir,                    tile/flip/extend
extern int floatspr[4][3];                                  //dir,                    tile/flip/extend
extern int swimspr[4][3];                                   //dir,                    tile/flip/extend
extern int divespr[4][3];                                   //dir,                    tile/flip/extend
extern int poundspr[4][3];                                  //dir,                    tile/flip/extend
extern int jumpspr[4][3];                                   //dir,                    tile/flip/extend
extern int chargespr[4][3];                                 //dir,                    tile/flip/extend
extern int castingspr[3];                                   //                        tile/flip/extend
extern int holdspr[2][2][3];                                //     land/water, hands. tile/flip/extend

void linktile(int *tile, int *flip, int state, int dir, int style);
void linktile(int *tile, int *flip, int *extend, int state, int dir, int style);
void setuplinktiles(int style);
void setlinktile(int tile, int flip, int extend, int state, int dir);
#endif
 
