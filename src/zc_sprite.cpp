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
	if(!(tmpscr->flags & fDARK)) return;
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

void sprite::check_conveyor()
{
    int32_t deltax=0;
    int32_t deltay=0;
    
    if(conveyclk<=0 && (z==0 || (tmpscr->flags2&fAIRCOMBOS)))
    {
        int32_t ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
        deltax=combo_class_buf[ctype].conveyor_x_speed;
        deltay=combo_class_buf[ctype].conveyor_y_speed;
        
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
	trigger = false;
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
	size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
	mapscr *m = FFCore.tempScreens[blockLayer];
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
	FFCore.reset_combo_script(blockLayer, combopos);
    putcombo(scrollbuf,x,y,*di,*ci);
    clk=32;
    blockmoving=true;
}

bool movingblock::animate(int32_t)
{
	mapscr* m = FFCore.tempScreens[blockLayer];
	if(fallclk)
	{
		if(fallclk == PITFALL_FALL_FRAMES)
			sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			blockmoving=false;
		}
		clk = 0;
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
		return false;
	}
	if(clk<=0)
		return false;
		
	move(step);
	
	if(--clk==0)
	{
		trigger = false; bhole = false;
		blockmoving=false;
		
		if(fallCombo = getpitfall(x+8,y+8))
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
		size_t combopos = size_t((int32_t(y)&0xF0)+(int32_t(x)>>4));
		int32_t f1 = m->sflag[combopos];
		int32_t f2 = MAPCOMBOFLAG2(blockLayer-1,x,y);
		auto maxLayer = get_bit(quest_rules, qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
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
			else if(!get_bit(quest_rules, qr_BLOCKHOLE_SAME_ONLY))
			{
				for(auto q = 0; q < maxLayer; ++q)
				{
					if(q==blockLayer) continue;
					if(FFCore.tempScreens[q]->sflag[combopos] == mfBLOCKTRIGGER
						|| MAPCOMBOFLAG2(q-1,x,y) == mfBLOCKTRIGGER)
					{
						trigger = true;
						mapscr* m2 = FFCore.tempScreens[q];
						m2->data[combopos] = m2->undercombo;
						m2->cset[combopos] = m2->undercset;
						m2->sflag[combopos] = 0;
					}
				}
			}
			if(trigger)
			{
				m->sflag[combopos]=mfPUSHED;
			}
		}
		
		if((f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE)
		{
			m->data[combopos]+=1;
			bhole=true;
		}
		else if(!get_bit(quest_rules, qr_BLOCKHOLE_SAME_ONLY))
		{
			for(auto q = 0; q < maxLayer; ++q)
			{
				if(q==blockLayer) continue;
				if((FFCore.tempScreens[q]->sflag[combopos]==mfBLOCKHOLE)
					|| MAPCOMBOFLAG2(q-1,x,y)==mfBLOCKHOLE)
				{
					m->data[combopos]+=1;
					if(q != blockLayer)
						FFCore.tempScreens[q]->data[combopos]+=1;
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
			
			if(!((f2==mfPUSHUDINS && dir<=down) ||
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
		
		if(oldflag>=mfPUSHUDINS&&oldflag&&!trigger&&!bhole)
		{
			m->sflag[combopos]=oldflag;
		}
		
		bool didtrigger = trigger;
		for(auto q = 0; q < maxLayer; ++q)
		{
			for(int32_t i=0; i<176; i++)
			{
				if(FFCore.tempScreens[q]->sflag[i]==mfBLOCKTRIGGER
					|| combobuf[FFCore.tempScreens[q]->data[i]].flag==mfBLOCKTRIGGER)
				{
					didtrigger=false;
				}
			}
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
		//if(oldflag<mfPUSHUDNS||didtrigger)
		{
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
				if(get_bit(quest_rules, qr_NONHEAVY_BLOCKTRIGGER_PERM) ||
					(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
						|| combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2))
				{
					if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
				}
			}
		}
		
		putcombo(scrollbuf,x,y,bcombo,cs);
	}
	
	return false;
}

/*** end of sprite.cc ***/

