#include "zc/scripting/arrays.h"

// messagedata arrays.

static ArrayRegistrar MESSAGEDATAMARGINS_registrar(MESSAGEDATAMARGINS, []{
	static ScriptingArray_ObjectMemberCArray<MsgStr, &MsgStr::margins> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MESSAGEDATAFLAGSARR_registrar(MESSAGEDATAFLAGSARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<MsgStr, &MsgStr::stringflags, 7> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MESSAGEDATASEGMENTS_registrar(MESSAGEDATASEGMENTS, []{
	static ScriptingArray_ObjectComputed<MsgStr, int32_t> impl(
		[](MsgStr* msg){ return msg->segmentsAsIntArray().size(); },
		[](MsgStr* msg, int index){ return msg->segmentsAsIntArray()[index]; },
		[](MsgStr* msg, int index, int value){}
	);
	impl.setMul10000(true);
	impl.setReadOnly();
	return &impl;
}());
