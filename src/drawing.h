#ifndef DRAW_H_
#define DRAW_H_

#include "base/headers.h"

struct newcombo;

void doDarkroomCircle(int32_t cx, int32_t cy, word glowRad,BITMAP* dest=NULL,BITMAP* transdest=NULL,
	int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1,
	optional<word> wave_rate=nullopt, optional<word> wave_size=nullopt);
void doDarkroomCone(int32_t sx, int32_t sy, word glowRad, int32_t dir, BITMAP* dest=NULL,BITMAP* transdest=NULL,
	int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1,
	optional<word> wave_rate=nullopt, optional<word> wave_size=nullopt);
void doDarkroomSquare(int32_t cx, int32_t cy, word glowRad, BITMAP* dest=NULL, BITMAP* transdest=NULL,
	int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1,
	optional<word> wave_rate=nullopt, optional<word> wave_size=nullopt);
void handle_lighting(int cx, int cy, byte shape, word rad, byte dir, BITMAP* dest=NULL,BITMAP* transdest=NULL,
	int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1,
	optional<word> wave_rate=nullopt, optional<word> wave_size=nullopt);
void do_torch_combo(newcombo const& cmb, int cx, int cy, BITMAP* dest=NULL,BITMAP* transdest=NULL);

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
	dithDots2, dithDots2Inv,
	dithDots3, dithDots3Inv,
	dithDots4, dithDots4Inv,
	dithMax
};
bool dither_staticcheck(int x, int y, double percentage);
void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color);
void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color, int32_t targStart, int32_t targEnd);
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats);
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats, int32_t targStart, int32_t targEnd);
void ditherblit(BITMAP* dest, BITMAP* src, int32_t color, byte dType, byte dArg, int32_t xoffs=0, int32_t yoffs=0);
void bmp_dither(BITMAP* dest, BITMAP* src, byte dType, byte dArg, int32_t xoffs=0, int32_t yoffs=0);
void custom_bmp_dither(BITMAP* dest, BITMAP* src, std::function<bool(int,int,int,int)> proc);
void dithercircfill(BITMAP* dest, int32_t x, int32_t y, int32_t rad, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);
void ditherrectfill(BITMAP* dest, int x1, int y1, int x2, int y2, int color, byte ditherType, byte ditherArg, int xoffs=0, int yoffs=0, optional<int> inv_color=nullopt);

void lampcone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color);
void ditherLampCone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);

void monocolor(BITMAP* dest, byte col, byte transp_passes = 0);
void replColor(BITMAP* dest, byte col, byte startCol, byte endCol, bool shift);
void replColors(BITMAP* dest, vector<byte> srcCol, vector<byte> dstCol);
int32_t countColor(BITMAP* src, BITMAP* mask, int32_t x, int32_t y, int32_t checkCol, int32_t maskCol);

vector<byte> getColors(BITMAP* bmp, int maxCount);

#endif