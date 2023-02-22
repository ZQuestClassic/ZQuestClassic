#ifndef DRAW_H
#define DRAW_H

std::string const& get_color_name(int r, int g, int b);
std::string const& get_color_name(int c, bool is8b); //Includes '(Trans)' for transparents
std::string const& get_color_name(int c);

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

void lampcone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color);
void ditherLampCone(BITMAP* dest, int32_t sx, int32_t sy, int32_t range, int32_t dir, int32_t color, byte ditherType, byte ditherArg, int32_t xoffs=0, int32_t yoffs=0);

void replColor(BITMAP* dest, byte col, byte startCol, byte endCol, bool shift);
int32_t countColor(BITMAP* src, BITMAP* mask, int32_t x, int32_t y, int32_t checkCol, int32_t maskCol);

#endif