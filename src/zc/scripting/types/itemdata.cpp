#include "zc/scripting/arrays.h"

// itemdata arrays.

static ArrayRegistrar IDATAINITDD_registrar(IDATAINITDD, []{
	static ScriptingArray_ObjectMemberCArray<itemdata, &itemdata::initiald> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar IDATAMISCD_registrar(IDATAMISCD, []{
	static ScriptingArray_ObjectMemberCArray<itemdata, &itemdata::wpn_misc_d> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar IDATAMOVEFLAGS_registrar(IDATAMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<itemdata, &itemdata::moveflags, 11> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar IDATAWMOVEFLAGS_registrar(IDATAWMOVEFLAGS, []{
	static ScriptingArray_ObjectSubMemberBitwiseFlags<itemdata, &itemdata::weap_data, &weapon_data::moveflags, 11> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar IDATAWPNINITD_registrar(IDATAWPNINITD, []{
	static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::initd> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar IDATABURNINGSPR_registrar(IDATABURNINGSPR, []{
	static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::burnsprs> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar IDATABURNINGLIGHTRAD_registrar(IDATABURNINGLIGHTRAD, []{
	static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::light_rads> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar IDATAATTRIB_registrar(IDATAATTRIB, []{
	static ScriptingArray_ObjectComputed<itemdata, int> impl(
		[](itemdata* item){
			return 10;
		},
		[](itemdata* item, int index) -> int {
			switch(index)
			{
				case 0: return (item->misc1);
				case 1: return (item->misc2);
				case 2: return (item->misc3);
				case 3: return (item->misc4);
				case 4: return (item->misc5);
				case 5: return (item->misc6);
				case 6: return (item->misc7);
				case 7: return (item->misc8);
				case 8: return (item->misc9);
				case 9: return (item->misc10);
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, int value){
			switch(index)
			{
				case 0: item->misc1 = value; break;
				case 1: item->misc2 = value; break;
				case 2: item->misc3 = value; break;
				case 3: item->misc4 = value; break;
				case 4: item->misc5 = value; break;
				case 5: item->misc6 = value; break;
				case 6: item->misc7 = value; break;
				case 7: item->misc8 = value; break;
				case 8: item->misc9 = value; break;
				case 9: item->misc10 = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar IDATAATTRIB_L_registrar(IDATAATTRIB_L, []{
	static ScriptingArray_ObjectComputed<itemdata, int> impl(
		[](itemdata* item){
			return 10;
		},
		[](itemdata* item, int index) -> int {
			switch(index)
			{
				case 0: return (item->misc1);
				case 1: return (item->misc2);
				case 2: return (item->misc3);
				case 3: return (item->misc4);
				case 4: return (item->misc5);
				case 5: return (item->misc6);
				case 6: return (item->misc7);
				case 7: return (item->misc8);
				case 8: return (item->misc9);
				case 9: return (item->misc10);
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, int value){
			switch(index)
			{
				case 0: item->misc1 = value; break;
				case 1: item->misc2 = value; break;
				case 2: item->misc3 = value; break;
				case 3: item->misc4 = value; break;
				case 4: item->misc5 = value; break;
				case 5: item->misc6 = value; break;
				case 6: item->misc7 = value; break;
				case 7: item->misc8 = value; break;
				case 8: item->misc9 = value; break;
				case 9: item->misc10 = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-1);
	impl.setMul10000(false);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar IDATASPRITE_registrar(IDATASPRITE, []{
	static ScriptingArray_ObjectComputed<itemdata, int> impl(
		[](itemdata* item){
			return 10;
		},
		[](itemdata* item, int index) -> int {
			switch(index)
			{
				case 0: return (item->wpn);
				case 1: return (item->wpn2);
				case 2: return (item->wpn3);
				case 3: return (item->wpn4);
				case 4: return (item->wpn5);
				case 5: return (item->wpn6);
				case 6: return (item->wpn7);
				case 7: return (item->wpn8);
				case 8: return (item->wpn9);
				case 9: return (item->wpn10);
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, int value){
			switch(index)
			{
				case 0: item->wpn = value; break;
				case 1: item->wpn2 = value; break;
				case 2: item->wpn3 = value; break;
				case 3: item->wpn4 = value; break;
				case 4: item->wpn5 = value; break;
				case 5: item->wpn6 = value; break;
				case 6: item->wpn7 = value; break;
				case 7: item->wpn8 = value; break;
				case 8: item->wpn9 = value; break;
				case 9: item->wpn10 = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar IDATAFLAGS_registrar(IDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<itemdata, bool> impl(
		[](itemdata* item){
			return 16;
		},
		[](itemdata* item, int index) -> bool {
			switch(index)
			{
				case 0: return item->flags & item_flag1;
				case 1: return item->flags & item_flag2;
				case 2: return item->flags & item_flag3;
				case 3: return item->flags & item_flag4;
				case 4: return item->flags & item_flag5;
				case 5: return item->flags & item_flag6;
				case 6: return item->flags & item_flag7;
				case 7: return item->flags & item_flag8;
				case 8: return item->flags & item_flag9;
				case 9: return item->flags & item_flag10;
				case 10: return item->flags & item_flag11;
				case 11: return item->flags & item_flag12;
				case 12: return item->flags & item_flag13;
				case 13: return item->flags & item_flag14;
				case 14: return item->flags & item_flag15;
				case 15: return item->flags & item_passive_script;
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, bool value){
			switch(index)
			{
				case 0:
					SETFLAG(item->flags, item_flag1, value);
					break;
				case 1:
					SETFLAG(item->flags, item_flag2, value);
					break;
				case 2:
					SETFLAG(item->flags, item_flag3, value);
					break;
				case 3:
					SETFLAG(item->flags, item_flag4, value);
					break;
				case 4:
					SETFLAG(item->flags, item_flag5, value);
					break;
				case 5:
					SETFLAG(item->flags, item_flag6, value);
					break;
				case 6:
					SETFLAG(item->flags, item_flag7, value);
					break;
				case 7:
					SETFLAG(item->flags, item_flag8, value);
					break;
				case 8:
					SETFLAG(item->flags, item_flag9, value);
					cache_tile_mod_clear();
					break;
				case 9:
					SETFLAG(item->flags, item_flag10, value);
					break;
				case 10:
					SETFLAG(item->flags, item_flag11, value);
					break;
				case 11:
					SETFLAG(item->flags, item_flag12, value);
					break;
				case 12:
					SETFLAG(item->flags, item_flag13, value);
					break;
				case 13:
					SETFLAG(item->flags, item_flag14, value);
					break;
				case 14:
					SETFLAG(item->flags, item_flag15, value);
					break;
				case 15:
					SETFLAG(item->flags, item_passive_script, value);
					break;
				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());
