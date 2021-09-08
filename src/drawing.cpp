#include "zdefs.h"
#include "drawing.h"
#include "zelda.h"
#include "util.h"
using namespace util;

enum dithType
{
	//Every odd index is the inverted of the index before it, always.
	//Yeah this could be seen as a bit janky, but it makes the switch-case
	//simpler, and ensures no inverted version is forgotten. -Em
	dithChecker, dithCheckerInv,
	dithCrissCross, dithCrissCrossInv,
	dithDiagULDR, dithDiagULDRInv,
	dithDiagURDL, dithDiagURDLInv,
	dithRow, dithRowInv,
	dithCol, dithColInv,
	dithDots, dithDotsInv,
	dithGrid, dithGridInv,
	dithStatic, dithStaticInv,
	dithStatic2, dithStatic2Inv,
	dithStatic3, dithStatic3Inv,
	dithMax
};
static inline bool dithercheck(byte type, byte arg, int x, int y, int wid=256, int hei=168)
{
	bool ret = false,
	     inv = (type%2); //invert return for odd types
	type &= ~1; //even-ize the type for switch simplicity
	
	switch(type) //arg bounding, prevent "/0" and "%0"
	{
		case dithStatic: case dithStatic2: case dithStatic3:
			break; //range is full byte, no bounding needed
		default:
			arg = zc_max(1,arg); //don't div by 0!
			break;
	}
	
	switch(type) //calculate
	{
		case dithChecker:
			ret = (((x/arg)&1)^((y/arg)&1));
			break;
		case dithCrissCross:
			ret = (((x%arg)==(y%arg)) || ((arg-(x%arg))==(y%arg)));
			break;
		case dithDiagULDR: 
			ret = ((x%arg)==(y%arg));
			break;
		case dithDiagURDL:
			ret = (((arg-1)-(x%arg))==(y%arg));
			break;
		case dithRow:
			ret = !(y%arg);
			break;
		case dithCol:
			ret = !(x%arg);
			break;
		case dithDots:
			ret = !(x%arg || y%arg);
			break;
		case dithGrid:
			ret = !(x%arg) || !(y%arg);
			break;
		case dithStatic2: //changes centering of the formula
		{
			x-=(wid/2);
			y-=(hei/2);
			goto dthr_static;
		}
		case dithStatic3: //changes centering of the formula
		{
			x+=(wid/2);
			y+=(hei/2);
			goto dthr_static;
		}
		case dithStatic:
		{
		dthr_static:
			double diff = abs(sin((double)((x*x)+(y*y))) - (cos((double)(x*y))));
			double filt = ((arg/255.0)*(2000))/1000.0;
			ret = diff < filt;
			break;
		}
		
		default:
			//don't dither if invalid type found,
			//just draw every pixel -Em
			return true;
	}
	return ret^inv;
}

static void ditherblit(BITMAP* dest, BITMAP* src, int color, byte dType, byte dArg)
{
	int wid = zc_min(dest->w, src->w);
	int hei = zc_min(dest->h, src->h);
	for(int tx = 0; tx < wid; ++tx)
	{
		for(int ty = 0; ty < hei; ++ty)
		{
			if(getpixel(src, tx, ty) && dithercheck(dType,dArg,tx,ty,wid,hei))
			{
				putpixel(dest, tx, ty, color);
			}
		}
	}
}

void dithercircfill(BITMAP* dest, int x, int y, int rad, int color, int ditherType, int ditherArg)
{
	BITMAP* tmp = create_bitmap_ex(8, dest->w, dest->h);
	clear_to_color(tmp, 0);
	circlefill(tmp, x, y, rad, 1);
	ditherblit(dest, tmp, color, (byte)ditherType, (byte)ditherArg);
	destroy_bitmap(tmp);
}

