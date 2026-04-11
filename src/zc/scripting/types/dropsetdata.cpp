#include "zc/scripting/types/dropsetdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include <cstdint>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t dropsetdata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case DROPSETCHOOSE:
		{
			if(ri->dropsetdataref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetdataref);
				ret = -10000;
				break;
			}
			ret = select_dropitem(GET_REF(dropsetdataref)) * 10000;
			break;
		}
		case DROPSETNULLCHANCE:
		{
			if(ri->dropsetdataref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetdataref);
				ret = -10000;
				break;
			}
			ret = item_drop_sets[GET_REF(dropsetdataref)].chance[0] * 10000;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void dropsetdata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case DROPSETNULLCHANCE:
		{
			if(ri->dropsetdataref > MAXITEMDROPSETS)
			{
				Z_scripterrlog("Invalid dropset pointer %d\n", ri->dropsetdataref);
				break;
			}
			item_drop_sets[GET_REF(dropsetdataref)].chance[0] = vbound((value / 10000),0,32767);
			break;
		}

		default:
			NOTREACHED();
	}
}

// dropsetdata arrays.

static ArrayRegistrar DROPSETITEMS_registrar(DROPSETITEMS, []{
	static ScriptingArray_ObjectMemberCArray<item_drop_object, &item_drop_object::item> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, MAXITEMS-1>);
	return &impl;
}());

static ArrayRegistrar DROPSETCHANCES_registrar(DROPSETCHANCES, []{
	static ScriptingArray_ObjectComputed<item_drop_object, int> impl(
		[](item_drop_object*){
			return 10;
		},
		[](item_drop_object* drop, int index) -> int {
			return drop->chance[index + 1]; // +1 is because '0' is 'nothing''s chance
		},
		[](item_drop_object* drop, int index, int value){
			drop->chance[index + 1] = value; // +1 is because '0' is 'nothing''s chance
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 32767>);
	return &impl;
}());
