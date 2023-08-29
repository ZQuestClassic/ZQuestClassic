#include "zq/autocombo/pattern_fence.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_fence::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		init_connections(ap);
		form_connections(ap, true);
		apply_changes();
		return true;
	}
	bool autopattern_fence::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		init_connections(ap);
		ap->write(layer, true);
		ap->in_set = false;
		form_connections(ap, false);
		apply_changes();
		return true;
	}
	void autopattern_fence::flip_single(apcombo*& ap)
	{
		int32_t slot = cid_to_slot(ap->cid);
		if (slot == -1)
			return;
		slot = flip_slot(slot);
		ap->cid = slot_to_cid(slot);
		ap->changed = true;
	}
	void autopattern_fence::flip_all_connected(int32_t exscreen, int32_t expos, int32_t max)
	{
		apcombo* ap = add(exscreen, expos);
		if (!ap)
			return;
		init_connections(ap);

		// Flip the first combo
		flip_single(ap);

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
								// If it's a loop, going down the second path will just undo everything
								looped = true;
							}
							else
							{
								// Flip the combo
								flip_single(curpath);

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
	void autopattern_fence::form_connections(apcombo* p, bool changecombo, bool noadj)
	{
		if (noadj && p->connflags & 0x100)
			return;
		p->connflags = 0;
		int32_t num_connections = 0;
		// Prioritize connections that are already formed
		for (int32_t i = 0; i < 4; ++i)
		{
			if (p->adj[i] && p->adj[i]->in_set && !(p->adj[i]->connflags & 0x100))
			{
				init_connections(p->adj[i]);
				if (num_connections < 2 && p->adj[i]->connflags & (1 << oppositeDir[i]))
				{
					p->connflags |= get_turn_edge_flags(p, i);
					++num_connections;
				}
			}
		}
		if (num_connections < 2)
		{
			// Next search for new connections
			for (int32_t i = 0; i < 4; ++i)
			{
				if (p->adj[i] && p->adj[i]->in_set && !(p->adj[i]->connflags & 0x100))
				{
					int32_t numconn = get_num_connections(p->adj[i]);
					if (num_connections < 2 && numconn < 2)
					{
						int32_t newflags = get_turn_edge_flags(p, i);
						// Disallow and invalid connection
						if (flags_to_slot(p->connflags | newflags) != -1)
							p->connflags |= newflags;
						++num_connections;
					}
				}
			}
		}

		// In case we're just updating the neighbors (after removing)
		if (changecombo)
		{
			int32_t slot = flags_to_slot(p->connflags);
			if (!(p->connflags & 0x100))
			{
				if (slot == -1)
					slot = 1;
				p->cid = slot_to_cid(slot);
				p->in_set = cid_to_slot(p->cid) > -1;
				p->changed = true;
			}
		}

		// Now update adjacent combos
		if (!noadj)
		{
			for (int32_t i = 0; i < 4; ++i)
			{
				if (p->adj[i] && p->adj[i]->in_set)
				{
					form_connections(p->adj[i], true, true);
				}
			}
		}
	}
	uint32_t autopattern_fence::slot_to_flags(int32_t slot)
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
			case 6: return U | R | UR;
			case 7: return L | R | UL | UR;
			case 8: return U | L | UL;
				// Outer corners
			case 9: return D | R | UR | DL;
			case 10: return D | L | UL | DR;
			case 11: return U | R | UL | DR;
			case 12: return U | L | UR | DL;
				// L/R dead ends
			case 13: return R | DR;
			case 14: return L | DL;
			case 17: return R | UR;
			case 18: return L | UL;
				// U/D dead ends
			case 15: return D | DR;
			case 16: return D | DL;
			case 19: return U | UR;
			case 20: return U | UL;
		}
		return 0;
	}
	int32_t autopattern_fence::flags_to_slot(uint32_t flags)
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
			case U | R | UR: return 6;
			case L | R | UL | UR: return 7;
			case U | L | UL: return 8;
				// Outer corners
			case D | R | UR | DL: return 9;
			case D | L | UL | DR: return 10;
			case U | R | UL | DR: return 11;
			case U | L | UR | DL: return 12;
				// L/R dead ends
			case R | DR: return 13;
			case L | DL: return 14;
			case R | UR: return 17;
			case L | UL: return 18;
				// U/D dead ends
			case D | DR: return 15;
			case D | DL: return 16;
			case U | UR: return 19;
			case U | UL: return 20;
		}
		return -1;
	}
	int32_t autopattern_fence::flip_slot(int32_t slot)
	{
		switch (slot)
		{
			// 3x3 ring
			case 0:	return 9;
			case 1: return 7;
			case 2: return 10;
			case 3:	return 5;
			case 4: return 4;
			case 5: return 3;
			case 6: return 11;
			case 7: return 1;
			case 8: return 12;
			// Outer corners
			case 9: return 0;
			case 10: return 2;
			case 11: return 6;
			case 12: return 8;
			// L/R dead ends
			case 13: return 17;
			case 14: return 18;
			case 17: return 13;
			case 18: return 14;
			// U/D dead ends
			case 15: return 16;
			case 16: return 15;
			case 19: return 20;
			case 20: return 19;
		}
		return 1;
	}
	// Returns connecting bitflags along the edge of a combo
	int32_t autopattern_fence::get_edge_flags(apcombo* &ap, int32_t dir, bool flipped)
	{
		int32_t ret = 0;
		switch (dir)
		{
			case up:
				ret = ap->connflags & (U | UL | UR);
				break;
			case down:
				ret = ap->connflags & (D | DL | DR);
				break;
			case left:
				ret = ap->connflags & (L | UL | DL);
				break;
			case right:
				ret = ap->connflags & (R | UR | DR);
				break;
		}
		// if flipped, return flags for the opposite cardinal direction
		if (flipped)
		{
			int32_t ret2 = 0;
			switch (dir)
			{
				case up:
					if (ret & U)
						ret2 |= D;
					if (ret & UL)
						ret2 |= DL;
					if (ret & UR)
						ret2 |= DR;
					return ret2;
				case down:
					if (ret & D)
						ret2 |= U;
					if (ret & DL)
						ret2 |= UL;
					if (ret & DR)
						ret2 |= UR;
					return ret2;
				case left:
					if (ret & L)
						ret2 |= R;
					if (ret & UL)
						ret2 |= UR;
					if (ret & DL)
						ret2 |= DR;
					return ret2;
				case right:
					if (ret & R)
						ret2 |= L;
					if (ret & UR)
						ret2 |= UL;
					if (ret & DR)
						ret2 |= DL;
					return ret2;
			}
		}
		return ret;
	}
	// Get connecting edge flags for an edge of this combo that connects
	// to an adjacent one, accounting for turning corners
	int32_t autopattern_fence::get_turn_edge_flags(apcombo*& ap, int32_t dir)
	{
		apcombo* adj = ap->adj[dir];
		int32_t ret = 0;
		if (adj)
		{
			int32_t adjslot = cid_to_slot(adj->cid);
			if (adjslot == -1)
				return 0;
			// If there's an adjacent combo already connected
			ret = get_edge_flags(adj, oppositeDir[dir], true);
			// If doing a turn
			if (!(ret & 0xF))
			{
				get_turn_flags(ret, dir, adjslot);
			}
			// If it's a dead end facing the wrong way
			if (!(ret & 0xF))
			{
				ret = get_edge_flags(adj, dir, false);
			}
			// If it's an empty square
			if (!(ret & 0xF))
			{
				// Special flag used by mountain bottoms
				if (!adj->connflags)
				{
					// If defualt orientation is flipped
					if (flip)
					{
						switch (dir)
						{
						case up:
							ret = U | UL;
							break;
						case down:
							ret = D | DL;
							break;
						case left:
							ret = L | UL;
							break;
						case right:
							ret = R | UR;
							break;
						}
					}
					else
					{
						switch (dir)
						{
							case up:
								ret = U | UR;
								break;
							case down:
								ret = D | DR;
								break;
							case left:
								ret = L | DL;
								break;
							case right:
								ret = R | DR;
								break;
						}
					}
				}
			}
		}
		return ret;
	}
	void autopattern_fence::get_turn_flags(int32_t &ret, int32_t dir, int32_t adjslot)
	{
		switch (dir)
		{
			case up:
				switch (adjslot)
				{
					case 14:
					case 17:
						ret = U | UL;
						break;
					case 13:
					case 18:
						ret = U | UR;
						break;
				}
				break;
			case down:
				switch (adjslot)
				{
					case 13:
					case 18:
						ret = D | DL;
						break;
					case 14:
					case 17:
						ret = D | DR;
						break;
				}
				break;
			case left:
				switch (adjslot)
				{
					case 16:
					case 19:
						ret = L | UL;
						break;
					case 15:
					case 20:
						ret = L | DL;
						break;
				}
				break;
			case right:
				switch (adjslot)
				{
					case 15:
					case 20:
						ret = R | UR;
						break;
					case 16:
					case 19:
						ret = R | DR;
						break;
				}
				break;
		}
	}
	int32_t autopattern_fence::get_num_connections(apcombo*& ap)
	{
		int32_t num_connections = 0;
		for (int32_t i = 0; i < 4; ++i)
		{
			if(ap->connflags & (1 << oppositeDir[i]))
				++num_connections;
		}
		return num_connections;
	}

}
