#include "zc/scripting/types/bottledata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/misctypes.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

bottletype* checkBottleData(int32_t ref, bool skipError)
{
	if(ref > 0 && ref <= 64)
	{
		return &QMisc.bottle_types[ref-1];
	}
	if(skipError) return NULL;

	scripting_log_error_with_context("Invalid {} using UID = {}", "bottledata", ref);
	return NULL;
}

int32_t bottledata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case BOTTLENEXT:
		{
			if(bottletype* ptr = checkBottleData(GET_REF(bottletyperef)))
			{
				ret = 10000L * ptr->next_type;
			}
			else ret = -10000L;
		}
		break;

		default:
			NOTREACHED();
	}

	return ret;
}

void bottledata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case BOTTLENEXT:
		{
			if(bottletype* ptr = checkBottleData(GET_REF(bottletyperef)))
			{
				ptr->next_type = vbound(value/10000, 0, 64);
			}
		}
		break;

		default:
			NOTREACHED();
	}
}

// bottledata arrays.

static ArrayRegistrar BOTTLEAMOUNT_registrar(BOTTLEAMOUNT, []{
	static ScriptingArray_ObjectMemberCArray<bottletype, &bottletype::amount> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar BOTTLECOUNTER_registrar(BOTTLECOUNTER, []{
	static ScriptingArray_ObjectMemberCArray<bottletype, &bottletype::counter> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-1, MAX_COUNTERS - 1>);
	return &impl;
}());

static ArrayRegistrar BOTTLEPERCENT_registrar(BOTTLEPERCENT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<bottletype, &bottletype::flags, 3> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar BOTTLEFLAGS_registrar(BOTTLEFLAGS, []{
	static ScriptingArray_ObjectComputed<bottletype, bool> impl(
		[](bottletype*){
			return 5;
		},
		[](bottletype* btl, int index) -> bool {
			int flag = 0;
			switch (index)
			{
				case 0: flag = BTFLAG_AUTOONDEATH; break;
				case 1: flag = BTFLAG_ALLOWIFFULL; break;
				case 2: flag = BTFLAG_CURESWJINX; break;
				case 3: flag = BTFLAG_CUREITJINX; break;
				case 4: flag = BTFLAG_CURESHJINX; break;
			}
			return btl->flags & flag;
		},
		[](bottletype* btl, int index, bool value){
			int flag = 0;
			switch (index)
			{
				case 0: flag = BTFLAG_AUTOONDEATH; break;
				case 1: flag = BTFLAG_ALLOWIFFULL; break;
				case 2: flag = BTFLAG_CURESWJINX; break;
				case 3: flag = BTFLAG_CUREITJINX; break;
				case 4: flag = BTFLAG_CURESHJINX; break;
			}
			SETFLAG(btl->flags, flag, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
