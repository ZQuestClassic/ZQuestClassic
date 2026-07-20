#include "zc/scripting/types/mapdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/maps.h"
#include "zc/guys.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"
#include "zc/zc_sys.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

mapscr* ResolveMapdataScr(int32_t mapdataref)
{
	auto mapdata = decode_mapdata_ref(mapdataref);
	if (!mapdata.scr)
		scripting_log_error_with_context("mapdata id is invalid: {}", mapdataref);
	return mapdata.scr;
}

ffc_handle_t ResolveMapdataFFC(int32_t mapdataref, int index)
{
	index -= 1;
	if (BC::checkMapdataFFC(index) != SH::_NoError)
		return ffc_handle_t{};

	auto result = decode_mapdata_ref(mapdataref);
	if (!result.scr)
	{
		scripting_log_error_with_context("mapdata id is invalid: {}", mapdataref);
		return ffc_handle_t{};
	}

	int screen_index_offset = 0;
	if (result.current() && result.layer == 0)
		screen_index_offset = get_region_screen_offset(result.screen);

	return *result.scr->getFFCHandle(index, screen_index_offset);
}

}

int mapdata::max_pos() const
{
	if (type == mapdata_type::TemporaryCurrentRegion)
		return (int)region_max_rpos;

	if (type == mapdata_type::TemporaryScrollingRegion)
		return (int)scrolling_region.screen_count * 176 - 1;

	return 175;
}

rpos_handle_t mapdata::resolve_pos(int pos) const
{
	if (!screenscrolling && scrolling())
	{
		int32_t mapdataref = create_mapdata_temp_ref(type, screen, layer);
		scripting_log_error_with_context("mapdata id is invalid: {} - screen is not scrolling right now", mapdataref);
		return rpos_handle_t{};
	}

	// mapdata loaded via `Game->LoadTempScreen(layer)` have access to the entire region.
	if (type == mapdata_type::TemporaryCurrentRegion)
	{
		rpos_t rpos = (rpos_t)pos;
		if (BC::checkComboRpos(rpos) != SH::_NoError)
			return rpos_handle_t{};

		return get_rpos_handle(rpos, layer);
	}

	// mapdata loaded via `Game->LoadScrollingScreen(layer)` have access to the entire scrolling region.
	if (type == mapdata_type::TemporaryScrollingRegion)
	{
		rpos_t rpos = (rpos_t)pos;
		rpos_t max = (rpos_t)(scrolling_region.screen_count * 176 - 1);
		if (BC::checkBoundsRpos(rpos, (rpos_t)0, max) != SH::_NoError)
			return rpos_handle_t{};

		return {base_scr, scr, scr->screen, layer, rpos, RPOS_TO_POS(rpos)};
	}

	// Otherwise, access is limited to just one screen.
	if (BC::checkComboPos(pos) != SH::_NoError)
		return rpos_handle_t{};

	if (type == mapdata_type::CanonicalScreen)
		return {base_scr, scr, screen, 0, (rpos_t)pos, pos};

	if (scrolling())
	{
		if (!scr->is_valid())
			return rpos_handle_t{};

		return {base_scr, scr, screen, layer, (rpos_t)pos, pos};
	}

	rpos_t rpos = POS_TO_RPOS(pos, screen);
	if (BC::checkComboRpos(rpos) != SH::_NoError)
		return rpos_handle_t{};

	return {base_scr, scr, screen, layer, rpos, pos};
}

ffc_handle_t mapdata::resolve_ffc_handle(int index) const
{
	index -= 1;
	if (BC::checkMapdataFFC(index) != SH::_NoError)
		return ffc_handle_t{};

	int screen_index_offset = 0;
	if (current() && layer == 0)
		screen_index_offset = get_region_screen_offset(screen);

	return *scr->getFFCHandle(index, screen_index_offset);
}

ffcdata* mapdata::resolve_ffc(int index) const
{
	return resolve_ffc_handle(index).ffc;
}

// Decodes a `mapdataref` (reference number) for a temporary screen.
//
// A mapdataref can refer to:
//
// - the canonical mapscr data, loaded via `Game->LoadMapData(int map, int screen)`
// - a temporary mapscr, loaded via `Game->LoadTempScreen(int layer, int? screen)`
// - a temporary mapscr, loaded via `Game->LoadScrollingScreen(int layer, int? screen)`
//
// The canonical maprefs are > 0, and temporary ones are all negative. 0 is invalid (null).
//
// If temporary, and loaded without specifiying a screen index, we allow combo array variables (like
// `ComboX[pos]`) to address any rpos in the region. Otherwise, only positions in the exact screen
// referenced by `mapdataref` can be used (0-175).
mapdata decode_mapdata_ref(int ref)
{
	if (ref == 0)
		return mapdata{};

	if (ref > 0)
	{
		ref -= 1;
		if (ref >= TheMaps.size())
			return mapdata{};

		int screen = ref % MAPSCRS;
		mapscr* scr = &TheMaps[ref];
		return mapdata{mapdata_type::CanonicalScreen, scr, scr, screen, 0};
	}

	// Negative values are for temporary screens.

	ref = -(ref + 1);
	bool is_scrolling = ref & 1;
	bool is_region = ref & 2;
	int screen = (ref & 0x0000FF00) >> 8;
	int layer  = (ref & 0x00FF0000) >> 16;

	if (is_region)
	{
		if (is_scrolling)
			screen = scrolling_region.origin_screen;
		else
			screen = cur_screen;
	}

	mapscr* base_scr = nullptr;
	mapscr* scr = nullptr;
	if (is_scrolling)
	{
		int index = screen * 7 + layer;
		if (index >= 0 && index < FFCore.ScrollingScreensAll.size())
		{
			base_scr = FFCore.ScrollingScreensAll[screen * 7];
			scr = FFCore.ScrollingScreensAll[index];
		}
	}
	else
	{
		if (layer >= 0 && layer <= 6 && is_in_current_region(screen))
		{
			base_scr = get_scr_layer(screen, 0);
			scr = get_scr_layer(screen, layer);
		}
	}

	if (!scr)
		return mapdata{};

	auto type = mapdata_type::None;
	if (is_region && is_scrolling)
		type = mapdata_type::TemporaryScrollingRegion;
	else if (is_region && !is_scrolling)
		type = mapdata_type::TemporaryCurrentRegion;
	else if (!is_region && is_scrolling)
		type = mapdata_type::TemporaryScrollingScreen;
	else if (!is_region && !is_scrolling)
		type = mapdata_type::TemporaryCurrentScreen;

	return mapdata{type, base_scr, scr, screen, layer};
}

int create_mapdata_temp_ref(mapdata_type type, int screen, int layer)
{
	bool is_scrolling = type == mapdata_type::TemporaryScrollingScreen || type == mapdata_type::TemporaryScrollingRegion;
	bool is_region = type == mapdata_type::TemporaryScrollingRegion || type == mapdata_type::TemporaryCurrentRegion;

	int ref = 0;
	ref |= is_scrolling ? 1 : 0;
	ref |= is_region ? 2 : 0;
	if (!is_region)
		ref |= ((screen & 0xFF) << 8);
	ref |= ((layer & 0xFF) << 16);
	return -ref-1;
}

int32_t mapdata_get_register(int32_t reg)
{
	int32_t ret = 0;

	#define	GET_MAPDATA_VAR_INT32(member) \
	{ \
		if ( mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)) ) \
		{ \
			ret = (m->member *10000); \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define	GET_MAPDATA_VAR_INT16(member) \
	{ \
		if ( mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)) ) \
		{ \
			ret = (m->member *10000); \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define	GET_MAPDATA_VAR_BYTE(member) \
	{ \
		if ( mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)) ) \
		{ \
			ret = (m->member *10000); \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define GET_MAPDATA_FLAG(member) \
	{ \
		int32_t flag =  (value/10000);  \
		if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			ret = (m->member&flag) ? 10000 : 0); \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define GET_MAPDATA_FFCPOS_INDEX32(member, indexbound) \
	{ \
		int32_t index = (GET_D(rINDEX) / 10000); \
		if (auto handle = ResolveMapdataFFC(ri->mapdataref, index)) \
		{ \
			ret = (handle.ffc->member).getZLong(); \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define GET_MAPDATA_FFC_INDEX32(member, indexbound) \
	{ \
		int32_t index = (GET_D(rINDEX) / 10000); \
		if (auto handle = ResolveMapdataFFC(ri->mapdataref, index)) \
		{ \
			ret = (handle.ffc->member)*10000; \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	#define GET_MAPDATA_FFC_INDEX32(member, indexbound) \
	{ \
		int32_t index = (GET_D(rINDEX) / 10000); \
		if (auto handle = ResolveMapdataFFC(ri->mapdataref, index)) \
		{ \
			ret = (handle.ffc->member)*10000; \
		} \
		else \
		{ \
			ret = -10000; \
		} \
	} \

	switch (reg)
	{
		case MAPDATABOSSSFX: 		GET_MAPDATA_VAR_INT16(bosssfx); break;	//B
		case MAPDATACATCHALL:	 	GET_MAPDATA_VAR_INT32(catchall); break; //W
		case MAPDATACOLOUR: 		GET_MAPDATA_VAR_INT32(color); break;	//w
		case MAPDATACSENSITIVE: 	GET_MAPDATA_VAR_BYTE(csensitive); break;	//B
		case MAPDATADOORCOMBOSET: 	GET_MAPDATA_VAR_INT32(door_combo_set); break;	//w
		case MAPDATAENEMYFLAGS: 	GET_MAPDATA_VAR_BYTE(flags11);	break;	//b
		case MAPDATAENTRYX: 		GET_MAPDATA_VAR_BYTE(entry_x); break;	//B
		case MAPDATAENTRYY: 		GET_MAPDATA_VAR_BYTE(entry_y); break;	//B
		case MAPDATAEXDOOR:
		{
			ret = 0;
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int mi = get_mi(GET_REF(mapdataref));
				if(mi < 0) break;
				int dir = SH::read_stack(ri->sp+1) / 10000;
				int ind = SH::read_stack(ri->sp+0) / 10000;
				if(unsigned(dir) > 3)
					Z_scripterrlog("Invalid dir '%d' passed to 'mapdata->GetExDoor()'; must be 0-3\n", dir);
				else if(unsigned(ind) > 7)
					Z_scripterrlog("Invalid index '%d' passed to 'mapdata->GetExDoor()'; must be 0-7\n", ind);
				else
				{
					int bit = 1<<ind;
					ret = (game->xdoors.get(mi)[dir]&bit) ? 10000 : 0;
				}
			}
			break;
		}
		case MAPDATAEXITDIR: 		GET_MAPDATA_VAR_BYTE(exitdir); break;	//b
		case MAPDATAGUY: 		GET_MAPDATA_VAR_BYTE(guy); break;		//b
		case MAPDATAGUYCOUNT:
		{
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int mi = get_mi(GET_REF(mapdataref));
				if(mi > -1)
				{
					ret = game->guys.get(mi) * 10000;
					break;
				}
			}
			ret = -10000;
			break;
		}
		case MAPDATAHASITEM: 		GET_MAPDATA_VAR_BYTE(hasitem); break;	//b
		case MAPDATAHOLDUPSFX:	 	GET_MAPDATA_VAR_INT16(holdupsfx); break; //B
		case MAPDATAINTID: 	 //Same form as SetScreenD()
			//SetFFCInitD(ffindex, d, value)
		{
			int32_t index = (GET_D(rINDEX)/10000);
			int32_t d_index = GET_D(rINDEX2)/10000;

			if (BC::checkBounds(d_index, 0, 7) != SH::_NoError)
				break;

			if (auto handle = ResolveMapdataFFC(ri->mapdataref, index))
				ret = handle.ffc->scrconfig.run_args[d_index];
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATAITEM:
		{
			if ( mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)) )
			{
				if(m->hasitem)
					ret = (m->item *10000);
				else ret = -10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATAITEMX:		GET_MAPDATA_VAR_BYTE(itemx); break; //itemx
		case MAPDATAITEMY:		GET_MAPDATA_VAR_BYTE(itemy); break;	//itemy
		case MAPDATALENSLAYER:	 	GET_MAPDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?
		case MAPDATAMAP:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				ret = (m->map + 1) * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATANEXTMAP: 		GET_MAPDATA_VAR_BYTE(nextmap); break;	//B
		case MAPDATANEXTSCREEN: 	GET_MAPDATA_VAR_BYTE(nextscr); break;	//B
		case MAPDATANOCARRY: 		GET_MAPDATA_VAR_INT32(nocarry); break;	//W
		case MAPDATANORESET: 		GET_MAPDATA_VAR_INT32(noreset); break;	//W
		case MAPDATANUMFF: 	
		{
			int index = GET_D(rINDEX) / 10000;

			if (auto handle = ResolveMapdataFFC(ri->mapdataref, index))
			{
				ret = (handle.data() != 0) ? 10000 : 0;
			}
			else
			{
				ret = 0;
			}
			break;
		}
		case MAPDATAOCEANSFX:	 	GET_MAPDATA_VAR_INT16(oceansfx); break;	//B
		case MAPDATAPATTERN: 		GET_MAPDATA_VAR_BYTE(pattern); break;	//b
		case MAPDATAREGIONID:
		{
			if (auto scr = ResolveMapdataScr(GET_REF(mapdataref)))
				ret = get_region_id(scr->map, scr->screen) * 10000;
			break;
		}
		case MAPDATAROOM: 		GET_MAPDATA_VAR_BYTE(room);	break;		//b
		case MAPDATASCRDATASIZE:
		{
			ret = -10000;
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int index = get_ref_map_index(GET_REF(mapdataref));
				if (index < 0) break;

				ret = 10000*game->scriptDataSize(index);
			}
			break;
		}
		case MAPDATASCREEN:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				ret = m->screen * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATASCREENHEIGHT: 	break;//GET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case MAPDATASCREENMIDI:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				if (unsigned(m->music) > quest_music.size())
					ret = -40000; // old value for using dmap music
				else if (!m->music)
					ret = 0;
				else
				{
					auto const& amus = quest_music[m->music-1];
					if (amus.enhanced.is_empty())
						ret = convert_to_old_midi_id(amus.midi, true) * 10000;
					else ret = -10000; // error using outdated zasm with new features
				}
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case MAPDATASCREENWIDTH: 	break;//GET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case MAPDATASCRIPT: 		GET_MAPDATA_VAR_INT32(scrconfig.script); break;	//W
		case MAPDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
			ret = -10000;
		}
		break;
		case MAPDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
			ret = -10000;
		}
		break;
		case MAPDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
			ret = -10000;
		}
		break;
		case MAPDATASECRETSFX:	 	GET_MAPDATA_VAR_INT16(secretsfx); break;	//B
		case MAPDATASIDEWARPINDEX: 	GET_MAPDATA_VAR_BYTE(sidewarpindex); break;	//b
		case MAPDATASTAIRX: 		GET_MAPDATA_VAR_BYTE(stairx); break;	//b
		case MAPDATASTAIRY: 		GET_MAPDATA_VAR_BYTE(stairy); break;	//b
		case MAPDATASTRING:		GET_MAPDATA_VAR_INT32(str); break;		//w
		case MAPDATATIMEDWARPTICS: 	GET_MAPDATA_VAR_INT32(timedwarptics); break;	//W
		case MAPDATAUNDERCOMBO: 	GET_MAPDATA_VAR_INT32(undercombo); break;	//w
		case MAPDATAUNDERCSET:	 	GET_MAPDATA_VAR_BYTE(undercset); break; //b
		case MAPDATAVALID:		GET_MAPDATA_VAR_BYTE(valid); break;		//b
		case MAPDATAVIEWX: 		break;//GET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case MAPDATAVIEWY: 		break;//GET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case MAPDATAWARPARRIVALX: 	GET_MAPDATA_VAR_BYTE(warparrivalx); break;	//b
		case MAPDATAWARPARRIVALY: 	GET_MAPDATA_VAR_BYTE(warparrivaly); break;	//b
		case MAPDATAWARPRETURNC: 	GET_MAPDATA_VAR_INT32(warpreturnc); break;	//w
		case MAPDATA_GRAVITY_STRENGTH:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				ret = m->screen_gravity.getZLong();
			}
			else ret = -10000;
			break;
		}
		case MAPDATA_MUSIC:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				if (m->music < -1 || m->music > quest_music.size())
					ret = -1;
				else ret = m->music;
			}
			else ret = -1;
			break;
		}
		case MAPDATA_TERMINAL_VELOCITY:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				ret = m->screen_terminal_v.getZLong();
			}
			else ret = -10000;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void mapdata_set_register(int32_t reg, int32_t value)
{
	#define	SET_MAPDATA_VAR_INT32(member) \
	{ \
		if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member = vbound((value / 10000),-214747,214747); \
		} \
		break; \
	} \

	#define	SET_MAPDATA_VAR_INT16(member) \
	{ \
		if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member = vbound((value / 10000),0,32767); \
		} \
		break; \
	} \

	#define	SET_MAPDATA_VAR_BYTE(member) \
	{ \
		if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member = vbound((value / 10000),0,255); \
		} \
		break; \
	} \

	#define SET_MAPDATA_VAR_INDEX32(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
		{ \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx] = vbound((value / 10000),-214747,214747); \
		} \
		break; \
	} \

	#define SET_MAPDATA_VAR_INDEX16(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
		{ \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx] = vbound((value / 10000),-32767,32767); \
		} \
		break; \
	} \

	#define SET_MAPDATA_BYTE_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
		{ \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx] = vbound((value / 10000),0,255); \
		} \
		break; \
	}\

	#define SET_MAPDATA_LAYER_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
		if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
		{ \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx-1] = vbound((value / 10000),0,255); \
		} \
		break; \
	} \

	#define SET_MAPDATA_LAYERSCREEN_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
		int32_t scrn_id = value/10000; \
		if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
		{ \
		} \
		else if ( scrn_id > MAPSCRS ) \
		{ \
			Z_scripterrlog("Script attempted to use a mapdata->LayerScreen[%d].\n",scrn_id); \
			Z_scripterrlog("Valid Screen values are (0) through (%d).\n",MAPSCRS); \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx-1] = vbound((scrn_id),0,MAPSCRS); \
		} \
		break; \
	}\

	#define SET_MAPDATA_BOOL_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
		{ \
		} \
		else if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			m->member[indx] =( (value/10000) ? 1 : 0 ); \
		} \
		break; \
	} \


	#define SET_FFC_MAPDATA_BOOL_INDEX(member, indexbound) \
	{ \
		int32_t index = GET_D(rINDEX) / 10000; \
		if (auto handle = ResolveMapdataFFC(ri->mapdataref, index)) \
		{ \
			handle.ffc->member =( (value/10000) ? 1 : 0 ); \
		} \
		break; \
	} \

	#define SET_MAPDATA_FLAG(member) \
	{ \
		int32_t flag =  (value/10000);  \
		if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref))) \
		{ \
			if ( flag != 0 ) \
			{ \
				m->member|=flag; \
			} \
			else m->.member|= ~flag; \
		} \
		break; \
	} \

	switch (reg)
	{
		case MAPDATABOSSSFX: 		SET_MAPDATA_VAR_INT16(bosssfx); break;	//B
		case MAPDATACATCHALL:	 	SET_MAPDATA_VAR_INT32(catchall); break; //W
		case MAPDATACOLOUR: 		SET_MAPDATA_VAR_INT32(color); break;	//w
		case MAPDATACSENSITIVE: 	SET_MAPDATA_VAR_BYTE(csensitive); break;	//B
		case MAPDATADOORCOMBOSET: 	SET_MAPDATA_VAR_INT32(door_combo_set); break;	//w
		case MAPDATAENEMYFLAGS: 	SET_MAPDATA_VAR_BYTE(flags11);	break;	//b
		case MAPDATAENTRYX: 		SET_MAPDATA_VAR_BYTE(entry_x); break;	//B
		case MAPDATAENTRYY: 		SET_MAPDATA_VAR_BYTE(entry_y); break;	//B
		case MAPDATAEXDOOR:
		{
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int mi = get_mi(GET_REF(mapdataref));
				if(mi < 0) break;
				int dir = SH::read_stack(ri->sp+1) / 10000;
				int ind = SH::read_stack(ri->sp+0) / 10000;
				if(unsigned(dir) > 3)
					Z_scripterrlog("Invalid dir '%d' passed to 'mapdata->SetExDoor()'; must be 0-3\n", dir);
				else if(unsigned(ind) > 7)
					Z_scripterrlog("Invalid index '%d' passed to 'mapdata->SetExDoor()'; must be 0-7\n", ind);
				else
					set_xdoorstate_mi(mi, dir, ind);
			}
			break;
		}
		case MAPDATAEXITDIR: 		SET_MAPDATA_VAR_BYTE(exitdir); break;	//b
		case MAPDATAGUY: 		SET_MAPDATA_VAR_BYTE(guy); break;		//b
		case MAPDATAGUYCOUNT:
		{
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int mi = get_mi(GET_REF(mapdataref));
				if(mi > -1)
				{
					game->guys[mi] = vbound(value/10000,10,0);
					break;
				}
			}
			break;
		}
		case MAPDATAHASITEM: 		SET_MAPDATA_VAR_BYTE(hasitem); break;	//b
		case MAPDATAHOLDUPSFX:	 	SET_MAPDATA_VAR_INT16(holdupsfx); break; //B
		case MAPDATAINTID:
		{
			int32_t index = (GET_D(rINDEX)/10000);
			int32_t dindex = GET_D(rINDEX2)/10000;

			if (BC::checkBounds(dindex, 0, 7) != SH::_NoError)
				break;

			if (auto handle = ResolveMapdataFFC(ri->mapdataref, index))
				handle.ffc->scrconfig.run_args[dindex] = value;
			break;
		}	
		case MAPDATAITEM:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				auto v = vbound((value / 10000),-1,MAXITEMS-1);
				if(v > -1)
					m->item = v;
				m->hasitem = v > -1;
			}
			break;
		}
		case MAPDATAITEMX:		SET_MAPDATA_VAR_BYTE(itemx); break; //itemx
		case MAPDATAITEMY:		SET_MAPDATA_VAR_BYTE(itemy); break;	//itemy
		case MAPDATALENSLAYER:	 	SET_MAPDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?
		case MAPDATANEXTMAP: 		SET_MAPDATA_VAR_BYTE(nextmap); break;	//B
		case MAPDATANEXTSCREEN: 	SET_MAPDATA_VAR_BYTE(nextscr); break;	//B
		case MAPDATANOCARRY: 		SET_MAPDATA_VAR_INT32(nocarry); break;	//W
		case MAPDATANORESET: 		SET_MAPDATA_VAR_INT32(noreset); break;	//W
		case MAPDATANUMFF: 	
		{
			break;
		}
		case MAPDATAOCEANSFX:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int32_t v = vbound(value/10000, 0, MAX_SFX);
				if(m == hero_scr && m->oceansfx != v)
				{
					stop_sfx(m->oceansfx);
					m->oceansfx = v;
					cont_sfx(m->oceansfx);
				}
				else m->oceansfx = v;
			}
			break;
		}
		case MAPDATAPATTERN: 		SET_MAPDATA_VAR_BYTE(pattern); break;	//b
		case MAPDATAREGIONID:
		{
			int region_id = value / 10000;
			if (BC::checkBounds(region_id, 0, 9) != SH::_NoError)
				break;

			auto result = decode_mapdata_ref(GET_REF(mapdataref));
			if (result.scr)
			{
				if (result.type == mapdata_type::CanonicalScreen)
				{
					Regions[result.scr->map].set_region_id(result.screen, region_id);
				}
				else
				{
					scripting_log_error_with_context("This may only be set for canonical screens");
				}
			}
			else
			{
				scripting_log_error_with_context("mapdata pointer is either invalid or uninitialised");
			}
			break;
		}
		case MAPDATAROOM: 		SET_MAPDATA_VAR_BYTE(room);	break;		//b
		case MAPDATASCRDATASIZE:
		{
			if (ResolveMapdataScr(GET_REF(mapdataref)))
			{
				int index = get_ref_map_index(GET_REF(mapdataref));
				if (index < 0) break;

				game->scriptDataResize(index, value/10000);
			}
			break;
		}
		case MAPDATASCREENHEIGHT: 	break;//SET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
		case MAPDATASCREENMIDI:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				m->music = find_or_make_midi_music(convert_from_old_midi_id(vbound(value / 10000, MAXMIDIS-MIDIOFFSET_ZSCRIPT, -4), true));
			}
			break;
		}
		case MAPDATASCREENWIDTH: 	break;//SET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
		case MAPDATASCRIPT:
		{
			auto result = decode_mapdata_ref(GET_REF(mapdataref));
			if (result.scr)
			{
				if (result.current())
				{
					if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
						result.scr->scrconfig.run_args.fill(0);
					result.scr->scrconfig.inst_init.clear();

					on_reassign_script_engine_data(ScriptType::Screen, ri->screenref);
				}

				result.scr->scrconfig.script = vbound(value/10000, 0, NUMSCRIPTSCREEN-1);
			} 
			else 
			{ 
				Z_scripterrlog("Script attempted to use a mapdata->%s on an invalid pointer\n","Script");
			} 
			break;
		}
		case MAPDATASCRIPTENTRY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptEntry");
		}
		break;
		case MAPDATASCRIPTEXIT:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ExitScript");
		}
		break;
		case MAPDATASCRIPTOCCUPANCY:
		{
			Z_scripterrlog("Unimplemented: %s\n", "ScriptOccupancy");
		}
		break;
		case MAPDATASECRETSFX:	 	SET_MAPDATA_VAR_INT16(secretsfx); break;	//B
		case MAPDATASIDEWARPINDEX: 	SET_MAPDATA_VAR_BYTE(sidewarpindex); break;	//b
		case MAPDATASTAIRX: 		SET_MAPDATA_VAR_BYTE(stairx); break;	//b
		case MAPDATASTAIRY: 		SET_MAPDATA_VAR_BYTE(stairy); break;	//b
		case MAPDATASTRING:		SET_MAPDATA_VAR_INT32(str); break;		//w
		case MAPDATATIMEDWARPTICS: 	SET_MAPDATA_VAR_INT32(timedwarptics); break;	//W
		case MAPDATAUNDERCOMBO: 	SET_MAPDATA_VAR_INT32(undercombo); break;	//w
		case MAPDATAUNDERCSET:	 	SET_MAPDATA_VAR_BYTE(undercset); break; //b
		case MAPDATAVALID:		SET_MAPDATA_VAR_BYTE(valid); break;		//b
		case MAPDATAVIEWX: 		break;//SET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
		case MAPDATAVIEWY: 		break;//SET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
		case MAPDATAWARPARRIVALX: 	SET_MAPDATA_VAR_BYTE(warparrivalx); break;	//b
		case MAPDATAWARPARRIVALY: 	SET_MAPDATA_VAR_BYTE(warparrivaly); break;	//b
		case MAPDATAWARPRETURNC: 	SET_MAPDATA_VAR_INT32(warpreturnc); break;	//w
		case MAPDATA_GRAVITY_STRENGTH:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				m->screen_gravity = zslongToFix(value);
			}
			break;
		}
		case MAPDATA_MUSIC:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
				if (m->music != value && (value == -1 || value == 0 || checkMusic(value)))
				{
					m->music = value;
					if (engine_music_active && m == hero_scr)
						playLevelMusic();
				}
			break;
		}
		case MAPDATA_TERMINAL_VELOCITY:
		{
			if (mapscr *m = ResolveMapdataScr(GET_REF(mapdataref)))
			{
				m->screen_terminal_v = zslongToFix(value);
			}
			break;
		}

		default:
			NOTREACHED();
	}
}

// mapdata arrays.

static ArrayRegistrar MAPDATADOOR_registrar(MAPDATADOOR, []{
	static ScriptingArray_ObjectComputed<mapdata, byte> impl(
		[](mapdata*){ return 4; },
		[](mapdata* mapdata, int index) -> byte {
			return mapdata->scr->door[index];
		},
		[](mapdata* mapdata, int index, byte value){
			if (mapdata->current())
				putdoor(mapdata->scr, scrollbuf, index, value, true, true);
			else
				mapdata->scr->door[index] = value;
		}
	);
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
		[](mapdata*, int, int){}
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
	static ScriptingArray_ObjectSubMemberContainer<mapscr, &mapscr::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENSTATED_registrar(MAPDATASCREENSTATED, []{
	static ScriptingArray_ObjectComputed<mapdata, bool> impl(
		[](mapdata*){ return mMAXIND; },
		[](mapdata* mapdata, int index) -> bool {
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return false;

			return (game->maps.get(mi) >> index) & 1;
		},
		[](mapdata* mapdata, int index, bool value){
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return;
			
			if (value)
				setmapflag_mi(mi, 1 << index);
			else
				unsetmapflag_mi(mi, 1 << index);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATACOMBOPOSSTATE_registrar(MAPDATACOMBOPOSSTATE, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){
			return mapdata->max_pos() + 1;
		},
		[](mapdata* mapdata, int index) -> int {
			auto rpos_handle = mapdata->resolve_pos(index);
			if (!rpos_handle)
				return 0;
			int mi = mapind(rpos_handle.base_scr);
			if (mi < 0)
				return 0;

			return game->pos_states.get(mi).get(index % 176);
		},
		[](mapdata* mapdata, int index, int value){
			auto rpos_handle = mapdata->resolve_pos(index);
			if (!rpos_handle)
				return;
			int mi = mapind(rpos_handle.base_scr);
			if (mi < 0)
				return;
			
			game->pos_states[mi][index % 176] = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATAEXSTATED_registrar(MAPDATAEXSTATED, []{
	static ScriptingArray_ObjectComputed<mapdata, bool> impl(
		[](mapdata*){ return 32; },
		[](mapdata* mapdata, int index) -> bool {
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return false;

			return (game->xstates.get(mi) >> index) & 1;
		},
		[](mapdata* mapdata, int index, bool value){
			int mi = get_mi(*mapdata);
			if (mi < 0)
				return;
			
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
		[](mapscr*){ return 4; },
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

			return game->screen_data.get(mapindex).get(index);
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
		[](mapscr*){ return 4; },
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
		[](mapscr*){ return 4; },
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
		[](mapscr*){ return 11; },
		[](mapscr* scr, int index){ return (&scr->flags)[index]; },
		[](mapscr* scr, int index, byte value){ (&scr->flags)[index] = value; }
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENEFLAGSD_registrar(MAPDATASCREENEFLAGSD, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr*){ return 3; },
		[](mapscr* scr, int index){ return get_screeneflags(scr, index); },
		[](mapscr*, int, int){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar MAPDATASCREENFLAGSD_registrar(MAPDATASCREENFLAGSD, []{
	static ScriptingArray_ObjectComputed<mapscr, int> impl(
		[](mapscr*){ return 10; },
		[](mapscr* scr, int index){ return get_screenflags(scr, index); },
		[](mapscr*, int, int){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar MAPDATA_FLAG_registrar(MAPDATA_FLAG, []{
	static ScriptingArray_ObjectComputed<mapscr, bool> impl(
		[](mapscr*){ return 8*11; },
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
		[](mapdata*){ return 8; },
		[](mapdata* mapdata, int index) -> int {
			int mi = mapind(mapdata->scr->map, mapdata->scr->screen);
			if (mi < 0)
				return -1;

			return game->screen_d.get(mi).get(index);
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

static ArrayRegistrar MAPDATAFFCPOSSTATE_registrar(MAPDATAFFCPOSSTATE, []{
	static ScriptingArray_ObjectComputed<mapdata, int> impl(
		[](mapdata* mapdata){ return mapdata->scr->numFFC(); },
		[](mapdata* mapdata, int index) -> int {
			auto ffc_handle = resolve_ffc_handle_for_scripting_index(mapdata, index);
			if (!ffc_handle)
				return 0;
			
			int mi = mapind(ffc_handle.scr);
			if (mi < 0)
				return 0;
			index = (index - 1) % MAXFFCS;
			return game->ffcpos_states.get(mi).get(index);
		},
		[](mapdata* mapdata, int index, int value){
			auto ffc_handle = resolve_ffc_handle_for_scripting_index(mapdata, index);
			if (!ffc_handle)
				return;
			
			int mi = mapind(ffc_handle.scr);
			if (mi < 0)
				return;
			index = (index - 1) % MAXFFCS;
			game->ffcpos_states[mi][index] = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

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
				return ffc->scrconfig.script;

			return -1;
		},
		[](mapdata* mapdata, int index, int value){
			if (auto ffc = resolve_ffc_for_scripting_index(mapdata, index))
				ffc->scrconfig.script = value;
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
