#include "zc/scripting/arrays.h"

// ffc arrays.

static ArrayRegistrar FFFLAGSD_registrar(FFFLAGSD, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ffcdata, &ffcdata::flags, 32> impl;
	impl.setSideEffect([](auto ffc, int index, int value){
		auto flag = F(index);
		if (flag == ffc_solid || flag == ffc_changer)
			ffc->updateSolid();
	});
	return &impl;
}());

static ArrayRegistrar FFINITDD_registrar(FFINITDD, []{
	static ScriptingArray_ObjectMemberCArray<ffcdata, &ffcdata::initd> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.compatBoundSetterIndex();
	return &impl;
}());

static ArrayRegistrar FFMISCD_registrar(FFMISCD, []{
	static ScriptingArray_ObjectMemberCArray<ffcdata, &ffcdata::miscellaneous> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.compatBoundSetterIndex();
	return &impl;
}());
