#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_basic.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_basic::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		init_connections(ap);
		for (int32_t q = 0; q < 4; ++q)
		{
			init_connections(ap->adj[q]);
		}
		calculate_connections(ap);
		for (int32_t q = 0; q < 4; ++q)
		{
			if (ap->adj[q])
			{
				calculate_connections(ap->adj[q]);
			}
		}
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		for (int32_t q = 0; q < 4; ++q)
		{
			if (ap->adj[q] && ap->adj[q]->in_set)
				ap->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	bool autopattern_basic::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		init_connections(ap);
		ap->write(layer, true);
		ap->in_set = false;
		for (int32_t q = 0; q < 4; ++q)
		{
			init_connections(ap->adj[q]);
		}
		for (int32_t q = 0; q < 4; ++q)
		{
			if (ap->adj[q])
			{
				calculate_connections(ap->adj[q]);
			}
		}
		for (int32_t q = 0; q < 4; ++q)
		{
			if (ap->adj[q] && ap->adj[q]->in_set)
				ap->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	int32_t autopattern_basic::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		init_connections(ap);
		calculate_connections(ap);
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		return ap->cid;
	}
	void autopattern_basic::calculate_connections(apcombo* p)
	{
		p->connflags = 0;
		if (connectsolid && (p->read_solid(layer) & 0xF) == 0xF)
		{
			p->connflags = -1;
			return;
		}
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q])
			{
				if(p->adj[q]->in_set)
					p->connflags |= (1 << q);
				else if (connectsolid)
				{
					if ((p->adj[q]->read_solid(layer) & 0xF) == 0xF)
						p->connflags |= (1 << q);
				}
			}
			else if(connectedge)
				p->connflags |= (1 << q);
		}
	}
	uint32_t autopattern_basic::slot_to_flags(int32_t slot)
	{
		switch (slot)
		{
			case 0:
				return D | R;
			case 1:
				return D | L | R;
			case 2:
				return D | L;
			case 3:
				return D;
			case 4:
				return U | D | R;
			case 5:
				return U | D | L | R;
			case 6:
				return U | D | L;
			case 7:
				return U | D;
			case 8:
				return U | R;
			case 9:
				return U | L | R;
			case 10:
				return U | L;
			case 11:
				return U;
			case 12:
				return R;
			case 13:
				return L | R;
			case 14:
				return L;
			default:
				return 0;
		}
	}
	int32_t autopattern_basic::flags_to_slot(uint32_t flags)
	{
		if (flags == -1)
			return -1;
		switch (flags & 0xF)
		{
			case D | R:
				return 0;
			case D | L |R :
				return 1;
			case D | L:
				return 2;
			case D:
				return 3;
			case U | D | R:
				return 4;
			case U | D | L | R:
				return 5;
			case U | D | L:
				return 6;
			case U | D:
				return 7;
			case U | R:
				return 8;
			case U | L | R:
				return 9;
			case U | L:
				return 10;
			case U:
				return 11;
			case R:
				return 12;
			case L | R:
				return 13;
			case L:
				return 14;
			default:
				return 15;
		}
	}

}