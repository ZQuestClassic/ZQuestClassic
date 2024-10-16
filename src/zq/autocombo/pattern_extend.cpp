#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_extend.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_extend::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		bool ret = execute_single(ap);
		if (ret && dogroup)
		{
			bool old_nocreate = nocreate;
			nocreate = true;
			for (int32_t dir = 0; dir < 4; ++dir)
			{
				if ((vertical && dir >= 2) || (!vertical && dir < 2))
				{
					int32_t xoff = 0;
					int32_t yoff = 0;
					switch (dir)
					{
						case up: yoff = -1; break;
						case down: yoff = 1; break;
						case left: xoff = -1; break;
						case right: xoff = 1; break;
					}
					for (int32_t q = 1; q < 256; ++q)
					{
						apcombo* ap2 = add_relative(ap, xoff * q, yoff * q);
						if (ap2)
						{
							if (!execute_single(ap2))
								break;
						}
						else
							break;
					}
				}
			}
			nocreate = old_nocreate;
		}
		return ret;
	}
	bool autopattern_extend::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		bool ret = erase_single(ap);
		if (ret && dogroup)
		{
			for (int32_t dir = 0; dir < 4; ++dir)
			{
				if ((vertical && dir >= 2) || (!vertical && dir < 2))
				{
					int32_t xoff = 0;
					int32_t yoff = 0;
					switch (dir)
					{
						case up: yoff = -1; break;
						case down: yoff = 1; break;
						case left: xoff = -1; break;
						case right: xoff = 1; break;
					}
					for (int32_t q = 1; q < 256; ++q)
					{
						apcombo* ap2 = add_relative(ap, xoff * q, yoff * q);
						if(ap2)
						{
							if (ap2->slot > -1)
							{
								if (!erase_single(ap2))
									break;
							}
							else
								break;
						}
						else
							break;
					}
				}
			}
		}
		return ret;
	}
	bool autopattern_extend::execute_single(apcombo* p)
	{
		if (!p)
			return false;
		init_connections(p);
		filter_connections(p);
		for (int32_t q = 0; q < 4; ++q)
		{
			init_connections(p->adj[q]);
			filter_connections(p->adj[q]);
		}
		p->in_set = true;
		p->force_cset = true;
		calculate_connections(p);
		if (nocreate && (!(p->connflags & 0xF) || !(p->connflags >> 4)))
			return false;

		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q])
			{
				calculate_connections(p->adj[q]);
			}
		}
		p->set_cid(slot_to_cid_pair(flags_to_slot(p->connflags)));
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q] && p->adj[q]->in_set)
				p->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(p->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	bool autopattern_extend::erase_single(apcombo* p)
	{
		if (!p)
			return false;
		p->cid = erase_cid;
		init_connections(p);
		filter_connections(p);
		if (nocreate && !(slot_to_flags(p->slot) & 0xF))
			return false;
		p->write(layer, true);
		p->in_set = false;
		p->force_cset = true;
		for (int32_t q = 0; q < 4; ++q)
		{
			init_connections(p->adj[q]);
			filter_connections(p->adj[q]);
		}
		p->in_set = false;
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q])
			{
				calculate_connections(p->adj[q]);
			}
		}
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q] && p->adj[q]->in_set)
				p->adj[q]->set_cid(slot_to_cid_pair(flags_to_slot(p->adj[q]->connflags)));
		}
		apply_changes();
		return true;
	}
	int32_t autopattern_extend::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		init_connections(ap);
		filter_connections(ap);
		calculate_connections(ap);
		ap->set_cid(slot_to_cid_pair(flags_to_slot(ap->connflags)));
		return ap->cid;
	}
	void autopattern_extend::filter_connections(apcombo* p)
	{
		if (!p)
			return;
		for (int32_t q = 0; q < 4; ++q)
		{
			if (p->adj[q])
			{
				if((vertical&&q>=2)||(!vertical&&q<2))
				{
					p->adj[q]->in_set = false;
				}
				else if(p->adj[q]->slot>-1)
					p->adj[q]->in_set = true;
			}
		}
	}
	void autopattern_extend::calculate_connections(apcombo* p)
	{
		p->connflags = 0;
		int32_t thisgroup = 0;
		if (cid_to_slot(p->cid) > -1)
		{
			thisgroup = cid_to_slot(p->cid) / 4 + 1;
			p->connflags = thisgroup << 4;
		}
		for (int32_t q = 0; q < 4; ++q)
		{
			if((q<2&&vertical)||(q>=2&&!vertical))
			{
				if (p->adj[q])
				{
					if (p->adj[q]->in_set)
					{
						int32_t adjslot = cid_to_slot(p->adj[q]->cid);
						int32_t adjgroup = adjslot / 4 + 1;
						p->connflags |= (1 << q);
						if (!(p->connflags & 0xFFF0))
							p->connflags |= adjgroup << 4;
					}
				}
				else if (connectedge)
					p->connflags |= (1 << q);
			}
		}
	}
	uint32_t autopattern_extend::slot_to_flags(int32_t slot)
	{
		int32_t thisgroup = ((slot / 4) + 1) << 4;
		if(vertical)
		{
			switch (slot % 4)
			{
				case 0:
					return thisgroup | D;
				case 1:
					return thisgroup | U | D;
				case 2:
					return thisgroup | U;
				case 3:
					return thisgroup;
			}
		}
		else
		{
			switch (slot % 4)
			{
				case 0:
					return thisgroup | R;
				case 1:
					return thisgroup | L | R;
				case 2:
					return thisgroup | L;
				case 3:
					return thisgroup;
			}
		}
		return -1;
	}
	int32_t autopattern_extend::flags_to_slot(uint32_t flags)
	{
		if (flags == -1)
			return -1;
		int32_t thisgroup = (((flags & 0xFFF0) >> 4) - 1) * 4;
		if (thisgroup < 0) thisgroup = 0;
		switch (flags & 0xF)
		{
			case D:
			case R:
				return thisgroup + 0;
			case U | D:
			case L | R:
				return thisgroup + 1;
			case U:
			case L:
				return thisgroup + 2;
			case 0:
				return thisgroup + 3;
		}
		return -1;
	}

}