#include "base/qrs.h"
#include "base/dmap.h"
#include "zc/zc_ffc.h"
#include "zc/zelda.h"
#include "sprite.h"
#include "zc/decorations.h"
#include "zc/combos.h"
#include "zc/maps.h"
#include "items.h"
#include "zc/guys.h"
#include "zc/ffscript.h"
#include "zc/hero.h"
#include "zc/title.h"
#include "base/mapscr.h"
#include "base/misctypes.h"

extern sprite_list items, decorations;
extern FFScript FFCore;
extern HeroClass Hero;

// CUTSCENE STUFF

void CutsceneState::clear()
{
	*this = CutsceneState();
}
bool CutsceneState::can_button(int q)
{
	if(!active) return true;
	return allowed_btns & (1<<q);
}
bool CutsceneState::can_f6()
{
	return !(active && nof6);
}
void CutsceneState::error()
{
	if(errsfx)
		sfx_no_repeat(errsfx);
}
CutsceneState active_cutscene;

// TRIGGERS STUFF

bool alwaysCTypeEffects(int32_t type)
{
	switch(type)
	{
		case cCUSTOMBLOCK:
			return true;
	}
	return false;
}

void do_generic_combo2(int32_t bx, int32_t by, int32_t cid, int32_t flag, int32_t flag2, int32_t ft, int32_t scombo, bool single16, int32_t layer)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	} 
	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	if (true) // Probably needs a way to only be triggered once...
	{
		if ((combobuf[cid].usrflags&cflag1)) 
		{
			//zprint("Adding decoration, sprite: %d\n", combobuf[cid].attributes[0] / 10000L);
			if (combobuf[cid].usrflags & cflag10)
			{
				switch (combobuf[cid].attribytes[0])
				{
					case 0:
					case 1:
					default:
						decorations.add(new dBushLeaves((zfix)COMBOX(scombo), (zfix)COMBOY(scombo), dBUSHLEAVES, 0, 0));
						break;
					case 2:
						decorations.add(new dFlowerClippings((zfix)COMBOX(scombo), (zfix)COMBOY(scombo), dFLOWERCLIPPINGS, 0, 0));
						break;
					case 3:
						decorations.add(new dGrassClippings((zfix)COMBOX(scombo), (zfix)COMBOY(scombo), dGRASSCLIPPINGS, 0, 0));
						break;
				}
			}
			else decorations.add(new comboSprite((zfix)COMBOX(scombo), (zfix)COMBOY(scombo), dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
		}
		
		int32_t it = -1; 
		int32_t thedropset = -1;
		if ( (combobuf[cid].usrflags&cflag2) )
		{
			if ( combobuf[cid].usrflags&cflag11 ) //specific item
			{
				it = combobuf[cid].attribytes[1];
			}
			else
			{
				it = select_dropitem(combobuf[cid].attribytes[1]);
				thedropset = combobuf[cid].attribytes[1];
			}
		}
		if( it != -1 )
		{
			item* itm = (new item((zfix)COMBOX(scombo), (zfix)COMBOY(scombo),(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(mSPECIALITEM))
		{
			items.add(new item((zfix)COMBOX(scombo),
				(zfix)COMBOY(scombo),
				(zfix)0,
				tmpscr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[tmpscr->catchall].family==itype_triforcepiece ||
				(tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_combo_modify_preroutine(tmpscr,scombo);
			tmpscr->data[scombo] = tmpscr->secretcombo[ft];
			tmpscr->cset[scombo] = tmpscr->secretcset[ft];
			tmpscr->sflag[scombo] = tmpscr->secretflag[ft];
			// newflag = s->secretflag[ft];
			screen_combo_modify_postroutine(tmpscr,scombo);
			if ( combobuf[cid].attribytes[2] > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
		}
		
		//loop next combo
		if((combobuf[cid].usrflags&cflag4))
		{
			do
			{
				
				
				if (layer) 
				{
					
					//screen_combo_modify_preroutine(tmpscr,scombo);
					screen_combo_modify_preroutine(FFCore.tempScreens[layer],scombo);
					
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						FFCore.tempScreens[layer]->data[scombo] = tmpscr->undercombo;
						FFCore.tempScreens[layer]->cset[scombo] = tmpscr->undercset;
						FFCore.tempScreens[layer]->sflag[scombo] = 0;	
					}
					else
						++FFCore.tempScreens[layer]->data[scombo];
					
					screen_combo_modify_postroutine(FFCore.tempScreens[layer],scombo);
					//screen_combo_modify_postroutine(FFCore.tempScreens[layer],cid);
					//screen_combo_modify_postroutine(tmpscr,scombo);
				}
				else
				{
					screen_combo_modify_preroutine(tmpscr,scombo);
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						tmpscr->data[scombo] = tmpscr->undercombo;
						tmpscr->cset[scombo] = tmpscr->undercset;
						tmpscr->sflag[scombo] = 0;	
					}
					else
					{
						tmpscr->data[scombo]=vbound(tmpscr->data[scombo]+1,0,MAXCOMBOS);
						//++tmpscr->data[scombo];
					}
					screen_combo_modify_postroutine(tmpscr,scombo);
				}
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ( layer ) ? MAPCOMBO2(layer,bx,by) : MAPCOMBO(bx,by);
				//if ( combobuf[cid].usrflags&cflag8 ) w->dead = 1;
				//tmpscr->sflag[scombo] = combobuf[cid].sflag;
				//combobuf[tmpscr->data[cid]].cset;
				//combobuf[tmpscr->data[cid]].cset;
				
				//tmpscr->cset[scombo] = combobuf[cid].cset;
				//tmpscr->sflag[scombo] = combobuf[cid].sflag;
				//zprint("++comboD\n");
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
			
			
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(COMBOX(scombo),COMBOY(scombo),(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
		}
		//zprint("continuous\n");
		
	}
	//set_bit(grid,(((bx>>4) + by)),1);
	
	//if ( c[cid].usrflags&cflag8 ) killgenwpn(w);
}

void do_generic_combo_ffc2(int32_t pos, int32_t cid, int32_t ft)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	} 
	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	ffcdata& ffc = tmpscr->ffcs[pos];
	if (true) // Probably needs a way to only be triggered once...
	{
		if (combobuf[cid].usrflags&cflag1) 
		{
			if (combobuf[cid].usrflags & cflag10)
			{
				switch (combobuf[cid].attribytes[0])
				{
					case 0:
					case 1:
					default:
						decorations.add(new dBushLeaves(ffc.x, ffc.y, dBUSHLEAVES, 0, 0));
						break;
					case 2:
						decorations.add(new dFlowerClippings(ffc.x, ffc.y, dFLOWERCLIPPINGS, 0, 0));
						break;
					case 3:
						decorations.add(new dGrassClippings(ffc.x, ffc.y, dGRASSCLIPPINGS, 0, 0));
						break;
				}
			}
			else decorations.add(new comboSprite(ffc.x, ffc.y, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
		}
		
		int32_t it = -1; 
		int32_t thedropset = -1;
		if (combobuf[cid].usrflags&cflag2)
		{
			if ( combobuf[cid].usrflags&cflag11 ) //specific item
			{
				it = combobuf[cid].attribytes[1];
			}
			else
			{
				it = select_dropitem(combobuf[cid].attribytes[1]);
				thedropset = combobuf[cid].attribytes[1];
			}
		}
		if( it != -1 )
		{
			item* itm = (new item(ffc.x, ffc.y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(mSPECIALITEM))
		{
			items.add(new item(ffc.x, ffc.y,(zfix)0,
				tmpscr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[tmpscr->catchall].family==itype_triforcepiece ||
				(tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_ffc_modify_preroutine(pos);
			zc_ffc_set(ffc, tmpscr->secretcombo[ft]);
			ffc.cset = tmpscr->secretcset[ft];
			// newflag = s->secretflag[ft];
			screen_ffc_modify_postroutine(pos);
			if ( combobuf[cid].attribytes[2] > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc.x));
		}
		
		//loop next combo
		if((combobuf[cid].usrflags&cflag4))
		{
			do
			{
				screen_ffc_modify_preroutine(pos);
				//undercombo or next?
				if((combobuf[cid].usrflags&cflag12))
				{
					zc_ffc_set(ffc, tmpscr->undercombo);
					ffc.cset = tmpscr->undercset;
				}
				else
				{
					zc_ffc_set(ffc, vbound(ffc.data+1,0,MAXCOMBOS));
				}
				screen_ffc_modify_postroutine(pos);
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ffc.data;
				
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc.x));
			
			
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(ffc.x,ffc.y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
		}
		
	}
}

void do_cutscene_flags(newcombo const& cmb)
{
	if(cmb.type != cCUTSCENETRIG) return;
	if(cmb.usrflags & cflag1) //Disable cutscene
	{
		active_cutscene.clear();
		return;
	}
	active_cutscene.active = true;
	active_cutscene.allowed_btns = cmb.attributes[0];
	active_cutscene.nof6 = (cmb.usrflags & cflag2)!=0;
	active_cutscene.errsfx = cmb.attribytes[0];
}

bool do_cswitch_combo(newcombo const& cmb, weapon* w)
{
	byte pair = cmb.attribytes[0];
	if(cmb.usrflags & cflag11) //global state
	{
		int32_t tmr = cmb.attributes[2] / 10000;
		bool oldstate = game->gswitch_timers[pair];
		if(tmr > 0)
		{
			game->gswitch_timers[pair] = tmr;
		}
		else
		{
			if(game->gswitch_timers[pair])
				game->gswitch_timers[pair] = 0;
			else game->gswitch_timers[pair] = -1;
		}
		if(oldstate != (game->gswitch_timers[pair] != 0))
		{
			toggle_gswitches(pair, false);
		}
	}
	else
	{
		if(pair > 31) return false;
		game->lvlswitches[dlevel] ^= (1 << pair);
		toggle_switches(1<<pair, false);
	}
	if(w && (cmb.usrflags&cflag1))
		killgenwpn(w); //Kill weapon
	if(cmb.attribytes[1]) sfx(cmb.attribytes[1]);
	return true;
}

void trigger_cswitch_block(int32_t layer, int32_t pos)
{
	if(unsigned(layer) > 6 || unsigned(pos) > 175) return;
	mapscr* scr = FFCore.tempScreens[layer];
	auto cid = scr->data[pos];
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	scr->data[pos] = BOUND_COMBO(cid + cmbofs);
	scr->cset[pos] = (scr->cset[pos] + csofs) & 15;
	auto newcid = scr->data[pos];
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
	}
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		if(lyr == layer) continue;
		if(!(cmb.usrflags&(1<<lyr))) continue;
		mapscr* scr_2 = FFCore.tempScreens[lyr];
		if(!scr_2->data[pos]) //Don't increment empty space
			continue;
		newcombo const& cmb_2 = combobuf[scr_2->data[pos]];
		scr_2->data[pos] = BOUND_COMBO(scr_2->data[pos] + cmbofs);
		scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
		int32_t newcid2 = scr_2->data[pos];
		if(combobuf[newcid2].animflags & AF_CYCLE)
		{
			combobuf[newcid2].tile = combobuf[newcid2].o_tile;
			combobuf[newcid2].cur_frame=0;
			combobuf[newcid2].aclk = 0;
		}
	}
	if (cmb.usrflags&cflag11)
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			if(ffcIsAt(i, COMBOX(pos)+8, COMBOY(pos)+8))
			{
				ffcdata& ffc2 = tmpscr->ffcs[i];
				newcombo const& cmb_2 = combobuf[ffc2.data];
				zc_ffc_set(ffc2, BOUND_COMBO(ffc2.data + cmbofs));
				ffc2.cset = (ffc2.cset + csofs) & 15;
				int32_t newcid2 = ffc2.data;
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
				}
			}
		}
	}
}

void trigger_cswitch_block_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return;
	ffcdata& ffc = tmpscr->ffcs[pos];
	auto cid = ffc.data;
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	zc_ffc_set(ffc, BOUND_COMBO(cid + cmbofs));
	ffc.cset = (ffc.cset + csofs) & 15;
	auto newcid = ffc.data;
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
	}
	int32_t pos2 = COMBOPOS(ffc.x+8, ffc.y+8);
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		if(!(cmb.usrflags&(1<<lyr))) continue;
		mapscr* scr_2 = FFCore.tempScreens[lyr];
		if(!scr_2->data[pos2]) //Don't increment empty space
			continue;
		newcombo const& cmb_2 = combobuf[scr_2->data[pos2]];
		scr_2->data[pos2] = BOUND_COMBO(scr_2->data[pos2] + cmbofs);
		scr_2->cset[pos2] = (scr_2->cset[pos2] + csofs) & 15;
		int32_t newcid2 = scr_2->data[pos2];
		if(combobuf[newcid2].animflags & AF_CYCLE)
		{
			combobuf[newcid2].tile = combobuf[newcid2].o_tile;
			combobuf[newcid2].cur_frame=0;
			combobuf[newcid2].aclk = 0;
		}
	}
	if (cmb.usrflags&cflag11)
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			if (i == pos) continue;
			if(ffcIsAt(i, ffc.x+8, ffc.y+8))
			{
				ffcdata& ffc2 = tmpscr->ffcs[i];
				newcombo const& cmb_2 = combobuf[ffc2.data];
				zc_ffc_set(ffc2, BOUND_COMBO(ffc2.data + cmbofs));
				ffc2.cset = (ffc2.cset + csofs) & 15;
				int32_t newcid2 = ffc2.data;
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
				}
			}
		}
	}
}



void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y)
{
	int16_t decotype = (cmb.usrflags & cflag1) ? ((cmb.usrflags & cflag10) ? (cmb.attribytes[0]) : (-1)) : (0);
	if(decotype > 3) decotype = 0;
	if(!decotype) decotype = (isBushType(cmb.type) ? 1 :
		(isFlowersType(cmb.type) ? 2 :
		(isGrassType(cmb.type) ? 3 :
		((cmb.usrflags & cflag1) ? -1 : -2))));
	switch(decotype)
	{
		case -2: break; //nothing
		case -1:
			decorations.add(new comboSprite(x, y, dCOMBOSPRITE, 0, cmb.attribytes[0]));
			break;
		case 1: decorations.add(new dBushLeaves(x, y, dBUSHLEAVES, 0, 0)); break;
		case 2: decorations.add(new dFlowerClippings(x, y, dFLOWERCLIPPINGS, 0, 0)); break;
		case 3: decorations.add(new dGrassClippings(x, y, dGRASSCLIPPINGS, 0, 0)); break;
	}
}
void spawn_decoration(newcombo const& cmb, int32_t pos)
{
	if(unsigned(pos) > 175) return;
	spawn_decoration_xy(cmb, COMBOX(pos), COMBOY(pos));
}

void trigger_cuttable(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	auto type = cmb.type;
	if(!isCuttableType(type)) return;
	auto flag = tmp->sflag[pos];
	auto flag2 = cmb.flag;
	auto x = COMBOX(pos), y = COMBOY(pos);
	
	bool skipSecrets = isNextType(type) && !get_qr(qr_OLD_SLASHNEXT_SECRETS);
	bool done = false;
	if(!skipSecrets)
	{
		done = true;
		if((flag >= 16 && flag <= 31))
		{  
			tmp->data[pos] = tmp->secretcombo[flag-16+4];
			tmp->cset[pos] = tmp->secretcset[flag-16+4];
			tmp->sflag[pos] = tmp->secretflag[flag-16+4];
		}
		else if(flag == mfARMOS_SECRET)
		{
			tmp->data[pos] = tmp->secretcombo[sSTAIRS];
			tmp->cset[pos] = tmp->secretcset[sSTAIRS];
			tmp->sflag[pos] = tmp->secretflag[sSTAIRS];
			sfx(tmpscr->secretsfx);
		}
		else if((flag>=mfSWORD && flag<=mfXSWORD) || flag==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; ++i)
			{
				findentrance(x,y,mfSWORD+i,true);
			}
			
			findentrance(x,y,mfSTRIKE,true);
		}
		else if(flag2 >= 16 && flag2 <= 31)
		{ 
			tmp->data[pos] = tmp->secretcombo[(tmp->sflag[pos])-16+4];
			tmp->cset[pos] = tmp->secretcset[(tmp->sflag[pos])-16+4];
			tmp->sflag[pos] = tmp->secretflag[(tmp->sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			tmp->data[pos] = tmp->secretcombo[sSTAIRS];
			tmp->cset[pos] = tmp->secretcset[sSTAIRS];
			tmp->sflag[pos] = tmp->secretflag[sSTAIRS];
			sfx(tmpscr->secretsfx);
		}
		else if((flag2>=mfSWORD && flag2<=mfXSWORD)|| flag2==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; i++)
			{
				findentrance(x,y,mfSWORD+i,true);
			}
			
			findentrance(x,y,mfSTRIKE,true);
		}
		else done = false;
	}
	if(!done)
	{
		if(isCuttableNextType(type))
		{
			tmp->data[pos]++;
		}
		else
		{
			tmp->data[pos] = tmp->undercombo;
			tmp->cset[pos] = tmp->undercset;
			tmp->sflag[pos] = 0;
		}
	}
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr->secretsfx);
	}
	else if(isCuttableItemType(type))
	{
		int32_t it = -1;
		int32_t thedropset = -1;
		if (cmb.usrflags&cflag2) //specific dropset or item
		{
			if (cmb.usrflags&cflag11) 
			{
				it = cmb.attribytes[1];
			}
			else
			{
				it = select_dropitem(cmb.attribytes[1]);
				thedropset = cmb.attribytes[1];
			}
		}
		else
		{
			it = select_dropitem(12);
			thedropset = 12;
		}
		
		if(it!=-1)
		{
			item* itm = (new item((zfix)x, (zfix)y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
	}
	
	if(get_qr(qr_MORESOUNDS))
	{
		if (cmb.usrflags&cflag3)
		{
			sfx(cmb.attribytes[2],int32_t(x));
		}
		else if (isBushType(type) || isFlowersType(type) || isGrassType(type))
		{
			sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(x));
		}
	}
	spawn_decoration(cmb, pos);
}

void trigger_cuttable_ffc(int32_t pos)
{
	if(unsigned(pos) > MAXFFCS) return;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	auto type = cmb.type;
	if (!isCuttableType(type)) return;
	auto flag2 = cmb.flag;
	auto x = ffc.x, y = ffc.y;
	
	bool skipSecrets = isNextType(type) && !get_qr(qr_OLD_SLASHNEXT_SECRETS);
	bool done = false;
	if(!skipSecrets)
	{
		done = true;
		if(flag2 >= 16 && flag2 <= 31)
		{ 
			zc_ffc_set(ffc, tmpscr->secretcombo[(tmpscr->sflag[pos])-16+4]);
			ffc.cset = tmpscr->secretcset[(tmpscr->sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			zc_ffc_set(ffc, tmpscr->secretcombo[sSTAIRS]);
			ffc.cset = tmpscr->secretcset[sSTAIRS];
			sfx(tmpscr->secretsfx);
		}
		else if((flag2>=mfSWORD && flag2<=mfXSWORD)|| flag2==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; i++)
			{
				findentrance(x,y,mfSWORD+i,true);
			}
			
			findentrance(x,y,mfSTRIKE,true);
		}
		else done = false;
	}
	if(!done)
	{
		if(isCuttableNextType(type))
		{
			zc_ffc_modify(ffc, 1);
		}
		else
		{
			zc_ffc_set(ffc, tmpscr->undercombo);
			ffc.cset = tmpscr->undercset;
		}
	}
	
	if((flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr->secretsfx);
	}
	else if(isCuttableItemType(type))
	{
		int32_t it = -1;
		int32_t thedropset = -1;
		if (cmb.usrflags&cflag2) //specific dropset or item
		{
			if (cmb.usrflags&cflag11) 
			{
				it = cmb.attribytes[1];
			}
			else
			{
				it = select_dropitem(cmb.attribytes[1]);
				thedropset = cmb.attribytes[1];
			}
		}
		else
		{
			it = select_dropitem(12);
			thedropset = 12;
		}
		
		if(it!=-1)
		{
			item* itm = (new item((zfix)x, (zfix)y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
	}
	
	if(get_qr(qr_MORESOUNDS))
	{
		if (cmb.usrflags&cflag3)
		{
			sfx(cmb.attribytes[2],int32_t(x));
		}
		else if (isBushType(type) || isFlowersType(type) || isGrassType(type))
		{
			sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(x));
		}
	}
	spawn_decoration_xy(cmb, x, y);
}

bool trigger_step(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	if(!isStepType(cmb.type) || cmb.type == cSTEPCOPY) return false;
	if(cmb.attribytes[1] && !game->item[cmb.attribytes[1]])
		return false; //lacking required item
	if((cmb.usrflags & cflag1) && !Hero.HasHeavyBoots())
		return false;
	if(cmb.attribytes[0])
		sfx(cmb.attribytes[0], pan(COMBOX(pos)));
	switch(cmb.type)
	{
		case cSTEP:
			++tmp->data[pos]; break;
		case cSTEPSAME:
		{
			int32_t id = tmp->data[pos];
			for(auto q = 0; q < 176; ++q)
			{
				if(tmpscr->data[q] == id)
				{
					++tmpscr->data[q];
				}
			}
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr->ffcs[i];
					if (ffc2.data == id)
					{
						zc_ffc_modify(ffc2, 1);
					}
				}
			}
			if(tmp != tmpscr) ++tmp->data[pos];
			break;
		}
		case cSTEPALL:
		{
			for(auto q = 0; q < 176; ++q)
			{
				if(isStepType(combobuf[tmpscr->data[q]].type))
				{
					++tmpscr->data[q];
				}
			}
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr->ffcs[i];
					if (isStepType(combobuf[ffc2.data].type))
					{
						zc_ffc_modify(ffc2, 1);
					}
				}
			}
			if(tmp != tmpscr) ++tmp->data[pos];
			break;
		}
	}
	return true;
}

bool trigger_step_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	if(!isStepType(cmb.type) || cmb.type == cSTEPCOPY) return false;
	if(cmb.attribytes[1] && !game->item[cmb.attribytes[1]])
		return false; //lacking required item
	if((cmb.usrflags & cflag1) && !Hero.HasHeavyBoots())
		return false;
	if(cmb.attribytes[0])
		sfx(cmb.attribytes[0], pan(ffc.x));
	switch(cmb.type)
	{
		case cSTEP:
		{
			zc_ffc_modify(ffc, 1); 
			break;
		}
		case cSTEPSAME:
		{
			int32_t id = ffc.data;
			for(auto q = 0; q < 176; ++q)
			{
				if(tmpscr->data[q] == id)
				{
					++tmpscr->data[q];
				}
			}
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr->ffcs[i];
					if (ffc2.data == id && i != pos)
					{
						zc_ffc_modify(ffc2, 1);
					}
				}
			}
			zc_ffc_modify(ffc, 1);
			break;
		}
		case cSTEPALL:
		{
			for(auto q = 0; q < 176; ++q)
			{
				if(isStepType(combobuf[tmpscr->data[q]].type))
				{
					++tmpscr->data[q];
				}
			}
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr->ffcs[i];
					if (isStepType(combobuf[ffc2.data].type) && i != pos)
					{
						zc_ffc_modify(ffc2, 1);
					}
				}
			}
			zc_ffc_modify(ffc, 1);
			break;
		}
	}
	return true;
}


bool can_locked_combo(newcombo const& cmb) //cLOCKBLOCK or cLOCKEDCHEST specifically
{
	switch(cmb.type) //sanity check
	{
		case cLOCKBLOCK: case cLOCKEDCHEST:
			break;
		default: return false; //not a locked container?
	}
	int32_t requireditem = cmb.usrflags&cflag1 ? cmb.attribytes[0] : 0;
	int32_t itemonly = cmb.usrflags&cflag2;
	int32_t thecounter = cmb.attribytes[1];
	int32_t ctr_amount = cmb.attributes[0]/10000L;
	if( requireditem && game->item[requireditem]) 
	{
		return true;
	}
	else if((cmb.usrflags&cflag1) && itemonly) return false; //Nothing but item works
	else if ( (cmb.usrflags&cflag4) )
	{
		if ( game->get_counter(thecounter) >= ctr_amount )
		{
			return true;
		}
		else if (cmb.usrflags&cflag6) //eat counter even if insufficient, but don't unlock
		{
			return false;
		}
	}
	else if (ctr_amount && canUseKey(ctr_amount) ) return true;
	else if(!ctr_amount && !requireditem && !itemonly && canUseKey() ) return true;
	return false;
}

bool try_locked_combo(newcombo const& cmb) //cLOCKBLOCK or cLOCKEDCHEST specifically
{
	switch(cmb.type) //sanity check
	{
		case cLOCKBLOCK: case cLOCKEDCHEST:
			break;
		default: return false; //not a locked container?
	}
	int32_t requireditem = cmb.usrflags&cflag1 ? cmb.attribytes[0] : 0;
	int32_t itemonly = cmb.usrflags&cflag2;
	int32_t thecounter = cmb.attribytes[1];
	int32_t ctr_amount = cmb.attributes[0]/10000L;
	if( requireditem && game->item[requireditem]) 
	{
		if ((cmb.usrflags&cflag5)) 
		{
			takeitem(requireditem);
		}
		return true;
	}
	else if((cmb.usrflags&cflag1) && itemonly) return false; //Nothing but item works
	else if ((cmb.usrflags&cflag4))
	{
		bool nodrain = (cmb.usrflags&cflag8);
		bool thief = !nodrain && (cmb.usrflags&cflag6);
		if ( game->get_counter(thecounter) >= ctr_amount )
		{
			//flag 6 only checks the required count; it doesn't drain it
			if (!nodrain)
				game->change_counter(-(ctr_amount), thecounter);
			return true;
		}
		else if (thief) //eat counter even if insufficient, but don't unlock
		{
			game->change_counter(-(game->get_counter(thecounter)), thecounter);
			return false;
		}
	}
	else if (ctr_amount && usekey(ctr_amount) ) return true;
	else if(!ctr_amount && !requireditem && !itemonly && usekey() ) return true;
	return false;
}

bool play_combo_string(int str)
{
	switch(str)
	{
		case -1: //Special case: Use Screen String
			str = tmpscr->str;
			break;
		case -2: //Special case: Use Screen Catchall
			str = tmpscr->catchall;
			break;
		case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17: //Special case: Screen->D[]
			int32_t di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			str = game->screen_d[di][abs(str)-10] / 10000L;
			break;
	}
	if(!str || unsigned(str) >= MAXMSGS)
		return false;
	donewmsg(str);
	return true;
}

void trigger_sign(newcombo const& cmb)
{
	play_combo_string(cmb.attributes[0]/10000L);
}

bool trigger_warp(newcombo const& cmb)
{
	if(!isWarpType(cmb.type)) return false;
	mapscr* wscr = &tmpscr[(currscr<128)?0:1];
	auto index = getWarpLetter(cmb.type);
	if(index == 4) index = zc_oldrand()%4; //Random warp
	auto wtype = wscr->tilewarptype[index];
	if(wtype==wtNOWARP)
		return false;
	auto stype = simplifyWarpType(cmb.type);
	auto wsfx = cmb.attribytes[0];
	auto tdm = wscr->tilewarpdmap[index];
	auto tscr = wscr->tilewarpscr[index];
	auto wrindex=(wscr->warpreturnc>>(index*2))&3;
	int32_t ws=(DMaps[tdm].map*MAPSCRS+wscr->tilewarpscr[index]+DMaps[tdm].xoff);
	mapscr* wscr2=&TheMaps[ws];
	auto wx = wscr2->warpreturnx[wrindex];
	auto wy = wscr2->warpreturny[wrindex];
	if(stype == cPIT)
	{
		wx = -1;
		wy = -1;
	}
	auto weff = warpEffectNONE;
	switch(wtype)
	{
		case wtIWARPZAP:
			wtype = wtIWARP;
			weff = warpEffectZap;
			break;
		case wtIWARPBLK:
			wtype = wtIWARP;
			weff = warpEffectInstant;
			break;
		case wtIWARPOPEN:
			wtype = wtIWARP;
			weff = warpEffectOpen;
			break;
		case wtIWARPWAVE:
			wtype = wtIWARP;
			weff = warpEffectWave;
			break;
	}
	
	auto wflag = 0;
	if(tmpscr->flags3&fIWARPFULLSCREEN) wflag |= warpFlagDONTCLEARSPRITES;
	//Queue the warp for the next frame, as doing anything else breaks terribly
	FFCore.queueWarp(wtype, tdm, tscr, wx, wy, weff, wsfx, wflag, -1);
	return true;
}

bool trigger_chest(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	newcombo const& cmb = combobuf[FFCore.tempScreens[lyr]->data[pos]];
	switch(cmb.type)
	{
		case cLOCKEDCHEST: //Special flags!
			//if(!usekey()) return; //Old check
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mCHEST);
			break;
			
		case cBOSSCHEST:
			if(!(game->lvlitems[dlevel]&liBOSSKEY))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			// Run Boss Key Script
			int32_t key_item = 0; //current_item_id(itype_bosskey); //not possible
			for ( int32_t q = 0; q < MAXITEMS; ++q )
			{
				if ( itemsbuf[q].family == itype_bosskey )
				{
					key_item = q; break;
				}
			}
			if ( key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
			{
				FFCore.reset_script_engine_data(ScriptType::Item, key_item);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllScriptOwned(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mBOSSCHEST);
			break;
	}
	
	sfx(cmb.attribytes[3]); //opening sfx
	
	bool itemflag = false;
	for(int32_t i=0; i<3; i++)
	{
		if(FFCore.tempScreens[i]->sflag[pos]==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
		if(combobuf[FFCore.tempScreens[i]->data[pos]].flag==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
	}
	bool itemstate = false;
	int32_t ipflag = 0;
	if(cmb.usrflags & cflag7)
	{
		itemstate = getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = tmpscr->catchall;
				break;
		}
		if(unsigned(itid) >= MAXITEMS) itid = 0;
		item* itm = new item(Hero.getX(), Hero.getY(), 0, itid, pflags, 0);
		itm->set_forcegrab(true);
		items.add(itm);
	}
	return true;
}

bool trigger_chest_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	int32_t cid = ffc.data;
	switch(cmb.type)
	{
		case cLOCKEDCHEST: //Special flags!
			//if(!usekey()) return; //Old check
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mCHEST);
			break;
			
		case cBOSSCHEST:
			if(!(game->lvlitems[dlevel]&liBOSSKEY))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			// Run Boss Key Script
			int32_t key_item = 0; //current_item_id(itype_bosskey); //not possible
			for ( int32_t q = 0; q < MAXITEMS; ++q )
			{
				if ( itemsbuf[q].family == itype_bosskey )
				{
					key_item = q; break;
				}
			}
			if ( key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
			{
				FFCore.reset_script_engine_data(ScriptType::Item, key_item);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllScriptOwned(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mBOSSCHEST);
			break;
	}
	
	sfx(cmb.attribytes[3]); //opening sfx
	
	bool itemflag = false;
	if(combobuf[cid].flag==mfARMOS_ITEM)
	{
		itemflag = true;
	}
	bool itemstate = false;
	int32_t ipflag = 0;
	if(cmb.usrflags & cflag7)
	{
		itemstate = getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = tmpscr->catchall;
				break;
		}
		if(unsigned(itid) >= MAXITEMS) itid = 0;
		item* itm = new item(Hero.getX(), Hero.getY(), 0, itid, pflags, 0);
		itm->set_forcegrab(true);
		items.add(itm);
	}
	return true;
}


bool trigger_lockblock(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	newcombo const& cmb = combobuf[FFCore.tempScreens[lyr]->data[pos]];
	switch(cmb.type)
	{
		case cLOCKBLOCK: //Special flags!
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mLOCKBLOCK);
			remove_lockblocks((currscr>=128)?1:0);
			break;
			
		case cBOSSLOCKBLOCK:
		{
			if (!(game->lvlitems[dlevel] & liBOSSKEY))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			// Run Boss Key Script
			int32_t key_item = 0;
			for (int32_t q = 0; q < MAXITEMS; ++q)
			{
				if (itemsbuf[q].family == itype_bosskey)
				{
					key_item = q; break;
				}
			}
			if (key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				FFCore.reset_script_engine_data(ScriptType::Item, key_item);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllScriptOwned(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mBOSSLOCKBLOCK);
			remove_bosslockblocks((currscr >= 128) ? 1 : 0);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}

bool trigger_lockblock_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	switch(cmb.type)
	{
		case cLOCKBLOCK: //Special flags!
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mLOCKBLOCK);
			remove_lockblocks((currscr>=128)?1:0);
			break;
			
		case cBOSSLOCKBLOCK:
		{
			if (!(game->lvlitems[dlevel] & liBOSSKEY))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			// Run Boss Key Script
			int32_t key_item = 0;
			for (int32_t q = 0; q < MAXITEMS; ++q)
			{
				if (itemsbuf[q].family == itype_bosskey)
				{
					key_item = q; break;
				}
			}
			if (key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				FFCore.reset_script_engine_data(ScriptType::Item, key_item);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllScriptOwned(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(1<<cmb.attribytes[5]);
				remove_xstatecombos((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(mBOSSLOCKBLOCK);
			remove_bosslockblocks((currscr >= 128) ? 1 : 0);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}


bool trigger_armos_grave(int32_t lyr, int32_t pos, int32_t trigdir)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	if(lyr != 0) return false; //Currently cannot activate on layers >0!
	//!TODO Expand 'guygrid' stuff to account for layers, so that layers >0 can be used
	if(guygrid[pos]) return false; //Currently activating
	int32_t gc = 0;
	for(int32_t i=0; i<guys.Count(); ++i)
	{
		if(((enemy*)guys.spr(i))->mainguy)
		{
			++gc;
		}
	}
	if(gc > 10) return false; //Unsure what this purpose is
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	int32_t eclk = -14;
	int32_t id2 = 0;
	int32_t tx = COMBOX(pos), ty = COMBOY(pos);
	bool nextcmb = false;
	switch(cmb.type)
	{
		case cARMOS:
		{
			if(cmb.usrflags&cflag1) //custom ID
			{
				int32_t r = (cmb.usrflags&cflag2) ? zc_oldrand()%2 : 0;
				id2 = cmb.attribytes[0+r];
			}
			else //default ID
			{
				for(int32_t i=0; i<eMAXGUYS; i++)
				{
					if(guysbuf[i].flags&guy_armos)
					{
						id2=i;
						
						// This is mostly for backwards-compatability
						if(guysbuf[i].family==eeWALK && guysbuf[i].attributes[8] == e9tARMOS)
						{
							eclk=0;
						}
						
						break;
					}
				}
			}
			
			if(cmb.usrflags&cflag3) //handle large enemy (EARLY RETURN)
			{
				guygrid[pos] = 61;
				//! To-do Adjust for larger enemies, but we need it to be directional. 
				int32_t ypos = 0; int32_t xpos = 0;
				int32_t chy = 0; int32_t chx = 0;
				//nmew idea = check while the upper-left corner combo is armos
				///move up one and check if it is armos, check the next, and stop as soon as that is not armos
				///then do the same going left
				
				int32_t searching = 1;
				int32_t armosxsz = 1;
				int32_t armosysz = 1;
				switch(guysbuf[id2].family)
				{
					case eeGHOMA:
						armosxsz = 3;
						break;
					case eeAQUA: //jesus christ I'm not considering the logistics of manhandlas and gleeoks
					case eeDIG:
						armosxsz = 2;
						armosysz = 2;
						break;
					default:
						break;
				}
				if ((guysbuf[id2].SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) != 0) armosxsz = guysbuf[id2].txsz;
				if ((guysbuf[id2].SIZEflags&guyflagOVERRIDE_TILE_WIDTH) != 0) armosysz = guysbuf[id2].tysz;
				
				if ( armosxsz > 1 || armosysz > 1 )
				{
					switch(trigdir)
					{
						case -1: //triggered not by touch
							[[fallthrough]];
						case up: //touched armos from below
						{
							while(searching == 1) //find the top edge of an armos block
							{
								chy += 16;
								if ( pos - chy < 0 ) break; //don't go out of bounds
								if ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS )
								{
									ypos -=16;
								}
								else searching = 2;
							}
							while(searching == 2) //find the left edge of an armos block
							{
								if ( (pos % 16) == 0 || pos == 0 ) break; //don't wrap rows
								++chx;
								if ( pos - chx < 0 ) break; //don't go out of bounds
								if ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS )
								{
									xpos -=16;
								}
								else searching = 3;
							}
							
							break;
						}
						case down: //touched armos from above
						{
							//zprint("touched armos from above\n");
							//zprint("cpos: %d\n", cpos);
							//int32_t tx2 = (int32_t)x; //COMBOX(COMBOPOS(tx,ty));
							//int32_t ty2 = (int32_t)y+16; //COMBOY(COMBOPOS(tx,ty));
							//tx2 = GridX(tx2);
							//ty2 = GridY(ty2);
							while(searching == 1) //find the left edge of an armos block
							{
								//zprint("searching\n");
								if ( (pos % 16) == 0 ) break; //don't wrap rows
								++chx;
								
								
								//zprint("chx: %d\n", chx);
								//zprint("tx2: %d\n", tx2);
								//zprint("ty2: %d\n", ty2);
								//zprint("MAPCOMBO(tx2,ty2): %d\n",MAPCOMBO(tx2,ty2));
								//zprint("MAPCOMBO(tx2-chx,ty2): %d\n",MAPCOMBO(GridX(tx2-chx),ty2));
								if ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS )
								{
									//zprint("found match\n");
									xpos -=16;
								}
								else searching = 3;
							}
							//zprint("xpos is: %d\n", xpos);
						}
						[[fallthrough]];
						case left: //touched right edge of armos
						{
							while(searching == 1) //find the top edge of an armos block
							{
								chy += 16;
								if ( pos - chy < 0 ) break; //don't go out of bounds
								if ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS )
								{
									ypos -=16;
								}
								else searching = 2;
							}
							while(searching == 2) //find the left edge of an armos block
							{
								if ( (pos % 16) == 0 || pos == 0 ) break; //don't wrap rows
								++chx;
								if ( pos - chx < 0 ) break; //don't go out of bounds
								if ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS ) 
								{
									xpos -=16;
								}
								else searching = 3;
							}
							break;
						}
							
						case right: //touched left edge of armos
						{
							//zprint("touched armos on left\n");
							while(searching == 1) //find the top edge of an armos block
							{
								chy += 16;
								if ( pos - chy < 0 ) break; //don't go out of bounds
								if ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS ) 
								{
									//zprint("found match\n");
									ypos -=16;
								}
								else searching = 2;
							}
							break;
						}
					
						
					}
				}
				
				int32_t xpos2 = tx+xpos;
				int32_t ypos2 = ty+ypos;
				int32_t id3 = COMBOPOS(xpos2, ypos2);
				for (int32_t n = 0; n < armosysz && id3 < 176; n++)
				{
					for (int32_t m = 0; m < armosxsz && id3 < 176; m++) 
					{
						if (id3 + m < 176)
							guygrid[(id3+m)]=61;
					}
					id3+=16;
				}
				if (guysbuf[id2].family == eeGHOMA) 
				{
					if ( combobuf[(tmpscr->data[pos-chx+1])].type == cARMOS ) xpos += 16;
				}
				if(addenemy(tx+xpos,ty+1+ypos,id2,0))
				{
					enemy* en = ((enemy*)guys.spr(guys.Count()-1));
					en->did_armos=false;
					en->fading=fade_flicker;
					en->flags |= guy_armos;
				}
				return true;
			}
			break;
		}
		case cBSGRAVE:
			nextcmb = true;
			[[fallthrough]];
		case cGRAVE:
		{
			if(cmb.usrflags&cflag1) //Custom ID
			{
				int32_t r = (cmb.usrflags&cflag2) ? zc_oldrand()%2 : 0;
				id2 = cmb.attribytes[0+r];
			}
			else //Default ID
			{
				for(int32_t i=0; i<eMAXGUYS; i++)
				{
					if(guysbuf[i].flags&guy_ghini)
					{
						id2=i;
						eclk=0; // This is mostly for backwards-compatability
						break;
					}
				}
			}
			if(nextcmb)
				tmp->data[pos]++;
			break;
		}
		default: return false;
	}
	guygrid[pos] = 61;
	if(addenemy(tx,ty+3,id2,eclk))
		((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
	else return false;
	return true;
}

bool trigger_armos_grave_ffc(int32_t pos, int32_t trigdir)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	if(guygridffc[pos]) return false; //Currently activating
	int32_t gc = 0;
	for(int32_t i=0; i<guys.Count(); ++i)
	{
		if(((enemy*)guys.spr(i))->mainguy)
		{
			++gc;
		}
	}
	if(gc > 10) return false; //Don't do it if there's already 10 enemies onscreen
	//!TODO: Maybe allow a custom limit?
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	int32_t eclk = -14;
	int32_t id2 = 0;
	int32_t tx = ffc.x, ty = ffc.y;
	bool nextcmb = false;
	switch(cmb.type)
	{
		case cARMOS:
		{
			if(cmb.usrflags&cflag1) //custom ID
			{
				int32_t r = (cmb.usrflags&cflag2) ? zc_oldrand()%2 : 0;
				id2 = cmb.attribytes[0+r];
			}
			else //default ID
			{
				for(int32_t i=0; i<eMAXGUYS; i++)
				{
					if(guysbuf[i].flags&guy_armos)
					{
						id2=i;
						
						// This is mostly for backwards-compatability
						if(guysbuf[i].family==eeWALK && guysbuf[i].attributes[8] == e9tARMOS)
						{
							eclk=0;
						}
						
						break;
					}
				}
			}
			break;
		}
		case cBSGRAVE:
			nextcmb = true;
			[[fallthrough]];
		case cGRAVE:
		{
			if(cmb.usrflags&cflag1) //Custom ID
			{
				int32_t r = (cmb.usrflags&cflag2) ? zc_oldrand()%2 : 0;
				id2 = cmb.attribytes[0+r];
			}
			else //Default ID
			{
				for(int32_t i=0; i<eMAXGUYS; i++)
				{
					if(guysbuf[i].flags&guy_ghini)
					{
						id2=i;
						eclk=0; // This is mostly for backwards-compatability
						break;
					}
				}
			}
			if(nextcmb)
				zc_ffc_modify(ffc, 1);
			break;
		}
		default: return false;
	}
	guygridffc[pos] = 61;
	if(addenemy(tx,ty+3,id2,eclk))
	{
		((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
		((enemy*)guys.spr(guys.Count()-1))->ffcactivated=pos+1;
	}
	else return false;
	return true;
}


bool trigger_damage_combo(int32_t cid, int type, int ptrval, int32_t hdir, bool force_solid)
{
	if(hdir > 3) hdir = -1;
	newcombo const& cmb = combobuf[cid];
	if(Hero.hclk || Hero.superman || Hero.fallclk)
		return false; //immune
	int32_t dmg = 0;
	if(cmb.usrflags & cflag1) //custom
		dmg = cmb.attributes[0] / -10000L;
	else dmg = combo_class_buf[cmb.type].modify_hp_amount;
	
	bool global_defring = ((itemsbuf[current_item_id(itype_ring)].flags & item_flag1));
	bool global_perilring = ((itemsbuf[current_item_id(itype_perilring)].flags & item_flag1));
	bool current_ring = ((tmpscr->flags6&fTOGGLERINGDAMAGE) != 0);
	if(current_ring)
	{
		global_defring = !global_defring;
		global_perilring = !global_perilring;
	}
	
	int32_t itemid = current_item_id(itype_boots);
	
	bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & item_flag1);
	bool ignoreBoots = itemid >= 0 && (itemsbuf[itemid].flags & item_flag3);
	if(dmg < 0)
	{
		if(itemid < 0 || ignoreBoots || (tmpscr->flags5&fDAMAGEWITHBOOTS)
			|| (4<<current_item_power(itype_boots)<(abs(dmg)))
			|| ((force_solid||(cmb.walk&0xF)) && bootsnosolid)
			|| !(checkbunny(itemid) && checkmagiccost(itemid)))
		{
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(-dmg*10000);
			ev.push_back(hdir*10000);
			ev.push_back(0);
			ev.push_back(Hero.DivineProtectionShieldClk>0?10000:0);
			ev.push_back(48*10000);
			ev.push_back(ZSD_COMBODATA*10000);
			ev.push_back(cid);
			ev.push_back(type*10000);
			ev.push_back(ptrval);
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
			int32_t dmg = ev[0]/10000;
			bool nullhit = ev[2] != 0;
			
			if(nullhit) {ev.clear(); return false;}
			
			//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
			ev[0] = Hero.ringpower(dmg, !global_perilring, !global_defring)*10000;
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
			dmg = ev[0]/10000;
			hdir = ev[1]/10000;
			nullhit = ev[2] != 0;
			bool divineprot = ev[3] != 0;
			int32_t iframes = ev[4] / 10000;
			ev.clear();
			if(nullhit) return false;
			
			if(!divineprot)
			{
				game->set_life(zc_max(game->get_life()-dmg,0));
			}
			Hero.doHit(hdir); //set hit action, iframes, etc
			Hero.hclk = iframes;
			return true;
		}
		else paymagiccost(itemid); //boots succeeded
	}
	return false;
}

bool trigger_stepfx(int32_t lyr, int32_t pos, bool stepped)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	int32_t tx = COMBOX(pos), ty = COMBOY(pos);
	int32_t thesfx = cmb.attribytes[0];
	sfx_no_repeat(thesfx,pan(COMBOX(pos)));
	if ( cmb.usrflags&cflag1) //landmine
	{
		int32_t wpn = cmb.attribytes[1];
		int32_t wpdir = cmb.attribytes[2];
		if ( ((unsigned)wpdir) > r_down )
		{
			wpdir = zc_oldrand()&3;
		}
		int32_t damg = cmb.attributes[0]/10000L;
		if(damg < 1) damg = 4;
		auto parentitem = cmb.attribytes[4]>0 ? cmb.attribytes[4] : -1;
		auto wlvl = parentitem>-1 ? itemsbuf[parentitem].fam_type : 0;
		switch(wpn)
		{
			//eweapons
			case ewFireball:
			case ewArrow:
			case ewBrang:
			case ewSword:
			case ewRock:
			case ewMagic:
			case ewBomb:
			case ewSBomb:
			case ewLitBomb:
			case ewLitSBomb:
			case ewFireTrail:
			case ewFlame:
			case ewWind:
			case ewFlame2:
			case ewFlame2Trail:
			case ewIce:
			case ewFireball2:
			
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wBeam:
			case wBrang:
			case wBomb:
			case wSBomb:
			case wLitBomb:
			case wLitSBomb:
			case wArrow:
			
			case wWhistle:
			case wBait:
			case wMagic:
			case wWind:
			case wRefMagic:
			case wRefFireball:
			case wRefRock:
			case wRefBeam:
			case wIce:
			case wFlame: 
			case wSound: // -Z: sound + defence split == digdogger, sound + one hit kill == pols voice -Z
			//case wThrown: 
			//case wPot: //Thrown pot or rock -Z
			//case wLit: //Lightning or Electric -Z
			//case wBombos: 
			//case wEther: 
			//case wQuake:// -Z
			//case wSword180: 
			//case wSwordLA:
			case wFire:
			case wRefArrow:
			case wRefFire:
			case wRefFire2:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
				
			case wScript1:
			case wScript2: 
			case wScript3:
			case wScript4:
			case wScript5:
			case wScript6:
			case wScript7:
			case wScript8:
			case wScript9:
			case wScript10:
				if (cmb.usrflags&cflag2) //wscript lwpn
				{
					if (cmb.usrflags&cflag4) //direct damage from custom/script lweapons
					{
						if(stepped)
						{
							Hero.hitdir = -1;
							if (Hero.action != rafting && Hero.action != freeze && Hero.action!=sideswimfreeze && !Hero.hclk)
							{
								int32_t dmgamt = damg;
								game->set_life(game->get_life()- Hero.ringpower(dmgamt));
								Hero.doHit(-1);
							}
						}
					}
					else
					{
						Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
						if (cmb.attribytes[3] > 0 )
						{
							weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
							w->LOADGFX(cmb.attribytes[3]);
						}
					}
					break;
				}
				else //wscript ewpn
				{
					Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false)); 
					if (cmb.attribytes[3] > 0 )
					{
						weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
						w->LOADGFX(cmb.attribytes[3]);
					}
					break;
				}
				
			case wSSparkle:
			case wFSparkle:
				if (cmb.usrflags&cflag4) //direct damage from custom/script lweapons
				{
					if(stepped)
					{
						Hero.hitdir = -1;
						if (Hero.action != rafting && Hero.action != freeze && Hero.action != sideswimfreeze && !Hero.hclk)
						{
							Hero.doHit(-1);
							int32_t dmgamt = damg;
							
							game->set_life(game->get_life()- Hero.ringpower(dmgamt));
						}
					}
				}
				else
				{
					Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
					if (cmb.attribytes[3] > 0 )
					{
						weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
						w->LOADGFX(cmb.attribytes[3]);
					}
				}
				break;
			
			default: //enemy bomb
				//(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentitem, int32_t prntid, bool isDummy, byte script_gen, byte isLW, byte special) : sprite(), parentid(
				//Ewpns.add(new weapon((zfix)tx+8,(zfix)ty+8,(zfix)0,ewLitBomb,16,0,0, -1,-1,false)); break;
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,damg,up, -1,-1,false)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			//x,y,z, wpn, 0, dmisc4, dir,-1,getUID(),false);
		}
		if (!(cmb.usrflags&cflag3) ) //Don't Advance
		{
			tmp->data[pos]++;
		}
	}
	return true;
}

bool trigger_stepfx_ffc(int32_t pos, bool stepped)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	int32_t tx = ffc.x, ty = ffc.y;
	int32_t thesfx = cmb.attribytes[0];
	sfx_no_repeat(thesfx, pan(ffc.x));
	if ( cmb.usrflags&cflag1) //landmine
	{
		int32_t wpn = cmb.attribytes[1];
		int32_t wpdir = cmb.attribytes[2];
		if ( ((unsigned)wpdir) > r_down )
		{
			wpdir = zc_oldrand()&3;
		}
		int32_t damg = cmb.attributes[0]/10000L;
		if(damg < 1) damg = 4;
		auto parentitem = cmb.attribytes[4]>0 ? cmb.attribytes[4] : -1;
		auto wlvl = parentitem>-1 ? itemsbuf[parentitem].fam_type : 0;
		switch(wpn)
		{
			//eweapons
			case ewFireball:
			case ewArrow:
			case ewBrang:
			case ewSword:
			case ewRock:
			case ewMagic:
			case ewBomb:
			case ewSBomb:
			case ewLitBomb:
			case ewLitSBomb:
			case ewFireTrail:
			case ewFlame:
			case ewWind:
			case ewFlame2:
			case ewFlame2Trail:
			case ewIce:
			case ewFireball2:
			
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wBeam:
			case wBrang:
			case wBomb:
			case wSBomb:
			case wLitBomb:
			case wLitSBomb:
			case wArrow:
			
			case wWhistle:
			case wBait:
			case wMagic:
			case wWind:
			case wRefMagic:
			case wRefFireball:
			case wRefRock:
			case wRefBeam:
			case wIce:
			case wFlame: 
			case wSound: // -Z: sound + defence split == digdogger, sound + one hit kill == pols voice -Z
			//case wThrown: 
			//case wPot: //Thrown pot or rock -Z
			//case wLit: //Lightning or Electric -Z
			//case wBombos: 
			//case wEther: 
			//case wQuake:// -Z
			//case wSword180: 
			//case wSwordLA:
			case wFire:
			case wRefArrow:
			case wRefFire:
			case wRefFire2:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
				
			case wScript1:
			case wScript2: 
			case wScript3:
			case wScript4:
			case wScript5:
			case wScript6:
			case wScript7:
			case wScript8:
			case wScript9:
			case wScript10:
				if (cmb.usrflags&cflag2) //wscript lwpn
				{
					if (cmb.usrflags&cflag4) //direct damage from custom/script lweapons
					{
						if(stepped)
						{
							Hero.hitdir = -1;
							if (Hero.action != rafting && Hero.action != freeze && Hero.action!=sideswimfreeze && !Hero.hclk)
							{
								int32_t dmgamt = damg;
								game->set_life(game->get_life()- Hero.ringpower(dmgamt));
								Hero.doHit(-1);
							}
						}
					}
					else
					{
						Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
						if (cmb.attribytes[3] > 0 )
						{
							weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
							w->LOADGFX(cmb.attribytes[3]);
						}
					}
					break;
				}
				else //wscript ewpn
				{
					Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false)); 
					if (cmb.attribytes[3] > 0 )
					{
						weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
						w->LOADGFX(cmb.attribytes[3]);
					}
					break;
				}
				
			case wSSparkle:
			case wFSparkle:
				if (cmb.usrflags&cflag4) //direct damage from custom/script lweapons
				{
					if(stepped)
					{
						Hero.hitdir = -1;
						if (Hero.action != rafting && Hero.action != freeze && Hero.action != sideswimfreeze && !Hero.hclk)
						{
							Hero.doHit(-1);
							int32_t dmgamt = damg;
							
							game->set_life(game->get_life()- Hero.ringpower(dmgamt));
						}
					}
				}
				else
				{
					Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
					if (cmb.attribytes[3] > 0 )
					{
						weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
						w->LOADGFX(cmb.attribytes[3]);
					}
				}
				break;
			
			default: //enemy bomb
				//(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentitem, int32_t prntid, bool isDummy, byte script_gen, byte isLW, byte special) : sprite(), parentid(
				//Ewpns.add(new weapon((zfix)tx+8,(zfix)ty+8,(zfix)0,ewLitBomb,16,0,0, -1,-1,false)); break;
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,damg,up, -1,-1,false)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Ewpns.spr(Ewpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			//x,y,z, wpn, 0, dmisc4, dir,-1,getUID(),false);
		}
		if (!(cmb.usrflags&cflag3)) //Don't Advance
		{
			zc_ffc_modify(ffc, 1);
		}
	}
	return true;
}


bool trigger_switchhookblock(int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	if(Hero.switchhookclk) return false;
	switching_object = NULL;
	hooked_combopos = pos;
	hooked_layerbits = 0;
	Hero.doSwitchHook(game->get_switchhookstyle());
	if(!hooked_layerbits) //failed
	{
		Hero.reset_hookshot();
		return false;
	}
	return true;
}

bool trigger_switchhookblock_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	if(Hero.switchhookclk) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	switching_object = &ffc;
	switching_object->switch_hooked = true;
	hooked_combopos = -1;
	hooked_layerbits = 0;
	Hero.doSwitchHook(game->get_switchhookstyle());
	return true;
}

static weapon* fire_shooter_wpn(newcombo const& cmb, zfix& wx, zfix& wy, bool angular, double radians, int32_t dir)
{
	byte weapid = cmb.attribytes[1];
	if(!weapid) return nullptr;
	byte weapspr = cmb.attribytes[2];
	int32_t damage = cmb.attrishorts[2];
	zfix steprate = zslongToFix(cmb.attributes[2]/100);
	if(damage < 0) damage = 0;
	bool lw = weapid < wEnemyWeapons;
	if(weapid >= wScript1 && weapid <= wScript10)
		lw = (cmb.usrflags&cflag5)!=0;
	bool autorot = (cmb.usrflags&cflag6);
	bool boss = cmb.usrflags&cflag8;
	
	auto wdir = autorot ? right : dir;
	weapon* wpn = nullptr;
	if(lw)
	{
		int pitem = cmb.attribytes[6]>0 ? cmb.attribytes[6] : -1;
		int plvl = pitem > -1 ? itemsbuf[pitem].fam_type : 0;
		
		wpn = new weapon((zfix)wx,(zfix)wy,(zfix)0,weapid,plvl,damage,wdir,pitem, Hero.getUID(),false,0,1,0,0,weapspr);
		Lwpns.add(wpn);
	}
	else
	{
		wpn = new weapon((zfix)wx,(zfix)wy,(zfix)0,weapid,0,damage,wdir, -1,-1,false,0,0,0,0,weapspr);
		Ewpns.add(wpn);
	}
	wpn->angular = angular;
	wpn->angle = radians;
	if(angular) dir = AngleToDir(radians);
	wpn->dir = dir;
	wpn->step = steprate;
	wpn->xofs = 0;
	wpn->yofs = (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
	if(autorot)
	{
		if(angular)
			wpn->rotation = RadiansToDegrees(wpn->angle);
		else wpn->rotation = DirToDegrees(wpn->dir);
		wpn->rotation = WrapDegrees(wpn->rotation);
		wpn->autorotate = true;
	}
	if(weapid == ewFireball || weapid == ewFireball2)
	{
		SETFLAG(wpn->type, 1, boss);
	}
	wpn->unblockable = cmb.attribytes[4] & WPNUNB_ALL;
	
	auto scrid = cmb.attribytes[5];
	bool valid_script = false;
	if(scrid > 0)
	{
		if(lw)
		{
			if(lwpnmap[scrid-1].hasScriptData()) valid_script = true;
		}
		else if(ewpnmap[scrid-1].hasScriptData()) valid_script = true;
	}
	if(valid_script)
	{
		wpn->weaponscript = scrid;
	}
	return wpn;
}
bool trigger_shooter(newcombo const& cmb, zfix wx, zfix wy)
{
	if(cmb.type != cSHOOTER) return false;
	if(!cmb.attribytes[1]) return false; //no weapon
	if(wx > 255 || wx < -15 || wy > 175 || wy < -15) return false;
	
	bool proxstop = cmb.usrflags&cflag4;
	bool invprox = cmb.usrflags&cflag9;
	zfix proxlim = zslongToFix(cmb.attributes[1]);
	auto dst = dist(wx,wy,Hero.getX(),Hero.getY());
	if(proxstop && (invprox ? (dst > proxlim) : (dst <= proxlim)))
		return false;
	
	bool angular = cmb.usrflags&cflag1;
	double radians = 0;
	int32_t dir = -1;
	if(angular)
	{
		int32_t zslDegrees = cmb.attributes[0];
		if(zslDegrees >= 0 && zslDegrees <= 3600000)
		{
			radians = WrapAngle(DegreesToRadians(zslDegrees/10000.0));
			dir = AngleToDir(radians);
		}
		else
		{
			double _MSVC2022_tmp1, _MSVC2022_tmp2;
			double at_player = atan2_MSVC2022_FIX(double(Hero.getY()-wy),double(Hero.getX()-wx));
			at_player = WrapAngle(at_player);
			switch(zslDegrees/10000)
			{
				case -1: //4-dir at player
				{
					angular = false;
					dir = AngleToDir4Rad(at_player);
					break;
				}
				case -2: //8-dir at player
				{
					angular = false;
					dir = AngleToDir(at_player);
					break;
				}
				case -3: //angular at player
				{
					radians = at_player;
					dir = AngleToDir(at_player);
					break;
				}
				default: dir = AngleToDir(0);
			}
		}
	}
	else dir = vbound(cmb.attributes[0]/10000, -1, 15);
	
	byte shotsfx = cmb.attribytes[0];
	auto shotcount = zc_max(1,cmb.attribytes[3]);
	if((cmb.usrflags&cflag7) && shotcount > 1) //multishot
	{
		if(!angular)
		{
			angular = true;
			radians = WrapAngle(DirToRadians(dir));
		}
		double spreadangle = WrapAngle(DegreesToRadians(cmb.attributes[3]/10000.0));
		double startangle = 0;
		if(shotcount&1) //odd
		{
			startangle = radians - (spreadangle * floor(shotcount/2));
		}
		else
		{
			startangle = radians - (spreadangle * ((shotcount/2.0)-0.5));
		}
		for(auto q = 0; q < shotcount; ++q)
		{
			double angle = WrapAngle(startangle + (q*spreadangle));
			fire_shooter_wpn(cmb, wx, wy, angular, angle, dir);
		}
	}
	else fire_shooter_wpn(cmb, wx, wy, angular, radians, dir);
	if(shotsfx)
		sfx(shotsfx);
	return true;
}
bool trigger_shooter(newcombo const& cmb, int32_t pos)
{
	if(unsigned(pos) > 175) return false;
	return trigger_shooter(cmb, COMBOX(pos), COMBOY(pos));
}
void trigger_save(newcombo const& cmb)
{
	if(cmb.type != cSAVE && cmb.type != cSAVE2) return;
	auto save_type = cmb.type == cSAVE2 ? 1 : 0;
	if(cmb.usrflags & cflag1) //restore hp
	{
		double perc = cmb.attribytes[0]/100.0;
		word life = word(perc*game->get_maxlife());
		if(cmb.attribytes[0]==100) //sanity incase of floating point error
			life = game->get_maxlife();
		if(game->get_life() < life)
			game->set_life(life);
	}
	if(cmb.usrflags & cflag2) //restore magic
	{
		double perc = cmb.attribytes[0]/100.0;
		word magic = word(perc*game->get_maxmagic());
		if(cmb.attribytes[0]==100) //sanity incase of floating point error
			magic = game->get_maxmagic();
		if(game->get_magic() < magic)
			game->set_magic(magic);
	}
	save_game((tmpscr->flags4&fSAVEROOM) != 0, save_type);
}


static byte copycat_id = 0;
bool do_copycat_trigger(int32_t lyr, int32_t pos)
{
	if(!copycat_id) return false;
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	int32_t cid = tmp->data[pos];
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo(lyr,pos);
		return true;
	}
	return false;
}

bool do_copycat_trigger_ffc(int32_t pos)
{
	if(!copycat_id) return false;
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	int32_t cid = ffc.data;
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo_ffc(pos);
		return true;
	}
	return false;
}

static int copycat_skip_lyr = -1, copycat_skip_pos = -1, copycat_skip_ffc = -1;
void trig_copycat(byte copyid)
{
	if(copycat_id)
		return;
	copycat_id = copyid;
	for(auto cclayer = 0; cclayer < 7; ++cclayer)
	{
		for(auto ccpos = 0; ccpos < 176; ++ccpos)
		{
			if(cclayer == copycat_skip_lyr && ccpos == copycat_skip_pos)
				continue;
			
			do_copycat_trigger(cclayer, ccpos);
		}
	}
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		word c = tmpscr->numFFC();
		for(word i=0; i<c; i++)
		{
			if(i == copycat_skip_ffc)
				continue;
			do_copycat_trigger_ffc(i);
		}
	}
	copycat_id = 0;
}

void trig_copycat(int cid, int lyr, int pos)
{
	if(copycat_id)
		return;
	if(FFCore.tempScreens[lyr]->data[pos] == cid) //skip self
	{
		copycat_skip_lyr = lyr;
		copycat_skip_pos = pos;
	}
	trig_copycat(combobuf[cid].trigcopycat);
	copycat_skip_lyr = -1;
	copycat_skip_pos = -1;
}
void trig_copycat(int cid, int ffpos)
{
	if(copycat_id)
		return;
	if(FFCore.tempScreens[0]->ffcs[ffpos].data == cid) //skip self
	{
		copycat_skip_ffc = ffpos;
	}
	trig_copycat(combobuf[cid].trigcopycat);
	copycat_skip_ffc = -1;
}

void do_ex_trigger(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return;
	mapscr* tmp = FFCore.tempScreens[lyr];
	int32_t cid = tmp->data[pos];
	int32_t ocs = tmp->cset[pos];
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		tmp->data[pos] = cid+cmb.trigchange;
	}
	if(cmb.trigcschange)
	{
		tmp->cset[pos] = (ocs+cmb.trigcschange) & 0xF;
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[tmp->data[pos]];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
		trig_copycat(cid, lyr, pos);
}

void do_ex_trigger_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return;
	ffcdata& ffc = tmpscr->ffcs[pos];
	int32_t cid = ffc.data;
	int32_t ocs = ffc.cset;
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		zc_ffc_set(ffc, cid+cmb.trigchange);
	}
	if(cmb.trigcschange)
	{
		ffc.cset = (ocs+cmb.trigcschange) & 0xF;
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[ffc.data];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
		trig_copycat(cid, pos);
}

bool force_ex_trigger(int32_t lyr, int32_t pos, char xstate)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(1<<cmb.exstate))
		{
			do_ex_trigger(lyr,pos);
			return true;
		}
	}
	return false;
}
bool force_ex_trigger_ffc(int32_t pos, char xstate)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(1<<cmb.exstate))
		{
			do_ex_trigger_ffc(pos);
			return true;
		}
	}
	return false;
}

bool force_ex_door_trigger(uint lyr, uint pos, int dir, uint ind)
{
	if(lyr > 6 || pos > 175 || dir > 3 || ind > 7) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	if(cmb.exdoor_dir > -1 && (dir < 0 || (dir == cmb.exdoor_dir && ind == cmb.exdoor_ind)))
	{
		if(dir >= 0 || getxdoor(cmb.exdoor_dir, cmb.exdoor_ind))
		{
			do_ex_trigger(lyr,pos);
			return true;
		}
	}
	return false;
}
bool force_ex_door_trigger_ffc(uint pos, int dir, uint ind)
{
	if(pos >= MAXFFCS || dir > 3 || ind > 7) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.data];
	if(cmb.exdoor_dir > -1 && (dir < 0 || (dir == cmb.exdoor_dir && ind == cmb.exdoor_ind)))
	{
		if(dir >= 0 || getxdoor(cmb.exdoor_dir, cmb.exdoor_ind))
		{
			do_ex_trigger_ffc(pos);
			return true;
		}
	}
	return false;
}

static bool triggering_generic_secrets = false;
static bool triggering_generic_switchstate = false;

void do_weapon_fx(weapon* w, newcombo const& cmb)
{
	if(!w) return;
	if(cmb.triggerflags[0] & combotriggerKILLWPN)
		killgenwpn(w);
	if(cmb.triggerflags[3] & combotriggerIGNITE_ANYFIRE)
		w->misc_wflags |= WFLAG_BURN_ANYFIRE;
	if(cmb.triggerflags[3] & combotriggerIGNITE_STRONGFIRE)
		w->misc_wflags |= WFLAG_BURN_STRONGFIRE;
	if(cmb.triggerflags[3] & combotriggerIGNITE_MAGICFIRE)
		w->misc_wflags |= WFLAG_BURN_MAGICFIRE;
	if(cmb.triggerflags[3] & combotriggerIGNITE_DIVINEFIRE)
		w->misc_wflags |= WFLAG_BURN_DIVINEFIRE;
}

bool handle_trigger_conditionals(newcombo const& cmb, int32_t cx, int32_t cy, bool& hasitem)
{
	if(cmb.triggeritem) //Item requirement
	{
		hasitem = game->get_item(cmb.triggeritem) && !item_disabled(cmb.triggeritem)
			&& checkbunny(cmb.triggeritem);
		if(cmb.triggerflags[1] & combotriggerINVERTITEM)
		{
			if(hasitem) return false;
		}
		else if(!hasitem) return false;
	}
	if(cmb.trigprox) //Proximity requirement
	{
		word d = word(dist(Hero.getX(), Hero.getY(), zfix(cx), zfix(cy)).getInt());
		if(cmb.triggerflags[0] & combotriggerINVERTPROX) //trigger outside the radius
		{
			if(d < cmb.trigprox) //inside, cancel
				return false;
		}
		else //trigger inside the radius
		{
			if(d >= cmb.trigprox) //outside, cancel
				return false;
		}
	}
	
	if(cmb.triggerflags[3] & combotriggerCOND_DARK)
		if(!get_lights())
			return false;
	if(cmb.triggerflags[3] & combotriggerCOND_NODARK)
		if(get_lights())
			return false;
	
	auto dmap_level = cmb.trigdmlevel > -1 ? cmb.trigdmlevel : dlevel;
	if(cmb.triggerflags[3] & combotriggerLITEM_COND)
		if((cmb.trig_levelitems & game->lvlitems[dmap_level]) != cmb.trig_levelitems)
			return false;
	if(cmb.triggerflags[3] & combotriggerLITEM_REVCOND)
		if((cmb.trig_levelitems & game->lvlitems[dmap_level]) == cmb.trig_levelitems)
			return false;
	
	if(!!(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG) && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
	{
		if(game->get_counter(cmb.trigctr) >= cmb.trigctramnt)
		{
			game->change_counter(-cmb.trigctramnt, cmb.trigctr);
		}
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERGE)
	{
		if(game->get_counter(cmb.trigctr) < cmb.trigctramnt)
			return false;
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERLT)
	{
		if(game->get_counter(cmb.trigctr) >= cmb.trigctramnt)
			return false;
	}
	return true;
}
void handle_trigger_results(newcombo const& cmb, int32_t cx, int32_t cy, bool& hasitem, bool& used_bit,
	int32_t special)
{
	if(cmb.triggerflags[3]&combotriggerTOGGLEDARK)
	{
		toggle_lights(pal_litOVERRIDE);
	}
	if (cmb.triggerflags[1]&combotriggerSECRETS)
	{
		used_bit = true;
		if(!(special & ctrigSECRETS) && !triggering_generic_secrets)
		{
			triggering_generic_secrets = true;
			hidden_entrance(0, true, false, -6);
			triggering_generic_secrets = false;
			if(tmpscr->secretsfx)
				sfx(tmpscr->secretsfx);
		}
		if(canPermSecret() && !(tmpscr->flags5&fTEMPSECRETS) && !getmapflag(mSECRET))
			setmapflag(mSECRET);
	}
	
	if (cmb.triggerflags[3] & combotriggerLEVELSTATE)
	{
		used_bit = true;
		if(!(special & ctrigSWITCHSTATE) && !triggering_generic_switchstate)
		{
			triggering_generic_switchstate = true;
			game->lvlswitches[dlevel] ^= 1<<cmb.trig_lstate;
			toggle_switches(1<<cmb.trig_lstate, false);
			triggering_generic_switchstate = false;
		}
	}
	if (cmb.triggerflags[3] & combotriggerGLOBALSTATE)
	{
		used_bit = true;
		if(!(special & ctrigSWITCHSTATE) && !triggering_generic_switchstate)
		{
			int tmr = cmb.trig_statetime, pair = cmb.trig_gstate;
			bool oldstate = game->gswitch_timers[pair]!=0;
			if(tmr > 0)
			{
				game->gswitch_timers[pair] = tmr;
			}
			else
			{
				if(game->gswitch_timers[pair])
					game->gswitch_timers[pair] = 0;
				else game->gswitch_timers[pair] = -1;
			}
			if(oldstate != (game->gswitch_timers[pair] != 0))
			{
				triggering_generic_switchstate = true;
				toggle_gswitches(pair, false);
				triggering_generic_switchstate = false;
			}
		}
	}
	
	if(cmb.trigsfx)
		sfx(cmb.trigsfx, pan(cx));
	
	if(cmb.triggerflags[3] & combotriggerKILLENEMIES)
		kill_em_all();
	if(cmb.triggerflags[3] & combotriggerCLEARENEMIES)
		guys.clear(true);
	if(cmb.triggerflags[3] & combotriggerCLEARLWEAPONS)
		Lwpns.clear(true);
	if(cmb.triggerflags[3] & combotriggerCLEAREWEAPONS)
		Ewpns.clear(true);
	
	if(cmb.triggeritem && hasitem && (cmb.triggerflags[1] & combotriggerCONSUMEITEM))
	{
		takeitem(cmb.triggeritem);
	}
	if(!(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG) && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
	{
		if(game->get_counter(cmb.trigctr) >= cmb.trigctramnt)
		{
			game->change_counter(-cmb.trigctramnt, cmb.trigctr);
		}
	}
	bool trigexstate = true;
	if(cmb.spawnenemy)
	{
		enemy* enm = nullptr;
		bool enm_ex = (cmb.triggerflags[2] & combotriggerEXSTENEMY);
		word numcreated = addenemy(cx, cy, cmb.spawnenemy, -10);
		if(numcreated)
		{
			word index = guys.Count() - numcreated;
			enm = (enemy*)guys.spr(index);
		}
		if(enm_ex)
		{
			trigexstate = false;
			if(enm)
			{
				enm->deathexstate = cmb.exstate;
			}
		}
	}
	if(cmb.spawnitem)
	{
		bool itm_ex = (cmb.triggerflags[2] & combotriggerEXSTITEM);
		bool specitem = (cmb.triggerflags[2] & combotriggerSPCITEM);
		if(specitem && getmapflag(mSPECIALITEM))
		{
			//already collected
			if(itm_ex) trigexstate = true;
		}
		else
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			int32_t pflags = cmb.spawnip & allowed_pflags;
			SETFLAG(pflags, ipONETIME2, specitem);
			int32_t item_id = cmb.spawnitem;
			if(item_id < 0)
			{
				item_id = select_dropitem(-item_id);
			}
			item* itm = nullptr;
			if(unsigned(item_id) < MAXITEMS)
			{
				itm = new item(cx, cy, 0, item_id, pflags, 0);
				items.add(itm);
			}
			if(itm_ex)
			{
				trigexstate = false;
				if(itm) itm->pickupexstate = cmb.exstate;
			}
			if(cmb.triggerflags[2] & combotriggerAUTOGRABITEM)
			{
				if(itm) itm->set_forcegrab(true);
			}
		}
	}
	
	//Level Item stuff
	{
		auto dmap_level = cmb.trigdmlevel > -1 ? cmb.trigdmlevel : dlevel;
		bool _set = cmb.triggerflags[3] & combotriggerLITEM_SET,
			_unset = cmb.triggerflags[3] & combotriggerLITEM_UNSET;
		if(_set && _unset) //toggle
			game->lvlitems[dmap_level] ^= cmb.trig_levelitems;
		else if(_set)
			game->lvlitems[dmap_level] |= cmb.trig_levelitems;
		else if(_unset)
			game->lvlitems[dmap_level] &= ~cmb.trig_levelitems;
	}
	
	//Graphical stuff
	{
		if(cmb.triggerflags[3] & combotriggerTINT_CLEAR)
			doClearTint();
		if(cmb.trigtint[0] || cmb.trigtint[1] || cmb.trigtint[2])
			doTint(cmb.trigtint[0], cmb.trigtint[1], cmb.trigtint[2]);
		if(cmb.triglvlpalette > -1)
			loadlvlpal(cmb.triglvlpalette);
		if(cmb.trigbosspalette > -1)
			loadpalset(csBOSS,pSprite(cmb.trigbosspalette));
		if(cmb.trigquaketime > -1)
			quakeclk = cmb.trigquaketime;
		if(cmb.trigwavytime > -1)
			wavy = cmb.trigwavytime;
	}
	
	//Status Effects
	{
		if(cmb.trig_swjinxtime > -2)
			Hero.setSwordClk(cmb.trig_swjinxtime);
		if(cmb.trig_itmjinxtime > -2)
			Hero.setItemClk(cmb.trig_itmjinxtime);
		if(cmb.trig_shieldjinxtime > -2)
			Hero.setShieldClk(cmb.trig_shieldjinxtime);
		if(cmb.trig_stuntime > -2)
			Hero.setStunClock(cmb.trig_stuntime);
		if(cmb.trig_bunnytime > -2)
			Hero.setBunnyClock(cmb.trig_bunnytime);
	}
	
	if(cmb.exstate > -1 && trigexstate)
	{
		setxmapflag(1<<cmb.exstate);
	}
	if(cmb.exdoor_dir > -1)
	{
		set_xdoorstate(cmb.exdoor_dir, cmb.exdoor_ind);
	}
}

//Triggers a combo at a given position
bool do_trigger_combo(int32_t lyr, int32_t pos, int32_t special, weapon* w)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	cpos_info& timer = cpos_get(lyr, pos);
	int32_t cid = tmp->data[pos];
	int32_t ocs = tmp->cset[pos];
	int32_t cx = COMBOX(pos);
	int32_t cy = COMBOY(pos);
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	if(w && (cmb.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,cmb);
		return true;
	}
	
	int32_t flag = tmp->sflag[pos];
	int32_t flag2 = cmb.flag;
	
	byte* grid = nullptr;
	bool check_bit = false;
	bool used_bit = false;
	
	if(cmb.exstate > -1)
	{
		if(force_ex_trigger(lyr,pos))
			return true;
	}
	if(cmb.exdoor_dir > -1)
	{
		if(force_ex_door_trigger(lyr,pos))
			return true;
	}
	if(!handle_trigger_conditionals(cmb, cx, cy, hasitem))
		return false;
	
	if(w)
	{
		grid = (lyr ? w->wscreengrid_layer[lyr-1] : w->wscreengrid);
		check_bit = get_bit(grid,(((cx>>4) + cy)));
	}
	bool dorun = !timer.trig_cd;
	if(dorun)
	{
		if((cmb.triggerflags[0] & combotriggerCMBTYPEFX) || alwaysCTypeEffects(cmb.type))
		{
			switch(cmb.type)
			{
				case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
				case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
				case cTRIGGERGENERIC:
					if(w)
						do_generic_combo(w, cx, cy, (w->useweapon > 0) ? w->useweapon : w->id, cid, flag, flag2, cmb.attribytes[3], pos, false, lyr);
					else do_generic_combo2(cx, cy, cid, flag, flag2, cmb.attribytes[3], pos, false, lyr);
					break;
				case cCUSTOMBLOCK:
					if(!w) break;
					killgenwpn(w);
					if(cmb.attribytes[0])
						sfx(cmb.attribytes[0]);
					break;
			}
			if(!check_bit)
			{
				used_bit = true;
				switch(cmb.type)
				{
					case cCUTSCENETRIG:
						do_cutscene_flags(cmb);
						break;
					case cCSWITCH:
						do_cswitch_combo(cmb, w);
						break;
					
					case cCSWITCHBLOCK:
						trigger_cswitch_block(lyr,pos);
						break;
					
					case cSIGNPOST:
					{
						if(!(special & ctrigIGNORE_SIGN))
						{
							trigger_sign(cmb);
						}
						break;
					}
					
					case cSLASH: case cSLASHITEM: case cBUSH: case cFLOWERS: case cTALLGRASS:
					case cTALLGRASSNEXT:case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
					case cSLASHTOUCHY: case cSLASHITEMTOUCHY: case cBUSHTOUCHY: case cFLOWERSTOUCHY:
					case cTALLGRASSTOUCHY: case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY:
					case cBUSHNEXTTOUCHY:
						trigger_cuttable(lyr, pos);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step(lyr,pos))
							return false;
						break;
					
					case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
					case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
					case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
					case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
					case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
					case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
						trigger_warp(cmb);
						break;
					
					case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
						if(!trigger_chest(lyr,pos))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock(lyr,pos))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave(lyr,pos))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(cid, ZSD_COMBOPOS, pos*10000);
						break;
					
					case cSTEPSFX:
						trigger_stepfx(lyr,pos);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock(pos))
							return false;
						break;
					
					case cSHOOTER:
						if(!trigger_shooter(cmb,pos))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb);
						break;
					default:
						used_bit = false;
				}
			}
		}
		
		if(!check_bit)
		{
			handle_trigger_results(cmb, cx, cy, hasitem, used_bit, special);
			
			if(cmb.trigchange)
			{
				used_bit = true;
				tmp->data[pos] = cid+cmb.trigchange;
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				tmp->cset[pos] = (ocs+cmb.trigcschange) & 0xF;
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[tmp->data[pos]];
				rcmb.tile = rcmb.o_tile;
				rcmb.cur_frame=0;
				rcmb.aclk = 0;
			}
			
			if(cmb.trigcopycat) //has a copycat set
				trig_copycat(cid, lyr, pos);
			
			timer.updateData(tmp->data[pos]);
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if(used_bit && grid)
		{
			set_bit(grid,(((cx>>4) + cy)),1);
		}
	}
	
	if(w)
		do_weapon_fx(w,cmb);
	
	if(dorun && cmb.trig_genscr)
		FFCore.runGenericFrozenEngine(cmb.trig_genscr);
	return true;
}

bool do_trigger_combo_ffc(int32_t pos, int32_t special, weapon* w)
{
	if (get_qr(qr_OLD_FFC_FUNCTIONALITY)) return false;
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr->ffcs[pos];
	if (ffc.flags & ffc_changer)
		return false; //Changers can't trigger!
	cpos_info& timer = tmpscr->ffcs[pos].info;
	int32_t cid = ffc.data;
	int32_t ocs = ffc.cset;
	int32_t cx = ffc.x;
	int32_t cy = ffc.y;
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	if(w && (cmb.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,cmb);
		return true;
	}
	
	int32_t flag2 = cmb.flag;
	
	byte* grid = nullptr;
	bool check_bit = false;
	bool used_bit = false;
	
	if(cmb.exstate > -1)
	{
		if(force_ex_trigger_ffc(pos))
			return true;
	}
	if(cmb.exdoor_dir > -1)
	{
		if(force_ex_door_trigger_ffc(pos))
			return true;
	}
	if(!handle_trigger_conditionals(cmb, cx, cy, hasitem))
		return false;
	
	if(w)
	{
		grid = w->wscreengrid_ffc;
		check_bit = get_bit(grid,pos);
	}
	bool dorun = !timer.trig_cd;
	if(dorun)
	{
		if((cmb.triggerflags[0] & combotriggerCMBTYPEFX) || alwaysCTypeEffects(cmb.type))
		{
			switch(cmb.type)
			{
				case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
				case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
				case cTRIGGERGENERIC:
					if(w)
						do_generic_combo_ffc(w, pos, cid, flag2);
					else do_generic_combo_ffc2(pos, cid, flag2);
					break;
				case cCUSTOMBLOCK:
					if(!w) break;
					killgenwpn(w);
					if(cmb.attribytes[0])
						sfx(cmb.attribytes[0]);
					break;
			}
			if(!check_bit)
			{
				used_bit = true;
				switch(cmb.type)
				{
					case cCUTSCENETRIG:
						do_cutscene_flags(cmb);
						break;
					case cCSWITCH:
						do_cswitch_combo(cmb, w);
						break;
					
					case cCSWITCHBLOCK:
						trigger_cswitch_block_ffc(pos);
						break;
					
					case cSIGNPOST:
					{
						if(!(special & ctrigIGNORE_SIGN))
						{
							trigger_sign(cmb);
						}
						break;
					}
					
					case cSLASH: case cSLASHITEM: case cBUSH: case cFLOWERS: case cTALLGRASS:
					case cTALLGRASSNEXT:case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
					case cSLASHTOUCHY: case cSLASHITEMTOUCHY: case cBUSHTOUCHY: case cFLOWERSTOUCHY:
					case cTALLGRASSTOUCHY: case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY:
					case cBUSHNEXTTOUCHY:
						trigger_cuttable_ffc(pos);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step_ffc(pos))
							return false;
						break;
					
					case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
					case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
					case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
					case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
					case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
					case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
						trigger_warp(cmb);
						break;
					
					case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
						if(!trigger_chest_ffc(pos))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock_ffc(pos))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave_ffc(pos))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(cid, ZSD_FFC, pos);
						break;
					
					case cSTEPSFX:
						trigger_stepfx_ffc(pos);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock_ffc(pos))
							return false;
						break;
					
					case cSHOOTER:
						if(!trigger_shooter(cmb,cx,cy))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb);
						break;
					default:
						used_bit = false;
				}
			}
		}
		
		if(!check_bit)
		{
			handle_trigger_results(cmb, cx, cy, hasitem, used_bit, special);
			
			if(cmb.trigchange)
			{
				used_bit = true;
				zc_ffc_set(ffc, cid+cmb.trigchange);
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				ffc.cset = (ocs+cmb.trigcschange) & 0xF;
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[ffc.data];
				rcmb.tile = rcmb.o_tile;
				rcmb.cur_frame=0;
				rcmb.aclk = 0;
			}
			
			if(cmb.trigcopycat) //has a copycat set
				trig_copycat(cid, pos);
			
			if(tmpscr->ffcs[pos].flags & ffc_changer)
				timer.updateData(-1);
			else timer.updateData(tmpscr->ffcs[pos].data);
			
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if(used_bit && grid)
		{
			set_bit(grid,pos,1);
		}
	}
	
	if(w)
		do_weapon_fx(w,cmb);
	
	if(dorun && cmb.trig_genscr)
		FFCore.runGenericFrozenEngine(cmb.trig_genscr);
	return true;
}


bool do_lift_combo(int32_t lyr, int32_t pos, int32_t gloveid)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	if(!Hero.can_lift(gloveid)) return false;
	if(Hero.lift_wpn) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	int32_t cid = tmp->data[pos];
	int32_t cset = tmp->cset[pos];
	int32_t cx = COMBOX(pos);
	int32_t cy = COMBOY(pos);
	newcombo const& cmb = combobuf[cid];
	itemdata const& glove = itemsbuf[gloveid];
	if(cmb.liftlvl > glove.fam_type) return false;
	//Able to lift, run effects
	if(cmb.liftsfx) sfx(cmb.liftsfx,pan(cx));
	else if(glove.usesound) sfx(glove.usesound,pan(cx));
	
	int32_t dropitem = -1, dropset = -1;
	bool hasitem = cmb.liftitm>0;
	auto pflags = ipBIGRANGE | ((cmb.liftflags&LF_SPECIALITEM) ? ipONETIME2 : ipTIMER);
	if(hasitem)
	{
		if(cmb.liftflags & LF_DROPSET)
			dropset = cmb.liftitm;
		else dropitem = cmb.liftitm;
	}
	
	if(hasitem && (cmb.liftflags&LF_SPECIALITEM) && getmapflag(mSPECIALITEM))
	{
		hasitem = false;
		dropitem = dropset = -1;
	}
	
	if(hasitem && (cmb.liftflags & LF_DROPONLIFT))
	{
		if(dropset > -1) dropitem = select_dropitem(dropset);
		if(dropitem > -1)
		{
			item* itm = (new item((zfix)cx, (zfix)cy,(zfix)0, dropitem, pflags, 0));
			itm->from_dropset = dropset;
			items.add(itm);
		}
	}
	
	weapon* w = nullptr;
	byte prntid = cmb.lift_parent_item;
	int wlvl = 0, wtype = wThrown;
	if(prntid)
	{
		itemdata const& prntitm = itemsbuf[prntid];
		switch(prntitm.family)
		{
			case itype_bomb:
				wtype = wLitBomb;
				wlvl = prntitm.fam_type;
				break;
			case itype_sbomb:
				wtype = wLitSBomb;
				wlvl = prntitm.fam_type;
				break;
			default:
				prntid = gloveid;
				break;
		}
	}
	else prntid = gloveid;
	w = new weapon(cx, cy, 0, wtype, wlvl, cmb.liftdmg*game->get_hero_dmgmult(),
		oppositeDir[NORMAL_DIR(HeroDir())], prntid, Hero.getUID(), false, 0, 1);
	if(hasitem && !(cmb.liftflags & LF_DROPONLIFT))
	{
		w->death_spawnitem = dropitem;
		w->death_spawndropset = dropset;
		w->death_item_pflags = pflags;
	}
	switch(cmb.liftgfx)
	{
		case 0: //This combo
		{
			w->LOADGFX_CMB(cid,cset);
			break;
		}
		case 1: //Other combo
		{
			auto lcs = (cmb.liftflags & LF_NOWPNCMBCSET) ? cset : cmb.liftcs;
			w->LOADGFX_CMB(cmb.liftcmb,lcs);
			break;
		}
		case 2: //Sprite Data
		{
			w->LOADGFX(cmb.liftsprite);
			break;
		}
	}
	
	w->moveflags |= move_obeys_grav;
	if(cmb.liftflags & LF_BREAKONSOLID)
		w->misc_wflags |= WFLAG_BREAK_ON_SOLID;
	
	w->death_sprite = cmb.liftbreaksprite;
	w->death_sfx = cmb.liftbreaksfx;
	
	Hero.lift(w, cmb.lifttime, cmb.lifthei);
	
	tmp->data[pos] = cmb.liftundercmb;
	if(!(cmb.liftflags & LF_NOUCSET))
		tmp->cset[pos] = cmb.liftundercs;
	tmp->sflag[pos] = 0;
	return true;
}

static void handle_shooter(newcombo const& cmb, cpos_info& timer, zfix wx, zfix wy)
{
	int32_t lowrate = zc_max(0,cmb.attrishorts[0]);
	int32_t highrate = zc_max(0,cmb.attrishorts[1]);
	bool splitrate = cmb.usrflags&cflag2;
	bool instashot = cmb.usrflags&cflag3;
	if(splitrate)
	{
		if(lowrate > highrate) return;
		if(lowrate == highrate) splitrate = false;
	}
	if(!splitrate)
	{
		if(!lowrate) return;
	}
	
	if(timer.type_clk > 1)
	{
		if(--timer.type_clk == 1)
		{
			if(!instashot) trigger_shooter(cmb, wx, wy);
		}
	}
	else
	{
		auto rate = (splitrate ? zc_rand(highrate,lowrate) : lowrate);
		timer.type_clk = zc_max(1,rate);
		if(instashot || timer.type_clk == 1) trigger_shooter(cmb, wx, wy);
	}
}

static void trigger_crumble(newcombo const& cmb, cpos_info& timer, word& cid)
{
	++cid;
	timer.updateData(cid);
	//Continue crumbling through the change
	newcombo const& ncmb = combobuf[cid];
	if(ncmb.type == cCRUMBLE && (cmb.usrflags&cflag1)
		&& (ncmb.attribytes[0] == CMBTY_CRUMBLE_INEVITABLE))
		timer.flags.set(CPOS_CRUMBLE_BREAKING,true);
}

static bool handle_crumble(newcombo const& cmb, cpos_info& timer, word& cid, zfix x, zfix y, zfix w, zfix h)
{
	bool breaking = false;
	byte ty = cmb.attribytes[0];
	if(ty == CMBTY_CRUMBLE_INEVITABLE)
		if(timer.flags.get(CPOS_CRUMBLE_BREAKING))
			breaking = true;
	if(!breaking)
	{
		byte sens_offset = cmb.attribytes[1];
		if(Hero.sideview_mode())
		{
			if((Hero.getY()+16-y).getAbs() < 0.5_zf
				&& (!sens_offset || sens_offset*2 < w))
			{
				if(sens_offset)
				{
					x += sens_offset;
					w -= sens_offset*2;
				}
				breaking = Hero.collide(x,0,w,255);
			}
		}
		else if(!sens_offset || (sens_offset*2 < w && sens_offset*2 < h))
		{
			if(sens_offset)
			{
				x += sens_offset;
				y += sens_offset;
				w -= sens_offset*2;
				h -= sens_offset*2;
			}
			breaking = Hero.collide(x,y,w,h);
		}
	}
	if(breaking)
	{
		// Crumble
		if(cmb.attrishorts[0] < 1)
		{
			trigger_crumble(cmb, timer, cid);
			return true;
		}
		if(timer.type_clk)
		{
			if(!--timer.type_clk)
			{
				trigger_crumble(cmb, timer, cid);
				return true;
			}
		}
		else timer.type_clk = cmb.attrishorts[0];
	}
	else if(ty == CMBTY_CRUMBLE_RESET && timer.flags.get(CPOS_CRUMBLE_BREAKING))
	{
		timer.type_clk = 0;
		if(int16_t diff = cmb.attrishorts[1])
		{
			cid += diff;
			timer.updateData(cid);
		}
	}
	timer.flags.set(CPOS_CRUMBLE_BREAKING, breaking);
	return false;
}

void trig_trigger_groups()
{
	mapscr* ffscr = FFCore.tempScreens[0];
	dword c = ffscr->numFFC();
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = FFCore.tempScreens[lyr];
		for(auto pos = 0; pos < 176; ++pos)
		{
			cpos_info& timer = cpos_get(lyr, pos);
			int cid = scr->data[pos];
			newcombo const& cmb = combobuf[cid];
			
			if(
				((cmb.triggerflags[3] & combotriggerTGROUP_LESS)
					&& cpos_trig_group_count(cmb.trig_group) < cmb.trig_group_val)
				|| ((cmb.triggerflags[3] & combotriggerTGROUP_GREATER)
					&& cpos_trig_group_count(cmb.trig_group) > cmb.trig_group_val)
				)
			{
				do_trigger_combo(lyr,pos);
				int cid2 = scr->data[pos];
				bool recheck = timer.data != cid2;
				timer.updateData(cid2);

				if (recheck) //check same pos again
				{
					--pos;
					continue;
				}
			}
		}
	}
	for(word ffc = 0; ffc < c; ++ffc)
	{
		ffcdata& f = ffscr->ffcs[ffc];
		if (f.flags & ffc_changer)
			continue; //changers don't contribute
		cpos_info& timer = f.info;
		int cid = f.data;
		newcombo const& cmb = combobuf[cid];
		
		if(
			((cmb.triggerflags[3] & combotriggerTGROUP_LESS)
				&& cpos_trig_group_count(cmb.trig_group) < cmb.trig_group_val)
			|| ((cmb.triggerflags[3] & combotriggerTGROUP_GREATER)
				&& cpos_trig_group_count(cmb.trig_group) > cmb.trig_group_val)
			)
		{
			do_trigger_combo_ffc(ffc);
			int cid2 = f.data;
			bool recheck = timer.data != cid2;
			
			if(f.flags & ffc_changer)
				timer.updateData(-1);
			else timer.updateData(cid2);
			
			if(recheck) //check same pos again
			{
				--ffc;
				continue;
			}
		}
	}
}

//COMBOTYPE POS STUFF

#define CXY(pos) COMBOX(pos), COMBOY(pos)
void handle_cpos_type(newcombo const& cmb, cpos_info& timer, int lyr, int pos)
{
	switch(cmb.type)
	{
		case cSHOOTER:
			handle_shooter(cmb, timer, CXY(pos));
			break;
		case cCRUMBLE:
		{
			word& cid = FFCore.tempScreens[lyr]->data[pos];
			handle_crumble(cmb, timer, cid, CXY(pos), 16, 16);
			break;
		}
	}
}
void handle_ffcpos_type(newcombo const& cmb, cpos_info& timer, ffcdata& f)
{
	switch(cmb.type)
	{
		case cSHOOTER:
		{
			zfix wx = f.x + (f.txsz-1)*8;
			zfix wy = f.y + (f.tysz-1)*8;
			handle_shooter(cmb, timer, wx, wy);
			break;
		}
		case cCRUMBLE:
		{
			word cid = f.data;
			handle_crumble(cmb, timer, cid, f.x+f.hxofs, f.y+f.hyofs, f.hit_width, f.hit_height);
			if(f.flags & ffc_changer)
				timer.updateData(-1);
			zc_ffc_update(f,cid);
			break;
		}
	}
}

//CPOS STUFF
static int cpos_spotlight_count = 0;
static int trig_groups[256];
static cpos_info combo_posinfos[7][176];

cpos_info& cpos_get(int32_t layer, int32_t pos)
{
	return combo_posinfos[layer][pos];
}
int cpos_trig_group_count(int ind)
{
	return unsigned(ind)<256 ? trig_groups[ind] : 0;
}
int cpos_exists_spotlight()
{
	return cpos_spotlight_count;
}

static void cpos_reset_cache()
{
	cpos_spotlight_count = 0;
	memset(trig_groups, 0, sizeof(trig_groups));
}
static void cpos_update_cache(newcombo const& cmb, int add)
{
	if(cmb.type == cSPOTLIGHT)
		cpos_spotlight_count += add;
	if(cmb.triggerflags[3] & combotriggerTGROUP_CONTRIB)
		trig_groups[cmb.trig_group] += add;
}
void cpos_update_cache(int32_t oldid, int32_t newid)
{
	if(oldid == newid) return;
	if(unsigned(oldid) < MAXCOMBOS)
		cpos_update_cache(combobuf[oldid],-1);
	if(unsigned(newid) < MAXCOMBOS)
		cpos_update_cache(combobuf[newid],1);
}

void cpos_clear_all()
{
	//Clearing these here just as a sanity check... -Em
	copycat_skip_lyr = -1;
	copycat_skip_pos = -1;
	copycat_skip_ffc = -1;
	//
	
	for(auto lyr = 0; lyr < 7; ++lyr)
		for(auto pos = 0; pos < 176; ++pos)
			combo_posinfos[lyr][pos].clear();
	
	int c = tmpscr->numFFC();
	for (int q = 0; q < c; ++q )
		tmpscr->ffcs[q].info.clear();
	
	cpos_reset_cache();
}
void cpos_force_update() //updates without side-effects
{
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = FFCore.tempScreens[lyr];
		for(auto pos = 0; pos < 176; ++pos)
			cpos_get(lyr, pos).updateData(scr->data[pos]);
	}
	dword c = tmpscr->numFFC();
	for(word ffc = 0; ffc < c; ++ffc)
	{
		ffcdata& f = tmpscr->ffcs[ffc];
		if(f.flags & ffc_changer)
			f.info.updateData(-1);
		else f.info.updateData(f.data);
	}
}
void cpos_update() //updates with side-effects
{
	mapscr* ffscr = FFCore.tempScreens[0];
	dword c = ffscr->numFFC();
	
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = FFCore.tempScreens[lyr];
		for(auto pos = 0; pos < 176; ++pos)
		{
			cpos_info& timer = cpos_get(lyr, pos);
			int cid = scr->data[pos];
			timer.updateData(cid);
			
			newcombo const& cmb = combobuf[cid];
			if(!timer.flags.get(CPOS_FL_APPEARED))
			{
				timer.flags.set(CPOS_FL_APPEARED,true);
				if(cmb.sfx_appear)
					sfx(cmb.sfx_appear);
				if(cmb.spr_appear)
					decorations.add(new comboSprite(COMBOX(pos), COMBOY(pos), dCOMBOSPRITE, 0, cmb.spr_appear));
				if(timer.sfx_onchange) //last combo's sfx_disappear
					sfx(timer.sfx_onchange);
				if(timer.spr_onchange) //last combo's spr_disappear
					decorations.add(new comboSprite(COMBOX(pos), COMBOY(pos), dCOMBOSPRITE, 0, timer.spr_onchange));
				timer.sfx_onchange = 0;
				timer.spr_onchange = 0;
			}
			if(cmb.sfx_loop)
				sfx_no_repeat(cmb.sfx_loop);
			
			if(cmb.trigtimer)
			{
				if(++timer.clk >= cmb.trigtimer)
				{
					timer.clk = 0;
					do_trigger_combo(lyr, pos);
					cid = scr->data[pos];
					timer.updateData(cid);
				}
			}
			if(timer.trig_cd) --timer.trig_cd;
			handle_cpos_type(cmb,timer,lyr,pos);
		}
	}
	for(word ffc = 0; ffc < c; ++ffc)
	{
		ffcdata& f = ffscr->ffcs[ffc];
		cpos_info& timer = f.info;
		if (f.flags & ffc_changer)
		{
			//changers don't contribute
			timer.updateData(-1);
			continue;
		}
		int cid = f.data;
		timer.updateData(cid);
		zfix wx = f.x + (f.txsz-1)*8;
		zfix wy = f.y + (f.tysz-1)*8;
		
		newcombo const& cmb = combobuf[cid];
		if(!timer.flags.get(CPOS_FL_APPEARED))
		{
			timer.flags.set(CPOS_FL_APPEARED,true);
			if(cmb.sfx_appear)
				sfx(cmb.sfx_appear);
			if(cmb.spr_appear)
				decorations.add(new comboSprite(wx, wy, dCOMBOSPRITE, 0, cmb.spr_appear));
			if(timer.sfx_onchange) //last combo's sfx_disappear
				sfx(timer.sfx_onchange);
			if(timer.spr_onchange) //last combo's spr_disappear
				decorations.add(new comboSprite(wx, wy, dCOMBOSPRITE, 0, timer.spr_onchange));
			timer.sfx_onchange = 0;
			timer.spr_onchange = 0;
		}
		if(cmb.sfx_loop)
			sfx_no_repeat(cmb.sfx_loop);
		
		if(cmb.trigtimer)
		{
			if(++timer.clk >= cmb.trigtimer)
			{
				timer.clk = 0;
				do_trigger_combo_ffc(ffc);
				cid = f.data;
				timer.updateData(cid);
			}
		}
		if(timer.trig_cd) --timer.trig_cd;
		if(cmb.type == cSHOOTER)
			handle_shooter(cmb, timer, wx, wy);
	}
	
	//Handle trigger groups
	trig_trigger_groups();
}

