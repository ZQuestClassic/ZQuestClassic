#include "base/dmap.h"
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
	int32_t arrayptr = get_register(sarg1) / 10000;
	
	if(ArrayH::setArray(arrayptr, string(game->get_name())) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetSaveName' not large enough\n");
}

void do_setsavename()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	
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
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, MsgStrings[ID].s) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetMessage' not large enough\n");
}

void do_setmessage(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkMessage(ID) != SH::_NoError)
		return;
	
	std::string text;
	ArrayH::getString(arrayptr, text, MSG_NEW_SIZE);
	MsgStrings[ID].setFromLegacyEncoding(text);
}

void do_getdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapName' not large enough\n");
}

void do_setdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;

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
	int32_t arrayptr = get_register(sarg2) / 10000;
	
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
	int32_t arrayptr = get_register(sarg2) / 10000;
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
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkDMapID(ID) != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}


void do_setdmapintro(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
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
		Z_scripterrlog("Invalid combo position (%d) passed to %s", pos, context);
		return -10000;
	}
	else if (screen >= MAPSCRS)
	{
		Z_scripterrlog("Invalid Screen (%d) passed to %s", screen, context);
		return -10000;
	}
	else if (map >= map_count) 
	{
		Z_scripterrlog("Invalid Map (%d) passed to %s", map, context);
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
			
		case GAMEGUYCOUNT:
		{
			int mi = mapind(cur_map, ri->d[rINDEX]/10000);
			ret=game->guys[mi]*10000;
		}
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
			
		case GAMECOUNTERD:
			ret=game->get_counter((ri->d[rINDEX])/10000)*10000;
			break;
			
		case GAMEMCOUNTERD:
			ret=game->get_maxcounter((ri->d[rINDEX])/10000)*10000;
			break;
			
		case GAMEDCOUNTERD:
			ret=game->get_dcounter((ri->d[rINDEX])/10000)*10000;
			break;
			
		case GAMEGENERICD:
		{
			auto indx = ri->d[rINDEX] / 10000;
			switch(indx)
			{
				case genCONTHP:
				{
					if(!get_qr(qr_SCRIPT_CONTHP_IS_HEARTS) || game->get_cont_percent())
						ret = game->get_generic(indx)*10000;
					else
						ret = (game->get_generic(indx)/game->get_hp_per_heart())*10000;
					break;
				}
				default:
					ret = game->get_generic(indx)*10000;
					break;
			}
			break;
		}
		
		case GAMEMISC:
		{
			int32_t indx = ri->d[rINDEX]/10000;
			if ( indx < 0 || indx > 31 )
			{
				ret = -10000;
				scripting_log_error_with_context("Invalid index: {}", indx);
			}
			else
			{
				ret = QMisc.questmisc[indx]*((get_qr(qr_OLDQUESTMISC)) ? 10000 : 1);
			}
			break;
		}
			
		case GAMEITEMSD:
			ret=(game->item[(ri->d[rINDEX])/10000] ? 10000 : 0);
			break;
		case DISABLEDITEM:
			ret = (game->items_off[(ri->d[rINDEX])/10000] ? 10000 : 0);
			break;
		case GAMESUSPEND:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( (unsigned) inx > (susptLAST-1) )
			{
				Z_scripterrlog("Invalid array index [%d] passed to Game->Suspend[]\n");
				break;
			}
			ret = (( FFCore.system_suspend[inx] ) ? 10000 : 0);
			break;
		}
		case GAMELITEMSD:
		{
			size_t index = ri->d[rINDEX] / 10000;
			if (index >= game->lvlitems.size())
			{
				ret = 0;
				Z_scripterrlog("Invalid array index [%d] passed to Game->LItems[]\n", index);
				break;
			}
			ret=game->lvlitems[index]*10000;
			break;
		}
		case GAMELSWITCH:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) >= MAXLEVELS)
				ret = 0;
			else ret=game->lvlswitches[ind];
			break;
		}
		case GAMEGSWITCH:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) >= NUM_GSWITCHES)
				ret = 0;
			else ret=game->gswitch_timers[ind]*10000;
			break;
		}
		case GAMEBOTTLEST:
			ret=game->get_bottle_slot((ri->d[rINDEX])/10000)*10000;
			break;
			
		case GAMELKEYSD:
			ret=game->lvlkeys[(ri->d[rINDEX])/10000]*10000;
			break;
		
		case GAMEMISCSPR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sprMAX )
			{
				scripting_log_error_with_context("Invalid index: {}", inx);
				ret = -10000;
			}
			else
			{
				ret = QMisc.sprites[inx] * 10000;
			}
			break;
		}
		case GAMEMISCSFX:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sfxMAX )
			{
				scripting_log_error_with_context("Invalid index: {}", inx);
				ret = -10000;
			}
			else
			{
				ret = QMisc.miscsfx[inx] * 10000;
			}
			break;
		}
		case GAMEOVERRIDEITEMS:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) >= itype_max)
			{
				scripting_log_error_with_context("Invalid index: {}", ind);
				ret = -20000;
			}
			else ret = game->OverrideItems[ind] * 10000;
			break;
		}
		case GAMEEVENTDATA:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			ret = 0;
			if ( ((unsigned)inx) < FFCore.eventData.size() )
			{
				ret = FFCore.eventData[inx];
			}
			break;
		}
		case GAMEMOUSECURSOR:
		{
			ret = game_mouse_index*10000;
			break;
		}
		case GAMETRIGGROUPS:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind)>255)
				scripting_log_error_with_context("Invalid index: {}", ind);
			ret = cpos_trig_group_count(ind)*10000;
			break;
		}
		
		case GAMEGRAVITY:
		{
			int32_t indx = ri->d[rINDEX]/10000;
			if ( ((unsigned)indx) > 3 )
			//if(indx < 0 || indx > 2)
			{
				ret = -10000;
				scripting_log_error_with_context("Invalid index: {}", indx);
			}
			else
			{
				switch(indx)
				{
					case 0: //Gravity Strength
						ret = zinit.gravity;
						break;
					case 1: //Terminal Velocity
						ret = zinit.terminalv * 100;
						break;
					case 2: //Sprite Layer Threshold
						ret = zinit.jump_hero_layer_threshold * 10000;
						break;
					case 3: //Air Drag
						ret = zinit.air_drag.getZLong();
						break;
				}
			}
			break;
		}
		
		case GAMESCROLLING:
		{
			int32_t indx = ri->d[rINDEX]/10000;
			if ( ((unsigned)indx) >= SZ_SCROLLDATA )
			{
				scripting_log_error_with_context("Invalid index: {}", indx);
			}
			else
			{
				ret = FFCore.ScrollingData[indx] * 10000L;
			}
			break;
		}
			
		case CURMAP:
			ret=(1+cur_map)*10000;
			break;
			
		case CURSCR:
			ret=cur_screen*10000;
			break;

		case HERO_SCREEN:
			ret=hero_screen*10000;
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

		#define GET_DMAP_VAR(member) \
		{ \
			int32_t ID = ri->d[rINDEX] / 10000; \
			if(BC::checkDMapID(ID) != SH::_NoError) \
				ret = -10000; \
			else \
				ret = DMaps[ID].member * 10000; \
		}

		case DMAPFLAGSD:
			GET_DMAP_VAR(flags)    break;
			
		case DMAPLEVELD:
			GET_DMAP_VAR(level)    break;
			
		case DMAPCOMPASSD:
			GET_DMAP_VAR(compass)  break;
			
		case DMAPCONTINUED:
			GET_DMAP_VAR(cont) break;
		
		case DMAPLEVELPAL:
			GET_DMAP_VAR(color)    break; 
			
		case DMAPOFFSET:
			GET_DMAP_VAR(xoff)   break;
			
		case DMAPMAP:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = (DMaps[ID].map+1) * 10000;
				
			break;
		}
		
		case DMAPMIDID:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID) == SH::_NoError)
			{
				// Based on play_DmapMusic
				switch(DMaps[ID].midi)
				{
				case 2:
					ret = -60000;
					break; // Dungeon
					
				case 3:
					ret = -30000;
					break; // Level 9
					
				case 1:
					ret = -20000;
					break; // Overworld
					
				case 0:
					ret = 0;
					break; // None
					
				default:
					ret = (DMaps[ID].midi - 3) * 10000;
				}
			}
			else
				ret = -10000; // Which is valid, but whatever.
				
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
		
		
			
		case GAMEGUYCOUNT:
		{
			int mi = mapind(cur_map, ri->d[rINDEX]/10000);
			game->guys[mi]=value/10000;
		}
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
			
		case GAMECOUNTERD:
			game->set_counter(value/10000, (ri->d[rINDEX])/10000);
			break;
			
		case GAMEMCOUNTERD:
			game->set_maxcounter(value/10000, (ri->d[rINDEX])/10000);
			break;
			
		case GAMEDCOUNTERD:
			game->set_dcounter(value/10000, (ri->d[rINDEX])/10000);
			break;
			
		case GAMEGENERICD:
		{
			auto indx = ri->d[rINDEX] / 10000;
			auto val = value/10000;
			switch(indx)
			{
				case genCONTHP:
				{
					if(!get_qr(qr_SCRIPT_CONTHP_IS_HEARTS) || game->get_cont_percent())
						game->set_generic(val, indx);
					else
						game->set_generic(val*game->get_hp_per_heart(), indx);
					break;
				}
				default:
					game->set_generic(val, indx);
					break;
			}
			break;
		}
		case GAMEMISC:
		{
			int32_t indx = ri->d[rINDEX]/10000;
			if ( indx < 0 || indx > 31 )
			{
				scripting_log_error_with_context("Invalid index: {}", indx);
			}
			else 
			{
				QMisc.questmisc[indx] = (value/((get_qr(qr_OLDQUESTMISC)) ? 10000 : 1));
			}
			break;
		}
		case GAMEITEMSD:
			game->set_item((ri->d[rINDEX])/10000,(value!=0));
			break;
		
		case DISABLEDITEM:
		{
			int id = (ri->d[rINDEX])/10000;
			if(unsigned(id) >= MAXITEMS)
				break;
			game->items_off[id]=value/10000;
			removeFromItemCache(itemsbuf[id].family);
			break;
		}
		
		case GAMESUSPEND:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( (unsigned) inx > (susptLAST-1) )
			{
				Z_scripterrlog("Invalid array index [%d] passed to Game->Suspend[]\n");
				break;
			}
			FFCore.system_suspend[inx]= ( (value) ? 1 : 0 );
			break;
		}
			
		case GAMELITEMSD:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) < MAXLEVELS)
				game->lvlitems[ind]=value/10000;
			break;
		}
		case GAMELSWITCH:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) < MAXLEVELS)
				game->lvlswitches[ind]=value;
			break;
		}
		case GAMEGSWITCH:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) < NUM_GSWITCHES)
				game->gswitch_timers[ind]=value/10000;
			break;
		}
		case GAMEBOTTLEST:
			game->set_bottle_slot((ri->d[rINDEX])/10000,value/10000);
			break;
		
		case GAMEMISCSPR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sprMAX )
			{
				scripting_log_error_with_context("Invalid index: {}", inx);
			}
			else
			{
				QMisc.sprites[inx] = vbound(value/10000, 0, 255);
			}
			break;
		}
		case GAMEMISCSFX:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sfxMAX )
			{
				scripting_log_error_with_context("Invalid index: {}", inx);
			}
			else
			{
				QMisc.miscsfx[inx] = vbound(value/10000, 0, 255);
			}
			break;
		}
		case GAMEOVERRIDEITEMS:
		{
			int32_t ind = (ri->d[rINDEX])/10000;
			if(unsigned(ind) >= itype_max)
			{
				scripting_log_error_with_context("Invalid index: {}", ind);
			}
			else
			{
				auto val = value/10000;
				game->OverrideItems[ind] = (val < -1 || val >= MAXITEMS) ? -2 : val;
			}
			break;
		}
		case GAMEEVENTDATA:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) < FFCore.eventData.size() )
			{
				FFCore.eventData[inx] = value;
			}
			break;
		}
		case GAMEMOUSECURSOR:
		{
			int v = value/10000;
			if(v < 0 || v >= ZCM_MAX)
				break;
			game_mouse_index = v;
			game_mouse();
			break;
		}
		case GAMETRIGGROUPS:
			break; //read-only
		
		case GAMELKEYSD:
			game->lvlkeys[(ri->d[rINDEX])/10000]=value/10000;
			break;
			
		case GAMEGRAVITY:
		{
			int32_t indx = ri->d[rINDEX]/10000;
			if(indx < 0 || indx > 3)
			{
				scripting_log_error_with_context("Invalid index: {}", indx);
			}
			else
			{
				switch(indx)
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
				}
			}
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

		#define SET_DMAP_VAR(member) \
		{ \
			int32_t ID = ri->d[rINDEX] / 10000; \
			if(BC::checkDMapID(ID) == SH::_NoError) \
				DMaps[ID].member = value / 10000; \
		}

		case DMAPFLAGSD:
			SET_DMAP_VAR(flags) break;
			
		case DMAPLEVELD:
			SET_DMAP_VAR(level) break;
			
		case DMAPCOMPASSD:
			SET_DMAP_VAR(compass) break;
			
		case DMAPCONTINUED:
			SET_DMAP_VAR(cont) break;
			
		case DMAPLEVELPAL:
		{
			int32_t ID = ri->d[rINDEX] / 10000; 
			int32_t pal = value/10000;
			pal = vbound(pal, 0, 0x1FF);
				
			if(BC::checkDMapID(ID) == SH::_NoError) 
				DMaps[ID].color = pal;

			if(ID == cur_dmap)
			{
				loadlvlpal(DMaps[ID].color);
				currcset = DMaps[ID].color;
			}
			break;
		}
		
		case DMAPMIDID:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID) == SH::_NoError)
			{
				// Based on play_DmapMusic
				switch(value / 10000)
				{
				case -6:
					DMaps[ID].midi = 2;
					break; // Dungeon
					
				case -3:
					DMaps[ID].midi = 3;
					break; // Level 9
					
				case -2:
					DMaps[ID].midi = 1;
					break; // Overworld
					
				case 0:
					DMaps[ID].midi = 0;
					break; // None
					
				default:
					DMaps[ID].midi = value / 10000 + 3;
				}
			}
			
			break;
		}

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
