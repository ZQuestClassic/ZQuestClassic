#include "moveinfo.h"
#include "tiles.h"



void BaseTileRef::forEach(std::function<void(int32_t)> proc) const
{
	for(auto [ex_t,w,h] : extra_rects)
	{
		auto t = ex_t + getTile() + offset();
		for(int x = 0; x < w; ++x)
			for(int y = 0; y < h; ++y)
				proc(t + x + (y*w));
	}
}
void TileRefPtr::forEach(std::function<void(int32_t)> proc) const
{
	BaseTileRef::forEach(proc);
	auto t = getTile() + offset();
	for(int x = 0; x < w; ++x)
		for(int y = 0; y < h; ++y)
			proc(t + x + y*TILES_PER_ROW);
}
void TileRefPtr10k::forEach(std::function<void(int32_t)> proc) const
{
	BaseTileRef::forEach(proc);
	auto t = getTile() + offset();
	for(int x = 0; x < w; ++x)
		for(int y = 0; y < h; ++y)
			proc(t + x + y*TILES_PER_ROW);
}
void TileRefCombo::forEach(std::function<void(int32_t)> proc) const
{
	BaseTileRef::forEach(proc);
	reset_combo_animation(*combo);
	do
	{
		proc(combo->tile);
		animate(*combo, true);
	}
	while(combo->tile != combo->o_tile);
}
