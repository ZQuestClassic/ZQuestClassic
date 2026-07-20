#include "zc/scripting/types/screendata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/rendertarget.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"
#include "zc/zc_sys.h"
#include "zc/zscriptversion.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t screendata_get_register(int32_t reg)
{
	switch (reg)
	{
		case SCREENDATANUMFF:
		{
			int id = GET_D(rINDEX) / 10000;
			if (auto ffc = ResolveFFCWithID(id))
				return ffc->data != 0 ? 10000 : 0;
			return 0;
		}
		case SHOWNMSG:
			return ((msgstr::active || msgstr::on_screen) ? msgstr::active_str : 0) * 10000L;
		case WAVY:
			return wavy*10000;
	}

	int32_t ret = 0;
	mapscr* scr = get_scr(GET_REF(screenref));

	#define	GET_SCREENDATA_VAR_INT32(member) \
	{ \
		ret = (scr->member *10000); \
	} \

	#define	GET_SCREENDATA_VAR_INT16(member) \
	{ \
		ret = (scr->member *10000); \
	} \

	#define	GET_SCREENDATA_VAR_BYTE(member) \
	{ \
		ret = (scr->member *10000); \
	} \

	#define GET_SCREENDATA_BYTE_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		ret = (scr->member[indx] *10000); \
	} \

	//byte
	#define GET_SCREENDATA_LAYER_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (scr->member[indx-1] *10000); \
		} \
	} \

	#define GET_SCREENDATA_BOOL_INDEX(member, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if (BC::checkIndex(indx, 0, indexbound) != SH::_NoError) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (scr->member[indx]?10000:0); \
		} \
	} \


	#define GET_SCREENDATA_FLAG(member, str, indexbound) \
	{ \
		int32_t flag =  (value/10000);  \
		ret = (scr->member&flag) ? 10000 : 0); \
	} \

	switch (reg)
	{
		case CREATELWPNDX:
		{
			//Z_message("Trying to get Hero->SetExtend().\n");
			int32_t ID = (GET_D(rINDEX) / 10000);
			int32_t itemid = (GET_D(rINDEX2)/10000);
			itemid = vbound(itemid,0,(MAXITEMS-1));

			// TODO: use has_space()
			if ( Lwpns.Count() < 256 )
			{

				(void)Lwpns.add
				(
					new weapon
					(
						(zfix)0, /*X*/
						(zfix)0, /*Y*/
						(zfix)0, /*Z*/
						ID,	 /*id*/
						0,	 /*type*/
						0,	 /*power*/
						0,	 /*dir*/
						-1,	 /*Parentid*/
						Hero.getUID(), /*prntid*/
						false,	 /*isdummy*/
						1,	 /*script_gen*/
						1,  /*islwpn*/
						(ID==wWind?1:0)  /*special*/
					)
				);
				ri->lwpnref = Lwpns.spr(Lwpns.Count() - 1)->getUID();

				weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
				//w->LOADGFX(FFCore.getDefWeaponSprite(ID)); //What the fuck Zoria, this broke old quests...
				w->ScriptGenerated = 1;
				w->isLWeapon = 1;
				if(ID == wWind) w->specialinfo = 1;
				//weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
				//w->LOADGFX(FFCore.getDefWeaponSprite(ID)); //not needed here because this has access to wpn->prent
			}
			else
			{
				Z_scripterrlog("Tried to create too many LWeapons on the screen. The current LWeapon count is: %d\n", Lwpns.Count());
				ri->lwpnref = 0;
			}

			ret = ri->lwpnref; 
		}
		break;
		case EWPNCOUNT:
			ret=Ewpns.Count()*10000;
			break;
		case GETRENDERTARGET:
			ret=(zscriptDrawingRenderTarget->GetCurrentRenderTarget())*10000;
			break;
		case LIT:
			ret= get_lights() ? 10000 : 0;
			break;
		case LWPNCOUNT:
			ret=Lwpns.Count()*10000;
			break;
		case NPCCOUNT:
			ret = guys.Count()*10000;
			break;
		case PORTALCOUNT:
		{
			ret = portals.Count()*10000;
			break;
		}
		case PUSHBLOCKCOMBO:
			ret = mblock2.bcombo*10000;
			break;
		case PUSHBLOCKCSET:
			ret = mblock2.cs*10000;
			break;
		case PUSHBLOCKLAYER:
			ret = mblock2.active() ? int32_t(mblock2.blockLayer)*10000 : -10000;
			break;
		case PUSHBLOCKX:
			ret = mblock2.active() ? int32_t(mblock2.x)*10000 : -10000;
			break;
		case PUSHBLOCKY:
			ret = mblock2.active() ? int32_t(mblock2.y)*10000 : -10000;
			break;
		case QUAKE:
			ret = quakeclk*10000;
			break;
		case REGION_NUM_COMBOS:
		{
			ret = region_num_rpos * 10000;
		}
		break;
		case ROOMDATA:
			ret = scr->catchall*10000;
			break;
		case ROOMTYPE:
			ret = scr->room*10000;
			break;
		case SCREENDATABOSSSFX: 		GET_SCREENDATA_VAR_INT16(bosssfx); break;	//B
		case SCREENDATACATCHALL:	 	GET_SCREENDATA_VAR_INT32(catchall); break; //W
		case SCREENDATACOLOUR: 		GET_SCREENDATA_VAR_INT32(color); break;	//w
		case SCREENDATACSENSITIVE: 	GET_SCREENDATA_VAR_BYTE(csensitive); break;	//B
		case SCREENDATADOOR: 		GET_SCREENDATA_BYTE_INDEX(door, 3); break;	//b, 4 of these
		case SCREENDATADOORCOMBOSET: 	GET_SCREENDATA_VAR_INT32(door_combo_set); break;	//w
		case SCREENDATAENEMYFLAGS: 	GET_SCREENDATA_VAR_BYTE(flags11);	break;	//b
		case SCREENDATAENTRYX: 		GET_SCREENDATA_VAR_BYTE(entry_x); break;	//B
		case SCREENDATAENTRYY: 		GET_SCREENDATA_VAR_BYTE(entry_y); break;	//B
		case SCREENDATAEXDOOR:
		{
			ret = 0;
			int mi = mapind(cur_map, GET_REF(screenref));
			if(mi < 0) break;
			int dir = SH::read_stack(ri->sp+1) / 10000;
			int ind = SH::read_stack(ri->sp+0) / 10000;
			if(unsigned(dir) > 3)
				Z_scripterrlog("Invalid dir '%d' passed to 'Screen->GetExDoor()'; must be 0-3\n", dir);
			else if(unsigned(ind) > 7)
				Z_scripterrlog("Invalid index '%d' passed to 'Screen->GetExDoor()'; must be 0-7\n", ind);
			else
			{
				int bit = 1<<ind;
				ret = (game->xdoors.get(mi)[dir]&bit) ? 10000 : 0;
			}
			break;
		}
		case SCREENDATAEXITDIR: 		GET_SCREENDATA_VAR_BYTE(exitdir); break;	//b
		case SCREENDATAFFINITIALISED: 	{
			int32_t indx = GET_D(rINDEX) / 10000;
			if (indx < 0 || indx > MAX_FFCID)
			{
				scripting_log_error_with_context("Invalid index: %d", (indx));
				ret = -10000;
			}
			else
			{
				ret = get_script_engine_data(ScriptType::FFC, indx).initialized ? 10000 : 0;
			}
		}
		break;
		case SCREENDATAGUY: 		GET_SCREENDATA_VAR_BYTE(guy); break;		//b
		case SCREENDATAGUYCOUNT:
		{
			int mi = mapind(cur_map, GET_REF(screenref));
			if(mi < 0)
				ret = -10000;
			else ret = game->guys.get(mi) * 10000;
			break;
		}
		case SCREENDATAHASITEM: 		GET_SCREENDATA_VAR_BYTE(hasitem); break;	//b
		case SCREENDATAHOLDUPSFX:	 	GET_SCREENDATA_VAR_INT16(holdupsfx); break; //B
		case SCREENDATAITEM:
		{
			if(scr->hasitem)
				ret = (scr->item *10000);
			else ret = -10000;
			break;
		}
		case SCREENDATAITEMX:		GET_SCREENDATA_VAR_BYTE(itemx); break; //itemx
		case SCREENDATAITEMY:		GET_SCREENDATA_VAR_BYTE(itemy); break;	//itemy
		case SCREENDATALENSLAYER:	 	GET_SCREENDATA_VAR_BYTE(lens_layer); break;	//B, OLD QUESTS ONLY?
		case SCREENDATANEXTMAP: 		GET_SCREENDATA_VAR_BYTE(nextmap); break;	//B
		case SCREENDATANEXTSCREEN: 	GET_SCREENDATA_VAR_BYTE(nextscr); break;	//B
		case SCREENDATANOCARRY: 		GET_SCREENDATA_VAR_INT32(nocarry); break;	//W
		case SCREENDATANORESET: 		GET_SCREENDATA_VAR_INT32(noreset); break;	//W
		case SCREENDATAOCEANSFX:	 	GET_SCREENDATA_VAR_INT16(oceansfx); break;	//B
		case SCREENDATAPATTERN: 		GET_SCREENDATA_VAR_BYTE(pattern); break;	//b
		case SCREENDATAROOM: 		GET_SCREENDATA_VAR_BYTE(room);	break;		//b
		case SCREENDATASCREENMIDI:
		{
			if (unsigned(scr->music) > quest_music.size())
				ret = -40000; // old value for using dmap music
			else if (!scr->music)
				ret = 0;
			else
			{
				auto const& amus = quest_music[scr->music-1];
				if (amus.enhanced.is_empty())
					ret = convert_to_old_midi_id(amus.midi, true) * 10000;
				else ret = -10000; // error using outdated zasm with new features
			}
			break;
		}
		case SCREENDATASECRETSFX:	 	GET_SCREENDATA_VAR_INT16(secretsfx); break;	//B
		case SCREENDATASIDEWARPINDEX: 	GET_SCREENDATA_VAR_BYTE(sidewarpindex); break;	//b
		case SCREENDATASTAIRX: 		GET_SCREENDATA_VAR_BYTE(stairx); break;	//b
		case SCREENDATASTAIRY: 		GET_SCREENDATA_VAR_BYTE(stairy); break;	//b
		case SCREENDATASTRING:		GET_SCREENDATA_VAR_INT32(str); break;		//w
		case SCREENDATATIMEDWARPTICS: 	GET_SCREENDATA_VAR_INT32(timedwarptics); break;	//W
		case SCREENDATAVALID:		GET_SCREENDATA_VAR_BYTE(valid); break;		//b
		case SCREENDATAWARPARRIVALX: 	GET_SCREENDATA_VAR_BYTE(warparrivalx); break;	//b
		case SCREENDATAWARPARRIVALY: 	GET_SCREENDATA_VAR_BYTE(warparrivaly); break;	//b
		case SCREENDATAWARPRETURNC: 	GET_SCREENDATA_VAR_INT32(warpreturnc); break;	//w
		case SCREENDATA_GRAVITY_STRENGTH:
		{
			ret = scr->screen_gravity.getZLong();
			break;
		}
		case SCREENDATA_MUSIC:
		{
			if (scr->music < -1 || scr->music > quest_music.size())
				ret = -1;
			else ret = scr->music;
			break;
		}
		case SCREENDATA_TERMINAL_VELOCITY:
		{
			ret = scr->screen_terminal_v.getZLong();
			break;
		}
		case SCREENSCRDATASIZE:
		{
			int index = map_screen_index(cur_map, GET_REF(screenref));
			if (index < 0) break;
			ret = 10000*game->scriptDataSize(index);
			break;
		}
		case SCREENSCRIPT:
			ret=scr->scrconfig.script*10000;
			break;
		case SCREENSECRETSTRIGGERED:
		{
			ret = get_screen_state(GET_REF(screenref)).triggered_secrets ? 10000L : 0L;
			break;
		}
		case SCREEN_DRAW_ORIGIN:
			ret = (int)ri->screen_draw_origin;
			break;
		case SCREEN_DRAW_ORIGIN_TARGET:
			ret = ri->screen_draw_origin_target;
			break;
		case SCREEN_INDEX:
			ret = ri->screenref*10000;
			break;
		case UNDERCOMBO:
			ret = scr->undercombo*10000;
			break;
		case UNDERCSET:
			ret = scr->undercset*10000;
			break;
		

		default:
			NOTREACHED();
	}

	return ret;
}

void screendata_set_register(int32_t reg, int32_t value)
{
	mapscr* scr = get_scr(GET_REF(screenref));

	#define	SET_SCREENDATA_VAR_INT32(member, str) \
	{ \
		scr->member = vbound((value / 10000),-214747,214747); \
	} \

	#define	SET_SCREENDATA_VAR_INT16(member, str) \
	{ \
		scr->member = vbound((value / 10000),0,32767); \
	} \

	#define	SET_SCREENDATA_VAR_BYTE(member, str) \
	{ \
		scr->member = vbound((value / 10000),0,255); \
	} \

	#define SET_SCREENDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		scr->member[indx] = vbound((value / 10000),0,255); \
	}

	///max screen id is higher! vbound properly... -Z
	#define SET_SCREENDATA_LAYERSCREEN_INDEX(member, str, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		int32_t scrn_id = value/10000; \
		if ( FFCore.quest_format[vFFScript] < 11 ) ++indx; \
		if (BC::checkIndex(indx, 1, indexbound) != SH::_NoError) \
		{ \
		} \
		else if ( scrn_id > MAPSCRS ) \
		{ \
			Z_scripterrlog("Script attempted to use a mapdata->LayerScreen[%d].\n",scrn_id); \
			Z_scripterrlog("Valid Screen values are (0) through (%d).\n",MAPSCRS); \
		} \
		else scr->member[indx-1] = vbound((scrn_id),0,MAPSCRS); \
	}

	#define SET_SCREENDATA_FLAG(member, str) \
	{ \
		int32_t flag =  (value/10000);  \
		if ( flag != 0 ) \
		{ \
			scr->member|=flag; \
		} \
		else scr->.member|= ~flag; \
	} \

	#define SET_SCREENDATA_BOOL_INDEX(member, str, indexbound) \
	{ \
		int32_t indx = GET_D(rINDEX) / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", (indx), str); \
			break; \
		} \
		scr->member[indx] =( (value/10000) ? 1 : 0 ); \
	}


	switch (reg)
	{
		case LIT:
			set_lights(value);
			break;
		case PUSHBLOCKCOMBO:
			mblock2.bcombo=value/10000;
			break;
		case PUSHBLOCKCSET:
			mblock2.cs=value/10000;
			mblock2.oldcset=value/10000;
			break;
		case PUSHBLOCKLAYER:
			mblock2.blockLayer=vbound(value/10000, 0, 6);
			break;
		case QUAKE:
			quakeclk=value/10000;
			break;
		case ROOMDATA:
			scr->catchall=value/10000;
			break;
		case ROOMTYPE:
			scr->room=value/10000; break; //this probably doesn't work too well...
		case SCREENDATABOSSSFX: 		SET_SCREENDATA_VAR_INT16(bosssfx, "BossSFX"); break;	//B
		case SCREENDATACATCHALL:	 	SET_SCREENDATA_VAR_INT32(catchall,	"Catchall"); break; //W
		case SCREENDATACOLOUR: 		SET_SCREENDATA_VAR_INT32(color, "CSet"); break;	//w
		case SCREENDATACSENSITIVE: 	SET_SCREENDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
		case SCREENDATADOOR: 		SET_SCREENDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
		case SCREENDATADOORCOMBOSET: 	SET_SCREENDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
		case SCREENDATAENEMYFLAGS: 	SET_SCREENDATA_VAR_BYTE(flags11, "EnemyFlags");	break;	//b
		case SCREENDATAENTRYX: 		
		{
			int32_t newx = vbound((value/10000),0,255);
			scr->entry_x = newx;
			if ( get_qr(qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS) )
			{
				Hero.respawn_x = (zfix)(newx);
			}
			break;
		}
		case SCREENDATAENTRYY: 		
		{

			int32_t newy = vbound((value/10000),0,175);
			scr->entry_y = newy;
			if ( get_qr(qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS) )
			{
				Hero.respawn_y = (zfix)(newy);
			}
			break;	//B
		}
		case SCREENDATAEXDOOR:
		{
			int mi = mapind(cur_map, GET_REF(screenref));
			if(mi < 0) break;
			int dir = SH::read_stack(ri->sp+1) / 10000;
			int ind = SH::read_stack(ri->sp+0) / 10000;
			if(unsigned(dir) > 3)
				Z_scripterrlog("Invalid dir '%d' passed to 'Screen->SetExDoor()'; must be 0-3\n", dir);
			else if(unsigned(ind) > 7)
				Z_scripterrlog("Invalid index '%d' passed to 'Screen->SetExDoor()'; must be 0-7\n", ind);
			else
				set_xdoorstate_mi(mi, dir, ind);
			break;
		}
		case SCREENDATAEXITDIR: 		SET_SCREENDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
		case SCREENDATAFFINITIALISED:
		{
			int32_t indx = GET_D(rINDEX) / 10000;
			if (indx < 0 || indx > MAX_FFCID)
			{
				Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", "FFCRunning", (indx));
				break;
			}
			get_script_engine_data(ScriptType::FFC, indx).initialized = (value/10000) ? true : false;
		}
		break;
		case SCREENDATAGUY: 		SET_SCREENDATA_VAR_BYTE(guy, "Guy"); break;		//b
		case SCREENDATAGUYCOUNT:
		{
			int mi = mapind(cur_map, GET_REF(screenref));
			if(mi > -1)
				game->guys[mi] = vbound(value/10000,10,0);
			break;
		}
		case SCREENDATAHASITEM: 		SET_SCREENDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
		case SCREENDATAHOLDUPSFX:	 	SET_SCREENDATA_VAR_INT16(holdupsfx,	"ItemSFX"); break; //B
		case SCREENDATAITEM:
		{
			auto v = vbound((value / 10000),-1,MAXITEMS-1);
			auto scr = get_scr(GET_REF(screenref));
			if(v > -1)
				scr->item = v;
			scr->hasitem = v > -1;
			break;
		}
		case SCREENDATAITEMX:		SET_SCREENDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
		case SCREENDATAITEMY:		SET_SCREENDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
		case SCREENDATALENSLAYER:	 	SET_SCREENDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
		case SCREENDATANEXTMAP: 		SET_SCREENDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
		case SCREENDATANEXTSCREEN: 	SET_SCREENDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
		case SCREENDATANOCARRY: 		SET_SCREENDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
		case SCREENDATANORESET: 		SET_SCREENDATA_VAR_INT32(noreset, "NoReset"); break;	//W
		case SCREENDATANUMFF: 	
		{
			break;
		}
		case SCREENDATAOCEANSFX:
		{
			int32_t v = vbound(value/10000, 0, MAX_SFX);
			auto scr = get_scr(GET_REF(screenref));
			if (scr == hero_scr && scr->oceansfx != v)
			{
				stop_sfx(scr->oceansfx);
				scr->oceansfx = v;
				cont_sfx(scr->oceansfx);
			}
			break;
		}
		case SCREENDATAPATTERN: 		SET_SCREENDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
		case SCREENDATAROOM: 		SET_SCREENDATA_VAR_BYTE(room, "RoomType");	break;		//b
		case SCREENDATASCREENMIDI:
		{
			scr->music = find_or_make_midi_music(convert_from_old_midi_id(vbound(value / 10000, MAXMIDIS-MIDIOFFSET_ZSCRIPT, -4), true));
			break;
		}
		case SCREENDATASECRETSFX:	 	SET_SCREENDATA_VAR_INT16(secretsfx, "SecretSFX"); break;	//B
		case SCREENDATASIDEWARPINDEX: 	SET_SCREENDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
		case SCREENDATASTAIRX: 		SET_SCREENDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
		case SCREENDATASTAIRY: 		SET_SCREENDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
		case SCREENDATASTRING:		SET_SCREENDATA_VAR_INT32(str, "String"); break;		//w
		case SCREENDATATIMEDWARPTICS: 	SET_SCREENDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
		case SCREENDATAVALID:
		{
			SET_SCREENDATA_VAR_BYTE(valid, "Valid"); //b
			mark_current_region_handles_dirty();
			break;
		}
		case SCREENDATAWARPARRIVALX: 	SET_SCREENDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
		case SCREENDATAWARPARRIVALY: 	SET_SCREENDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
		case SCREENDATAWARPRETURNC: 	SET_SCREENDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
		case SCREENDATA_GRAVITY_STRENGTH:
		{
			scr->screen_gravity = zslongToFix(value);
			break;
		}
		case SCREENDATA_MUSIC:
		{
			if (scr->music != value && (value == -1 || value == 0 || checkMusic(value)))
			{
				scr->music = value;
				if (engine_music_active && scr == hero_scr)
					playLevelMusic();
			}
			break;
		}
		case SCREENDATA_TERMINAL_VELOCITY:
		{
			scr->screen_terminal_v = zslongToFix(value);
			break;
		}
		case SCREENSCRDATASIZE:
		{
			int index = map_screen_index(cur_map, GET_REF(screenref));
			if (index < 0) break;

			game->scriptDataResize(index, value/10000);
			break;
		}
		case SCREENSCRIPT:
		{
			if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				scr->scrconfig.run_args.fill(0);
			scr->scrconfig.inst_init.clear();
			scr->scrconfig.script=vbound(value/10000, 0, NUMSCRIPTSCREEN-1);
			on_reassign_script_engine_data(ScriptType::Screen, ri->screenref);
			break;
		}
		case SCREEN_DRAW_ORIGIN:
			if (BC::checkBounds(value, (int)DrawOrigin::First, (int)DrawOrigin::Last) != SH::_NoError)
				break;

			ri->screen_draw_origin = (DrawOrigin)value;
			break;
		case SCREEN_DRAW_ORIGIN_TARGET:
		{
			if (ResolveBaseSprite(value))
				ri->screen_draw_origin_target = value;

			break;
		}
		case UNDERCOMBO:
			scr->undercombo=value/10000;
			break;
		case UNDERCSET:
			scr->undercset=value/10000;
			break;
		case WAVY:
			wavy=value/10000;
			break;

		default:
			NOTREACHED();
	}
}

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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
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
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			auto& cmb = rpos_handle.combo();
			cmb.walk &= ~0xF0;
			cmb.walk |= value << 4;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
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
		[](int, int index, int value) {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			auto& cmb = rpos_handle.combo();
			cmb.walk &= ~0x0F;
			cmb.walk |= value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
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
		[](int, int, int) {
			return false;
		}
	);
	impl.compatSetDefaultValue(-1);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCRDOORD_registrar(SCRDOORD, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::door> impl;
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		putdoor(scr, scrollbuf, index, value, true, true);
	});
	return &impl;
}());

static ArrayRegistrar SCREEN_FLAG_registrar(SCREEN_FLAG, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata*){ return 8 * 11; },
		[](screendata* scr, int index) -> bool {
			return (&scr->flags)[index/8] & (1 << (index%8));
		},
		[](screendata* scr, int index, bool value){
			byte& flag = (&scr->flags)[index/8];
			SETFLAG(flag, 1 << (index%8), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASWARPRETSQR_registrar(SCREENDATASWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata*){ return 4; },
		[](screendata* scr, int index){ return (scr->warpreturnc >> (8+(index*2))) & 3; },
		[](screendata* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(8+(index*2)))) | (value<<(8+(index*2)));
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar SCREENDATATWARPRETSQR_registrar(SCREENDATATWARPRETSQR, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata*){ return 4; },
		[](screendata* scr, int index){ return (scr->warpreturnc >> (index*2)) & 3; },
		[](screendata* scr, int index, int value){
			scr->warpreturnc = (scr->warpreturnc&~(3<<(index*2))) | (value<<(index*2));
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar SCREENDATAFLAGS_registrar(SCREENDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<screendata, byte> impl(
		[](screendata*){ return 11; },
		[](screendata* scr, int index){ return (&scr->flags)[index]; },
		[](screendata* scr, int index, byte value){ (&scr->flags)[index] = value; }
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENEFLAGSD_registrar(SCREENEFLAGSD, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata*){ return 3; },
		[](screendata* scr, int index){ return get_screeneflags(scr, index); },
		[](screendata*, int, int){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCREENFLAGSD_registrar(SCREENFLAGSD, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata*){ return 10; },
		[](screendata* scr, int index){ return get_screenflags(scr, index); },
		[](screendata*, int, int){}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar SCREENDATAENEMY_registrar(SCREENDATAENEMY, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::enemy> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::validate<MAXGUYS>);
	return &impl;
}());

static ArrayRegistrar SCREENDATANORESETARR_registrar(SCREENDATANORESETARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::noreset, mMAXIND> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATANOCARRYARR_registrar(SCREENDATANOCARRYARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::nocarry, mMAXIND> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATAEXRESET_registrar(SCREENDATAEXRESET, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::exstate_reset, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATAEXCARRY_registrar(SCREENDATAEXCARRY, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::exstate_carry, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATALAYERINVIS_registrar(SCREENDATALAYERINVIS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::hidelayers, 7> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASCRIPTDRAWS_registrar(SCREENDATASCRIPTDRAWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::hidescriptlayers, 7> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPOVFLAGS_registrar(SCREENDATASIDEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::sidewarpoverlayflags, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPOVFLAGS_registrar(SCREENDATATILEWARPOVFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::tilewarpoverlayflags, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENLENSHIDES_registrar(SCREENLENSHIDES, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::lens_hide, 7> impl;
	impl.setMul10000(true);
	impl.setSideEffect([](auto scr, int index, int value) {
		if (value) scr->lens_show &= ~(1<<index);
	});
	return &impl;
}());

static ArrayRegistrar SCREENLENSSHOWS_registrar(SCREENLENSSHOWS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<screendata, &screendata::lens_show, 7> impl;
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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
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
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAPATH_registrar(SCREENDATAPATH, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::path> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPSC_registrar(SCREENDATASIDEWARPSC, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarpscr> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPDMAP_registrar(SCREENDATASIDEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarpdmap> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETCOMBO_registrar(SCREENDATASECRETCOMBO, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretcombo> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETCSET_registrar(SCREENDATASECRETCSET, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretcset> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASECRETFLAG_registrar(SCREENDATASECRETFLAG, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::secretflag> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATASIDEWARPTYPE_registrar(SCREENDATASIDEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::sidewarptype> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPSCREEN_registrar(SCREENDATATILEWARPSCREEN, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarpscr> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPTYPE_registrar(SCREENDATATILEWARPTYPE, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarptype> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAWARPRETX_registrar(SCREENDATAWARPRETX, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::warpreturnx> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATAWARPRETY_registrar(SCREENDATAWARPRETY, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::warpreturny> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SCREENDATATILEWARPDMAP_registrar(SCREENDATATILEWARPDMAP, []{
	static ScriptingArray_ObjectMemberCArray<screendata, &screendata::tilewarpdmap> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundInt);
	return &impl;
}());

static ArrayRegistrar SCREENINITD_registrar(SCREENINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<screendata, &screendata::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SCREENSTATED_registrar(SCREENSTATED, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata*){ return mMAXIND; },
		[](screendata* scr, int index) -> bool {
			int mi = mapind(scr);
			if (mi < 0)
				return false;

			return (game->maps.get(mi) >> index) & 1;
		},
		[](screendata* scr, int index, bool value){
			int mi = mapind(scr);
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

static ArrayRegistrar SCREENCOMBOPOSSTATE_registrar(SCREENCOMBOPOSSTATE, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return region_num_rpos; },
		[](int, int index) -> int {
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			if (!rpos_handle)
				return 0;
			
			int mi = mapind(rpos_handle.base_scr);
			if (mi < 0)
				return 0;

			return game->pos_states.get(mi).get(index % 176);
		},
		[](int, int index, int value){
			auto rpos = (rpos_t)index;
			auto rpos_handle = get_rpos_handle(rpos, 0);
			if (!rpos_handle)
				return false;
			
			int mi = mapind(rpos_handle.base_scr);
			if (mi < 0)
				return false;
			
			game->pos_states[mi][index % 176] = value;
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SCREENEXSTATED_registrar(SCREENEXSTATED, []{
	static ScriptingArray_ObjectComputed<screendata, bool> impl(
		[](screendata*){ return 32; },
		[](screendata* scr, int index) -> bool {
			int mi = mapind(scr);
			if (mi < 0)
				return false;

			return (game->xstates.get(mi) >> index) & 1;
		},
		[](screendata* scr, int index, bool value){
			int mi = mapind(scr);
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

static ArrayRegistrar SCREENSIDEWARPID_registrar(SCREENSIDEWARPID, []{
	static ScriptingArray_ObjectComputed<screendata, int> impl(
		[](screendata*){ return 4; },
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
	impl.compatSetDefaultValue(-10000);
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

			return game->screen_data.get(mapindex).get(index);
		},
		[](int ref, int index, int value){
			int mapindex = map_screen_index(cur_map, ref);
			if (mapindex < 0) return false;

			game->screen_data[mapindex][index] = value;
			return true;
		}
	);
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
		[](int, int, int){
			return false;
		}
	);
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
		[](int, int, int){
			return false;
		}
	);
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
		[](int, int, int){
			return false;
		}
	);
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
		[](int, int, int){
			return false;
		}
	);
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
		[](int, int, int){
			return false;
		}
	);
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
			if (auto ffc = ResolveFFCWithID(index))
			{
				if (ZScriptVersion::ffcRefIsSpriteId())
					return ffc->getUID();

				return index * 10000;
			}

			return 0;
		},
		[](int, int, int){
			return false;
		}
	);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());
