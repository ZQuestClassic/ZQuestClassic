#include "zq/autocombo/autopattern_base.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"

namespace AutoPattern
{

    // autopattern_container

	autopattern_container::autopattern_container(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource) :
		type(ntype), layer(nlayer), basescreen(nbasescreen), basepos(nbasepos), source(nsource), 
		nocrossedge(!(nsource->flags&ACF_CROSSSCREENS)), connectedge(nsource->flags&ACF_CONNECTEDGE),
		basescreen_x((basescreen % 16) * 16), basescreen_y((basescreen / 16) * 11),
		base_x(basescreen_x + (basepos % 16)), base_y(basescreen_y + (basepos / 16)),
		screenboundary_x(0), screenboundary_y(0)
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
	std::pair<int32_t, int32_t> autopattern_container::slot_to_cid_pair(int32_t slot)
	{
		return std::make_pair(slot_to_cid(slot), slot);
	}

	apcombo* autopattern_container::add(int32_t screenpos, bool forcevalid, bool andgenerate)
	{
		if (!combos.count(screenpos))
		{
			apcombo* p = new apcombo(layer, screenpos);
			combos[screenpos] = p;
			p->slot = cid_to_slot(p->cid);
			p->in_set = p->slot > -1 || forcevalid;
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
			p->slot = cid_to_slot(p->cid);
			p->in_set = p->slot > -1 || forcevalid;
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
		switch (dir)
		{
			case up: --y; break;
			case down: ++y; break;
			case left: --x; break;
			case right: ++x; break;
			case l_up: --x; --y; break;
			case r_up: ++x; --y; break;
			case l_down: --x; ++ y; break;
			case r_down: ++x; ++y; break;
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
	apcombo* autopattern_container::add_relative(apcombo*& ap, int32_t xoff, int32_t yoff)
	{
		int32_t dx = std::abs(xoff);
		int32_t dy = std::abs(yoff);
		apcombo* cur = ap;
		// The greater of two diffs is more likely to go out of bounds.
		// If it does go out of bounds, the target pos will be out of bounds as well.
		if (dx >= dy)
		{
			for (int32_t x = 0; x < dx; ++x)
			{
				if (xoff < 0)
				{
					cur = add(cur, left, false, false);
					if (cur == NULL)
						return cur;
				}
				else if (xoff > 0)
				{
					cur = add(cur, right, false, false);
					if (cur == NULL)
						return cur;
				}
			}
			for (int32_t y = 0; y < dy; ++y)
			{
				if (yoff < 0)
				{
					cur = add(cur, up, false, false);
					if (cur == NULL)
						return cur;
				}
				else if (yoff > 0)
				{
					cur = add(cur, down, false, false);
					if (cur == NULL)
						return cur;
				}
			}
			return cur;
		}
		else if (dx > 0 || dy > 0)
		{
			for (int32_t y = 0; y < dy; ++y)
			{
				if (yoff < 0)
				{
					cur = add(cur, up, false, false);
					if (cur == NULL)
						return cur;
				}
				else if (yoff > 0)
				{
					cur = add(cur, down, false, false);
					if (cur == NULL)
						return cur;
				}
			}
			for (int32_t x = 0; x < dx; ++x)
			{
				if (xoff < 0)
				{
					cur = add(cur, left, false, false);
					if (cur == NULL)
						return cur;
				}
				else if (xoff > 0)
				{
					cur = add(cur, right, false, false);
					if (cur == NULL)
						return cur;
				}
			}
			return cur;
		}
		else
			return ap;
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
		if (screenboundary_x|| screenboundary_y)
		{
			if (x<basescreen_x - screenboundary_x || x > basescreen_x + 15 + screenboundary_x || y < basescreen_y - screenboundary_y || y > basescreen_y + 10 + screenboundary_y)
			{
				return true;
			}
		}
		return false;
	}

	// apcombo

	apcombo::apcombo(byte nlayer, int32_t nscreenpos) :
		screenpos(nscreenpos), screen(nscreenpos / 176), pos(nscreenpos % 176)
	{
		x = (pos % 16) + (screen % 16) * 16;
		y = (pos / 16) + (screen / 16) * 11;
		read(nlayer);
	}
	apcombo::apcombo(byte nlayer, int32_t nscreen, int32_t npos) :
		screenpos(nscreen * 176 + npos), screen(nscreen), pos(npos)
	{
		x = (pos % 16) + (screen % 16) * 16;
		y = (pos / 16) + (screen / 16) * 11;
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
		if (!(mapscr_ptr->valid & mVALID))
		{
			mapscr_ptr->valid |= mVALID;
			mapscr_ptr->color = Map.CurrScr()->color;
		}
		int32_t cset = mapscr_ptr->cset[pos];
		Map.DoSetComboCommand(drawmap, drawscreen, pos, cid, base ? CSet : cset);
	}
}
