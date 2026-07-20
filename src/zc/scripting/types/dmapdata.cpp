#include "zc/scripting/types/dmapdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"
#include "zc/zc_sys.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t dmapdata_get_register(int32_t reg)
{
	int32_t ret = 0;
	dmap& dmap = DMaps[GET_REF(dmapdataref)];

	switch (reg)
	{
		case DMAPDATAASUBSCRIPT:	//word
		{
			ret = (dmap.active_sub_scrconfig.script) * 10000; break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			ret = ((byte)dmap.compass) * 10000; break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			ret = ((byte)dmap.cont) * 10000; break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			ret = (dmap.flags) * 10000; break;
		}
		case DMAPDATAFLOOR:	//byte
		{
			ret = ((byte)dmap.floor) * 10000; break;
		}
		case DMAPDATAID: ret = ri->dmapdataref*10000; break; //read-only, equal to CurrentDMap
		case DMAPDATAINTROSTRINGID:
		{
			ret = (dmap.intro_string_id * 10000); break;
		}
		case DMAPDATALEVEL:	//word
		{
			ret = ((word)dmap.level) * 10000; break;
		}
		case DMAPDATALOOPEND:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.loop_end;
			break;
		}
		case DMAPDATALOOPSTART:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.loop_start;
			break;
		}
		case DMAPDATAMAP: 	//byte
		{
			ret = ((byte)dmap.map + 1) * 10000; break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			ret = (dmap.onmap_scrconfig.script) * 10000; break;
		}
		case DMAPDATAMIDI:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = convert_to_old_midi_id(amus->midi) * 10000;
			break;
		}
		case DMAPDATAMIRRDMAP:
		{
			ret = (dmap.mirrorDMap) * 10000; break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.track * 10000;
			break;
		}
		case DMAPDATAOFFSET:	//char
		{
			ret = ((char)dmap.xoff) * 10000; break;
		}
		case DMAPDATAPALETTE:	//word
		{
			ret = ((word)dmap.color) * 10000; break;
		}
		case DMAPDATAPSUBSCRIPT:	//word
		{
			ret = (dmap.passive_sub_scrconfig.script) * 10000; break;
		}
		case DMAPDATASIDEVIEW:	//byte
		{
			ret = ((dmap.sideview) ? 10000 : 0); break;
		}
		case DMAPDATASUBSCRA:
		{
			ret = ((byte)dmap.active_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRO:
		{
			ret = ((byte)dmap.overlay_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRP:
		{
			ret = ((byte)dmap.passive_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRM:
		{
			ret = ((byte)dmap.map_subscreen) * 10000; break;
		}
		case DMAPDATATYPE:	//byte
		{
			ret = ((byte)dmap.type&dmfTYPE) * 10000; break;
		}
		case DMAPDATAXFADEIN:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.xfade_in * 10000;
			break;
		}
		case DMAPDATAXFADEOUT:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.xfade_out * 10000;
			break;
		}
		case DMAPDATA_GRAVITY_STRENGTH:
		{
			ret = dmap.dmap_gravity.getZLong();
			break;
		}
		case DMAPDATA_MUSIC:
		{
			ret = dmap.music;
			break;
		}
		case DMAPDATA_TERMINAL_VELOCITY:
		{
			ret = dmap.dmap_terminal_v.getZLong();
			break;
		}
		case DMAPSCRIPT:	//word
		{
			ret = (dmap.active_scrconfig.script) * 10000; break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void dmapdata_set_register(int32_t reg, int32_t value)
{
	dmap& dmap = DMaps[GET_REF(dmapdataref)];

	switch (reg)
	{
		case DMAPDATAASUBSCRIPT:	//byte
		{
			dmap.active_sub_scrconfig.script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::ScriptedActiveSubscreen, ri->dmapdataref);
			break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			dmap.compass = ((byte)(value / 10000)); break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			dmap.cont = ((byte)(value / 10000)); break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			dmap.flags = (value / 10000); break;
		}
		case DMAPDATAFLOOR:	//byte
		{
			dmap.floor = ((byte)(value / 10000)); break;
		}
		case DMAPDATAINTROSTRINGID:
		{
			dmap.intro_string_id = (value / 10000);
			break;
		}
		case DMAPDATALEVEL:	//word
		{
			dmap.level = ((word)(value / 10000)); break;
		}
		case DMAPDATALOOPEND:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
			{
				amus->enhanced.loop_end = value; 
				if (ri->dmapdataref == cur_dmap && amus->is_playing(false) && zcmusic)
					zcmusic_set_loop(zcmusic, (amus->enhanced.loop_start / 10000.0), (amus->enhanced.loop_end / 10000.0));
			}
			break;
		}
		case DMAPDATALOOPSTART:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
			{
				amus->enhanced.loop_start = value; 
				if (ri->dmapdataref == cur_dmap && amus->is_playing(false) && zcmusic)
					zcmusic_set_loop(zcmusic, (amus->enhanced.loop_start / 10000.0), (amus->enhanced.loop_end / 10000.0));
			}
			break;
		}
		case DMAPDATAMAP: 	//byte
		{
			dmap.map = ((byte)(value / 10000)) - 1; break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			dmap.onmap_scrconfig.script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::OnMap, ri->dmapdataref);
			break;
		}
		case DMAPDATAMIDI:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				amus->midi = convert_from_old_midi_id(value / 10000);
			break;
		}
		case DMAPDATAMIRRDMAP:
		{
			dmap.mirrorDMap = vbound(value / 10000, -1, MAXDMAPS); break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				amus->enhanced.track = ((byte)(value / 10000));
			break;
		}
		case DMAPDATAOFFSET:	//char
		{
			dmap.xoff = ((char)(value / 10000)); break;
		}
		case DMAPDATAPALETTE:	//word
		{
			dmap.color= ((word)(value / 10000));
			if(ri->dmapdataref == cur_dmap)
			{
				loadlvlpal(dmap.color);
				currcset = dmap.color;
			}
			break;
		}
		case DMAPDATAPSUBSCRIPT:	//byte
		{
			FFScript::deallocateAllScriptOwned(ScriptType::ScriptedPassiveSubscreen, ri->dmapdataref);
			word val = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			if (FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && ri->dmapdataref == cur_dmap && val == dmap.passive_sub_scrconfig.script)
				break;
			dmap.passive_sub_scrconfig.script = val;
			if(ri->dmapdataref == cur_dmap)
			{
				FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) = val != 0;
			};
			break;
		}
		case DMAPDATASIDEVIEW:	//byte, treat as bool
		{
			dmap.sideview = ((value) ? 1 : 0); break;
		}
		case DMAPDATASUBSCRA:
		{
			bool changed = dmap.active_subscreen != ((byte)(value / 10000));
			dmap.active_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRO:
		{
			bool changed = dmap.overlay_subscreen != ((byte)(value / 10000));
			dmap.overlay_subscreen = ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRP:
		{
			bool changed = dmap.passive_subscreen != ((byte)(value / 10000));
			dmap.passive_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRM:
		{
			bool changed = dmap.map_subscreen != ((byte)(value / 10000));
			dmap.map_subscreen = ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATATYPE:	//byte
		{
			dmap.type = (((byte)(value / 10000))&dmfTYPE) | (dmap.type&~dmfTYPE); break;
		}
		case DMAPDATAXFADEIN:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				amus->enhanced.xfade_in = (value / 10000);
			break;
		}
		case DMAPDATAXFADEOUT:
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
			{
				amus->enhanced.xfade_out = (value / 10000);
				if (amus->is_playing(false) && zcmusic)
					zcmusic->fadeoutframes = (value / 10000);
			}
			break;
		}
		case DMAPDATA_GRAVITY_STRENGTH:
		{
			dmap.dmap_gravity = zslongToFix(value);
			break;
		}
		case DMAPDATA_MUSIC:
		{
			auto dmid = GET_REF(dmapdataref);
			auto& dm = DMaps[dmid];
			if (dm.music != value && (value == 0 || checkMusic(value)))
			{
				dm.music = value;
				if (engine_music_active && dmid == cur_dmap && hero_scr->music == -1)
					playLevelMusic();
			}
			break;
		}
		case DMAPDATA_TERMINAL_VELOCITY:
		{
			dmap.dmap_terminal_v = zslongToFix(value);
			break;
		}
		case DMAPSCRIPT:	//byte
		{
			dmap.active_scrconfig.script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::DMap, ri->dmapdataref);
			break;
		}

		default:
			NOTREACHED();
	}
}

// dmapdata arrays.

static ArrayRegistrar DMAPINITD_registrar(DMAPINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<dmap, &dmap::active_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar DMAPDATADISABLEDITEMS_registrar(DMAPDATADISABLEDITEMS, []{
	static ScriptingArray_ObjectMemberBitstring<dmap, &dmap::disabled_items, MAXITEMS> impl;
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
	static ScriptingArray_ObjectSubMemberContainer<dmap, &dmap::onmap_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar DMAPDATASUBINITD_registrar(DMAPDATASUBINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<dmap, &dmap::active_sub_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.setSideEffect([](auto* dmap, auto*, auto*, int index, int val){
		if (get_qr(qr_SCRIPTS_SHARE_INITD))
			dmap->passive_sub_scrconfig.run_args[index] = val;
	});
	return &impl;
}());

static ArrayRegistrar DMAPDATAPSUBINITD_registrar(DMAPDATAPSUBINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<dmap, &dmap::passive_sub_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
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
				return game->bmaps.get(di);
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
