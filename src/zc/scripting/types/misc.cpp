#include "zc/scripting/types/misc.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "gamedata.h"
#include "zc/scripting/common.h"
#include "zc/zscriptversion.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

template <typename T, size_t N>
static int read_array(const T(&arr)[N], int index)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return 0;

	return arr[index];
}

template <typename T, size_t N>
static bool write_array(T(&arr)[N], int index, T value)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return false;

	arr[index] = value;
	return true;
}

int32_t misc_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case CLASS_THISKEY: ret = ri->thiskey; break;
		case CLASS_THISKEY2: ret = ri->thiskey2; break;
		case GDD: // Unused, remove?
			ret = read_array(game->global_d, GET_D(rINDEX) / 10000);
			break;
		case PC: ret = ri->pc; break;
		case SP:
			ret = ri->sp * 10000;
			break;
		case SP2:
			ret = ri->sp;
			break;
		case SWITCHKEY:
			ret = ri->switchkey;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void misc_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case CLASS_THISKEY: ri->thiskey = value; break;
		case CLASS_THISKEY2: ri->thiskey2 = value; break;
		case GDD:
			write_array(game->global_d, GET_D(rINDEX) / 10000, value);
			break;
		case PC:
			ri->pc = value;
			break;
		case SP:
			ri->sp = value / 10000;
			break;
		case SP2:
			ri->sp = value;
			break;
		case SWITCHKEY:
			ri->switchkey = value;
			break;

		default:
			NOTREACHED();
	}
}
