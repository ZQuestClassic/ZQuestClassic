#include "zc/scripting/types/region.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/maps.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t region_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case REGION_HEIGHT:
		{
			ret = world_h * 10000;
		}
		break;
		case REGION_ID:
		{
			ret = get_current_region_id() * 10000;
		}
		break;
		case REGION_ORIGIN_SCREEN:
		{
			ret = cur_screen * 10000;
		}
		break;
		case REGION_SCREEN_HEIGHT:
		{
			ret = cur_region.screen_height * 10000;
		}
		break;
		case REGION_SCREEN_WIDTH:
		{
			ret = cur_region.screen_width * 10000;
		}
		break;
		case REGION_WIDTH:
		{
			ret = world_w * 10000;
		}
		break;

		default:
			NOTREACHED();
	}

	return ret;
}

void region_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	NOTREACHED();
}
