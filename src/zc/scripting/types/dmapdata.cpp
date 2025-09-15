// dmapdata arrays.

#include "zc/scripting/arrays.h"

static ArrayRegistrar DMAPINITD_registrar(DMAPINITD, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::initD> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar DMAPDATADISABLEDITEMS_registrar(DMAPDATADISABLEDITEMS, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::disableditems> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATAGRID_registrar(DMAPDATAGRID, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::grid> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATAMAPINITD_registrar(DMAPDATAMAPINITD, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::onmap_initD> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATASUBINITD_registrar(DMAPDATASUBINITD, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::sub_initD> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATAMINIMAPTILE_registrar(DMAPDATAMINIMAPTILE, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::minimap_tile> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATAMINIMAPCSET_registrar(DMAPDATAMINIMAPCSET, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::minimap_cset> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATALARGEMAPCSET_registrar(DMAPDATALARGEMAPCSET, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::largemap_cset> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATALARGEMAPTILE_registrar(DMAPDATALARGEMAPTILE, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::largemap_tile> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATAFLAGARR_registrar(DMAPDATAFLAGARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<dmap, &dmap::flags, 32> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPDATACHARTED_registrar(DMAPDATACHARTED, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAPSCRSNORMAL; },
		[](int ref, int screen) -> int {
			if (auto dmap = checkDmap(ref))
			{
				int32_t col = (screen&15)-(dmap->type==dmOVERW ? 0 : dmap->xoff);
				if((dmap->type&dmfTYPE)!=dmOVERW ? (((unsigned)col) > 7) : (((unsigned)col) > 15))
					return -1; //Out-of-bounds; don't attempt read!
	
				int32_t di = (ref << 7) + (screen & 0x7F);
				return game->bmaps[di];
			}

			return -1;
		},
		[](int ref, int screen, int value){
			if (auto dmap = checkDmap(ref))
			{
				int32_t col = (screen&15)-(dmap->type==dmOVERW ? 0 : dmap->xoff);
				if((dmap->type&dmfTYPE)!=dmOVERW ? (((unsigned)col) > 7) : (((unsigned)col) > 15))
					return false; //Out-of-bounds; don't attempt write!
	
				int32_t di = (ref << 7) + (screen & 0x7F);
				game->bmaps[di] = value;
				return true;
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x8F>);
	return &impl;
}());
