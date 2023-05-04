#include "zelda.h"
#include "sprite.h"
#include "decorations.h"
#include "combos.h"
#include "maps.h"
#include "items.h"
#include "guys.h"
#include "ffscript.h"
#include "hero.h"
#include "title.h"

extern sprite_list items, decorations;
extern FFScript FFCore;
extern HeroClass Hero;
extern refInfo *ri;
extern refInfo itemScriptData[256];
extern word item_doscript[256];
extern int32_t item_stack[256][MAX_SCRIPT_REGISTERS];
extern byte itemscriptInitialised[256];

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
	if(errsfx && !sfx_allocated(errsfx))
		sfx(errsfx);
}
CutsceneState active_cutscene;

cpos_info combo_posinfos[7][176];
std::vector<cpos_info> ffc_posinfos;

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
			else decorations.add(new comboSprite((zfix)COMBOX(scombo), (zfix)COMBOY(scombo), 0, 0, combobuf[cid].attribytes[0]));
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
				tmpscr.catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[tmpscr.catchall].family==itype_triforcepiece ||
				(tmpscr.flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((tmpscr.flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_combo_modify_preroutine(&tmpscr,scombo);
			tmpscr.data[scombo] = tmpscr.secretcombo[ft];
			tmpscr.cset[scombo] = tmpscr.secretcset[ft];
			tmpscr.sflag[scombo] = tmpscr.secretflag[ft];
			// newflag = s->secretflag[ft];
			screen_combo_modify_postroutine(&tmpscr,scombo);
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
					
					screen_combo_modify_preroutine(FFCore.tempScreens[layer],scombo);
					
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						FFCore.tempScreens[layer]->data[scombo] = tmpscr.undercombo;
						FFCore.tempScreens[layer]->cset[scombo] = tmpscr.undercset;
						FFCore.tempScreens[layer]->sflag[scombo] = 0;	
					}
					else
						++FFCore.tempScreens[layer]->data[scombo];
					
					screen_combo_modify_postroutine(FFCore.tempScreens[layer],scombo);
				}
				else
				{
					screen_combo_modify_preroutine(&tmpscr,scombo);
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						tmpscr.data[scombo] = tmpscr.undercombo;
						tmpscr.cset[scombo] = tmpscr.undercset;
						tmpscr.sflag[scombo] = 0;	
					}
					else
					{
						tmpscr.data[scombo]=vbound(tmpscr.data[scombo]+1,0,MAXCOMBOS);
						//++tmpscr.data[scombo];
					}
					screen_combo_modify_postroutine(&tmpscr,scombo);
				}
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ( layer ) ? MAPCOMBO2(layer,bx,by) : MAPCOMBO(bx,by);
				//if ( combobuf[cid].usrflags&cflag8 ) w->dead = 1;
				//tmpscr.sflag[scombo] = combobuf[cid].sflag;
				//combobuf[tmpscr.data[cid]].cset;
				//combobuf[tmpscr.data[cid]].cset;
				
				//tmpscr.cset[scombo] = combobuf[cid].cset;
				//tmpscr.sflag[scombo] = combobuf[cid].sflag;
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
	ffcdata& ffc = tmpscr.ffcs[pos];
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
			else decorations.add(new comboSprite(ffc.x, ffc.y, 0, 0, combobuf[cid].attribytes[0]));
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
				tmpscr.catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[tmpscr.catchall].family==itype_triforcepiece ||
				(tmpscr.flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((tmpscr.flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_ffc_modify_preroutine(pos);
			ffc.setData(tmpscr.secretcombo[ft]);
			ffc.cset = tmpscr.secretcset[ft];
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
					ffc.setData(tmpscr.undercombo);
					ffc.cset = tmpscr.undercset;
				}
				else
				{
					ffc.setData(vbound(ffc.getData()+1,0,MAXCOMBOS));
				}
				screen_ffc_modify_postroutine(pos);
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ffc.getData();
				
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

static void trigger_cswitch_block(const pos_handle_t& pos_handle)
{
	if(unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return;

	int pos = RPOS_TO_POS(pos_handle.rpos);
	int cid = pos_handle.screen->data[pos];
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	pos_handle.screen->data[pos] = BOUND_COMBO(cid + cmbofs);
	pos_handle.screen->cset[pos] = (pos_handle.screen->cset[pos] + csofs) & 15;
	auto newcid = pos_handle.screen->data[pos];
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
	}
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		if(lyr == pos_handle.layer) continue;
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
		word c = tmpscr.numFFC();
		for(word i=0; i<c; i++)
		{
			if(ffcIsAt(i, COMBOX(pos)+8, COMBOY(pos)+8))
			{
				ffcdata& ffc2 = tmpscr.ffcs[i];
				newcombo const& cmb_2 = combobuf[ffc2.getData()];
				ffc2.setData(BOUND_COMBO(ffc2.getData() + cmbofs));
				ffc2.cset = (ffc2.cset + csofs) & 15;
				int32_t newcid2 = ffc2.getData();
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
	ffcdata& ffc = tmpscr.ffcs[pos];
	auto cid = ffc.getData();
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	ffc.setData(BOUND_COMBO(cid + cmbofs));
	ffc.cset = (ffc.cset + csofs) & 15;
	auto newcid = ffc.getData();
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
		word c = tmpscr.numFFC();
		for(word i=0; i<c; i++)
		{
			if (i == pos) continue;
			if(ffcIsAt(i, ffc.x+8, ffc.y+8))
			{
				ffcdata& ffc2 = tmpscr.ffcs[i];
				newcombo const& cmb_2 = combobuf[ffc2.getData()];
				ffc2.setData(BOUND_COMBO(ffc2.getData() + cmbofs));
				ffc2.cset = (ffc2.cset + csofs) & 15;
				int32_t newcid2 = ffc2.getData();
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


void spawn_decoration(newcombo const& cmb, int32_t pos)
{
	if(unsigned(pos) > 175) return;
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
			decorations.add(new comboSprite(COMBOX(pos), COMBOY(pos), 0, 0, cmb.attribytes[0]));
			break;
		case 1: decorations.add(new dBushLeaves(COMBOX(pos), COMBOY(pos), dBUSHLEAVES, 0, 0)); break;
		case 2: decorations.add(new dFlowerClippings(COMBOX(pos), COMBOY(pos), dFLOWERCLIPPINGS, 0, 0)); break;
		case 3: decorations.add(new dGrassClippings(COMBOX(pos), COMBOY(pos), dGRASSCLIPPINGS, 0, 0)); break;
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
			decorations.add(new comboSprite(x, y, 0, 0, cmb.attribytes[0]));
			break;
		case 1: decorations.add(new dBushLeaves(x, y, dBUSHLEAVES, 0, 0)); break;
		case 2: decorations.add(new dFlowerClippings(x, y, dFLOWERCLIPPINGS, 0, 0)); break;
		case 3: decorations.add(new dGrassClippings(x, y, dGRASSCLIPPINGS, 0, 0)); break;
	}
}

void trigger_cuttable(const pos_handle_t& pos_handle)
{
	if(unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return;

	int pos = RPOS_TO_POS(pos_handle.rpos);
	mapscr* tmp = pos_handle.screen;
	newcombo const& cmb = combobuf[tmp->data[pos]];
	auto type = cmb.type;
	if(!isCuttableType(type)) return;
	auto flag = tmp->sflag[pos];
	auto flag2 = cmb.flag;

	int32_t x, y;
	COMBOXY_REGION(pos_handle.rpos, x, y);
	
	bool skipSecrets = isNextType(type) && !get_bit(quest_rules,qr_OLD_SLASHNEXT_SECRETS);
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
			sfx(tmpscr.secretsfx);
		}
		else if((flag>=mfSWORD && flag<=mfXSWORD) || flag==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; ++i)
			{
				trigger_secrets_if_flag(x,y,mfSWORD+i,true);
			}
			
			trigger_secrets_if_flag(x,y,mfSTRIKE,true);
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
			sfx(tmpscr.secretsfx);
		}
		else if((flag2>=mfSWORD && flag2<=mfXSWORD)|| flag2==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; i++)
			{
				trigger_secrets_if_flag(x,y,mfSWORD+i,true);
			}
			
			trigger_secrets_if_flag(x,y,mfSTRIKE,true);
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
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((pos_handle.screen_index < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmp->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, tmp->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmp->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmp->secretsfx);
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
	
	//putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
	
	if(get_bit(quest_rules,qr_MORESOUNDS))
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
	ffcdata& ffc = tmpscr.ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
	auto type = cmb.type;
	if (!isCuttableType(type)) return;
	auto flag2 = cmb.flag;
	auto x = ffc.x, y = ffc.y;
	
	bool skipSecrets = isNextType(type) && !get_bit(quest_rules,qr_OLD_SLASHNEXT_SECRETS);
	bool done = false;
	if(!skipSecrets)
	{
		done = true;
		if(flag2 >= 16 && flag2 <= 31)
		{ 
			ffc.setData(tmpscr.secretcombo[(tmpscr.sflag[pos])-16+4]);
			ffc.cset = tmpscr.secretcset[(tmpscr.sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			ffc.setData(tmpscr.secretcombo[sSTAIRS]);
			ffc.cset = tmpscr.secretcset[sSTAIRS];
			sfx(tmpscr.secretsfx);
		}
		else if((flag2>=mfSWORD && flag2<=mfXSWORD)|| flag2==mfSTRIKE)
		{
			for(int32_t i=0; i <= 3; i++)
			{
				trigger_secrets_if_flag(x,y,mfSWORD+i,true);
			}
			
			trigger_secrets_if_flag(x,y,mfSTRIKE,true);
		}
		else done = false;
	}
	if(!done)
	{
		if(isCuttableNextType(type))
		{
			ffc.incData(1);
		}
		else
		{
			ffc.setData(tmpscr.undercombo);
			ffc.cset = tmpscr.undercset;
		}
	}
	
	if((flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr.flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, tmpscr.catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr.flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr.secretsfx);
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
	
	//putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
	
	if(get_bit(quest_rules,qr_MORESOUNDS))
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

bool trigger_step(const pos_handle_t& pos_handle)
{
	// TODO z3 this is repeated lots of places.
	if(unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;

	int32_t pos = RPOS_TO_POS(pos_handle.rpos);
	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];
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
			++pos_handle.screen->data[pos]; break;
		case cSTEPSAME:
		{
			// Increment all combos of the same id as the triggered combo on the base screen.
			// If the trigger is on a layer screen, that will be the only combo on that layer incremented.
			int32_t id = pos_handle.screen->data[pos];
			for_every_screen_in_region([&](mapscr* z3_scr, int screen_index, unsigned int z3_scr_dx, unsigned int z3_scr_dy) {
				for(auto q = 0; q < 176; ++q)
				{
					if (z3_scr->data[q] == id)
					{
						++z3_scr->data[q];
					}
				}
			});
			// TODO z3 ffc
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = pos_handle.screen->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = pos_handle.screen->ffcs[i];
					if (ffc2.getData() == id)
					{
						ffc2.incData(1);
					}
				}
			}
			if (pos_handle.layer > 0) ++pos_handle.screen->data[pos];
			break;
		}
		case cSTEPALL:
		{
			for_every_screen_in_region([&](mapscr* z3_scr, int screen_index, unsigned int z3_scr_dx, unsigned int z3_scr_dy) {
				for(auto q = 0; q < 176; ++q)
				{
					if (isStepType(combobuf[z3_scr->data[q]].type))
					{
						++z3_scr->data[q];
					}
				}
			});
			// TODO z3 ffc
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = pos_handle.screen->numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = pos_handle.screen->ffcs[i];
					if (isStepType(combobuf[ffc2.getData()].type))
					{
						ffc2.incData(1);
					}
				}
			}
			if (pos_handle.layer > 0) ++pos_handle.screen->data[pos];
			break;
		}
	}
	return true;
}

bool trigger_step_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr.ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
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
			ffc.incData(1); 
			break;
		}
		case cSTEPSAME:
		{
			int32_t id = ffc.getData();
			for(auto q = 0; q < 176; ++q)
			{
				if(tmpscr.data[q] == id)
				{
					++tmpscr.data[q];
				}
			}
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr.numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr.ffcs[i];
					if (ffc2.getData() == id && i != pos)
					{
						ffc2.incData(1);
					}
				}
			}
			ffc.incData(1);
			break;
		}
		case cSTEPALL:
		{
			for(auto q = 0; q < 176; ++q)
			{
				if(isStepType(combobuf[tmpscr.data[q]].type))
				{
					++tmpscr.data[q];
				}
			}
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr.numFFC();
				for(word i=0; i<c; i++)
				{
					ffcdata& ffc2 = tmpscr.ffcs[i];
					if (isStepType(combobuf[ffc2.getData()].type) && i != pos)
					{
						ffc2.incData(1);
					}
				}
			}
			ffc.incData(1);
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

void play_combo_string(int str, int screen_index)
{
	switch(str)
	{
		case -1: //Special case: Use Screen String
			str = get_scr(currmap, screen_index)->str;
			break;
		case -2: //Special case: Use Screen Catchall
			str = get_scr(currmap, screen_index)->catchall;
			break;
		case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17: //Special case: Screen->D[]
			int32_t di = (currdmap<<7) + screen_index-(DMaps[currdmap].type==dmOVERW ? 0 : DMaps[currdmap].xoff);
			str = game->screen_d[di][abs(str)-10] / 10000L;
			break;
	}
	if(unsigned(str) >= MAXMSGS)
		str = 0;
	if(str)
		donewmsg(str);
}

void play_combo_string(int str)
{
	play_combo_string(str, currscr);
}

void trigger_sign(newcombo const& cmb, int screen_index)
{
	play_combo_string(cmb.attributes[0]/10000L, screen_index);
}

void trigger_sign(newcombo const& cmb)
{
	play_combo_string(cmb.attributes[0]/10000L, currscr);
}

bool trigger_warp(newcombo const& cmb)
{
	if(!isWarpType(cmb.type)) return false;
	mapscr* wscr = currscr >= 128 ? &special_warp_return_screen : &tmpscr;
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
	if(tmpscr.flags3&fIWARPFULLSCREEN) wflag |= warpFlagDONTCLEARSPRITES;
	//Queue the warp for the next frame, as doing anything else breaks terribly
	FFCore.queueWarp(wtype, tdm, tscr, wx, wy, weff, wsfx, wflag, -1);
	return true;
}

bool trigger_chest(const pos_handle_t& pos_handle)
{
	int pos = RPOS_TO_POS(pos_handle.rpos);
	if (unsigned(pos_handle.layer) > 6 || unsigned(pos) > unsigned(region_max_rpos)) return false;

	mapscr* base_screen = pos_handle.layer == 0 ? pos_handle.screen : get_scr(currmap, pos_handle.screen_index);

	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];
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
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				// remove_xstatecombos_old((currscr>=128)?1:0, cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				// remove_xstatecombos2(pos_handle.screen, pos_handle.screen_index, cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mCHEST);
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
			if ( key_item > 0 && itemsbuf[key_item].script && !(item_doscript[key_item] && get_bit(quest_rules,qr_ITEMSCRIPTSKEEPRUNNING)) ) 
			{
				ri = &(itemScriptData[key_item]);
				for ( int32_t q = 0; q < 1024; q++ ) item_stack[key_item][q] = 0xFFFF;
				ri->Clear();
				item_doscript[key_item] = 1;
				itemscriptInitialised[key_item] = 0;
				ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllArrays(SCRIPT_ITEM,(key_item));
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				// remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mBOSSCHEST);
			break;
	}
	
	sfx(cmb.attribytes[3]); //opening sfx
	
	bool itemflag = false;
	for(int32_t i=0; i<3; i++)
	{
		mapscr* layer_scr = get_layer_scr(currmap, pos_handle.screen_index, i - 1);
		if(layer_scr->sflag[pos]==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
		if(combobuf[layer_scr->data[pos]].flag==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
	}
	bool itemstate = false;
	int32_t ipflag = 0;
	if(cmb.usrflags & cflag7)
	{
		itemstate = getmapflag((pos_handle.screen_index < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (pos_handle.screen_index < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((base_screen->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((currdmap)<<7) + pos_handle.screen_index-(DMaps[currdmap].type==dmOVERW ? 0 : DMaps[currdmap].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = base_screen->catchall;
				break;
		}
		if(unsigned(itid) >= MAXITEMS) itid = 0;
		item* itm = new item(Hero.getX(), Hero.getY(), 0, itid, pflags, 0);
		itm->set_forcegrab(true);
		items.add(itm);
	}
	return true;
}

bool trigger_chest_ffc(const pos_handle_t& pos_handle)
{
	int pos = (int)pos_handle.rpos;
	DCHECK(pos >= MAXFFCS);
	if (pos >= MAXFFCS) return false;

	ffcdata& ffc = pos_handle.screen->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
	int32_t cid = ffc.getData();
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
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mCHEST);
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
			if ( key_item > 0 && itemsbuf[key_item].script && !(item_doscript[key_item] && get_bit(quest_rules,qr_ITEMSCRIPTSKEEPRUNNING)) ) 
			{
				ri = &(itemScriptData[key_item]);
				for ( int32_t q = 0; q < 1024; q++ ) item_stack[key_item][q] = 0xFFFF;
				ri->Clear();
				item_doscript[key_item] = 1;
				itemscriptInitialised[key_item] = 0;
				ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllArrays(SCRIPT_ITEM,(key_item));
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mBOSSCHEST);
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
		itemstate = getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((tmpscr.flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((get_currdmap())<<7) + pos_handle.screen_index-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = tmpscr.catchall;
				break;
		}
		if(unsigned(itid) >= MAXITEMS) itid = 0;
		item* itm = new item(Hero.getX(), Hero.getY(), 0, itid, pflags, 0);
		itm->set_forcegrab(true);
		items.add(itm);
	}
	return true;
}


// TODO z3 test
bool trigger_lockblock(const pos_handle_t& pos_handle)
{
	DCHECK(pos_handle.rpos <= region_max_rpos);
	
	int pos = RPOS_TO_POS(pos_handle.rpos);
	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];
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
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos2(pos_handle.screen, pos_handle.screen_index, 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mLOCKBLOCK);
			remove_lockblocks(pos_handle.screen, pos_handle.screen_index);
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
			if (key_item > 0 && itemsbuf[key_item].script && !(item_doscript[key_item] && get_bit(quest_rules, qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				ri = &(itemScriptData[key_item]);
				for (int32_t q = 0; q < 1024; q++) item_stack[key_item][q] = 0xFFFF;
				ri->Clear();
				item_doscript[key_item] = 1;
				itemscriptInitialised[key_item] = 0;
				ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllArrays(SCRIPT_ITEM, (key_item));
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos2(pos_handle.screen, pos_handle.screen_index, 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mBOSSLOCKBLOCK);
			remove_bosslockblocks(pos_handle.screen, pos_handle.screen_index);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}

bool trigger_lockblock_ffc(const pos_handle_t& pos_handle)
{
	int pos = (int)pos_handle.rpos;
	DCHECK(pos >= MAXFFCS);
	if (pos >= MAXFFCS) return false;

	ffcdata& ffc = pos_handle.screen->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
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
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mLOCKBLOCK);
			remove_lockblocks_old((currscr>=128)?1:0);
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
			if (key_item > 0 && itemsbuf[key_item].script && !(item_doscript[key_item] && get_bit(quest_rules, qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				ri = &(itemScriptData[key_item]);
				for (int32_t q = 0; q < 1024; q++) item_stack[key_item][q] = 0xFFFF;
				ri->Clear();
				item_doscript[key_item] = 1;
				itemscriptInitialised[key_item] = 0;
				ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[key_item].script, key_item);
				FFCore.deallocateAllArrays(SCRIPT_ITEM, (key_item));
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(pos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos_old((currscr>=128)?1:0, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(pos_handle.screen, pos_handle.screen_index, mBOSSLOCKBLOCK);
			remove_bosslockblocks_old((currscr >= 128) ? 1 : 0);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}


bool trigger_armos_grave(const pos_handle_t& pos_handle, int32_t trigdir)
{
	if (pos_handle.layer != 0) return false; // Currently cannot activate on layers >0!
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;
	
	int pos = RPOS_TO_POS(pos_handle.rpos);
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
	mapscr* tmp = pos_handle.screen;
	newcombo const& cmb = combobuf[tmp->data[pos]];
	int32_t eclk = -14;
	int32_t id2 = 0;
	int32_t tx, ty;
	COMBOXY_REGION(pos_handle.rpos, tx, ty);
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
					if(guysbuf[i].flags2&cmbflag_armos)
					{
						id2=i;
						
						// This is mostly for backwards-compatability
						if(guysbuf[i].family==eeWALK && guysbuf[i].misc9==e9tARMOS)
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
								if ( combobuf[(tmpscr.data[pos-chy])].type == cARMOS )
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
								if ( combobuf[(tmpscr.data[pos-chx])].type == cARMOS )
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
								if ( combobuf[(tmpscr.data[pos-chx])].type == cARMOS )
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
								if ( combobuf[(tmpscr.data[pos-chy])].type == cARMOS )
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
								if ( combobuf[(tmpscr.data[pos-chx])].type == cARMOS ) 
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
								if ( combobuf[(tmpscr.data[pos-chy])].type == cARMOS ) 
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
					if ( combobuf[(tmpscr.data[pos-chx+1])].type == cARMOS ) xpos += 16;
				}
				if(addenemy(tx+xpos,ty+1+ypos,id2,0))
				{
					enemy* en = ((enemy*)guys.spr(guys.Count()-1));
					en->did_armos=false;
					en->fading=fade_flicker;
					en->flags2 |= cmbflag_armos;
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
					if(guysbuf[i].flags2&cmbflag_ghini)
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
	ffcdata& ffc = tmpscr.ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
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
					if(guysbuf[i].flags2&cmbflag_armos)
					{
						id2=i;
						
						// This is mostly for backwards-compatability
						if(guysbuf[i].family==eeWALK && guysbuf[i].misc9==e9tARMOS)
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
					if(guysbuf[i].flags2&cmbflag_ghini)
					{
						id2=i;
						eclk=0; // This is mostly for backwards-compatability
						break;
					}
				}
			}
			if(nextcmb)
				ffc.incData(1);
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


bool trigger_damage_combo(int32_t cid, int32_t hdir, bool force_solid)
{
	if(hdir > 3) hdir = -1;
	newcombo const& cmb = combobuf[cid];

	if(Hero.hclk || Hero.superman || Hero.fallclk)
		return false; //immune
	int32_t dmg = 0;
	if(cmb.usrflags & cflag1) //custom
		dmg = cmb.attributes[0] / -10000L;
	else dmg = combo_class_buf[cmb.type].modify_hp_amount;
	
	bool global_defring = ((itemsbuf[current_item_id(itype_ring)].flags & ITEM_FLAG1));
	bool global_perilring = ((itemsbuf[current_item_id(itype_perilring)].flags & ITEM_FLAG1));

	// TODO z3 !
	// bool current_ring = ((pos_handle.screen->flags6&fTOGGLERINGDAMAGE) != 0);
	bool current_ring = ((tmpscr.flags6&fTOGGLERINGDAMAGE) != 0);
	if(current_ring)
	{
		global_defring = !global_defring;
		global_perilring = !global_perilring;
	}
	
	int32_t itemid = current_item_id(itype_boots);
	
	bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & ITEM_FLAG1);
	bool ignoreBoots = itemid >= 0 && (itemsbuf[itemid].flags & ITEM_FLAG3);
	if(dmg < 0)
	{
		if(itemid < 0 || ignoreBoots || (tmpscr.flags5&fDAMAGEWITHBOOTS)
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

bool trigger_damage_combo(const pos_handle_t& pos_handle)
{
	int pos = RPOS_TO_POS(pos_handle.rpos);
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;
	int cid = pos_handle.screen->data[pos];
	return trigger_damage_combo(cid);
}

bool trigger_stepfx(const pos_handle_t& pos_handle, bool stepped)
{
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;

	int32_t tx, ty;
	COMBOXY_REGION(pos_handle.rpos, tx, ty);
	tx += 8;
	ty += 8;

	int pos = RPOS_TO_POS(pos_handle.rpos);
	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];

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
			
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir, -1,-1,false)); 
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
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wFire:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
								int32_t dmgamt = ((damg > 0) ? damg : 4);
								game->set_life(game->get_life()- Hero.ringpower(dmgamt));
								Hero.doHit(-1);
							}
						}
					}
					else
					{
						Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
					Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir, -1,-1,false)); 
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
							int32_t dmgamt = ((damg > 0) ? damg : 4);
							
							game->set_life(game->get_life()- Hero.ringpower(dmgamt));
						}
					}
				}
				else
				{
					Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,((damg > 0) ? damg : 4),up, -1,-1,false)); 
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
			pos_handle.screen->data[pos]++;
		}
	}
	return true;
}

bool trigger_stepfx_ffc(int32_t pos, bool stepped)
{
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr.ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];
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
			
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir, -1,-1,false)); 
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
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wFire:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
								int32_t dmgamt = ((damg > 0) ? damg : 4);
								game->set_life(game->get_life()- Hero.ringpower(dmgamt));
								Hero.doHit(-1);
							}
						}
					}
					else
					{
						Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
					Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir, -1,-1,false)); 
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
							int32_t dmgamt = ((damg > 0) ? damg : 4);
							
							game->set_life(game->get_life()- Hero.ringpower(dmgamt));
						}
					}
				}
				else
				{
					Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
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
				Ewpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,((damg > 0) ? damg : 4),up, -1,-1,false)); 
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
			ffc.incData(1);
		}
	}
	return true;
}


bool trigger_switchhookblock(const pos_handle_t& pos_handle)
{
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;
	if(Hero.switchhookclk) return false;

	int pos = RPOS_TO_POS(pos_handle.rpos);
	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];
	switching_object = NULL;
	hooked_comborpos = pos_handle.rpos;
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
	ffcdata& ffc = tmpscr.ffcs[pos];
	switching_object = &ffc;
	switching_object->switch_hooked = true;
	hooked_comborpos = rpos_t::NONE;
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
		wpn = new weapon((zfix)wx,(zfix)wy,(zfix)0,weapid,0,damage,wdir,-1, Hero.getUID(),false,0,1,0,0,weapspr);
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
	wpn->yofs = (get_bit(quest_rules, qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
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
		wpn->doscript = 1;
	}
	return wpn;
}
bool trigger_shooter(newcombo const& cmb, zfix wx, zfix wy)
{
	if(cmb.type != cSHOOTER) return false;
	if(!cmb.attribytes[1]) return false; //no weapon
	if(wx >= world_w || wx < -15 || wy >= world_h || wy < -15) return false;
	
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
	// TODO z3 !
	save_game((tmpscr.flags4&fSAVEROOM) != 0, save_type);
}

static byte copycat_id = 0;
static bool do_copycat_trigger(const pos_handle_t& pos_handle)
{
	if(!copycat_id) return false;
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;
	int pos = RPOS_TO_POS(pos_handle.rpos);
	
	if (unsigned(pos) > 175) return false;
	int32_t cid = pos_handle.screen->data[pos];
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo(pos_handle.layer, pos);
		return true;
	}
	return false;
}

bool do_copycat_trigger_ffc(int32_t pos)
{
	if(!copycat_id) return false;
	if(unsigned(pos) >= MAXFFCS) return false;
	ffcdata& ffc = tmpscr.ffcs[pos];
	int32_t cid = ffc.getData();
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo_ffc(pos);
		return true;
	}
	return false;
}

void do_ex_trigger(const pos_handle_t& pos_handle)
{
	if (pos_handle.rpos > region_max_rpos) return;

	int32_t pos = RPOS_TO_POS(pos_handle.rpos);
	int32_t cid = pos_handle.screen->data[pos];
	int32_t ocs = pos_handle.screen->cset[pos];
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		pos_handle.screen->data[pos] = cid+cmb.trigchange;
	}
	if(cmb.trigcschange)
	{
		pos_handle.screen->cset[pos] = (ocs+cmb.trigcschange) & 0xF;
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[pos_handle.screen->data[pos]];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
	{
		if(!copycat_id) //not already in a copycat
		{
			bool skipself = pos_handle.screen->data[pos] == cid;
			copycat_id = cmb.trigcopycat;
			for_every_rpos_in_region([&](const pos_handle_t& cc_pos_handle) {
				if (skipself && cc_pos_handle.layer == pos_handle.layer && cc_pos_handle.rpos == pos_handle.rpos) return;
				do_copycat_trigger(cc_pos_handle);
			});
			// TODO z3 ffc
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr.numFFC();
				for(word i=0; i<c; i++)
				{
					do_copycat_trigger_ffc(i);
				}
			}
			copycat_id = 0;
		}
	}
}

void do_ex_trigger_ffc(int32_t pos)
{
	if(unsigned(pos) >= MAXFFCS) return;
	ffcdata& ffc = tmpscr.ffcs[pos];
	int32_t cid = ffc.getData();
	int32_t ocs = ffc.cset;
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		ffc.setData(cid+cmb.trigchange);
	}
	if(cmb.trigcschange)
	{
		ffc.cset = (ocs+cmb.trigcschange) & 0xF;
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[ffc.getData()];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
	{
		if(!copycat_id) //not already in a copycat
		{
			bool skipself = ffc.getData() == cid;
			copycat_id = cmb.trigcopycat;
			for(auto cclayer = 0; cclayer < 7; ++cclayer)
			{
				for(auto ccpos = 0; ccpos < 176; ++ccpos)
				{
					// TODO z3 ffc
					do_copycat_trigger(get_pos_handle((rpos_t)ccpos, cclayer));
				}
			}
			copycat_id = 0;
			if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
			{
				word c = tmpscr.numFFC();
				for(word i=0; i<c; i++)
				{
					if (i == pos && skipself)
						continue;
					do_copycat_trigger_ffc(i);
				}
			}
		}
	}
}

bool force_ex_trigger(const pos_handle_t& pos_handle, char xstate)
{
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;
	int pos = RPOS_TO_POS(pos_handle.rpos);
	newcombo const& cmb = combobuf[pos_handle.screen->data[pos]];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(pos_handle.screen_index, 1<<cmb.exstate))
		{
			do_ex_trigger(pos_handle);
			return true;
		}
	}
	return false;
}

bool force_ex_trigger_ffc(const pos_handle_t& pos_handle, char xstate)
{
	int pos = RPOS_TO_POS(pos_handle.rpos);
	if (unsigned(pos) >= MAXFFCS) return false;

	ffcdata& ffc = pos_handle.screen->ffcs[pos];
	newcombo const& cmb = combobuf[ffc.getData()];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(pos_handle.screen_index, 1<<cmb.exstate))
		{
			// TODO z3 !!!
			do_ex_trigger_ffc(pos);
			return true;
		}
	}
	return false;
}

static bool triggering_generic_secrets = false;
static bool triggering_generic_switchstate = false;

// TODO z3 remove
bool do_trigger_combo(int layer, int pos, int32_t special, weapon* w)
{
	if (unsigned(pos) > 175) return false;
	return do_trigger_combo(get_pos_handle((rpos_t)pos, layer), special, w);
}

// Forcibly triggers a combo at a given position
bool do_trigger_combo(const pos_handle_t& pos_handle, int32_t special, weapon* w)
{
	if (unsigned(pos_handle.layer) > 6 || pos_handle.rpos > region_max_rpos) return false;

	int lyr = pos_handle.layer;
	int32_t pos = RPOS_TO_POS(pos_handle.rpos);
	// TODO z3 !!
	cpos_info& timer = combo_posinfos[lyr][pos];
	int32_t cid = pos_handle.screen->data[pos];
	int32_t cx, cy;
	COMBOXY_REGION(pos_handle.rpos, cx, cy);

	int32_t ocs = pos_handle.screen->cset[pos];
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	
	word ctramnt = game->get_counter(cmb.trigctr);
	bool onlytrigctr = !(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG);
	
	int32_t flag = pos_handle.screen->sflag[pos];
	int32_t flag2 = cmb.flag;
	
	byte* grid = nullptr;
	bool check_bit = false;
	bool used_bit = false;
	
	uint32_t exflag = 0;
	if(cmb.exstate > -1)
	{
		exflag = 1<<cmb.exstate;
		if (force_ex_trigger(pos_handle))
			return true;
	}
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
	
	if(!onlytrigctr && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
	{
		if(ctramnt >= cmb.trigctramnt)
		{
			game->change_counter(-cmb.trigctramnt, cmb.trigctr);
		}
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERGE)
	{
		if(ctramnt < cmb.trigctramnt)
			return false;
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERLT)
	{
		if(ctramnt >= cmb.trigctramnt)
			return false;
	}
	
	if(w)
	{
		// TODO z3
		grid = (lyr ? w->wscreengrid_layer[lyr-1] : w->wscreengrid);
		check_bit = get_bit(grid, pos);
	}
	if(!timer.trig_cd)
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
						trigger_cswitch_block(pos_handle);
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
						trigger_cuttable(pos_handle);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step(pos_handle))
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
						if(!trigger_chest(pos_handle))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock(pos_handle))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave(pos_handle))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(pos_handle);
						break;
					
					case cSTEPSFX:
						trigger_stepfx(pos_handle);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock(pos_handle))
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
			if (cmb.triggerflags[1]&combotriggerSECRETS)
			{
				used_bit = true;
				if(!(special & ctrigSECRETS) && !triggering_generic_secrets)
				{
					triggering_generic_secrets = true;
					trigger_secrets_for_screen(false, -6);
					triggering_generic_secrets = false;
					if(pos_handle.screen->secretsfx)
						sfx(pos_handle.screen->secretsfx);
				}
				if(canPermSecret(currdmap, pos_handle.screen_index) && !(pos_handle.screen->flags5&fTEMPSECRETS))
					setmapflag(pos_handle.screen, pos_handle.screen_index, mSECRET);
			}
			
			if (cmb.triggerflags[3] & combotriggerLEVELSTATE)
			{
				used_bit = true;
				if(!(special & ctrigSWITCHSTATE) && !triggering_generic_switchstate)
				{
					triggering_generic_switchstate = true;
					// TODO z3 !
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
						// TODO z3 !
						toggle_gswitches(pair, false);
						triggering_generic_switchstate = false;
					}
				}
			}
			
			if(cmb.trigchange)
			{
				used_bit = true;
				pos_handle.screen->data[pos] = cid+cmb.trigchange;
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				pos_handle.screen->cset[pos] = (ocs+cmb.trigcschange) & 0xF;
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[pos_handle.screen->data[pos]];
				rcmb.tile = rcmb.o_tile;
				rcmb.cur_frame=0;
				rcmb.aclk = 0;
			}
			
			if(cmb.trigsfx)
				sfx(cmb.trigsfx, pan(COMBOX(pos)));
			
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
			if(onlytrigctr && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
			{
				if(ctramnt >= cmb.trigctramnt)
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
			
			if(cmb.exstate > -1 && trigexstate)
			{
				setxmapflag(pos_handle.screen_index, exflag);
			}
			
			if(cmb.trigcopycat) //has a copycat set
			{
				if(!copycat_id) //not already in a copycat
				{
					bool skipself = pos_handle.screen->data[pos] == cid;
					copycat_id = cmb.trigcopycat;
					for_every_rpos_in_region([&](const pos_handle_t& cc_pos_handle) {
						if (skipself && cc_pos_handle.layer == pos_handle.layer && cc_pos_handle.rpos == pos_handle.rpos) return;
						do_copycat_trigger(cc_pos_handle);
					});
					// TODO z3 ffc
					if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
					{
						word c = tmpscr.numFFC();
						for(word i=0; i<c; i++)
						{
							do_copycat_trigger_ffc(i);
						}
					}
					copycat_id = 0;
				}
			}
			
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if(used_bit && grid)
		{
			set_bit(grid,pos,1);
		}
	}

	if(w && (cmb.triggerflags[0] & combotriggerKILLWPN))
		killgenwpn(w);
	return true;
}

// TODO z3 !!!
bool do_trigger_combo_ffc(int32_t i, int32_t special, weapon* w)
{
	if (get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY)) return false;
	if(unsigned(i) >= MAXFFCS) return false;

	// TODO z3 this is an abuse of pos_handle_t
	pos_handle_t pos_handle = {&tmpscr, currscr, 0, (rpos_t)i};
	ffcdata& ffc = tmpscr.ffcs[i];
	cpos_info& timer = ffc_posinfos[i];
	int32_t cid = ffc.getData();
	int32_t ocs = ffc.cset;
	int32_t cx = ffc.x;
	int32_t cy = ffc.y;
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	
	word ctramnt = game->get_counter(cmb.trigctr);
	bool onlytrigctr = !(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG);
	
	int32_t flag2 = cmb.flag;
	
	byte* grid = nullptr;
	bool check_bit = false;
	bool used_bit = false;
	
	uint32_t exflag = 0;
	if(cmb.exstate > -1)
	{
		exflag = 1<<cmb.exstate;
		if(force_ex_trigger_ffc(pos_handle))
			return true;
	}
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
	
	if(!onlytrigctr && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
	{
		if(ctramnt >= cmb.trigctramnt)
		{
			game->change_counter(-cmb.trigctramnt, cmb.trigctr);
		}
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERGE)
	{
		if(ctramnt < cmb.trigctramnt)
			return false;
	}
	if(cmb.triggerflags[1] & combotriggerCOUNTERLT)
	{
		if(ctramnt >= cmb.trigctramnt)
			return false;
	}
	
	if(w)
	{
		grid = w->wscreengrid_ffc;
		check_bit = get_bit(grid,i);
	}
	if(!timer.trig_cd)
	{
		if((cmb.triggerflags[0] & combotriggerCMBTYPEFX) || alwaysCTypeEffects(cmb.type))
		{
			switch(cmb.type)
			{
				case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
				case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
				case cTRIGGERGENERIC:
					if(w)
						do_generic_combo_ffc(w, i, cid, flag2);
					else do_generic_combo_ffc2(i, cid, flag2);
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
						trigger_cswitch_block_ffc(i);
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
						trigger_cuttable_ffc(i);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step_ffc(i))
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
						if(!trigger_chest_ffc(pos_handle))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock_ffc(pos_handle))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave_ffc(i))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(cid);
						break;
					
					case cSTEPSFX:
						trigger_stepfx_ffc(i);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock_ffc(i))
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
			if (cmb.triggerflags[1]&combotriggerSECRETS)
			{
				used_bit = true;
				if(!(special & ctrigSECRETS) && !triggering_generic_secrets)
				{
					triggering_generic_secrets = true;
					hidden_entrance(0, true, false, -6);
					triggering_generic_secrets = false;
				}
				// TODO z3 ffc
				if(canPermSecret(currdmap, currscr) && !(tmpscr.flags5&fTEMPSECRETS))
					setmapflag(mSECRET);
				sfx(tmpscr.secretsfx);
			}
			
			if(cmb.trigchange)
			{
				used_bit = true;
				ffc.setData(cid+cmb.trigchange);
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				ffc.cset = (ocs+cmb.trigcschange) & 0xF;
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[ffc.getData()];
				rcmb.tile = rcmb.o_tile;
				rcmb.cur_frame=0;
				rcmb.aclk = 0;
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
			if(onlytrigctr && (cmb.triggerflags[1] & combotriggerCOUNTEREAT))
			{
				if(ctramnt >= cmb.trigctramnt)
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
			
			if(cmb.exstate > -1 && trigexstate)
			{
				setxmapflag(pos_handle.screen_index, exflag);
			}
			
			if(cmb.trigcopycat) //has a copycat set
			{
				if(!copycat_id) //not already in a copycat
				{
					bool skipself = ffc.getData() == cid;
					copycat_id = cmb.trigcopycat;
					for(auto cclayer = 0; cclayer < 7; ++cclayer)
					{
						for(auto ccpos = 0; ccpos < 176; ++ccpos)
						{
							do_copycat_trigger(get_pos_handle((rpos_t)ccpos, cclayer));
						}
					}
					if (!get_bit(quest_rules,qr_OLD_FFC_FUNCTIONALITY))
					{
						word c = tmpscr.numFFC();
						for(word j=0; j<c; j++)
						{
							if (j == i && skipself)
								continue;
							do_copycat_trigger_ffc(j);
						}
					}
					copycat_id = 0;
				}
			}
			
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if(used_bit && grid)
		{
			set_bit(grid,i,1);
		}
	}
	
	if(w && (cmb.triggerflags[0] & combotriggerKILLWPN))
		killgenwpn(w);
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
	auto pflags = ipBIGRANGE | ((cmb.liftflags&LF_SPECIALITEM) ? ipONETIME : ipTIMER);
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
	
	weapon* w = new weapon(cx, cy, 0, wThrown, 0, cmb.liftdmg*game->get_hero_dmgmult(),
		oppositeDir[NORMAL_DIR(HeroDir())], gloveid, Hero.getUID(), false, 0, 1);
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
	
	w->moveflags |= FLAG_OBEYS_GRAV;
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

void init_combo_timers()
{
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		for(auto pos = 0; pos < 176; ++pos)
		{
			combo_posinfos[lyr][pos].clear();
		}
	}
	ffc_posinfos.clear();
}

bool on_cooldown(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 7 || unsigned(pos) > 176)
		return false;
	return combo_posinfos[lyr][pos].trig_cd != 0;
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
	
	if(timer.shootrclk > 1)
	{
		if(--timer.shootrclk == 1)
		{
			if(!instashot) trigger_shooter(cmb, wx, wy);
		}
	}
	else
	{
		auto rate = (splitrate ? zc_rand(highrate,lowrate) : lowrate);
		timer.shootrclk = zc_max(1,rate);
		if(instashot || timer.shootrclk == 1) trigger_shooter(cmb, wx, wy);
	}
}

// TODO z3
static void handle_shooter(newcombo const& cmb, cpos_info& timer, rpos_t rpos)
{
	int x, y;
	COMBOXY_REGION(rpos, x, y);
	handle_shooter(cmb, timer, x, y);
}

void update_combo_timers()
{
	for_every_rpos_in_region([&](const pos_handle_t& pos_handle) {
		int pos = RPOS_TO_POS(pos_handle.rpos);
		// TODO z3 !
		cpos_info& timer = combo_posinfos[pos_handle.layer][pos];
		timer.updateData(pos_handle.screen->data[pos]);
		newcombo const& cmb = combobuf[timer.data];
		if(cmb.trigtimer)
		{
			if(++timer.clk >= cmb.trigtimer)
			{
				timer.clk = 0;
				do_trigger_combo(pos_handle.layer, pos);
				timer.updateData(pos_handle.screen->data[pos]);
			}
		}
		if(timer.trig_cd) --timer.trig_cd;
		if(cmb.type == cSHOOTER)
		{
			handle_shooter(cmb, timer, pos_handle.rpos);
		}
	});

	// TODO z3 ffc stuff

	mapscr* ffscr = FFCore.tempScreens[0];
	dword c = ffscr->numFFC();
	if(ffc_posinfos.size() != c)
	{
		dword osz = ffc_posinfos.size();
		ffc_posinfos.resize(c);
		for(dword q = osz; q < c; ++q) //Is this needed? -Em
		{
			ffc_posinfos[q].clear();
		}
	}
	for(word ffc = 0; ffc < c; ++ffc)
	{
		cpos_info& timer = ffc_posinfos[ffc];
		timer.updateData(ffscr->ffcs[ffc].getData());
		newcombo const& cmb = combobuf[timer.data];
		if(cmb.trigtimer)
		{
			if(++timer.clk >= cmb.trigtimer)
			{
				timer.clk = 0;
				do_trigger_combo_ffc(ffc);
				timer.updateData(ffscr->ffcs[ffc].getData());
			}
		}
		if(timer.trig_cd) --timer.trig_cd;
		if(cmb.type == cSHOOTER)
		{
			zfix wx = ffscr->ffcs[ffc].x;
			zfix wy = ffscr->ffcs[ffc].y;
			wx += (ffscr->ffTileWidth(ffc)-1)*8;
			wy += (ffscr->ffTileHeight(ffc)-1)*8;
			handle_shooter(cmb, timer, wx, wy);
		}
	}
}
