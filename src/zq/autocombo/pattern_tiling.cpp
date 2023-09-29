#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_tiling.h"
#include <base/zsys.h>

namespace AutoPattern
{

	bool autopattern_tiling::execute(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		int32_t x = (size.first + ap->x - offsets.first) % size.first;
		int32_t y = (size.second + ap->y - offsets.second) % size.second;
		ap->set_cid(slot_to_cid_pair(x + y * size.first));
		if (ap->cid == 0)
			ap->changed = false;

		apply_changes();
		return true;
	}
	bool autopattern_tiling::erase(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return false;
		ap->cid = erase_cid;
		apply_changes();
		return true;
	}
	int32_t autopattern_tiling::get_floating_cid(int32_t exscreen, int32_t expos)
	{
		apcombo* ap = add(exscreen, expos, true);
		if (!ap)
			return 0;
		int32_t x = (size.first + ap->x - offsets.first) % size.first;
		int32_t y = (size.second + ap->y - offsets.second) % size.second;
		ap->set_cid(slot_to_cid_pair(x + y * size.first));
		if (ap->cid == 0)
			return 0;
		return ap->cid;
	}

	uint32_t autopattern_tiling::slot_to_flags(int32_t slot)
	{
		return 0;
	}
	int32_t autopattern_tiling::flags_to_slot(uint32_t flags)
	{
		return 0;
	}

}