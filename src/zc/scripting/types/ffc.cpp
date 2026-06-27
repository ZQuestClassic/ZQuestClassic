#include "zc/scripting/types/ffc.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/zc_ffc.h"

#include <cstdint>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t ffc_get_register(int32_t reg)
{
	if (reg == MAX_FFC_ID)
		return (MAX_FFCID + 1) * 10000;

	int32_t ret = 0;
	ffcdata* ffc = ResolveFFC(GET_REF(ffcref));
	if (!ffc)
		return ret;

	switch (reg)
	{
		case DATA:
			ret = ffc->data * 10000;
			break;
		case DELAY:
			ret = ffc->delay * 10000;
			break;
		case FCSET:
			ret = ffc->cset * 10000;
			break;
		case FFCHEIGHT:
			ret = ffc->hit_height * 10000;
			break;
		case FFCID:
			ret = (get_region_screen_offset(ffc->screen_spawned) * MAXFFCS + ffc->index + 1) * 10000;
			break;
		case FFCINDEX:
			ret = ffc->index * 10000;
			break;
		case FFCLASTCHANGERX:
			ret = ffc->changer_x * 10000;
			break;
		case FFCLASTCHANGERY:
			ret = ffc->changer_y * 10000;
			break;
		case FFCLAYER:
			ret = ffc->layer * 10000;
			break;
		case FFCWIDTH:
			ret = ffc->hit_width * 10000;
			break;
		case FFLINK:
			ret = ffc->link * 10000;
			break;
		case FFSCRIPT:
			ret = ffc->scrconfig.script * 10000;
			break;
		case FFTHEIGHT:
			ret = ffc->tysz * 10000;
			break;
		case FFTWIDTH:
			ret = ffc->txsz * 10000;
			break;
		case FX:
			ret = ffc->x.getZLong();
			break;
		case FY:
			ret = ffc->y.getZLong();
			break;
		case XD:
			ret = ffc->vx.getZLong();
			break;
		case XD2:
			ret = ffc->ax.getZLong();
			break;
		case YD:
			ret = ffc->vy.getZLong();
			break;
		case YD2:
			ret = ffc->ay.getZLong();
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void ffc_set_register(int32_t reg, int32_t value)
{
	ffcdata* ffc = ResolveFFC(GET_REF(ffcref));
	if (!ffc)
		return;

	switch (reg)
	{
		case DATA:
			zc_ffc_set(*ffc, vbound(value/10000,0,MAXCOMBOS-1));
			break;
		case DELAY:
			ffc->delay = value/10000;
			break;
		case FCSET:
			ffc->cset = (value/10000)&15;
			break;
		case FFCHEIGHT:
			ffc->hit_height = (value/10000);
			break;
		case FFCID:
			break;
		case FFCLASTCHANGERX:
			ffc->changer_x=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
		case FFCLASTCHANGERY:
			ffc->changer_y=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
		case FFCLAYER:
			ffc->layer = vbound(value/10000, 0, 7);
			break;
		case FFCWIDTH:
			ffc->hit_width = (value/10000);
			break;
		case FFLINK:
			(ffc->link)=vbound(value/10000, 0, MAXFFCS-1); // Allow "ffc->Link = 0" to unlink ffc.
			//0 is none, setting this before made it impssible to clear it. -Z
			break;
		case FFSCRIPT:
		{
			ffc->scrconfig.script = vbound(value/10000, 0, NUMSCRIPTFFC-1);
			for(int32_t i=0; i<16; i++)
				ffc->miscellaneous[i] = 0;
			if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				ffc->scrconfig.run_args.fill(0);
			ffc->scrconfig.inst_init.clear();
			on_reassign_script_engine_data(ScriptType::FFC, ffc->index);
			break;
		}
		case FFTHEIGHT:
			ffc->tysz = vbound(value/10000, 1, 4);
			break;
		case FFTWIDTH:
			ffc->txsz = vbound(value/10000, 1, 4);
			break;
		case FX:
			ffc->x = zslongToFix(value);
			break;
		case FY:
			ffc->y=zslongToFix(value);
			break;
		case XD:
			ffc->vx=zslongToFix(value);
			break;
		case XD2:
			ffc->ax=zslongToFix(value);
			break;
		case YD:
			ffc->vy=zslongToFix(value);
			break;
		case YD2:
			ffc->ay=zslongToFix(value);
			break;

		default:
			NOTREACHED();
	}
}

// ffc arrays.

static ArrayRegistrar FFFLAGSD_registrar(FFFLAGSD, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ffcdata, &ffcdata::flags, 32> impl;
	impl.setSideEffect([](auto ffc, int index, int){
		auto flag = F(index);
		if (flag == ffc_solid || flag == ffc_cmb_solid || flag == ffc_changer)
			ffc->updateSolid();
	});
	return &impl;
}());

static ArrayRegistrar FFC_POSSTATE_registrar(FFC_POSSTATE, []{
	static ScriptingArray_ObjectComputed<ffcdata, bool> impl(
		[](ffcdata*){ return 8; },
		[](ffcdata* ffc, int index) -> bool {
			int mi = mapind(cur_map, ffc->screen_spawned >= 0x80 ? home_screen : ffc->screen_spawned);
			if (mi < 0)
				return false;

			return game->ffcpos_states.get(mi).get(ffc->index % MAXFFCS) & (1 << index);
		},
		[](ffcdata* ffc, int index, bool value){
			int mi = mapind(cur_map, ffc->screen_spawned >= 0x80 ? home_screen : ffc->screen_spawned);
			if (mi < 0)
				return;
			
			SETFLAG(game->ffcpos_states[mi][ffc->index % MAXFFCS], 1 << index, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar FFINITDD_registrar(FFINITDD, []{
	static ScriptingArray_ObjectSubMemberContainer<ffcdata, &ffcdata::scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.compatBoundSetterIndex();
	return &impl;
}());

static ArrayRegistrar FFMISCD_registrar(FFMISCD, []{
	static ScriptingArray_ObjectMemberCArray<ffcdata, &ffcdata::miscellaneous> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.compatBoundSetterIndex();
	return &impl;
}());
