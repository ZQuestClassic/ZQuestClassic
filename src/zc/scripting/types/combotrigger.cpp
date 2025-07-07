#include "zc/scripting/arrays.h"

// combotrigger arrays.

static ArrayRegistrar CMBTRIGBUTTON_registrar(CMBTRIGBUTTON, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::triggerbtn, 8> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQLVLSTATE_registrar(CMBTRIGREQLVLSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::req_level_state, 32> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQLVLSTATE_registrar(CMBTRIGUNREQLVLSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::unreq_level_state, 32> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQGLOBALSTATE_registrar(CMBTRIGREQGLOBALSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::req_global_state, 256> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQGLOBALSTATE_registrar(CMBTRIGUNREQGLOBALSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::unreq_global_state, 256> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGGERREQPLAYERDIR_registrar(CMBTRIGGERREQPLAYERDIR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::req_player_dir, 4> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGFLAGS_registrar(CMBTRIGFLAGS, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int ref) -> int {
			if (checkComboTrigger(ref))
				return 32*6;

			return 0;
		},
		[](int ref, int index) -> bool {
			if (auto* trig = checkComboTrigger(ref))
			{
				if (index/32 == 0 && (1<<index%32) == combotriggerONLYGENTRIG)
				{
					auto cmb = checkComboFromTriggerRef(ref);
					return cmb ? cmb->only_gentrig : 0;
				}
				else
					return (trig->triggerflags[index/32] & (1<<index%32));
			}

			return -1;
		},
		[](int ref, int index, bool value){
			if (auto* trig = checkComboTrigger(ref))
			{
				SETFLAG(trig->triggerflags[index/32],1<<(index%32),value);
				if (index/32 == 0 && (1<<index%32) == combotriggerONLYGENTRIG)
				{
					if (auto cmb = checkComboFromTriggerRef(ref))
						cmb->only_gentrig = value;
				}
				return true;
			}

			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());
