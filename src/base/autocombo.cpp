#include "autocombo.h"
#include "cpool.h"
#include "random.h"
#include <assert.h>

combo_auto combo_autos[MAXCOMBOPOOLS];

int16_t autocombo_entry::base_engrave_offset(byte type)
{
	switch (type)
	{
		case AUTOCOMBO_Z1: return 0;
		case AUTOCOMBO_BASIC: return 16;
		case AUTOCOMBO_FENCE: return 32;
		case AUTOCOMBO_Z4: return 64;
	}
	return 0;
}

combo_auto& combo_auto::operator=(combo_auto const& other)
{
	clear();
	for(autocombo_entry const& cp : other.combos)
	{
		add(cp.cid, cp.offset, cp.engrave_offset);
	}
	type = other.type;
	cid_display = other.cid_display;
	flags = other.flags;
	cid_erase = other.cid_erase;
	arg = other.arg;
	return *this;
}

void combo_auto::push(int32_t cid, int32_t of, int32_t eo) //add a combo with quantity
{
	if(!of) return;
	combos.emplace_back(cid,of,eo);
}
void combo_auto::add(int32_t cid, int32_t of, int32_t eo) //add a new combo entry
{
	if (eo < 0)
		eo = of + autocombo_entry::base_engrave_offset(type);
	autocombo_entry e = combos.emplace_back(cid,of,eo);
}

void combo_auto::updateValid()
{
	if (type == AUTOCOMBO_NONE)
	{
		flags &= ~ACF_VALID;
		return;
	}
	for (auto c : combos)
	{
		if (c.cid == 0)
		{
			flags &= ~ACF_VALID;
			return;
		}
		else
		{
			for (auto c2 : combos)
			{
				if (c2.offset != c.offset && c2.cid == c.cid)
				{
					flags &= ~ACF_VALID;
					return;
				}
			}
		}
	}
	flags |= ACF_VALID;
}

bool combo_auto::containsCombo(int32_t cid) const
{
	if (!valid())
		return false;

	for (auto c : combos)
	{
		if (c.cid == cid)
			return true;
	}
	return false;
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

int32_t combo_auto::convert_offsets(byte type, int16_t offset)
{
	switch (type)
	{
		case AUTOCOMBO_BASIC:
		{
			return offset;
			break;
		}
		case AUTOCOMBO_Z1:
		{
			switch (offset)
			{
				case 0:
				case 1:
				case 2:
					return offset;
				case 3:
				case 4:
				case 5:
					return offset + 1;
			}
			break;
		}
		case AUTOCOMBO_Z4:
		{
			switch (offset)
			{
				case 0:
				case 1:
				case 2:
					return offset;
				case 3:
				case 4:
				case 5:
					return offset + 1;
				case 6:
				case 7:
				case 8:
					return offset + 2;
				case 9:
				case 10:
				case 11:
					return offset + 3;
				case 12:
				case 13:
				case 14:
					return offset + 4;
				case 15:
				case 16:
				case 17:
					return offset + 5;
				case 18:
				case 19:
					return offset + 6;
				case 20:
				case 21:
					return offset + 8;
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
					return offset + 10;
			}
			break;
		}
		case AUTOCOMBO_FENCE:
		{
			switch (offset)
			{
				case 0:
				case 1:
				case 2:
					return offset;
				case 3:
				case 4:
				case 5:
					return offset + 1;
				case 6:
				case 7:
				case 8:
					return offset + 2;
				case 9:
				case 10:
					return offset + 3;
				case 11:
				case 12:
					return offset + 5;
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
					return offset + 7;
			}
			break;
		}
	}
	return offset;
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

