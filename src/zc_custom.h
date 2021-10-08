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

#include "zdefs.h"

enum
{
    ls_walk, ls_slash, ls_stab, ls_pound, ls_float, ls_dive,
    ls_landhold1, ls_landhold2, ls_waterhold1, ls_waterhold2, ls_cast,
    ls_swim, ls_jump, ls_charge, ls_drown, ls_falling, ls_lavadrown, 
    ls_sideswim, ls_sideswimslash, ls_sideswimstab, ls_sideswimpound, 
    ls_sideswimcharge, ls_sidewaterhold1, ls_sidewaterhold2, 
    ls_sideswimcast, ls_sidedrown, ls_max
};
enum {spr_tile, spr_flip, spr_extend};
//enum {cst_tile, cst_extend};
enum {spr_landhold, spr_waterhold};
enum {spr_hold1, spr_hold2, spr_holdsword, spr_holdmax};
enum { las_original, las_bszelda, las_zelda3, las_zelda3slow, las_max };

extern int script_link_sprite;
extern int script_link_cset;
extern int script_link_flip;

extern byte link_defence[wMax];

extern int hammeroffsets[4];

extern int old_floatspr, old_slashspr, linkspr;
extern int walkspr[4][3];                                   //dir,                           tile/flip/extend
extern int stabspr[4][3];                                   //dir,                           tile/flip/extend
extern int slashspr[4][3];                                  //dir,                           tile/flip/extend
extern int floatspr[4][3];                                  //dir,                           tile/flip/extend
extern int swimspr[4][3];                                   //dir,                           tile/flip/extend
extern int divespr[4][3];                                   //dir,                           tile/flip/extend
extern int poundspr[4][3];                                  //dir,                           tile/flip/extend
extern int jumpspr[4][3];                                   //dir,                           tile/flip/extend
extern int chargespr[4][3];                                 //dir,                           tile/flip/extend
extern int castingspr[3];                                   //                               tile/flip/extend
extern int sideswimcastingspr[3];                           //                               tile/flip/extend
extern int holdspr[2][3][3];                                //land/water, 1hand/2hand/sword, tile/flip/extend
extern int frozenspr[4][3];                                 //dir,                           tile/flip/extend
extern int frozen_waterspr[4][3];                           //dir,                           tile/flip/extend
extern int onfirespr[4][3];                                 //dir,                           tile/flip/extend
extern int onfire_waterspr[4][3];                           //dir,                           tile/flip/extend
extern int diggingspr[4][3];                                //dir,                           tile/flip/extend
extern int usingrodspr[4][3];                               //dir,                           tile/flip/extend
extern int usingcanespr[4][3];                              //dir,                           tile/flip/extend
extern int pushingspr[4][3];                                //dir,                           tile/flip/extend
extern int liftingspr[4][3];                                //dir,                           tile/flip/extend
extern int liftingheavyspr[4][3];                           //dir,                           tile/flip/extend
extern int stunnedspr[4][3];                                //dir,                           tile/flip/extend
extern int stunned_waterspr[4][3];                          //dir,                           tile/flip/extend
extern int drowningspr[4][3];                               //dir,                           tile/flip/extend
extern int sidedrowningspr[4][3];                               //dir,                           tile/flip/extend
extern int drowning_lavaspr[4][3];                          //dir,                           tile/flip/extend
extern int sideswimspr[4][3];                               //dir,                           tile/flip/extend
extern int sideswimslashspr[4][3];                          //dir,                           tile/flip/extend
extern int sideswimstabspr[4][3];                           //dir,                           tile/flip/extend
extern int sideswimpoundspr[4][3];                          //dir,                           tile/flip/extend
extern int sideswimchargespr[4][3];                         //dir,                           tile/flip/extend
extern int sideswimholdspr[3][3];                           //1hand/2hand/sword,             tile/flip/extend
extern int fallingspr[4][3];                                //dir,                           tile/flip/extend
extern int shockedspr[4][3];                                //dir,                           tile/flip/extend
extern int shocked_waterspr[4][3];                          //dir,                           tile/flip/extend
extern int pullswordspr[4][3];                              //dir,                           tile/flip/extend
extern int readingspr[4][3];                                //dir,                           tile/flip/extend
extern int slash180spr[4][3];                               //dir,                           tile/flip/extend
extern int slashZ4spr[4][3];                                //dir,                           tile/flip/extend
extern int dashspr[4][3];                                   //dir,                           tile/flip/extend
extern int bonkspr[4][3];                                   //dir,                           tile/flip/extend
extern int medallionsprs[3][3];                             //medallion,                     tile/flip/extend

void linktile(int *tile, int *flip, int state, int dir, int style);
void linktile(int *tile, int *flip, int *extend, int state, int dir, int style);
void setuplinktiles(int style);
void setlinktile(int tile, int flip, int extend, int state, int dir);
void setuplinkdefenses();
void setuplinkoffsets();
#endif

