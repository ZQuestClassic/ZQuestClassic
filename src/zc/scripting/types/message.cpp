#include "zc/scripting/arrays.h"

// messagedata arrays.

static ArrayRegistrar MESSAGEDATAMARGINS_registrar(MESSAGEDATAMARGINS, []{
	static ScriptingArray_ObjectMemberCArray<MsgStr, &MsgStr::margins> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MESSAGEDATAFLAGSARR_registrar(MESSAGEDATAFLAGSARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<MsgStr, &MsgStr::stringflags, 7> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());
