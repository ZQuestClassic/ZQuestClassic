//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "sprite.h"
#include "zelda.h"
#include "maps.h"
#include "tiles.h"
#include "ffscript.h"

extern FFScript FFCore;
/*
void sprite::check_conveyor()
{
  if (conveyclk<=0)
  {
    int32_t ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
    if((ctype>=cOLD_CVUP) && (ctype<=cOLD_CVRIGHT))
    {
      switch (ctype-cOLD_CVUP)
      {
        case up:
        if(!_walkflag(x,y+8-2,2))
        {
          y=y-2;
        }
        break;
        case down:
        if(!_walkflag(x,y+15+2,2))
        {
          y=y+2;
        }
        break;
        case left:
        if(!_walkflag(x-2,y+8,1))
        {
          x=x-2;
        }
        break;
        case right:
        if(!_walkflag(x+15+2,y+8,1))
        {
          x=x+2;
        }
        break;
      }
    }
  }
}
*/

void sprite::handle_sprlighting()
{
	if(!(tmpscr.flags & fDARK)) return;
	if(!get_bit(quest_rules, qr_NEW_DARKROOM)) return;
	if(!glowRad) return;
	switch(glowShape)
	{
		case 0:
			doDarkroomCircle(x.getInt()+(hxsz/2), y.getInt()+(hysz/2), glowRad);
			break;
		case 1:
			doDarkroomCone(x.getInt()+(hxsz/2), y.getInt()+(hysz/2), glowRad, NORMAL_DIR(dir));
			break;
	}
}

bool is_conveyor(int32_t type)
{
	auto& cc = combo_class_buf[type];
	return cc.conveyor_x_speed || cc.conveyor_y_speed;
}

int32_t get_conveyor(int32_t x, int32_t y)
{
	x = vbound(x, 0, world_w - 1);
	y = vbound(y, 0, world_h - 1);
	int32_t cmbid = MAPCOMBO(x,y);
	newcombo const* cmb = &combobuf[cmbid];
	if (!_effectflag(x,y,1,-1, true))
	{
		cmbid = -1;
		cmb = NULL;
	}
	if(get_bit(quest_rules, qr_CONVEYORS_L1_L2))
		for (int32_t i = 0; i <= 1; ++i)
		{
			mapscr* layer_scr = get_layer_scr_for_xy(x, y, i);
			if (!layer_scr->valid) continue;
			
			auto tcid = MAPCOMBO2(i,x,y);
			if(is_conveyor(combobuf[tcid].type))
			{
				if (_effectflag_layer(x,y,1,layer_scr,true))
				{
					cmbid = tcid;
					cmb = &combobuf[tcid];
				}
				else
				{
					cmbid = -1;
					cmb = NULL;
				}
			}
		}
	if(!cmb) return -1;
	bool custom_spd = (cmb->usrflags&cflag2);
	if(custom_spd || conveyclk<=0)
	{
		int32_t ctype=cmb->type;
		for (int32_t i = 0; i <= 1; ++i)
		{
			auto tcid = MAPCOMBO2(i,x,y);
			if(combobuf[tcid].type == cBRIDGE)
			{
				if (get_bit(quest_rules, qr_OLD_BRIDGE_COMBOS))
				{
					if (!_walkflag_layer(x,y,i)) return -1;
				}
				else
				{
					if (_effectflag_layer(x,y,i,1,true)) return -1;
				}
			}
		}
		auto rate = custom_spd ? zc_max(cmb->attribytes[0], 1) : 3;
		if(custom_spd && (newconveyorclk % rate)) return -1;
		return cmbid;
	}
	return -1;
}

void sprite::check_conveyor()
{
    int32_t deltax=0;
    int32_t deltay=0;
    int32_t cmbid = get_conveyor(x+8,y+8);
	if(cmbid < 0) return;
	newcombo const* cmb = &combobuf[cmbid];
	bool custom_spd = (cmb->usrflags&cflag2);
    if(((z==0&&fakez==0) || (tmpscr.flags2&fAIRCOMBOS)))
    {
        int32_t ctype=(combobuf[cmbid].type);
        deltax=combo_class_buf[ctype].conveyor_x_speed;
        deltay=combo_class_buf[ctype].conveyor_y_speed;
		if (is_conveyor(ctype) && custom_spd)
		{
			deltax = zslongToFix(cmb->attributes[0]);
			deltay = zslongToFix(cmb->attributes[1]);
		}
        if(deltax!=0||deltay!=0)
        {
            if(deltay<0&&!_walkflag(x,y+8-2,2))
            {
                y=y-abs(deltay);
            }
            else if(deltay>0&&!_walkflag(x,y+15+2,2))
            {
                y=y+abs(deltay);
            }
            
            if(deltax<0&&!_walkflag(x-2,y+8,1))
            {
                x=x-abs(deltax);
            }
            else if(deltax>0&&!_walkflag(x+15+2,y+8,1))
            {
                x=x+abs(deltax);
            }
        }
    }
}

void movingblock::clear()
{
	trigger = bhole = force_many = false;
	endx=x=endy=y=0;
	dir=-1;
	oldflag=0;
	bcombo = 0;
	oldcset = 0;
	cs = 0;
	tile = 0;
	flip = 0;
	blockLayer = 0;
	clk = 0;
	step = 0;
}

void movingblock::set(int32_t X, int32_t Y, int32_t combo, int32_t cset, int32_t layer, int32_t placedfl)
{
	endx=x=X;
	endy=y=Y;
	bcombo = combo;
	oldcset=cs=cset;
	blockLayer=layer;
	oldflag=placedfl;
}

void movingblock::push(zfix bx,zfix by,int32_t d2,int32_t f)
{
	step = 0.5;
    trigger=false;
    endx=x=bx;
    endy=y=by;
    dir=d2;
    oldflag=f;
	rpos_t rpos = COMBOPOS_REGION(x.getInt(), y.getInt());
	size_t combopos = RPOS_TO_POS(rpos);
	auto pos_handle = get_pos_handle(rpos, blockLayer);
	mapscr *m = pos_handle.screen;
    word *di = &(m->data[combopos]);
    byte *ci = &(m->cset[combopos]);
    bcombo =  m->data[combopos];
    oldcset = m->cset[combopos];
    cs     = (isdungeon() && !get_bit(quest_rules, qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
    tile = combobuf[bcombo].tile;
    flip = combobuf[bcombo].flip;
    //   cs = ((*di)&0x700)>>8;
    *di = m->undercombo;
    *ci = m->undercset;
	// TODO z3
	FFCore.reset_combo_script(blockLayer, combopos);
    putcombo(scrollbuf,x-viewport.x,y-viewport.y,*di,*ci);
    clk=32;
    blockmoving=true;
}

bool is_push_flag(int32_t flag)
{
	switch(flag)
	{
		case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
		case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
		case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
		case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
		case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
		case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
		case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
			return true;
	}
	return false;
}

bool is_push(mapscr* m, int32_t pos)
{
	if(is_push_flag(m->sflag[pos]))
		return true;
	newcombo const& cmb = combobuf[m->data[pos]];
	if(is_push_flag(cmb.flag))
		return true;
	if(cmb.type == cSWITCHHOOK && (cmb.usrflags&cflag7))
		return true; //Counts as 'pushblock' flag
	return false;
}

bool movingblock::animate(int32_t)
{
	if (x > world_w || y > world_h)
	{
		// TODO z3
		// movingblock keeps animating even when moving to a different screen ...
		return false;
	}

	auto pos_handle = get_pos_handle_for_world_xy(x, y, blockLayer);
	mapscr* m = pos_handle.screen;

	if(fallclk)
	{
		if(fallclk == PITFALL_FALL_FRAMES)
			sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			blockmoving=false;
		}
		clk = 0;
		solid_update(false);
		return false;
	}
	if(drownclk)
	{
		//if(drownclk == WATER_DROWN_FRAMES)
		//sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
		//!TODO: Drown SFX
		if(!--drownclk)
		{
			blockmoving=false;
		}
		clk = 0;
		solid_update(false);
		return false;
	}
	if(clk<=0)
	{
		solid_update(false);
		return false;
	}
		
	move(step);
	
	if(--clk==0)
	{
		// TODO z3 just set pos handle to this above.
		auto base_pos_handle = get_pos_handle_for_world_xy(x, y, 0);

		trigger = false; bhole = false;
		blockmoving=false;
		
		if((fallCombo = getpitfall(x+8,y+8)))
		{
			fallclk = PITFALL_FALL_FRAMES;
		}
		/*
		//!TODO: Moving Block Drowning
		if(drownCombo = iswaterex_z3(MAPCOMBO(x+8,y+8), -1, x+8,y+8, false, false, true))
		{
			drownclk = WATER_DROWN_FRAMES;
		}
		*/
		size_t combopos = RPOS_TO_POS(pos_handle.rpos);
		int32_t f1 = m->sflag[combopos];
		int32_t f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
		auto maxLayer = get_bit(quest_rules, qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
		bool no_trig_replace = get_bit(quest_rules, qr_BLOCKS_DONT_LOCK_OTHER_LAYERS);
		bool trig_hole_same_only = get_bit(quest_rules, qr_BLOCKHOLE_SAME_ONLY);
		bool trig_is_layer = false;
		if(!fallclk && !drownclk)
		{
			m->data[combopos]=bcombo;
			m->cset[combopos]=oldcset;
			FFCore.reset_combo_script(blockLayer, combopos);
		}
		if(!fallclk && !drownclk)
		{
			if((f1==mfBLOCKTRIGGER)||f2==mfBLOCKTRIGGER)
			{
				trigger = true;
			}
			else if(!trig_hole_same_only)
			{
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					if(lyr==blockLayer) continue;

					mapscr* m0 = get_screen_layer_for_xy_offset(x, y, lyr);
					if (m0->sflag[combopos] == mfBLOCKTRIGGER
						|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
					{
						trigger = true;
						trig_is_layer = true;
						if(!no_trig_replace)
						{
							mapscr* m2 = get_screen_layer_for_xy_offset(x, y, lyr);
							m2->data[combopos] = m2->undercombo;
							m2->cset[combopos] = m2->undercset;
							m2->sflag[combopos] = 0;
						}
					}
				}
			}
			if(trigger)
			{
				if(!(no_trig_replace && trig_is_layer))
					m->sflag[combopos]=mfPUSHED;
			}
		}
		
		if((f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE)
		{
			m->data[combopos]+=1;
			bhole=true;
		}
		else if(!trig_hole_same_only)
		{
			for(auto lyr = 0; lyr <= maxLayer; ++lyr)
			{
				if(lyr==blockLayer) continue;
				mapscr* m2 = get_screen_layer_for_xy_offset(x, y, lyr);
				if((m2->sflag[combopos]==mfBLOCKHOLE)
					|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
				{
					m->data[combopos] = m->undercombo;
					m->cset[combopos] = m->undercset;
					m2->data[combopos] = bcombo+1;
					m2->cset[combopos] = oldcset;
					m2->sflag[combopos] = mfNONE;
					bhole=true;
					break;
				}
			}
		}
		if(bhole)
		{
			m->sflag[combopos]=mfNONE;
			if(fallclk||drownclk)
			{
				fallclk = 0;
				drownclk = 0;
				return false;
			}
		}
		else if(!fallclk&&!drownclk)
		{
			f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
			
			if(!(force_many || (f2==mfPUSHUDINS && dir<=down) ||
					(f2==mfPUSHLRINS && dir>=left) ||
					(f2==mfPUSHUINS && dir==up) ||
					(f2==mfPUSHDINS && dir==down) ||
					(f2==mfPUSHLINS && dir==left) ||
					(f2==mfPUSHRINS && dir==right) ||
					(f2==mfPUSH4INS)))
			{
				m->sflag[combopos]=mfPUSHED;
			}
		}
		if(fallclk||drownclk) return false;
		
		bool didtrigger = trigger;
		if(didtrigger)
		{
			for(auto lyr = 0; lyr <= maxLayer; ++lyr)
			{
				mapscr *tmp = get_screen_layer_for_xy_offset(x, y, lyr);
				for(int32_t pos=0; pos<176; pos++)
				{
					if((!trig_hole_same_only || lyr == blockLayer) && pos == combopos)
						continue;
					if(tmp->sflag[pos]==mfBLOCKTRIGGER
						|| combobuf[tmp->data[pos]].flag==mfBLOCKTRIGGER)
					{
						bool found = false;
						if(no_trig_replace)
							for(auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								mapscr *tmp2 = get_screen_layer_for_xy_offset(x, y, lyr2);
								if (is_push(tmp2, pos))
								{
									found = true;
									break;
								}
							}
						if(!found)
						{
							didtrigger=false;
							break;
						}
					}
				}
				if(!didtrigger) break;
			}
		}
		
		if(oldflag>=mfPUSHUDINS && !(trigger && !(no_trig_replace && trig_is_layer))
			&& !bhole)
		{
			m->sflag[combopos]=oldflag;
		}
		
		//triggers a secret
		f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
		
		if((oldflag==mfPUSH4 ||
			(oldflag==mfPUSHUD && dir<=down) ||
			(oldflag==mfPUSHLR && dir>=left) ||
			(oldflag==mfPUSHU && dir==up) ||
			(oldflag==mfPUSHD && dir==down) ||
			(oldflag==mfPUSHL && dir==left) ||
			(oldflag==mfPUSHR && dir==right) ||
			f2==mfPUSH4 ||
			(f2==mfPUSHUD && dir<=down) ||
			(f2==mfPUSHLR && dir>=left) ||
			(f2==mfPUSHU && dir==up) ||
			(f2==mfPUSHD && dir==down) ||
			(f2==mfPUSHL && dir==left) ||
			(f2==mfPUSHR && dir==right)) ||
		   didtrigger)
		{
			if(didtrigger && no_trig_replace)
			{
				//Lock in place all blocks on triggers,
				// and replace triggers with undercombo.
				// 'no_trig_replace' delays this to now, instead of
				// happening as each combo is placed.
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					mapscr* tmp = get_screen_layer_for_xy_offset(x, y, lyr);
					for(int32_t pos=0; pos<176; pos++)
					{
						if(tmp->sflag[pos]==mfBLOCKTRIGGER
							|| combobuf[tmp->data[pos]].flag==mfBLOCKTRIGGER)
						{
							for(auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								if(lyr2 == lyr) continue;

								mapscr* tmp2 = get_screen_layer_for_xy_offset(x, y, lyr2);
								if (is_push(tmp2, pos))
								{
									tmp2->sflag[pos] = mfPUSHED;
								}
							}
							tmp->data[pos] = tmp->undercombo;
							tmp->cset[pos] = tmp->undercset;
							tmp->sflag[pos] = 0;
						}
					}
				}
			}
			
			if(hiddenstair2(base_pos_handle.screen,true))
			{
				sfx(base_pos_handle.screen->secretsfx);
			}
			else
			{
				trigger_secrets_for_screen(base_pos_handle.screen_index, true);
				
				if((combobuf[bcombo].type == cPUSH_WAIT) ||
						(combobuf[bcombo].type == cPUSH_HW) ||
						(combobuf[bcombo].type == cPUSH_HW2) || didtrigger)
				{
					sfx(base_pos_handle.screen->secretsfx);
				}
			}
			
			if (isdungeon() && base_pos_handle.screen->flags&fSHUTTERS)
			{
				opendoors=8;
			}
			
			if(canPermSecret(currdmap, base_pos_handle.screen_index))
			{
				if(get_bit(quest_rules, qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
					(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
						|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
				{
					if (!(base_pos_handle.screen->flags5&fTEMPSECRETS)) setmapflag2(base_pos_handle.screen, base_pos_handle.screen_index, mSECRET);
				}
			}
		}
		
		putcombo(scrollbuf,x,y,bcombo,cs);
	}
	
	return false;
}

/*** end of sprite.cc ***/

