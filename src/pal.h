#ifndef ZC_PAL_H_
#define ZC_PAL_H_

#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/colors.h"

extern int32_t CSET_SIZE;                                       // this is only changed to 4 in the NES title screen
extern int32_t CSET_SHFT;                                       // log2 of CSET_SIZE

#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14

extern RGB _RGB(byte *si);
extern RGB _RGB(int32_t r,int32_t g,int32_t b);
extern RGB invRGB(RGB s);
extern RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio);

extern void copy_pal(RGB *src,RGB *dest);
extern void loadfullpal();
extern void loadlvlpal(int32_t level);
extern void loadpalset(int32_t cset,int32_t dataset,bool update_tint=true);
extern void loadfadepal(int32_t dataset);
extern void interpolatedfade();

extern void ringcolor(bool forceDefault);
extern void fade(int32_t level,bool blackall,bool fromblack);
extern void lighting(bool existslight, bool setnaturaldark = false, int32_t specialstate = pal_litRESET);
extern void lightingInstant();
extern bool get_lights();
extern void set_lights(bool state, int32_t specialstate = pal_litRESET);
extern void toggle_lights(int32_t specialstate = pal_litRESET);
extern void dryuplake();
extern void rehydratelake(bool instant);
extern void reset_pal_cycling();
extern void cycle_palette();

INLINE RGB NESpal(int32_t i)
{
	static bool has_converted;
	if (!has_converted)
	{
		for (int i = 0; i < 64*3; i++)
			nes_pal[i] = _rgb_scale_6[nes_pal[i]];
		has_converted = true;
	}
    return _RGB(nes_pal+i*3);
}
extern int32_t reverse_NESpal(RGB c);

#endif                                                      // _ZC_PAL_H_
