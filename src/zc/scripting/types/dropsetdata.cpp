#include "zc/scripting/arrays.h"

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
		[](item_drop_object* drop){
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
