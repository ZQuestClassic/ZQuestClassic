#include "zc/scripting/types/global.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/ffscript.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t global_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case DISTANCE: 
		{
			double x1 = double(GET_D(rSFTEMP) / 10000.0);
			double y1 = double(GET_D(rINDEX) / 10000.0);
			double x2 = double(GET_D(rINDEX2) / 10000.0);
			double y2 = double(GET_D(rEXP1) / 10000.0);



			int32_t result = FFCore.Distance(x1, y1, x2, y2);
			ret = (result);

			break;
		}
		case DISTANCESCALE: 
		{
			double x1 = (double)(GET_D(rSFTEMP) / 10000.0);
			double y1 = (double)(GET_D(rINDEX) / 10000.0);
			double x2 = (double)(GET_D(rINDEX2) / 10000.0);
			double y2 = (double)(GET_D(rEXP1) / 10000.0);

			int32_t scale = (GET_D(rWHAT_NO_7)/10000);

			if ( !scale ) scale = 10000;
			int32_t result = FFCore.Distance(x1, y1, x2, y2, scale);
			ret = (result);

			break;
		}
		case LONGDISTANCE: 
		{
			double x1 = double(GET_D(rSFTEMP));
			double y1 = double(GET_D(rINDEX));
			double x2 = double(GET_D(rINDEX2));
			double y2 = double(GET_D(rEXP1));



			int32_t result = FFCore.LongDistance(x1, y1, x2, y2);
			ret = (result);

			break;
		}
		case LONGDISTANCESCALE: 
		{
			double x1 = (double)(GET_D(rSFTEMP));
			double y1 = (double)(GET_D(rINDEX));
			double x2 = (double)(GET_D(rINDEX2));
			double y2 = (double)(GET_D(rEXP1));

			int32_t scale = (GET_D(rWHAT_NO_7));

			if ( !scale ) scale = 1;
			int32_t result = FFCore.LongDistance(x1, y1, x2, y2, scale);
			ret = (result);

			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void global_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	NOTREACHED();
}
