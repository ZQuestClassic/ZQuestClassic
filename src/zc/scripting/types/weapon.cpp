#include "zc/scripting/arrays.h"

// weapon arrays.

struct WeaponArrayRegistrar
{
	WeaponArrayRegistrar(int zasm_var_1, int zasm_var_2, IScriptingArray* arrayImpl)
	{
		zasm_array_register(zasm_var_1, arrayImpl);
		zasm_array_register(zasm_var_2, arrayImpl);
	}
};

static WeaponArrayRegistrar WPNFLAGS_registrar(LWPNFLAGS, EWPNFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<weapon, &weapon::misc_wflags, WFLAG_MAX> impl;
	impl.setMul10000(true);
	return &impl;
}());

static WeaponArrayRegistrar WPNMOVEFLAGS_registrar(LWPNMOVEFLAGS, EWPNMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<weapon, &weapon::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static WeaponArrayRegistrar WPNBURNLIGHTRADIUS_registrar(LWPNBURNLIGHTRADIUS, EWPNBURNLIGHTRADIUS, []{
	static ScriptingArray_ObjectMemberCArray<weapon, &weapon::light_rads> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static WeaponArrayRegistrar WPNINITD_registrar(LWPNINITD, EWPNINITD, []{
	static ScriptingArray_ObjectMemberCArray<weapon, &weapon::initD> impl;
	impl.setMul10000(false);
	impl.compatBoundIndex();
	return &impl;
}());

static WeaponArrayRegistrar WPNMISCD_registrar(LWPNMISCD, EWPNMISCD, []{
	static ScriptingArray_ObjectMemberCArray<weapon, &weapon::miscellaneous> impl;
	impl.setMul10000(false);
	impl.compatBoundIndex();
	return &impl;
}());

static WeaponArrayRegistrar WPNSPRITES_registrar(LWPNSPRITES, EWPNSPRITES, []{
	static ScriptingArray_ObjectMemberCArray<weapon, &weapon::misc_wsprites> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());
