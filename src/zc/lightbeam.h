// Traces light beams from spotlights through mirrors, prisms, glass, blocks and light targets.
//
// The tracer only reads a `lightbeam_world`, which HeroClass::handleSpotlights builds from the
// current region every frame, so it can be tested without loading a quest (see lightbeam_test.cpp).
// It writes the cells each beam passes through to a grid (used to draw the beams) and the trigger
// flags of the beams that reached each combo position to `lightbeam_world::istrig`.
//
// Combo spotlights trace along the combo grid. FFC spotlights trace from the FFC's center in
// 16px steps, so their spots go in a `lightbeam_lattice` for that offset from the combo grid.

#ifndef ZC_LIGHTBEAM_H_
#define ZC_LIGHTBEAM_H_

#include "base/ints.h"
#include "core/zdefs.h"
#include "zc/maps.h"
#include <array>
#include <map>
#include <vector>

typedef word spot_t;

#define SP_VISITED       0x1
#define SP_FLAGS         0x01E
#define SP_MASK          (SP_VISITED|SP_FLAGS)
// A beam passed through the side of the cell facing `dir`.
#define SP_FLAG(dir)     (0x2<<dir)
// A beam left the cell travelling in `dir`.
#define SP_GOFLAG(dir)   (0x20<<dir)
#define BEAM_AGE_LIMIT   512

// Cell type for anything solid that is not itself a light combo (solid combos and objects).
inline constexpr int32_t SPTYPE_SOLID = -1;

struct lightbeam_cell
{
	// The combo type that affects beams here (cMIRROR, cLIGHTTARGET, ...), SPTYPE_SOLID, or 0.
	int32_t type;
	// cMIRRORNEW only: the direction a beam travelling in each direction leaves in. 4-7 lets the
	// beam pass through unchanged, and anything higher stops it.
	std::array<byte, 4> mirror_dirs;
	// cLIGHTTARGET only: the target blocks light.
	bool blocks_light;
};

struct lightbeam_world;

// A pixel position on an FFC beam's path.
struct lightbeam_xy
{
	int16_t x;
	int16_t y;
	lightbeam_xy(int32_t nx, int32_t ny);
	bool valid(const lightbeam_world& world) const;
	rpos_t pos(const lightbeam_world& world) const;
	bool herocollide(const lightbeam_world& world, byte beamwid) const;
	// FFC beams are drawn in the order of this key (unsigned x, then unsigned y), which overlapping
	// beams depend on.
	dword key() const { return (dword(word(x))<<16)|word(y); }
};

// The spots FFC beams with one offset from the combo grid passed through. An FFC beam moves in
// 16px steps, and nothing moves it off the offset it started on, so its spots fit a dense array
// covering the region plus the 16px margin beams may leave it by and one step past that.
struct lightbeam_lattice
{
	// The first position, and the size in spots.
	int x0, y0, w, h;
	std::vector<spot_t> spots;

	lightbeam_lattice(const region_t& region, int ox, int oy);
	spot_t& at(lightbeam_xy xy)
	{
		int ix = (xy.x - x0) / 16, iy = (xy.y - y0) / 16;
		DCHECK(xy.x >= x0 && ix < w && xy.y >= y0 && iy < h);
		return spots[iy * w + ix];
	}
	lightbeam_xy xy(int index) const
	{
		return lightbeam_xy(x0 + index % w * 16, y0 + index / w * 16);
	}
};

struct lightbeam_world
{
	region_t region;
	// Indexed by rpos.
	std::vector<lightbeam_cell> cells;
	// Output, indexed by rpos: the trigger flags (see lbeam_flag) of every beam that reached it.
	std::vector<int32_t> istrig;
	// Output, for drawing: the spots every beam with a given beam id passed through. Combo beams'
	// grids are indexed by rpos. FFC beams get a lattice per offset from the combo grid, keyed by
	// (y&15)<<4 | (x&15).
	std::map<int32_t, std::vector<spot_t>> grids;
	std::map<int32_t, std::map<int, lightbeam_lattice>> ffgrids;
	// qr_BROKEN_LIGHTBEAM_HITBOX: light targets bounce beams back.
	bool broken_hitbox;

	// The Hero's combo position, or rpos_t::None if beams can't reach the Hero.
	rpos_t hero_rpos;
	int hero_x, hero_y, hero_dir;
	// The Hero's shield blocks beams coming at it head-on / reflects beams in the Hero's direction.
	bool hero_blocks, hero_reflects;

	// Total steps traced. Once past `max_steps` (0 for no limit), all tracing stops - that is only
	// for tests, so a beam that never stops fails the test instead of hanging it.
	size_t steps, max_steps;

	// Clears everything, and sizes the cells for the given region.
	void reset(const region_t& region);
};

// The istrig bit for a light beam trigger set (0 is every set).
int lbeam_flag(uint idx);

// Launches a beam from the combo spotlight at `rpos`. A `spotdir` above 3 lights only that spot.
void lightbeam_launch(lightbeam_world& world, int32_t beam_id, byte spotdir, rpos_t rpos, byte set);
// Launches a beam from an FFC spotlight centered on (x, y).
void lightbeam_launch_ffc(lightbeam_world& world, int32_t beam_id, byte spotdir, int32_t x, int32_t y, byte set, byte beamwid);

#endif
