#include "zc/scripting/arrays.h"

// subscreendata arrays.

static ArrayRegistrar SUBDATABTNLEFT_registrar(SUBDATABTNLEFT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ZCSubscreenActive, &ZCSubscreenActive::btn_left, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATABTNRIGHT_registrar(SUBDATABTNRIGHT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ZCSubscreenActive, &ZCSubscreenActive::btn_right, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATAINITD_registrar(SUBDATAINITD, []{
	static ScriptingArray_ObjectMemberCArray<ZCSubscreenActive, &ZCSubscreenActive::initd> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATAFLAGS_registrar(SUBDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreen, bool> impl(
		[](ZCSubscreen* sub){
			switch(sub->sub_type)
			{
				case sstACTIVE:
					return 2;
				case sstPASSIVE:
					return 0;
				case sstOVERLAY:
					return 0;
			}

			return 0;
		},
		[](ZCSubscreen* sub, int index) -> bool {
			return sub->flags & (1<<index);
		},
		[](ZCSubscreen* sub, int index, bool value){
			SETFLAG(sub->flags, 1<<index, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORASPD_registrar(SUBDATASELECTORASPD, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].speed;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].speed = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSARGS_registrar(SUBDATATRANSARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			auto& trans = subscr_pg_transition;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return;
			}

			if (!subscreen_open) return;

			auto& trans = subscr_pg_transition;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSLEFTARGS_registrar(SUBDATATRANSLEFTARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			auto& trans = sub->trans_left;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			auto& trans = sub->trans_left;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSRIGHTARGS_registrar(SUBDATATRANSRIGHTARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			auto& trans = sub->trans_right;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			auto& trans = sub->trans_right;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSFLAGS_registrar(SUBDATATRANSFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive* sub){
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			auto& trans = subscr_pg_transition;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			if (sub != new_subscreen_active)
			{
				// TODO: can this even happen, given `checkSubData(ref, sstACTIVE)`?
				scripting_log_error_with_context("Only valid for the current active subscreen!");
				return;
			}

			if (!subscreen_open) return;

			auto& trans = subscr_pg_transition;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSLEFTFLAGS_registrar(SUBDATATRANSLEFTFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			auto& trans = sub->trans_left;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			auto& trans = sub->trans_left;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSRIGHTFLAGS_registrar(SUBDATATRANSRIGHTFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			auto& trans = sub->trans_right;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			auto& trans = sub->trans_right;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORCSET_registrar(SUBDATASELECTORCSET, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			return cs&0x0F;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			cs = (cs&0xF0) | value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORFLASHCSET_registrar(SUBDATASELECTORFLASHCSET, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			return (cs&0xF0)>>4;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			cs = (cs&0x0F) | (value<<4);
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORDELAY_registrar(SUBDATASELECTORDELAY, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].delay;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].delay = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORFRM_registrar(SUBDATASELECTORFRM, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].frames;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].frames = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORWID_registrar(SUBDATASELECTORWID, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].sw;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].sw = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORHEI_registrar(SUBDATASELECTORHEI, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].sh;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].sh = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORTILE_registrar(SUBDATASELECTORTILE, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].tile;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].tile = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES - 1>);
	return &impl;
}());

static ArrayRegistrar SUBDATAPAGES_registrar(SUBDATAPAGES, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto* sub = checkSubData(ref))
				return sub->sub_type == sstACTIVE ? sub->pages.size() : 1;
			return 0;
		},
		[](int ref, int index) -> int {
			if (checkSubData(ref))
			{
				auto [sb, ty, _pg, _ind] = from_subref(ref);
				return get_subref(sb, ty, index, 0);
			}

			return 0;
		},
		[](int, int index, int value){ return false; }
	);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());
