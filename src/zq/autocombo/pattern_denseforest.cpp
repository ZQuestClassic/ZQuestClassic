#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_denseforest.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_denseforest::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->set_cid(slot_to_cid_pair(6));
		init_connections(ap);
		filter_connections(ap);
		for (int32_t q = 0; q < 8; ++q)
		{
			init_connections(ap->adj[q]);
			filter_connections(ap->adj[q]);
		}
		calculate_connections(ap);
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q])
			{
				calculate_connections(ap->adj[q]);
			}
		}
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		for (int32_t q = 0; q < 8; ++q)
		{
			if (ap->adj[q] && ap->adj[q]->in_set)
				ap->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	bool autopattern_denseforest::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		init_connections(ap);
		filter_connections(ap);
		ap->write(layer, true);
		ap->in_set = false;
		for (int32_t q = 0; q < 8; ++q)
		{
			init_connections(ap->adj[q]);
			filter_connections(ap->adj[q]);
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
				ap->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(ap->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	int32_t autopattern_denseforest::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		return ap->cid;
		if (!ap)
			return 0;
		init_connections(ap);
		calculate_connections(ap);
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		return ap->cid;
	}
	int32_t autopattern_denseforest::overlap_slot(int32_t slot)
	{
		switch (slot)
		{
			case 0:
			case 5:
				return 2;
			case 1:
			case 4:
				return 3;
		}
		return slot;
	}
	void autopattern_denseforest::filter_connections(apcombo* p)
	{
		if (!p)
			return;
		for (int32_t q = 0; q < 8; ++q)
		{
			if(p->adj[q])
			{
				if (p->adj[q]->slot == 6)
					continue;
				int32_t x = (2 + p->adj[q]->x - offsets.first) % 2;
				int32_t y = (2 + p->adj[q]->y - offsets.second) % 2;
				int32_t slotOnGrid = x + y * 4;
				x = (3 + p->adj[q]->x - offsets.first) % 2;
				y = (3 + p->adj[q]->y - offsets.second) % 2;
				int32_t slotOffGrid = x + y * 4;
				if (p->adj[q]->slot != slotOnGrid && p->adj[q]->slot != overlap_slot(slotOnGrid) &&
					p->adj[q]->slot != slotOffGrid && p->adj[q]->slot != overlap_slot(slotOffGrid))
				{
					p->adj[q]->in_set = false;
				}
			}
		}
	}
	void autopattern_denseforest::calculate_connections(apcombo* p)
	{
		p->connflags = 0;

		int32_t countUL = 0;
		int32_t countUR = 0;
		int32_t countDL = 0;
		int32_t countDR = 0;

		int32_t x = (2 + p->x - offsets.first) % 2;
		int32_t y = (2 + p->y - offsets.second) % 2;
		p->connflags = x + y * 4;
		int32_t slotOnGrid = x + y * 4;

		x = (3 + p->x - offsets.first) % 2;
		y = (3 + p->y - offsets.second) % 2;
		int32_t slotOffGrid = x + y * 4;

		for (int32_t q = 0; q < 8; ++q)
		{
			bool incrementcount = false;
			if (p->adj[q])
			{
				if(p->adj[q]->in_set)
				{
					incrementcount = true;
				}
				else if (connectsolid && cid_to_slot(p->adj[q]->cid) == -1)
				{
					if ((p->adj[q]->read_solid(layer) & 0xF) == 0xF)
						incrementcount = true;
				}
			}
			else if (connectedge)
			{
				incrementcount = true;
			}
			if(incrementcount)
			{
				switch (q)
				{
					case l_up:
						++countUL;
						break;
					case r_up:
						++countUR;
						break;
					case l_down:
						++countDL;
						break;
					case r_down:
						++countDR;
						break;
					case up:
						++countUL;
						++countUR;
						break;
					case down:
						++countDL;
						++countDR;
						break;
					case left:
						++countUL;
						++countDL;
						break;
					case right:
						++countUR;
						++countDR;
						break;
				}
			}
		}
		
		switch (slotOnGrid)
		{
			case 0:
			{
				if (countDR < 3)
					slotOnGrid = 6;
				break;
			}
			case 1:
			{
				if (countDL < 3)
					slotOnGrid = 6;
				break;
			}
			case 4:
			{
				if (countUR < 3)
					slotOnGrid = 6;
				break;
			}
			case 5:
			{
				if (countUL < 3)
					slotOnGrid = 6;
				break;
			}
		}
		switch (slotOffGrid)
		{
			case 0:
			{
				if (countDR < 3)
					slotOffGrid = 6;
				break;
			}
			case 1:
			{
				if (countDL < 3)
					slotOffGrid = 6;
				break;
			}
			case 4:
			{
				if (countUR < 3)
					slotOffGrid = 6;
				break;
			}
			case 5:
			{
				if (countUL < 3)
					slotOffGrid = 6;
				break;
			}
		}

		if (slotOnGrid != 6 && slotOffGrid != 6)
		{
			p->connflags = overlap_slot(slotOnGrid);
			return;
		}
		if (slotOnGrid != 6)
		{
			p->connflags = slotOnGrid;
			return;
		}
		if (slotOffGrid != 6)
		{
			p->connflags = slotOffGrid;
			return;
		}

		p->connflags = 6;
	}
	uint32_t autopattern_denseforest::slot_to_flags(int32_t slot)
	{
		return slot;
	}
	int32_t autopattern_denseforest::flags_to_slot(uint32_t flags)
	{
		if (flags == -1)
			return -1;
		return flags;
	}

}