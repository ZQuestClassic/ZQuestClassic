#include "zc/scripting/arrays.h"

// bottledata arrays.

static ArrayRegistrar BOTTLEAMOUNT_registrar(BOTTLEAMOUNT, []{
	static ScriptingArray_ObjectMemberCArray<bottletype, &bottletype::amount> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar BOTTLECOUNTER_registrar(BOTTLECOUNTER, []{
	static ScriptingArray_ObjectMemberCArray<bottletype, &bottletype::counter> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-1, MAX_COUNTERS - 1>);
	return &impl;
}());

static ArrayRegistrar BOTTLEPERCENT_registrar(BOTTLEPERCENT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<bottletype, &bottletype::flags, 3> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar BOTTLEFLAGS_registrar(BOTTLEFLAGS, []{
	static ScriptingArray_ObjectComputed<bottletype, bool> impl(
		[](bottletype* btl){
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());
