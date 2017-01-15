#ifndef PALETTEBACKEND_H
#define PALETTEBACKEND_H

#include "../zc_alleg.h"

class PaletteBackend
{
public:
	~PaletteBackend();

	void setPalette(PALETTE newpal);
	void getPalette(PALETTE targetpal);

	void setPaletteRange(PALETTE newpal, int first, int last);

	void setPaletteEntry(int entry, RGB color);
	void getPaletteEntry(int entry, RGB &color);

	int virtualColorOfEntry(int entry);

	void interpolatePalettes(PALETTE first, PALETTE second, int interpValue, int rangeFirst, int rangeSecond, PALETTE result);
	void copyPalette(PALETTE source, PALETTE dest);

	void applyPaletteToScreen();

	friend class Backend;
private:
	PaletteBackend();

	PALETTE pal_;
};


#endif
