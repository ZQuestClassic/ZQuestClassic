#include "zc/guys.h"
#include "zc/scripting/arrays.h"
#include "zscriptversion.h"

// Screen arrays.

static ArrayRegistrar COMBODD_registrar(COMBODD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.data();
		},
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			screen_combo_modify_preroutine(rpos_handle);
			rpos_handle.set_data(value);
			screen_combo_modify_postroutine(rpos_handle);
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAXCOMBOS>);
	return &impl;
}());

static ArrayRegistrar COMBOCD_registrar(COMBOCD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.cset();
		},
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			screen_combo_modify_preroutine(rpos_handle);
			rpos_handle.set_cset(value);
			screen_combo_modify_postroutine(rpos_handle);
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<15>);
	return &impl;
}());

static ArrayRegistrar COMBOFD_registrar(COMBOFD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.sflag();
		},
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			rpos_handle.set_sflag(value);
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<256>);
	return &impl;
}());

static ArrayRegistrar COMBOID_registrar(COMBOID, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.cflag();
		},
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			rpos_handle.combo().flag = value;
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<256>);
	return &impl;
}());

static ArrayRegistrar COMBOTD_registrar(COMBOTD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.ctype();
		},
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			auto cid = rpos_handle.data();
			screen_combo_modify_pre(cid);
			combobuf[cid].type = value;
			screen_combo_modify_post(cid);
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<256>);
	return &impl;
}());

static ArrayRegistrar COMBOED_registrar(COMBOED, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return (rpos_handle.combo().walk & 0xF0) >> 4;
		},
		[](int ref, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			auto& cmb = rpos_handle.combo();
			cmb.walk &= ~0xF0;
			cmb.walk |= value << 4;
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<16>);
	return &impl;
}());

static ArrayRegistrar COMBOSD_registrar(COMBOSD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.combo().walk & 0x0F;
		},
		[](int ref, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			auto& cmb = rpos_handle.combo();
			cmb.walk &= ~0x0F;
			cmb.walk |= value;
			return true;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<16>);
	return &impl;
}());

static ArrayRegistrar COMBODATAD_registrar(COMBODATAD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			return rpos_handle.data();
		},
		[](int ref, int index, int value) {
			return false;
		}
	);
	impl.setDefaultValue(-1);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCRDOORD_registrar(SCRDOORD, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::door> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		putdoor(scr, scrollbuf, index, value, true, true);
	});
	return &impl;
}());

static ArrayRegistrar SCREEN_FLAG_registrar(SCREEN_FLAG, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata* scr){ return 8 * 11; },
		[](screendata* scr, int index) -> bool {
			return (&scr->flags)[index/8] & (1 << (index%8));
		},
		[](screendata* scr, int index, bool value){
			byte& flag = (&scr->flags)[index/8];
			SETFLAG(flag, 1 << (index%8), value);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASWARPRETSQR_registrar(SCREENDATASWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata* scr){ return 4; },
		[](screendata* scr, int index){ return (scr->warpreturnc >> (8+(index*2))) & 3; },
		[](screendata* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(8+(index*2)))) | (value<<(8+(index*2)));
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar SCREENDATATWARPRETSQR_registrar(SCREENDATATWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata* scr){ return 4; },
		[](screendata* scr, int index){ return (scr->warpreturnc >> (index*2)) & 3; },
		[](screendata* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(index*2))) | (value<<(index*2));
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar SCREENDATAFLAGS_registrar(SCREENDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<screendata, byte> impl(
		[](screendata* scr){ return 11; },
		[](screendata* scr, int index){ return (&scr->flags)[index]; },
		[](screendata* scr, int index, byte value){ (&scr->flags)[index] = value; }
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENEFLAGSD_registrar(SCREENEFLAGSD, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata* scr){ return 3; },
		[](screendata* scr, int index){ return get_screeneflags(scr, index); },
		[](screendata* scr, int index, int value){}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.boundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCREENFLAGSD_registrar(SCREENFLAGSD, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata* scr){ return 10; },
		[](screendata* scr, int index){ return get_screenflags(scr, index); },
		[](screendata* scr, int index, int value){}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.boundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCREENDATAENEMY_registrar(SCREENDATAENEMY, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::enemy> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAXGUYS>);
	return &impl;
}());

static ArrayRegistrar SCREENDATALAYERINVIS_registrar(SCREENDATALAYERINVIS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::hidelayers, 7> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASCRIPTDRAWS_registrar(SCREENDATASCRIPTDRAWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::hidescriptlayers, 7> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPOVFLAGS_registrar(SCREENDATASIDEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::sidewarpoverlayflags, 4> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPOVFLAGS_registrar(SCREENDATATILEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::tilewarpoverlayflags, 4> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENLENSHIDES_registrar(SCREENLENSHIDES, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::lens_hide, 7> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		if (value) scr->lens_show &= ~(1<<index);
	});
	return &impl;
}());

static ArrayRegistrar SCREENLENSSHOWS_registrar(SCREENLENSSHOWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::lens_show, 7> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		if (value) scr->lens_hide &= ~(1<<index);
	});
	return &impl;
}());

static ArrayRegistrar SCREENDATALAYERMAP_registrar(SCREENDATALAYERMAP, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkScreen(ref))
				return comptime_array_size(scr->layermap);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkScreen(ref))
				return scr->layermap[index - 1];

			return -1;
		},
		[](int ref, int index, int value){
			if (auto scr = checkScreen(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				scr->layermap[index - 1] = value;
				mark_current_region_handles_dirty();
				return true;
			}

			return false;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATALAYERSCREEN_registrar(SCREENDATALAYERSCREEN, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkScreen(ref))
				return comptime_array_size(scr->layerscreen);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkScreen(ref))
				return scr->layerscreen[index - 1];

			return -1;
		},
		[](int ref, int index, int value){
			if (auto scr = checkScreen(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				scr->layerscreen[index - 1] = value;
				mark_current_region_handles_dirty();
				return true;
			}

			return false;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAPSCRS>);
	return &impl;
}());

static ArrayRegistrar SCREENDATALAYEROPACITY_registrar(SCREENDATALAYEROPACITY, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkScreen(ref))
				return comptime_array_size(scr->layeropacity);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkScreen(ref))
				return scr->layeropacity[index - 1];

			return -1;
		},
		[](int ref, int index, int value){
			if (auto scr = checkScreen(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				scr->layeropacity[index - 1] = value;
				return true;
			}

			return false;
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAPATH_registrar(SCREENDATAPATH, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::path> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPSC_registrar(SCREENDATASIDEWARPSC, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarpscr> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPDMAP_registrar(SCREENDATASIDEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarpdmap> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETCOMBO_registrar(SCREENDATASECRETCOMBO, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretcombo> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETCSET_registrar(SCREENDATASECRETCSET, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretcset> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETFLAG_registrar(SCREENDATASECRETFLAG, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretflag> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPTYPE_registrar(SCREENDATASIDEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarptype> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPSCREEN_registrar(SCREENDATATILEWARPSCREEN, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarpscr> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPTYPE_registrar(SCREENDATATILEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarptype> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAWARPRETX_registrar(SCREENDATAWARPRETX, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::warpreturnx> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAWARPRETY_registrar(SCREENDATAWARPRETY, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::warpreturny> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPDMAP_registrar(SCREENDATATILEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarpdmap> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENINITD_registrar(SCREENINITD, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::screeninitd> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SCREENSTATED_registrar(SCREENSTATED, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata* scr){ return 16; },
		[](screendata* scr, int index) -> bool {
			int mi = mapind(cur_map, scr->screen);
			if (mi < 0)
				return false;

			return (game->maps[mi] >> index) & 1;
		},
		[](screendata* scr, int index, bool value){
			int mi = mapind(cur_map, scr->screen);
			if (value)
				setmapflag_mi(mi, 1 << index);
			else
				unsetmapflag_mi(mi, 1 << index);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENEXSTATED_registrar(SCREENEXSTATED, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata* scr){ return 32; },
		[](screendata* scr, int index) -> bool {
			int mi = mapind(cur_map, scr->screen);
			if (mi < 0)
				return false;

			return (game->xstates[mi] >> index) & 1;
		},
		[](screendata* scr, int index, bool value){
			int mi = mapind(cur_map, scr->screen);
			if (value)
				setxmapflag_mi(mi, 1 << index);
			else
				unsetxmapflag_mi(mi, 1 << index);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENSIDEWARPID_registrar(SCREENSIDEWARPID, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata* scr){ return 4; },
		[](screendata* scr, int index) -> int {
			return ((scr->flags2 >> index) & 1)
				? (scr->sidewarpindex >> (2*index)) & 3 //Return which warp is set
				: -1; //Returns -1 if no warp is set
		},
		[](screendata* scr, int index, int value){
			// value: none (-1), A, B, C, D
			if(value == -1)
			{
				scr->flags2 &= ~(1<<index); //Unset the "Enabled" flag for this dir
				scr->sidewarpindex &= ~(3<<(2*index)); //Clear the dir as well.
			}
			else
			{
				scr->flags2 |= 1<<index; //Set the "Enabled" flag for this dir
				scr->sidewarpindex &= ~(3<<(2*index)); //Clear the dir bits
				scr->sidewarpindex |= (value<<(2*index)); //Set the new dir
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-1, 3>);
	return &impl;
}());

static ArrayRegistrar SCREENSCRDATA_registrar(SCREENSCRDATA, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref){
			int mapindex = map_screen_index(cur_map, ref);
			if (mapindex < 0) return 0;

			return (int)game->scriptDataSize(mapindex);
		},
		[](int ref, int index){
			int mapindex = map_screen_index(cur_map, ref);
			if (mapindex < 0) return 0;

			return game->screen_data[mapindex][index];
		},
		[](int ref, int index, int value){
			int mapindex = map_screen_index(cur_map, ref);
			if (mapindex < 0) return false;

			game->screen_data[mapindex][index] = value;
			return true;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SDD_registrar(SDD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			int32_t di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			if (di < 0 || di >= game->screen_d.size())
				return 0;

			return 8;
		},
		[](int, int index){
			int32_t di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			return FFScript::get_screen_d(di, index);
		},
		[](int, int index, int value){
			int32_t di2 = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			FFScript::set_screen_d(di2, index, value);
			return true;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SCREEN_NPCS_registrar(SCREEN_NPCS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return guys.Count();
		},
		[](int, int index){
			return guys.spr(index)->getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar SCREEN_ITEMS_registrar(SCREEN_ITEMS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return items.Count();
		},
		[](int, int index){
			return items.spr(index)->getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar SCREEN_LWEAPONS_registrar(SCREEN_LWEAPONS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return Lwpns.Count();
		},
		[](int, int index){
			return Lwpns.spr(index)->getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar SCREEN_EWEAPONS_registrar(SCREEN_EWEAPONS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return Ewpns.Count();
		},
		[](int, int index){
			return Ewpns.spr(index)->getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar SCREEN_PORTALS_registrar(SCREEN_PORTALS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return portals.Count();
		},
		[](int, int index){
			return portals.spr(index)->getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar SCREEN_FFCS_registrar(SCREEN_FFCS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){
			return MAX_FFCID + 1;
		},
		[](int, int index){
			if (auto ffc = checkFFC(index))
			{
				if (ZScriptVersion::ffcRefIsSpriteId())
					return ffc->getUID();

				return index * 10000;
			}

			return 0;
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());
