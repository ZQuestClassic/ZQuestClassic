#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_dormtn.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_dormtn::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->connflags = TOP;
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		
		load_all_tops(ap);
		update_top_combos();
		if(!key[KEY_LSHIFT])
		{
			find_sides();
			update_sides();
		}
		mask_out_context(ap);
		apply_changes();
		return true;
	}
	bool autopattern_dormtn::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, false);
		if (!ap)
			return false;
		if(is_top(ap->slot)||!ap->in_set)
		{
			ap->cid = erase_cid;
			ap->write(layer, true);
			ap->in_set = false;
		}
		if(is_top(ap->slot))
		{
			erase_sides(ap);
			load_all_tops(ap);
			ap->slot = -1;
			ap->connflags = 0;
			update_top_combos();
			if (!key[KEY_LSHIFT])
			{
				find_sides();
				update_sides();
			}
			mask_out_context(ap);
			apply_changes();
		}
		return true;
	}
	int32_t autopattern_dormtn::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		ap->connflags = TOP;
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));

		load_all_tops(ap);
		update_top_combos();
		return ap->cid;
	}
	void autopattern_dormtn::calculate_connections(apcombo* p)
	{
		p->connflags = 0;
		if (is_top(p->slot))
		{
			p->connflags |= TOP;
			for (int32_t q = 0; q < 8; ++q)
			{
				apcombo* adj = p->adj[q];
				if (adj)
				{
					if (is_top(adj->slot))
					{
						p->connflags |= (1 << q);
					}
				}
				else if(connectedge)
					p->connflags |= (1 << q);
			}
		}
	}
	uint32_t autopattern_dormtn::slot_to_flags(int32_t slot)
	{
		switch (slot)
		{
			case 24:
				return TOP | D | R | DR;
			case 25:
				return TOP | D | L | R | DL | DR;
			case 26:
				return TOP | D | L | DL;
			case 27:
				return TOP | D | R | UR | DL | DR;
			case 28:
				return TOP | U | D | L | R | UL | UR | DL | DR;
			case 29:
				return TOP | D | L | UL | DL | DR;
			case 30:
				return TOP | U | D | R | UR | DR;
			case 32:
				return TOP | U | D | L | UL | DL;
			case 33:
				return TOP | U | R | UL | UR | DR;
			case 35:
				return TOP | U | L | UL | UR | DL;
			case 36:
				return TOP | U | R | UR;
			case 37:
				return TOP | U | L | R | UL | UR;
			case 38:
				return TOP | U | L | UL;
			default:
				return 31;
		}
		return 0;
	}
	int32_t autopattern_dormtn::flags_to_slot(uint32_t flags)
	{
		if (flags & TOP)
		{
			// Mask out diagonal flags for disabled directions
			if (!(flags & U))
				flags &= ~(UL | UR);
			if (!(flags & D))
				flags &= ~(DL | DR);
			if (!(flags & L))
				flags &= ~(UL | DL);
			if (!(flags & R))
				flags &= ~(UR | DR);

			switch (flags & 0xFF)
			{
				case D | R | DR:
					return 24;
				case D | L | R | DL | DR:
					return 25;
				case D | L | DL:
					return 26;
				case U | D | L | R | UR | DL | DR:
					return 27;
				case U | D | L | R | UL | UR | DL | DR:
					return 28;
				case U | D | L | R | UL | DL | DR:
					return 29;
				case U | D | R | UR | DR:
					return 30;
				case U | D | L | UL | DL:
					return 32;
				case U | D | L | R | UL | UR | DR:
					return 33;
				case U | D | L | R | UL | UR | DL:
					return 35;
				case U | R | UR:
					return 36;
				case U | L | R | UL | UR:
					return 37;
				case U | L | UL:
					return 38;
				default:
					return 31;
			}
		}

		if (flags & SIDE)
		{
			switch (flags & 0xFFF)
			{
				case U:
					return 9;
				case U | RIM:
					return 13;
				case U|BASE:
					return 10;
				case U | RIM | BASE:
					return 14;
				case U | BASE | HALF:
					return 21;
				case U | RIM | BASE | HALF:
					return 23;

				case D:
					return 43;
				case D | RIM:
					return 40;
				case D | BASE:
					return 46;

				case D | R:
				case D | R | BASE:
					if (flags & SOUTHUPPER)
						return 50;
					return 54;
				case D | R | RIM:
				case D | R | BASE | RIM:
					if (flags & SOUTHUPPER)
						return 58;
					return 62;
				case D | L:
				case D | L | BASE:
					if (flags & SOUTHUPPER)
						return 51;
					return 55;
				case D | L | RIM:
				case D | L | BASE | RIM:
					if (flags & SOUTHUPPER)
						return 59;
					return 63;

				case L:
					return 70;
				case L | RIM:
					return 71;
				case L | BASE:
					return 69;
				case L | RIM | BASE:
					return 68;

				case R:
					return 73;
				case R | RIM:
					return 72;
				case R | BASE:
					return 74;
				case R | RIM | BASE:
					return 75;

				case UL:
				case UL | RIM:
					return 7;
				case UL | RIM | BASE:
				case UL | BASE:
					return 3;
				case UL | U | RIM:
					if (flags & OUTERCORNER)
						return 18;
					return 15;
				case UL | U:
					if (flags & OUTERCORNER)
						return 18;
					return 11;
				case UL | U | RIM | BASE:
					if (flags & OUTERCORNER)
						return 16;
					return 15;
				case UL | U | BASE:
					if (flags & OUTERCORNER)
						return 16;
					return 11;
				case UL | U | HALF | RIM | BASE:
					if (flags & OUTERCORNER)
						return 20;
					return 2;
				case UL | U | HALF | BASE:
					if (flags & OUTERCORNER)
						return 20;
					return 6;

				case UR:
				case UR | RIM:
					return 4;
				case UR | RIM | BASE:
				case UR | BASE:
					return 0;
				case UR | U | RIM:
					if (flags & OUTERCORNER)
						return 19;
					return 12;
				case UR | U:
					if (flags & OUTERCORNER)
						return 19;
					return 8;
				case UR | U | RIM | BASE:
					if (flags & OUTERCORNER)
						return 17;
					return 12;
				case UR | U | BASE:
					if (flags & OUTERCORNER)
						return 17;
					return 8;
				case UR | U | HALF | RIM | BASE:
					if (flags & OUTERCORNER)
						return 22;
					return 1;
				case UR | U | HALF | BASE:
					if (flags & OUTERCORNER)
						return 22;
					return 5;

				case DL:
					return 42;
				case DL | RIM:
					return 39;
				case DL | RIM | BASE:
				case DL | BASE:
					return 45;
				case DL | L:
					if (flags & OUTERCORNER)
						return 53;
					return 49;
				case DL | L | RIM:
					if (flags & OUTERCORNER)
						return 67;
					return 57;
				case DL | L | RIM | BASE:
					if (flags & OUTERCORNER)
						return 61;
					return 57;
				case DL | L | BASE:
					if (flags & OUTERCORNER)
						return 65;
					return 49;

				case DR:
					return 44;
				case DR | RIM:
					return 41;
				case DR | RIM | BASE:
				case DR | BASE:
					return 47;
				case DR | R:
					if (flags & OUTERCORNER)
						return 52;
					return 48;
				case DR | R | RIM:
					if (flags & OUTERCORNER)
						return 66;
					return 56;
				case DR | R | RIM | BASE:
					if (flags & OUTERCORNER)
						return 60;
					return 56;
				case DR | R | BASE:
					if (flags & OUTERCORNER)
						return 64;
					return 48;

				default:
					return 34;
			}
		}
		
		return 34;
	}
	bool autopattern_dormtn::is_top(int32_t slot)
	{
		switch (slot)
		{
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 32:
			case 31:
			case 33:
			case 35:
			case 36:
			case 37:
			case 38:
				return true;
		}
		return false;
	}
	void autopattern_dormtn::load_all_tops(apcombo* p)
	{
		p->connflags = 0;
		for (int32_t q = 0; q < 4; ++q)
		{
			apcombo* adj = add(p, q, false, true);
			if (adj)
			{
				if (adj->in_set)
				{
					if (is_top(adj->slot))
					{
						p->connflags |= (1 << q);
						if(p->x >= basescreen_x - 16 && p->x <= basescreen_x + 15 + 16 && p->y >= basescreen_y - 11 && p->y <= basescreen_y + 10 + 11)
						{
							if (tops.count(adj->screenpos) == 0)
							{
								tops[adj->screenpos] = adj;
								load_all_tops(adj);
							}
						}
					}
				}
			}
		}
	}
	void autopattern_dormtn::update_top_combos()
	{
		for (auto a : tops)
		{
			apcombo* ap = a.second;
			calculate_connections(ap);
			ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		}
	}
	bool autopattern_dormtn::is_vertex(apcombo* ap)
	{
		int32_t adj_slot[8];
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q])
				adj_slot[q] = ap->adj[q]->slot;
			else
				adj_slot[q] = -1;
		}
		switch (ap->slot)
		{
			case 24:
				if (adj_slot[l_down] != 24)
					return true;
				break;
			case 26:
				if (adj_slot[l_up] != 26)
					return true;
				break;
			case 36:
				if (adj_slot[l_up] != 36)
					return true;
				break;
			case 38:
				if (adj_slot[l_down] != 38)
					return true;
				break;

			case 25:
				if (adj_slot[left] != 25)
					return true;
				break;
			case 30:
				if (adj_slot[up] != 30)
					return true;
				break;
			case 32:
				if (adj_slot[up] != 32)
					return true;
				break;
			case 37:
				if (adj_slot[left] != 37)
					return true;
				break;
		}
		return false;
	}
	void autopattern_dormtn::find_sides()
	{
		for (auto a : tops)
		{
			apcombo* ap = a.second;
			if (ap != NULL)
			{
				switch (ap->slot)
				{
					case 24:
						if(!ap->adj[right])
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, up, height);
						else if (ap->adj[right] && ap->adj[right]->slot == 26)
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, up, height);
						break;
					case 26:
						if (!ap->adj[left])
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, up, height);
						else if (ap->adj[left] && ap->adj[left]->slot == 24)
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, up, height);
						break;
					case 36:
						if (ap->adj[up] && ap->adj[up]->slot == 24)
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, left, height);
						break;
					case 38:
						if (ap->adj[up] && ap->adj[up]->slot == 26)
							side_faces[ap->screenpos + 0x10000] = new dor_face(ap, this, right, height);
						break;
				}
				if (is_vertex(ap))
				{
					if (!side_faces.count(ap->screenpos))
					{
						side_faces[ap->screenpos] = new dor_face(ap, this, height);
					}
				}
			}
		}
	}
	void autopattern_dormtn::update_sides()
	{
		for (auto a : side_faces)
		{
			dor_face* p = a.second;
			if (p->valid)
			{
				write_side(p);
			}
			delete p;
		}
		for (auto a : sides)
		{
			a.second->set_cid(slot_to_cid_pair(flags_to_slot(a.second->connflags)));
			a.second->force_cset = true;
		}
	}
	void autopattern_dormtn::write_side(dor_face* p)
	{
		int32_t h = p->height - 1;
		switch (p->facing_dir)
		{
			case up:
				h /= 2;
				for (int32_t q = 0; q <= h; ++q)
				{
					int32_t left_edge = 0;
					int32_t right_edge = 0;
					if (p->vert_dir[0] == l_up)
						left_edge -= q + 1 - (p->special_corner ? 1 : 0);
					else if (p->vert_dir[0] == r_up)
						left_edge += q;
					if (p->vert_dir[1] == l_up)
						right_edge -= q;
					else if (p->vert_dir[1] == r_up)
						right_edge += q + 1 - (p->special_corner ? 1 : 0);
					for (int32_t x = -p->height - 1; x <= p->side_length + p->height + 1; ++x)
					{
						if (x >= left_edge && x <= p->side_length + right_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, -1 - q);
							if(rel)
							{
								rel->connflags |= SIDE | U;
								if ((x == left_edge && p->outer[0]) || (x == p->side_length + right_edge && p->outer[1]))
									rel->connflags |= OUTERCORNER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
									if (p->height % 2 == 1)
										rel->connflags |= HALF;
								}
							}
						}
					}
				}
				break;
			case down:
				h = (h + 1) * 2 - 1;
				for (int32_t q = 0; q <= h; ++q)
				{
					int32_t left_edge = 0;
					int32_t right_edge = p->side_length;
					if (p->vert_dir[0] == r_down)
						left_edge += q / 2;
					if (p->vert_dir[1] == l_down)
						right_edge -= q / 2;
					for (int32_t x = 0; x <= p->side_length; ++x)
					{
						if (x >= left_edge && x <= right_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, 1 + q);
							if (rel)
							{
								rel->connflags |= SIDE | D;
								if (q % 2 == 0)
									rel->connflags |= SOUTHUPPER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case left:
				for(int32_t q = 0; q<= h; ++q)
				{
					int32_t x = -1 - q;
					for (int32_t y = -height; y <= p->side_length + height + 1; ++y)
					{
						int32_t top_edge = 0;
						int32_t bottom_edge = p->side_length;
						if (p->vert_dir[0] == l_down)
						{
							top_edge = q;
							if (p->steep)
								top_edge = q * 2;
						}
						if (p->vert_dir[1] == l_down)
							bottom_edge = q + p->side_length + 1 - (p->special_corner ? 1 : 0);;
						if(y >= top_edge && y<= bottom_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | L;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case right:
				for (int32_t q = 0; q <= h; ++q)
				{
					int32_t x = 1 + q;
					for (int32_t y = -height; y <= p->side_length + height + 1; ++y)
					{
						int32_t top_edge = 0;
						int32_t bottom_edge = p->side_length;
						if (p->vert_dir[0] == r_down)
						{
							top_edge = q;
							if (p->steep)
								top_edge = q * 2;
						}
						if (p->vert_dir[1] == r_down)
							bottom_edge = q + p->side_length + 1 - (p->special_corner ? 1 : 0);;
						if (y >= top_edge && y <= bottom_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | R;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case l_up:
				for (int32_t q = 0; q <= h; ++q)
				{
					for (int32_t x = -p->height - 1; x <= p->side_length; ++x)
					{
						int32_t y = -1 - q - x;
						int32_t top_edge = -p->side_length;
						int32_t bottom_edge = 0;
						int32_t left_edge = -p->height - 1;
						int32_t right_edge = p->side_length;
						if (p->vert_dir[0] == l_up)
						{
							left_edge = -q / 2 - 1;
						}
						if (p->vert_dir[1] == l_up)
						{
							top_edge -= h + 1;
							right_edge = -(q + 1) / 2 + p->side_length;
						}
						if (x >= left_edge && x <= right_edge && y >= top_edge && y <= bottom_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | UL;
								if ((x == left_edge && p->outer[0]) || (x == right_edge && p->outer[1]))
									rel->connflags |= OUTERCORNER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case r_up:
				for (int32_t q = 0; q <= h; ++q)
				{
					for (int32_t x = 0; x <= p->side_length + p->height + 1; ++x)
					{
						int32_t y = -1 - q + x;
						int32_t top_edge = 0;
						int32_t bottom_edge = p->side_length;
						int32_t left_edge = 0;
						int32_t right_edge = p->side_length + p->height + 1;
						if (p->vert_dir[0] == r_up)
						{
							top_edge -= h + 1;
							left_edge = (q + 1) / 2;
						}
						if (p->vert_dir[1] == r_up)
							right_edge = q / 2 + p->side_length + 1;
						if (x >= left_edge && x <= right_edge && y >= top_edge && y <= bottom_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | UR;
								if ((x == left_edge && p->outer[0]) || (x == right_edge && p->outer[1]))
									rel->connflags |= OUTERCORNER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case l_down:
				h = (h + 1) * 2 - 1;
				for (int32_t q = 0; q <= h; ++q)
				{
					for (int32_t x = -h / 2 - 1; x <= p->side_length; ++x)
					{
						int32_t y = 1 + q + x;
						int32_t left_edge = 0;
						int32_t right_edge = p->side_length;
						if (p->vert_dir[0] == l_down)
							left_edge = -q / 2 - 1;
						if (p->vert_dir[1] == l_down)
							right_edge = -(q + 1) / 2 + p->side_length;
						if (x >= left_edge && x <= right_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | DL;
								if ((x == left_edge && p->outer[0]) || (x == right_edge && p->outer[1]))
									rel->connflags |= OUTERCORNER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
			case r_down:
				h = (h + 1) * 2 - 1;
				for (int32_t q = 0; q <= h; ++q)
				{
					for (int32_t x = 0; x <= p->side_length + h / 2 + 1; ++x)
					{
						int32_t y = 1 + q - x;
						int32_t left_edge = 0;
						int32_t right_edge = p->side_length;
						if (p->vert_dir[0] == r_down)
							left_edge = (q + 1) / 2;
						if (p->vert_dir[1] == r_down)
							right_edge = q / 2 + p->side_length + 1;
						if (x >= left_edge && x <= right_edge)
						{
							apcombo* rel = add_side_combo(p->vert[0], x, y);
							if (rel)
							{
								rel->connflags |= SIDE | DR;
								if ((x == left_edge && p->outer[0]) || (x == right_edge && p->outer[1]))
									rel->connflags |= OUTERCORNER;
								if (q == 0)
									rel->connflags |= RIM;
								if (q == h)
								{
									rel->connflags |= BASE;
								}
							}
						}
					}
				}
				break;
		}
	}
	void autopattern_dormtn::mask_out_context(apcombo* center)
	{
		for (auto a : combos)
		{
			apcombo* ap = a.second;
			if (ap->x < center->x - height - 1 || ap->x > center->x + height + 1 || ap->y < center->y - (height + 1) / 2 - 1 || ap->y > center->y + height * 2 + 1)
				ap->changed = false;
		}
	}
	void autopattern_dormtn::erase_sides(apcombo* center)
	{
		for (int32_t x = -height; x <= height; ++x)
		{
			for (int32_t y = -(height + 1) / 2; y <= height * 2; ++y)
			{
				int32_t top_edge;
				int32_t bottom_edge;
				if (x < 0)
				{
					top_edge = -x - height;
					bottom_edge = x + height * 2;
				}
				else
				{
					top_edge = x - height;
					bottom_edge = -x + height * 2;
				}
				if (y >= top_edge && y <= bottom_edge)
				{
					apcombo* rel = add_relative(center, x, y);
					if (rel)
					{
						if (!is_top(rel->slot))
						{
							rel->cid = erase_cid;
							rel->force_cset = true;
							rel->changed = true;
						}
					}
				}
			}
		}
	}
	apcombo* autopattern_dormtn::add_side_combo(apcombo* p, int32_t x, int32_t y)
	{
		apcombo* ret = add_relative(p, x, y);
		if (ret)
		{
			if (is_top(ret->slot))
				return NULL;
			if (!sides.count(ret->screenpos))
			{
				sides[ret->screenpos] = ret;
				ret->connflags = 0;
			}
		}
		return ret;
	}

	dor_face::dor_face(apcombo* firstvertex, autopattern_dormtn* npattern, byte nheight) :
		valid(true), scan_dir(0), pattern(npattern), height(nheight), facing_dir(0), side_length(0), special_corner(false), steep(false)
	{
		byte vertexfacing = 255;
		byte scan = 255;
		byte facing = 255;
		get_vertex_dir(firstvertex, facing, vertexfacing, scan, true);
		if (facing == 255 || scan == 255 )
		{
			valid = false;
		}
		else
		{
			facing_dir = facing;
			vert[0] = firstvertex;
			vert_dir[0] = vertexfacing;
			scan_dir = scan;

			apcombo* cur = firstvertex;
			int32_t og_slot = cur->slot;
			apcombo* last = cur;
			for (int32_t q = 0; q < 256; ++q)
			{
				last = cur;
				cur = npattern->add(cur, scan_dir, false, false);
				if (cur)
				{
					if(cur->slot!=og_slot)
					{
						int32_t slot = cur->slot;
						if (slot != og_slot)
						{
							get_vertex_dir(last, facing, vertexfacing, scan, false);
							vert[1] = last;
							vert_dir[1] = vertexfacing;
							side_length = q;
							break;
						}
					}
				}
				else
				{
					get_vertex_dir(last, facing, vertexfacing, scan, false);
					vert[1] = last;
					vert_dir[1] = vertexfacing;
					side_length = q;
					break;
				}
			}
		}
	}
	dor_face::dor_face(apcombo* firstvertex, autopattern_dormtn* npattern, byte ndir, byte nheight) :
		valid(true), scan_dir(0), pattern(npattern), height(nheight), facing_dir(ndir), side_length(0), special_corner(true), steep(false)
	{
		if (firstvertex)
		{
			vert[0] = firstvertex;
			vert[1] = firstvertex;
			outer[0] = true;
			outer[1] = true;
			valid = true;
			switch (firstvertex->slot)
			{
				case 24:
					vert_dir[0] = l_up;
					vert_dir[1] = up;
					break;
				case 26:
					vert_dir[0] = up;
					vert_dir[1] = r_up;
					break;
				case 36:
					vert_dir[0] = left;
					vert_dir[1] = l_down;
					break;
				case 38:
					vert_dir[0] = right;
					vert_dir[1] = r_down;
					break;
			}
		}
		else
			valid = false;
	}
	void dor_face::get_vertex_dir(apcombo* v, byte& facing, byte& vertexfacing, byte& scan, bool first)
	{
		int32_t adj_slot[8];
		int32_t slot = v->slot;
		for (int32_t q = 0; q < 8; ++q)
		{
			if (v->adj[q])
				adj_slot[q] = v->adj[q]->slot;
			else
				adj_slot[q] = -1;
		}
		switch (slot)
		{
			case 24:
				if (first)
				{
					if (adj_slot[l_down] == 25)
						vertexfacing = l_up;
					else
					{
						vertexfacing = left;
						outer[0] = true;
					}
				}
				else
				{
					if (adj_slot[r_up] == 30)
						vertexfacing = left;
					else
					{
						vertexfacing = l_up;
						outer[1] = true;
					}
				}
				scan = r_up;
				facing = l_up;
				break;
			case 26:
				if (first)
				{
					if (adj_slot[l_up] == 32)
						vertexfacing = right;
					else
					{
						vertexfacing = r_up;
						outer[0] = true;
					}
				}
				else
				{
					if (adj_slot[r_down] == 25)
					{
						vertexfacing = r_up;
					}
					else
					{
						vertexfacing = right;
						outer[1] = true;
					}
				}
				scan = r_down;
				facing = r_up;
				break;
			case 36:
				if (first)
				{
					if (adj_slot[l_up] == 37)
						vertexfacing = down;
					else
					{
						vertexfacing = l_down;
						outer[0] = true;
					}
				}
				else
				{
					if (adj_slot[r_down] == 30)
						vertexfacing = l_down;
					else{
						vertexfacing = down;
						outer[1] = true;
					}
				}
				scan = r_down;
				facing = l_down;
				break;
			case 38:
				if (first)
				{
					if (adj_slot[l_down] == 32)
						vertexfacing = r_down;
					else
					{
						vertexfacing = down;
						outer[0] = true;
					}
				}
				else
				{
					if (adj_slot[r_up] == 37)
						vertexfacing = down;
					else
					{
						vertexfacing = r_down;
						outer[1] = true;
					}
				}
				scan = r_up;
				facing = r_down;
				break;

			case 25:
				if (first)
				{
					if (adj_slot[left] == -1)
						vertexfacing = up;
					else if (adj_slot[left] == 24)
						vertexfacing = l_up;
					else
						vertexfacing = r_up;
				}
				else
				{
					if (adj_slot[right] == -1)
						vertexfacing = up;
					else if (adj_slot[right] == 26)
						vertexfacing = r_up;
					else
						vertexfacing = l_up;
				}
				scan = right;
				facing = up;
				break;
			case 30:
				if (first)
				{
					if (adj_slot[up] == 33)
					{
						vertexfacing = l_down;
						if (adj_slot[l_up] == 37)
							steep = true;
					}
					else
						vertexfacing = left;
				}
				else
				{
					if (adj_slot[down] == 36)
						vertexfacing = l_down;
					else
						vertexfacing = left;
				}
				scan = down;
				facing = left;
				break;
			case 32:
				if (first)
				{
					if (adj_slot[up] == 35)
					{
						vertexfacing = r_down;
						if (adj_slot[r_up] == 37)
							steep = true;
					}
					else
						vertexfacing = right;
				}
				else
				{
					if (adj_slot[down] == 38)
						vertexfacing = r_down;
					else
						vertexfacing = right;
				}
				scan = down;
				facing = right;
				break;
			case 37:
				if (first)
				{
					if (adj_slot[l_down] == 32)
						vertexfacing = r_down;
					else
						vertexfacing = down;
				}
				else
				{
					if (adj_slot[r_down] == 30)
						vertexfacing = l_down;
					else
						vertexfacing = down;
				}
				scan = right;
				facing = down;
				break;
		}
	}
}