#include "zq/autocombo/pattern_cakemtn.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_cakemtn::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		init_connections(ap);
		autopattern_fence::form_connections(ap, true);
		recalculate_height(ap, height);
		for (int32_t i = 0; i < 4; ++i)
		{
			if (ap->adj[i] && ap->connflags & (1<<i))
			{
				recalculate_height(ap->adj[i], height);
			}
		}
		apply_changes();
		return true;
	}
	bool autopattern_cakemtn::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		int32_t oldconn;
		for (int32_t i = 0; i < 4; ++i)
		{
			if (ap->adj[i] && ap->adj[i]->connflags & (1 << oppositeDir[i]))
				oldconn |= (1 << i);
		}
		ap->cid = erase_cid;
		init_connections(ap);
		ap->write(layer, true);
		ap->in_set = false;
		autopattern_fence::form_connections(ap, false);
		recalculate_height(ap, height);
		for (int32_t i = 0; i < 4; ++i)
		{
			if (ap->adj[i] && oldconn & (1<<i))
			{
				recalculate_height(ap->adj[i], height);
			}
		}
		apply_changes();
		return true;
	}
	void autopattern_cakemtn::flip_single(apcombo*& ap)
	{
		autopattern_fence::flip_single(ap);
		recalculate_height(ap, height);
	}
	uint32_t autopattern_cakemtn::slot_to_flags(int32_t slot)
	{
		switch (slot)
		{
			// 3x3 ring
			case 0:	return D | R | DR;
			case 1: return L | R | DL | DR;
			case 2: return D | L | DL;
			case 3:	return U | D | UR | DR;
			case 4: return 0;
			case 5: return U | D | UL | DL;
			case 15:
			case 6: return U | R | UR;
			case 16:
			case 7: return L | R | UL | UR;
			case 17:
			case 8: return U | L | UL;
			// Base combos
			case 12:
			case 9:
			case 13:
			case 10:
			case 14:
			case 11: return 0x100;
			// Outer corners
			case 18: return D | R | UR | DL;
			case 19: return D | L | UL | DR;
			case 20: return U | R | UL | DR;
			case 21: return U | L | UR | DL;
			// L/R dead ends
			case 22: return R | DR;
			case 23: return L | DL;
			case 26: return R | UR;
			case 27: return L | UL;
			// U/D dead ends
			case 24: return D | DR;
			case 25: return D | DL;
			case 28: return U | UR;
			case 29: return U | UL;
		}
		return 0;
	}
	int32_t autopattern_cakemtn::flags_to_slot(uint32_t flags)
	{
		switch (flags)
		{
			// 3x3 ring
			case D | R | DR: return 0;
			case L | R | DL | DR: return 1;
			case D | L | DL: return 2;
			case U | D | UR | DR: return 3;
			case 0: return 4;
			case U | D | UL | DL: return 5;
			case U | R | UR: return 15;
			case L | R | UL | UR: return 16;
			case U | L | UL: return 17;
			// Outer corners
			case D | R | UR | DL: return 18;
			case D | L | UL | DR: return 19;
			case U | R | UL | DR: return 20;
			case U | L | UR | DL: return 21;
			// L/R dead ends
			case R | DR: return 22;
			case L | DL: return 23;
			case R | UR: return 26;
			case L | UL: return 27;
			// U/D dead ends
			case D | DR: return 24;
			case D | DL: return 25;
			case U | UR: return 28;
			case U | UL: return 29;
		}
		return -1;
	}
	int32_t autopattern_cakemtn::flip_slot(int32_t slot)
	{
		switch (slot)
		{
			// 3x3 ring
			case 0:	return 18;
			case 1: return 16;
			case 2: return 19;
			case 3:	return 5;
			case 4: return 4;
			case 5: return 3;
			case 15:
			case 6: return 20;
			case 16:
			case 7: return 1;
			case 17:
			case 8: return 21;
			// Outer corners
			case 18: return 0;
			case 19: return 2;
			case 20: return 6;
			case 21: return 8;
			// L/R dead ends
			case 22: return 26;
			case 23: return 27;
			case 26: return 22;
			case 27: return 23;
			// U/D dead ends
			case 24: return 25;
			case 25: return 24;
			case 28: return 29;
			case 29: return 28;
		}
		return slot;
	}
	void autopattern_cakemtn::get_turn_flags(int32_t &ret, int32_t dir, int32_t adjslot)
	{
		switch (dir)
		{
			case up:
				switch (adjslot)
				{
					case 23:
					case 26:
						ret = U | UL;
						break;
					case 22:
					case 27:
						ret = U | UR;
						break;
				}
				break;
			case down:
				switch (adjslot)
				{
					case 22:
					case 27:
						ret = D | DL;
						break;
					case 23:
					case 26:
						ret = D | DR;
						break;
				}
				break;
			case left:
				switch (adjslot)
				{
					case 25:
					case 28:
						ret = L | UL;
						break;
					case 24:
					case 29:
						ret = L | DL;
						break;
				}
				break;
			case right:
				switch (adjslot)
				{
					case 24:
					case 29:
						ret = R | UR;
						break;
					case 25:
					case 28:
						ret = R | DR;
						break;
				}
				break;
		}
	}
	int32_t autopattern_cakemtn::get_south_face_id(int32_t slot)
	{
		switch (slot)
		{
			case 9:
			case 12:
				return 0;
			case 10:
			case 13:
				return 1;
			case 11:
			case 14:
				return 2;
			case 6:
			case 15:
				return 3;
			case 7:
			case 16:
				return 4;
			case 8:
			case 17:
				return 5;
			case 26:
				return 6;
			case 27:
				return 8;
		}
		return -1;
	}
	void autopattern_cakemtn::recalculate_height(apcombo*& ap, int32_t oldheight)
	{
		int32_t slot = cid_to_slot(ap->cid);
		int32_t south_face = get_south_face_id(slot);
		int32_t orig_south_face = south_face;
		// Is a south facing cliff
		if (south_face > 2)
		{

			int32_t safety = 0;
			apcombo* cur = ap;
			while (cur && safety < 9)
			{
				bool found_south_face = false;
				slot = cid_to_slot(cur->cid);
				south_face = get_south_face_id(slot);

				// Pick the combo's new cid
				int32_t new_cid = erase_cid;
				if (height <= 1 && safety == 0)
				{
					if (orig_south_face > 5)
						new_cid = cur->cid; // left and right end pieces
					else
						new_cid = slot_to_cid(15 + orig_south_face % 3); // 1 tile high
				}
				else
				{
					if (safety < height - 1)
					{
						if (safety == 0)
						{
							if(orig_south_face > 5)
								new_cid = cur->cid; // left and right end pieces
							else
								new_cid = slot_to_cid(6 + orig_south_face % 3); // top tile
						}
						else
							new_cid = slot_to_cid(9 + orig_south_face % 3); // middle tile

					}
					else if (safety == height - 1)
						new_cid = slot_to_cid(12 + orig_south_face % 3); // bottom tile
				}

				// Flag if it's a south facing cliff that isn't a top
				if (south_face > -1 && south_face < 3)
					found_south_face = true;

				// Don't update if intersecting with another cliff top
				if (safety == 0 || found_south_face || (!cur->in_set && new_cid != erase_cid))
				{
					int32_t slot = cid_to_slot(cur->cid);
					cur->cid = new_cid;
					cur->changed = true;
					cur->force_cset = true;
					cur->in_set = true;
				}
				else
					break;

				cur = add(cur, down, false, false);
				++safety;
				// break at having gone past the height and not finding a south face
				if ((safety >= height && safety >= oldheight && !found_south_face))
					break;
			}
		}
		// Is not, clean up any south facing cliffs below
		else if(south_face < 0)
		{
			int32_t safety = 1;
			apcombo* cur = add(ap, down, false, false);
			while (cur && safety < 9 && safety < oldheight)
			{
				// Must be an autocombo
				if (cur->in_set)
				{
					slot = cid_to_slot(cur->cid);
					south_face = get_south_face_id(slot);
					// Must be a south face
					if (south_face > -1 && south_face < 3)
					{
						cur->cid = erase_cid;
						cur->changed = true;
						cur->force_cset = true;
					}
					else
						break;
				}
				else
					break;
				cur = add(cur, down, false, false);
				++safety;
			}
		}
	}
	void autopattern_cakemtn::resize_connected(int32_t exscreen, int32_t expos, int32_t max, int32_t newheight)
	{
		apcombo* ap = add(exscreen, expos);
		if (!ap)
			return;
		init_connections(ap);

		// Flip the first combo
		int32_t oldheight = height;
		height = newheight;
		recalculate_height(ap, oldheight);

		bool looped = false;
		// Trace a path for every connected direction
		for (int32_t i = 0; i < 4 && !looped; ++i)
		{
			if (ap->connflags & (1 << i))
			{
				apcombo* curpath = ap;
				int32_t dir = i;
				int32_t len = 0;
				bool broken = false;
				while (!broken && len < max)
				{
					curpath = add(curpath, dir);
					if (curpath)
					{
						if (curpath->in_set)
						{
							++len;
							if (curpath->screenpos == ap->screenpos)
							{
								broken = true;
								// If it's a loop, going down the second path is pointless
								looped = true;
							}
							else
							{
								recalculate_height(curpath, oldheight);

								int32_t newdir = -1;
								for (int32_t j = 0; j < 4; ++j)
								{
									if (j != oppositeDir[dir] && curpath->connflags & (1 << j))
									{
										newdir = j;
									}
								}
								// If there's no new dir, it's a dead end
								if (newdir == -1)
								{
									broken = true;
								}
								else
								{
									dir = newdir;
								}
							}
						}
						// something went wrong
						else
							broken = true;
					}
					// invalid pointer, break out
					else
						broken = true;
				}
			}
		}

		apply_changes();
	}

}
