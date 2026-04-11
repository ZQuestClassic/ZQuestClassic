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
	int32_t ret = 0;

	switch (reg)
	{
		case DATA:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->data * 10000;
			break;
		case DELAY:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->delay * 10000;
			break;
		case FCSET:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->cset * 10000;
			break;
		case FFCHEIGHT:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->hit_height * 10000;
			break;
		case FFCID:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = (get_region_screen_offset(ffc->screen_spawned) * MAXFFCS + ffc->index + 1) * 10000;
			break;
		case FFCINDEX:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->index * 10000;
			break;
		case FFCLAYER:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->layer * 10000;
			break;
		case FFCWIDTH:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->hit_width * 10000;
			break;
		case FFLINK:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->link * 10000;
			break;
		case FFSCRIPT:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->script * 10000;
			break;
		case FFTHEIGHT:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->tysz * 10000;
			break;
		case FFTWIDTH:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->txsz * 10000;
			break;
		case FX:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->x.getZLong();
			break;
		case FY:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->y.getZLong();
			break;
		case MAX_FFC_ID:
		{
			ret = (MAX_FFCID + 1) * 10000;
			break;
		}
		case XD:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->vx.getZLong();
			break;
		case XD2:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->ax.getZLong();
			break;
		case YD:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->vy.getZLong();
			break;
		case YD2:
			if(auto ffc = ResolveFFC(GET_REF(ffcref)))
				ret = ffc->ay.getZLong();
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void ffc_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case DATA:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
			{
				zc_ffc_set(*ffc, vbound(value/10000,0,MAXCOMBOS-1));
			}
			break;
		case DELAY:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->delay = value/10000;
			break;
		case FCSET:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->cset = (value/10000)&15;
			break;
		case FFCHEIGHT:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->hit_height = (value/10000);
			break;
		case FFCID:
			break;
		case FFCLASTCHANGERX:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)) )
				ffc->changer_x=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
		case FFCLASTCHANGERY:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)) )
				ffc->changer_y=vbound(zslongToFix(value).getInt(),-32768, 32767);
			break;
		case FFCLAYER:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->layer = vbound(value/10000, 0, 7);
			break;
		case FFCWIDTH:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->hit_width = (value/10000);
			break;
		case FFLINK:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				(ffc->link)=vbound(value/10000, 0, MAXFFCS-1); // Allow "ffc->Link = 0" to unlink ffc.
			//0 is none, setting this before made it impssible to clear it. -Z
			break;
		case FFSCRIPT:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
			{
				ffc->script = vbound(value/10000, 0, NUMSCRIPTFFC-1);
				for(int32_t i=0; i<16; i++)
					ffc->miscellaneous[i] = 0;
				if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t i=0; i<8; i++)
						ffc->initd[i] = 0;
				}
				on_reassign_script_engine_data(ScriptType::FFC, ffc->index);
			}
			break;
		case FFTHEIGHT:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->tysz = vbound(value/10000, 1, 4);
			break;
		case FFTWIDTH:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->txsz = vbound(value/10000, 1, 4);
			break;
		case FX:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->x = zslongToFix(value);
			break;
		case FY:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->y=zslongToFix(value);
			break;
		case XD:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->vx=zslongToFix(value);
			break;
		case XD2:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->ax=zslongToFix(value);
			break;
		case YD:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
				ffc->vy=zslongToFix(value);
			break;
		case YD2:
			if (auto ffc = ResolveFFC(GET_REF(ffcref)))
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
		if (flag == ffc_solid || flag == ffc_changer)
			ffc->updateSolid();
	});
	return &impl;
}());

static ArrayRegistrar FFINITDD_registrar(FFINITDD, []{
	static ScriptingArray_ObjectMemberCArray<ffcdata, &ffcdata::initd> impl;
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
