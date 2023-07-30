//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  colors.h
//
//  Palette data for ZQuest Classic.
//
//--------------------------------------------------------

#ifndef _ZC_COLORS_H_
#define _ZC_COLORS_H_

#include "base/zdefs.h"

extern byte *colordata;
extern void create_zc_trans_table(COLOR_MAP *table, AL_CONST PALETTE pal, int32_t r, int32_t g, int32_t b);

// offsets in "data sets"
#define poFULL   0                                          // main palette
#define poLEVEL  15                                         // first level palette
#define poFADE1  15+4                                       //
#define poFADE2  15+7                                       //
#define poFADE3  15+10                                      //
#define poNEWCSETS  15+13                                   // new level specific palettes in csets

// palette data sizes in "data sets"
//The below determines the size of each palette section. 
#define pdFULL   15                                         // number of csets in the full palette
#define pdLEVEL  17                                         // complete level pal (normal & fade pals)
#define pdSPRITE 30                                         // number of sprite csets

//The below defines the size of the fade palette "sets"; AKA it's just saying to loop through csets 2, 3, and 4.
//It is not used for any data storage; the above values handle that. The below is just a convenience variable for
//when the game sets the fading palettes when fading (like screen transitioning, going into a dark room, etc).
//Basically what I'm saying is that if you're trying to save new palettes, you can savely ignore pdFADE. -Deedee
#define pdFADE   3                                          // size of a fade pal

//start of the sprite palettes.
//should be pdFULL + (pdLEVEL*(number of level palettes))
//As of the moment of writing this, there's no definition for the number of level palettes, but currently it's 512.
//I need to find out where it's trying to read from the number of level palettes, make a definition, and replace those with the definition. -Deedee
#define oldpoSPRITE 210                                     // first sprite pal
#define newpoSPRITE 3343                                    // first sprite pal
#define newerpoSPRITE 6671
#define poSPRITE255 8719
#define pSprite(x) ((x)+poSPRITE255)

// total data sets
//The 'total data sets' can be calculated as:
//pdFULL + (pdLEVEL*(number of level palettes)) + pdSPRITE
//As of the moment of writing this, there's no definition for the number of level palettes, but currently it's 512.
//I need to find out where it's trying to read from the number of level palettes, make a definition, and replace those with the definition. -Deedee
#define oldpdTOTAL  240                                     // total data sets
#define newpdTOTAL  3373                                    // total data sets
#define newerpdTOTAL 6701
#define pdTOTAL255 8749                                     // This is the newest. The ones above are named that cause whoever named them that is dumb. -Deedee

// palette data sizes in bytes
#define oldpsTOTAL  ((oldpdTOTAL<<4)*3)
#define newpsTOTAL  ((newpdTOTAL<<4)*3)
#define newerpsTOTAL ((newerpdTOTAL<<4)*3)
#define psTOTAL255 ((pdTOTAL255<<4)*3)

extern byte nes_pal[];
extern byte nes_colors[];

//bool init_colordata();
extern bool init_palnames();
#endif                                                      // _ZC_COLORS_H_
 
