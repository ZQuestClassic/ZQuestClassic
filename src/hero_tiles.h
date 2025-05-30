#ifndef ZC_HERO_TILES_H_
#define ZC_HERO_TILES_H_

#include "base/zdefs.h"

enum
{
    ls_walk, ls_slash, ls_stab, ls_pound, ls_float, ls_dive,
    ls_landhold1, ls_landhold2, ls_waterhold1, ls_waterhold2, ls_cast,
    ls_swim, ls_jump, ls_charge, ls_drown, ls_falling, ls_lavadrown, 
    ls_sideswim, ls_sideswimslash, ls_sideswimstab, ls_sideswimpound, 
    ls_sideswimcharge, ls_sidewaterhold1, ls_sidewaterhold2, 
    ls_sideswimcast, ls_sidedrown, ls_revslash, ls_lifting, ls_liftwalk, ls_max
};
enum {spr_tile, spr_flip, spr_extend, spr_frames};
//enum {cst_tile, cst_extend};
enum {spr_landhold, spr_waterhold};
enum {spr_hold1, spr_hold2, spr_holdsword, spr_holdmax};
enum { las_original, las_bszelda, las_zelda3, las_zelda3slow, las_max };

extern int32_t script_hero_sprite;
extern int32_t script_hero_cset;
extern int32_t script_hero_flip;

extern int32_t hero_animation_speed;
extern int32_t liftspeed;

extern byte hero_defenses[wMax];

extern int32_t hammeroffsets[4];

extern int32_t old_floatspr, old_slashspr, herospr;
extern int32_t walkspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t stabspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t slashspr[4][3];                                  //dir,                           tile/flip/extend
extern int32_t revslashspr[4][3];                               //dir,                     tile/flip/extend
extern int32_t floatspr[4][3];                                  //dir,                           tile/flip/extend
extern int32_t swimspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t divespr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t poundspr[4][3];                                  //dir,                           tile/flip/extend
extern int32_t jumpspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t chargespr[4][3];                                 //dir,                           tile/flip/extend
extern int32_t castingspr[3];                                   //                               tile/flip/extend
extern int32_t sideswimcastingspr[3];                           //                               tile/flip/extend
extern int32_t holdspr[2][3][3];                                //land/water, 1hand/2hand/sword, tile/flip/extend
extern int32_t frozenspr[4][3];                                 //dir,                           tile/flip/extend
extern int32_t frozen_waterspr[4][3];                           //dir,                           tile/flip/extend
extern int32_t onfirespr[4][3];                                 //dir,                           tile/flip/extend
extern int32_t onfire_waterspr[4][3];                           //dir,                           tile/flip/extend
extern int32_t diggingspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t usingrodspr[4][3];                               //dir,                           tile/flip/extend
extern int32_t usingcanespr[4][3];                              //dir,                           tile/flip/extend
extern int32_t pushingspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t liftingspr[4][4];                                //dir,                           tile/flip/extend/frames
extern int32_t liftingwalkspr[4][3];                            //dir,                           tile/flip/extend
extern int32_t stunnedspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t stunned_waterspr[4][3];                          //dir,                           tile/flip/extend
extern int32_t drowningspr[4][3];                               //dir,                           tile/flip/extend
extern int32_t sidedrowningspr[4][3];                           //dir,                           tile/flip/extend
extern int32_t drowning_lavaspr[4][3];                          //dir,                           tile/flip/extend
extern int32_t sideswimspr[4][3];                               //dir,                           tile/flip/extend
extern int32_t sideswimslashspr[4][3];                          //dir,                           tile/flip/extend
extern int32_t sideswimstabspr[4][3];                           //dir,                           tile/flip/extend
extern int32_t sideswimpoundspr[4][3];                          //dir,                           tile/flip/extend
extern int32_t sideswimchargespr[4][3];                         //dir,                           tile/flip/extend
extern int32_t sideswimholdspr[3][3];                           //1hand/2hand/sword,             tile/flip/extend
extern int32_t fallingspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t shockedspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t shocked_waterspr[4][3];                          //dir,                           tile/flip/extend
extern int32_t pullswordspr[4][3];                              //dir,                           tile/flip/extend
extern int32_t readingspr[4][3];                                //dir,                           tile/flip/extend
extern int32_t slash180spr[4][3];                               //dir,                           tile/flip/extend
extern int32_t slashZ4spr[4][3];                                //dir,                           tile/flip/extend
extern int32_t dashspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t bonkspr[4][3];                                   //dir,                           tile/flip/extend
extern int32_t medallionsprs[3][3];                             //medallion,                     tile/flip/extend

void herotile(int32_t *tile, int32_t *flip, int32_t state, int32_t dir, int32_t style);
void herotile(int32_t *tile, int32_t *flip, int32_t *extend, int32_t state, int32_t dir, int32_t style);
void setupherotiles(int32_t style);
void setherotile(int32_t tile, int32_t flip, int32_t extend, int32_t state, int32_t dir);
void setupherodefenses();
void setupherooffsets();

#endif
