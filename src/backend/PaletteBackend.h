#ifndef PALETTEBACKEND_H
#define PALETTEBACKEND_H

#include "../zc_alleg.h"

class PaletteBackend
{
public:
	~PaletteBackend();

	/*
	* Sets the entire palette of the virtual screen to the specified palette.
	* Note that this new palette will not take effect until the screen
	* is rerendered; call showBackBuffer() in the graphics backend to show
	* the new colors immediately.
	*/
	void setPalette(PALETTE newpal);

	/*
	* Copies the entire current virtual screen palette into targetpal.
	*/
	void getPalette(PALETTE targetpal);

	/*
	* Copies only some of the color from the palette newpal into the virtual
	* screen palette. The range in inclusive: all colors in the range
	* [first, last] will be modified. Does nothing unless
	* 0 <= first <= last <= 255.
	* Like setPalette(), this call does not immediately refresh the screen;
	* visible colors will not change until the next call to showBackBuffer()
	* in the graphics backend.
	*/
	void setPaletteRange(PALETTE newpal, int first, int last);

	/*
	* Sets an individual palette entry's color definition, on the virtual
	* screen palette. Does nothing unless 0 <= entry <= 255.
	* Like setPalette(), this call does not immediately refresh the screen;
	* visible colors will not change until the next call to showBackBuffer()
	* in the graphics backend.
	*/
	void setPaletteEntry(int entry, RGB color);

	/*
	* Reads one current virtual screen palette entry into color. The current
	* data in color is overwritten. Does nothing unless 0 <= entry <= 255.
	*/
	void getPaletteEntry(int entry, RGB &color);

	/*
	* Returns the color of the virtual screen palette entry entry, packed into
	* the virtual screen's color format. Currently does nothing, since the
	* virtual screen always has 8-bit color depth, but this function was
	* preserved in case this invariant changes in the future.
	*/
	int virtualColorOfEntry(int entry);

	/*
	* Linearly interpolates some entries between two palettes, first and 
	* second, and stores the resulting palette in result.
	* Only affects entries between rangeFirst and rangeSecond, inclusive; this
	* function does nothing unless 0 <= rangeFirst <= rangeSecond <= 255.
	* interpValue determines how much to weight each of the two palettes.
	* interpValue = 0 uses the first palette only, and interpValue = 64
	* uses only the second. Values between 0 and 64 linearly blend between
	* the two; extrapolation (using interpValue < 0 or > 64) is also supported.
	*/
	void interpolatePalettes(PALETTE first, PALETTE second, int interpValue, int rangeFirst, int rangeSecond, PALETTE result);

	/*
	* Copies all colors from the source palette to the destination.
	*/
	void copyPalette(PALETTE source, PALETTE dest);

	/*
	* Sets the physical screen's palette to the current virtual screen palette.
	* DANGER: there are restrictions on when this function can be called safely.
	* This function should therefore only be called in carefully controlled
	* circumstances (i.e., within the graphics backend.) The correct way to
	* refresh the screen and see the new palette colors is to call
	* showBackBuffers() in the graphics backend, rather than this function
	* directly.
	*/
	void applyPaletteToScreen();

	friend class Backend;
private:
	PaletteBackend();

	PALETTE pal_;
};


#endif
