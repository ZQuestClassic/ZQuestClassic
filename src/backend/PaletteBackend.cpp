#include "PaletteBackend.h"
#include "GraphicsBackend.h"
#include "Backend.h"
#include "../zc_alleg.h"
#include <cassert>

PaletteBackend::PaletteBackend()
{
	copyPalette(black_palette, pal_);
}

PaletteBackend::~PaletteBackend() {}

void PaletteBackend::applyPaletteToScreen()
{
	set_palette(pal_);
}

void PaletteBackend::copyPalette(PALETTE src, PALETTE dest)
{
	memcpy(dest, src, sizeof(PALETTE));
}

void PaletteBackend::setPalette(PALETTE newpal)
{
	copyPalette(newpal, pal_);
}

void PaletteBackend::getPalette(PALETTE targetpal)
{
	copyPalette(pal_, targetpal);
}

void PaletteBackend::setPaletteEntry(int entry, RGB color)
{
	if (entry >= 0 && entry <= 255)
		pal_[entry] = color;
}

void PaletteBackend::getPaletteEntry(int entry, RGB &color)
{
	if (entry >= 0 && entry <= 255)
		color = pal_[entry];
}

void PaletteBackend::setPaletteRange(PALETTE newpal, int first, int last)
{
	if (first >= 0 && first <= last && last <= 255)
	{
		memcpy(pal_ + first, newpal + first, (last - first + 1) * sizeof(RGB));
	}
}

void PaletteBackend::interpolatePalettes(PALETTE first, PALETTE second, int interpValue, int rangeFirst, int rangeSecond, PALETTE result)
{
	if (rangeFirst >= 0 && rangeFirst <= rangeSecond && rangeSecond <= 255)
	{
		for (int i = rangeFirst; i <= rangeSecond; i++)
		{
			result[i].r = (first[i].r * (64 - interpValue) + second[i].r * interpValue) / 64;
			result[i].g = (first[i].g * (64 - interpValue) + second[i].g * interpValue) / 64;
			result[i].b = (first[i].b * (64 - interpValue) + second[i].b * interpValue) / 64;
		}
	}
}

int PaletteBackend::virtualColorOfEntry(int entry)
{
	return entry;
}