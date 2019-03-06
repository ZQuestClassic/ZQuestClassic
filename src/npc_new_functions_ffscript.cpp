void FFScript::do_isdeadnpc()
{
	enemyclass enemy;
	//enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
	if(GuyH::loadNPC(ri->guyref, "npc->isDead") != SH::_NoError)
	{
		int dead = enemyclass.Dead(getNPCIndex(ri->guysref));
		set_register(sarg1, (dead ? 10000 : 0));
	}
	else set_register(sarg1, -10000);
}


void FFScript::do_canslidenpc()
{
	
	if(GuyH::loadNPC(ri->guyref, "npc->CanSlide") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		bool candoit = e->can_slide();
		set_register(sarg1, (candoit ? 10000 : 0));
	}
	else set_register(sarg1, -10000);
}

void FFScript::do_slidenpc()
{
	
	if(GuyH::loadNPC(ri->guyref, "npc->Slide()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		bool candoit = e->slide();
		set_register(sarg1, (candoit ? 10000 : 0));
	}
	else set_register(sarg1, -10000);
}

void FFScript::do_npckickbucket()
{
	
	if(GuyH::loadNPC(ri->guyref, "npc->Remove()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		e->kickbucket();
	}
	//else Z_scripterrlog
}

void FFScript::do_npc_stopbgsfx()
{
	//enemyclass enemy;
	//enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
	if(GuyH::loadNPC(ri->guyref, "npc->StopBGSFX()") != SH::_NoError)
	{
		enemyclass.stop_bgsfx(GuyH::getNPCIndex(ri->guysref));
	}
}

void FFScript::do_npcattack()
{
	if(GuyH::loadNPC(ri->guyref, "npc->Attack()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		e->FireWeapon();
	}
}
void FFScript::do_npc_newdir()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->NewDir()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz > 0 ) 
		{
			if ( sz > 3 ) 
			{
				Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
				return;
			}
			e->newdir( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000) );
		}
		else e->newdir();
		
		
	}
}

void FFScript::do_npc_constwalk()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->ConstantWalk()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz > 0 ) 
		{
			if ( sz > 3 ) 
			{
				Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
				return;
			}
			e->constant_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000) );
		}
		else e->constant_walk();
		
		
	}
}

void FFScript::do_npc_varwalk()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->VariableWalk()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 3 ) 
		{
			
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000) );
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
		
		
	}
}

void FFScript::do_npc_varwalk8()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	//void variable_walk_8(int rate,int homing,int newclk,int special);
	// same as above but with variable enemy size
	//void variable_walk_8(int rate,int homing,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
    
	
	if(GuyH::loadNPC(ri->guyref, "npc->VariableWalk8()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 4 ) 
		{
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000) );
		}
		else if ( sz == 8 ) 
		{
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000),
				(FFCore.getElement(arrayptr, 4)/10000), (FFCore.getElement(arrayptr, 5)/10000),
				(FFCore.getElement(arrayptr, 6)/10000), (FFCore.getElement(arrayptr, 7)/10000),
			);
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
	}
}

void FFScript::do_npc_constwalk8()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	//void variable_walk_8(int rate,int homing,int newclk,int special);
	// same as above but with variable enemy size
	//void variable_walk_8(int rate,int homing,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
    
	
	if(GuyH::loadNPC(ri->guyref, "npc->ConstantWalk8()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 3 ) 
		{
			e->constant_walk_8( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000) );
		}
		
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
	}
}


void FFScript::do_npc_haltwalk()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->HaltingWalk()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 5 ) 
		{
			
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000),
				(FFCore.getElement(arrayptr, 4)/10000));
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
		
		
	}
}

void FFScript::do_npc_haltwalk8()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->HaltingWalk8()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 6 ) 
		{
			
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000),
				(FFCore.getElement(arrayptr, 4)/10000),(FFCore.getElement(arrayptr, 5)/10000));
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
		
		
	}
}


void FFScript::do_npc_floatwalk()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->FloatingWalk()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 3 ) 
		{
			
			e->floater_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(fix)(FFCore.getElement(arrayptr, 2)/10000));
		
		}
		else if ( sz == 7 ) 
		{
			
			e->variable_walk( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(fix)(FFCore.getElement(arrayptr, 2)/10000), (fix)(FFCore.getElement(arrayptr, 3)/10000),
				(FFCore.getElement(arrayptr, 4)/10000),(FFCore.getElement(arrayptr, 5)/10000),
				(FFCore.getElement(arrayptr, 6)/10000));
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
		
		
	}
}

void FFScript::do_npc_breathefire()
{
	bool seek = (get_register(sarg2));
	if(GuyH::loadNPC(ri->guyref, "npc->BreathAttack()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		e->FireBreath(seek);
		
	}
}


void FFScript::do_npc_newdir8()
{
	long arrayptr = get_register(sarg2) / 10000;
	int sz = FFCore.getSize(arrayptr);
	 //(FFCore.getElement(sdci[2]/10000, q))/10000;
	
	if(GuyH::loadNPC(ri->guyref, "npc->NewDir8()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		
		if ( sz == 3 ) 
		{
			
			e->newdir_8( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000));
		
		}
		else if ( sz == 7 ) 
		{
			
			e->newdir_8( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000),
				(FFCore.getElement(arrayptr, 4)/10000),(FFCore.getElement(arrayptr, 5)/10000),
				(FFCore.getElement(arrayptr, 6)/10000));
		}
		else Z_scripterrlog("Invalid array size (%d) passed to npc->VariableWalk(int arr[])\n",sz);
		
		
	}
}

	
long FFScript::npc_collision()
{
	if(GuyH::loadNPC(ri->guyref, "npc->Collision()") != SH::_NoError)
	{
		long _obj_type = (ri->d[0] / 10000);
		long _obj_ptr = (ri->d[1]);
		long isColl = 0;
		switch(_obj_type)
		{
			case obj_type_lweapon:
			{
				Z_scripterrlog("Checking collision on npc (%d) against lweapon (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			case obj_type_eweapon:
			{
				Z_scripterrlog("Checking collision on npc (%d) against eweapon (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			case obj_type_npc:
			{
				Z_scripterrlog("Checking collision on npc (%d) against npc (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			case obj_type_link:
			{
				Z_scripterrlog("Checking collision on npc (%d) against Player\n", ri->guysref);
				isColl = 0;
				break;
			}
			case obj_type_ffc:
			{
				_obj_ptr *= 10000; _obj_ptr -= 1;
				Z_scripterrlog("Checking collision on npc (%d) against ffc (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			case obj_type_combo_pos:
			{
				_obj_ptr *= 10000;
				Z_scripterrlog("Checking collision on npc (%d) against combo position (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			case obj_type_item:
			{
				Z_scripterrlog("Checking collision on npc (%d) against item (%d)\n", ri->guysref, _obj_ptr);
				isColl = 0;
				break;
			}
			default: 
			{
				Z_scripterrlog("Invalid object type (%d) passed to npc->Collidion(int type, int ptr)\n", _obj_type);
				isColl = 0;
				break;
			}
		}
	}
	
    return isColl;
}


long FFScript::npc_linedup()
{
	if(GuyH::loadNPC(ri->guyref, "npc->LinedUp()") != SH::_NoError)
	{
		long range = (ri->d[0] / 10000);
		bool dir8 = (ri->d[1]);
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		return (long)e->lined_up(range,dir8);
	}
	
    return 0;
}


void FFScript::do_npc_link_in_range(const bool v)
{
	int dist = (int)SH::get_arg(sarg1, v) / 10000;
	bool in_range = false;
	if(GuyH::loadNPC(ri->guyref, "npc->LinedUp()") != SH::_NoError)
	{
		long range = (ri->d[0] / 10000);
		bool dir8 = (ri->d[1]);
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		in_range = (e->LinkInRange(dist));
	}
	set_register(sarg1, ( in_range ? 10000 : 0));
}



void FFScript::do_npc_simulate_hit(const bool v)
{
	a
	bool ishit = false;
	
	if(GuyH::loadNPC(ri->guyref, "npc->SimulateHit()") != SH::_NoError)
	{
		Z_scripterrlog("Trying to simulate a hit on npc\n");
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		if ( sz == 2 ) //type and pointer
		{
			int type = FFCore.getElement(arrayptr, 0)/10000;
			
			//switch(type)
			//{
			//	case simulate_hit_type_weapon:
			//	{
			//		ishit = e->hit(*);
			//		break;
			//	}
			//	case simulate_hit_type_sprite:
			//	{
			//		ishit = e->hit(*);
			//		break;
			//	}
			//}
			ishit = false;
		}
		if ( sz == 6 ) //hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
		{
			ishit = e->hit( (FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000),
				(FFCore.getElement(arrayptr, 2)/10000), (FFCore.getElement(arrayptr, 3)/10000), 
				(FFCore.getElement(arrayptr, 4)/10000), (FFCore.getElement(arrayptr, 5)/10000) );			
			
		}
		else 
		{
			Z_scripterrlog("Invalid array size (%d) passed to npc->SimulateHit(). The array size must be [1] or [3].\n". sz);
			ishit = false;
		}
	}
	set_register(sarg1, ( ishit ? 10000 : 0));
}


void FFScript::do_npc_add(const bool v)
{
   
	long arrayptr = SH::get_arg(sarg1, v) / 10000;
	int sz = FFCore.getSize(arrayptr);
	
	int id = 0, nx = 0, ny = 0, clk = -10;
	
	if ( sz < 1 ) 
	{
		Z_scripterrlog("Invalid array size (%d) passed to npc->Create(). The array size must be [1] or [3].\n". sz);
		return;
	}
	else //size is valid
	{
		id = (FFCore.getElement(arrayptr, 0)/10000);
		
		if ( sz == 3 ) //x and y
		{
			nx = (FFCore.getElement(arrayptr, 1)/10000);
			ny = (FFCore.getElement(arrayptr, 2)/10000);
		}
	}
	
    
	if(BC::checkGuyID(id, "npc->Create()") != SH::_NoError)
		return;
        
	//If we make a segmented enemy there'll be more than one sprite created
	word numcreated = addenemy(nx, ny, id, -10);
    
	if(numcreated == 0)
	{
		ri->guyref = LONG_MAX;
		Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", guy_string[id]);
	}
	else
	{
		word index = guys.Count() - numcreated; //Get the main enemy, not a segment
		ri->guyref = guys.spr(index)->getUID();
        
		for(; index<guys.Count(); index++)
			((enemy*)guys.spr(index))->script_spawned=true;
            
		Z_eventlog("Script created NPC \"%s\" with UID = %ld\n", guy_string[id], ri->guyref);
	}
}

case NPCCANMOVE:
            FFCore.do_npc_canmove(false);
            break;


void FFScript::do_npc_canmove(const bool v)
{
	long arrayptr = SH::get_arg(sarg1, v) / 10000;
	int sz = FFCore.getSize(arrayptr);
	bool can_mv = false;
	if(GuyH::loadNPC(ri->guyref, "npc->LinedUp()") != SH::_NoError)
	{
		enemy *e = (enemy*)guys.spr(GuyH::getNPCIndex(ri->guysref));
		if ( sz == 1 ) //bool canmove(int ndir): dir only, uses 'step' IIRC
		{
			can_mv = e->canmove((FFCore.getElement(arrayptr, 0)/10000));
		}
		if ( sz == 2 ) //bool canmove(int ndir, int special): I think that this also uses the default 'step'
		{
			can_mv = e->canmove((FFCore.getElement(arrayptr, 0)/10000), (FFCore.getElement(arrayptr, 1)/10000));
		}
		if ( sz == 3 ) //bool canmove(int ndir,fix s,int special) : I'm pretty sure that 'fix s' is 'step' here. 
		{
			can_mv = e->canmove((FFCore.getElement(arrayptr, 0)/10000), (fix)(FFCore.getElement(arrayptr, 1)/10000), (FFCore.getElement(arrayptr, 2)/10000));
		}
		if ( sz == 7 ) //bool canmove(int ndir,fix s,int special) : I'm pretty sure that 'fix s' is 'step' here. 
		{
			can_mv = e->canmove((FFCore.getElement(arrayptr, 0)/10000), 
			(fix)(FFCore.getElement(arrayptr, 1)/10000), (FFCore.getElement(arrayptr, 2)/10000),
			(FFCore.getElement(arrayptr, 3)/10000), (FFCore.getElement(arrayptr, 4)/10000), 
			(FFCore.getElement(arrayptr, 5)/10000), (FFCore.getElement(arrayptr, 5)/10000)	);
		}
		else 
		{
			Z_scripterrlog("Invalid array size (%d) passed to npc->CanMove(). The array size must be [1], [2], [3], or [7].\n". sz);
			ishit = false;
		}
	}
	set_register(sarg1, ( can_mv ? 10000 : 0));
}
