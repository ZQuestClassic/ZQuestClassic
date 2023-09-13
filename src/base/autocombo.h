#ifndef _AUTOCOMBO_H_
#define _AUTOCOMBO_H_

#include <vector>
#include <map>
#include "base/ints.h"
#include "base/general.h"

enum { ACT_NORMAL, ACT_CPOOL };
enum { AUTOCOMBO_NONE, AUTOCOMBO_Z1, AUTOCOMBO_BASIC, AUTOCOMBO_FENCE, AUTOCOMBO_Z4, AUTOCOMBO_RELATIONAL, AUTOCOMBO_DGNCARVE, AUTOCOMBO_DOR, AUTOCOMBO_TILING, AUTOCOMBO_REPLACE };

enum
{
	ACF_VALID =        0x1,
	ACF_CROSSSCREENS = 0x2,
	ACF_FLIP         = 0x4,
	ACF_LEGACY       = 0x4 // used by Relational and Dungeon Carving, neither of which use flip
};

struct autocombo_entry
{
	int32_t cid = -1;
	byte ctype = 0; // currently unused, will be used for combo pools / future expansion
	int16_t offset = -1;
	int16_t engrave_offset = 0;
	void clear()
	{
		*this = autocombo_entry();
	}
	bool valid() const
	{
		return offset > -1 && unsigned(cid) < MAXCOMBOS;
	}

	autocombo_entry() = default;
	autocombo_entry(int32_t data, byte ct, int32_t of, int32_t eo) :
		cid(data), ctype(ct), offset(of), engrave_offset(eo)
	{}

	static int16_t base_engrave_offset(byte type);
};

struct combo_auto
{
	std::vector<autocombo_entry> combos;
	byte flags = 0;
	
	combo_auto()
	{}
	combo_auto& operator=(combo_auto const& other);
	void add(int32_t cid, byte ct, int32_t of, int32_t eo); //add a new combo entry
	void clear(bool clear_all = false)
	{
		if (clear_all)
		{
			type = AUTOCOMBO_NONE;
			cid_display = 0;
			flags = 0;
			cid_erase = 0;
		}
		combos.clear();
		combos.shrink_to_fit();
	}
	bool valid() const
	{
		return flags & ACF_VALID;
	}
	void updateValid();

	bool containsCombo(int32_t cid, bool requirevalid = true) const;
	bool isIgnoredCombo(int32_t cid) const;

	byte getFlags() const
	{
		return flags;
	}
	void setFlags(byte newflags)
	{
		flags = newflags;
	}
	int32_t getType() const
	{
		return type;
	}
	void setType(int32_t newtype)
	{
		type = newtype;
	}

	byte getArg() const
	{
		return arg;
	}
	void setArg(byte newarg)
	{
		arg = newarg;
	}

	std::pair<byte,byte> getOffsets()
	{
		return offsets;
	}
	void setOffsets(byte xo, byte yo)
	{
		if (xo < 255)
			offsets.first = xo;
		if (yo < 255)
			offsets.second = yo;
	}

	int32_t getDisplay() const;
	int32_t getIconDisplay() const
	{
		return cid_display;
	}
	void setDisplay(int32_t newdisplay)
	{
		cid_display = newdisplay;
	}
	int32_t getEraseCombo() const
	{
		return cid_erase;
	}
	void setEraseCombo(int32_t newerase)
	{
		cid_erase = newerase;
	}

	static int32_t legacy_offsets(byte type, int16_t offset);
	static int32_t convert_offsets(byte type, int16_t offset);
	static bool ignore_fill(byte type, int32_t slot);

	std::map<int32_t,byte> getMapping();

private:
	byte type = AUTOCOMBO_NONE;
	byte arg = 0;
	int32_t cid_display = 0;
	int32_t cid_erase = 0;
	std::pair<byte, byte> offsets;
};

extern combo_auto combo_autos[MAXCOMBOPOOLS];

#endif

