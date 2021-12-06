#include "zdefs.h"
#include "drawing.h"
#include "zelda.h"
#include "util.h"
#include <allegro/internal/aintern.h>

using namespace util;

static inline bool dithercheck(byte type, byte arg, int32_t x, int32_t y, int32_t wid=256, int32_t hei=168)
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
			double diff = abs(sin((double)((x*double(x))+(y*double(y)))) - (cos((double(x)*y))));
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

void maskblit(BITMAP* dest, BITMAP* src, int32_t color)
{
	int32_t wid = zc_min(dest->w, src->w);
	int32_t hei = zc_min(dest->h, src->h);
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if(bmp_read8(read_addr+tx))
			{
				bmp_write8(write_addr+tx, color);
			}
		}
	}
	bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}

void ditherblit(BITMAP* dest, BITMAP* src, int32_t color, byte dType, byte dArg, int32_t xoffs, int32_t yoffs)
{
	int32_t wid = zc_min(dest->w, src->w);
	int32_t hei = zc_min(dest->h, src->h);
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if(bmp_read8(read_addr+tx) && dithercheck(dType,dArg,tx+xoffs,ty+yoffs,wid,hei))
			{
				bmp_write8(write_addr+tx, color);
			}
		}
	}
	bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}

void dithercircfill(BITMAP* dest, int32_t x, int32_t y, int32_t rad, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs, int32_t yoffs)
{
	BITMAP* tmp = create_bitmap_ex(8, dest->w, dest->h);
	clear_bitmap(tmp);
	circlefill(tmp, x, y, rad, 1);
	ditherblit(dest, tmp, color, ditherType, ditherArg, xoffs, yoffs);
	destroy_bitmap(tmp);
}

void lampcone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color)
{
	int32_t vert[34] = {sx,sy};
	int32_t vertcnt = 11;
	switch(dir)
	{
		case up:
		{
			vert[2] = sx-ceil((17.0/66.0)*range);
			vert[3] = sy-ceil((8.0/66.0)*range);
			//
			vert[4] = vert[2]-ceil((16.0/66.0)*range);
			vert[5] = vert[3]-ceil((31.0/66.0)*range);
			//
			vert[6] = vert[4];
			vert[7] = vert[5]-ceil((4.0/66.0)*range);
			//
			vert[8] = vert[6]+ceil((8.0/66.0)*range);
			vert[9] = vert[7]-ceil((15.0/66.0)*range);
			//
			vert[10] = vert[8]+ceil((15.0/66.0)*range);
			vert[11] = vert[9]-ceil((8.0/66.0)*range);
			//
			vert[12] = vert[10]+ceil((19.0/66.0)*range);
			vert[13] = vert[11];
			//
			vert[14] = vert[12]+ceil((15.0/66.0)*range);
			vert[15] = vert[13]+ceil((8.0/66.0)*range);
			//
			vert[16] = vert[14]+ceil((8.0/66.0)*range);
			vert[17] = vert[15]+ceil((15.0/66.0)*range);
			//
			vert[18] = vert[16];
			vert[19] = vert[17]+ceil((4.0/66.0)*range);
			//
			vert[20] = sx+ceil((17.0/66.0)*range);
			vert[21] = sy-ceil((8.0/66.0)*range);
			break;
		}
		case down:
		{
			vert[2] = sx-ceil((17.0/66.0)*range);
			vert[3] = sy+ceil((8.0/66.0)*range);
			//
			vert[4] = vert[2]-ceil((16.0/66.0)*range);
			vert[5] = vert[3]+ceil((31.0/66.0)*range);
			//
			vert[6] = vert[4];
			vert[7] = vert[5]+ceil((4.0/66.0)*range);
			//
			vert[8] = vert[6]+ceil((8.0/66.0)*range);
			vert[9] = vert[7]+ceil((15.0/66.0)*range);
			//
			vert[10] = vert[8]+ceil((15.0/66.0)*range);
			vert[11] = vert[9]+ceil((8.0/66.0)*range);
			//
			vert[12] = vert[10]+ceil((19.0/66.0)*range);
			vert[13] = vert[11];
			//
			vert[14] = vert[12]+ceil((15.0/66.0)*range);
			vert[15] = vert[13]-ceil((8.0/66.0)*range);
			//
			vert[16] = vert[14]+ceil((8.0/66.0)*range);
			vert[17] = vert[15]-ceil((15.0/66.0)*range);
			//
			vert[18] = vert[16];
			vert[19] = vert[17]-ceil((4.0/66.0)*range);
			//
			vert[20] = sx+ceil((17.0/66.0)*range);
			vert[21] = sy+ceil((8.0/66.0)*range);
			break;
		}
		case left:
		{
			vert[2] = sx-ceil((8.0/66.0)*range);
			vert[3] = sy-ceil((17.0/66.0)*range);
			//
			vert[4] = vert[2]-ceil((31.0/66.0)*range);
			vert[5] = vert[3]-ceil((16.0/66.0)*range);
			//
			vert[6] = vert[4]-ceil((4.0/66.0)*range);
			vert[7] = vert[5];
			//
			vert[8] = vert[6]-ceil((15.0/66.0)*range);
			vert[9] = vert[7]+ceil((8.0/66.0)*range);
			//
			vert[10] = vert[8]-ceil((8.0/66.0)*range);
			vert[11] = vert[9]+ceil((15.0/66.0)*range);
			//
			vert[12] = vert[10];
			vert[13] = vert[11]+ceil((19.0/66.0)*range);
			//
			vert[14] = vert[12]+ceil((8.0/66.0)*range);
			vert[15] = vert[13]+ceil((15.0/66.0)*range);
			//
			vert[16] = vert[14]+ceil((15.0/66.0)*range);
			vert[17] = vert[15]+ceil((8.0/66.0)*range);
			//
			vert[18] = vert[16]+ceil((4.0/66.0)*range);
			vert[19] = vert[17];
			//
			vert[20] = sx-ceil((8.0/66.0)*range);
			vert[21] = sy+ceil((17.0/66.0)*range);
			break;
		}
		case right:
		{
			vert[2] = sx+ceil((8.0/66.0)*range);
			vert[3] = sy-ceil((17.0/66.0)*range);
			//
			vert[4] = vert[2]+ceil((31.0/66.0)*range);
			vert[5] = vert[3]-ceil((16.0/66.0)*range);
			//
			vert[6] = vert[4]+ceil((4.0/66.0)*range);
			vert[7] = vert[5];
			//
			vert[8] = vert[6]+ceil((15.0/66.0)*range);
			vert[9] = vert[7]+ceil((8.0/66.0)*range);
			//
			vert[10] = vert[8]+ceil((8.0/66.0)*range);
			vert[11] = vert[9]+ceil((15.0/66.0)*range);
			//
			vert[12] = vert[10];
			vert[13] = vert[11]+ceil((19.0/66.0)*range);
			//
			vert[14] = vert[12]-ceil((8.0/66.0)*range);
			vert[15] = vert[13]+ceil((15.0/66.0)*range);
			//
			vert[16] = vert[14]-ceil((15.0/66.0)*range);
			vert[17] = vert[15]+ceil((8.0/66.0)*range);
			//
			vert[18] = vert[16]-ceil((4.0/66.0)*range);
			vert[19] = vert[17];
			//
			vert[20] = sx+ceil((8.0/66.0)*range);
			vert[21] = sy+ceil((17.0/66.0)*range);
			break;
		}
		case l_up:
		{
			vertcnt = 17;
			vert[2] = sx+ceil((6.0/66.0)*range);
			vert[3] = sy-ceil((16.0/66.0)*range);
			//
			vert[4] = vert[2];
			vert[5] = vert[3]-ceil((3.0/66.0)*range);
			//
			vert[6] = vert[4]-ceil((4.0/66.0)*range);
			vert[7] = vert[5]-ceil((12.0/66.0)*range);
			//
			vert[8] = vert[6]-ceil((6.0/66.0)*range);
			vert[9] = vert[7]-ceil((18.0/66.0)*range);
			//
			vert[10] = vert[8]-ceil((1.0/66.0)*range);
			vert[11] = vert[9]-ceil((1.0/66.0)*range);
			//
			vert[12] = vert[10]-ceil((14.0/66.0)*range);
			vert[13] = vert[11]-ceil((4.0/66.0)*range);
			//
			vert[14] = vert[12]-ceil((6.0/66.0)*range);
			vert[15] = vert[13];
			//
			vert[16] = vert[14]-ceil((18.0/66.0)*range);
			vert[17] = vert[15]+ceil((4.0/66.0)*range);
			//
			vert[18] = vert[16]-ceil((12.0/66.0)*range);
			vert[19] = vert[17]+ceil((12.0/66.0)*range);
			//
			vert[20] = vert[18]-ceil((4.0/66.0)*range);
			vert[21] = vert[19]+ceil((18.0/66.0)*range);
			//
			vert[22] = vert[20];
			vert[23] = vert[21]+ceil((6.0/66.0)*range);
			//
			vert[24] = vert[22]+ceil((4.0/66.0)*range);
			vert[25] = vert[23]+ceil((14.0/66.0)*range);
			//
			vert[26] = vert[24]+ceil((1.0/66.0)*range);
			vert[27] = vert[25]+ceil((1.0/66.0)*range);
			//
			vert[28] = vert[26]+ceil((18.0/66.0)*range);
			vert[29] = vert[27]+ceil((6.0/66.0)*range);
			//
			vert[30] = vert[28]+ceil((12.0/66.0)*range);
			vert[31] = vert[29]+ceil((4.0/66.0)*range);
			//
			vert[32] = vert[30]+ceil((3.0/66.0)*range);
			vert[33] = vert[31];
			break;
		}
		case r_up:
		{
			vertcnt = 17;
			vert[2] = sx-ceil((6.0/66.0)*range);
			vert[3] = sy-ceil((16.0/66.0)*range);
			//
			vert[4] = vert[2];
			vert[5] = vert[3]-ceil((3.0/66.0)*range);
			//
			vert[6] = vert[4]+ceil((4.0/66.0)*range);
			vert[7] = vert[5]-ceil((12.0/66.0)*range);
			//
			vert[8] = vert[6]+ceil((6.0/66.0)*range);
			vert[9] = vert[7]-ceil((18.0/66.0)*range);
			//
			vert[10] = vert[8]+ceil((1.0/66.0)*range);
			vert[11] = vert[9]-ceil((1.0/66.0)*range);
			//
			vert[12] = vert[10]+ceil((14.0/66.0)*range);
			vert[13] = vert[11]-ceil((4.0/66.0)*range);
			//
			vert[14] = vert[12]+ceil((6.0/66.0)*range);
			vert[15] = vert[13];
			//
			vert[16] = vert[14]+ceil((18.0/66.0)*range);
			vert[17] = vert[15]+ceil((4.0/66.0)*range);
			//
			vert[18] = vert[16]+ceil((12.0/66.0)*range);
			vert[19] = vert[17]+ceil((12.0/66.0)*range);
			//
			vert[20] = vert[18]+ceil((4.0/66.0)*range);
			vert[21] = vert[19]+ceil((18.0/66.0)*range);
			//
			vert[22] = vert[20];
			vert[23] = vert[21]+ceil((6.0/66.0)*range);
			//
			vert[24] = vert[22]-ceil((4.0/66.0)*range);
			vert[25] = vert[23]+ceil((14.0/66.0)*range);
			//
			vert[26] = vert[24]-ceil((1.0/66.0)*range);
			vert[27] = vert[25]+ceil((1.0/66.0)*range);
			//
			vert[28] = vert[26]-ceil((18.0/66.0)*range);
			vert[29] = vert[27]+ceil((6.0/66.0)*range);
			//
			vert[30] = vert[28]-ceil((12.0/66.0)*range);
			vert[31] = vert[29]+ceil((4.0/66.0)*range);
			//
			vert[32] = vert[30]-ceil((3.0/66.0)*range);
			vert[33] = vert[31];
			break;
		}
		case l_down:
		{
			vertcnt = 17;
			vert[2] = sx+ceil((6.0/66.0)*range);
			vert[3] = sy+ceil((16.0/66.0)*range);
			//
			vert[4] = vert[2];
			vert[5] = vert[3]+ceil((3.0/66.0)*range);
			//
			vert[6] = vert[4]-ceil((4.0/66.0)*range);
			vert[7] = vert[5]+ceil((12.0/66.0)*range);
			//
			vert[8] = vert[6]-ceil((6.0/66.0)*range);
			vert[9] = vert[7]+ceil((18.0/66.0)*range);
			//
			vert[10] = vert[8]-ceil((1.0/66.0)*range);
			vert[11] = vert[9]+ceil((1.0/66.0)*range);
			//
			vert[12] = vert[10]-ceil((14.0/66.0)*range);
			vert[13] = vert[11]+ceil((4.0/66.0)*range);
			//
			vert[14] = vert[12]-ceil((6.0/66.0)*range);
			vert[15] = vert[13];
			//
			vert[16] = vert[14]-ceil((18.0/66.0)*range);
			vert[17] = vert[15]-ceil((4.0/66.0)*range);
			//
			vert[18] = vert[16]-ceil((12.0/66.0)*range);
			vert[19] = vert[17]-ceil((12.0/66.0)*range);
			//
			vert[20] = vert[18]-ceil((4.0/66.0)*range);
			vert[21] = vert[19]-ceil((18.0/66.0)*range);
			//
			vert[22] = vert[20];
			vert[23] = vert[21]-ceil((6.0/66.0)*range);
			//
			vert[24] = vert[22]+ceil((4.0/66.0)*range);
			vert[25] = vert[23]-ceil((14.0/66.0)*range);
			//
			vert[26] = vert[24]+ceil((1.0/66.0)*range);
			vert[27] = vert[25]-ceil((1.0/66.0)*range);
			//
			vert[28] = vert[26]+ceil((18.0/66.0)*range);
			vert[29] = vert[27]-ceil((6.0/66.0)*range);
			//
			vert[30] = vert[28]+ceil((12.0/66.0)*range);
			vert[31] = vert[29]-ceil((4.0/66.0)*range);
			//
			vert[32] = vert[30]+ceil((3.0/66.0)*range);
			vert[33] = vert[31];
			break;
		}
		case r_down:
		{
			vertcnt = 17;
			vert[2] = sx-ceil((6.0/66.0)*range);
			vert[3] = sy+ceil((16.0/66.0)*range);
			//
			vert[4] = vert[2];
			vert[5] = vert[3]+ceil((3.0/66.0)*range);
			//
			vert[6] = vert[4]+ceil((4.0/66.0)*range);
			vert[7] = vert[5]+ceil((12.0/66.0)*range);
			//
			vert[8] = vert[6]+ceil((6.0/66.0)*range);
			vert[9] = vert[7]+ceil((18.0/66.0)*range);
			//
			vert[10] = vert[8]+ceil((1.0/66.0)*range);
			vert[11] = vert[9]+ceil((1.0/66.0)*range);
			//
			vert[12] = vert[10]+ceil((14.0/66.0)*range);
			vert[13] = vert[11]+ceil((4.0/66.0)*range);
			//
			vert[14] = vert[12]+ceil((6.0/66.0)*range);
			vert[15] = vert[13];
			//
			vert[16] = vert[14]+ceil((18.0/66.0)*range);
			vert[17] = vert[15]-ceil((4.0/66.0)*range);
			//
			vert[18] = vert[16]+ceil((12.0/66.0)*range);
			vert[19] = vert[17]-ceil((12.0/66.0)*range);
			//
			vert[20] = vert[18]+ceil((4.0/66.0)*range);
			vert[21] = vert[19]-ceil((18.0/66.0)*range);
			//
			vert[22] = vert[20];
			vert[23] = vert[21]-ceil((6.0/66.0)*range);
			//
			vert[24] = vert[22]-ceil((4.0/66.0)*range);
			vert[25] = vert[23]-ceil((14.0/66.0)*range);
			//
			vert[26] = vert[24]-ceil((1.0/66.0)*range);
			vert[27] = vert[25]-ceil((1.0/66.0)*range);
			//
			vert[28] = vert[26]-ceil((18.0/66.0)*range);
			vert[29] = vert[27]-ceil((6.0/66.0)*range);
			//
			vert[30] = vert[28]-ceil((12.0/66.0)*range);
			vert[31] = vert[29]-ceil((4.0/66.0)*range);
			//
			vert[32] = vert[30]-ceil((3.0/66.0)*range);
			vert[33] = vert[31];
			break;
		}
	}
	polygon(dest, vertcnt, vert, color);
}

void ditherLampCone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs, int32_t yoffs)
{
	BITMAP* tmp = create_bitmap_ex(8, dest->w, dest->h);
	clear_bitmap(tmp);
	lampcone(tmp, sx, sy, range, dir, 1);
	ditherblit(dest, tmp, color, ditherType, ditherArg, xoffs, yoffs);
	destroy_bitmap(tmp);
}

void replColor(BITMAP* dest, byte col, byte startCol, byte endCol, bool shift)
{
	int32_t wid = dest->w;
	int32_t hei = dest->h;
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(dest, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			byte c = bmp_read8(read_addr+tx);
			if(c >= startCol && c <= endCol)
			{
				bmp_write8(write_addr+tx, shift ? byte(c+col) : col);
			}
		}
	}
	bmp_unwrite_line(dest);
}

