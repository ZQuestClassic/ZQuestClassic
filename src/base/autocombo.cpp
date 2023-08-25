#include "autocombo.h"
#include "cpool.h"
#include "random.h"
#include <assert.h>

combo_auto combo_autos[MAXCOMBOPOOLS];

combo_auto& combo_auto::operator=(combo_auto const& other)
{
	clear();
	for(autocombo_entry const& cp : other.combos)
	{
		add(cp.cid, cp.cset, cp.slot, cp.offset, cp.engrave_offset);
	}
	type = other.type;
	cid_display = other.cid_display;
	wasvalid = other.wasvalid;
	return *this;
}
void combo_auto::push(int32_t cid, int8_t cs, int32_t sl, int32_t q, int32_t eo) //add a combo with quantity
{
	if(!q) return;
	combos.emplace_back(cid,cs,sl,q,eo);
}
void combo_auto::add(int32_t cid, int8_t cs, int32_t sl, int32_t q, int32_t eo) //add a new combo entry
{
	combos.emplace_back(cid,cs,sl,q,eo);
}

void combo_auto::updateValid()
{
	if (type == AUTOCOMBO_NONE)
	{
		wasvalid = false;
		return;
	}
	for (auto c : combos)
	{
		if (c.cid == 0 && c.offset > -1)
		{
			wasvalid = false;
			return;
		}
	}
	wasvalid = true;
}

int32_t combo_auto::getDisplay() const
{
	if (cid_display)
		return cid_display;
	for (auto c : combos)
	{
		if (c.cid > 0)
			return c.cid;
	}
	return 0;
}

int32_t combo_auto::convert_offsets(int32_t entry)
{
	autocombo_entry const& e = combos.at(entry);
	zprint2("Type %d Offset %d Engrave %d\n", type, e.offset, e.engrave_offset);
	switch (type)
	{
		case AUTOCOMBO_BASIC:
		{
			return e.offset;
			break;
		}
		case AUTOCOMBO_Z1:
		{
			switch (e.offset)
			{
				case 0:
				case 1:
				case 2:
					return e.offset;
				case 3:
				case 4:
				case 5:
					return e.offset + 1;
			}
			break;
		}
		case AUTOCOMBO_Z4:
		{
			switch (e.offset)
			{
				case 0:
				case 1:
				case 2:
					return e.offset;
				case 3:
					return e.offset + 1;
				case 4:
					return e.offset + 2;
				case 5:
				case 6:
				case 7:
					return e.offset + 3;
				case 8:
				case 9:
				case 10:
					return e.offset + 4;
				case 11:
				case 12:
				case 13:
					return e.offset + 5;
				case 14:
				case 15:
				case 16:
					return e.offset + 6;
				case 17:
				case 18:
					return e.offset + 7;
				case 19:
				case 20:
					return e.offset + 9;
			}
			break;
		}
		case AUTOCOMBO_FENCE:
		{
			switch (e.offset)
			{
				case 0:
				case 1:
				case 2:
					return e.offset;
				case 3:
					return e.offset + 1;
				case 4:
					return e.offset + 2;
				case 5:
				case 6:
				case 7:
					return e.offset + 3;
				case 8:
				case 9:
					return e.offset + 4;
				case 10:
				case 11:
					return e.offset + 6;
			}
			break;
		}
	}
	return e.offset;
}

std::map<int32_t,byte> combo_auto::getMapping()
{
	std::map<int32_t,byte> ret;
	byte b = 0;
	for(auto& cmb : combos)
	{
		if(cmb.cid > -1)
			ret[cmb.cid] = b;
		//
		if(cmb.cpoolid > -1)
			for(cpool_entry const& entry : combo_pools[cmb.cpoolid].combos)
				if(entry.cid > -1)
					ret[entry.cid] = b;
		//
		++b;
	}
	return ret;
}

