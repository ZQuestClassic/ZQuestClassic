#include "zc/scripting/types/viewport.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/ffscript.h"
#include "zc/maps.h"
#include "zc/scripting/common.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t viewport_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case VIEWPORT_HEIGHT:
		{
			ret = viewport.h * 10000;
		}
		break;
		case VIEWPORT_MODE:
		{
			ret = (int)viewport_mode;
		}
		break;
		case VIEWPORT_TARGET:
		{
			ret = get_viewport_sprite()->uid;
		}
		break;
		case VIEWPORT_WIDTH:
		{
			ret = viewport.w * 10000;
		}
		break;
		case VIEWPORT_X:
		{
			ret = viewport.x * 10000;
		}
		break;
		case VIEWPORT_Y:
		{
			ret = viewport.y * 10000;
		}
		break;

		default:
			NOTREACHED();
	}

	return ret;
}

void viewport_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case VIEWPORT_HEIGHT:
		{
			int val = value / 10000;
			if (BC::checkBounds(val, 0, 232) != SH::_NoError)
				break;

			viewport.h = val;
		}
		break;
		case VIEWPORT_MODE:
		{
			int val = value;
			if (BC::checkBounds(val, (int)ViewportMode::First, (int)ViewportMode::Last) != SH::_NoError)
			{
				break;
			}

			viewport_mode = (ViewportMode)val;
		}
		break;
		case VIEWPORT_TARGET:
		{
			if (auto s = ResolveBaseSprite(value))
			{
				set_viewport_sprite(s);
				update_viewport();
			}
		}
		break;
		case VIEWPORT_WIDTH:
		{
			int val = value / 10000;
			if (BC::checkBounds(val, 0, 256) != SH::_NoError)
				break;

			viewport.w = val;
		}
		break;
		case VIEWPORT_X:
		{
			viewport.x = value / 10000;
		}
		break;
		case VIEWPORT_Y:
		{
			viewport.y = value / 10000;
		}
		break;

		default:
			NOTREACHED();
	}
}
