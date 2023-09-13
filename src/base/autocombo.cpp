#include "autocombo.h"
#include "cpool.h"
#include "random.h"
#include <assert.h>
#include <zq/zquest.h>

combo_auto combo_autos[MAXCOMBOPOOLS];

int16_t autocombo_entry::base_engrave_offset(byte type)
{
	switch (type)
	{
		case AUTOCOMBO_Z1: return 0;
		case AUTOCOMBO_BASIC: return 16;
		case AUTOCOMBO_FENCE: return 32;
		case AUTOCOMBO_Z4: return 64;
		case AUTOCOMBO_RELATIONAL: return 96;
		case AUTOCOMBO_DGNCARVE: return 144;
		case AUTOCOMBO_DOR: return 240;
	}
	return 0;
}

combo_auto& combo_auto::operator=(combo_auto const& other)
{
	clear();
	for(autocombo_entry const& cp : other.combos)
	{
		add(cp.cid, cp.ctype, cp.offset, cp.engrave_offset);
	}
	type = other.type;
	cid_display = other.cid_display;
	flags = other.flags;
	cid_erase = other.cid_erase;
	arg = other.arg;
	return *this;
}

void combo_auto::add(int32_t cid, byte ct, int32_t of, int32_t eo) //add a new combo entry
{
	if (eo < 0)
		eo = of + autocombo_entry::base_engrave_offset(type);
	autocombo_entry e = combos.emplace_back(cid,ct,of,eo);
}

void combo_auto::updateValid()
{
	if (type == AUTOCOMBO_NONE)
	{
		flags &= ~ACF_VALID;
		return;
	}
	if (type == AUTOCOMBO_TILING)
	{
		flags |= ACF_VALID;
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

bool combo_auto::containsCombo(int32_t cid, bool requirevalid) const
{
	if (!valid() && requirevalid)
		return false;

	for (auto c : combos)
	{
		if (c.cid == cid && cid != 0)
			return true;
	}
	return false;
}

// used for ignoring certain combos with the fill tool
bool combo_auto::isIgnoredCombo(int32_t cid) const
{
	switch (type)
	{
		case AUTOCOMBO_DGNCARVE:
		case AUTOCOMBO_DOR:
			break;
		default:
			return false;
	}
	int32_t slot = 0;
	for (auto c : combos)
	{
		if (c.cid == cid)
		{
			if (ignore_fill(type, slot))
				return true;
		}
		++slot;
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

int32_t combo_auto::legacy_offsets(byte type, int16_t offset)
{
	switch (type)
	{
		case AUTOCOMBO_RELATIONAL:
			switch (offset)
			{
				case 0:
					return 31;
				case 1:
					return 20;
				case 2:
					return 24;
				case 3:
					return 33;
				case 4:
					return 26;
				case 5:
					return 0;
				case 6:
					return 16;
				case 7:
					return 30;
				case 8:
					return 42;
				case 9:
					return 34;
				case 10:
					return 38;
				case 11:
					return 44;
				case 12:
					return 45;
				case 13:
					return 40;
				case 14:
					return 41;
				case 15:
					return 46;
				case 16:
					return 8;
				case 17:
					return 4;
				case 18:
					return 32;
				case 19:
					return 25;
				case 20:
					return 1;
				case 21:
					return 2;
				case 22:
					return 43;
				case 23:
					return 39;
				case 24:
					return 12;
				case 25:
					return 6;
				case 26:
					return 23;
				case 27:
					return 19;
				case 28:
					return 9;
				case 29:
					return 3;
				case 30:
					return 29;
				case 31:
					return 37;
				case 32:
					return 21;
				case 33:
					return 22;
				case 34:
					return 27;
				case 35:
					return 17;
				case 36:
					return 36;
				case 37:
					return 35;
				case 38:
					return 28;
				case 39:
					return 18;
				case 40:
					return 7;
				case 41:
					return 11;
				case 42:
					return 5;
				case 43:
					return 10;
				case 44:
					return 14;
				case 45:
					return 13;
				case 46:
					return 15;
			}
			break;
		case AUTOCOMBO_DGNCARVE:
			switch (offset)
			{
				case 0:
					return 0;
				case 1:
					return 46;
				case 2:
					return 95;
				case 3:
					return 94;
				case 4:
					return 8;
				case 5:
					return 4;
				case 6:
					return 56;
				case 7:
					return 52;
				case 8:
					return 1;
				case 9:
					return 2;
				case 10:
					return 49;
				case 11:
					return 50;
				case 12:
					return 31;
				case 13:
					return 24;
				case 14:
					return 79;
				case 15:
					return 72;
				case 16:
					return 42;
				case 17:
					return 38;
				case 18:
					return 90;
				case 19:
					return 86;
				case 20:
					return 68;
				case 21:
					return 34;
				case 22:
					return 74;
				case 23:
					return 26;
				case 24:
					return 20;
				case 25:
					return 82;
				case 26:
					return 16;
				case 27:
					return 64;
				case 28:
					return 33;
				case 29:
					return 41;
				case 30:
					return 81;
				case 31:
					return 89;
				case 32:
					return 45;
				case 33:
					return 44;
				case 34:
					return 93;
				case 35:
					return 92;
				case 36:
					return 30;
				case 37:
					return 40;
				case 38:
					return 78;
				case 39:
					return 88;
				case 40:
					return 32;
				case 41:
					return 25;
				case 42:
					return 80;
				case 43:
					return 73;
				case 44:
					return 43;
				case 45:
					return 39;
				case 46:
					return 91;
				case 47:
					return 87;
				case 48:
					return 23;
				case 49:
					return 19;
				case 50:
					return 71;
				case 51:
					return 67;
				case 52:
					return 29;
				case 53:
					return 37;
				case 54:
					return 77;
				case 55:
					return 85;
				case 56:
					return 12;
				case 57:
					return 6;
				case 58:
					return 60;
				case 59:
					return 54;
				case 60:
					return 9;
				case 61:
					return 3;
				case 62:
					return 57;
				case 63:
					return 51;
				case 64:
					return 7;
				case 65:
					return 11;
				case 66:
					return 55;
				case 67:
					return 59;
				case 68:
					return 14;
				case 69:
					return 13;
				case 70:
					return 62;
				case 71:
					return 61;
				case 72:
					return 27;
				case 73:
					return 17;
				case 74:
					return 75;
				case 75:
					return 65;
				case 76:
					return 28;
				case 77:
					return 18;
				case 78:
					return 76;
				case 79:
					return 66;
				case 80:
					return 21;
				case 81:
					return 22;
				case 82:
					return 69;
				case 83:
					return 70;
				case 84:
					return 36;
				case 85:
					return 35;
				case 86:
					return 84;
				case 87:
					return 83;
				case 88:
					return 10;
				case 89:
					return 5;
				case 90:
					return 58;
				case 91:
					return 53;
				case 92:
					return 15;
				case 93:
					return 63;
				default:
					return 0;
			}
			break;
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
		case AUTOCOMBO_DGNCARVE:
		{
			switch (offset)
			{
				case 83:
					return offset + 1;
				default:
					return offset;
			}
			break;
		}
		case AUTOCOMBO_DOR:
		{
			switch (offset)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
					return offset;
				case 27:
				case 28:
				case 29:
					return offset + 1;
				case 30:
				case 31:
				case 32:
					return offset + 2;
				case 33:
				case 34:
				case 35:
					return offset + 3;
				case 36:
				case 37:
				case 38:
					return offset + 4;
				case 39:
				case 40:
				case 41:
					return offset + 5;
				case 42:
				case 43:
				case 44:
					return offset + 6;
				case 45:
				case 46:
				case 47:
					return offset + 7;
				default:
					return offset + 8;
			}
			break;
		}
	}
	return offset;
}

bool combo_auto::ignore_fill(byte type, int32_t slot)
{
	switch (type)
	{
		case AUTOCOMBO_DGNCARVE:
			switch (slot)
			{
				case 20:
				case 22:
				case 25:
				case 27:
				case 93:
					return true;
				case 21:
				case 23:
				case 24:
				case 26:
					return false;
			}
			if (slot % 4 > 1)
				return true;
			break;
		case AUTOCOMBO_DOR:
			if (slot == 34)
				return true;
			if (slot >= 24 && slot <= 38)
				return false;
			else
				return true;
			break;
	}
	return false;
}

std::map<int32_t,byte> combo_auto::getMapping()
{
	std::map<int32_t,byte> ret;
	byte b = 0;
	for(auto& cmb : combos)
	{
		switch (cmb.ctype)
		{
			case ACT_NORMAL:
				ret[cmb.cid] = b;
				break;
			case ACT_CPOOL:
				for (cpool_entry const& entry : combo_pools[cmb.cid].combos)
					if (entry.cid > -1)
						ret[entry.cid] = b;
		}
		++b;
	}
	return ret;
}

