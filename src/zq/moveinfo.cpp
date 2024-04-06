#include "moveinfo.h"
#include "tiles.h"

void TileRefPtr::forEach(std::function<void(int32_t)> proc) const
{
	auto t = getTile() + offset();
	for(int x = 0; x < w; ++x)
		for(int y = 0; y < h; ++y)
			proc(t + x + y*TILES_PER_ROW);
}
void TileRefPtr10k::forEach(std::function<void(int32_t)> proc) const
{
	auto t = getTile() + offset();
	for(int x = 0; x < w; ++x)
		for(int y = 0; y < h; ++y)
			proc(t + x + y*TILES_PER_ROW);
}
void TileRefCombo::forEach(std::function<void(int32_t)> proc) const
{
	reset_combo_animation(*combo);
	do
	{
		proc(combo->tile);
		animate(*combo, true);
	}
	while(combo->tile != combo->o_tile);
}
