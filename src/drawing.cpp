#include "base/zdefs.h"
#include "drawing.h"
#include "base/util.h"
#include "base/zc_math.h"
#include "base/combo.h"
#include "zc/replay.h"
#include "zc/zelda.h"
#include <allegro/internal/aintern.h>
#include <cstdlib>

using namespace util;

#ifdef IS_PLAYER
extern BITMAP *darkscr_bmp_curscr, *darkscr_bmp_curscr_trans, *darkscr_bmp_scrollscr, *darkscr_bmp_scrollscr_trans, *darkscr_bmp_z3, *darkscr_bmp_z3_trans;

#include "base/initdata.h"
#include "gamedata.h"
#define DITH_PERC (game ? game->get_dither_perc() : zinit.dither_percent)
#define TRANS_PERC (game ? game->get_transdark_perc() : zinit.transdark_percent)
#define DITH_TYPE (game ? game->get_dither_type() : zinit.dither_type)
#define DITH_ARG (game ? game->get_dither_arg() : zinit.dither_arg)
//end IS_PLAYER
#elif defined(IS_EDITOR)

#include "base/initdata.h"
#include "gamedata.h"
#define DITH_PERC (game ? game->get_dither_perc() : zinit.dither_percent)
#define TRANS_PERC (game ? game->get_transdark_perc() : zinit.transdark_percent)
#define DITH_TYPE (game ? game->get_dither_type() : zinit.dither_type)
#define DITH_ARG (game ? game->get_dither_arg() : zinit.dither_arg)
#else
#define DITH_PERC 0
#define TRANS_PERC 0
#define DITH_TYPE 0
#define DITH_ARG 0
#endif

extern COLOR_MAP trans_table2;

void doDarkroomCircle(int32_t cx, int32_t cy, byte glowRad,BITMAP* dest, BITMAP* transdest, int dith_perc, int trans_perc, int dith_type, int dith_arg)
{
	if(!glowRad) return;

#ifdef IS_PLAYER
	cx -= viewport.x;
	cy -= viewport.y;
	//Default bitmap handling
	if(!dest) dest = darkscr_bmp_curscr;
	if(dest == darkscr_bmp_scrollscr) transdest = darkscr_bmp_scrollscr_trans;
	else if(dest == darkscr_bmp_z3) transdest = darkscr_bmp_z3_trans;
	else if(!transdest || dest == darkscr_bmp_curscr) transdest = darkscr_bmp_curscr_trans;
#endif

	if(dith_perc < 0) dith_perc = DITH_PERC;
	if(trans_perc < 0) trans_perc = TRANS_PERC;
	if(dith_type < 0) dith_type = DITH_TYPE;
	if(dith_arg < 0) dith_arg = DITH_ARG;
	
	int32_t ditherRad = glowRad + (int32_t)(glowRad * (dith_perc/(double)100.0));
	int32_t transRad = glowRad + (int32_t)(glowRad * (trans_perc/(double)100.0));
	auto maxRad = zc_max(glowRad,transRad);

	int offx = 0;
	int offy = 0;
#ifdef IS_PLAYER
	offx = viewport.x;
	offy = viewport.y;
#endif

	if(dest)
	{
		dithercircfill(dest, cx, cy, ditherRad, 0, dith_type, dith_arg, offx, offy);
		circlefill(dest, cx, cy, maxRad, 0);
	}
	if(transdest)
	{
		dithercircfill(transdest, cx, cy, ditherRad, 0, dith_type, dith_arg, offx, offy);
		circlefill(transdest, cx, cy, glowRad, 0);
	}
}

void doDarkroomCone(int32_t sx, int32_t sy, byte glowRad, int32_t dir, BITMAP* dest, BITMAP* transdest, int dith_perc, int trans_perc, int dith_type, int dith_arg)
{
	if(!glowRad) return;

	#ifdef IS_PLAYER
	sx -= viewport.x;
	sy -= viewport.y;
	//Default bitmap handling
	if(!dest) dest = darkscr_bmp_curscr;
	if(dest == darkscr_bmp_scrollscr) transdest = darkscr_bmp_scrollscr_trans;
	else if(dest == darkscr_bmp_z3) transdest = darkscr_bmp_z3_trans;
	else if(!transdest || dest == darkscr_bmp_curscr) transdest = darkscr_bmp_curscr_trans;
	#endif
	if(dith_perc < 0) dith_perc = DITH_PERC;
	if(trans_perc < 0) trans_perc = TRANS_PERC;
	if(dith_type < 0) dith_type = DITH_TYPE;
	if(dith_arg < 0) dith_arg = DITH_ARG;
	
	int32_t ditherDiff = (int32_t)(glowRad * (dith_perc/(double)100.0));
	int32_t transDiff = (int32_t)(glowRad * (trans_perc/(double)100.0));
	
	int32_t ditherRad = glowRad + 2*ditherDiff;
	int32_t transRad = glowRad + 2*transDiff;
	auto maxRad = zc_max(glowRad,transRad);
	
	double xs = 0, ys = 0;
	int32_t d = NORMAL_DIR(dir);
	if(d<0) return;
	switch(d)
	{
		case up: case l_up: case r_up: ys=1; break;
		case down: case l_down: case r_down: ys=-1; break;
	}
	switch(d)
	{
		case left: case l_up: case l_down: xs=1; break;
		case right: case r_up: case r_down: xs=-1; break;
	}
	if(d&4) {xs*=0.75; ys*=0.75;}
	if(glowRad>transRad) transDiff = 0;

	int offx = 0;
	int offy = 0;
#ifdef IS_PLAYER
	offx = viewport.x;
	offy = viewport.y;
#endif

	if(dest)
	{
		ditherLampCone(dest, sx+(xs*ditherDiff), sy+(ys*ditherDiff), ditherRad, d, 0, dith_type, dith_arg, offx, offy);
		lampcone(dest, sx+(xs*transDiff), sy+(ys*transDiff), maxRad, d, 0);
	}
	if(transdest)
	{
		ditherLampCone(transdest, sx+(xs*ditherDiff), sy+(ys*ditherDiff), ditherRad, d, 0, dith_type, dith_arg, offx, offy);
		lampcone(transdest, sx, sy, glowRad, d, 0);
	}
}

void doDarkroomSquare(int32_t cx, int32_t cy, byte glowRad, BITMAP* dest, BITMAP* transdest, int dith_perc, int trans_perc, int dith_type, int dith_arg)
{
	if(!glowRad) return;


	#ifdef IS_PLAYER
	cx -= viewport.x;
	cy -= viewport.y;
	//Default bitmap handling
	if(!dest) dest = darkscr_bmp_curscr;
	if(dest == darkscr_bmp_scrollscr) transdest = darkscr_bmp_scrollscr_trans;
	else if(dest == darkscr_bmp_z3) transdest = darkscr_bmp_z3_trans;
	else if(!transdest || dest == darkscr_bmp_curscr) transdest = darkscr_bmp_curscr_trans;
	#endif

	if(dith_perc < 0) dith_perc = DITH_PERC;
	if(trans_perc < 0) trans_perc = TRANS_PERC;
	if(dith_type < 0) dith_type = DITH_TYPE;
	if(dith_arg < 0) dith_arg = DITH_ARG;
	
	int32_t ditherRad = glowRad + (int32_t)(glowRad * (dith_perc/(double)100.0));
	int32_t transRad = glowRad + (int32_t)(glowRad * (trans_perc/(double)100.0));
	auto mrad = zc_max(glowRad,transRad);

	int offx = 0;
	int offy = 0;
#ifdef IS_PLAYER
	offx = viewport.x;
	offy = viewport.y;
#endif

	if(dest)
	{
		ditherrectfill(dest, cx-ditherRad, cy-ditherRad, cx+ditherRad, cy+ditherRad, 0, dith_type, dith_arg, offx, offy);
		rectfill(dest, cx-mrad, cy-mrad, cx+mrad, cy+mrad, 0);
	}
	if(transdest)
	{
		ditherrectfill(transdest, cx-ditherRad, cy-ditherRad, cx+ditherRad, cy+ditherRad, 0, dith_type, dith_arg, offx, offy);
		rectfill(transdest, cx-glowRad, cy-glowRad, cx+glowRad, cy+glowRad, 0);
	}
}

void handle_lighting(int cx, int cy, byte shape, byte rad, byte dir, BITMAP* dest, BITMAP* transdest)
{
	if(!rad) return;
	dir = NORMAL_DIR(dir);
	switch(shape)
	{
		case 0:
			doDarkroomCircle(cx,cy,rad,dest,transdest);
			break;
		case 1:
			doDarkroomCone(cx,cy,rad,dir,dest,transdest);
			break;
		case 2:
			doDarkroomSquare(cx,cy,rad,dest,transdest);
			break;
	}
}

void do_torch_combo(newcombo const& cmb, int cx, int cy, BITMAP* dest, BITMAP* transdest)
{
	ASSERT(cmb.type == cTORCH);
	handle_lighting(cx, cy, cmb.attribytes[1], cmb.attribytes[0], cmb.attribytes[2], dest, transdest);
}

bool dither_staticcheck(int x, int y, double percentage)
{
#ifdef IS_PLAYER
	double diff = abs(zc::math::Sin((double)((x*double(x))+(y*double(y)))) - (zc::math::Cos((double(x)*y))));
#else
	double diff = abs(sin((double)((x*double(x))+(y*double(y)))) - (cos((double(x)*y))));
#endif
	double filt = (percentage*(2000))/1000.0;
	return diff < filt;
}
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

#ifdef IS_PLAYER
	// This can be negative because of the workaround in `ditherblit` for replay compat.
	if (replay_is_active()) y = std::abs(y);
#endif
	
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
		case dithStatic:
			ret = dither_staticcheck(x,y,(arg/255.0));
			break;
		case dithStatic2: //changes centering of the formula
			ret = dither_staticcheck(x-(wid/2),y-(hei/2),(arg/255.0));
			break;
		case dithStatic3: //changes centering of the formula
			ret = dither_staticcheck(x+(wid/2),y+(hei/2),(arg/255.0));
			break;
		case dithDots2:
		{
			auto a2 = (arg+1)*2;
			ret = !(x%a2 || y%a2) || ((x%a2==arg+1) && (y%a2==arg+1));
			break;
		}
		case dithDots3:
		{
			auto a2 = (arg+1)*2;
			ret = !(x%a2 || y%2) || ((x%a2==arg+1) && (y%2==1));
			break;
		}
		case dithDots4:
		{
			auto a2 = (arg+1)*2;
			ret = !(x%2 || y%a2) || ((x%2==1) && (y%a2==arg+1));
			break;
		}
		default:
			//don't dither if invalid type found,
			//just draw every pixel -Em
			return true;
	}
	return ret^inv;
}

void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color)
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
void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color, int32_t targStart, int32_t targEnd)
{
	int32_t wid = zc_min(dest->w, src->w);
	int32_t hei = zc_min(dest->h, src->h);
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			auto oldc = bmp_read8(read_addr+tx);
			if(oldc >= targStart && oldc <= targEnd)
			{
				bmp_write8(write_addr+tx, color);
			}
		}
	}
	bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats)
{
	int32_t wid = zc_min(dest->w, mask->w);
	int32_t hei = zc_min(dest->h, mask->h);
	if(!repeats)
	{
		wid = zc_min(wid, pattern->w);
		hei = zc_min(hei, pattern->h);
	}
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t mask_addr = bmp_read_line(mask, ty);
		uintptr_t pattern_addr = bmp_read_line(mask, ty % pattern->h);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if(bmp_read8(mask_addr+tx))
			{
				auto color = bmp_read8(pattern_addr + (tx % pattern->w));
				bmp_write8(write_addr+tx, color);
			}
		}
	}
	bmp_unwrite_line(mask);
	bmp_unwrite_line(pattern);
	bmp_unwrite_line(dest);
}
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats, int32_t targStart, int32_t targEnd)
{
	int32_t wid = zc_min(dest->w, mask->w);
	int32_t hei = zc_min(dest->h, mask->h);
	if(!repeats)
	{
		wid = zc_min(wid, pattern->w);
		hei = zc_min(hei, pattern->h);
	}
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t mask_addr = bmp_read_line(mask, ty);
		uintptr_t pattern_addr = bmp_read_line(pattern, ty % pattern->h);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			auto oldc = bmp_read8(mask_addr+tx);
			if(oldc >= targStart && oldc <= targEnd)
			{
				auto patternc = bmp_read8(pattern_addr + (tx % pattern->w));
				bmp_write8(write_addr+tx, patternc);
			}
		}
	}
	bmp_unwrite_line(mask);
	bmp_unwrite_line(pattern);
	bmp_unwrite_line(dest);
}

void ditherblit(BITMAP* dest, BITMAP* src, int32_t color, byte dType, byte dArg, int32_t xoffs, int32_t yoffs)
{
	int32_t wid = src ? zc_min(dest->w, src->w) : dest->w;
	int32_t hei = src ? zc_min(dest->h, src->h) : dest->h;

#ifdef IS_PLAYER
	if (replay_is_active())
	{
		// z3 changed how dark bitmap drawing works, so to get the same results as before we must remove the delta
		// of the old/new dark bitmaps...
		yoffs -= 48;
		// ...and the playing field offset.
		yoffs -= playing_field_offset;

		extern bool screenscrolling;
		extern direction scrolling_dir;
		if (screenscrolling)
		{
			switch(scrolling_dir)
			{
				case up:
					yoffs += 176;
					break;
				case down:
					yoffs -= 176;
					break;
				case left:
					xoffs += 256;
					break;
				case right:
					xoffs -= 256;
					break;
			}
		}
	}
#endif

	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = src ? bmp_read_line(src, ty) : 0;
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if((!src || bmp_read8(read_addr+tx)) && dithercheck(dType,dArg,tx+xoffs,ty+yoffs,wid,hei))
			{
				bmp_write8(write_addr+tx, color);
			}
		}
	}
	if(src) bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}
void bmp_dither(BITMAP* dest, BITMAP* src, byte dType, byte dArg, int32_t xoffs, int32_t yoffs)
{
	int32_t wid = zc_min(dest->w, src->w);
	int32_t hei = zc_min(dest->h, src->h);
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if(dithercheck(dType,dArg,tx+xoffs,ty+yoffs,wid,hei))
			{
				bmp_write8(write_addr+tx, bmp_read8(read_addr+tx));
			}
		}
	}
	bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}
void custom_bmp_dither(BITMAP* dest, BITMAP* src, std::function<bool(int,int,int,int)> proc)
{
	int32_t wid = zc_min(dest->w, src->w);
	int32_t hei = zc_min(dest->h, src->h);
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			if(proc(tx,ty,wid,hei))
			{
				bmp_write8(write_addr+tx, bmp_read8(read_addr+tx));
			}
		}
	}
	bmp_unwrite_line(src);
	bmp_unwrite_line(dest);
}

void ditherblit_clipped(BITMAP* dest, BITMAP* src, int32_t color, byte dType, byte dArg, int32_t xoffs, int32_t yoffs)
{
	int32_t wid = std::min(dest->w, src->w);
	int32_t hei = std::min(dest->h, src->h);
	int32_t max_x = std::min(wid, src->cr);
	int32_t max_y = std::min(hei, src->cb);

	for(int32_t ty = dest->ct; ty < max_y; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(src, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = dest->cl; tx < max_x; ++tx)
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
void ditherrectfill(BITMAP* dest, int x1, int y1, int x2, int y2, int color,
	byte dType, byte dArg, int xoffs, int yoffs, optional<int> inv_color)
{
	if(x1 > x2) zc_swap(x1,x2);
	if(y1 > y2) zc_swap(y1,y2);
	uint wid = zc_min(x2-x1+1, dest->w-x1);
	uint hei = zc_min(y2-y1+1, dest->h-y1);
	for(int ty = 0; ty < hei; ++ty)
	{
		uintptr_t write_addr = bmp_write_line(dest, ty+y1);
		for(int tx = 0; tx < wid; ++tx)
			if(dithercheck(dType,dArg,tx+x1+xoffs,ty+y1+yoffs,wid,hei))
				bmp_write8(write_addr+tx+x1, color);
			else if(inv_color)
				bmp_write8(write_addr+tx+x1, *inv_color);
	}
	bmp_unwrite_line(dest);
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

void monocolor(BITMAP* dest, byte col, byte transp_passes)
{
	int32_t wid = dest->w;
	int32_t hei = dest->h;
	for (int ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(dest, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for (int tx = 0; tx < wid; ++tx)
		{
			byte c = bmp_read8(read_addr + tx);
			if (c)
			{
				if (transp_passes)
				{
					for (int q = 0; q < transp_passes; ++q)
					{
						c = trans_table2.data[c][col];
					}
				}
				else
					c = col;
				bmp_write8(write_addr + tx, c);
			}
		}
	}
	bmp_unwrite_line(dest);
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

void replColors(BITMAP* dest, vector<byte> srcCol, vector<byte> dstCol)
{
	size_t sz = std::min(srcCol.size(), dstCol.size());
	if(!sz) return;
	int32_t wid = dest->w;
	int32_t hei = dest->h;
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(dest, ty);
		uintptr_t write_addr = bmp_write_line(dest, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			byte c = bmp_read8(read_addr+tx);
			int dc = -1;
			for(size_t q = 0; q < sz; ++q)
			{
				if(c == srcCol[q])
				{
					dc = dstCol[q];
					break;
				}
			}
			if(dc > -1)
			{
				bmp_write8(write_addr+tx, byte(dc));
			}
		}
	}
	bmp_unwrite_line(dest);
}

//Counts the number of pixels in 'src' matching 'checkCol' in the masked area that matches 'maskCol'
//Any color < 0 will match with any non-zero color.
int32_t countColor(BITMAP* src, BITMAP* mask, int32_t x, int32_t y, int32_t checkCol, int32_t maskCol)
{
	int32_t wid = zc_min(src->w-x, mask->w);
	int32_t hei = zc_min(src->h-y, mask->h);
	int32_t count = 0;
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t mask_addr = bmp_read_line(mask, ty);
		uintptr_t read_addr = bmp_read_line(src, ty+y);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			int32_t maskc = bmp_read8(mask_addr+tx);
			if(maskCol < 0 ? maskc : maskc==maskCol)
			{
				int32_t color = bmp_read8(read_addr+tx+x);
				if(checkCol < 0 ? color : color==checkCol)
					++count;
			}
		}
	}
	bmp_unwrite_line(mask);
	bmp_unwrite_line(src);
	return count;
}

vector<byte> getColors(BITMAP* bmp, int maxCount)
{
	vector<byte> ret;
	
	int32_t wid = bmp->w;
	int32_t hei = bmp->h;
	for(int32_t ty = 0; ty < hei; ++ty)
	{
		uintptr_t read_addr = bmp_read_line(bmp, ty);
		for(int32_t tx = 0; tx < wid; ++tx)
		{
			byte c = bmp_read8(read_addr+tx);
			bool dupe = false;
			for(byte col : ret)
			{
				if(c == col)
				{
					dupe = true;
					break;
				}
			}
			if(!dupe)
			{
				ret.push_back(c);
				if(ret.size() >= maxCount)
				{
					ty = hei;
					break;
				}
			}
		}
	}
	bmp_unwrite_line(bmp);
	return ret;
}

