#include "zc/lightbeam.h"
#include "base/general.h"

void lightbeam_world::reset(const region_t& region)
{
	this->region = region;
	cells.clear();
	cells.resize(region.screen_count*176);
	istrig.clear();
	istrig.resize(region.screen_count*176);
	grids.clear();
	ffgrids.clear();
	broken_hitbox = false;
	hero_rpos = rpos_t::None;
	hero_x = hero_y = hero_dir = 0;
	hero_blocks = hero_reflects = false;
	steps = max_steps = 0;
}

lightbeam_xy::lightbeam_xy(int32_t nx, int32_t ny)
{
	x = vbound(nx,-32768,32767);
	y = vbound(ny,-32768,32767);
}

bool lightbeam_xy::valid(const lightbeam_world& world) const
{
	return x+16 >= 0 && x-16 < world.region.width && y+16 >= 0 && y-16 < world.region.height;
}

rpos_t lightbeam_xy::pos(const lightbeam_world& world) const
{
	return COMBOPOS_REGION(world.region, vbound(x,0,world.region.width-1), vbound(y,0,world.region.height-1));
}

bool lightbeam_xy::herocollide(const lightbeam_world& world, byte beamwid) const
{
	int bx = x-beamwid/2, by = y-beamwid/2;
	int hx = world.hero_x, hy = world.hero_y;
	return hx+15 >= bx && hx < bx+beamwid
		&& hy+15 >= by && hy < by+beamwid;
}

int lbeam_flag(uint idx)
{
	if (idx)
		return 1 << (idx - 1);
	return ~0;
}

static bool lightbeam_out_of_steps(lightbeam_world& world)
{
	++world.steps;
	return world.max_steps && world.steps > world.max_steps;
}

// Turns a beam travelling in `spotdir` that reached `cell`, or returns false if the cell stops
// it. Prisms are left to the callers, since they split the beam.
static bool turn_beam(const lightbeam_world& world, const lightbeam_cell& cell, byte& spotdir)
{
	switch(cell.type)
	{
		case cLIGHTTARGET:
			if(cell.blocks_light)
				return false;
			if(world.broken_hitbox)
				spotdir = oppositeDir[spotdir];
			break;
		case cMIRROR:
			spotdir = oppositeDir[spotdir];
			break;
		case cMIRRORSLASH:
			switch(spotdir)
			{
				case up:
					spotdir = right; break;
				case right:
					spotdir = up; break;
				case down:
					spotdir = left; break;
				case left:
					spotdir = down; break;
			}
			break;
		case cMIRRORBACKSLASH:
			switch(spotdir)
			{
				case up:
					spotdir = left; break;
				case left:
					spotdir = up; break;
				case down:
					spotdir = right; break;
				case right:
					spotdir = down; break;
			}
			break;
		case cMIRRORNEW:
		{
			byte newdir = cell.mirror_dirs[spotdir];
			if(newdir > 7)
				return false;
			if(newdir <= 3)
				spotdir = newdir;
			break;
		}
	}
	return true;
}

static void handleBeam(lightbeam_world& world, std::vector<spot_t>& grid, size_t age, byte spotdir, rpos_t rpos, byte set)
{
	if(spotdir > 3) return; //invalid dir

	int combos_wide = world.region.screen_width * 16;
	int combos_tall = world.region.screen_height * 11;
	int32_t trigflag = lbeam_flag(set);
	bool doAge = true;
	spot_t f = 0;
	while (true)
	{
		if (lightbeam_out_of_steps(world))
			return;

		auto [x, y] = COMBOXY_REGION_INDEX(world.region, rpos);

		f = SP_GOFLAG(spotdir);
		if((grid[(int)rpos] & f) == f)
			return;
		else grid[(int)rpos] |= f;
		f = SP_FLAG(spotdir);
		if((grid[(int)rpos] & f) != f)
		{
			grid[(int)rpos] |= f;
			world.istrig[(int)rpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		switch(spotdir)
		{
			case up:
				y -= 1;
				break;
			case down:
				y += 1;
				break;
			case left:
				x -= 1;
				break;
			case right:
				x += 1;
				break;
		}

		if (!(x >= 0 && x < combos_wide && y >= 0 && y < combos_tall)) return;

		rpos = COMBOPOS_REGION_INDEX(world.region, x, y);
		auto const& cell = world.cells[(int)rpos];

		switch (cell.type)
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				return;
		}

		if (rpos == world.hero_rpos && world.hero_blocks && spotdir == oppositeDir[world.hero_dir])
			return;

		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[(int)rpos] & f) != f)
		{
			grid[(int)rpos] |= f;
			world.istrig[(int)rpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		if(doAge)
		{
			if(++age > BEAM_AGE_LIMIT)
				return;
		}
		else doAge = true;

		if (rpos == world.hero_rpos && world.hero_reflects)
			spotdir = world.hero_dir;
		else if (cell.type == cMAGICPRISM || cell.type == cMAGICPRISM4)
		{
			for(byte d = 0; d < 4; ++d)
			{
				if(cell.type == cMAGICPRISM && d == oppositeDir[spotdir]) continue;
				handleBeam(world, grid, age, d, rpos, set);
			}
			return;
		}
		else if (!turn_beam(world, cell, spotdir))
			return;
	}
}

static void handleFFBeam(lightbeam_world& world, std::map<lightbeam_xy,spot_t>& grid, size_t age, byte spotdir, lightbeam_xy curxy, byte set, byte beamwid)
{
	if(spotdir > 3) return; //invalid dir
	int32_t trigflag = lbeam_flag(set);
	bool doAge = true;
	spot_t f = 0;
	while(curxy.valid(world))
	{
		if (lightbeam_out_of_steps(world))
			return;

		// Stop if a beam already left this spot in this direction. Without this, a beam that
		// loops back into a prism splits again every time around.
		f = SP_GOFLAG(spotdir);
		if((grid[curxy] & f) == f)
			return;
		grid[curxy] |= f;
		f = SP_FLAG(spotdir);
		if((grid[curxy] & f) != f)
		{
			grid[curxy] |= f;
			world.istrig[(int)curxy.pos(world)] |= trigflag;
			doAge = false;
			age = 0;
		}
		switch(spotdir)
		{
			case up:
				curxy.y -= 16;
				break;
			case down:
				curxy.y += 16;
				break;
			case left:
				curxy.x -= 16;
				break;
			case right:
				curxy.x += 16;
				break;
		}
		auto curpos = curxy.pos(world);
		auto const& cell = world.cells[(int)curpos];
		switch(cell.type)
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				return;
			case cMIRRORNEW:
			{
				// Unlike a combo beam, this stops before lighting the mirror.
				byte newdir = cell.mirror_dirs[spotdir];
				if(newdir > 7) return;
				break;
			}
		}
		bool collided_hero = world.hero_rpos != rpos_t::None && curxy.herocollide(world, beamwid);
		if(world.hero_blocks && (spotdir == oppositeDir[world.hero_dir]) && collided_hero)
			return;

		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[curxy] & f) != f)
		{
			grid[curxy] |= f;
			world.istrig[(int)curpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		if(!curxy.valid(world)) return;
		if(doAge)
		{
			if(++age > BEAM_AGE_LIMIT)
				return;
		}
		else doAge = true;

		if(world.hero_reflects && collided_hero)
			spotdir = world.hero_dir;
		else if (cell.type == cMAGICPRISM || cell.type == cMAGICPRISM4)
		{
			for(byte d = 0; d < 4; ++d)
			{
				if(cell.type == cMAGICPRISM && d == oppositeDir[spotdir]) continue;
				handleFFBeam(world, grid, age, d, curxy, set, beamwid);
			}
			return;
		}
		else if (!turn_beam(world, cell, spotdir))
			return;
	}
}

void lightbeam_launch(lightbeam_world& world, int32_t beam_id, byte spotdir, rpos_t rpos, byte set)
{
	auto& grid = world.grids[beam_id];
	if (grid.empty())
		grid.resize(world.cells.size());
	if(spotdir > 3)
	{
		grid[(int)rpos] |= SP_VISITED;
		world.istrig[(int)rpos] |= lbeam_flag(set);
	}
	if(world.hero_reflects && rpos == world.hero_rpos)
	{
		spotdir = world.hero_dir;
	}
	handleBeam(world, grid, 0, spotdir, rpos, set);
}

void lightbeam_launch_ffc(lightbeam_world& world, int32_t beam_id, byte spotdir, int32_t x, int32_t y, byte set, byte beamwid)
{
	auto& grid = world.ffgrids[beam_id];
	lightbeam_xy sxy(x, y);
	if(spotdir > 3 && sxy.valid(world))
	{
		grid[sxy] |= SP_VISITED;
		world.istrig[(int)sxy.pos(world)] |= lbeam_flag(set);
	}
	if(world.hero_reflects && world.hero_rpos != rpos_t::None && sxy.herocollide(world, beamwid))
	{
		spotdir = world.hero_dir;
	}
	switch(spotdir)
	{
		case up:
			sxy.y = zc_min(world.region.height-1,sxy.y);
			break;
		case down:
			sxy.y = zc_max(0,sxy.y);
			break;
		case left:
			sxy.x = zc_min(world.region.width-1,sxy.x);
			break;
		case right:
			sxy.x = zc_max(0,sxy.x);
			break;
	}
	handleFFBeam(world, grid, 0, spotdir, sxy, set, beamwid);
}
