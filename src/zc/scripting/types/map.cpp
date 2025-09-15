#include "zc/guys.h"
#include "zc/scripting/arrays.h"
#include "zc/ffscript.h"

// mapdata arrays.

static ArrayRegistrar MAPDATADOOR_registrar(MAPDATADOOR, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::door> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBODD_registrar(MAPDATACOMBODD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.data();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto rpos_handle = mapdata->resolve_pos(index))
			{
				if (mapdata->current())
					screen_combo_modify_preroutine(rpos_handle);
				rpos_handle.set_data(value);
				if (mapdata->current())
					screen_combo_modify_postroutine(rpos_handle);
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAXCOMBOS>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBODATAD_registrar(MAPDATACOMBODATAD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.data();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.setValueTransform(transforms::validate<MAXCOMBOS>);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOCD_registrar(MAPDATACOMBOCD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.cset();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto rpos_handle = mapdata->resolve_pos(index))
			{
				if (mapdata->current())
					screen_combo_modify_preroutine(rpos_handle);
				rpos_handle.set_cset(value);
				if (mapdata->current())
					screen_combo_modify_postroutine(rpos_handle);
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<15>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOTD_registrar(MAPDATACOMBOTD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.ctype();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto rpos_handle = mapdata->resolve_pos(index))
			{
				auto cid = rpos_handle.data();
				if (mapdata->current())
					screen_combo_modify_pre(rpos_handle);
				combobuf[cid].type = value;
				if (mapdata->current())
					screen_combo_modify_post(rpos_handle);
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<cMAX>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOFD_registrar(MAPDATACOMBOFD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.sflag();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto rpos_handle = mapdata->resolve_pos(index))
				rpos_handle.set_sflag(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<256>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOID_registrar(MAPDATACOMBOID, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->max_pos() + 1; },
		[](mapdata* mapdata, int index) -> int {
			if (auto rpos_handle = mapdata->resolve_pos(index))
				return rpos_handle.cflag();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto rpos_handle = mapdata->resolve_pos(index))
				rpos_handle.combo().flag = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<256>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOSD_registrar(MAPDATACOMBOSD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) {
			if (auto mapdata = checkMapData(ref))
				return mapdata->max_pos() + 1;
			return 0;
		},
		[](int ref, int index) {
			if (auto mapdata = checkMapData(ref))
			{
				if (auto rpos_handle = mapdata->resolve_pos(index))
					return rpos_handle.combo().walk & 0xF;
			}

			return -1;
		},
		[](int ref, int index, int value) {
			if (auto mapdata = checkMapData(ref))
			{
				if (auto rpos_handle = mapdata->resolve_pos(index))
				{
					int32_t cid = rpos_handle.data();
					combobuf[cid].walk &= ~0x0F;
					combobuf[cid].walk |= value;
					return true;
				}
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<16>);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOED_registrar(MAPDATACOMBOED, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) {
			if (auto mapdata = checkMapData(ref))
				return mapdata->max_pos() + 1;
			return 0;
		},
		[](int ref, int index) {
			if (auto mapdata = checkMapData(ref))
			{
				if (auto rpos_handle = mapdata->resolve_pos(index))
					return (rpos_handle.combo().walk & 0xF0)>>4;
			}

			return -1;
		},
		[](int ref, int index, int value) {
			if (auto mapdata = checkMapData(ref))
			{
				if (auto rpos_handle = mapdata->resolve_pos(index))
				{
					int32_t cid = rpos_handle.data();
					combobuf[cid].walk &= ~0xF0;
					combobuf[cid].walk |= value << 4;
					return true;
				}
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<16>);
	return &impl;
}());

static ArrayRegistrar MAPDATAPATH_registrar(MAPDATAPATH, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::path> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATASIDEWARPSC_registrar(MAPDATASIDEWARPSC, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::sidewarpscr> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATASIDEWARPDMAP_registrar(MAPDATASIDEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::sidewarpdmap> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar MAPDATASECRETCOMBO_registrar(MAPDATASECRETCOMBO, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::secretcombo> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar MAPDATASECRETCSET_registrar(MAPDATASECRETCSET, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::secretcset> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATASECRETFLAG_registrar(MAPDATASECRETFLAG, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::secretflag> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATASIDEWARPTYPE_registrar(MAPDATASIDEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::sidewarptype> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATATILEWARPSCREEN_registrar(MAPDATATILEWARPSCREEN, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::tilewarpscr> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATATILEWARPTYPE_registrar(MAPDATATILEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::tilewarptype> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATAWARPRETX_registrar(MAPDATAWARPRETX, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::warpreturnx> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATAWARPRETY_registrar(MAPDATAWARPRETY, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::warpreturny> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATATILEWARPDMAP_registrar(MAPDATATILEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::tilewarpdmap> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar MAPDATAINITDARRAY_registrar(MAPDATAINITDARRAY, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::screeninitd> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENSTATED_registrar(MAPDATASCREENSTATED, []{
	static ScriptingArray_ObjectComputed<mapdata, bool> impl(
		[](mapdata* mapdata){ return mMAXIND; },
		[](mapdata* mapdata, int index) -> bool {
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return false;

			return (game->maps[mi] >> index) & 1;
		},
		[](mapdata* mapdata, int index, bool value){
			int mi = get_mi(*mapdata);
			if (value)
				setmapflag_mi(mi, 1 << index);
			else
				unsetmapflag_mi(mi, 1 << index);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATAEXSTATED_registrar(MAPDATAEXSTATED, []{
	static ScriptingArray_ObjectComputed<mapdata, bool> impl(
		[](mapdata* mapdata){ return 32; },
		[](mapdata* mapdata, int index) -> bool {
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return false;

			return (game->xstates[mi] >> index) & 1;
		},
		[](mapdata* mapdata, int index, bool value){
			int mi = get_mi(*mapdata);
			if (value)
				setxmapflag_mi(mi, 1 << index);
			else
				unsetxmapflag_mi(mi, 1 << index);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATASIDEWARPID_registrar(MAPDATASIDEWARPID, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr* scr){ return 4; },
		[](mapscr* scr, int index) -> int {
			return ((scr->flags2 >> index) & 1)
				? (scr->sidewarpindex >> (2*index)) & 3 //Return which warp is set
				: -1; //Returns -1 if no warp is set
		},
		[](mapscr* scr, int index, int value){
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
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-1, 3>);
	return &impl;
}());

static ArrayRegistrar MAPDATASCRDATA_registrar(MAPDATASCRDATA, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref){
			int mapindex = get_ref_map_index(ref);
			if (mapindex < 0) return 0;

			return (int)game->scriptDataSize(mapindex);
		},
		[](int ref, int index){
			int mapindex = get_ref_map_index(ref);
			if (mapindex < 0) return 0;

			return game->screen_data[mapindex][index];
		},
		[](int ref, int index, int value){
			int mapindex = get_ref_map_index(ref);
			if (mapindex < 0) return false;

			game->screen_data[mapindex][index] = value;
			return true;
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar MAPDATASWARPRETSQR_registrar(MAPDATASWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr* scr){ return 4; },
		[](mapscr* scr, int index){ return (scr->warpreturnc >> (8+(index*2))) & 3; },
		[](mapscr* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(8+(index*2)))) | (value<<(8+(index*2)));
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar MAPDATATWARPRETSQR_registrar(MAPDATATWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr* scr){ return 4; },
		[](mapscr* scr, int index){ return (scr->warpreturnc >> (index*2)) & 3; },
		[](mapscr* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(index*2))) | (value<<(index*2));
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar MAPDATAENEMY_registrar(MAPDATAENEMY, []{
	static ScriptingArray_ObjectMemberCArray<mapscr, &mapscr::enemy> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAXGUYS>);
	return &impl;
}());

static ArrayRegistrar MAPDATANORESETARR_registrar(MAPDATANORESETARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::noreset, mMAXIND> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATANOCARRYARR_registrar(MAPDATANOCARRYARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::nocarry, mMAXIND> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATAEXRESET_registrar(MAPDATAEXRESET, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::exstate_reset, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATAEXCARRY_registrar(MAPDATAEXCARRY, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::exstate_carry, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATALAYERINVIS_registrar(MAPDATALAYERINVIS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::hidelayers, 7> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATASCRIPTDRAWS_registrar(MAPDATASCRIPTDRAWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::hidescriptlayers, 7> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATASIDEWARPOVFLAGS_registrar(MAPDATASIDEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::sidewarpoverlayflags, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATATILEWARPOVFLAGS_registrar(MAPDATATILEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::tilewarpoverlayflags, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATALENSHIDES_registrar(MAPDATALENSHIDES, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::lens_hide, 7> impl;
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		if (value) scr->lens_show &= ~(1<<index);
	});
	return &impl;
}());

static ArrayRegistrar MAPDATALENSSHOWS_registrar(MAPDATALENSSHOWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<mapscr, &mapscr::lens_show, 7> impl;
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		if (value) scr->lens_hide &= ~(1<<index);
	});
	return &impl;
}());

static ArrayRegistrar MAPDATAFLAGS_registrar(MAPDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<mapscr, byte> impl(
		[](mapscr* scr){ return 11; },
		[](mapscr* scr, int index){ return (&scr->flags)[index]; },
		[](mapscr* scr, int index, byte value){ (&scr->flags)[index] = value; }
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENEFLAGSD_registrar(MAPDATASCREENEFLAGSD, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr* scr){ return 3; },
		[](mapscr* scr, int index){ return get_screeneflags(scr, index); },
		[](mapscr* scr, int index, int value){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENFLAGSD_registrar(MAPDATASCREENFLAGSD, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr* scr){ return 10; },
		[](mapscr* scr, int index){ return get_screenflags(scr, index); },
		[](mapscr* scr, int index, int value){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar MAPDATA_FLAG_registrar(MAPDATA_FLAG, []{
	static ScriptingArray_ObjectComputed<mapscr, bool> impl(
		[](mapscr* scr){ return 8*11; },
		[](mapscr* scr, int index) -> bool {
			return (&scr->flags)[index/8] & (1 << (index%8));
		},
		[](mapscr* scr, int index, bool value){
			byte& flag = (&scr->flags)[index/8];
			SETFLAG(flag, 1 << (index%8), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATAMISCD_registrar(MAPDATAMISCD, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return 8; },
		[](mapdata* mapdata, int index) -> int {
			int mi = mapind(mapdata->scr->map, mapdata->scr->screen);
			if (mi < 0)
				return -1;

			return game->screen_d[mi][index];
		},
		[](mapdata* mapdata, int index, int value){
			int mi = mapind(mapdata->scr->map, mapdata->scr->screen);
			if (mi < 0)
				return;

			game->screen_d[mi][index] = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATALAYERMAP_registrar(MAPDATALAYERMAP, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkMapDataScr(ref))
				return comptime_array_size(scr->layermap);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkMapDataScr(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				return scr->layermap[index - 1];
			}

			return -1;
		},
		[](int ref, int index, int value){
			if (auto mapdata = checkMapData(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				mapdata->scr->layermap[index - 1] = value;
				if (mapdata->current())
					mark_current_region_handles_dirty();
				return true;
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MAPDATALAYERSCREEN_registrar(MAPDATALAYERSCREEN, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkMapDataScr(ref))
				return comptime_array_size(scr->layerscreen);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkMapDataScr(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				return scr->layerscreen[index - 1];
			}

			return -1;
		},
		[](int ref, int index, int value){
			if (auto mapdata = checkMapData(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				mapdata->scr->layerscreen[index - 1] = value;
				if (mapdata->current())
					mark_current_region_handles_dirty();
				return true;
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAPSCRS>);
	return &impl;
}());

static ArrayRegistrar MAPDATALAYEROPACITY_registrar(MAPDATALAYEROPACITY, []{
	static ScriptingArray_GlobalComputedOneIndexed impl(
		[](int ref) -> int {
			if (auto scr = checkMapDataScr(ref))
				return comptime_array_size(scr->layeropacity);
			return 0;
		},
		[](int ref, int index) -> int {
			if (auto scr = checkMapDataScr(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				return scr->layeropacity[index - 1];
			}

			return -1;
		},
		[](int ref, int index, int value){
			if (auto mapdata = checkMapData(ref))
			{
				if (FFCore.quest_format[vFFScript] < 11)
					index++;

				mapdata->scr->layeropacity[index - 1] = value;
				return true;
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

// mapdata ffc arrays.

// The size of all these arrays is based on the highest valid FFC, but writing beyond that is
// allowed and is a way to initialize an FFC. Therefore "skipIndexCheck" is used.

static ffc_handle_t resolve_ffc_handle_for_scripting_index(mapdata* mapdata, int index)
{
	bool supports_neg_indices = !get_qr(qr_OLD_SCRIPTS_INTERNAL_ARRAYS_BOUND_INDEX);
	if (supports_neg_indices && index < 0)
	{
		size_t size = mapdata->scr->numFFC();
		index += size;
		if (index < 0)
		{
			scripting_log_error_with_context("Invalid array index {} for internal array of size {}", index, size);
			return ffc_handle_t{};
		}
	}

	return mapdata->resolve_ffc_handle(index);
}

static ffcdata* resolve_ffc_for_scripting_index(mapdata* mapdata, int index)
{
	return resolve_ffc_handle_for_scripting_index(mapdata, index).ffc;
}

static ArrayRegistrar MAPDATAFFCSET_registrar(MAPDATAFFCSET, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index){
			if (auto ffc = resolve_ffc_handle_for_scripting_index(mapdata, index))
				return ffc.cset();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_handle_for_scripting_index(mapdata, index))
				ffc.set_cset(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFDATA_registrar(MAPDATAFFDATA, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index){
			if (auto ffc = resolve_ffc_handle_for_scripting_index(mapdata, index))
				return ffc.data();

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_handle_for_scripting_index(mapdata, index))
				ffc.set_data(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFDELAY_registrar(MAPDATAFFDELAY, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->delay;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->delay = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFEFFECTWIDTH_registrar(MAPDATAFFEFFECTWIDTH, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->hit_width;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->hit_width = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFEFFECTHEIGHT_registrar(MAPDATAFFEFFECTHEIGHT, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->hit_height;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->hit_height = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	impl.setValueTransform(transforms::validate<0, 10000>);
	return &impl;
}());

static ArrayRegistrar MAPDATAFFWIDTH_registrar(MAPDATAFFWIDTH, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->txsz;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->txsz = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	impl.setValueTransform(transforms::validate<0, 10000>);
	return &impl;
}());

static ArrayRegistrar MAPDATAFFHEIGHT_registrar(MAPDATAFFHEIGHT, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->tysz;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->tysz = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	impl.setValueTransform(transforms::validate<0, 4>);
	return &impl;
}());

static ArrayRegistrar MAPDATAFFX_registrar(MAPDATAFFX, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->x.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->x = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFY_registrar(MAPDATAFFY, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->y.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->y = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFXDELTA_registrar(MAPDATAFFXDELTA, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->vx.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->vx = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFYDELTA_registrar(MAPDATAFFYDELTA, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->vy.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->vy = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFXDELTA2_registrar(MAPDATAFFXDELTA2, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->ax.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->ax = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFYDELTA2_registrar(MAPDATAFFYDELTA2, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->ay.getZLong();

			return -10000;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->ay = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFFLAGS_registrar(MAPDATAFFFLAGS, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->flags;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
			{
				ffc->flags = (ffc_flags)value;
				ffc->updateSolid();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());

static ArrayRegistrar MAPDATAFFLINK_registrar(MAPDATAFFLINK, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->link;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->link = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	impl.setValueTransform(transforms::validate<0, MAXFFCS>);
	return &impl;
}());

static ArrayRegistrar MAPDATAFFSCRIPT_registrar(MAPDATAFFSCRIPT, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return ffc->script;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->script = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	// TODO: Not sure this makes sense...
	impl.setValueTransform(transforms::validate<0, MAXFFCS>);
	return &impl;
}());

static ArrayRegistrar MAPDATAFFINITIALISED_registrar(MAPDATAFFINITIALISED, []{
	static ScriptingArray_ObjectComputed<mapdata, bool> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> bool {
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				return get_ffc_script_engine_data(ffc->index).initialized;
			return false;
		},
		[](mapdata* mapdata, int index, bool value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				get_ffc_script_engine_data(ffc->index).initialized = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.skipIndexCheck();
	return &impl;
}());
