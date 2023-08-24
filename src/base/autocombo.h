#ifndef _AUTOCOMBO_H_
#define _AUTOCOMBO_H_

#include <vector>
#include "base/ints.h"
#include "base/general.h"

enum { AUTOCOMBO_NONE, AUTOCOMBO_Z1, AUTOCOMBO_BASIC, AUTOCOMBO_FENCE, AUTOCOMBO_Z4 };

struct autocombo_entry
{
	int32_t cid = -1;
	int8_t cset = -1;
	int32_t slot = 0;
	int32_t offset = -1;
	int32_t engrave_offset = 0;
	void clear()
	{
		*this = autocombo_entry();
	}
	bool valid() const
	{
		return offset > -1 && unsigned(cid) < MAXCOMBOS;
	}
	autocombo_entry() = default;
	autocombo_entry(int32_t data, int8_t cs, int32_t sl, int32_t q, int32_t eo) :
		cid(data), cset(cs), slot(sl), offset(q), engrave_offset(eo)
	{}
};

struct combo_auto
{
	std::vector<autocombo_entry> combos;
	
	combo_auto()
	{}
	combo_auto& operator=(combo_auto const& other);
	void push(int32_t cid, int8_t cs, int32_t sl, int32_t q, int32_t eo); //add a quantity of a combo entry
	void add(int32_t cid, int8_t cs, int32_t sl, int32_t q, int32_t eo); //add a new combo entry
	void clear()
	{
		combos.clear();
		combos.shrink_to_fit();
	}
	bool valid() const
	{
		return wasvalid;
	}
	void updateValid();

	int32_t getType()
	{
		return type;
	}
	void setType(int32_t newtype)
	{
		type = newtype;
	}

	int32_t getDisplay() const;
	int32_t getIconDisplay()
	{
		return cid_display;
	}
	void setDisplay(int32_t newdisplay)
	{
		cid_display = newdisplay;
	}

	int32_t convert_offsets(int32_t entry);

private:
	int32_t type = AUTOCOMBO_NONE;
	int32_t cid_display = 0;
	bool wasvalid = false;
};

extern combo_auto combo_autos[MAXCOMBOPOOLS];

#endif

