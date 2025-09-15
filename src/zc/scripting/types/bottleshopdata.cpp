#include "zc/scripting/arrays.h"

// bottleshopdata arrays.

static ArrayRegistrar BSHOPCOMBO_registrar(BSHOPCOMBO, []{
	static ScriptingArray_ObjectMemberCArray<bottleshoptype, &bottleshoptype::comb> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-1, MAX_COUNTERS - 1>);
	return &impl;
}());

static ArrayRegistrar BSHOPCSET_registrar(BSHOPCSET, []{
	static ScriptingArray_ObjectMemberCArray<bottleshoptype, &bottleshoptype::cset> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 11>);
	return &impl;
}());

static ArrayRegistrar BSHOPFILL_registrar(BSHOPFILL, []{
	static ScriptingArray_ObjectMemberCArray<bottleshoptype, &bottleshoptype::fill> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 64>);
	return &impl;
}());

static ArrayRegistrar BSHOPPRICE_registrar(BSHOPPRICE, []{
	static ScriptingArray_ObjectMemberCArray<bottleshoptype, &bottleshoptype::price> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar BSHOPSTR_registrar(BSHOPSTR, []{
	static ScriptingArray_ObjectMemberCArray<bottleshoptype, &bottleshoptype::str> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());
