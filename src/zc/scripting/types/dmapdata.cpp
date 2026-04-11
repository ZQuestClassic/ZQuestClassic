#include "zc/scripting/types/dmapdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

void playLevelMusic();

int32_t dmapdata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case DMAPDATAASUBSCRIPT:	//word
		{
			ret = (DMaps[GET_REF(dmapdataref)].active_sub_script) * 10000; break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].compass) * 10000; break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].cont) * 10000; break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			ret = (DMaps[GET_REF(dmapdataref)].flags) * 10000; break;
		}
		case DMAPDATAFLOOR:	//byte
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].floor) * 10000; break;
		}
		case DMAPDATAID: ret = ri->dmapdataref*10000; break; //read-only, equal to CurrentDMap
		case DMAPDATAINTROSTRINGID:
		{
			ret = (DMaps[GET_REF(dmapdataref)].intro_string_id * 10000); break;
		}
		case DMAPDATALEVEL:	//word
		{
			ret = ((word)DMaps[GET_REF(dmapdataref)].level) * 10000; break;
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
			ret = ((byte)DMaps[GET_REF(dmapdataref)].map + 1) * 10000; break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			ret = (DMaps[GET_REF(dmapdataref)].onmap_script) * 10000; break;
		}
		case DMAPDATAMIDI:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = convert_to_old_midi_id(amus->midi) * 10000;
			break;
		}
		case DMAPDATAMIRRDMAP:
		{
			ret = (DMaps[GET_REF(dmapdataref)].mirrorDMap) * 10000; break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				ret = amus->enhanced.track * 10000;
			break;
		}
		case DMAPDATAOFFSET:	//char
		{
			ret = ((char)DMaps[GET_REF(dmapdataref)].xoff) * 10000; break;
		}
		case DMAPDATAPALETTE:	//word
		{
			ret = ((word)DMaps[GET_REF(dmapdataref)].color) * 10000; break;
		}
		case DMAPDATAPSUBSCRIPT:	//word
		{
			ret = (DMaps[GET_REF(dmapdataref)].passive_sub_script) * 10000; break;
		}
		case DMAPDATASIDEVIEW:	//byte
		{
			ret = ((DMaps[GET_REF(dmapdataref)].sideview) ? 10000 : 0); break;
		}
		case DMAPDATASUBSCRA:
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].active_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRO:
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].overlay_subscreen) * 10000; break;
		}
		case DMAPDATASUBSCRP:
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].passive_subscreen) * 10000; break;
		}
		case DMAPDATATYPE:	//byte
		{
			ret = ((byte)DMaps[GET_REF(dmapdataref)].type&dmfTYPE) * 10000; break;
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
			ret = DMaps[GET_REF(dmapdataref)].dmap_gravity.getZLong();
			break;
		}
		case DMAPDATA_MUSIC:
		{
			ret = DMaps[GET_REF(dmapdataref)].music;
			break;
		}
		case DMAPDATA_TERMINAL_VELOCITY:
		{
			ret = DMaps[GET_REF(dmapdataref)].dmap_terminal_v.getZLong();
			break;
		}
		case DMAPSCRIPT:	//word
		{
			ret = (DMaps[GET_REF(dmapdataref)].script) * 10000; break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void dmapdata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case DMAPDATAASUBSCRIPT:	//byte
		{
			DMaps[GET_REF(dmapdataref)].active_sub_script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::ScriptedActiveSubscreen, ri->dmapdataref);
			break;
		}
		case DMAPDATACOMPASS:	//byte
		{
			DMaps[GET_REF(dmapdataref)].compass = ((byte)(value / 10000)); break;
		}
		case DMAPDATACONTINUE:	//byte
		{
			DMaps[GET_REF(dmapdataref)].cont = ((byte)(value / 10000)); break;
		}
		case DMAPDATAFLAGS:	 //int32_t
		{
			DMaps[GET_REF(dmapdataref)].flags = (value / 10000); break;
		}
		case DMAPDATAFLOOR:	//byte
		{
			DMaps[GET_REF(dmapdataref)].floor = ((byte)(value / 10000)); break;
		}
		case DMAPDATAINTROSTRINGID:
		{
			DMaps[GET_REF(dmapdataref)].intro_string_id = (value / 10000);
			break;
		}
		case DMAPDATALEVEL:	//word
		{
			DMaps[GET_REF(dmapdataref)].level = ((word)(value / 10000)); break;
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
			DMaps[GET_REF(dmapdataref)].map = ((byte)(value / 10000)) - 1; break;
		}
		case DMAPDATAMAPSCRIPT:	//byte
		{
			DMaps[GET_REF(dmapdataref)].onmap_script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
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
			DMaps[GET_REF(dmapdataref)].mirrorDMap = vbound(value / 10000, -1, MAXDMAPS); break;
		}
		case DMAPDATAMUISCTRACK:	//byte
		{
			if (auto* amus = checkMusic(find_or_make_dmap_music(GET_REF(dmapdataref))))
				amus->enhanced.track = ((byte)(value / 10000));
			break;
		}
		case DMAPDATAOFFSET:	//char
		{
			DMaps[GET_REF(dmapdataref)].xoff = ((char)(value / 10000)); break;
		}
		case DMAPDATAPALETTE:	//word
		{
			DMaps[GET_REF(dmapdataref)].color= ((word)(value / 10000));
			if(ri->dmapdataref == cur_dmap)
			{
				loadlvlpal(DMaps[GET_REF(dmapdataref)].color);
				currcset = DMaps[GET_REF(dmapdataref)].color;
			}
			break;
		}
		case DMAPDATAPSUBSCRIPT:	//byte
		{
			FFScript::deallocateAllScriptOwned(ScriptType::ScriptedPassiveSubscreen, ri->dmapdataref);
			word val = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			if (FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && ri->dmapdataref == cur_dmap && val == DMaps[GET_REF(dmapdataref)].passive_sub_script)
				break;
			DMaps[GET_REF(dmapdataref)].passive_sub_script = val;
			if(ri->dmapdataref == cur_dmap)
			{
				FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) = val != 0;
			};
			break;
		}
		case DMAPDATASIDEVIEW:	//byte, treat as bool
		{
			DMaps[GET_REF(dmapdataref)].sideview = ((value) ? 1 : 0); break;
		}
		case DMAPDATASUBSCRA:
		{
			bool changed = DMaps[GET_REF(dmapdataref)].active_subscreen != ((byte)(value / 10000));
			DMaps[GET_REF(dmapdataref)].active_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRO:
		{
			bool changed = DMaps[GET_REF(dmapdataref)].overlay_subscreen != ((byte)(value / 10000));
			DMaps[GET_REF(dmapdataref)].overlay_subscreen = ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATASUBSCRP:
		{
			bool changed = DMaps[GET_REF(dmapdataref)].passive_subscreen != ((byte)(value / 10000));
			DMaps[GET_REF(dmapdataref)].passive_subscreen= ((byte)(value / 10000));
			if(changed&&ri->dmapdataref==cur_dmap)
				update_subscreens();
			break;
		}
		case DMAPDATATYPE:	//byte
		{
			DMaps[GET_REF(dmapdataref)].type = (((byte)(value / 10000))&dmfTYPE) | (DMaps[GET_REF(dmapdataref)].type&~dmfTYPE); break;
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
			DMaps[GET_REF(dmapdataref)].dmap_gravity = zslongToFix(value);
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
			DMaps[GET_REF(dmapdataref)].dmap_terminal_v = zslongToFix(value);
			break;
		}
		case DMAPSCRIPT:	//byte
		{
			DMaps[GET_REF(dmapdataref)].script = vbound((value / 10000),0,NUMSCRIPTSDMAP-1);
			on_reassign_script_engine_data(ScriptType::DMap, ri->dmapdataref);
			break;
		}

		default:
			NOTREACHED();
	}
}

// dmapdata arrays.

static ArrayRegistrar DMAPINITD_registrar(DMAPINITD, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::initD> impl;
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
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::onmap_initD> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar DMAPDATASUBINITD_registrar(DMAPDATASUBINITD, []{
	static ScriptingArray_ObjectMemberCArray<dmap, &dmap::sub_initD> impl;
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
