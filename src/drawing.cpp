#include "zdefs.h"
#include "drawing.h"
#include "zelda.h"

enum dithType
{
	dithChecker, dithChecker2, dithChecker3,
	dithCrissCross, dithCrissCross2,
	dithCrissCross3, dithCrissCross4,
	dithDiagULDR, dithDiagULDR2,
	dithDiagURDL, dithDiagURDL2,
	dithRow1, dithRow2, dithRow3,
	dithCol1, dithCol2, dithCol3,
	dithDots, dithGrid,
	dithMax
};
static inline bool dithercheck(byte type, int x, int y)
{
	switch(type)
	{
		case dithChecker:
			return (x&1)^(y&1);
		case dithChecker2:
			return (x&2)^(y&2);
		case dithChecker3:
			return (x&4)^(y&4);
		case dithCrissCross:
			return (((x%4)==(y%4)) || ((4-(x%4))==(y%4)));
		case dithCrissCross2:
			return !(((x%4)==(y%4)) || ((4-(x%4))==(y%4)));
		case dithCrissCross3:
			return (((x%8)==(y%8)) || ((8-(x%8))==(y%8)));
		case dithCrissCross4:
			return !(((x%8)==(y%8)) || ((8-(x%8))==(y%8)));
		case dithDiagULDR:
			return ((x%4)==(y%4));
		case dithDiagURDL:
			return ((3-(x%4))==(y%4));
		case dithDiagULDR2:
			return ((x%8)==(y%8));
		case dithDiagURDL2:
			return ((7-(x%8))==(y%8));
		case dithRow1:
			return !(y%2);
		case dithRow2:
			return !(y%4);
		case dithRow3:
			return (y%4);
		case dithCol1:
			return !(x%2);
		case dithCol2:
			return !(x%4);
		case dithCol3:
			return (x%4);
		case dithDots:
			return !(x%2 || y%2);
		case dithGrid:
			return x%2 || y%2;
		
		default: return true;
	}
}

static void ditherblit(BITMAP* dest, BITMAP* src, int color, byte dType)
{
	int wid = zc_min(dest->w, src->w);
	int hei = zc_min(dest->h, src->h);
	for(int tx = 0; tx < wid; ++tx)
	{
		for(int ty = 0; ty < hei; ++ty)
		{
			if(dithercheck(dType,tx,ty) && getpixel(src, tx, ty))
			{
				putpixel(dest, tx, ty, color);
			}
		}
	}
}

void dithercircfill(BITMAP* dest, int x, int y, int rad, int color, int ditherType)
{
	BITMAP* tmp = create_bitmap_ex(8, dest->w, dest->h);
	clear_to_color(tmp, 0);
	circlefill(tmp, x, y, rad, 1);
	ditherblit(dest, tmp, color, (byte)ditherType);
	destroy_bitmap(tmp);
}

