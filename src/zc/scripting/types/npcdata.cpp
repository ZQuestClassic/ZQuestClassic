#include "zc/scripting/arrays.h"

// npcdata arrays.

static ArrayRegistrar NPCDATAATTRIBUTE_registrar(NPCDATAATTRIBUTE, []{
	static ScriptingArray_ObjectMemberCArray<guydata, &guydata::attributes> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATADEFENSE_registrar(NPCDATADEFENSE, []{
	static ScriptingArray_ObjectMemberCArray<guydata, &guydata::defense> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAINITD_registrar(NPCDATAINITD, []{
	static ScriptingArray_ObjectMemberCArray<guydata, &guydata::initD> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAWEAPONINITD_registrar(NPCDATAWEAPONINITD, []{
	static ScriptingArray_ObjectSubMemberCArray<guydata, &guydata::weap_data, &weapon_data::initd> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAFLAG_registrar(NPCDATAFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::flags, MAX_NPC_FLAGS> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATABEHAVIOUR_registrar(NPCDATABEHAVIOUR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::editorflags, 16> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATAMOVEFLAGS_registrar(NPCDATAMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::moveflags, 16> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATAWMOVEFLAGS_registrar(NPCDATAWMOVEFLAGS, []{
	static ScriptingArray_ObjectSubMemberBitwiseFlags<guydata, &guydata::weap_data, &weapon_data::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATASHIELD_registrar(NPCDATASHIELD, []{
	static ScriptingArray_ObjectComputed<guydata, bool> impl(
		[](guydata* guy){ return 5; },
		[](guydata* guy, int index) -> bool {
			switch (index)
			{
				case 0: return guy->flags & guy_shield_front;
				case 1: return guy->flags & guy_shield_left;
				case 2: return guy->flags & guy_shield_right;
				case 3: return guy->flags & guy_shield_back;
				case 4: return guy->flags & guy_bkshield;
				default: NOTREACHED();
			}
		},
		[](guydata* guy, int index, bool value){
			switch (index)
			{
				case 0: SETFLAG(guy->flags, guy_shield_front, value); break;
				case 1: SETFLAG(guy->flags, guy_shield_left, value); break;
				case 2: SETFLAG(guy->flags, guy_shield_right, value); break;
				case 3: SETFLAG(guy->flags, guy_shield_back, value); break;
				case 4: SETFLAG(guy->flags, guy_bkshield, value); break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());
