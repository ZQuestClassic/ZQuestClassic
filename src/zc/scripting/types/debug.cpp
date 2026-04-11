#include "zc/scripting/types/debug.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/zelda.h"

int32_t debug_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case DEBUGTESTING:
			ret = use_testingst_start ? 10000 : 0;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void debug_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	NOTREACHED();
}
