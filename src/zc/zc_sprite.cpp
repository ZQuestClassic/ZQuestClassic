#include "base/qrs.h"
#include "sprite.h"
#include "zc/replay.h"
#include "zc/zelda.h"
#include "zc/maps.h"
#include "tiles.h"
#include "zc/ffscript.h"
#include "zc/combos.h"
#include "drawing.h"
#include "base/mapscr.h"
#include "iter.h"

void sprite::handle_sprlighting()
{
	if(!get_qr(qr_NEW_DARKROOM)) return;
	if(!is_any_room_dark) return;

	int x0 = x.getInt()+(hit_width/2);
	int y0 = y.getInt()+(hit_height/2);
	//if(!(get_scr_for_world_xy(x0, y0)->flags & fDARK)) return;

	handle_lighting(x0, y0 + playing_field_offset,glowShape,glowRad,dir, darkscr_bmp);
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
	int maxlayer = get_qr(qr_CONVEYORS_ALL_LAYERS) ? 6 : (get_qr(qr_CONVEYORS_L1_L2) ? 2 : 0);
	rpos_t rpos = COMBOPOS_REGION(x,y);
	int pos = RPOS_TO_POS(rpos);
	int found_layer = -1;
	int cmbid = -1;
	int ffc_cid = MAPFFCOMBO(x,y);
	if(maxlayer == 6 && ffc_cid && is_conveyor(combobuf[ffc_cid].type))
	{
		cmbid = ffc_cid;
		found_layer = 999;
	}
	else for(int q = maxlayer; q >= 0; --q)
	{
		mapscr* layer_scr = get_scr_for_world_xy_layer(x, y, q);
		if (!layer_scr->is_valid()) continue;

		int cid = layer_scr->data[pos];
		if(is_conveyor(combobuf[cid].type)
			&& _effectflag_layer(x,y,1,layer_scr,true))
		{
			found_layer = q;
			cmbid = cid;
			break;
		}
	}
	if(cmbid < 0) return -1;

	newcombo const& cmb = combobuf[cmbid];
	bool custom_spd = (cmb.usrflags&cflag2);
	if(custom_spd || conveyclk<=0)
	{
		for (int i = found_layer; i <= 1; ++i)
		{
			auto tcid = MAPCOMBO2(i,x,y);
			if(combobuf[tcid].type == cBRIDGE)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS))
				{
					if (!_walkflag_layer(x,y,i)) return -1;
				}
				else
				{
					if (_effectflag_layer(x,y,i,1,true)) return -1;
				}
			}
		}
		auto rate = custom_spd ? zc_max(cmb.attribytes[0], 1) : 3;
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
    if(((z==0&&fakez==0) || (get_scr_for_world_xy(x, y)->flags2&fAIRCOMBOS)))
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
	trigger = bhole = force_many = no_icy = new_block = false;
	fallclk = drownclk = 0;
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
	grav_falling = false;
	blockinfo.clear();
	solid_update(false);
}

void movingblock::set(int32_t X, int32_t Y, int32_t combo, int32_t cset, int32_t layer, int32_t placedfl)
{
	endx=x=X;
	endy=y=Y;
	bcombo = combo;
	oldcset=cs=cset;
	blockLayer=layer;
	oldflag=placedfl;
	solid_update(false);
}

void movingblock::push(zfix bx,zfix by,int32_t d2,int32_t f)
{
	new_block = false;
	step = 0.5;
	grav_falling = false;
	trigger=false;
	x=bx;
	y=by;
	dir=d2;
	xofs = 0;
	yofs = playing_field_offset;
    oldflag=f;
	rpos_t rpos = COMBOPOS_REGION(x.getInt(), y.getInt());
	size_t combopos = RPOS_TO_POS(rpos);
	auto rpos_handle = get_rpos_handle(rpos, blockLayer);
	mapscr *m = rpos_handle.scr;
    bcombo =  m->data[combopos];
    oldcset = m->cset[combopos];
    cs     = (isdungeon(m->screen) && !get_qr(qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
    tile = combobuf[bcombo].tile;
    flip = combobuf[bcombo].flip;
	bool antigrav = get_gravity() < 0;
	switch(dir)
	{
		case up:
			endx = x;
			endy = y;
			if (antigrav && check_side_fall(antigrav))
				grav_falling = true;
			endy = y-16;
			break;
		case down:
			endx = x;
			endy = y;
			if (!antigrav && check_side_fall(antigrav))
				grav_falling = true;
			endy = y+16;
			yofs = playing_field_offset - 0.5;
			break;
		case left:
			endx = x-16;
			endy = y;
			break;
		case right:
			endx = x+16;
			endy = y;
			xofs = -0.5;
			break;
		default:
			endx = x;
			endy = y;
	}
	// TODO(replays): remove in future bulk replay update. These are just visual changes.
	if (replay_version_check(0, 37))
	{
		xofs = 0;
		yofs = playing_field_offset;
	}
    //   cs = ((*di)&0x700)>>8;
    m->data[combopos] = m->undercombo;
    m->cset[combopos] = m->undercset;
	FFCore.clear_combo_script(rpos_handle);
    clk=32;
	if(!get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(true);
	solid_update(false);
}
void movingblock::push_new(zfix bx,zfix by,int d2,int f,zfix spd)
{
	new_block = true;
	step = spd;
	grav_falling = false;
	trigger=false;
	x=bx;
	y=by;
	dir=d2;
	xofs = 0;
	yofs = playing_field_offset;
    oldflag=f;
	rpos_t rpos = COMBOPOS_REGION(x.getInt(), y.getInt());
	auto rpos_handle = get_rpos_handle(rpos, blockLayer);
	int32_t combopos = RPOS_TO_POS(rpos);
	mapscr *m = rpos_handle.scr;
    bcombo =  m->data[combopos];
    oldcset = m->cset[combopos];
    cs     = (isdungeon(m->screen) && !get_qr(qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
    tile = combobuf[bcombo].tile;
    flip = combobuf[bcombo].flip;
	bool antigrav = get_gravity() < 0;
	switch(dir)
	{
		case up:
			endx = x;
			endy = y;
			if (antigrav && check_side_fall(antigrav))
				grav_falling = true;
			endy = y-16;
			break;
		case down:
			endx = x;
			endy = y;
			if (!antigrav && check_side_fall(antigrav))
				grav_falling = true;
			endy = y+16;
			yofs = playing_field_offset - 0.5;
			break;
		case left:
			endx = x-16;
			endy = y;
			break;
		case right:
			endx = x+16;
			endy = y;
			xofs = -0.5;
			break;
		default:
			endx = x;
			endy = y;
	}
	// TODO(replays): remove in future bulk replay update. These are just visual changes.
	if (replay_version_check(0, 37))
	{
		xofs = 0;
		yofs = playing_field_offset;
	}
    //   cs = ((*di)&0x700)>>8;
    m->data[combopos] = m->undercombo;
    m->cset[combopos] = m->undercset;
	FFCore.clear_combo_script(rpos_handle);
    clk=32;
	if(!get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(true);
	solid_update(false);
}

bool movingblock::check_hole() const
{
	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, blockLayer);
	if ((rpos_handle.sflag()==mfBLOCKHOLE)||MAPCOMBOFLAG2(blockLayer-1,x,y)==mfBLOCKHOLE)
		return true;

	if (!get_qr(qr_BLOCKHOLE_SAME_ONLY))
	{
		int maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
		for (int lyr = 0; lyr <= maxLayer; ++lyr)
		{
			if(lyr==blockLayer) continue;

			auto rpos_handle_2 = get_rpos_handle_for_screen(rpos_handle.screen, lyr, rpos_handle.pos);
			if ((rpos_handle_2.sflag()==mfBLOCKHOLE)
				|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
				return true;
		}
	}
	return false;
}

bool movingblock::check_side_fall(bool antigrav) const
{
	if(!isSideViewGravity()) return false;
	if(!(new_block ? (combobuf[bcombo].usrflags&cflag11) : get_qr(qr_PUSHBLOCKS_FALL_IN_SIDEVIEW)))
		return false;
	
	if (!get_gravity() && (!grav_falling || !step))
		return false; // no gravity to move it, and no motion happening, so can't fall
	
	int tx = endx, ty = endy + 8 + (antigrav ? -16 : 16);
	
	if(ty < 0 || ty >= world_h)	
		return false;
	auto rpos_handle = get_rpos_handle_for_world_xy(tx, ty, blockLayer);
	if(rpos_handle.sflag() == mfNOBLOCKS || rpos_handle.cflag() == mfNOBLOCKS)
		return false;
	if(!antigrav && checkSVLadderPlatform(tx, ty))
		return false;
	return !_walkflag(tx, ty, 2);
}

bool movingblock::check_trig() const
{
	auto rpos_handle = get_rpos_handle_for_world_xy(x, y, blockLayer);

	if(fallclk || drownclk)
		return false;
	if(rpos_handle.sflag() == mfBLOCKTRIGGER || rpos_handle.cflag() == mfBLOCKTRIGGER)
		return true;
	else if(!get_qr(qr_BLOCKHOLE_SAME_ONLY))
	{
		auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
		for(auto lyr = 0; lyr <= maxLayer; ++lyr)
		{
			if(lyr==blockLayer) continue;

			auto rpos_handle_2 = get_rpos_handle_for_screen(rpos_handle.screen, lyr, rpos_handle.pos);
			if(rpos_handle_2.sflag() == mfBLOCKTRIGGER
				|| rpos_handle_2.cflag() == mfBLOCKTRIGGER)
				return true;
		}
	}
	return false;
}

bool movingblock::active() const
{
	return clk > 0 || fallclk || drownclk;
}

bool movingblock::animate(int32_t)
{
	if (x > world_w || y > world_h)
	{
		return false;
	}
	update_current_screen();

	auto end_rpos_handle = get_rpos_handle_for_world_xy(endx, endy, blockLayer);
	int end_pos = end_rpos_handle.pos;
	mapscr* m = end_rpos_handle.scr;
	mapscr* m0 = get_scr_for_world_xy(endx, endy);
	if(get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(false);
	else setSolid(clk > 0 && !(fallclk || drownclk));
	newcombo const& block_cmb = combobuf[bcombo];
	if(fallclk)
	{
		if(fallclk == PITFALL_FALL_FRAMES)
		{
			int s = combobuf[fallCombo].attribytes[0];
			if(block_cmb.sfx_falling)
				s = block_cmb.sfx_falling;
			sfx(s, pan(x));
		}
		clk = 0;
		solid_update(false);
		if(!--fallclk)
			clear();
		return false;
	}
	if(drownclk)
	{
		if(drownclk == WATER_DROWN_FRAMES)
		{
			int s = combobuf[drownCombo].attribytes[4];
			if(combobuf[drownCombo].usrflags&cflag1)
			{
				if(block_cmb.sfx_lava_drowning)
					s = block_cmb.sfx_lava_drowning;
			}
			else if(block_cmb.sfx_drowning)
				s = block_cmb.sfx_drowning;
			sfx(s, pan(x));
		}
		clk = 0;
		solid_update(false);
		if(!--drownclk)
			clear();
		return false;
	}
	if(clk<=0)
	{
		solid_update(false);
		return false;
	}
	
	bool done = false;
	
	//Move
	move(step);
	zfix ox = x, oy = y; //grab the x/y after moving, before any snapping
	
	//Check if the block has reached the next grid-alignment
	if(new_block || grav_falling)
	{
		switch(dir)
		{
			case up:
				if(y <= endy)
					done = true;
				break;
			case down:
				if(y >= endy)
					done = true;
				break;
			case left:
				if(x <= endx)
					done = true;
				break;
			case right:
				if(x >= endx)
					done = true;
				break;
		}
		if(done)
		{
			x = endx;
			y = endy;
		}
	}
	else done = (--clk==0);
	
	//Check for sideview gravity
	zfix grav = get_gravity();
	if(grav_falling || (done && check_side_fall(grav < 0)))
	{
		zfix terminalv = get_terminalv().doAbs();
		auto grav_dir = (grav < 0 ? up : down);
		if (dir != grav_dir) // changing dir
		{
			if (grav_falling) // gravity changed?
			{
				if (dir != up && dir != down) // sanity check?
				{
					dir = grav_dir;
					step = 0;
				}
				else
				{
					step = zc_min(step - abs(grav), terminalv);
					if (step < 0) // falling negatively; flip direction
					{
						dir = grav_dir;
						step.doAbs();
					}
				}
			}
			else // just starting falling; reset step
			{
				dir = grav_dir;
				step = 0;
			}
		}
		else step = zc_min(step + abs(grav), terminalv);
		
		if(done && (!grav_falling || check_side_fall(dir == up)))
		{
			// was about to snap into place, but falls instead
			endy += (dir == up) ? -16 : 16; //already sanity checked in check_side_fall()
			grav_falling = true;
			done = false;
			clk = 32;
			
			end_rpos_handle = get_rpos_handle_for_world_xy(endx, endy, blockLayer);
			end_pos = end_rpos_handle.pos;
			m = end_rpos_handle.scr;
			m0 = get_scr_for_world_xy(endx, endy);
		}
		else // sanity check for gravity flipping
		{
			if (endy > y && dir == up)
				endy -= 16;
			else if(endy < y && dir == down)
				endy += 16;
		}
	}
	
	//Check if the block is falling into a pitfall (if aligned)
	if(done)
	{
		if((fallCombo = getpitfall(x+8,y+8)))
		{
			fallclk = PITFALL_FALL_FRAMES;
		}
		if(get_qr(qr_BLOCKS_DROWN))
			if((drownCombo = iswaterex_z3(MAPCOMBO(x+8,y+8), -1, x+8,y+8, false, false, true)))
			{
				drownclk = WATER_DROWN_FRAMES;
			}
	}
	
	//Check for icy blocks/floors that might continue the slide
	if(!grav_falling && done && !no_icy && !fallclk && !drownclk)
	{
		int icy_cid = get_icy(endx+8,endy+8 + ((isSideViewGravity() && !get_qr(qr_BROKEN_ICY_FLOOR_SIDEVIEW)) ? 16 : 0),ICY_BLOCK);
		if(new_block)
		{
			if(((block_cmb.usrflags&cflag7) || //icy blocks keep sliding?
				(!(block_cmb.usrflags&cflag10) && icy_cid)))
			{
				bool canslide = true;
				auto new_endx = endx, new_endy = endy;
				switch(dir)
				{
					case up:
						new_endy -= 16;
						break;
					case down:
						new_endy += 16;
						break;
					case left:
						new_endx -= 16;
						break;
					case right:
						new_endx += 16;
						break;
				}
				if(new_endx < 0 || new_endx > world_w-16 || new_endy < 0 || new_endy > world_h-8)
					canslide = false;
				else if(check_hole()) //Falls into block holes on the way
					canslide = false;
				else
				{
					bool solid = _walkflag(new_endx,new_endy+8,2);
					int pflag = MAPFLAG2(blockLayer-1,new_endx,new_endy+8);
					int iflag = MAPCOMBOFLAG2(blockLayer-1,new_endx,new_endy+8);
					if(get_qr(qr_SOLIDBLK))
					{
						if(solid && iflag != mfBLOCKHOLE && pflag != mfBLOCKHOLE)
							canslide = false;
					}
					if(iflag == mfNOBLOCKS || pflag == mfNOBLOCKS)
						canslide = false;
				}
				if(canslide)
				{
					done = false;
					endx = new_endx;
					endy = new_endy;

					end_rpos_handle = get_rpos_handle_for_world_xy(endx, endy, blockLayer);
					end_pos = end_rpos_handle.pos;
					m = end_rpos_handle.scr;
					m0 = get_scr_for_world_xy(endx, endy);
				}
			}
		}
		else if(icy_cid)
		{
			bool canslide = true;
			auto new_endx = endx, new_endy = endy;
			switch(dir)
			{
				case up:
					new_endy -= 16;
					break;
				case down:
					new_endy += 16;
					break;
				case left:
					new_endx -= 16;
					break;
				case right:
					new_endx += 16;
					break;
			}
			if(new_endx < 0 || new_endx > world_w-16 || new_endy < 0 || new_endy > world_h-8)
				canslide = false;
			else
			{
				bool solid = _walkflag(new_endx,new_endy+8,2);
				int pflag = MAPFLAG2(blockLayer-1,new_endx,new_endy+8);
				int iflag = MAPCOMBOFLAG2(blockLayer-1,new_endx,new_endy+8);
				if(get_qr(qr_SOLIDBLK))
				{
					if(solid || iflag == mfBLOCKHOLE || pflag == mfBLOCKHOLE)
						canslide = false;
				}
				if(iflag == mfNOBLOCKS || pflag == mfNOBLOCKS)
					canslide = false;
			}
			if(canslide)
			{
				done = false;
				endx = new_endx;
				endy = new_endy;
				clk = 32;

				end_rpos_handle = get_rpos_handle_for_world_xy(endx, endy, blockLayer);
				end_pos = end_rpos_handle.pos;
				m = end_rpos_handle.scr;
				m0 = get_scr_for_world_xy(endx, endy);
			}
		}
	}
	
	if(!done)
	{
		x = ox;
		y = oy;
	}
	solid_update(); //Handle solid object movement
	//Click the block into place, the push ended.
	if(done)
	{
		if (!end_rpos_handle.scr->is_valid())
		{
			Z_message("Push block error: destination screen does not exist. Check the screen's layers.\n");
		}

		if(new_block)
		{
			clk = 0;
			x = endx;
			y = endy;
			trigger = false; bhole = false;

			int f1 = end_rpos_handle.sflag();
			int f2 = end_rpos_handle.cflag();
			auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
			bool no_trig_replace = get_qr(qr_BLOCKS_DONT_LOCK_OTHER_LAYERS);
			bool trig_hole_same_only = get_qr(qr_BLOCKHOLE_SAME_ONLY);
			bool trig_is_layer = false;
			if(!fallclk && !drownclk)
			{
				end_rpos_handle.set_data(bcombo);
				end_rpos_handle.set_cset(oldcset);
				FFCore.clear_combo_script(end_rpos_handle);
				
				if((f1==mfBLOCKTRIGGER)||f2==mfBLOCKTRIGGER)
				{
					trigger = true;
				}
				else if(!trig_hole_same_only)
				{
					for(auto lyr = 0; lyr <= maxLayer; ++lyr)
					{
						if(lyr==blockLayer) continue;

						mapscr* m0 = get_scr_for_world_xy_layer(x, y, lyr);
						if(m0->sflag[end_pos] == mfBLOCKTRIGGER
							|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
						{
							trigger = true;
							trig_is_layer = true;
							if(!no_trig_replace)
							{
								mapscr* m2 = m0;
								m2->data[end_pos] = m2->undercombo;
								m2->cset[end_pos] = m2->undercset;
								m2->sflag[end_pos] = 0;
							}
						}
					}
				}
				if(trigger)
				{
					if(!(no_trig_replace && trig_is_layer))
						m->sflag[end_pos]=mfPUSHED;
				}
			}
			
			if((f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE)
			{
				m->data[end_pos]+=1;
				bhole=true;
			}
			else if(!trig_hole_same_only)
			{
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					if(lyr==blockLayer) continue;
					mapscr* m0 = get_scr_for_world_xy_layer(x, y, lyr);
					if((m0->sflag[end_pos]==mfBLOCKHOLE)
						|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
					{
						mapscr* m2 = m0;
						m->data[end_pos] = m->undercombo;
						m->cset[end_pos] = m->undercset;
						m2->data[end_pos] = bcombo+1;
						m2->cset[end_pos] = oldcset;
						m2->sflag[end_pos] = mfNONE;
						bhole=true;
						break;
					}
				}
			}
			if(bhole)
			{
				m->sflag[end_pos]=mfNONE;
				if(fallclk||drownclk)
				{
					fallclk = 0;
					drownclk = 0;
					return false;
				}
			}
			if(fallclk||drownclk) return false;
			
			if(block_cmb.attribytes[2])
				sfx(block_cmb.attribytes[2],pan(x));
			bool didtrigger = trigger;
			if(didtrigger)
			{
				for_every_rpos([&](const rpos_handle_t& rpos_handle) {
					if (!didtrigger)
						return;
					if (rpos_handle.layer > maxLayer)
						return;
					if ((!trig_hole_same_only || rpos_handle.layer == blockLayer) && rpos_handle.rpos == end_rpos_handle.rpos)
						return;

					if (rpos_handle.sflag() == mfBLOCKTRIGGER || rpos_handle.cflag() == mfBLOCKTRIGGER)
					{
						bool found = false;
						if (no_trig_replace)
							for (auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								mapscr* tmp2 = get_scr_for_world_xy_layer(x, y, lyr2);
								if (is_push(tmp2, rpos_handle.pos))
								{
									found = true;
									break;
								}
							}
						if (!found)
						{
							didtrigger=false;
						}
					}
				});
			}
			
			if(get_qr(qr_BROKEN_PUSHBLOCK_FLAG_CLONING) && !(trigger && !(no_trig_replace && trig_is_layer)) && !bhole)
				m->sflag[end_pos]=oldflag;
			
			//triggers a secret
			if(didtrigger)
			{
				if(no_trig_replace)
				{
					//Lock in place all blocks on triggers,
					// and replace triggers with undercombo.
					// 'no_trig_replace' delays this to now, instead of
					// happening as each combo is placed.
					for_every_rpos([&](const rpos_handle_t& rpos_handle) {
						if (rpos_handle.layer > maxLayer)
							return;

						if (rpos_handle.sflag() == mfBLOCKTRIGGER || rpos_handle.cflag() == mfBLOCKTRIGGER)
						{
							for (auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								if (lyr2 == rpos_handle.layer) continue;

								mapscr* lyr2_scr = get_scr_for_rpos_layer(rpos_handle.rpos, lyr2);
								if (is_push(lyr2_scr, rpos_handle.pos))
								{
									lyr2_scr->sflag[rpos_handle.pos] = mfPUSHED;
								}
							}

							rpos_handle.set_data(rpos_handle.scr->undercombo);
							rpos_handle.set_cset(rpos_handle.scr->undercset);
							rpos_handle.set_sflag(0);
						}
					});
				}
				
				if (reveal_hidden_stairs(end_rpos_handle.scr, end_rpos_handle.screen, true))
				{
					sfx(m0->secretsfx);
				}
				else
				{
					trigger_secrets_for_screen(TriggerSource::Unspecified, end_rpos_handle.base_scr(), true);
					
					if((combobuf[bcombo].type == cPUSH_WAIT) ||
							(combobuf[bcombo].type == cPUSH_HW) ||
							(combobuf[bcombo].type == cPUSH_HW2) || didtrigger)
					{
						sfx(m0->secretsfx);
					}
				}
				
				if (m0->flags&fSHUTTERS && isdungeon(m0->screen))
				{
					get_screen_state(m0->screen).open_doors = 8;
				}
				
				if(canPermSecret(cur_dmap, end_rpos_handle.screen))
				{
					if(get_qr(qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
						(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
							|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
					{
						if(!(m0->flags5&fTEMPSECRETS))
							setmapflag(end_rpos_handle.scr, mSECRET);
					}
				}
			}
			
			if(m->data[end_pos] == bcombo)
			{
				cpos_get(end_rpos_handle).updateInfo(blockinfo);
			}
		}
		else
		{
			if(grav_falling)
			{
				x = endx;
				y = endy;
			}
			
			int32_t f1 = end_rpos_handle.sflag();
			int32_t f2 = end_rpos_handle.cflag();
			trigger = false; bhole = (f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE;
			
			auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
			bool no_trig_replace = get_qr(qr_BLOCKS_DONT_LOCK_OTHER_LAYERS);
			bool trig_hole_same_only = get_qr(qr_BLOCKHOLE_SAME_ONLY);
			bool trig_is_layer = false;
			if(!fallclk && !drownclk || (bhole && !get_qr(qr_BROKEN_BLOCKHOLE_PITFALLS)))
			{
				end_rpos_handle.set_data(bcombo);
				end_rpos_handle.set_cset(oldcset);
				FFCore.clear_combo_script(end_rpos_handle);
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

						mapscr* scr = get_scr_for_world_xy_layer(x, y, lyr);
						if (scr->sflag[end_pos] == mfBLOCKTRIGGER
							|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
						{
							trigger = true;
							trig_is_layer = true;
							if(!no_trig_replace)
							{
								scr->data[end_pos] = scr->undercombo;
								scr->cset[end_pos] = scr->undercset;
								scr->sflag[end_pos] = 0;
							}
						}
					}
				}
				if(trigger)
				{
					if(!(no_trig_replace && trig_is_layer))
						end_rpos_handle.set_sflag(mfPUSHED);
				}
			}
			
			if(bhole)
			{
				end_rpos_handle.increment_data();
			}
			else if(!trig_hole_same_only)
			{
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					if(lyr==blockLayer) continue;

					mapscr* scr = get_scr_for_world_xy_layer(x, y, lyr);
					if ((scr->sflag[end_pos]==mfBLOCKHOLE)
						|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
					{
						mapscr* m2 = scr;
						m->data[end_pos] = m->undercombo;
						m->cset[end_pos] = m->undercset;
						m2->data[end_pos] = bcombo+1;
						m2->cset[end_pos] = oldcset;
						m2->sflag[end_pos] = mfNONE;
						bhole=true;
						break;
					}
				}
			}
			if(bhole)
			{
				m->sflag[end_pos]=mfNONE;
				if(fallclk||drownclk)
				{
					fallclk = 0;
					drownclk = 0;
					return false;
				}
			}
			else if(!fallclk&&!drownclk)
			{
				f2 = end_rpos_handle.cflag();
				
				if(!(force_many || (f2==mfPUSHUDINS && dir<=down) ||
						(f2==mfPUSHLRINS && dir>=left) ||
						(f2==mfPUSHUINS && dir==up) ||
						(f2==mfPUSHDINS && dir==down) ||
						(f2==mfPUSHLINS && dir==left) ||
						(f2==mfPUSHRINS && dir==right) ||
						(f2==mfPUSH4INS)))
				{
					m->sflag[end_pos]=mfPUSHED;
				}
			}
			if(fallclk||drownclk) return false;
			
			bool didtrigger = trigger;
			if(didtrigger)
			{
				for_every_rpos([&](const rpos_handle_t& rpos_handle) {
					if (!didtrigger)
						return;

					if((!trig_hole_same_only || rpos_handle.layer == blockLayer) && rpos_handle.rpos == end_rpos_handle.rpos)
						return;

					if (rpos_handle.sflag() == mfBLOCKTRIGGER || rpos_handle.cflag() == mfBLOCKTRIGGER)
					{
						bool found = false;
						if (no_trig_replace)
							for (auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								mapscr* lyr2_scr = get_scr_for_rpos_layer(rpos_handle.rpos, lyr2);
								if (is_push(lyr2_scr, rpos_handle.pos))
								{
									found = true;
									break;
								}
							}

						if (!found)
						{
							didtrigger = false;
							return;
						}
					}
				});
			}
			
			if(oldflag>=mfPUSHUDINS && (oldflag <= mfPUSHRINS || get_qr(qr_BROKEN_PUSHBLOCK_FLAG_CLONING))
				&& !(trigger && !(no_trig_replace && trig_is_layer))
				&& !bhole)
			{
				m->sflag[end_pos]=oldflag;
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
					for_every_rpos([&](const rpos_handle_t& rpos_handle) {
						if (rpos_handle.layer > maxLayer)
							return;

						if (rpos_handle.sflag() == mfBLOCKTRIGGER || rpos_handle.cflag() == mfBLOCKTRIGGER)
						{
							for (auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
							{
								if (lyr2 == rpos_handle.layer) continue;

								mapscr* lyr2_scr = get_scr_for_rpos_layer(rpos_handle.rpos, lyr2);
								if (is_push(lyr2_scr, rpos_handle.pos))
								{
									lyr2_scr->sflag[rpos_handle.pos] = mfPUSHED;
								}
							}

							rpos_handle.set_data(rpos_handle.scr->undercombo);
							rpos_handle.set_cset(rpos_handle.scr->undercset);
							rpos_handle.set_sflag(0);
						}
					});
				}
				
				if (reveal_hidden_stairs(m0, end_rpos_handle.screen, true))
				{
					sfx(m0->secretsfx);
				}
				else
				{
					trigger_secrets_for_screen(TriggerSource::Unspecified, end_rpos_handle.base_scr(), true);
					
					if((combobuf[bcombo].type == cPUSH_WAIT) ||
							(combobuf[bcombo].type == cPUSH_HW) ||
							(combobuf[bcombo].type == cPUSH_HW2) || didtrigger)
					{
						sfx(m0->secretsfx);
					}
				}
				
				if (m0->flags&fSHUTTERS && isdungeon(m0->screen))
				{
					get_screen_state(m0->screen).open_doors = 8;
				}
				
				if(canPermSecret(cur_dmap, end_rpos_handle.screen))
				{
					if(get_qr(qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
						(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
							|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
					{
						if(!(m0->flags5&fTEMPSECRETS))
							setmapflag(end_rpos_handle.scr, mSECRET);
					}
				}
			}
		}
		trig_each_combo_trigger(end_rpos_handle, [](combo_trigger const& trig){
			return trig.trigger_flags.get(TRIGFLAG_PUSHEDTRIG);
		});
		clear();
	}
	return false;
}
