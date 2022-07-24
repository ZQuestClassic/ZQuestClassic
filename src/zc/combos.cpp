#include "zelda.h"
#include "sprite.h"
#include "decorations.h"
#include "combos.h"
#include "maps.h"
#include "items.h"
#include "guys.h"
#include "ffscript.h"
#include "hero.h"

extern sprite_list items, decorations;
extern FFScript FFCore;
extern HeroClass Hero;
extern refInfo *ri;
extern refInfo itemScriptData[256];
extern word item_doscript[256];
extern int32_t item_stack[256][MAX_SCRIPT_REGISTERS];
extern byte itemscriptInitialised[256];

struct cmbtimer
{
	int32_t data;
	byte clk;
	void clear()
	{
		data = 0;
		clk = 0;
	}
	cmbtimer() : data(0), clk(0) {}
};
cmbtimer combo_trig_timers[7][176];

bool alwaysCTypeEffects(int32_t type)
{
	switch(type)
	{
		case cCUSTOMBLOCK:
			return true;
	}
	return false;
}

bool isNextType(int32_t type)
{
	switch(type)
	{
		case cLIFTSLASHNEXT:
		case cLIFTSLASHNEXTSPECITEM:
		case cLIFTSLASHNEXTITEM:
		case cDIGNEXT:
		case cLIFTNEXT:
		case cLIFTNEXTITEM:
		case cLIFTNEXTSPECITEM:
		case cSLASHNEXT:
		case cBUSHNEXT:
		case cTALLGRASSNEXT:
		case cSLASHNEXTITEM:
		case cSLASHNEXTTOUCHY:
		case cSLASHNEXTITEMTOUCHY:
		case cBUSHNEXTTOUCHY:
		{
			return true;
		}
		default: return false;
	}
}

bool isWarpType(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
			return true;
	}
	return false;
}

int32_t getWarpLetter(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSWIMWARP: case cDIVEWARP: case cPIT:
		case cAWARPA: case cSWARPA:
			return 0;
		case cSTAIRB: case cSWIMWARPB: case cDIVEWARPB: case cPITB:
		case cAWARPB: case cSWARPB:
			return 1;
		case cSTAIRC: case cSWIMWARPC: case cDIVEWARPC: case cPITC:
		case cAWARPC: case cSWARPC:
			return 2;
		case cSTAIRD: case cSWIMWARPD: case cDIVEWARPD: case cPITD:
		case cAWARPD: case cSWARPD:
			return 3;
		case cSTAIRR: case cPITR: case cAWARPR: case cSWARPR:
			return 4;
	}
	return -1;
}

int32_t simplifyWarpType(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
			return cSTAIR;
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
			return cSWIMWARP;
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
			return cDIVEWARP;
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
			return cPIT;
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
			return cAWARPA;
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
			return cSWARPA;
	}
	return 0;
}

bool isStepType(int32_t type)
{
	switch(type)
	{
		case cSTEP: case cSTEPSAME:
		case cSTEPALL: case cSTEPCOPY:
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
		if ( (combobuf[cid].usrflags&cflag2) )
		{
			if ( combobuf[cid].usrflags&cflag11 ) //specific item
			{
				it = combobuf[cid].attribytes[1];
			}
			else it = select_dropitem(combobuf[cid].attribytes[1]); 
		}
		if( it != -1 )
		{
			items.add(new item((zfix)COMBOX(scombo), (zfix)COMBOY(scombo),(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(mSPECIALITEM))
		{
			items.add(new item((zfix)COMBOX(scombo),
				(zfix)COMBOY(scombo),
				(zfix)0,
				tmpscr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[tmpscr->item].family==itype_triforcepiece ||
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
					
					screen_combo_modify_preroutine(tmpscr,scombo);
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
					screen_combo_modify_postroutine(tmpscr,scombo);
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

bool do_cswitch_combo(newcombo const& cmb, int32_t layer, int32_t cpos, weapon* w)
{
	mapscr* scr = (layer ? &tmpscr2[layer] : tmpscr);
	byte pair = cmb.attribytes[0];
	if(pair > 31) return false;
	game->lvlswitches[dlevel] ^= (1 << pair);
	toggle_switches(1<<pair, false);
	if(w && (cmb.usrflags&cflag1))
		killgenwpn(w); //Kill weapon
	if(cmb.attribytes[1]) sfx(cmb.attribytes[1]);
	return true;
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
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr->secretsfx);
	}
	else if(isCuttableItemType(type))
	{
		int32_t it = -1;
		if (cmb.usrflags&cflag2) //specific dropset or item
		{
			if (cmb.usrflags&cflag11) 
			{
				it = cmb.attribytes[1];
			}
			else
			{
				it = select_dropitem(cmb.attribytes[1]);
			}
		}
		else it = select_dropitem(12);
		
		if(it!=-1)
		{
			items.add(new item((zfix)x, (zfix)y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
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
			if(tmp != tmpscr) ++tmp->data[pos];
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
	else if ( (cmb.usrflags&cflag4) )
	{
		if ( game->get_counter(thecounter) >= ctr_amount )
		{
			//flag 6 only checks the required count; it doesn't drain it
			if (!(cmb.usrflags&cflag7)) game->change_counter(-(ctr_amount), thecounter);
			return true;
		}
		else if (cmb.usrflags&cflag6) //eat counter even if insufficient, but don't unlock
		{
			//shadowtiger requested this on 29th Dec, 2019 -Z
			if (!(cmb.usrflags&cflag7)) game->change_counter(-(game->get_counter(thecounter)), thecounter);
			return false;
		}
	}
	else if (ctr_amount && usekey(ctr_amount) ) return true;
	else if(!ctr_amount && !requireditem && !itemonly && usekey() ) return true;
	return false;
}

void trigger_sign(newcombo const& cmb)
{
	int32_t str = cmb.attributes[0]/10000L;
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
	if(unsigned(str) >= MAXMSGS)
		str = 0;
	if(str)
		donewmsg(str);
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
	auto wx = wscr->warpreturnx[wrindex];
	auto wy = wscr->warpreturny[wrindex];
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
			if(!try_locked_combo(cmb)) return false;
			
			setmapflag(mLOCKEDCHEST);
			break;
			
		case cCHEST:
			setmapflag(mCHEST);
			break;
			
		case cBOSSCHEST:
			if(!(game->lvlitems[dlevel]&liBOSSKEY)) return false;
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
	
	if(itemflag && !getmapflag((currscr < 128 && get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM))
	{
		item* itm = new item(Hero.getX(), Hero.getY(), 0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0);
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
			if(!try_locked_combo(cmb)) return false;
			
			setmapflag(mLOCKBLOCK);
			remove_lockblocks((currscr>=128)?1:0);
			break;
			
		case cBOSSLOCKBLOCK:
		{
			if (!(game->lvlitems[dlevel] & liBOSSKEY)) return false;
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
				
				if ( ( armosxsz > 1 ) || ( armosysz > 1 ) )
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
								if ( ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS ) ) 
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
								if ( ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS ) ) 
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
								if ( ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS ) ) 
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
								if ( ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS ) ) 
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
								if ( ( combobuf[(tmpscr->data[pos-chx])].type == cARMOS ) ) 
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
								if ( ( combobuf[(tmpscr->data[pos-chy])].type == cARMOS ) ) 
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
					if ( ( combobuf[(tmpscr->data[pos-chx+1])].type == cARMOS ) ) xpos += 16;
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

bool trigger_damage_combo(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
	if(Hero.hclk || Hero.superman || Hero.fallclk)
		return false; //immune
	int32_t dmg = 0;
	if(cmb.usrflags & cflag1) //custom
		dmg = cmb.attributes[0] / -10000L;
	else dmg = combo_class_buf[cmb.type].modify_hp_amount;
	
	bool global_ring = (((itemsbuf[current_item_id(itype_ring)].flags & ITEM_FLAG1)) || ((itemsbuf[current_item_id(itype_perilring)].flags & ITEM_FLAG1)));
	bool global_defring = ((itemsbuf[current_item_id(itype_perilring)].flags & ITEM_FLAG1));
	bool global_perilring = ((itemsbuf[current_item_id(itype_perilring)].flags & ITEM_FLAG1));
	bool current_ring = ((tmpscr->flags6&fTOGGLERINGDAMAGE) != 0);
	
	int32_t itemid = current_item_id(itype_boots);
	
	bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & ITEM_FLAG1);
	bool ignoreBoots = itemid >= 0 && (itemsbuf[itemid].flags & ITEM_FLAG3);
	if(dmg < 0)
	{
		if(itemid < 0 || ignoreBoots || (tmpscr->flags5&fDAMAGEWITHBOOTS)
			|| (4<<current_item_power(itype_boots)<(abs(dmg))) || ((cmb.walk&0xF) && bootsnosolid)
			|| !(checkbunny(itemid) && checkmagiccost(itemid)))
		{
			if(Hero.NayrusLoveShieldClk<=0)
			{
				int32_t ringpow = Hero.ringpower(-dmg, !global_perilring, !global_defring);
				game->set_life(zc_max(game->get_life()-(global_ring!=current_ring ? ringpow:-dmg),0));
			}
			Hero.doHit(-1); //set hit action, iframes, etc
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
	int32_t tx = COMBOX(pos)+8, ty = COMBOY(pos)+8;
	int32_t thesfx = cmb.attribytes[0];
	if ( thesfx > 0 && !sfx_allocated(thesfx))
		sfx(thesfx,pan(COMBOX(pos)));
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
				if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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
			//case wThrowRock: 
			//case wPot: //Thrown pot or rock -Z
			//case wLit: //Lightning or Electric -Z
			//case wBombos: 
			//case wEther: 
			//case wQuake:// -Z
			//case wSword180: 
			//case wSwordLA:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wFire:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,((damg > 0) ? damg : 4),wpdir,-1, Hero.getUID(),false,0,1,0));
				if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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
						if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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
					if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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
					if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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
				if (cmb.attribytes[3] > 0 && cmb.attribytes[3] < 256 )
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

bool trigger_switchhookblock(int32_t lyr, int32_t pos)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return false;
	if(Hero.switchhookclk) return false;
	mapscr* tmp = FFCore.tempScreens[lyr];
	newcombo const& cmb = combobuf[tmp->data[pos]];
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

//Forcibly triggers a combo at a given position
void do_trigger_combo(int32_t lyr, int32_t pos, int32_t special, weapon* w)
{
	if(unsigned(lyr) > 6 || unsigned(pos) > 175) return;
	mapscr* tmp = FFCore.tempScreens[lyr];
	int32_t cid = tmp->data[pos];
	int32_t cx = COMBOX(pos);
	int32_t cy = COMBOY(pos);
	newcombo const& cmb = combobuf[cid];
	if(cmb.triggeritem && (!game->get_item(cmb.triggeritem)
		|| item_disabled(cmb.triggeritem) || !checkbunny(cmb.triggeritem)))
		return;
	int32_t flag = tmp->sflag[pos];
	int32_t flag2 = cmb.flag;
	
	byte* grid = nullptr;
	bool check_bit = false;
	bool used_bit = false;
	if(w)
	{
		grid = (lyr ? w->wscreengrid_layer[lyr-1] : w->wscreengrid);
		check_bit = get_bit(grid,(((cx>>4) + cy)));
	}
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
				case cCSWITCH:
					do_cswitch_combo(cmb, lyr, pos, w);
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
						return;
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
						return;
					break;
				case cLOCKBLOCK: case cBOSSLOCKBLOCK:
					if(!trigger_lockblock(lyr,pos))
						return;
					break;
				
				case cARMOS: case cBSGRAVE: case cGRAVE:
					if(!trigger_armos_grave(lyr,pos))
						return;
					break;
				
				case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
				case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
					trigger_damage_combo(lyr,pos);
					break;
				
				case cSTEPSFX:
					trigger_stepfx(lyr,pos);
					break;
				
				case cSWITCHHOOK:
					if(!trigger_switchhookblock(lyr,pos))
						return;
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
			hidden_entrance(0, true, false, -6);
			if(canPermSecret() && !(tmpscr->flags5&fTEMPSECRETS))
				setmapflag(mSECRET);
			sfx(tmpscr->secretsfx);
		}
		
		if(cmb.trigchange)
		{
			used_bit = true;
			tmp->data[pos] = cid+cmb.trigchange;
		}
		
		if(cmb.triggerflags[0] & combotriggerRESETANIM)
		{
			newcombo& rcmb = combobuf[tmp->data[pos]];
            rcmb.tile = rcmb.o_tile;
			rcmb.cur_frame=0;
			rcmb.aclk = 0;
		}
		
		if(cmb.trigsfx)
			sfx(cmb.trigsfx, pan(COMBOX(pos)));
	}
	if(used_bit && grid)
	{
		set_bit(grid,(((cx>>4) + cy)),1);
	}
	if(w && (cmb.triggerflags[0] & combotriggerKILLWPN))
		killgenwpn(w);
}

void init_combo_timers()
{
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		for(auto pos = 0; pos < 176; ++pos)
		{
			combo_trig_timers[lyr][pos].clear();
		}
	}
}

void update_combo_timers()
{
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		mapscr* scr = FFCore.tempScreens[lyr];
		for(auto pos = 0; pos < 176; ++pos)
		{
			cmbtimer& timer = combo_trig_timers[lyr][pos];
			if(timer.data != scr->data[pos])
			{
				timer.data = scr->data[pos];
				timer.clk = 0;
			}
			newcombo const& cmb = combobuf[timer.data];
			if(cmb.trigtimer)
			{
				if(++timer.clk >= cmb.trigtimer)
				{
					timer.clk = 0;
					do_trigger_combo(lyr, pos);
					timer.data = scr->data[pos];
				}
			}
		}
	}
}

