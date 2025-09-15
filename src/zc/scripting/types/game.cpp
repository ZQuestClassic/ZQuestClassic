#include "base/dmap.h"
#include "base/general.h"
#include "base/handles.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
#include "base/msgstr.h"
#include "base/qrs.h"
#include "base/version.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/combos.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/hero.h"
#include "zc/maps.h"
#include "zc/scripting/arrays.h"
#include "zc/zelda.h"

#include <optional>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

// If `index` is currently being used as a layer for the origin screen, return that layer index.
int whichlayer(int map, int screen)
{
	for (int32_t i = 0; i < 6; i++)
	{
		if (map == origin_scr->layermap[i] - 1 && screen == origin_scr->layerscreen[i])
			return i;
	}

	return -1;
}

void do_getsavename()
{
	int32_t arrayptr = get_register(sarg1);
	
	if(ArrayH::setArray(arrayptr, string(game->get_name())) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetSaveName' not large enough\n");
}

void do_setsavename()
{
	int32_t arrayptr = get_register(sarg1);
	
	string str;
	ArrayH::getString(arrayptr, str);
	byte j;

	for(j = 0; str[j] != '\0'; j++)
	{
		if(j >= 8)
		{
			Z_scripterrlog("String supplied to 'Game->GetSaveName' too large\n");
			return;
		}
	}

	game->set_name(str);
}

void do_getmessage(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, MsgStrings[ID].s) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetMessage' not large enough\n");
}

void do_setmessage(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
	
	std::string text;
	ArrayH::getString(arrayptr, text, MSG_NEW_SIZE);
	MsgStrings[ID].setFromLegacyEncoding(text);
}

void do_getdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapName' not large enough\n");
}

void do_setdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);

	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 22);
	strncpy(DMaps[ID].name, filename_str.c_str(), 21);
	DMaps[ID].name[20]='\0';
}

void do_getdmaptitle(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
	{
		ArrayManager am(arrayptr);
		am.resize(DMaps[ID].title.size() + 1);
	}
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].title)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapTitle' not large enough\n");
}


void do_setdmaptitle(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if (get_qr(qr_OLD_DMAP_INTRO_STRINGS))
	{
		char namestr[21];
		ArrayH::getString(arrayptr, filename_str, 21);
		strncpy(namestr, filename_str.c_str(), 20);
		namestr[20] = '\0';
		DMaps[ID].title.assign(namestr);
	}
	else
	{
		ArrayH::getString(arrayptr, filename_str, ArrayH::getSize(arrayptr));
		DMaps[ID].title = filename_str;
	}
}

void do_getdmapintro(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}


void do_setdmapintro(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2);
	string filename_str;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 73);
	strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
	DMaps[ID].intro[72]='\0';
}

} // end namespace

// This is for the deprecated Game->GetComboData functions (and friends).
static int HandleGameScreenGetter(std::function<int(mapscr*, int)> cb, const char* context)
{
	int32_t pos = (ri->d[rINDEX])/10000;
	int32_t screen = (ri->d[rEXP1]/10000);
	int32_t map = (ri->d[rINDEX2]/10000)-1;
	int32_t index = zc_max(map_screen_index(map, screen), 0);
	int32_t layr = whichlayer(map, screen);

	if (pos < 0 || pos >= 176) 
	{
		Z_scripterrlog("Invalid combo position (%d) passed to %s\n", pos, context);
		return -10000;
	}
	else if (screen >= MAPSCRS)
	{
		Z_scripterrlog("Invalid Screen (%d) passed to %s\n", screen, context);
		return -10000;
	}
	else if (map >= map_count) 
	{
		Z_scripterrlog("Invalid Map (%d) passed to %s\n", map, context);
		return -10000;
	}
	else if (map < 0) return 0; // No layer present. [2025 note: weird...]
	else
	{
		// Since this is deprecated, we only support looking at the temporary for the origin screen.
		if (map == cur_map && screen == cur_screen)
			return cb(origin_scr, pos);
		else if (layr > -1)
			return cb(get_scr_layer(cur_screen, layr + 1), pos);
		else return cb(&TheMaps[index], pos);
	}
}

// This is for the deprecated SetComboData functions (and friends).
static auto ResolveGameScreens(const char* context)
{
	struct result_t
	{
		rpos_handle_t canonical_rpos_handle;
		rpos_handle_t tmp_rpos_handle;
		rpos_handle_t tmp_layer_rpos_handle;
	};
	result_t result{};

	int32_t pos = (ri->d[rINDEX])/10000;
	int32_t screen = (ri->d[rEXP1]/10000);
	int32_t map = (ri->d[rINDEX2]/10000)-1;
	int32_t index = zc_max(map_screen_index(map, screen), 0);
	int32_t layr = whichlayer(map, screen);

	if (pos < 0 || pos >= 176) 
	{
		Z_scripterrlog("Invalid combo position (%d) passed to %s", pos, context);
		return result;
	}
	if (screen >= MAPSCRS) 
	{
		Z_scripterrlog("Invalid Screen (%d) passed to %s", screen, context);
		return result;
	}
	if (unsigned(map) >= map_count) 
	{
		Z_scripterrlog("Invalid Map (%d) passed to %s", map, context);
		return result;
	}

	result.canonical_rpos_handle = {&TheMaps[index], screen, 0, (rpos_t)pos, pos};

	// Since this is deprecated, we only support looking at the temporary for the origin screen.
	if (map == cur_map && screen == cur_screen)
		result.tmp_rpos_handle = {origin_scr, screen, 0, (rpos_t)pos, pos};
	if (layr > -1)
		result.tmp_layer_rpos_handle = {get_scr_layer(cur_screen, layr + 1), screen, 0, (rpos_t)pos, pos};

	return result;
}

std::optional<int32_t> game_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case ZELDAVERSION:
		{
			auto version = getVersion();
			ret = version.major * 10000 + version.minor * 100 + version.patch; //Do *not* multiply by 10,000!
			break;
		}
		case ZELDABUILD:
			ret = (int32_t)VERSION_BUILD*10000;
			break;
		
		case ZSCRIPTVERSION: 
		{
			ret = (FFCore.quest_format[vLastCompile]) * 10000;
			break;
		}
		
		case ZELDABETATYPE:
		{
			ret = int32_t(getAlphaState()*10000);
			break;
		}
		case ZELDABETA:
		{
			int ver = 120;
			ret = int32_t(ver*10000);
			break;
		}
		case GAMEDEATHS:
			ret=game->get_deaths()*10000;
			break;
			
		case GAMECHEAT:
			ret=cheat*10000;
			break;
			
		case GAMEMAXCHEAT:
			ret=maxcheat*10000;
			break;
			
		case GAMETIME:
			ret=game->get_time();
			break;// Can't multiply by 10000 or the maximum result is too big
		case ACTIVESSSPEED:
			ret=Hero.subscr_speed*10000;
			break;// Can't multiply by 10000 or the maximum result is too big
			
		case GAMETIMEVALID:
			ret=game->get_timevalid()?10000:0;
			break;
			
		case GAMEHASPLAYED:
			ret=game->get_hasplayed()?10000:0;
			break;
		
		case TYPINGMODE:
			ret=FFCore.kb_typing_mode?10000:0;
			break;
		
		case SKIPCREDITS:
			ret=FFCore.skip_ending_credits?10000:0;
			break;
		
		case SKIPF6:
			ret=get_qr(qr_NOCONTINUE)?10000:0;
			break;
			
		case GAMESTANDALONE:
			ret=standalone_mode?10000:0;
			break;
		
		case GAMECONTSCR:
			ret=game->get_continue_scrn()*10000;
			break;
			
		case GAMECONTDMAP:
			ret=game->get_continue_dmap()*10000;
			break;
			
		case GAMEENTRSCR:
			ret=lastentrance*10000;
			break;
			
		case GAMEENTRDMAP:
			ret=lastentrance_dmap*10000;
			break;
		
		case GAMEMOUSECURSOR:
		{
			ret = game_mouse_index*10000;
			break;
		}

		case CURMAP:
			ret=(1+cur_map)*10000;
			break;
			
		case CURSCR:
			ret=cur_screen*10000;
			break;

		case HERO_SCREEN:
			ret=Hero.current_screen*10000;
			break;

		case ALLOCATEBITMAPR:
			ret=FFCore.get_free_bitmap();
			break;
			
		case GETMIDI:
			ret=(currmidi-(ZC_MIDI_COUNT-1))*10000;
			break;
			
		case CURDSCR:
		{
			int32_t di = (get_currscr()-DMaps[get_currdmap()].xoff);
			ret=(DMaps[get_currdmap()].type==dmOVERW ? cur_screen : di)*10000;
		}
		break;
		
		case GAMEMAXMAPS:
			ret = (map_count)*10000;
			break;
		case GAMENUMMESSAGES:
			ret = (msg_count-1) * 10000; 
			break;
		
		case CURDMAP:
			ret=cur_dmap*10000;
			break;
			
		case CURLEVEL:
			ret=DMaps[get_currdmap()].level*10000;
			break;
			
		case GAMECLICKFREEZE:
			ret=disableClickToFreeze?0:10000;
			break;

		case NOACTIVESUBSC:
			ret=Hero.stopSubscreenFalling()?10000:0;
			break;

		case COMBODDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return scr->data[pos] * 10000; }, "Game->GetComboData");
		}
		break;
		
		case COMBOCDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return scr->cset[pos] * 10000; }, "Game->GetComboCSet");
		}
		break;
		
		case COMBOFDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return scr->sflag[pos] * 10000; }, "Game->GetComboFlag");
		}
		break;
		
		case COMBOTDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return combobuf[scr->data[pos]].type * 10000; }, "Game->GetComboType");
		}
		break;
		
		case COMBOIDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return combobuf[scr->data[pos]].flag * 10000; }, "Game->GetComboInherentFlag");
		}
		break;
		
		case COMBOSDM:
		{
			ret = HandleGameScreenGetter([](mapscr* scr, int pos) { return (combobuf[scr->data[pos]].walk&15) * 10000; }, "Game->GetComboSolid");
		}
		break;

		case SCREENSTATEDD:
		{
			// Gah! >:(  Screen state is stored in game->maps, which uses 128 screens per map,
			// but the compiler multiplies the map number by 136, so it has to be corrected here.
			// Yeah, the compiler could be fixed, but that wouldn't cover existing quests...
			int32_t mi = ri->d[rINDEX] / 10000;
			mi -= 8*((ri->d[rINDEX] / 10000) / MAPSCRS);
			
			if(BC::checkMapID(mi>>7) == SH::_NoError)
				ret=(game->maps[mi] >> (ri->d[rINDEX2] / 10000) & 1) ? 10000 : 0;
			else
				ret=0;
				
			break;
		}

		default: return std::nullopt;
	}

	return ret;
}

bool game_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case GAMEDEATHS:
			game->set_deaths(value/10000);
			break;
			
		case GAMECHEAT:
			cheat=vbound(value/10000,0,4);
			if(maxcheat < cheat) maxcheat = cheat;
			if(cheat) game->did_cheat(true);
			break;
		
		case GAMEMAXCHEAT:
			maxcheat=vbound(value/10000,0,4);
			game->set_cheat(maxcheat);
			if(cheat > maxcheat) cheat = maxcheat;
			break;
			
		case GAMETIME:
			game->set_time(value);
			break; // Can't multiply by 10000 or the maximum result is too big
		
		case ACTIVESSSPEED:
			Hero.subscr_speed = vbound((value/10000),1,85);
			break; // Can't multiply by 10000 or the maximum result is too big
			
		case GAMETIMEVALID:
			game->set_timevalid((value/10000)?1:0);
			break;
			
		case GAMEHASPLAYED:
			game->set_hasplayed((value/10000)?1:0);
			break;
		
		case TYPINGMODE:
			FFCore.kb_typing_mode = ((value/10000)?true:false);
			break;
		
		case SKIPCREDITS:
			FFCore.skip_ending_credits = ((value/10000)?true:false);
			break;
		
		case SKIPF6:
			set_qr(qr_NOCONTINUE,((value/10000)?1:0));
			break;
		
		case GAMECONTSCR:
			game->set_continue_scrn(value/10000);
			break;
			
		case GAMECONTDMAP:
			game->set_continue_dmap(value/10000);
			break;
			
		case GAMEENTRSCR:
			lastentrance=value/10000;
			break;
			
		case GAMEENTRDMAP:
			lastentrance_dmap=value/10000;
			break;
		
		case GAMEMOUSECURSOR:
		{
			int v = value/10000;
			if(v < 0 || v >= ZCM_MAX)
				break;
			game_mouse_index = v;
			game_mouse();
			break;
		}

		case GAMECLICKFREEZE:
			disableClickToFreeze=value==0;
			break;

		case COMBODDM:
		{
			value = vbound(value/10000,0,MAXCOMBOS);
			auto result = ResolveGameScreens("Game->SetComboData");
			if (result.canonical_rpos_handle)
				result.canonical_rpos_handle.set_data(value);
			if (result.tmp_rpos_handle)
			{
				screen_combo_modify_preroutine(result.tmp_rpos_handle);
				result.tmp_rpos_handle.set_data(value);
				screen_combo_modify_postroutine(result.tmp_rpos_handle);
				//Start the script for the new combo
				FFCore.reset_script_engine_data(ScriptType::Combo, get_combopos_ref(result.tmp_rpos_handle));
				//Not ure if combodata arrays clean themselves up, or leak. -Z
				//Not sure if this could result in stack corruption. 
			}
			// TODO: shouldn't this be running screen_combo_modify_preroutine/screen_combo_modify_postroutine?
			if (result.tmp_layer_rpos_handle)
				result.tmp_layer_rpos_handle.set_data(value);
		}
		break;
		
		case COMBOCDM:
		{
			value = (value/10000)&15;
			auto result = ResolveGameScreens("Game->SetComboCSet");
			if (result.canonical_rpos_handle)
				result.canonical_rpos_handle.set_cset(value);
			// NOTE: should probably run screen_combo_modify_preroutine/screen_combo_modify_postroutine like other cset setters,
			// but i think it doesn't matter yet b/c those things don't ever look at cset.
			if (result.tmp_rpos_handle)
				result.tmp_rpos_handle.set_cset(value);
			if (result.tmp_layer_rpos_handle)
				result.tmp_layer_rpos_handle.set_cset(value);
		}
		break;
		
		case COMBOFDM:
		{
			value = value / 10000;
			auto result = ResolveGameScreens("Game->SetComboFlag");
			if (result.canonical_rpos_handle)
				result.canonical_rpos_handle.set_sflag(value);
			if (result.tmp_rpos_handle)
				result.tmp_rpos_handle.set_sflag(value);
			if (result.tmp_layer_rpos_handle)
				result.tmp_layer_rpos_handle.set_sflag(value);
		}
		break;
		
		case COMBOTDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t screen = zc_max(m*MAPSCRS+sc,0);
						    
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to SetComboType", pos);
				break;
			}
			if(screen < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboType", screen);
				break;
			}
			if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboType", sc);
				break;
			}
			if(unsigned(m) >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to SetComboType", m);
				break;
			}
				
			int32_t cdata = TheMaps[screen].data[pos];
			screen_combo_modify_pre(cdata);
			combobuf[cdata].type=value/10000;
			screen_combo_modify_post(cdata);
		}
		break;
		
		case COMBOIDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t screen = zc_max(m*MAPSCRS+sc,0);
						    
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboInherentFlag", pos);
				break;
			}
			if(screen < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboInherentFlag", screen);
				break;
			}
			if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboInherentFlag", sc);
				break;
			}
			if(unsigned(m) >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboInherentFlag", m);
				break;
			}
				
			combobuf[TheMaps[screen].data[pos]].flag=value/10000;
		}
		break;
		
		case COMBOSDM:
		{
			//This is how it was in 2.50.1-2
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t screen = (ri->d[rINDEX2]/10000)*MAPSCRS+(ri->d[rEXP1]/10000);
			//This (below) us the precise code from 2.50.1 (?)
			//int32_t screen = zc_max((ri->d[rINDEX2]/10000)*MAPSCRS+(ri->d[rEXP1]/10000),0); //Not below 0. 

			//if(pos < 0 || pos >= 176 || scr < 0) break;
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetSolid", pos);
				break;
			}
			if(screen < 0) 
			{
				Z_scripterrlog("Invalid MapScreen ID (%d) passed to GetSolid", screen);
				break;
			}
			combobuf[TheMaps[screen].data[pos]].walk &= ~0x0F;
			combobuf[TheMaps[screen].data[pos]].walk |= (value/10000)&15;	    
		}
		break;

		case SCREENSTATEDD:
		{
			int32_t mi = ri->d[rINDEX]/10000;
			mi -= 8*(mi/MAPSCRS);
			
			if(BC::checkMapID(mi>>7) == SH::_NoError)
			{
				if (value)
					setmapflag_mi(mi, 1<<(ri->d[rINDEX2]/10000));
				else
					unsetmapflag_mi(mi, 1 << (ri->d[rINDEX2] / 10000), true);
			}
		}
		break;

		default: return false;
	}

	return true;
}

std::optional<int32_t> game_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case LOADSTACK:
			FFCore.do_loadstack(); break;
		case LOADDROPSETR:
			FFCore.do_loaddropset(false); break;
		case LOADBOTTLETYPE:
			FFCore.do_loadbottle(false); break;
		case LOADBSHOPDATA:
			FFCore.do_loadbottleshop(false); break;
		case LOADDMAPDATAR:
			FFScript::do_loaddmapdata(false); break;
		case LOADDMAPDATAV:
			FFScript::do_loaddmapdata(true); break;
		case LOADSUBDATARV:
			FFScript::do_load_subscreendata(false, true); break;

		case SWAPSUBSCREENV:
		{
			auto ty = sarg1/10000;
			std::vector<ZCSubscreen>* vec = nullptr;
			switch(ty)
			{
				case sstACTIVE:
					vec = &subscreens_active;
					break;
				case sstPASSIVE:
					vec = &subscreens_passive;
					break;
				case sstOVERLAY:
					vec = &subscreens_overlay;
					break;
				default:
					Z_scripterrlog("Invalid Subscreen Type passed to ???: %d\n", ty);
					break;
			}
			if(vec)
			{
				auto& v = *vec;
				int p1 = SH::read_stack(ri->sp+1);
				int p2 = SH::read_stack(ri->sp+0);
				if(unsigned(p1) >= v.size())
					Z_scripterrlog("Invalid susbcr index '%d' passed to subscreendata->Swap*Pages()\n", p1);
				else if(unsigned(p2) >= v.size())
					Z_scripterrlog("Invalid susbcr index '%d' passed to subscreendata->Swap*Pages()\n", p2);
				else zc_swap(v[p1],v[p2]);
			}
			break;
		}

		case GETSAVENAME:
			do_getsavename();
			break;
		case SETSAVENAME:
			do_setsavename();
			break;
		case GETMESSAGE:
			do_getmessage(false);
			break;
		case SETMESSAGE:
			do_setmessage(false);
			break;
			
		case GETDMAPNAME:
			do_getdmapname(false);
			break;
			
		case GETDMAPTITLE:
			do_getdmaptitle(false);
			break;
			
		case GETDMAPINTRO:
			do_getdmapintro(false);
			break;
			
		case SETDMAPNAME:
			do_setdmapname(false);
			break;

		case SETDMAPTITLE:
			do_setdmaptitle(false);
			break;
		
		case SETDMAPINTRO:
			do_setdmapintro(false);
			break;

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

// Game arrays.

static ArrayRegistrar GAMEBOTTLEST_registrar(GAMEBOTTLEST, []{
	static ScriptingArray_ObjectMemberCArray<gamedata, &gamedata::bottleSlots> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMECOUNTERD_registrar(GAMECOUNTERD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return MAX_COUNTERS; },
		[](int, int index) -> int {
			return game->get_counter(index);
		},
		[](int, int index, int value) {
			game->set_counter(value, index);
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEDCOUNTERD_registrar(GAMEDCOUNTERD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return MAX_COUNTERS; },
		[](int, int index) -> int {
			return game->get_dcounter(index);
		},
		[](int, int index, int value) {
			game->set_dcounter(value, index);
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEEVENTDATA_registrar(GAMEEVENTDATA, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return FFCore.eventData.size(); },
		[](int, int index) -> int {
			return FFCore.eventData[index];
		},
		[](int, int index, int value) {
			FFCore.eventData[index] = value;
			return true;
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar GAMEGENERICD_registrar(GAMEGENERICD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return genMAX; },
		[](int ref, int index) {
			switch (index)
			{
				case genCONTHP:
				{
					if(!get_qr(qr_SCRIPT_CONTHP_IS_HEARTS) || game->get_cont_percent())
						return game->get_generic(index);
					else
						return (game->get_generic(index)/game->get_hp_per_heart());
				}
				default:
					return game->get_generic(index);
			}
		},
		[](int ref, int index, int value) {
			switch (index)
			{
				case genCONTHP:
				{
					if (!get_qr(qr_SCRIPT_CONTHP_IS_HEARTS) || game->get_cont_percent())
						game->set_generic(value, index);
					else
						game->set_generic(value*game->get_hp_per_heart(), index);
					break;
				}
				default:
					game->set_generic(value, index);
					break;
			}

			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEGSWITCH_registrar(GAMEGSWITCH, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return NUM_GSWITCHES; },
		[](int, int index) -> int {
			return game->gswitch_timers[index];
		},
		[](int, int index, int value) {
			bool old = game->gswitch_timers[index];
			game->gswitch_timers[index] = value;
			if (old != bool(value) && !get_qr(qr_OLD_SCRIPT_LEVEL_GLOBAL_STATES))
				toggle_gswitches(index, false);
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEGUYCOUNT_registrar(GAMEGUYCOUNT, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return MAPSCRSNORMAL; },
		[](int ref, int index) -> int { // index is screen number for current map
			int mi = mapind(cur_map, index);
			return game->guys[mi];
		},
		[](int ref, int index, int value) {
			int mi = mapind(cur_map, index);
			game->guys[mi] = value;
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMELITEMSD_registrar(GAMELITEMSD, []{
	static ScriptingArray_ObjectMemberContainer<gamedata, &gamedata::lvlitems> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMELKEYSD_registrar(GAMELKEYSD, []{
	static ScriptingArray_ObjectMemberContainer<gamedata, &gamedata::lvlkeys> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMELSWITCH_registrar(GAMELSWITCH, []{
	static ScriptingArray_GlobalComputed<dword> impl(
		[](int) { return game->lvlswitches.size(); },
		[](int, int index) -> dword {
			return game->lvlswitches[index];
		},
		[](int, int index, dword value) {
			auto old = game->lvlswitches[index];
			game->lvlswitches[index] = value;
			if (index == dlevel && !get_qr(qr_OLD_SCRIPT_LEVEL_GLOBAL_STATES))
				toggle_switches(old ^ value, false);
			return true;
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar GAMEOVERRIDEITEMS_registrar(GAMEOVERRIDEITEMS, []{
	static ScriptingArray_ObjectMemberContainer<gamedata, &gamedata::OverrideItems> impl;
	impl.compatSetDefaultValue(-20000);
	impl.setMul10000(true);
	impl.setValueTransform([](int value){ return value < -1 || value >= MAXITEMS ? -2 : value; });
	return &impl;
}());

static ArrayRegistrar GAMEMCOUNTERD_registrar(GAMEMCOUNTERD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return MAX_COUNTERS; },
		[](int ref, int index) -> int {
			return game->get_maxcounter(index);
		},
		[](int ref, int index, int value) {
			game->set_maxcounter(value, index);
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEMISCSFX_registrar(GAMEMISCSFX, []{
	static ScriptingArray_GlobalCArray impl(QMisc.miscsfx, comptime_array_size(QMisc.miscsfx));
	impl.compatSetDefaultValue(-10000);
	impl.setValueTransform(transforms::vboundByte);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMEMISCSPR_registrar(GAMEMISCSPR, []{
	static ScriptingArray_GlobalCArray impl(QMisc.sprites, comptime_array_size(QMisc.sprites));
	impl.compatSetDefaultValue(-10000);
	impl.setValueTransform(transforms::vboundByte);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMESCROLLING_registrar(GAMESCROLLING, []{
	static ScriptingArray_GlobalCArray impl(FFCore.ScrollingData, comptime_array_size(FFCore.ScrollingData));
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar GAMESUSPEND_registrar(GAMESUSPEND, []{
	static ScriptingArray_GlobalCArray impl(FFCore.system_suspend, comptime_array_size(FFCore.system_suspend));
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAMETRIGGROUPS_registrar(GAMETRIGGROUPS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return 256; },
		[](int, int index) {
			return cpos_trig_group_count(index);
		},
		[](int, int index, int value) { return false; }
	);
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar DMAPMAP_registrar(DMAPMAP, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].map + 1;
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar DMAPOFFSET_registrar(DMAPOFFSET, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].xoff;
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar DMAPFLAGSD_registrar(DMAPFLAGSD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].flags;
		},
		[](int, int index, int value){
			DMaps[index].flags = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPLEVELD_registrar(DMAPLEVELD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].level;
		},
		[](int, int index, int value){
			DMaps[index].level = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPCOMPASSD_registrar(DMAPCOMPASSD, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].compass;
		},
		[](int, int index, int value){
			DMaps[index].compass = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPCONTINUED_registrar(DMAPCONTINUED, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].cont;
		},
		[](int, int index, int value){
			DMaps[index].cont = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPLEVELPAL_registrar(DMAPLEVELPAL, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			return DMaps[index].color;
		},
		[](int, int index, int value){
			DMaps[index].color = value;
			if (index == cur_dmap)
			{
				loadlvlpal(value);
				currcset = value;
			}
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x1FF>);
	return &impl;
}());

static ArrayRegistrar DISABLEDITEM_registrar(DISABLEDITEM, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int){ return MAXITEMS; },
		[](int, int index) -> bool {
			return game->items_off[index];
		},
		[](int, int index, bool value){
			game->items_off[index] = value;
			removeFromItemCache(itemsbuf[index].type);
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DMAPMIDID_registrar(DMAPMIDID, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return MAXDMAPS; },
		[](int, int index) -> int {
			// Based on play_DmapMusic
			switch (DMaps[index].midi)
			{
				case 2: return -6; // Dungeon
				case 3: return -3; // Level 9
				case 1: return -2; // Overworld
				case 0: return 0; // None
				default: return DMaps[index].midi - 3;
			}
		},
		[](int, int index, int value){
			switch (DMaps[index].midi)
			{
				case -6: DMaps[index].midi = 2; // Dungeon
				case -3: DMaps[index].midi = 3; // Level 9
				case -2: DMaps[index].midi = 1; // Overworld
				case 0: DMaps[index].midi = 0; // None
				default: DMaps[index].midi = value + 3;
			}
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SETGAMEOVERELEMENT_registrar(SETGAMEOVERELEMENT, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return SAVESC_LAST; },
		[](int, int index) -> int {
			scripting_log_error_with_context("This array is write-only");
			return 0;
		},
		[](int, int index, int value){
			SetSaveScreenSetting(index, value);
			return true;
		}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar SETGAMEOVERSTRING_registrar(SETGAMEOVERSTRING, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return SAVESC_LAST; },
		[](int, int index) -> int {
			scripting_log_error_with_context("This array is write-only");
			return 0;
		},
		[](int, int index, int value){
			string filename_str;
			ArrayH::getString(value, filename_str, 73);
			ChangeSubscreenText(index, filename_str.c_str());
			return true;
		}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar FFRULE_registrar(FFRULE, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return qr_MAX; },
		[](int, int index) {
			return get_qr(index);
		},
		[](int, int index, bool value) {
			set_qr(index, value);
			apply_qr_rule(index);
			return true;
		}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar GAMEGRAVITY_registrar(GAMEGRAVITY, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return 4; },
		[](int, int index) {
			switch (index)
			{
				case 0: //Gravity Strength
					return zinit.gravity;
				case 1: //Terminal Velocity
					return zinit.terminalv * 100;
				case 2: //Sprite Layer Threshold
					return zinit.jump_hero_layer_threshold * 10000;
				case 3: //Air Drag
					return zinit.air_drag.getZLong();
				default: NOTREACHED();
			}
		},
		[](int, int index, int value) {
			switch (index)
			{
				case 0: //Gravity Strength
					zinit.gravity = value;
					break;
				case 1: //Terminal Velocity
					zinit.terminalv = value / 100;
					break;
				case 2: //Sprite Layer Threshold
					zinit.jump_hero_layer_threshold = value / 10000;
					break;
				case 3: //Air Drag
					zinit.air_drag = zslongToFix(value);
					break;
				default: NOTREACHED();
			}

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar GAMELAYERZTHRESHOLDS_registrar(GAMELAYERZTHRESHOLDS, []{
	static ScriptingArray_GlobalCArray impl(zinit.sprite_z_thresholds, comptime_array_size(zinit.sprite_z_thresholds));
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GAME_SAVED_PORTALS_registrar(GAME_SAVED_PORTALS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return game->user_portals.size(); },
		[](int, int index) -> int {
			return game->user_portals[index].getUID();
		},
		[](int, int index, int value){
			return false;
		}
	);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());
