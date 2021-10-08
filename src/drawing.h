#ifndef DRAW_H
#define DRAW_H

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
void maskblit(BITMAP* dest, BITMAP* src, int color);
void ditherblit(BITMAP* dest, BITMAP* src, int color, byte dType, byte dArg, int xoffs=0, int yoffs=0);
void dithercircfill(BITMAP* dest, int x, int y, int rad, int color, byte ditherType, byte ditherArg, int xoffs=0, int yoffs=0);

void lampcone(BITMAP* dest, int sx, int sy, int range, int dir, int color);
void ditherLampCone(BITMAP* dest, int sx, int sy, int range, int dir, int color, byte ditherType, byte ditherArg, int xoffs=0, int yoffs=0);

void replColor(BITMAP* dest, byte col, byte startCol, byte endCol, bool shift);

#endif