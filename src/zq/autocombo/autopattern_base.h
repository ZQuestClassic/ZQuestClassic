#ifndef _AUTOPATTERN_BASE_H_
#define _AUTOPATTERN_BASE_H_

#include <functional>
#include "base/autocombo.h"
#include "base/combo.h"

namespace AutoPattern
{

	enum
	{
		U = 0x01,
		D = 0x02,
		L = 0x04,
		R = 0x08,
		UL = 0x10,
		UR = 0x20,
		DL = 0x40,
		DR = 0x80
	};

	class apcombo;

	class autopattern_container
	{
	public:
		autopattern_container(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource);
		~autopattern_container();
		virtual bool execute(int32_t s, int32_t p) = 0;
		virtual bool erase(int32_t s, int32_t p) = 0;
		virtual int32_t get_floating_cid(int32_t s, int32_t p) = 0;
		virtual uint32_t slot_to_flags(int32_t slot) = 0;
		virtual int32_t flags_to_slot(uint32_t flags) = 0;
		virtual void init_pattern();
		void apply_changes();
		int32_t cid_to_slot(int32_t cid);
		int32_t slot_to_cid(int32_t slot);
		std::pair<int32_t, int32_t> slot_to_cid_pair(int32_t slot);
		apcombo* add(int32_t sp, bool forcevalid = false, bool andgenerate = true);
		apcombo* add(int32_t s, int32_t p, bool forcevalid = false, bool andgenerate = true);
		apcombo* add(apcombo* &ap, int32_t dir, bool forcevalid = false, bool andgenerate = true);
		apcombo* add_relative(apcombo*& ap, int32_t xoff, int32_t yoff);
		void remove(apcombo* ptr);
		void init_connections(apcombo* ap, bool andgenerate = true);
		bool offscreen(int32_t x, int32_t y);

	protected:
		combo_auto* source;
		int32_t erase_cid;
		int32_t type;
		int32_t layer;
		int32_t basescreen;
		int32_t basepos;
		int32_t basescreen_x, basescreen_y;
		int32_t base_x, base_y;
		byte screenboundary_x, screenboundary_y;
		// indexes apcombo by screen pos, for making sure an object isn't created for the same tile twice
		std::map<uint16_t, apcombo*> combos;
		std::map<int32_t, int32_t> pattern_cids; // for converting slots to cids
		std::map<int32_t, int32_t> pattern_slots; // for converting cids to slots
		bool nocrossedge;
		bool connectedge;
	};

	class apcombo
	{
	public:
		apcombo(byte layer , int32_t screenpos);
		apcombo(byte layer, int32_t screen, int32_t pos);

		void read(byte layer);
		byte read_solid(byte layer);
		void write(byte layer, bool base = false);
		void set_cid(std::pair<int32_t, int32_t> newcid)
		{
			if (newcid.first != cid)
				changed = true;
			cid = newcid.first;
			slot = newcid.second;
		}

		byte screen;
		byte pos;
		byte x, y;
		uint16_t screenpos;
		uint16_t cid = 0;
		int32_t slot = 0;
		bool in_set = false;
		bool changed = false;
		bool force_cset = false;
		apcombo* adj[8] = { nullptr };
		uint32_t connflags = 0;
	};

}
#endif