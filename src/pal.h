//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  pal.h
//
//  Palette code for ZC.
//
//--------------------------------------------------------

#ifndef _ZC_PAL_H_
#define _ZC_PAL_H_

#include "zc_alleg.h"
#include "zdefs.h"
#include "colors.h"

extern int CSET_SIZE;                                       // this is only changed to 4 in the NES title screen
extern int CSET_SHFT;                                       // log2 of CSET_SIZE

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

extern RGB _RGB(byte *si);
extern RGB _RGB(int r,int g,int b);
extern RGB invRGB(RGB s);
extern RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio);

extern void copy_pal(RGB *src,RGB *dest);
extern void loadfullpal();
extern void loadlvlpal(int level);
extern void loadpalset(int cset,int dataset);
extern void loadfadepal(int dataset);
extern void interpolatedfade();

extern void ringcolor(bool forceDefault);
extern void fade(int level,bool blackall,bool fromblack);
extern void lighting(bool existslight, bool setnaturaldark = false, int specialstate = pal_litRESET);
extern void lightingInstant();
extern void dryuplake();
extern void rehydratelake(bool instant);
extern void reset_pal_cycling();
extern void cycle_palette();

INLINE RGB NESpal(int i)
{
    return _RGB(nes_pal+i*3);
}
extern int reverse_NESpal(RGB c);

#endif                                                      // _ZC_PAL_H_

