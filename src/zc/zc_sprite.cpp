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
	if(!get_qr(qr_NEW_DARKROOM)) return;
	if(!(tmpscr->flags & fDARK)) return;
	handle_lighting(x.getInt()+(hit_width/2), y.getInt()+(hit_height/2),glowShape,glowRad,dir);
}

bool is_conveyor(int32_t type)
{
	auto& cc = combo_class_buf[type];
	return cc.conveyor_x_speed || cc.conveyor_y_speed;
}

int32_t get_conveyor(int32_t x, int32_t y)
{
	x = vbound(x, 0, (16*16)-1);
	y = vbound(y, 0, (11*16)-1);
	int maxlayer = get_qr(qr_CONVEYORS_ALL_LAYERS) ? 6 : (get_qr(qr_CONVEYORS_L1_L2) ? 2 : 0);
	int pos = COMBOPOS(x,y);
	int found_layer = -1;
	int cmbid = -1;
	int ffcid = MAPFFCOMBO(x,y);
	if(maxlayer == 6 && ffcid && is_conveyor(combobuf[ffcid].type))
	{
		cmbid = ffcid;
		found_layer = 999;
	}
	else for(int q = maxlayer; q >= 0; --q)
	{
		mapscr* m = FFCore.tempScreens[q];
		int cid = m->data[pos];
		if(is_conveyor(combobuf[cid].type)
			&& _effectflag_layer(x,y,1,m,true))
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
			if(!tmpscr2[i].valid) continue;
			
			auto tcid = MAPCOMBO2(i,x,y);
			if(combobuf[tcid].type == cBRIDGE)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS))
				{
					if (!_walkflag_layer(x,y,1,&(tmpscr2[i]))) return -1;
				}
				else
				{
					if (_effectflag_layer(x,y,1,&(tmpscr2[i]), true)) return -1;
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
    if(((z==0&&fakez==0) || (tmpscr->flags2&fAIRCOMBOS)))
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
	trigger=false;
	x=bx;
	y=by;
	dir=d2;
	xofs = 0;
	yofs = playing_field_offset;
	switch(dir)
	{
		case up:
			endx = x;
			endy = y-16;
			break;
		case down:
			endx = x;
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
	oldflag=f;
	size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
	mapscr *m = FFCore.tempScreens[blockLayer];
	word *di = &(m->data[combopos]);
	byte *ci = &(m->cset[combopos]);
	bcombo =  m->data[combopos];
	oldcset = m->cset[combopos];
	cs     = (isdungeon() && !get_qr(qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
	tile = combobuf[bcombo].tile;
	flip = combobuf[bcombo].flip;
	//   cs = ((*di)&0x700)>>8;
	*di = m->undercombo;
	*ci = m->undercset;
	FFCore.clear_combo_script(blockLayer, combopos);
	putcombo(scrollbuf,x,y,*di,*ci);
	clk=32;
	if(!get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(true);
	solid_update(false);
}
void movingblock::push_new(zfix bx,zfix by,int d2,int f,zfix spd)
{
	new_block = true;
	step = spd;
	trigger=false;
	x=bx;
	y=by;
	dir=d2;
	xofs = 0;
	yofs = playing_field_offset;
	switch(dir)
	{
		case up:
			endx = x;
			endy = y-16;
			break;
		case down:
			endx = x;
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
	oldflag=f;
	size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
	mapscr *m = FFCore.tempScreens[blockLayer];
	word *di = &(m->data[combopos]);
	byte *ci = &(m->cset[combopos]);
	bcombo =  m->data[combopos];
	oldcset = m->cset[combopos];
	cs     = (isdungeon() && !get_qr(qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
	tile = combobuf[bcombo].tile;
	flip = combobuf[bcombo].flip;
	//   cs = ((*di)&0x700)>>8;
	*di = m->undercombo;
	*ci = m->undercset;
	FFCore.clear_combo_script(blockLayer, combopos);
	putcombo(scrollbuf,x,y,*di,*ci);
	clk=32;
	if(!get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(true);
	solid_update(false);
}

bool movingblock::check_hole() const
{
	mapscr* m = FFCore.tempScreens[blockLayer];
	size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
	if((m->sflag[combopos]==mfBLOCKHOLE)||MAPCOMBOFLAG2(blockLayer-1,x,y)==mfBLOCKHOLE)
		return true;
	else if(!get_qr(qr_BLOCKHOLE_SAME_ONLY))
	{
		auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
		for(auto lyr = 0; lyr <= maxLayer; ++lyr)
		{
			if(lyr==blockLayer) continue;
			if((FFCore.tempScreens[lyr]->sflag[combopos]==mfBLOCKHOLE)
				|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
				return true;
		}
	}
	return false;
}

bool movingblock::check_trig() const
{
	mapscr* m = FFCore.tempScreens[blockLayer];
	size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
	if(fallclk || drownclk)
		return false;
	if((m->sflag[combopos]==mfBLOCKTRIGGER)||MAPCOMBOFLAG2(blockLayer-1,x,y)==mfBLOCKTRIGGER)
		return true;
	else if(!get_qr(qr_BLOCKHOLE_SAME_ONLY))
	{
		auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
		for(auto lyr = 0; lyr <= maxLayer; ++lyr)
		{
			if(lyr==blockLayer) continue;
			if(FFCore.tempScreens[lyr]->sflag[combopos] == mfBLOCKTRIGGER
				|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
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
	mapscr* m = FFCore.tempScreens[blockLayer];
	if(get_qr(qr_MOVINGBLOCK_FAKE_SOLID))
		setSolid(false);
	else setSolid(clk > 0 && !(fallclk || drownclk));
	if(fallclk)
	{
		if(fallclk == PITFALL_FALL_FRAMES)
			sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		clk = 0;
		solid_update(false);
		if(!--fallclk)
			clear();
		return false;
	}
	if(drownclk)
	{
		//if(drownclk == WATER_DROWN_FRAMES)
		//sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
		//!TODO: Drown SFX
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
	newcombo const& block_cmb = combobuf[bcombo];
	
	//Move
	move(step);
	zfix ox = x, oy = y; //grab the x/y after moving, before any snapping
	
	//Check if the block has reached the next grid-alignment
	if(new_block)
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
	
	//Check if the block is falling into a pitfall (if aligned)
	if(done)
	{
		if((fallCombo = getpitfall(x+8,y+8)))
		{
			fallclk = PITFALL_FALL_FRAMES;
		}
		/*
		//!TODO: Moving Block Drowning
		if(drownCombo = iswaterex(MAPCOMBO(x+8,y+8), currmap, currscr, -1, x+8,y+8, false, false, true))
		{
			drownclk = WATER_DROWN_FRAMES;
		}
		*/
	}
	
	//Check for icy blocks/floors that might continue the slide
	if(done && !no_icy && !fallclk && !drownclk)
	{
		if(new_block)
		{
			if(((block_cmb.usrflags&cflag7) || //icy blocks keep sliding?
				(!(block_cmb.usrflags&cflag10) && get_icy(endx+8,endy+8,0))))
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
				if(new_endx < 0 || new_endx > 240 || new_endy < 0 || new_endy > 168)
					canslide = false;
				else if(check_hole()) //Falls into block holes on the way
					canslide = false;
				else
				{
					bool solid = false;
					int iflag = 0, pflag = 0;
					switch(dir)
					{
						case up:
							solid = _walkflag(endx,endy-8,2);
							pflag = MAPFLAG2(blockLayer-1,endx,endy-8);
							iflag = MAPCOMBOFLAG2(blockLayer-1,endx,endy-8);
							break;
						case down:
							solid = _walkflag(endx,endy+24,2);
							pflag = MAPFLAG2(blockLayer-1,endx,endy+24);
							iflag = MAPCOMBOFLAG2(blockLayer-1,endx,endy+24);
							break;
						case left:
							solid = _walkflag(endx-16,endy+8,2);
							pflag = MAPFLAG2(blockLayer-1,endx-16,endy+8);
							iflag = MAPCOMBOFLAG2(blockLayer-1,endx-16,endy+8);
							break;
						case right:
							solid = _walkflag(endx+16,endy+8,2);
							pflag = MAPFLAG2(blockLayer-1,endx+16,endy+8);
							iflag = MAPCOMBOFLAG2(blockLayer-1,endx+16,endy+8);
							break;
					}
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
				}
			}
		}
		else if(int c = get_icy(x+8,y+8,0))
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
			if(new_endx < 0 || new_endx > 240 || new_endy < 0 || new_endy > 168)
				canslide = false;
			else
			{
				bool solid = false;
				int iflag = 0, pflag = 0;
				switch(dir)
				{
					case up:
						solid = _walkflag(endx,endy-8,2);
						pflag = MAPFLAG2(blockLayer-1,endx,endy-8);
						iflag = MAPCOMBOFLAG2(blockLayer-1,endx,endy-8);
						break;
					case down:
						solid = _walkflag(endx,endy+24,2);
						pflag = MAPFLAG2(blockLayer-1,endx,endy+24);
						iflag = MAPCOMBOFLAG2(blockLayer-1,endx,endy+24);
						break;
					case left:
						solid = _walkflag(endx-16,endy+8,2);
						pflag = MAPFLAG2(blockLayer-1,endx-16,endy+8);
						iflag = MAPCOMBOFLAG2(blockLayer-1,endx-16,endy+8);
						break;
					case right:
						solid = _walkflag(endx+16,endy+8,2);
						pflag = MAPFLAG2(blockLayer-1,endx+16,endy+8);
						iflag = MAPCOMBOFLAG2(blockLayer-1,endx+16,endy+8);
						break;
				}
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
		size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
		if(new_block)
		{
			clk = 0;
			x = endx;
			y = endy;
			trigger = false; bhole = false;
			
			int f1 = m->sflag[combopos];
			int f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
			auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
			bool no_trig_replace = get_qr(qr_BLOCKS_DONT_LOCK_OTHER_LAYERS);
			bool trig_hole_same_only = get_qr(qr_BLOCKHOLE_SAME_ONLY);
			bool trig_is_layer = false;
			if(!fallclk && !drownclk)
			{
				m->data[combopos]=bcombo;
				m->cset[combopos]=oldcset;
				FFCore.clear_combo_script(blockLayer, combopos);
				
				if((f1==mfBLOCKTRIGGER)||f2==mfBLOCKTRIGGER)
				{
					trigger = true;
				}
				else if(!trig_hole_same_only)
				{
					for(auto lyr = 0; lyr <= maxLayer; ++lyr)
					{
						if(lyr==blockLayer) continue;
						if(FFCore.tempScreens[lyr]->sflag[combopos] == mfBLOCKTRIGGER
							|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
						{
							trigger = true;
							trig_is_layer = true;
							if(!no_trig_replace)
							{
								mapscr* m2 = FFCore.tempScreens[lyr];
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
					if((FFCore.tempScreens[lyr]->sflag[combopos]==mfBLOCKHOLE)
						|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
					{
						mapscr* m2 = FFCore.tempScreens[lyr];
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
			if(fallclk||drownclk) return false;
			
			if(block_cmb.attribytes[2])
				sfx(block_cmb.attribytes[2],(int32_t)x);
			bool didtrigger = trigger;
			if(didtrigger)
			{
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					mapscr* tmp = FFCore.tempScreens[lyr];
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
									if(is_push(FFCore.tempScreens[lyr2], pos))
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
			
			if(!(trigger && !(no_trig_replace && trig_is_layer)) && !bhole)
			{
				m->sflag[combopos]=oldflag;
			}
			
			//triggers a secret
			if(didtrigger)
			{
				if(no_trig_replace)
				{
					//Lock in place all blocks on triggers,
					// and replace triggers with undercombo.
					// 'no_trig_replace' delays this to now, instead of
					// happening as each combo is placed.
					for(auto lyr = 0; lyr <= maxLayer; ++lyr)
					{
						mapscr* tmp = FFCore.tempScreens[lyr];
						for(int32_t pos=0; pos<176; pos++)
						{
							if(tmp->sflag[pos]==mfBLOCKTRIGGER
								|| combobuf[tmp->data[pos]].flag==mfBLOCKTRIGGER)
							{
								for(auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
								{
									if(lyr2 == lyr) continue;
									if(is_push(FFCore.tempScreens[lyr2], pos))
									{
										FFCore.tempScreens[lyr2]->sflag[pos] = mfPUSHED;
									}
								}
								tmp->data[pos] = tmp->undercombo;
								tmp->cset[pos] = tmp->undercset;
								tmp->sflag[pos] = 0;
							}
						}
					}
				}
				
				if(hiddenstair(0,true))
				{
					sfx(tmpscr->secretsfx);
				}
				else
				{
					hidden_entrance(0,true,true);
					
					if((combobuf[bcombo].type == cPUSH_WAIT) ||
							(combobuf[bcombo].type == cPUSH_HW) ||
							(combobuf[bcombo].type == cPUSH_HW2) || didtrigger)
					{
						sfx(tmpscr->secretsfx);
					}
				}
				
				if(isdungeon() && tmpscr->flags&fSHUTTERS)
				{
					opendoors=8;
				}
				
				if(canPermSecret())
				{
					if(get_qr(qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
						(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
							|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
					{
						if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
					}
				}
			}
			
			putcombo(scrollbuf,x,y,bcombo,cs);
			
			if(m->data[combopos] == bcombo)
			{
				cpos_get(blockLayer, combopos).updateInfo(blockinfo);
			}
		}
		else
		{
			int32_t f1 = m->sflag[combopos];
			int32_t f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
			trigger = false; bhole = (f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE;
			
			auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
			bool no_trig_replace = get_qr(qr_BLOCKS_DONT_LOCK_OTHER_LAYERS);
			bool trig_hole_same_only = get_qr(qr_BLOCKHOLE_SAME_ONLY);
			bool trig_is_layer = false;
			if(!fallclk && !drownclk || (bhole && !get_qr(qr_BROKEN_BLOCKHOLE_PITFALLS)))
			{
				m->data[combopos]=bcombo;
				m->cset[combopos]=oldcset;
				FFCore.clear_combo_script(blockLayer, combopos);
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
						if(FFCore.tempScreens[lyr]->sflag[combopos] == mfBLOCKTRIGGER
							|| MAPCOMBOFLAG2(lyr-1,x,y) == mfBLOCKTRIGGER)
						{
							trigger = true;
							trig_is_layer = true;
							if(!no_trig_replace)
							{
								mapscr* m2 = FFCore.tempScreens[lyr];
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
			
			if(bhole)
			{
				m->data[combopos]+=1;
			}
			else if(!trig_hole_same_only)
			{
				for(auto lyr = 0; lyr <= maxLayer; ++lyr)
				{
					if(lyr==blockLayer) continue;
					if((FFCore.tempScreens[lyr]->sflag[combopos]==mfBLOCKHOLE)
						|| MAPCOMBOFLAG2(lyr-1,x,y)==mfBLOCKHOLE)
					{
						mapscr* m2 = FFCore.tempScreens[lyr];
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
					mapscr* tmp = FFCore.tempScreens[lyr];
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
									if(is_push(FFCore.tempScreens[lyr2], pos))
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
						mapscr* tmp = FFCore.tempScreens[lyr];
						for(int32_t pos=0; pos<176; pos++)
						{
							if(tmp->sflag[pos]==mfBLOCKTRIGGER
								|| combobuf[tmp->data[pos]].flag==mfBLOCKTRIGGER)
							{
								for(auto lyr2 = 0; lyr2 <= maxLayer; ++lyr2)
								{
									if(lyr2 == lyr) continue;
									if(is_push(FFCore.tempScreens[lyr2], pos))
									{
										FFCore.tempScreens[lyr2]->sflag[pos] = mfPUSHED;
									}
								}
								tmp->data[pos] = tmp->undercombo;
								tmp->cset[pos] = tmp->undercset;
								tmp->sflag[pos] = 0;
							}
						}
					}
				}
				
				if(hiddenstair(0,true))
				{
					sfx(tmpscr->secretsfx);
				}
				else
				{
					hidden_entrance(0,true,true);
					
					if((combobuf[bcombo].type == cPUSH_WAIT) ||
							(combobuf[bcombo].type == cPUSH_HW) ||
							(combobuf[bcombo].type == cPUSH_HW2) || didtrigger)
					{
						sfx(tmpscr->secretsfx);
					}
				}
				
				if(isdungeon() && tmpscr->flags&fSHUTTERS)
				{
					opendoors=8;
				}
				
				if(canPermSecret())
				{
					if(get_qr(qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
						(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
							|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
					{
						if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
					}
				}
			}
			
			putcombo(scrollbuf,x,y,bcombo,cs);
		}
		newcombo const& blockcmb = combobuf[bcombo];
		if(blockcmb.triggerflags[3] & combotriggerPUSHEDTRIG)
		{
			do_trigger_combo(blockLayer, combopos);
		}
		clear();
	}
	return false;
}
