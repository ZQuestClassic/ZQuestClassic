#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_dungeoncarve.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_dungeoncarve::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		form_connections(ap, false);
		apply_changes();
		return true;
	}
	bool autopattern_dungeoncarve::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		ap->write(layer, true);
		ap->in_set = false;
		form_connections(ap, true);
		apply_changes();
		return true;
	}
	int32_t autopattern_dungeoncarve::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		form_connections(ap, false);
		return ap->cid;
	}
	void autopattern_dungeoncarve::form_connections(apcombo* p, bool rem)
	{
		apcombo* relatives[5][5];
		for (int32_t x = 0; x < 5; ++x)
		{
			for (int32_t y = 0; y < 5; ++y)
			{
				relatives[x][y] = add_relative(p, x - 2, y - 2);
				init_connections(relatives[x][y], true);
				if (relatives[x][y])
				{
					if (rem)
					{
						if (x >= 1 && y >= 1 && x <= 3 && y <= 3)
						{
							relatives[x][y]->connflags &= ~0x100;
							if (x == 2 && y == 2)
							{
								relatives[x][y]->in_set = false;
							}
						}
					}
					else
					{
						if (!relatives[x][y]->in_set && x >= 1 && y >= 1 && x <= 3 && y <= 3)
						{
							relatives[x][y]->in_set = true;
						}
						if (x == 2 && y == 2)
							relatives[x][y]->connflags |= 0x100;
					}
				}
			}
		}
		for (int32_t x = 0; x < 5; ++x)
		{
			for (int32_t y = 0; y < 5; ++y)
			{
				if (relatives[x][y] && relatives[x][y]->in_set)
				{
					calculate_connections(relatives[x][y]);
					int32_t slot = flags_to_slot(relatives[x][y]->connflags);
					relatives[x][y]->cid = slot_to_cid(slot);
					relatives[x][y]->force_cset = !rem;
					relatives[x][y]->changed = true;
				}
			}
		}
	}
	void autopattern_dungeoncarve::calculate_connections(apcombo* p)
	{
		int32_t h = p->connflags & 0x100;
		uint32_t newflags = 0;
		for (int32_t q = 0; q < 8; ++q)
		{
			if (p->adj[q])
			{
				if (p->adj[q]->in_set)
				{
					if (h)
					{
						if (p->adj[q]->connflags & 0x100)
							newflags |= (1 << q);
					}
					else
					{
						newflags |= (1 << q);
					}
				}
			}
			else if(connectedge || !(p->connflags & 0x100))
				newflags |= (1 << q);
		}
		p->connflags = newflags | h;
	}
	uint32_t autopattern_dungeoncarve::slot_to_flags(int32_t slot)
	{
		switch (slot)
		{
			// First four
			case 0:
				return 0x100 | U | D | L | R | UL | UR | DL | DR;
			case 1:
				return 0x100;
			case 2:
				return U | D | L | R | UL | UR | DL | DR;
			case 3:
				return 0x0;
			// Inner corners
			case 4:
				return 0x100 | U | D | L | R | UL | UR | DL;
			case 5:
				return 0x100 | U | D | L | R | UL | UR | DR;
			case 6:
				return U | D | L | R | UL | UR | DL;
			case 7:
				return U | D | L | R | UL | UR | DR;
			case 8:
				return 0x100 | U | D | L | R | UL | DL | DR;
			case 9:
				return 0x100 | U | D | L | R | UR | DL | DR;
			case 10:
				return U | D | L | R | UL | DL | DR;
			case 11:
				return U | D | L | R | UR | DL | DR;
			// Inner corners
			case 12:
				return 0x100 | D | R | DR;
			case 13:
				return 0x100 | D | L | DL;
			case 14:
				return D | R | DR;
			case 15:
				return D | L | DL;
			case 16:
				return 0x100 | U | R | UR;
			case 17:
				return 0x100 | U | L | UL;
			case 18:
				return U | R | UR;
			case 19:
				return U | L | UL;
			// Walls
			case 20:
				return D | L | R | DL | DR;
			case 21:
				return 0x100 | U | L | R | UL | UR;
			case 22:
				return U | D | R | UR | DR;
			case 23:
				return 0x100 | U | D | R | UR | DR;
			case 24:
				return 0x100 | D | L | R | DL | DR;
			case 25:
				return U | L | R | UL | UR;
			case 26:
				return 0x100 | U | D | L | UL | DL;
			case 27:
				return U | D | L | UL | DL;
			// Dead ends
			case 28:
				return 0x100 | D;
			case 29:
				return 0x100 | L;
			case 30:
				return D;
			case 31:
				return L;
			case 32:
				return 0x100 | R;
			case 33:
				return 0x100 | U;
			case 34:
				return R;
			case 35:
				return U;
			// Pipe ends
			case 36:
				return 0x100 | U | D;
			case 37:
				return 0x100 | L | R;
			case 38:
				return U | D;
			case 39:
				return L | R;
			// Pipe corners
			case 40:
				return 0x100 | D | R;
			case 41:
				return 0x100 | D | L;
			case 42:
				return D | R;
			case 43:
				return D | L;
			case 44:
				return 0x100 | U | R;
			case 45:
				return 0x100 | U | L;
			case 46:
				return U | R;
			case 47:
				return U | L;
			// T pieces
			case 48:
				return 0x100 | D | L | R;
			case 49:
				return 0x100 | U | D | L;
			case 50:
				return D | L | R;
			case 51:
				return U | D | L;
			case 52:
				return 0x100 | U | D | R;
			case 53:
				return 0x100 | U | L | R;
			case 54:
				return U | D | R;
			case 55:
				return U | L | R;
			// T connections
			case 56:
				return 0x100 | U | D | L | R | UL | UR;
			case 57:
				return 0x100 | U | D | L | R | UR | DR;
			case 58:
				return U | D | L | R | UL | UR;
			case 59:
				return U | D | L | R | UR | DR;
			case 60:
				return 0x100 | U | D | L | R | UL | DL;
			case 61:
				return 0x100 | U | D | L | R | DL | DR;
			case 62:
				return U | D | L | R | UL | DL;
			case 63:
				return U | D | L | R | DL | DR;
			// Double T connections
			case 64:
				return 0x100 | U | D | L | R | DR;
			case 65:
				return 0x100 | U | D | L | R | DL;
			case 66:
				return U | D | L | R | DR;
			case 67:
				return U | D | L | R | DL;
			case 68:
				return 0x100 | U | D | L | R | UR;
			case 69:
				return 0x100 | U | D | L | R | UL;
			case 70:
				return U | D | L | R | UR;
			case 71:
				return U | D | L | R | UL;
			// H
			case 72:
				return 0x100 | U | D | R | DR;
			case 73:
				return 0x100 | U | D | L | DL;
			case 74:
				return U | D | R | DR;
			case 75:
				return U | D | L | DL;
			case 76:
				return 0x100 | U | D | R | UR;
			case 77:
				return 0x100 | U | D | L | UL;
			case 78:
				return U | D | R | UR;
			case 79:
				return U | D | L | UL;
			// Sideways H
			case 80:
				return 0x100 | D | L | R | DR;
			case 81:
				return 0x100 | D | L | R | DL;
			case 82:
				return D | L | R | DR;
			case 83:
				return D | L | R | DL;
			case 84:
				return 0x100 | U | L | R | UR;
			case 85:
				return 0x100 | U | L | R | UL;
			case 86:
				return U | L | R | UR;
			case 87:
				return U | L | R | UL;
			// Diagonals
			case 88:
				return 0x100 | U | D | L | R | UR | DL;
			case 89:
				return 0x100 | U | D | L | R | UL | DR;
			case 90:
				return U | D | L | R | UR | DL;
			case 91:
				return U | D | L | R | UL | DR;
			// +
			case 92:
				return 0x100 | U | D | L | R;
			case 93:
				return U | D | L | R;
			default:
				return 0x0;
		}
	}
	int32_t autopattern_dungeoncarve::flags_to_slot(uint32_t flags)
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

		switch (flags)
		{
			// First four
			case  0x100 | U | D | L | R | UL | UR | DL | DR:
				return 0;
			case 0x100:
				return 1;
			case U | D | L | R | UL | UR | DL | DR:
				return 2;
			case 0x0:
				return 3;
			// Inner corners
			case 0x100 | U | D | L | R | UL | UR | DL:
				return 4;
			case 0x100 | U | D | L | R | UL | UR | DR:
				return 5;
			case U | D | L | R | UL | UR | DL:
				return 6;
			case U | D | L | R | UL | UR | DR:
				return 7;
			case 0x100 | U | D | L | R | UL | DL | DR:
				return 8;
			case 0x100 | U | D | L | R | UR | DL | DR:
				return 9;
			case U | D | L | R | UL | DL | DR:
				return 10;
			case U | D | L | R | UR | DL | DR:
				return 11;
			// Inner corners
			case 0x100 | D | R | DR:
				return 12;
			case 0x100 | D | L | DL:
				return 13;
			case D | R | DR:
				return 14;
			case D | L | DL:
				return 15;
			case 0x100 | U | R | UR:
				return 16;
			case 0x100 | U | L | UL:
				return 17;
			case U | R | UR:
				return 18;
			case U | L | UL:
				return 19;
			// Walls
			case D | L | R | DL | DR:
				return 20;
			case 0x100 | U | L | R | UL | UR:
				return 21;
			case U | D | R | UR | DR:
				return 22;
			case 0x100 | U | D | R | UR | DR:
				return 23;
			case 0x100 | D | L | R | DL | DR:
				return 24;
			case U | L | R | UL | UR:
				return 25;
			case 0x100 | U | D | L | UL | DL:
				return 26;
			case U | D | L | UL | DL:
				return 27;
			// Dead ends
			case 0x100 | D:
				return 28;
			case 0x100 | L:
				return 29;
			case D:
				return 30;
			case L:
				return 31;
			case 0x100 | R:
				return 32;
			case 0x100 | U:
				return 33;
			case R:
				return 34;
			case U:
				return 35;
			// Pipe ends
			case 0x100 | U | D:
				return 36;
			case 0x100 | L | R:
				return 37;
			case U | D:
				return 38;
			case L | R:
				return 39;
			// Pipe corners
			case 0x100 | D | R:
				return 40;
			case 0x100 | D | L:
				return 41;
			case D | R:
				return 42;
			case D | L:
				return 43;
			case 0x100 | U | R:
				return 44;
			case 0x100 | U | L:
				return 45;
			case U | R:
				return 46;
			case U | L:
				return 47;
			// T pieces
			case 0x100 | D | L | R:
				return 48;
			case 0x100 | U | D | L:
				return 49;
			case D | L | R:
				return 50;
			case U | D | L:
				return 51;
			case 0x100 | U | D | R:
				return 52;
			case 0x100 | U | L | R:
				return 53;
			case U | D | R:
				return 54;
			case U | L | R:
				return 55;
			// T connections
			case 0x100 | U | D | L | R | UL | UR:
				return 56;
			case 0x100 | U | D | L | R | UR | DR:
				return 57;
			case U | D | L | R | UL | UR:
				return 58;
			case U | D | L | R | UR | DR:
				return 59;
			case 0x100 | U | D | L | R | UL | DL:
				return 60;
			case 0x100 | U | D | L | R | DL | DR:
				return 61;
			case U | D | L | R | UL | DL:
				return 62;
			case U | D | L | R | DL | DR:
				return 63;
			// Double T connections
			case 0x100 | U | D | L | R | DR:
				return 64;
			case 0x100 | U | D | L | R | DL:
				return 65;
			case U | D | L | R | DR:
				return 66;
			case U | D | L | R | DL:
				return 67;
			case 0x100 | U | D | L | R | UR:
				return 68;
			case 0x100 | U | D | L | R | UL:
				return 69;
			case U | D | L | R | UR:
				return 70;
			case U | D | L | R | UL:
				return 71;
			// H
			case 0x100 | U | D | R | DR:
				return 72;
			case 0x100 | U | D | L | DL:
				return 73;
			case U | D | R | DR:
				return 74;
			case U | D | L | DL:
				return 75;
			case 0x100 | U | D | R | UR:
				return 76;
			case 0x100 | U | D | L | UL:
				return 77;
			case U | D | R | UR:
				return 78;
			case U | D | L | UL:
				return 79;
			// Sideways H
			case 0x100 | D | L | R | DR:
				return 80;
			case 0x100 | D | L | R | DL:
				return 81;
			case D | L | R | DR:
				return 82;
			case D | L | R | DL:
				return 83;
			case 0x100 | U | L | R | UR:
				return 84;
			case 0x100 | U | L | R | UL:
				return 85;
			case U | L | R | UR:
				return 86;
			case U | L | R | UL:
				return 87;
			// Diagonals
			case 0x100 | U | D | L | R | UR | DL:
				return 88;
			case 0x100 | U | D | L | R | UL | DR:
				return 89;
			case U | D | L | R | UR | DL:
				return 90;
			case U | D | L | R | UL | DR:
				return 91;
			// +
			case 0x100 | U | D | L | R:
				return 92;
			case U | D | L | R:
				return 93;
			default:
				return -1;
		}
	}

}