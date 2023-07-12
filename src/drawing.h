#ifndef DRAW_H
#define DRAW_H

void doDarkroomCircle(int32_t cx, int32_t cy, byte glowRad,BITMAP* dest=NULL,BITMAP* transdest=NULL, int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1);
void doDarkroomCone(int32_t sx, int32_t sy, byte glowRad, int32_t dir, BITMAP* dest=NULL,BITMAP* transdest=NULL, int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1);
void doDarkroomSquare(int32_t cx, int32_t cy, byte glowRad, BITMAP* dest=NULL, BITMAP* transdest=NULL, int dith_perc=-1, int trans_perc=-1, int dith_type=-1, int dith_arg=-1);
void handle_lighting(int cx, int cy, byte shape, byte rad, byte dir, BITMAP* dest=NULL,BITMAP* transdest=NULL);
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
	dithMax
};
void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color);
void mask_colorfill(BITMAP* dest, BITMAP* src, int32_t color, int32_t targStart, int32_t targEnd);
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats);
void mask_blit(BITMAP* dest, BITMAP* mask, BITMAP* pattern, bool repeats, int32_t targStart, int32_t targEnd);
void ditherblit(BITMAP* dest, BITMAP* src, int32_t color, byte dType, byte dArg, int32_t xoffs=0, int32_t yoffs=0);
void dithercircfill(BITMAP* dest, int32_t x, int32_t y, int32_t rad, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);
void ditherrectfill(BITMAP* dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);

void lampcone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color);
void ditherLampCone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);

void replColor(BITMAP* dest, byte col, byte startCol, byte endCol, bool shift);
void replColors(BITMAP* dest, std::vector<byte> srcCol, std::vector<byte> dstCol);
int32_t countColor(BITMAP* src, BITMAP* mask, int32_t x, int32_t y, int32_t checkCol, int32_t maskCol);

std::vector<byte> getColors(BITMAP* bmp, int maxCount);

#endif