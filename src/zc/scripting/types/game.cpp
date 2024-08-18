#include "base/dmap.h"
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
#include "zc/zelda.h"

#include <optional>

extern refInfo *ri;
extern HeroClass Hero;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

// If scr is currently being used as a layer, return that layer no.
int32_t whichlayer(int32_t scr)
{
	for (int32_t i = 0; i < 6; i++)
	{
		if (scr == (tmpscr->layermap[i] - 1) * MAPSCRS + tmpscr->layerscreen[i])
			return i;
	}

	return -1;
}

int32_t get_screendoor(mapscr *m, int32_t d)
{
	int32_t f = m->door[d];
	return f*10000;
}



int32_t get_screenlayeropacity(mapscr *m, int32_t d)
{
	int32_t f = m->layeropacity[d]; //6 of these
	return f*10000;
}

int32_t get_screensecretcombo(mapscr *m, int32_t d)
{
	int32_t f = m->secretcombo[d]; //128 of these
	return f*10000;
}

int32_t get_screensecretcset(mapscr *m, int32_t d)
{
	int32_t f = m->secretcset[d]; //128 of these
	return f*10000;
}

int32_t get_screensecretflag(mapscr *m, int32_t d)
{
	int32_t f = m->secretflag[d]; //128 of these
	return f*10000;
}

int32_t get_screenlayermap(mapscr *m, int32_t d)
{
	int32_t f = m->layermap[d]; //6 of these
	return f*10000;
}

int32_t get_screenlayerscreen(mapscr *m, int32_t d)
{
	int32_t f = m->layerscreen[d]; //6 of these
	return f*10000;
}

int32_t get_screenpath(mapscr *m, int32_t d)
{
	int32_t f = m->path[d]; //4 of these
	return f*10000;
}

int32_t get_screenwarpReturnX(mapscr *m, int32_t d)
{
	int32_t f = m->warpreturnx[d]; //4 of these
	return f*10000;
}

int32_t get_screenwarpReturnY(mapscr *m, int32_t d)
{
	int32_t f = m->warpreturny[d]; //4 of these
	return f*10000;
}

int32_t get_screenGuy(mapscr *m)
{
	int32_t f = m->guy;
	return f*10000;
}

int32_t get_screenString(mapscr *m)
{
	int32_t f = m->str;
	return f*10000;
}

int32_t get_screenRoomtype(mapscr *m)
{
	int32_t f = m->room;
	return f*10000;
}

int32_t get_screenEntryX(mapscr *m)
{
	int32_t f = m->entry_x;
	return f*10000;
}

int32_t get_screenEntryY(mapscr *m)
{
	int32_t f = m->entry_y;
	return f*10000;
}

int32_t get_screenitem(mapscr *m)
{
	int32_t f = m->item;
	return f*10000;
}

int32_t get_screenundercombo(mapscr *m)
{
	int32_t f = m->undercombo;
	return f*10000;
}

int32_t get_screenundercset(mapscr *m)
{
	int32_t f = m->undercset;
	return f*10000;
}

int32_t get_screenatchall(mapscr *m)
{
	int32_t f = m->catchall;
	return f*10000;
}

int32_t get_screennpc(mapscr *m, int32_t index)
{
	int32_t f = m->enemy[index];
	return f*10000;
}

void set_screendoor(mapscr *m, int32_t d, int32_t value)
{
	int32_t dr = vbound(d,0,3);
	int32_t doortype = vbound(value,0,16);
	m->door[dr] = doortype;
}

void do_getscreenLayerOpacity()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetLayerOpacity(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetLayerOpacity(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 6, "Game->GetLayerOpacity(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenlayeropacity(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenSecretCombo()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSecretCombo(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretCombo(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 127, "Game->GetSecretCombo(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screensecretcombo(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenSecretCSet()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSecretCSet(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretCSet(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 127, "Game->GetSecretCSet(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screensecretcset(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenSecretFlag()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSecretFlag(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretFlag(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0,127, "Game->GetSecretFlag(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screensecretflag(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenLayerMap()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSreenLayerMap(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenLayerMap(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 6, "Game->GetSreenLayerMap(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenlayermap(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenLayerscreen()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSreenLayerScreen(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenLayerScreen(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 6, "Game->GetSreenLayerScreen(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenlayerscreen(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenPath()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetSreenPath(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenPath(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 3, "Game->GetSreenPath(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenpath(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenWarpReturnX()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenWarpReturnX(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenWarpReturnX(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 3, "Game->GetScreenWarpReturnX(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenwarpReturnX(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenWarpReturnY()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t d = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenWarpReturnY(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenWarpReturnY(...screen...)") != SH::_NoError ||
			BC::checkBounds(d, 0, 3, "Game->GetScreenWarpReturnY(...val...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenwarpReturnY(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreendoor()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t door = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenDoor(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenDoor(...screen...)") != SH::_NoError ||
			BC::checkBounds(door, 0, 3, "Game->GetScreenDoor(...doorindex...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screendoor(&TheMaps[map * MAPSCRS + scrn], door));
}

void do_getscreennpc()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t enemy = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenEnemy(...map...)") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEnemy(...screen...)") != SH::_NoError ||
			BC::checkBounds(enemy, 0, 9, "Game->GetScreenEnemy(...enemy...)") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screennpc(&TheMaps[map * MAPSCRS + scrn], enemy));
}

void do_getscreenflags()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t flagset = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenFlags") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenFlags") != SH::_NoError ||
			BC::checkBounds(flagset, 0, 9, "Game->GetScreenFlags") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screenflags(&TheMaps[map * MAPSCRS + scrn], flagset));
}

void do_getscreeneflags()
{
	int32_t map     = (ri->d[rEXP1] / 10000) - 1;
	int32_t scrn  = ri->d[rINDEX2] / 10000;
	int32_t flagset = ri->d[rINDEX] / 10000;
	
	if(BC::checkMapID(map, "Game->GetScreenEFlags") != SH::_NoError ||
			BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEFlags") != SH::_NoError ||
			BC::checkBounds(flagset, 0, 9, "Game->GetScreenEFlags") != SH::_NoError)
		return;
		
	set_register(sarg1, get_screeneflags(&TheMaps[map * MAPSCRS + scrn], flagset));
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
	
	if(BC::checkMessage(ID, "Game->GetMessage") != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, MsgStrings[ID].s) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetMessage' not large enough\n");
}

void do_setmessage(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkMessage(ID, "Game->SetMessage") != SH::_NoError)
		return;
	
	ArrayH::getString(arrayptr, MsgStrings[ID].s, MSG_NEW_SIZE);
}

void do_getdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkDMapID(ID, "Game->GetDMapName") != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapName' not large enough\n");
}

void do_setdmapname(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;

	string filename_str;
	
	if(BC::checkDMapID(ID, "Game->SetDMapName") != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 22);
	strncpy(DMaps[ID].name, filename_str.c_str(), 21);
	DMaps[ID].name[20]='\0';
}

void do_getdmaptitle(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if(BC::checkDMapID(ID, "Game->GetDMapTitle") != SH::_NoError)
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
	
	if(BC::checkDMapID(ID, "Game->SetDMapTitle") != SH::_NoError)
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
	
	if(BC::checkDMapID(ID, "Game->GetDMapIntro") != SH::_NoError)
		return;
		
	if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}


void do_setdmapintro(const bool v)
{
	int32_t ID = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	string filename_str;
	
	if(BC::checkDMapID(ID, "Game->SetDMapIntro") != SH::_NoError)
		return;
		
	ArrayH::getString(arrayptr, filename_str, 73);
	strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
	DMaps[ID].intro[72]='\0';
}

} // end namespace

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
			int32_t mi = (currmap*MAPSCRSNORMAL)+(ri->d[rINDEX]/10000);
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
				Z_scripterrlog("Invalid index used to access Game->Misc: %d\n", indx);
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

		case TANGOARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->Tango[].\n", inx);
				ret = -10000;
				break;
			}
			else
			{
				ret=FFCore.TangoArray[inx]*10000;
				break;
			}
		}
		case GHOSTARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->Ghost[].\n", inx);
				ret = -10000;
				break;
			}
			else
			{
				ret=FFCore.GhostArray[inx]*10000;
				break;
			}
		}
		case STDARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->STD[].\n", inx);
				ret = -10000;
				break;
			}
			else
			{
				ret=FFCore.StdArray[inx]*10000;
				break;
			}
		}
		
		case GAMEMISCSPR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sprMAX )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->MiscSprites[].\n", inx);
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
				Z_scripterrlog("Invalid index %d supplied to Game->MiscSFX[].\n", inx);
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
				Z_scripterrlog("Invalid index %d supplied to Game->OverrideItems[].\n", ind);
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
				Z_scripterrlog("Invalid index %d supplied to Game->TrigGroups[]\n",ind);
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
				Z_scripterrlog("Invalid index used to access Game->Gravity[]: %d\n", indx);
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
				Z_scripterrlog("Invalid index used to access Game->Scrolling[]: %d\n", indx);
			}
			else
			{
				ret = FFCore.ScrollingData[indx] * 10000L;
			}
			break;
		}
			
		case CURMAP:
			ret=(1+currmap)*10000;
			break;
			
		case CURSCR:
			ret=currscr*10000;
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
			ret=(DMaps[get_currdmap()].type==dmOVERW ? currscr : di)*10000;
		}
		break;
		
		case GAMEMAXMAPS:
			ret = (map_count)*10000;
			break;
		case GAMENUMMESSAGES:
			ret = (msg_count-1) * 10000; 
			break;
		
		case CURDMAP:
			ret=currdmap*10000;
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
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboData", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboData", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboData", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboData", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
					
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
			{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=tmpscr->data[pos]*10000;
				else if(layr>-1)
					ret=tmpscr2[layr].data[pos]*10000;
				else ret=TheMaps[scr].data[pos]*10000;
			}
			
		}
		break;
		
		case COMBOCDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboCSet", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboCSet", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboCSet", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboCSet", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
			
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
			{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=tmpscr->cset[pos]*10000;
				else if(layr>-1)
					ret=tmpscr2[layr].cset[pos]*10000;
				else ret=TheMaps[scr].cset[pos]*10000;
			}
			
		}
		break;
		
		case COMBOFDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboFlag", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboFlag", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboFlag", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboFlag", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
			{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=tmpscr->sflag[pos]*10000;
				else if(layr>-1)
					ret=tmpscr2[layr].sflag[pos]*10000;
				else ret=TheMaps[scr].sflag[pos]*10000;
			}
			
		}
		break;
		
		case COMBOTDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboType", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboType", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboType", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboType", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
			
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
			{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=combobuf[tmpscr->data[pos]].type*10000;
				else if(layr>-1)
					ret=combobuf[tmpscr2[layr].data[pos]].type*10000;
				else ret=combobuf[
								 TheMaps[scr].data[pos]].type*10000;
			}
		}
		break;
		
		case COMBOIDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboInherentFlag", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboInherentFlag", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboInherentFlag", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboInherentFlag", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
			
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
					{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=combobuf[tmpscr->data[pos]].flag*10000;
				else if(layr>-1)
					ret=combobuf[tmpscr2[layr].data[pos]].flag*10000;
				else ret=combobuf[TheMaps[scr].data[pos]].flag*10000;
			}
		}
		break;
		
		case COMBOSDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboSolid", pos);
				ret = -10000;
			}
			else if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboSolid", scr);
				ret = -10000;
			}
			else if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboSolid", sc);
				ret = -10000;
			}
			else if(m >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to GetComboSolid", m);
				ret = -10000;
			}
			else if(m < 0) ret = 0; //No layer present
			
			//if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
			else
			{
				if(scr==(currmap*MAPSCRS+currscr))
					ret=(combobuf[tmpscr->data[pos]].walk&15)*10000;
				else if(layr>-1)
					ret=(combobuf[tmpscr2[layr].data[pos]].walk&15)*10000;
				else ret=(combobuf[TheMaps[scr].data[pos]].walk&15)*10000;
			}
		
		}
		break;

		#define GET_DMAP_VAR(member, str) \
		{ \
			int32_t ID = ri->d[rINDEX] / 10000; \
			if(BC::checkDMapID(ID, str) != SH::_NoError) \
				ret = -10000; \
			else \
				ret = DMaps[ID].member * 10000; \
		}

		case DMAPFLAGSD:
			GET_DMAP_VAR(flags,   "Game->DMapFlags")    break;
			
		case DMAPLEVELD:
			GET_DMAP_VAR(level,   "Game->DMapLevel")    break;
			
		case DMAPCOMPASSD:
			GET_DMAP_VAR(compass, "Game->DMapCompass")  break;
			
		case DMAPCONTINUED:
			GET_DMAP_VAR(cont,    "Game->DMapContinue") break;
		
		case DMAPLEVELPAL:
			GET_DMAP_VAR(color,   "Game->DMapPalette")    break; 
			
		case DMAPOFFSET:
			GET_DMAP_VAR(xoff,    "Game->DMapOffset")   break;
			
		case DMAPMAP:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID, "Game->DMapMap") != SH::_NoError)
				ret = -10000;
			else
				ret = (DMaps[ID].map+1) * 10000;
				
			break;
		}
		
		case DMAPMIDID:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID, "Game->DMapMIDI") == SH::_NoError)
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

		case SCREENGUY:
			ret=get_screenGuy(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENSTRING:
			ret=get_screenString(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENROOM:
			ret=get_screenRoomtype(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENENTX:
			ret=get_screenEntryX(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENENTY:
			ret=get_screenEntryY(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENITEM:
			ret=get_screenitem(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENUNDCMB:
			ret=get_screenundercombo(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENUNDCST:
			ret=get_screenundercset(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

		case SCREENCATCH:
			ret=get_screenatchall(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)]);
			break;

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
			int32_t mi2 = (currmap*MAPSCRSNORMAL)+(ri->d[rINDEX]/10000);
			game->guys[mi2]=value/10000;
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
				Z_scripterrlog("Invalid index used to access Game->Misc: %d\n", indx);
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
		
		case TANGOARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->Tango[].\n", inx);
				break;
			}
			else
			{
				FFCore.TangoArray[inx]=value/10000;
				break;
			}
		}
		
		case GHOSTARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->Ghost[].\n", inx);
				break;
			}
			else
			{
			
				FFCore.GhostArray[inx]=value/10000;;
				break;
			}
		}
		case STDARR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > 255 )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->STD[].\n", inx);
				break;
			}
			else
			{
				FFCore.StdArray[inx]=value/10000;
				break;
			}
		}
		
		case GAMEMISCSPR:
		{
			int32_t inx = (ri->d[rINDEX])/10000;
			if ( ((unsigned)inx) > sprMAX )
			{
				Z_scripterrlog("Invalid index %d supplied to Game->MiscSprites[].\n", inx);
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
				Z_scripterrlog("Invalid index %d supplied to Game->MiscSFX[].\n", inx);
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
				Z_scripterrlog("Invalid index %d supplied to Game->OverrideItems[].\n", ind);
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
				Z_scripterrlog("Invalid index used to access Game->Gravity[]: %d\n", indx);
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
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			int32_t layr = whichlayer(scr);
			
			//if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
	    
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to SetComboData", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboData", scr);
				break;
			}
			if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboData", sc);
				break;
			}
			if(unsigned(m) >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to SetComboData", m);
				break;
			}
			int32_t combo = vbound(value/10000,0,MAXCOMBOS);
			if(scr==(currmap*MAPSCRS+currscr))
			{
				screen_combo_modify_preroutine(tmpscr,pos);
			}
				
			TheMaps[scr].data[pos]=combo;
			
			if(scr==(currmap*MAPSCRS+currscr))
			{
				tmpscr->data[pos] = combo;
				screen_combo_modify_postroutine(tmpscr,pos);
				//Start the script for the new combo
				int index = get_combopos_ref(pos, 0);
				FFCore.reset_script_engine_data(ScriptType::Combo, index);
				//Not ure if combodata arrays clean themselves up, or leak. -Z
				//Not sure if this could result in stack corruption. 
			}
			
			if(layr>-1)
			{
				tmpscr2[layr].data[pos]=combo;
				int index = get_combopos_ref(pos, layr + 1);
				FFCore.reset_script_engine_data(ScriptType::Combo, index);
			}
		}
		break;
		
		case COMBOCDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			
			//if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
        
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to SetComboCSet", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboCSet", scr);
				break;
			}
			if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboCSet", sc);
				break;
			}
			if(unsigned(m) >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to SetComboCSet", m);
				break;
			}
			
			TheMaps[scr].cset[pos]=(value/10000)&15;
			
			if(scr==(currmap*MAPSCRS+currscr))
				tmpscr->cset[pos] = value/10000;
				
			int32_t layr = whichlayer(scr);
			
			if(layr>-1)
				tmpscr2[layr].cset[pos]=(value/10000)&15;
		}
		break;
		
		case COMBOFDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
			
			//if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
        
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to SetComboFlag", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboFlag", scr);
				break;
			}
			if(sc >= MAPSCRS) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboFlag", sc);
				break;
			}
			if(unsigned(m) >= map_count) 
			{
				Z_scripterrlog("Invalid Map ID (%d) passed to SetComboFlag", m);
				break;
			}
			
			TheMaps[scr].sflag[pos]=value/10000;
			
			if(scr==(currmap*MAPSCRS+currscr))
				tmpscr->sflag[pos] = value/10000;
				
			int32_t layr = whichlayer(scr);
			
			if(layr>-1)
				tmpscr2[layr].sflag[pos]=value/10000;
		}
		break;
		
		case COMBOTDM:
		{
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t sc = (ri->d[rEXP1]/10000);
			int32_t m = (ri->d[rINDEX2]/10000)-1;
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
						    
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to SetComboType", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to SetComboType", scr);
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
				
			int32_t cdata = TheMaps[scr].data[pos];
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
			int32_t scr = zc_max(m*MAPSCRS+sc,0);
						    
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetComboInherentFlag", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid Screen ID (%d) passed to GetComboInherentFlag", scr);
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
				
			combobuf[TheMaps[scr].data[pos]].flag=value/10000;
		}
		break;
		
		case COMBOSDM:
		{
			//This is how it was in 2.50.1-2
			int32_t pos = (ri->d[rINDEX])/10000;
			int32_t scr = (ri->d[rINDEX2]/10000)*MAPSCRS+(ri->d[rEXP1]/10000);
			//This (below) us the precise code from 2.50.1 (?)
			//int32_t scr = zc_max((ri->d[rINDEX2]/10000)*MAPSCRS+(ri->d[rEXP1]/10000),0); //Not below 0. 

			//if(pos < 0 || pos >= 176 || scr < 0) break;
			if(pos < 0 || pos >= 176) 
			{
				Z_scripterrlog("Invalid combo position (%d) passed to GetSolid", pos);
				break;
			}
			if(scr < 0) 
			{
				Z_scripterrlog("Invalid MapScreen ID (%d) passed to GetSolid", scr);
				break;
			}
			combobuf[TheMaps[scr].data[pos]].walk &= ~0x0F;
			combobuf[TheMaps[scr].data[pos]].walk |= (value/10000)&15;	    
		}
		break;

		#define SET_DMAP_VAR(member, str) \
		{ \
			int32_t ID = ri->d[rINDEX] / 10000; \
			if(BC::checkDMapID(ID, str) == SH::_NoError) \
				DMaps[ID].member = value / 10000; \
		}

		case DMAPFLAGSD:
			SET_DMAP_VAR(flags, "Game->DMapFlags") break;
			
		case DMAPLEVELD:
			SET_DMAP_VAR(level, "Game->DMapLevel") break;
			
		case DMAPCOMPASSD:
			SET_DMAP_VAR(compass, "Game->DMapCompass") break;
			
		case DMAPCONTINUED:
			SET_DMAP_VAR(cont, "Game->DMapContinue") break;
			
		case DMAPLEVELPAL:
		{
			int32_t ID = ri->d[rINDEX] / 10000; 
			int32_t pal = value/10000;
			pal = vbound(pal, 0, 0x1FF);
				
			if(BC::checkDMapID(ID, "Game->DMapPalette") == SH::_NoError) 
				DMaps[ID].color = pal;

			if(ID == currdmap)
			{
				loadlvlpal(DMaps[ID].color);
				currcset = DMaps[ID].color;
			}
			break;
		}
		
		case DMAPMIDID:
		{
			int32_t ID = ri->d[rINDEX] / 10000;
			
			if(BC::checkDMapID(ID, "Game->DMapMIDI") == SH::_NoError)
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
			int32_t mi2 = ri->d[rINDEX]/10000;
			mi2 -= 8*(mi2/MAPSCRS);
			
			if(BC::checkMapID(mi2>>7, "Game->SetScreenState") == SH::_NoError)
				(value)?setmapflag(mi2, 1<<(ri->d[rINDEX2]/10000)) : unsetmapflag(mi2, 1 << (ri->d[rINDEX2] / 10000), true);
		}
		break;

		//SetScreenLayerOpacity(int32_t map, int32_t scr, int32_t layer, int32_t v)
		case SETSCREENLAYOP:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenLayerOpacity(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerOpacity(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenLayerOpacity(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenlayeropacity(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENSECCMB:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenSecretCombo(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretCombo(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenSecretCombo(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screensecretcombo(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENSECCST:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenSecretCSet(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretCSet(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenSecretCSet(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screensecretcset(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENSECFLG:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenSecretFlag(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretFlag(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenSecretFlag(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screensecretflag(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENLAYMAP:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenLayerMap(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerMap(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenLayerMap(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenlayermap(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENLAYSCR:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenLayerScreen(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerScreen(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenLayerScreen(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenlayerscreen(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENPATH:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenPath(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenPath(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenPath(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenpath(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENWARPRX:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenWarpReturnX(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenWarpReturnX(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenWarpReturnX(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenwarpReturnX(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SETSCREENWARPRY:
		{ 
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; //Should this be +1? -Z
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenWarpReturnY(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenWarpReturnY(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenWarpReturnY(...index...)") != SH::_NoError)
				break;
				
			FFScript::set_screenwarpReturnY(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;

		case SCREENGUY:
			FFScript::set_screenGuy(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENSTRING:
		{
			FFScript::set_screenString(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			//should this be either
			//set_screenString(&TheMaps[((ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)])-1), value/10000);
			//or
			//set_screenString(&TheMaps[((ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)])-+1), value/10000);
			Z_message("Map ref is: %d\n",((ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)));
		}
		break;

		case SCREENROOM:
			FFScript::set_screenRoomtype(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENENTX:
			FFScript::set_screenEntryX(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENENTY:
			FFScript::set_screenEntryY(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENITEM:
			FFScript::set_screenitem(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENUNDCMB:
			FFScript::set_screenundercombo(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENUNDCST:
			FFScript::set_screenundercset(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
			break;

		case SCREENCATCH:
			FFScript::set_screenatchall(&TheMaps[(ri->d[rINDEX2] / 10000) * MAPSCRS + (ri->d[rINDEX]/10000)], value/10000);
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

		case GETSCREENDOOR:
			do_getscreendoor();
			break;
		
		case GETSCREENENEMY:
			do_getscreennpc();
			break;
		
		case GETSCREENFLAGS:
			do_getscreenflags();
			break;
			
		case GETSCREENEFLAGS:
			do_getscreeneflags();
			break;
		
		//screendata and mapdata
		case SETSCREENENEMY:
		{ //void SetScreenEnemy(int32_t map, int32_t screen, int32_t index, int32_t value);
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; 
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 

			if(BC::checkMapID(map, "Game->SetScreenEnemy(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenEnemy(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenEnemy(...index...)") != SH::_NoError)
				return RUNSCRIPT_ERROR;
			
			FFScript::set_screenenemy(&TheMaps[map * MAPSCRS + scrn], index, nn); 
		}
		break;
		
		case SETSCREENDOOR:
		{ //void SetScreenDoor(int32_t map, int32_t screen, int32_t index, int32_t value);
			int32_t map     = (ri->d[rINDEX2] / 10000) - 1; 
			int32_t scrn  = ri->d[rEXP1] / 10000; 
			int32_t index = ri->d[rINDEX] / 10000; 
			int32_t nn = ri->d[rEXP2]/10000; 
			
			if(BC::checkMapID(map, "Game->SetScreenDoor(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenDoor(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 3, "Game->SetScreenDoor(...doorindex...)") != SH::_NoError)
			{
				return RUNSCRIPT_ERROR; break;
			}
			else
			{
				set_screendoor(&TheMaps[map * MAPSCRS + scrn], index, nn); 
				break;
			}
			
		}

		case GETSCREENLAYOP:
			do_getscreenLayerOpacity();
			break;
		case GETSCREENSECCMB:
			do_getscreenSecretCombo();
			break;
		case GETSCREENSECCST:
			do_getscreenSecretCSet();
			break;
		case GETSCREENSECFLG:
			do_getscreenSecretFlag();
			break;
		case GETSCREENLAYMAP:
			do_getscreenLayerMap();
			break;
		case GETSCREENLAYSCR:
			do_getscreenLayerscreen();
			break;
		case GETSCREENPATH:
			do_getscreenPath();
			break;
		case GETSCREENWARPRX:
			do_getscreenWarpReturnX();
			break;
		case GETSCREENWARPRY:
			do_getscreenWarpReturnY();
			break;

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
