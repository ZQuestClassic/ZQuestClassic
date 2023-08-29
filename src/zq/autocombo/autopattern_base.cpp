#include "zq/autocombo/autopattern_base.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"

namespace AutoPattern
{

    // autopattern_container

	autopattern_container::autopattern_container(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, bool nnocrossedge) :
		type(ntype), layer(nlayer), basescreen(nbasescreen), basepos(nbasepos), source(nsource), nocrossedge(nnocrossedge)
	{
		erase_cid = source->getEraseCombo();
		init_pattern();
	}
	autopattern_container::~autopattern_container()
	{
		for (auto c : combos)
		{
			delete c.second;
		}
	}
	void autopattern_container::init_pattern()
	{
		for (auto ca : source->combos)
		{
			pattern_cids[ca.offset] = ca.cid;
			pattern_slots[ca.cid] = ca.offset + 1;
		}
	}
	void autopattern_container::apply_changes()
	{
		for (auto c : combos)
		{
			if (c.second->changed)
			{
				c.second->write(layer, basescreen * 176 + basepos == c.first || c.second->force_cset);
			}
		}
	}
	
	int32_t autopattern_container::cid_to_slot(int32_t cid)
	{
		if (pattern_slots.count(cid))
		{
			return pattern_slots.at(cid) - 1;
		}
		return -1;
	}
	int32_t autopattern_container::slot_to_cid(int32_t slot)
	{
		if (pattern_cids.count(slot))
		{
			return pattern_cids.at(slot);
		}
		return 0;
	}

	apcombo* autopattern_container::add(int32_t screenpos, bool forcevalid, bool andgenerate)
	{
		if (!combos.count(screenpos))
		{
			apcombo* p = new apcombo(layer, screenpos);
			combos[screenpos] = p;
			p->in_set = cid_to_slot(p->cid) > -1 || forcevalid;
			if (forcevalid)
				p->changed = true;
			if(p->in_set)
				p->connflags = slot_to_flags(cid_to_slot(p->cid));
			init_connections(p, andgenerate);
			return p;
		}
		else
			return combos[screenpos];
	}
	apcombo* autopattern_container::add(int32_t screen, int32_t pos, bool forcevalid, bool andgenerate)
	{
		int32_t screenpos = screen * 176 + pos;
		if (!combos.count(screenpos))
		{
			apcombo* p = new apcombo(layer, screenpos);
			combos[screenpos] = p;
			p->in_set = cid_to_slot(p->cid) > -1 || forcevalid;
			if (forcevalid)
				p->changed = true;
			if (p->in_set)
				p->connflags = slot_to_flags(cid_to_slot(p->cid));
			init_connections(p, andgenerate);
			return p;
		}
		else
			return combos[screenpos];
	}
	apcombo* autopattern_container::add(apcombo*& ap, int32_t dir, bool forcevalid, bool andgenerate)
	{
		int32_t x = (ap->screen % 16) * 16 + ap->pos % 16;
		int32_t y = (ap->screen / 16) * 11 + ap->pos / 16;
		byte apscreen = ((x / 16) + (y / 11) * 16);
		byte appos = ((x % 16) + (y % 11) * 16);
		int32_t iq;
		switch (dir)
		{
			case up: --y; iq = 1; break;
			case down: ++y; iq = 0; break;
			case left: --x; iq = 3; break;
			case right: ++x; iq = 2; break;
		}
		if (offscreen(x, y))
		{
			return nullptr;
		}
		else
		{
			int16_t screenpos = ((x / 16) + (y / 11) * 16) * 176 + ((x % 16) + (y % 11) * 16);
			apcombo* ret = add(screenpos, forcevalid, andgenerate);
			if (ret && combos.count(screenpos))
			{
				ap->adj[dir] = combos[screenpos];
				combos[screenpos]->adj[oppositeDir[dir]] = ap;
			}
			return ret;
		}
	}
	void autopattern_container::remove(apcombo* ptr)
	{
		combos.erase(ptr->screenpos);
		delete ptr;
	}
	void autopattern_container::init_connections(apcombo* p, bool andgenerate)
	{
		if (!p)
			return;
		for (int32_t q = 0; q < 8; ++q)
		{
			p->adj[q] = nullptr;

			int32_t x = (p->screen % 16) * 16 + p->pos % 16;
			int32_t y = (p->screen / 16) * 11 + p->pos / 16;
			byte apscreen = ((x / 16) + (y / 11) * 16);
			byte appos = ((x % 16) + (y % 11) * 16);
			switch (q)
			{
				case up: --y; break;
				case down: ++y; break;
				case left: --x; break;
				case right: ++x; break;
				case l_up: --x;  --y; break;
				case r_up: ++x;  --y; break;
				case l_down: --x;  ++y; break;
				case r_down: ++x;  ++y; break;
			}
			if (offscreen(x, y))
			{
				p->adj[q] = nullptr;
			}
			else
			{
				int16_t adjp = ((x / 16) + (y / 11) * 16) * 176 + ((x % 16) + (y % 11) * 16);
				if (combos.count(adjp))
				{
					p->adj[q] = combos[adjp];
					combos[adjp]->adj[oppositeDir[q]] = p;
				}
				else
				{
					if (andgenerate)
					{
						apscreen = ((x / 16) + (y / 11) * 16);
						appos = ((x % 16) + (y % 11) * 16);
						apcombo* apc = add(apscreen, appos, false, false);
					}
					else
						p->adj[q] = nullptr;
				}
			}
		}
	}
	bool autopattern_container::offscreen(int32_t x, int32_t y)
	{
		if (x < 0 || x>255 || y < 0 || y>87)
			return true;
		if (nocrossedge)
		{
			byte apscreen = ((x / 16) + (y / 11) * 16);
			if (apscreen != basescreen)
				return true;
		}
		return false;
	}

	// apcombo

	apcombo::apcombo(byte nlayer, int32_t nscreenpos) :
		screenpos(nscreenpos), screen(nscreenpos / 176), pos(nscreenpos % 176)
	{
		read(nlayer);
	}
	apcombo::apcombo(byte nlayer, int32_t nscreen, int32_t npos) :
		screenpos(nscreen * 176 + npos), screen(nscreen), pos(npos)
	{
		read(nlayer);
	}
	void apcombo::read(byte layer)
	{
		int32_t drawmap = Map.getCurrMap();
		mapscr* mapscr_ptr = Map.AbsoluteScr(drawmap, screen);
		if (mapscr_ptr)
		{
			if (layer > 0)
			{
				if (mapscr_ptr->layermap[layer - 1])
				{
					mapscr_ptr = Map.AbsoluteScr(mapscr_ptr->layermap[layer - 1] - 1, mapscr_ptr->layerscreen[layer - 1]);
				}
			}
			cid = mapscr_ptr->data[pos];
		}
		else
		{
			cid = 0;
		}
	}
	void apcombo::write(byte layer, bool base)
	{
		int32_t drawmap = Map.getCurrMap();
		int32_t drawscreen = screen;
		mapscr* mapscr_ptr = Map.AbsoluteScr(drawmap, drawscreen);
		if (layer > 0)
		{
			if (mapscr_ptr->layermap[layer - 1])
			{
				drawmap = mapscr_ptr->layermap[layer - 1] - 1;
				drawscreen = mapscr_ptr->layerscreen[layer - 1];
				mapscr_ptr = Map.AbsoluteScr(drawmap, drawscreen);
			}
		}
		int32_t cset = mapscr_ptr->cset[pos];
		Map.DoSetComboCommand(drawmap, drawscreen, pos, cid, base ? CSet : cset);
	}
}
