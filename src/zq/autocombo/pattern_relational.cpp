#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_relational.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_relational::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		init_connections(ap);
		for (int32_t q = 0; q < 8; ++q)
		{
			init_connections(ap->adj[q]);
		}
		calculate_connections(ap);
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q])
			{
				calculate_connections(ap->adj[q]);
			}
		}
		ap->set_cid(slot_to_cid(flags_to_slot(ap->connflags)));
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q] && ap->adj[q]->in_set)
				ap->adj[q]->set_cid(slot_to_cid(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	bool autopattern_relational::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		init_connections(ap);
		ap->write(layer, true);
		ap->in_set = false;
		for (int32_t q = 0; q < 8; ++q)
		{
			init_connections(ap->adj[q]);
		}
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q])
			{
				calculate_connections(ap->adj[q]);
			}
		}
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q] && ap->adj[q]->in_set)
				ap->adj[q]->set_cid(slot_to_cid(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	void autopattern_relational::calculate_connections(apcombo* p)
	{
		p->connflags = 0;
		for (int32_t q = 0; q < 8; ++q)
		{
			if (p->adj[q] && p->adj[q]->in_set)
			{
				p->connflags |= (1 << q);
			}
		}
	}
	uint32_t autopattern_relational::slot_to_flags(int32_t slot)
	{
		switch (slot)
		{
			case 0:
				return D | R | DR;
			case 1:
				return D | L | R | DL | DR;
			case 2:
				return D | L | DL;
			case 3:
				return D;
			case 4:
				return U | D | R | UR | DR;
			case 5:
				return U | D | L | R | UL | UR | DL | DR;
			case 6:
				return U | D | L | UL | DL;
			case 7:
				return U | D;
			case 8:
				return U | R | UR;
			case 9:
				return U | L | R | UL | UR;
			case 10:
				return U | L | UL;
			case 11:
				return U;
			case 12:
				return R;
			case 13:
				return L | R;
			case 14:
				return L;
			case 15:
				return 0;
			case 16:
				return U | D | L | R | UL | UR | DL;
			case 17:
				return U | D | L | R | UL | UR | DR;
			case 18:
				return D | R;
			case 19:
				return D | L;
			case 20:
				return U | D | L | R | UL | DL | DR;
			case 21:
				return U | D | L | R | UR | DL | DR;
			case 22:
				return U | R;
			case 23:
				return U | L;
			case 24:
				return U | D | L | R | UL | UR;
			case 25:
				return U | D | L | R | UR | DR;
			case 26:
				return D | L | R;
			case 27:
				return U | D | L;
			case 28:
				return U | D | L | R | UL | DL;
			case 29:
				return U | D | L | R | DL | DR;
			case 30:
				return U | D | R;
			case 31:
				return U | L | R;
			case 32:
				return D | L | R | DR;
			case 33:
				return D | L | R | DL;
			case 34:
				return U | D | R | DR;
			case 35:
				return U | D | L | DL;
			case 36:
				return U | L | R | UR;
			case 37:
				return U | L | R | UL;
			case 38:
				return U | D | R | UR;
			case 39:
				return U | D | L | UL;
			case 40:
				return U | D | L | R | DR;
			case 41:
				return U | D | L | R | DL;
			case 42:
				return U | D | L | R | UL | DR;
			case 43:
				return U | D | L | R | UR | DL;
			case 44:
				return U | D | L | R | UR;
			case 45:
				return U | D | L | R | UL;
			case 46:
				return U | D | L | R;
			default:
				return 0;
		}
	}
	int32_t autopattern_relational::flags_to_slot(uint32_t flags)
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
				return 0;
			case D | L | R | DL | DR:
				return 1;
			case D | L | DL:
				return 2;
			case D:
				return 3;
			case U | D | R | UR | DR:
				return 4;
			case U | D | L | R | UL | UR | DL | DR:
				return 5;
			case U | D | L | UL | DL:
				return 6;
			case U | D:
				return 7;
			case U | R | UR:
				return 8;
			case U | L | R | UL | UR:
				return 9;
			case U | L | UL:
				return 10;
			case U:
				return 11;
			case R:
				return 12;
			case L | R:
				return 13;
			case L:
				return 14;
			case 0:
				return 15;
			case U | D | L | R | UL | UR | DL:
				return 16;
			case U | D | L | R | UL | UR | DR:
				return 17;
			case D | R:
				return 18;
			case D | L:
				return 19;
			case U | D | L | R | UL | DL | DR:
				return 20;
			case U | D | L | R | UR | DL | DR:
				return 21;
			case U | R:
				return 22;
			case U | L:
				return 23;
			case U | D | L | R | UL | UR:
				return 24;
			case U | D | L | R | UR | DR:
				return 25;
			case D | L | R:
				return 26;
			case U | D | L:
				return 27;
			case U | D | L | R | UL | DL:
				return 28;
			case U | D | L | R | DL | DR:
				return 29;
			case U | D | R:
				return 30;
			case U | L | R:
				return 31;
			case D | L | R | DR:
				return 32;
			case D | L | R | DL:
				return 33;
			case U | D | R | DR:
				return 34;
			case U | D | L | DL:
				return 35;
			case U | L | R | UR:
				return 36;
			case U | L | R | UL:
				return 37;
			case U | D | R | UR:
				return 38;
			case U | D | L | UL:
				return 39;
			case U | D | L | R | DR:
				return 40;
			case U | D | L | R | DL:
				return 41;
			case U | D | L | R | UL | DR:
				return 42;
			case U | D | L | R | UR | DL:
				return 43;
			case U | D | L | R | UR:
				return 44;
			case U | D | L | R | UL:
				return 45;
			case U | D | L | R:
				return 46;
			default:
				return 15;
		}
	}

}