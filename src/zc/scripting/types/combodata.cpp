#include "base/qrs.h"
#include "zc/guys.h"
#include "zc/scripting/arrays.h"

// combodata arrays.

static ArrayRegistrar COMBODATTRIBUTES_registrar(COMBODATTRIBUTES, []{
	static ScriptingArray_ObjectMemberCArray<newcombo, &newcombo::attributes> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar COMBODATTRISHORTS_registrar(COMBODATTRISHORTS, []{
	static ScriptingArray_ObjectMemberCArray<newcombo, &newcombo::attrishorts> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-32768, 32767>);
	return &impl;
}());

static ArrayRegistrar COMBODATTRIBYTES_registrar(COMBODATTRIBYTES, []{
	static ScriptingArray_ObjectMemberCArray<newcombo, &newcombo::attribytes> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);

	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar COMBODGENFLAGARR_registrar(COMBODGENFLAGARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<newcombo, &newcombo::genflags, 2> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODLIFTFLAGS_registrar(COMBODLIFTFLAGS, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo* cmb){
			return 8;
		},
		[](newcombo* cmb, int index) -> int {
			auto bit = 1 << index;
			if(bit == LF_BREAKONSOLID)
				return cmb->lift_weap_data.wflags & WFLAG_BREAK_ON_SOLID;
			return cmb->liftflags & (1 << index);
		},
		[](newcombo* cmb, int index, int value){
			auto bit = lift_flags(1 << index);
			if(bit == LF_BREAKONSOLID)
				SETFLAG(cmb->lift_weap_data.wflags, WFLAG_BREAK_ON_SOLID, value);
			else
				SETFLAG(cmb->liftflags, bit, value);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODUSRFLAGARR_registrar(COMBODUSRFLAGARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<newcombo, &newcombo::usrflags, 16> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODATAINITD_registrar(COMBODATAINITD, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo* cmb){
			return comptime_array_size(cmb->initd);
		},
		[](newcombo* cmb, int index) -> int {
			return cmb->initd[index] * (get_qr(qr_COMBODATA_INITD_MULT_TENK) ? 10000 : 1);
		},
		[](newcombo* cmb, int index, int value){
			// TODO: Not sure why this compat qr multiplies by 10000, should probably divide?
			cmb->initd[index] = value * ( get_qr(qr_COMBODATA_INITD_MULT_TENK) ? 10000 : 1);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERBUTTON_registrar(COMBODTRIGGERBUTTON, []{
	static ScriptingArray_ObjectComputed<newcombo, bool> impl(
		[](newcombo* cmb){
			return 8;
		},
		[](newcombo* cmb, int index) -> bool {
			if (auto* trig = get_first_combo_trigger())
				return trig->triggerbtn & (1<<index);
			return 0;
		},
		[](newcombo* cmb, int index, bool value){
			if (auto* trig = get_first_combo_trigger())
				SETFLAG(trig->triggerbtn, 1<<index, value);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERFLAGS2_registrar(COMBODTRIGGERFLAGS2, []{
	static ScriptingArray_ObjectComputed<newcombo, bool> impl(
		[](newcombo* cmb){
			return 32*6;
		},
		[](newcombo* cmb, int index) -> bool {
			if (auto* trig = get_first_combo_trigger())
			{
				if (index/32 == 0 && (1<<index%32) == combotriggerONLYGENTRIG)
					return cmb->only_gentrig;
				else
					return (trig->triggerflags[index/32] & (1<<index%32));
			}

			return 0;
		},
		[](newcombo* cmb, int index, bool value){
			if (auto* trig = get_first_combo_trigger())
			{
				SETFLAG(trig->triggerflags[index/32],1<<(index%32),value);
				if (index/32 == 0 && (1<<index%32) == combotriggerONLYGENTRIG)
					cmb->only_gentrig = value;
			}
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERS_registrar(COMBODTRIGGERS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto cmb = checkCombo(ref))
				return zc_min(cmb->triggers.size(), MAX_COMBO_TRIGGERS);

			return 0;
		},
		[](int ref, int index) -> int {
			return dword(ref) | dword(index) << 24;
		},
		[](int ref, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERFLAGS_registrar(COMBODTRIGGERFLAGS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto cmb = checkCombo(ref))
			{
				if (cmb->triggers.empty())
					cmb->triggers.emplace_back();
				return comptime_array_size(cmb->triggers[0].triggerflags);
			}

			return 0;
		},
		[](int ref, int index) -> int {
			if (auto cmb = checkCombo(ref))
			{
				auto& trig = cmb->triggers[0];
				int ret = trig.triggerflags[index];
				if (index == 0)
					SETFLAG(ret, combotriggerONLYGENTRIG, cmb->only_gentrig);
				return ret;
			}

			return -1;
		},
		[](int ref, int index, int value){
			if (auto cmb = checkCombo(ref))
			{
				auto& trig = cmb->triggers[0];
				screen_combo_modify_pre(ref);
				trig.triggerflags[index] = value;
				if (index == 0)
					cmb->only_gentrig = trig.triggerflags[0] & combotriggerONLYGENTRIG;
				screen_combo_modify_post(ref);
				return true;
			}

			return false;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());
