#include "base/mapscr.h"
#include "base/zsys.h"
#include "base/qrs.h"

byte mapscr::ffEffectWidth(size_t ind) const
{
	return (byte)ffcs[ind].hit_width;
}

void mapscr::ffEffectWidth(size_t ind, byte val)
{
	ffcs[ind].hit_width = val;
}

byte mapscr::ffEffectHeight(size_t ind) const
{
	return ffcs[ind].hit_height;
}

void mapscr::ffEffectHeight(size_t ind, byte val)
{
	ffcs[ind].hit_height = val;
}

byte mapscr::ffTileWidth(size_t ind) const
{
	return ffcs[ind].txsz;
}

void mapscr::ffTileWidth(size_t ind, byte val)
{
	ffcs[ind].txsz = val;
}

byte mapscr::ffTileHeight(size_t ind) const
{
	return ffcs[ind].tysz;
}

void mapscr::ffTileHeight(size_t ind, byte val)
{
	ffcs[ind].tysz = val;
}

void mapscr::zero_memory()
{
	*this = mapscr();
}

// TODO z3 ! return uint8_t
word mapscr::numFFC()
{
	if (!ffc_count_dirty)
		return num_ffcs;

	int lastffc = 0;
	for (int w = MAXFFCS - 1; w >= 0; --w)
	{
		if (ffcs[w].getData())
		{
			lastffc = w;
			break;
		}
	}
	if (lastffc < 31 && get_qr(qr_OLD_FFC_FUNCTIONALITY))
		lastffc = 31;
	num_ffcs = lastffc + 1;
	ffc_count_dirty = false;
	return num_ffcs;
}

void mapscr::ffcCountMarkDirty()
{
	ffc_count_dirty = true;
}
