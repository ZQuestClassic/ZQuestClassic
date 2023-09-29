#include "zq/autocombo/pattern_flatmtn.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_flatmtn::execute(int32_t exscreen, int32_t expos)
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
	bool autopattern_flatmtn::erase(int32_t exscreen, int32_t expos)
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
	int32_t autopattern_flatmtn::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		init_connections(ap);
		calculate_connections(ap);
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		return ap->cid;
	}
	void autopattern_flatmtn::calculate_connections(apcombo* p)
	{
		p->connflags = 0;
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q] && p->adj[q]->in_set)
			{
				p->connflags |= (1 << q);
			}
		}
	}
	uint32_t autopattern_flatmtn::slot_to_flags(int32_t slot)
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
			return U | R;
		case 4:
			return U | L | R;
		case 5:
			return L | R;
		default:
			return U | D | L | R;
		}
	}
	int32_t autopattern_flatmtn::flags_to_slot(uint32_t flags)
	{
		switch (flags & 0xF)
		{
		case D | R:
			return 0;
		case D | L | R:
			return 1;
		case D | L:
			return 2;
		case D:
			return 1;
		case U | D | R:
			return 4;
		case U | D | L | R:
			return 4;
		case U | D | L:
			return 4;
		case U | D:
			return 4;
		case U | R:
			return 3;
		case U | L | R:
			return 4;
		case U | L:
			return 5;
		case U:
			return 4;
		case R:
			return 1;
		case L | R:
			return 1;
		case L:
			return 1;
		default:
			return 1;
		}
	}

}
