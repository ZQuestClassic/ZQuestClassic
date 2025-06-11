#include "zc/scripting/types/npc.h"

#include "base/check.h"
#include "base/qrs.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/scripting/arrays.h"

#include <optional>
#include <type_traits>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;
extern char *guy_string[];

// For the "npc" type - also called "guys" internally, and backed by the "enemy" class... :)

int32_t GuyH::loadNPC(const int32_t uid)
{
	tempenemy = ResolveNpc(uid);
	if (!tempenemy) 
		return _InvalidSpriteUID;
	
	return _NoError;
}

enemy *GuyH::getNPC()
{
	return tempenemy;
}

int32_t GuyH::getNPCIndex(const int32_t eid)
{
	for(word i = 0; i < guys.Count(); i++)
	{
		if(guys.spr(i)->getUID() == eid)
			return i;
	}
	
	return -1;
}

int32_t GuyH::getNPCDMisc(const byte a)
{
	switch(a)
	{
		case 0: return tempenemy->dmisc1;
		case 1: return tempenemy->dmisc2;
		case 2: return tempenemy->dmisc3;
		case 3: return tempenemy->dmisc4;
		case 4: return tempenemy->dmisc5;
		case 5: return tempenemy->dmisc6;
		case 6: return tempenemy->dmisc7;
		case 7: return tempenemy->dmisc8;
		case 8: return tempenemy->dmisc9;
		case 9: return tempenemy->dmisc10;
		case 10: return tempenemy->dmisc11;
		case 11: return tempenemy->dmisc12;
		case 12: return tempenemy->dmisc13;
		case 13: return tempenemy->dmisc14;
		case 14: return tempenemy->dmisc15;
		case 15: return tempenemy->dmisc16;
		case 16: return tempenemy->dmisc17;
		case 17: return tempenemy->dmisc18;
		case 18: return tempenemy->dmisc19;
		case 19: return tempenemy->dmisc20;
		case 20: return tempenemy->dmisc21;
		case 21: return tempenemy->dmisc22;
		case 22: return tempenemy->dmisc23;
		case 23: return tempenemy->dmisc24;
		case 24: return tempenemy->dmisc25;
		case 25: return tempenemy->dmisc26;
		case 26: return tempenemy->dmisc27;
		case 27: return tempenemy->dmisc28;
		case 28: return tempenemy->dmisc29;
		case 29: return tempenemy->dmisc30;
		case 30: return tempenemy->dmisc31;
		case 31: return tempenemy->dmisc32;
	}
	
	return 0;
}

bool GuyH::hasHero()
{
	if(tempenemy->family == eeWALLM)
		return ((eWallM *) tempenemy)->hashero;
		
	if(tempenemy->family == eeWALK)
		return ((eStalfos *) tempenemy)->hashero;
		
	return false;
}

int32_t GuyH::getMFlags()
{
	clear_ornextflag();
	flagpos = 5;
	// Must be in the same order as in the Enemy Editor pane
	ornextflag(tempenemy->flags&(guy_lens_only));
	ornextflag(tempenemy->flags&(guy_flashing));
	ornextflag(tempenemy->flags&(guy_blinking));
	ornextflag(tempenemy->flags&(guy_transparent));
	ornextflag(tempenemy->flags&(guy_shield_front));
	ornextflag(tempenemy->flags&(guy_shield_left));
	ornextflag(tempenemy->flags&(guy_shield_right));
	ornextflag(tempenemy->flags&(guy_shield_back));
	ornextflag(tempenemy->flags&(guy_bkshield));
	return (tempenemy->flags&0x1F) | flagval;
}

void GuyH::clearTemp()
{
	tempenemy = NULL;
}

enemy *GuyH::tempenemy = NULL;

namespace {

void do_npcattack()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		GuyH::getNPC()->FireWeapon();
	}
}
void do_npc_newdir()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz != -1 ) 
		{
			if ( sz != 3 ) 
			{
				scripting_log_error_with_context("Invalid array size: {}", sz);
				return;
			}
			GuyH::getNPC()->newdir((am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000));
		}
		//else e->newdir();
		else GuyH::getNPC()->newdir();
	}
}

void do_npc_constwalk()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz != -1 ) 
		{
			if ( sz != 3 ) 
			{
				scripting_log_error_with_context("Invalid array size: {}", sz);
				return;
			}
			GuyH::getNPC()->constant_walk( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000) );
		}
		else GuyH::getNPC()->constant_walk();//e->constant_walk();
	}
}

void do_npc_varwalk()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 3 ) 
		{
			
			GuyH::getNPC()->variable_walk( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000) );
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}

void do_npc_varwalk8()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 4 ) 
		{
			GuyH::getNPC()->variable_walk_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000) );
		}
		else if ( sz == 8 ) 
		{
			GuyH::getNPC()->variable_walk_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000),
				(am.get(4)/10000), (am.get(5)/10000),
				(am.get(6)/10000), (am.get(7)/10000)
			);
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}

void do_npc_constwalk8()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 3 ) 
		{
			GuyH::getNPC()->constant_walk_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000) );
		}
		
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}


void do_npc_haltwalk()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 5 ) 
		{
			GuyH::getNPC()->halting_walk( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000),
				(am.get(4)/10000));
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}

void do_npc_haltwalk8()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 6 ) 
		{
			
			GuyH::getNPC()->halting_walk_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000),
				(am.get(4)/10000),(am.get(5)/10000));
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}


void do_npc_floatwalk()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 3 ) 
		{
			
			GuyH::getNPC()->floater_walk( (am.get(0)/10000), (am.get(1)/10000),
				(zfix)(am.get(2)/10000));
		
		}
		else if ( sz == 7 ) 
		{
			
			GuyH::getNPC()->floater_walk( (am.get(0)/10000), (am.get(1)/10000),
				(zfix)(am.get(2)/10000), (zfix)(am.get(3)/10000),
				(am.get(4)/10000),(am.get(5)/10000),
				(am.get(6)/10000));
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}

void do_npc_breathefire()
{
	bool seek = (get_register(sarg1));
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		GuyH::getNPC()->FireBreath(seek);
	}
}


void do_npc_newdir8()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 3 ) 
		{
			
			GuyH::getNPC()->newdir_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000));
		
		}
		else if ( sz == 7 ) 
		{
			
			GuyH::getNPC()->newdir_8( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000),
				(am.get(4)/10000),(am.get(5)/10000),
				(am.get(6)/10000));
		}
		else scripting_log_error_with_context("Invalid array size: {}", sz);
	}
}

int32_t npc_collision()
{
	int32_t isColl = 0;
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		int32_t _obj_type = (ri->d[rINDEX] / 10000);
		int32_t _obj_ptr = (ri->d[rINDEX2]);
		
		switch(_obj_type)
		{
			case obj_type_lweapon:
			{
				isColl = 0;
				break;
			}
			case obj_type_eweapon:
			{
				isColl = 0;
				break;
			}
			case obj_type_npc:
			{
				isColl = 0;
				break;
			}
			case obj_type_player:
			{
				isColl = 0;
				break;
			}
			case obj_type_ffc:
			{
				_obj_ptr *= 10000; _obj_ptr -= 1;
				isColl = 0;
				break;
			}
			case obj_type_combo_pos:
			{
				_obj_ptr *= 10000;
				isColl = 0;
				break;
			}
			case obj_type_item:
			{
				isColl = 0;
				break;
			}
			default: 
			{
				Z_scripterrlog("Invalid object type (%d) passed to npc->Collision(int32_t type, int32_t ptr)\n", _obj_type);
				isColl = 0;
				break;
			}
		}
	}
	
	return isColl;
}

int32_t npc_linedup()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		int32_t range = (ri->d[rINDEX] / 10000);
		bool dir8 = (ri->d[rINDEX2]);
		return (int32_t)(GuyH::getNPC()->lined_up(range,dir8)*10000);
	}
	
	return 0;
}


void do_npc_hero_in_range(const bool v)
{
	int32_t dist = get_register(sarg1) / 10000;
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		bool in_range = GuyH::getNPC()->HeroInRange(dist);
		set_register(sarg1, (in_range ? 10000 : 0)); //This isn't setting the right value, it seems. 
	}
	else set_register(sarg2, 0);
}

void do_npc_simulate_hit(const bool v)
{
	int32_t arrayptr = SH::get_arg(sarg1, v) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	bool ishit = false;
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		if ( sz == 2 ) //type and pointer
		{
			ishit = false;
		}
		if ( sz == 6 ) //hit(int32_t tx,int32_t ty,int32_t tz,int32_t txsz,int32_t tysz,int32_t tzsz);
		{
			ishit = GuyH::getNPC()->hit( (am.get(0)/10000), (am.get(1)/10000),
				(am.get(2)/10000), (am.get(3)/10000), 
				(am.get(4)/10000), (am.get(5)/10000) );			
			
		}
		else 
		{
			Z_scripterrlog("Invalid array size (%d) passed to npc->SimulateHit(). The array size must be [1] or [3].\n", sz);
			ishit = false;
		}
	}
	set_register(sarg1, ( ishit ? 10000 : 0));
}

void do_npc_knockback(const bool v)
{
	int32_t time = SH::get_arg(sarg1, v) / 10000;
	int32_t dir = SH::get_arg(sarg2, v) / 10000;
	int32_t spd = vbound(ri->d[rINDEX] / 10000, 0, 255);
	bool ret = false;
	
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		ret = GuyH::getNPC()->knockback(time, dir, spd);
	}
	set_register(sarg1, ( ret ? 10000 : 0));
}

void do_npc_add(const bool v)
{
	
	int32_t arrayptr = SH::get_arg(sarg1, v) / 10000;
	ArrayManager am(arrayptr);
	if(am.invalid()) return;
	int32_t sz = am.size();
	
	int32_t id = 0, nx = 0, ny = 0;
	
	if ( sz < 1 ) 
	{
		Z_scripterrlog("Invalid array size (%d) passed to npc->Create(). The array size must be [1] or [3].\n", sz);
		return;
	}
	else //size is valid
	{
		id = (am.get(0)/10000);
		
		if ( sz == 3 ) //x and y
		{
			nx = (am.get(1)/10000);
			ny = (am.get(2)/10000);
		}
	}
	
	
	if(BC::checkGuyID(id) != SH::_NoError)
		return;
		
	//If we make a segmented enemy there'll be more than one sprite created
	word numcreated = addenemy(cur_screen, nx, ny, id, -10);
	
	if(numcreated == 0)
	{
		ri->guyref = MAX_DWORD;
		Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", guy_string[id]);
	}
	else
	{
		word index = guys.Count() - numcreated; //Get the main enemy, not a segment
		ri->guyref = guys.spr(index)->getUID();
		
		for(; index<guys.Count(); index++)
			((enemy*)guys.spr(index))->script_spawned=true;
		
		ri->d[rEXP1] = ri->guyref;
		ri->d[rEXP2] = ri->guyref;
		Z_eventlog("Script created NPC \"%s\" with UID = %ld\n", guy_string[id], ri->guyref);
	}
}

void do_getnpcname()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	
	if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
		return;
		
	word ID = (GuyH::getNPC()->id & 0xFFF);
	
	if(ArrayH::setArray(arrayptr, guy_string[ID]) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'npc->GetName' not large enough\n");
}

void do_getnpcdata_getname()
{
	int32_t arrayptr = get_register(sarg1) / 10000;
	int32_t npc_id = ri->npcdataref;
	if((unsigned)npc_id > 511)
	{
		Z_scripterrlog("Invalid npc ID (%d) passed to npcdata->GetName().\n", npc_id);
		return;
	}
		
	if(ArrayH::setArray(arrayptr, guy_string[npc_id]) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'npcdata->GetName()' not large enough\n");
}

void do_isdeadnpc()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		set_register(sarg1,
			((GuyH::getNPC()->dying && !GuyH::getNPC()->immortal)
				? 10000 : 0));
	}
	else set_register(sarg1, 0);
}


void do_canslidenpc()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		set_register(sarg1, ((GuyH::getNPC()->can_slide()) ? 10000 : 0));
	}
	else set_register(sarg1, -10000);
}

void do_slidenpc()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		set_register(sarg1, ((GuyH::getNPC()->slide())*10000));
	}
	else set_register(sarg1, -10000);
}

void do_npc_stopbgsfx()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		GuyH::getNPC()->stop_bgsfx(GuyH::getNPCIndex(ri->guyref));
	}
}

void do_npc_delete()
{
	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		auto ind = GuyH::getNPCIndex(ri->guyref);
		GuyH::getNPC()->stop_bgsfx(ind);
		guys.del(ind);
	}
}

void do_npc_canmove(const bool v)
{
	int32_t arrayptr = SH::get_arg(sarg1, v) / 10000;
	int32_t sz = ArrayH::getSize(arrayptr);

	if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		ArrayManager am(arrayptr);
		if(am.invalid()) return;
		if ( sz == 1 ) //bool canmove(int32_t ndir): dir only, uses 'step' IIRC
		{
			set_register(sarg1, ( GuyH::getNPC()->canmove((am.get(0)/10000),false)) ? 10000 : 0);
		}
		else if ( sz == 2 ) //bool canmove(int32_t ndir, int32_t special): I think that this also uses the default 'step'
		{
			set_register(sarg1, ( GuyH::getNPC()->canmove((am.get(0)/10000),(zfix)(am.get(1)/10000), false)) ? 10000 : 0);
		}
		else if ( sz == 3 ) //bool canmove(int32_t ndir,zfix s,int32_t special) : I'm pretty sure that 'zfix s' is 'step' here. 
		{
			set_register(sarg1, ( GuyH::getNPC()->canmove((am.get(0)/10000),(zfix)(am.get(1)/10000),(am.get(2)/10000),false)) ? 10000 : 0);
		}
		else if ( sz == 7 ) //bool canmove(int32_t ndir,zfix s,int32_t special) : I'm pretty sure that 'zfix s' is 'step' here. 
		{
			set_register(sarg1, ( GuyH::getNPC()->canmove((am.get(0)/10000),(zfix)(am.get(1)/10000),(am.get(2)/10000),(am.get(3)/10000),(am.get(4)/10000),(am.get(5)/10000),(am.get(6)/10000),false)) ? 10000 : 0);
		}
		else 
		{
			Z_scripterrlog("Invalid array size (%d) passed to npc->CanMove(). The array size must be [1], [2], [3], or [7].\n", sz);
			//can_mv = false;
			set_register(sarg1, 0);
		}
	}
	//set_register(sarg1, ( can_mv ? 10000 : 0));
}

void get_npcdata_initd_label(const bool v)
{
	int32_t init_d_index = SH::get_arg(sarg1, v) / 10000;
	int32_t arrayptr = get_register(sarg2) / 10000;
	
	if((unsigned)init_d_index > 7)
	{
		Z_scripterrlog("Invalid InitD[] index (%d) passed to npcdata->GetInitDLabel().\n", init_d_index);
		return;
	}
		
	if(ArrayH::setArray(arrayptr, string(guysbuf[ri->npcdataref].initD_label[init_d_index])) == SH::_Overflow)
		Z_scripterrlog("Array supplied to 'npcdata->GetInitDLabel()' not large enough\n");
}

void do_breakshield()
{
	int32_t UID = get_register(sarg1);

	for (int32_t j = 0; j < guys.Count(); j++)
		if (guys.spr(j)->getUID() == UID)
		{
			((enemy*)guys.spr(j))->break_shield();
			return;
		}
}

void do_repairshield()
{
	if (GuyH::loadNPC(ri->guyref) == SH::_NoError)
	{
		GuyH::getNPC()->repair_shield();
	}
}

} // end namespace

std::optional<int32_t> npc_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		#define GET_NPC_VAR_INT(member) \
		{ \
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) \
				ret = -10000; \
			else \
				ret = GuyH::getNPC()->member * 10000; \
		}

		case NPCDIR:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = zc_max(GuyH::getNPC()->dir * 10000, 0);
				
			break;
			
		case NPCHITDIR:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->hitdir * 10000);
				
			break;
			
		case NPCSLIDECLK:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->sclk * 10000);
				
			break;
			
		case NPCHALTCLK:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->clk2 * 10000);
				
			break;
			
		case NPCFRAME:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->clk * 10000);
				
			break;
			
		case NPCMOVESTATUS:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->movestatus * 10000);
				
			break;
			
		case NPCFADING:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->fading * 10000);
				
			break;
			
		case NPCRATE:
			GET_NPC_VAR_INT(rate) break;
			
		case NPCHOMING:
			GET_NPC_VAR_INT(homing) break;
			
		case NPCFRAMERATE:
			GET_NPC_VAR_INT(frate) break;
			
		case NPCHALTRATE:
			GET_NPC_VAR_INT(hrate) break;
		
		case NPCRANDOM:
			GET_NPC_VAR_INT(rate) break;
			
		case NPCDRAWTYPE:
			GET_NPC_VAR_INT(drawstyle) break;
		
		case NPCHP:
			GET_NPC_VAR_INT(hp) break;

		case NPCORIGINALHP:
			GET_NPC_VAR_INT(starting_hp) break;
			
		case NPCCOLLDET:
			GET_NPC_VAR_INT(scriptcoldet) break;
		
		case NPCENGINEANIMATE:
			GET_NPC_VAR_INT(do_animation) break;
			
		case NPCSTUN:
			GET_NPC_VAR_INT(stunclk) break;
			
		case NPCHUNGER:
			GET_NPC_VAR_INT(grumble) break;
		
		case NPCWEAPSPRITE:
			GET_NPC_VAR_INT(wpnsprite) break;
			
		case NPCTYPE:
			GET_NPC_VAR_INT(family) break;
			
		case NPCDP:
			GET_NPC_VAR_INT(dp) break;
			
		case NPCWDP:
			GET_NPC_VAR_INT(wdp) break;
			
		case NPCOTILE:
			GET_NPC_VAR_INT(o_tile) break;
			
		case NPCTILE:
			GET_NPC_VAR_INT(tile) break;
		
		case NPCSCRIPTTILE:
			GET_NPC_VAR_INT(scripttile) break;
			
		case NPCSCRIPTFLIP:
			GET_NPC_VAR_INT(scriptflip) break;
			
		case NPCWEAPON:
			GET_NPC_VAR_INT(wpn) break;
			
		case NPCITEMSET:
			GET_NPC_VAR_INT(item_set) break;
			
		case NPCCSET:
			GET_NPC_VAR_INT(cs) break;
			
		case NPCBOSSPAL:
			GET_NPC_VAR_INT(bosspal) break;
			
		case NPCBGSFX:
			GET_NPC_VAR_INT(bgsfx) break;
			
		case NPCEXTEND:
			GET_NPC_VAR_INT(extend) break;
			
		case NPCHXOFS:
			GET_NPC_VAR_INT(hxofs) break;
			
		case NPCHYOFS:
			GET_NPC_VAR_INT(hyofs) break;
			
		case NPCHXSZ:
			GET_NPC_VAR_INT(hit_width) break;
			
		case NPCHYSZ:
			GET_NPC_VAR_INT(hit_height) break;
			
		case NPCHZSZ:
			GET_NPC_VAR_INT(hzsz) break;
		
		case NPCROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1;
				break;
			}
			GET_NPC_VAR_INT(rotation)
			break;

		case NPCTXSZ:
			GET_NPC_VAR_INT(txsz) break;
			
		case NPCTYSZ:
			GET_NPC_VAR_INT(tysz) break;
		

		//And zfix
		#define GET_NPC_VAR_FIX(member) \
		{ \
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) \
			{ \
				ret = -10000; \
				break; \
			} \
			else \
				ret = (int32_t(GuyH::getNPC()->member) * 10000); \
		}

		case NPCX:
		//GET_NPC_VAR_FIX(x, "npc->X") break;     
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) 
			{
				ret = -10000; 
			}
			else 
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret = ((GuyH::getNPC()->x).getZLong()); 
				}
				else
				{
					ret = (int32_t(GuyH::getNPC()->x) * 10000);   
				}
			}
			break;
		}
		
		case SPRITEMAXNPC:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			ret = guys.getMax() * 10000;
			break;
		}
		
		case NPCSUBMERGED:
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) 
			{
				ret = -10000; 
			}    
			else
			{
				ret = ((GuyH::getNPC()->isSubmerged()) ? 10000 : 0);
				
			}
			break;	
		}
			
			
		case NPCY:
			//GET_NPC_VAR_FIX(y, "npc->Y") break;
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) 
			{
				ret = -10000; 
			}
			else 
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret = ((GuyH::getNPC()->y).getZLong()); 
				}
				else
				{
					ret = (int32_t(GuyH::getNPC()->y) * 10000);   
				}
			}
			break;
		}
		
			
		case NPCZ:
			//GET_NPC_VAR_FIX(z, "npc->Z") break;
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) 
			{
				ret = -10000; 
			}
			else 
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret = ((GuyH::getNPC()->z).getZLong()); 
				}
				else
				{
					ret = (int32_t(GuyH::getNPC()->z) * 10000);   
				}
			}
			break;
		}
			
		case NPCXOFS:
			GET_NPC_VAR_FIX(xofs) break;
			
		case NPCYOFS:
			GET_NPC_VAR_FIX(yofs) ret-=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)*10000;
			break;
		case NPCSHADOWXOFS:
			GET_NPC_VAR_FIX(shadowxofs) break;
			
		case NPCSHADOWYOFS:
			GET_NPC_VAR_FIX(shadowyofs) break;
			
		case NPCTOTALDYOFFS:
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
			{
				ret = -10000;
			}
			else
			{
				ret = ((int32_t(GuyH::getNPC()->yofs - (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((GuyH::getNPC()->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0)) * 10000);
			}
			break;
		}
			
		case NPCZOFS:
			GET_NPC_VAR_FIX(zofs) break;
			
			//These variables are all different to the templates (casting for jump and step is slightly non-standard)
		case NPCJUMP:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
			{
				ret = GuyH::getNPC()->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
		
		case NPCFAKEJUMP:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
			{
				ret = GuyH::getNPC()->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
				
			break;
		
		
		case NPCSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (int32_t(GuyH::getNPC()->scale) * 100.0);
				
			break;
		
		case NPCIMMORTAL:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = GuyH::getNPC()->immortal ? 10000 : 0;
			break;
		
		case NPCNOSLIDE:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->knockbackflags & FLAG_NOSLIDE) ? 10000 : 0;
			break;
		
		case NPCNOSCRIPTKB:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->knockbackflags & FLAG_NOSCRIPTKNOCKBACK) ? 10000 : 0;
			break;
		
		case NPCKNOCKBACKSPEED:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = GuyH::getNPC()->knockbackSpeed * 10000;
			break;
			
		case NPCSTEP:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					ret = ( ( (GuyH::getNPC()->step).getZLong() ) * 100 );
				}
				//old, buggy code replication, round two: Go! -Z
				//else ret = ( ( (GuyH::getNPC()->step) * 100.0 ).getZLong() );
				else 
				{
					double s2 = ( (GuyH::getNPC()->step).getZLong() );
					ret = int32_t(s2*100);
					//ret = int32_t( ( (GuyH::getNPC()->step) * 100.0 )) * 10000;
				}
				//else ret = int32_t(GuyH::getNPC()->step * fix(100.0)) * 10000;
				
				//else 
				//{
					//old, buggy code replication, round THREE: Go! -Z
				//	double tmp = ( (GuyH::getNPC()->step) ) * 1000000.0;
				//	ret = (int32_t)tmp;
				//}
			}
				
			break;
		
		case NPCGRAVITY:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = ((GuyH::getNPC()->moveflags & move_obeys_grav) ? 10000 : 0);
				
			break;
		
			
		case NPCID:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (GuyH::getNPC()->id & 0xFFF) * 10000;
				
			break;
		
		case NPCISCORE:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = ((GuyH::getNPC()->isCore) ? 10000 : 0);
				
			break;
		
		case NPCSCRIPTUID:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = ((GuyH::getNPC()->getUID()));
				
			break;
		
		case NPCPARENTUID:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = GuyH::getNPC()->parent ? GuyH::getNPC()->parent->getUID() : 0; //literal, not *10000
				
			break;

		case NPCMFLAGS:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = GuyH::getMFlags() * 10000;
				
			break;

		case NPCSCRIPT:
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError )
				ret = -10000;
			else
			{
				//enemy *e = (enemy*)guys.spr(ri->guyref);
				ret = (int32_t)GuyH::getNPC()->script * 10000;
			}
		}
		break;
		
		case NPCINVINC:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (int32_t)GuyH::getNPC()->hclk * 10000;
				
			break;
		
		case NPCHASITEM:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = 0;
			else
				ret = GuyH::getNPC()->itemguy?10000:0;
				
			break;
		
		case NPCRINGLEAD:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = 0;
			else
				ret = GuyH::getNPC()->leader?10000:0;
				
			break;
		
		case NPCSUPERMAN:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = (int32_t)GuyH::getNPC()->superman * 10000;
				
			break;
		
		case NPCFROZENTILE:
			GET_NPC_VAR_INT(frozentile); break;
		
		case NPCFROZENCSET:
			GET_NPC_VAR_INT(frozencset); break;
		
		case NPCFROZEN:
			GET_NPC_VAR_INT(frozenclock); break;
		
		

		case NPCFALLCLK:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->fallclk * 10000;
			}
			break;
		
		case NPCFALLCMB:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->fallCombo * 10000;
			}
			break;
			
		case NPCDROWNCLK:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->drownclk * 10000;
			}
			break;
		
		case NPCDROWNCMB:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->drownCombo * 10000;
			}
			break;
		
		case NPCFAKEZ:
		{
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError) 
			{
				ret = -10000; 
			}
			else 
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret = ((GuyH::getNPC()->fakez).getZLong()); 
				}
				else
				{
					ret = (int32_t(GuyH::getNPC()->fakez) * 10000);   
				}
			}
			break;
		}

		case NPCGLOWRAD:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->glowRad * 10000;
			}
			break;
			
		case NPCGLOWSHP:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->glowShape * 10000;
			}
			break;
			
		case NPCSHADOWSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->spr_shadow * 10000;
			}
			break;
		case NPCSPAWNSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->spr_spawn * 10000;
			}
			break;
		case NPCDEATHSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->spr_death * 10000;
			}
			break;
		case NPCSWHOOKED:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->switch_hooked ? 10000 : 0;
			}
			break;
		case NPCCANFLICKER:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ret = GuyH::getNPC()->getCanFlicker() ? 10000 : 0;
			}
			break;
		case NPCFLICKERCOLOR:
			GET_NPC_VAR_INT(flickercolor) break;
		case NPCFLASHINGCSET:
		{
			if (GuyH::loadNPC(ri->guyref) != SH::_NoError)
				ret = -10000;
			else
				ret = GuyH::getNPC()->getFlashingCSet() * 10000;
			break;
		}
		case NPCFLICKERTRANSP:
			GET_NPC_VAR_INT(flickertransp) break;
		
		case NPCCOLLISION:
			ret = npc_collision();
			break;

		case NPCLINEDUP:
			ret = npc_linedup();
			break;

		case NPCFIRESFX:
			GET_NPC_VAR_INT(firesfx) break;

		default: return std::nullopt;
	}

	return ret;
}

bool npc_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case NPCX:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->x = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
				if(GuyH::hasHero())
					Hero.setXfix(get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
			}
		}
		break;
		
		case NPCSCALE:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->scale = (value / 100.0);
			}
		}
		break;
		
		case NPCIMMORTAL:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->immortal = (value ? true : false);
			}
			break;
		
		case NPCNOSLIDE:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(value)
				{
					GuyH::getNPC()->knockbackflags |= FLAG_NOSLIDE;
				}
				else
				{
					GuyH::getNPC()->knockbackflags &= ~FLAG_NOSLIDE;
				}
			}
			break;
		
		case NPCNOSCRIPTKB:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(value)
				{
					GuyH::getNPC()->knockbackflags |= FLAG_NOSCRIPTKNOCKBACK;
				}
				else
				{
					GuyH::getNPC()->knockbackflags &= ~FLAG_NOSCRIPTKNOCKBACK;
				}
			}
			break;
		
		case NPCKNOCKBACKSPEED:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->knockbackSpeed = vbound(value/10000, 0, 255);
			}
			break;
		
		case SPRITEMAXNPC:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			guys.setMax(vbound((value/10000),1,MAX_NPC_SPRITES));
			break;
		}
			
		case NPCY:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				zfix oldy = GuyH::getNPC()->y;
				GuyH::getNPC()->y = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				GuyH::getNPC()->floor_y += ((get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000)) - oldy);
				
				if(GuyH::hasHero())
					Hero.setYfix(get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
			}
		}
		break;
		
		case NPCZ:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(!never_in_air(GuyH::getNPC()->id))
				{
					if(value < 0)
						GuyH::getNPC()->z = 0_zf;
					else
						GuyH::getNPC()->z = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
						
					if(GuyH::hasHero())
						Hero.setZfix(get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
				}
			}
		}
		break;
		
		case NPCJUMP:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(canfall(GuyH::getNPC()->id))
					GuyH::getNPC()->fall =zslongToFix(value)*-100;
					
				if(GuyH::hasHero())
					Hero.setFall(zslongToFix(value)*-100);
			}
		}
		break;
		
		case NPCFAKEJUMP:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(canfall(GuyH::getNPC()->id))
					GuyH::getNPC()->fakefall =zslongToFix(value)*-100;
					
				if(GuyH::hasHero())
					Hero.setFakeFall(zslongToFix(value)*-100);
			}
		}
		break;
		
		case NPCSTEP:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{	
					GuyH::getNPC()->step = zslongToFix(value / 100);
				}
				else
				{
					//old, buggy code replication, round two: Go! -Z
					//zfix val = zslongToFix(value);
					//val.doFloor();
					//GuyH::getNPC()->step = ((val / 100.0).getFloat());
					
					//old, buggy code replication, round THREE: Go! -Z
					GuyH::getNPC()->step = ((value/10000)/100.0);
				}
			}
		}
		break;
		
		case NPCGRAVITY:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(value)
					GuyH::getNPC()->moveflags |= move_obeys_grav;
				else
					GuyH::getNPC()->moveflags &= ~move_obeys_grav;
			}
		}
		break;
		
		case NPCXOFS:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->xofs = zfix(value / 10000);
		}
		break;
		
		case NPCYOFS:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->yofs = zfix(value / 10000) + (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
		}
		break;
		
		case NPCSHADOWXOFS:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->shadowxofs = zfix(value / 10000);
		}
		break;
		
		case NPCSHADOWYOFS:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->shadowyofs = zfix(value / 10000);
		}
		break;
		
		case NPCTOTALDYOFFS:
			break; //READ-ONLY
		
		case NPCROTATION:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->rotation = (value / 10000);
		}
		break;
		
		case NPCZOFS:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->zofs = zfix(value / 10000);
		}
		break;
		
		#define SET_NPC_VAR_INT(member) \
		{ \
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError) \
				GuyH::getNPC()->member = value / 10000; \
		}
		
		
		case NPCISCORE:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			GuyH::getNPC()->isCore = ( (value / 10000) ? true : false );
			break;
		
		
		case NPCDIR:
			SET_NPC_VAR_INT(dir) break;
			
		case NPCHITDIR:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				(GuyH::getNPC()->hitdir) = vbound(value/10000, 0, 3);
				
			break;
			
		case NPCSLIDECLK:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				GuyH::getNPC()->sclk = value/10000;//vbound(value/10000,0,255);
				
			break;
			
		case NPCFADING:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				(GuyH::getNPC()->fading) = vbound(value/10000,0,4);
				
			break;
			
		case NPCHALTCLK:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				(GuyH::getNPC()->clk2) = vbound(value/10000,0,214748);
				
			break;
			
		case NPCFRAME:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				(GuyH::getNPC()->clk2) = vbound(value/10000,0,214748);
				
			break;
		
		case NPCMOVESTATUS:
			if(GuyH::loadNPC(ri->guyref) != SH::_NoError)
				(GuyH::getNPC()->movestatus) = vbound(value/10000,0,3);
				
			break;
			
		case NPCRATE:
			SET_NPC_VAR_INT(rate) break;
			
		case NPCHOMING:
			SET_NPC_VAR_INT(homing) break;
			
		case NPCFRAMERATE:
			SET_NPC_VAR_INT(frate) break;
			
		case NPCHALTRATE:
			SET_NPC_VAR_INT(hrate) break;
		
		case NPCRANDOM:
			SET_NPC_VAR_INT(rate) break;
			
		case NPCDRAWTYPE:
			SET_NPC_VAR_INT(drawstyle) break;
			
		case NPCHP:
			SET_NPC_VAR_INT(hp) break;
		
		case NPCORIGINALHP:
			SET_NPC_VAR_INT(starting_hp) break;
			
			//case NPCID:        SET_NPC_VAR_INT(id) break; ~Disallowed
		case NPCDP:
			SET_NPC_VAR_INT(dp) break;
			
		case NPCTYPE:
		{
			SET_NPC_VAR_INT(family) break;
		}
		
		case NPCWDP:
			SET_NPC_VAR_INT(wdp) break;
			
		case NPCITEMSET:
			SET_NPC_VAR_INT(item_set) break;
			
		case NPCBOSSPAL:
			SET_NPC_VAR_INT(bosspal) break;
			
		case NPCBGSFX:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				enemy *en=GuyH::getNPC();
				int32_t newSFX = value / 10000;
				
				// Stop the old sound and start the new one
				if(en->bgsfx != newSFX)
				{
					en->stop_bgsfx(GuyH::getNPCIndex(ri->guyref));
					cont_sfx(newSFX);
					en->bgsfx = newSFX;
				}
			}
			break;
			
			
		case NPCEXTEND:
			SET_NPC_VAR_INT(extend) break;
			
		case NPCHXOFS:
			SET_NPC_VAR_INT(hxofs) break;
			
		case NPCHYOFS:
			SET_NPC_VAR_INT(hyofs) break;
			
		case NPCHXSZ:
			SET_NPC_VAR_INT(hit_width) break;
			
		case NPCHYSZ:
			SET_NPC_VAR_INT(hit_height) break;
			
		case NPCHZSZ:
			SET_NPC_VAR_INT(hzsz) break;
			
		case NPCCOLLDET:
			SET_NPC_VAR_INT(scriptcoldet) break;
			
		case NPCENGINEANIMATE:
			SET_NPC_VAR_INT(do_animation) break;
			
		case NPCSTUN:
			SET_NPC_VAR_INT(stunclk) break;
			
		case NPCHUNGER:
			SET_NPC_VAR_INT(grumble) break;
		
		case NPCWEAPSPRITE:
			SET_NPC_VAR_INT(wpnsprite) break;
			
		case NPCCSET:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->cs = (value / 10000) & 0xF;
				if(GuyH::getNPC()->family == eeLEV) GuyH::getNPC()->dcset = (value / 10000) & 0xF;
			}
		}
		break;
		
		//Bounds on value
		case NPCTXSZ:
		{
			int32_t height = value / 10000;
			
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError &&
					BC::checkIndex(height, 0, 20) == SH::_NoError)
				GuyH::getNPC()->txsz = height;
		}
		break;
		
		case NPCTYSZ:
		{
			int32_t width = value / 10000;
			
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError &&
					BC::checkBounds(width, 0, 20) == SH::_NoError)
				GuyH::getNPC()->tysz = width;
		}
		break;
		
		case NPCOTILE:
		{
			int32_t tile = value / 10000;
			
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError &&
					BC::checkTile(tile) == SH::_NoError)
				GuyH::getNPC()->o_tile = tile;
		}
		break;
		
		case NPCTILE:
		{
			int32_t tile = value / 10000;
			
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError &&
					BC::checkTile(tile) == SH::_NoError)
				GuyH::getNPC()->tile = tile;
		}
		break;
		
		case NPCSCRIPTTILE:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->scripttile = vbound((value/10000),-1, NEWMAXTILES-1);
		}
		break;
		
		case NPCSCRIPTFLIP:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError )
				GuyH::getNPC()->scriptflip = vbound(value/10000, -1, 127);
		}
		break;
		
		case NPCWEAPON:
		{
			int32_t weapon = value / 10000;
			
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError &&
					BC::checkBounds(weapon, 0, MAXWPNS-1) == SH::_NoError)
			{
				GuyH::getNPC()->wpn = weapon;
			
				if ( get_qr(qr_SETENEMYWEAPONSPRITESONWPNCHANGE) )
				{
					GuyH::getNPC()->wpnsprite = FFCore.GetDefaultWeaponSprite(weapon);
				}
				if (get_qr(qr_SETENEMYWEAPONSOUNDSONWPNCHANGE))
				{
					GuyH::getNPC()->firesfx = FFCore.GetDefaultWeaponSFX(weapon);
				}
				//else GuyH::getNPC()->wpnsprite = FFCore.GetDefaultWeaponSprite(weapon); //just to test that this works. 
			}
		}
		break;

		case NPCPARENTUID:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->setParent(sprite::getByUID(value));
			}
			break;

		case NPCSCRIPT:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t q=0; q<8; q++)
						GuyH::getNPC()->initD[q] = 0;
				}
				GuyH::getNPC()->script = vbound((value/10000), 0, NUMSCRIPTGUYS-1);
				on_reassign_script_engine_data(ScriptType::NPC, ri->guyref);
			}
		}
		break;

		case NPCINVINC:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->hclk = (int32_t)value/10000;
		}
		break;
		
		case NPCSUPERMAN:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->superman = (int32_t)value/10000;
		}
		break;
		
		case NPCHASITEM:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->itemguy = (value/10000)?1:0;
		}
		break;
		
		case NPCRINGLEAD:
		{
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				GuyH::getNPC()->leader = (value/10000)?1:0;
		}
		break;

		case NPCFROZENTILE:
			SET_NPC_VAR_INT(frozentile); break;
		case NPCFROZENCSET:
			SET_NPC_VAR_INT(frozencset); break;
		case NPCFROZEN:
			SET_NPC_VAR_INT(frozenclock); break;

		case NPCFALLCLK:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(GuyH::getNPC()->fallclk != 0 && value == 0)
				{
					GuyH::getNPC()->cs = GuyH::getNPC()->o_cset;
					GuyH::getNPC()->tile = GuyH::getNPC()->o_tile;
				}
				else if(GuyH::getNPC()->fallclk == 0 && value != 0) GuyH::getNPC()->o_cset = GuyH::getNPC()->cs;
				GuyH::getNPC()->fallclk = vbound(value/10000,0,70);
			}
			break;
		case NPCFALLCMB:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case NPCDROWNCLK:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				if(GuyH::getNPC()->drownclk != 0 && value == 0)
				{
					GuyH::getNPC()->cs = GuyH::getNPC()->o_cset;
					GuyH::getNPC()->tile = GuyH::getNPC()->o_tile;
				}
				else if(GuyH::getNPC()->drownclk == 0 && value != 0) GuyH::getNPC()->o_cset = GuyH::getNPC()->cs;
				GuyH::getNPC()->drownclk = vbound(value/10000,0,70);
			}
			break;
		case NPCDROWNCMB:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
		case NPCFAKEZ:
			{
				if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
				{
					if(!never_in_air(GuyH::getNPC()->id))
					{
						if(value < 0)
							GuyH::getNPC()->fakez = 0_zf;
						else
							GuyH::getNPC()->fakez = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
							
						if(GuyH::hasHero())
							Hero.setFakeZfix(get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
					}
				}
			}
			break;

		case NPCGLOWRAD:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->glowRad = vbound(value/10000,0,255);
			}
			break;
		case NPCGLOWSHP:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->glowShape = vbound(value/10000,0,255);
			}
			break;
			
		case NPCSHADOWSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->spr_shadow = vbound(value/10000,0,255);
			}
			break;
		case NPCSPAWNSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->spr_spawn = vbound(value/10000,0,255);
			}
			break;
		case NPCDEATHSPR:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->spr_death = vbound(value/10000,0,255);
			}
			break;
		case NPCSWHOOKED:
			break; //read-only
		case NPCCANFLICKER:
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->setCanFlicker(value != 0);
			}
			break;
		case NPCFLICKERCOLOR:
			if (GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->flickercolor = vbound(value/10000,-1,255);
			}
			break;
		case NPCFLICKERTRANSP:
			if (GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->flickertransp = vbound(value / 10000, -1, 255);
			}
			break;

		case NPCFIRESFX:
			if (GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				GuyH::getNPC()->firesfx = vbound(value / 10000, 0, 255);
			}
			break;

		default: return false;
	}

	return true;
}

std::optional<int32_t> npc_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case NPCNAME:
			do_getnpcname();
			break;

		case NPCSTOPBGSFX:
			do_npc_stopbgsfx();
			break;
		
		case NPCATTACK:
			do_npcattack();
			break;
		
		case NPCNEWDIR:
			do_npc_newdir();
			break;
		
		case NPCCONSTWALK:
			do_npc_constwalk();
			break;
		
		
		
		case NPCVARWALK:
			do_npc_varwalk();
			break;
		
		case NPCVARWALK8:
			do_npc_varwalk8();
			break;
		
		case NPCCONSTWALK8:
			do_npc_constwalk8();
			break;
		
		case NPCHALTWALK:
			do_npc_haltwalk();
			break;
		
		case NPCHALTWALK8:
			do_npc_haltwalk8();
			break;
		
		case NPCFLOATWALK:
			do_npc_floatwalk();
			break;
		
		case NPCFIREBREATH:
			do_npc_breathefire();
			break;
		
		case NPCNEWDIR8:
			do_npc_newdir8();
			break;
		
		case NPCLINKINRANGE:
			do_npc_hero_in_range(false);
			break;
		
		case NPCCANMOVE:
			do_npc_canmove(false);
			break;
		
		case NPCHITWITH:
			do_npc_simulate_hit(false);
			break;
			
		case NPCKNOCKBACK:
			do_npc_knockback(false);
			break;
		
		case NPCADD:
			do_npc_add(false);
			break;
		
		case NPCMOVEPAUSED:
		{
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->is_move_paused() ? 10000 : 0;
			}
			break;
		}
		case NPCMOVE:
		{
			int32_t dir = ri->d[rINDEX] / 10000;
			zfix px = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->moveDir(dir, px, special) ? 10000 : 0;
			}
			break;
		}
		case NPCMOVEANGLE:
		{
			zfix degrees = zslongToFix(ri->d[rINDEX]);
			zfix px = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->moveAtAngle(degrees, px, special) ? 10000 : 0;
			}
			break;
		}
		case NPCMOVEXY:
		{
			zfix dx = zslongToFix(ri->d[rINDEX]);
			zfix dy = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->movexy(dx, dy, special) ? 10000 : 00;
			}
			break;
		}
		case NPCCANMOVEDIR:
		{
			int32_t dir = ri->d[rINDEX] / 10000;
			zfix px = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->can_moveDir(dir, px, special) ? 10000 : 0;
			}
			break;
		}
		case NPCCANMOVEANGLE:
		{
			zfix degrees = zslongToFix(ri->d[rINDEX]);
			zfix px = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->can_moveAtAngle(degrees, px, special) ? 10000 : 0;
			}
			break;
		}
		case NPCCANMOVEXY:
		{
			zfix dx = zslongToFix(ri->d[rINDEX]);
			zfix dy = zslongToFix(ri->d[rEXP2]);
			int32_t special = ri->d[rEXP1] / 10000;
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->can_movexy(dx, dy, special) ? 10000 : 0;
			}
			break;
		}
		case NPCCANPLACE:
		{
			ri->guyref = SH::read_stack(ri->sp + 6);
			ri->d[rEXP1] = 0;
			if(GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				zfix nx = zslongToFix(SH::read_stack(ri->sp + 5));
				zfix ny = zslongToFix(SH::read_stack(ri->sp + 4));
				int special = SH::read_stack(ri->sp + 3) / 10000;
				bool kb = SH::read_stack(ri->sp + 2)!=0;
				int nw = SH::read_stack(ri->sp + 1) / 10000;
				int nh = SH::read_stack(ri->sp + 0) / 10000;
				ri->d[rEXP1] = GuyH::getNPC()->scr_canplace(nx, ny, special, kb, nw, nh) ? 10000 : 0;
			}
			break;
		}
		case NPCISFLICKERFRAME:
		{
			ri->d[rEXP1] = 0;
			if (GuyH::loadNPC(ri->guyref) == SH::_NoError)
			{
				ri->d[rEXP1] = GuyH::getNPC()->is_hitflickerframe(get_qr(qr_OLDSPRITEDRAWS)) ? 10000 : 0;
			}
			break;
		}
		case NPCDEAD:
			do_isdeadnpc();
			break;
		
		case NPCCANSLIDE:
			do_canslidenpc();
			break;
		
		case NPCSLIDE:
			do_slidenpc();
			break;
		
		case NPCKICKBUCKET:
		{
			FFScript::deallocateAllScriptOwned(ScriptType::NPC, ri->guyref);
			if (type == ScriptType::NPC && ri->guyref == i)
			{
				do_npc_delete();
				return RUNSCRIPT_SELFDELETE;
			}

			do_npc_delete();
			break;
		}
		case BREAKSHIELD:
			do_breakshield();
			break;

		case REPAIRSHIELD:
			do_repairshield();
			break;

		// TODO: oops, these two should be in a "npcdata.cpp" file.
		case NPCDATAGETNAME:
			do_getnpcdata_getname();
			break;
		
		case NPCGETINITDLABEL:
			get_npcdata_initd_label(false);
			break;

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

// NPC arrays.

static ArrayRegistrar NPCINITD_registrar(NPCINITD, []{
	static ScriptingArray_ObjectMemberCArray<enemy, &enemy::initD> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar NPCMISCD_registrar(NPCMISCD, []{
	static ScriptingArray_ObjectMemberCArray<enemy, &enemy::miscellaneous> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar NPCDD_registrar(NPCDD, []{
	static ScriptingArray_ObjectComputed<enemy, int32_t> impl(
		[](enemy* enemy){ return FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ? 32 : 16; },
		[](enemy* enemy, int index){ return enemy->get_dmisc(index); },
		[](enemy* enemy, int index, int value){ enemy->set_dmisc(index, value); }
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCSCRDEFENSED_registrar(NPCSCRDEFENSED, []{
	static ScriptingArray_ObjectComputed<enemy, byte> impl(
		[](enemy* enemy){ return edefSCRIPTDEFS_MAX + 1; },
		[](enemy* enemy, int index){ return enemy->defense[index + edefSCRIPT01]; },
		[](enemy* enemy, int index, byte value){ enemy->defense[index + edefSCRIPT01] = value; }
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDEFENSED_registrar(NPCDEFENSED, []{
	static ScriptingArray_ObjectMemberCArray<enemy, &enemy::defense> impl;
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	impl.setSideEffect([](enemy* enemy, int index, int value){
		if (!get_qr(qr_250WRITEEDEFSCRIPT)) return;
		if (index != edefSCRIPT) return;

		for (int sd = edefSCRIPT01; sd <= edefSCRIPT10; sd++)
			enemy->defense[sd] = value;
	});
	return &impl;
}());

static ArrayRegistrar NPCHITBY_registrar(NPCHITBY, []{
	static ScriptingArray_ObjectComputed<enemy, int32_t> impl(
		[](enemy* enemy){ return NUM_HIT_TYPES_USED; },
		[](enemy* enemy, int index){
			switch (index)
			{
				//screen indixes
				case 0:
				case 1:
				case 2:
				case 3:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 16:
					return enemy->hitby[index] * 10000; // * 10000; //do not multiply by 10000! UIDs are not *10000!

				//UIDs
				case 4:
				case 5:
				case 6:
				case 7:
				case 13:
				case 14:
				case 15:
					return enemy->hitby[index]; // * 10000; //do not multiply by 10000! UIDs are not *10000!

				default: NOTREACHED();
			}
		},
		[](enemy* enemy, int index, int value){
			switch (index)
			{
				//screen index objects
				case 0:
				case 1:
				case 2:
				case 3:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 16:
				{
					enemy->hitby[index] = vbound((value / 10000),0,255); //Once again, why did I vbound this, and why did I allow it to be written? UIDs are LONGs, with a starting value of 0.0001. -Z
					break;
				}

				//UIDs
				case 4:
				case 5:
				case 6:
				case 7:
				case 13:
				case 14:
				case 15:
				{
					enemy->hitby[index] = value; //Once again, why did I vbound this, and why did I allow it to be written? UIDs are LONGs, with a starting value of 0.0001. -Z
					break;
				}
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar NPCMOVEFLAGS_registrar(NPCMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<enemy, &enemy::moveflags, 16> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCBEHAVIOUR_registrar(NPCBEHAVIOUR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<enemy, &enemy::editorflags, 16> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCSHIELD_registrar(NPCSHIELD, []{
	static ScriptingArray_ObjectComputed<enemy, bool> impl(
		[](enemy* enemy){ return 5; },
		[](enemy* enemy, int index) -> bool {
			switch (index)
			{
				case 0: return enemy->flags&guy_shield_front;
				case 1: return enemy->flags&guy_shield_left;
				case 2: return enemy->flags&guy_shield_right;
				case 3: return enemy->flags&guy_shield_back;
				case 4: return enemy->flags&guy_bkshield;
				default: NOTREACHED();
			}
		},
		[](enemy* enemy, int index, bool value){
			switch (index)
			{
				case 0: SETFLAG(enemy->flags, guy_shield_front, value); break;
				case 1: SETFLAG(enemy->flags, guy_shield_left, value); break;
				case 2: SETFLAG(enemy->flags, guy_shield_right, value); break;
				case 3: SETFLAG(enemy->flags, guy_shield_back, value); break;
				case 4: SETFLAG(enemy->flags, guy_bkshield, value); break;
				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());
