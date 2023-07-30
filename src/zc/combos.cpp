#include "base/handles.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "zc/zelda.h"
#include "sprite.h"
#include "zc/decorations.h"
#include "zc/combos.h"
#include "zc/maps.h"
#include "items.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/hero.h"
#include "zc/title.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
#include "iter.h"

extern sprite_list items, decorations;
extern FFScript FFCore;
extern HeroClass Hero;

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

static std::vector<cpos_info> combo_posinfos;

void clear_combo_posinfo()
{
	combo_posinfos.resize(region_num_rpos * 7);
	for (int i = 0; i < region_num_rpos * 7; i++)
	{
		combo_posinfos[i].clear();
	}
}

cpos_info& get_combo_posinfo(const rpos_handle_t& rpos_handle)
{
	int index = rpos_handle.layer * region_num_rpos + (int)rpos_handle.rpos;
	return combo_posinfos[index];
}

void set_combo_posinfo(const rpos_handle_t& rpos_handle, cpos_info& posinfo)
{
	int index = rpos_handle.layer * region_num_rpos + (int)rpos_handle.rpos;
	combo_posinfos[index] = posinfo;
}

int trig_groups[256];

bool alwaysCTypeEffects(int32_t type)
{
	switch(type)
	{
		case cCUSTOMBLOCK:
			return true;
	}
	return false;
}

static void do_generic_combo2(int32_t bx, int32_t by, int32_t cid, int32_t flag, int32_t flag2, int32_t ft, const rpos_handle_t& rpos_handle, bool single16)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	}

	mapscr* screen = rpos_handle.screen;
	int pos = rpos_handle.pos();
	int x, y;
	COMBOXY_REGION(rpos_handle.rpos, x, y);

	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	if (true) // Probably needs a way to only be triggered once...
	{
		if ((combobuf[cid].usrflags&cflag1)) 
		{
			if (combobuf[cid].usrflags & cflag10)
			{
				switch (combobuf[cid].attribytes[0])
				{
					case 0:
					case 1:
					default:
						decorations.add(new dBushLeaves(x, y, dBUSHLEAVES, 0, 0));
						break;
					case 2:
						decorations.add(new dFlowerClippings(x, y, dFLOWERCLIPPINGS, 0, 0));
						break;
					case 3:
						decorations.add(new dGrassClippings(x, y, dGRASSCLIPPINGS, 0, 0));
						break;
				}
			}
			else decorations.add(new comboSprite(x, y, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
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
			item* itm = (new item(x, y,0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(mSPECIALITEM))
		{
			items.add(new item(x, y, 0,
				screen->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[screen->catchall].family==itype_triforcepiece ||
				(screen->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((screen->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_combo_modify_preroutine(rpos_handle);
			screen->data[pos] = screen->secretcombo[ft];
			screen->cset[pos] = screen->secretcset[ft];
			screen->sflag[pos] = screen->secretflag[ft];
			// newflag = s->secretflag[ft];
			screen_combo_modify_postroutine(rpos_handle);
			if ( combobuf[cid].attribytes[2] > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
		}
		
		//loop next combo
		if((combobuf[cid].usrflags&cflag4))
		{
			do
			{
				if (rpos_handle.layer) 
				{
					screen_combo_modify_preroutine(rpos_handle);
					
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						screen->data[pos] = screen->undercombo;
						screen->cset[pos] = screen->undercset;
						screen->sflag[pos] = 0;	
					}
					else
						++screen->data[pos];
					
					screen_combo_modify_postroutine(rpos_handle);
				}
				else
				{
					screen_combo_modify_preroutine(rpos_handle);
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						screen->data[pos] = screen->undercombo;
						screen->cset[pos] = screen->undercset;
						screen->sflag[pos] = 0;	
					}
					else
					{
						screen->data[pos]=vbound(rpos_handle.data()+1,0,MAXCOMBOS);
					}
					screen_combo_modify_postroutine(rpos_handle);
				}
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				
				if ( (combobuf[cid].usrflags&cflag5) ) cid = rpos_handle.data();
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(rpos_handle.screen_index,x,y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
		}
	}
}

void do_generic_combo_ffc2(const ffc_handle_t& ffc_handle, int32_t cid, int32_t ft)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	} 
	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	ffcdata* ffc = ffc_handle.ffc;
	mapscr* screen = ffc_handle.screen;
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
						decorations.add(new dBushLeaves(ffc->x, ffc->y, dBUSHLEAVES, 0, 0));
						break;
					case 2:
						decorations.add(new dFlowerClippings(ffc->x, ffc->y, dFLOWERCLIPPINGS, 0, 0));
						break;
					case 3:
						decorations.add(new dGrassClippings(ffc->x, ffc->y, dGRASSCLIPPINGS, 0, 0));
						break;
				}
			}
			else decorations.add(new comboSprite(ffc->x, ffc->y, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
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
			item* itm = (new item(ffc->x, ffc->y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(mSPECIALITEM))
		{
			items.add(new item(ffc->x, ffc->y,(zfix)0,
				screen->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[screen->catchall].family==itype_triforcepiece ||
				(screen->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((screen->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_ffc_modify_preroutine(ffc_handle);
			ffc->setData(screen->secretcombo[ft]);
			ffc->cset = screen->secretcset[ft];
			// newflag = s->secretflag[ft];
			screen_ffc_modify_postroutine(ffc_handle);
			if ( combobuf[cid].attribytes[2] > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc->x));
		}
		
		//loop next combo
		if((combobuf[cid].usrflags&cflag4))
		{
			do
			{
				screen_ffc_modify_preroutine(ffc_handle);
				//undercombo or next?
				if((combobuf[cid].usrflags&cflag12))
				{
					ffc->setData(screen->undercombo);
					ffc->cset = screen->undercset;
				}
				else
				{
					ffc->setData(vbound(ffc->getData()+1,0,MAXCOMBOS));
				}
				screen_ffc_modify_postroutine(ffc_handle);
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ffc->getData();
				
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc->x));
			
			
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(ffc_handle.screen_index,ffc->x,ffc->y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
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

static void trigger_cswitch_block(const rpos_handle_t& rpos_handle)
{
	if(unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return;

	int pos = rpos_handle.pos();
	int cid = rpos_handle.data();
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	auto newcid = BOUND_COMBO(cid + cmbofs);
	rpos_handle.set_data(newcid);
	rpos_handle.set_cset((rpos_handle.cset() + csofs) & 15);
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
	}
	// TODO z3 !
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		if(lyr == rpos_handle.layer) continue;
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
		int bx = COMBOX_REGION(rpos_handle.rpos)+8, by = COMBOY_REGION(rpos_handle.rpos)+8;
		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by))
			{
				ffcdata* ffc = ffc_handle.ffc;
				ffc->setData(BOUND_COMBO(ffc->getData() + cmbofs));
				ffc->cset = (ffc->cset + csofs) & 15;
				int32_t newcid2 = ffc->getData();
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
				}
			}
		});
	}
}

static void trigger_cswitch_block_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	auto cid = ffc->getData();
	newcombo const& cmb = combobuf[cid];
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	ffc->setData(BOUND_COMBO(cid + cmbofs));
	ffc->cset = (ffc->cset + csofs) & 15;
	auto newcid = ffc->getData();
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
	}
	int32_t pos2 = COMBOPOS(ffc->x+8, ffc->y+8);
	for(auto lyr = 0; lyr < 7; ++lyr)
	{
		if(!(cmb.usrflags&(1<<lyr))) continue;

		mapscr* scr_2 = get_layer_scr(currmap, ffc_handle.screen_index, lyr);
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
		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle_2) {
			if (&ffc_handle_2.ffc == &ffc_handle.ffc) return;

			if (ffcIsAt(ffc_handle_2, ffc->x+8, ffc->y+8))
			{
				ffcdata* ffc2 = ffc_handle_2.ffc;
				ffc2->setData(BOUND_COMBO(ffc2->getData() + cmbofs));
				ffc2->cset = (ffc2->cset + csofs) & 15;
				int32_t newcid2 = ffc2->getData();
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
				}
			}
		});
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

void spawn_decoration(newcombo const& cmb, const rpos_handle_t& rpos_handle)
{
	if (!is_valid_rpos(rpos_handle.rpos)) return;

	int x, y;
	COMBOXY_REGION(rpos_handle.rpos, x, y);
	spawn_decoration_xy(cmb, x, y);
}

void trigger_cuttable(const rpos_handle_t& rpos_handle)
{
	if(unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return;

	int pos = rpos_handle.pos();
	mapscr* tmp = rpos_handle.screen;
	newcombo const& cmb = combobuf[rpos_handle.data()];
	auto type = cmb.type;
	if(!isCuttableType(type)) return;
	auto flag = tmp->sflag[pos];
	auto flag2 = cmb.flag;

	int32_t x, y;
	COMBOXY_REGION(rpos_handle.rpos, x, y);
	
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
			sfx(tmpscr->secretsfx);
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
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((rpos_handle.screen_index < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmp->flags9&fBELOWRETURN)))
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
	spawn_decoration(cmb, rpos_handle);
}

void trigger_cuttable_ffc(const ffc_handle_t& ffc_handle)
{
	int pos = ffc_handle.i;
	if (ffc_handle.id > MAX_FFCID) return;
	ffcdata& ffc = *ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc.getData()];
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
			ffc.setData(ffc_handle.screen->secretcombo[(ffc_handle.screen->sflag[pos])-16+4]);
			ffc.cset = ffc_handle.screen->secretcset[(ffc_handle.screen->sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			ffc.setData(ffc_handle.screen->secretcombo[sSTAIRS]);
			ffc.cset = ffc_handle.screen->secretcset[sSTAIRS];
			sfx(ffc_handle.screen->secretsfx);
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
			ffc.setData(tmpscr->undercombo);
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

bool trigger_step(const rpos_handle_t& rpos_handle)
{
	// TODO z3 this is repeated lots of places.
	if(unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;

	int32_t pos = rpos_handle.pos();
	newcombo const& cmb = combobuf[rpos_handle.data()];
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
			++rpos_handle.screen->data[pos]; break;
		case cSTEPSAME:
		{
			// Increment all combos of the same id as the triggered combo on the base screen.
			// If the trigger is on a layer screen, that will be the only combo on that layer incremented.
			int32_t id = rpos_handle.data();
			for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
				for (int q = 0; q < 176; ++q)
				{
					if (screen->data[q] == id)
					{
						++screen->data[q];
					}
				}
			});
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
					if (ffc_handle.data() == id)
					{
						ffc_handle.ffc->incData(1);
					}
				});
			}
			if (rpos_handle.layer > 0) ++rpos_handle.screen->data[pos];
			break;
		}
		case cSTEPALL:
		{
			for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
				for (int q = 0; q < 176; ++q)
				{
					if (isStepType(combobuf[screen->data[q]].type))
					{
						++screen->data[q];
					}
				}
			});
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
					if (isStepType(combobuf[ffc_handle.data()].type))
					{
						ffc_handle.ffc->incData(1);
					}
				});
			}
			if (rpos_handle.layer > 0) ++rpos_handle.screen->data[pos];
			break;
		}
	}
	return true;
}

bool trigger_step_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc_handle.data()];
	if(!isStepType(cmb.type) || cmb.type == cSTEPCOPY) return false;
	if(cmb.attribytes[1] && !game->item[cmb.attribytes[1]])
		return false; //lacking required item
	if((cmb.usrflags & cflag1) && !Hero.HasHeavyBoots())
		return false;
	if(cmb.attribytes[0])
		sfx(cmb.attribytes[0], pan(ffc->x));
	switch(cmb.type)
	{
		case cSTEP:
		{
			ffc_handle.increment_data();
			break;
		}
		case cSTEPSAME:
		{
			int32_t id = ffc->getData();
			for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
				if (rpos_handle.data() == id)
				{
					rpos_handle.increment_data();
				}
			});
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle_2) {
					if (ffc_handle_2.data() == id && ffc_handle_2.ffc != ffc_handle.ffc)
					{
						ffc_handle_2.increment_data();
					}
				});
			}
			ffc_handle.increment_data();
			break;
		}
		case cSTEPALL:
		{
			for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
				if (isStepType(combobuf[rpos_handle.data()].type))
				{
					rpos_handle.increment_data();
				}
			});
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle_2) {
					if (isStepType(combobuf[ffc_handle_2.data()].type) && ffc_handle_2.ffc != ffc_handle.ffc)
					{
						ffc_handle_2.increment_data();
					}
				});
			}
			ffc_handle.increment_data();
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
	mapscr* wscr = currscr >= 128 ? &special_warp_return_screen : tmpscr;
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

bool trigger_chest(const rpos_handle_t& rpos_handle)
{
	mapscr* base_screen = rpos_handle.layer == 0 ? rpos_handle.screen : get_scr(currmap, rpos_handle.screen_index);

	newcombo const& cmb = combobuf[rpos_handle.data()];
	switch(cmb.type)
	{
		case cLOCKEDCHEST: //Special flags!
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(rpos_handle.screen, rpos_handle.screen_index, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(rpos_handle.screen, rpos_handle.screen_index, mCHEST);
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
				FFCore.deallocateAllArrays(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(rpos_handle.screen, rpos_handle.screen_index, mBOSSCHEST);
			break;
	}
	
	sfx(cmb.attribytes[3]); //opening sfx
	
	bool itemflag = false;
	for(int32_t i=0; i<3; i++)
	{
		mapscr* layer_scr = get_layer_scr(currmap, rpos_handle.screen_index, i - 1);
		if(layer_scr->sflag[rpos_handle.pos()]==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
		if(combobuf[layer_scr->data[rpos_handle.pos()]].flag==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
	}
	bool itemstate = false;
	int32_t ipflag = 0;
	if(cmb.usrflags & cflag7)
	{
		itemstate = getmapflag((rpos_handle.screen_index < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (rpos_handle.screen_index < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
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
				int32_t di = ((currdmap)<<7) + rpos_handle.screen_index-(DMaps[currdmap].type==dmOVERW ? 0 : DMaps[currdmap].xoff);
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

bool trigger_chest_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc->getData()];
	int32_t cid = ffc->getData();
	switch(cmb.type)
	{
		case cLOCKEDCHEST: //Special flags!
			if(!try_locked_combo(cmb))
			{
				play_combo_string(cmb.attributes[3]/10000L);
				return false;
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(ffc_handle.screen, ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.screen, ffc_handle.screen_index, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(ffc_handle.screen, ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.screen, ffc_handle.screen_index, mCHEST);
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
				FFCore.deallocateAllArrays(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(ffc_handle.screen, ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.screen, ffc_handle.screen_index, mBOSSCHEST);
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
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((ffc_handle.screen->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((get_currdmap())<<7) + ffc_handle.screen_index-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
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

bool trigger_lockblock(const rpos_handle_t& rpos_handle)
{
	DCHECK(rpos_handle.rpos <= region_max_rpos);
	
	newcombo const& cmb = combobuf[rpos_handle.data()];
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
				setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(rpos_handle.screen, rpos_handle.screen_index, mLOCKBLOCK);
			remove_lockblocks(rpos_handle.screen, rpos_handle.screen_index);
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
				FFCore.deallocateAllArrays(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(rpos_handle.screen, rpos_handle.screen_index, mBOSSLOCKBLOCK);
			remove_bosslockblocks(rpos_handle.screen, rpos_handle.screen_index);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}

bool trigger_lockblock_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc->getData()];
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
				setxmapflag(ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(ffc_handle.screen, ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.screen, ffc_handle.screen_index, mLOCKBLOCK);
			remove_lockblocks(ffc_handle.screen, ffc_handle.screen_index);
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
				FFCore.deallocateAllArrays(ScriptType::Item, key_item);
			}
			
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				remove_xstatecombos(ffc_handle.screen, ffc_handle.screen_index, 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.screen, ffc_handle.screen_index, mBOSSLOCKBLOCK);
			remove_bosslockblocks(ffc_handle.screen, ffc_handle.screen_index);
			break;
		}
		default: return false;
	}
	
	if(cmb.attribytes[3])
		sfx(cmb.attribytes[3]); //opening sfx
	return true;
}


bool trigger_armos_grave(const rpos_handle_t& rpos_handle, int32_t trigdir)
{
	if (rpos_handle.layer != 0) return false; // Currently cannot activate on layers >0!
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;
	
	int pos = rpos_handle.pos();
	//!TODO Expand 'activation_counters' stuff to account for layers, so that layers >0 can be used
	if (activation_counters[pos]) return false;
	int32_t gc = 0;
	for(int32_t i=0; i<guys.Count(); ++i)
	{
		if(((enemy*)guys.spr(i))->mainguy)
		{
			++gc;
		}
	}
	if(gc > 10) return false; //Unsure what this purpose is
	mapscr* tmp = rpos_handle.screen;
	newcombo const& cmb = combobuf[tmp->data[pos]];
	int32_t eclk = -14;
	int32_t id2 = 0;
	int32_t tx, ty;
	COMBOXY_REGION(rpos_handle.rpos, tx, ty);
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
				activation_counters[pos] = 61;
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
				// TODO z3 ?
				for (int32_t n = 0; n < armosysz && id3 < 176; n++)
				{
					for (int32_t m = 0; m < armosxsz && id3 < 176; m++) 
					{
						if (id3 + m < 176)
							activation_counters[(id3+m)]=61;
					}
					id3+=16;
				}
				if (guysbuf[id2].family == eeGHOMA) 
				{
					if ( combobuf[(tmpscr->data[pos-chx+1])].type == cARMOS ) xpos += 16;
				}
				if(addenemy(rpos_handle.screen_index,tx+xpos,ty+1+ypos,id2,0))
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
	activation_counters[pos] = 61;
	if(addenemy(rpos_handle.screen_index,tx,ty+3,id2,eclk))
		((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
	else return false;
	return true;
}

bool trigger_armos_grave_ffc(const ffc_handle_t& ffc_handle, int32_t trigdir)
{
	if (activation_counters_ffc[ffc_handle.id]) return false; //Currently activating

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
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc->getData()];
	int32_t eclk = -14;
	int32_t id2 = 0;
	int32_t tx = ffc->x, ty = ffc->y;
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
				ffc->incData(1);
			break;
		}
		default: return false;
	}
	activation_counters_ffc[ffc_handle.id] = 61;
	if(addenemy(ffc_handle.screen_index,tx,ty+3,id2,eclk))
	{
		((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
		((enemy*)guys.spr(guys.Count()-1))->ffcactivated=ffc_handle.id+1;
	}
	else return false;
	return true;
}

bool trigger_damage_combo(const rpos_handle_t& rpos_handle, int type, int ptrval, int32_t hdir, bool force_solid)
{
	return trigger_damage_combo(rpos_handle.screen, rpos_handle.data(), type, ptrval, hdir, force_solid);
}

bool trigger_damage_combo(mapscr* screen, int32_t cid, int type, int ptrval, int32_t hdir, bool force_solid)
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

	bool current_ring = (screen->flags6&fTOGGLERINGDAMAGE) != 0;
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

bool trigger_stepfx(const rpos_handle_t& rpos_handle, bool stepped)
{
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;

	int32_t tx, ty;
	COMBOXY_REGION(rpos_handle.rpos, tx, ty);
	tx += 8;
	ty += 8;

	int pos = rpos_handle.pos();
	newcombo const& cmb = combobuf[rpos_handle.data()];

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
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wFire:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
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
			rpos_handle.screen->data[pos]++;
		}
	}
	return true;
}

bool trigger_stepfx_ffc(const ffc_handle_t& ffc_handle, bool stepped)
{
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc->getData()];
	int32_t tx = ffc->x, ty = ffc->y;
	int32_t thesfx = cmb.attribytes[0];
	sfx_no_repeat(thesfx, pan(ffc->x));
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
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0)); 
				if (cmb.attribytes[3] > 0 )
				{
					weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //last created
					w->LOADGFX(cmb.attribytes[3]);
				}
				break;
			
			case wFire:
				Lwpns.add(new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
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
			ffc->incData(1);
		}
	}
	return true;
}


bool trigger_switchhookblock(const rpos_handle_t& rpos_handle)
{
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;
	if(Hero.switchhookclk) return false;

	switching_object = NULL;
	hooked_comborpos = rpos_handle.rpos;
	hooked_layerbits = 0;
	Hero.doSwitchHook(game->get_switchhookstyle());
	if(!hooked_layerbits) //failed
	{
		Hero.reset_hookshot();
		return false;
	}
	return true;
}

static bool trigger_switchhookblock_ffc(const ffc_handle_t& ffc_handle)
{
	if(Hero.switchhookclk) return false;
	switching_object = ffc_handle.ffc;
	switching_object->switch_hooked = true;
	hooked_comborpos = rpos_t::None;
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

void trigger_save(newcombo const& cmb, mapscr* screen)
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
	save_game((screen->flags4&fSAVEROOM) != 0, save_type);
}

static byte copycat_id = 0;
static bool do_copycat_trigger(const rpos_handle_t& rpos_handle)
{
	if(!copycat_id) return false;
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;
	int pos = rpos_handle.pos();
	
	if (unsigned(pos) > 175) return false;
	int32_t cid = rpos_handle.data();
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo(rpos_handle.layer, pos);
		return true;
	}
	return false;
}

static bool do_copycat_trigger_ffc(const ffc_handle_t& ffc_handle)
{
	if(!copycat_id) return false;

	int32_t cid = ffc_handle.data();
	newcombo const& cmb = combobuf[cid];
	if(cmb.trigcopycat == copycat_id)
	{
		do_trigger_combo_ffc(ffc_handle);
		return true;
	}
	return false;
}

void do_ex_trigger(const rpos_handle_t& rpos_handle)
{
	if (!is_valid_rpos(rpos_handle.rpos)) return;

	int32_t cid = rpos_handle.data();
	int32_t ocs = rpos_handle.cset();
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		rpos_handle.set_data(cid+cmb.trigchange);
	}
	if(cmb.trigcschange)
	{
		rpos_handle.set_cset((ocs+cmb.trigcschange) & 0xF);
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[rpos_handle.data()];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
	{
		if(!copycat_id) //not already in a copycat
		{
			bool skipself = rpos_handle.data() == cid;
			copycat_id = cmb.trigcopycat;
			for_every_rpos_in_region([&](const rpos_handle_t& cc_rpos_handle) {
				if (skipself && cc_rpos_handle.screen_index == rpos_handle.screen_index && cc_rpos_handle.layer == rpos_handle.layer && cc_rpos_handle.rpos == rpos_handle.rpos) return;
				do_copycat_trigger(cc_rpos_handle);
			});
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region(do_copycat_trigger_ffc);
			}
			copycat_id = 0;
		}
	}
}

void do_ex_trigger_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	int32_t cid = ffc->getData();
	int32_t ocs = ffc->cset;
	newcombo const& cmb = combobuf[cid];	
	if(cmb.trigchange)
	{
		ffc->setData(cid+cmb.trigchange);
	}
	if(cmb.trigcschange)
	{
		ffc->cset = (ocs+cmb.trigcschange) & 0xF;
	}
	if(cmb.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[ffc->getData()];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
	}
	
	if(cmb.trigcopycat) //has a copycat set
	{
		if(!copycat_id) //not already in a copycat
		{
			bool skipself = ffc->getData() == cid;
			copycat_id = cmb.trigcopycat;
			for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
				do_copycat_trigger(rpos_handle);
			});
			copycat_id = 0;
			if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
			{
				for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle_2) {
					if (skipself && &ffc_handle.ffc == &ffc_handle_2.ffc)
						return;
					do_copycat_trigger_ffc(ffc_handle_2);
				});
			}
		}
	}
}

bool force_ex_trigger(const rpos_handle_t& rpos_handle, char xstate)
{
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;
	newcombo const& cmb = combobuf[rpos_handle.data()];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(rpos_handle.screen_index, 1<<cmb.exstate))
		{
			do_ex_trigger(rpos_handle);
			return true;
		}
	}
	return false;
}

bool force_ex_trigger_ffc(const ffc_handle_t& ffc_handle, char xstate)
{
	ffcdata* ffc = ffc_handle.ffc;
	newcombo const& cmb = combobuf[ffc->getData()];	
	if(cmb.exstate > -1 && (xstate < 0 || xstate == cmb.exstate))
	{
		if(xstate >= 0 || getxmapflag(ffc_handle.screen_index, 1<<cmb.exstate))
		{
			do_ex_trigger_ffc(ffc_handle);
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

// TODO z3 remove
bool do_trigger_combo(int layer, int pos, int32_t special, weapon* w)
{
	if (unsigned(pos) > 175) return false;
	return do_trigger_combo(get_rpos_handle((rpos_t)pos, layer), special, w);
}

// Triggers a combo at a given position.
bool do_trigger_combo(const rpos_handle_t& rpos_handle, int32_t special, weapon* w)
{
	if (unsigned(rpos_handle.layer) > 6 || !is_valid_rpos(rpos_handle.rpos)) return false;

	int32_t pos = rpos_handle.pos();
	cpos_info& timer = get_combo_posinfo(rpos_handle);
	int32_t cid = rpos_handle.data();
	int32_t cx, cy;
	COMBOXY_REGION(rpos_handle.rpos, cx, cy);

	int32_t ocs = rpos_handle.cset();
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	if(w && (cmb.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,cmb);
		return true;
	}
	
	word ctramnt = game->get_counter(cmb.trigctr);
	bool onlytrigctr = !(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG);
	
	int32_t flag = rpos_handle.sflag();
	int32_t flag2 = cmb.flag;
	
	bool check_bit = false;
	bool used_bit = false;
	
	uint32_t exflag = 0;
	
	if(cmb.exstate > -1)
	{
		exflag = 1<<cmb.exstate;
		if (force_ex_trigger(rpos_handle))
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
	
	if (w)
	{
		check_bit = w->rposes_checked.contains({rpos_handle.layer, rpos_handle.rpos});
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
						do_generic_combo(rpos_handle, w, (w->useweapon > 0) ? w->useweapon : w->id, cid, flag, flag2, cmb.attribytes[3], false);
					else do_generic_combo2(cx, cy, cid, flag, flag2, cmb.attribytes[3], rpos_handle, false);
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
						trigger_cswitch_block(rpos_handle);
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
						trigger_cuttable(rpos_handle);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step(rpos_handle))
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
						if(!trigger_chest(rpos_handle))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock(rpos_handle))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave(rpos_handle))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(rpos_handle, ZSD_COMBOPOS, (int)rpos_handle.rpos*10000);
						break;
					
					case cSTEPSFX:
						trigger_stepfx(rpos_handle);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock(rpos_handle))
							return false;
						break;
					
					case cSHOOTER:
						if(!trigger_shooter(cmb,pos))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb, rpos_handle.screen);
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
					trigger_secrets_for_screen(TriggerSource::GenericCombo, false);
					triggering_generic_secrets = false;
					if(rpos_handle.screen->secretsfx)
						sfx(rpos_handle.screen->secretsfx);
				}
				if(canPermSecret(currdmap, rpos_handle.screen_index) && !(rpos_handle.screen->flags5&fTEMPSECRETS) && !getmapflag(rpos_handle.screen_index, mSECRET))
					setmapflag(rpos_handle.screen, rpos_handle.screen_index, mSECRET);
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
				rpos_handle.set_data(cid+cmb.trigchange);
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				rpos_handle.set_cset((ocs+cmb.trigcschange) & 0xF);
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[rpos_handle.data()];
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
				word numcreated = addenemy(rpos_handle.screen_index, cx, cy, cmb.spawnenemy, -10);
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
				setxmapflag(rpos_handle.screen_index, exflag);
			}
			
			if(cmb.trigcopycat) //has a copycat set
			{
				if(!copycat_id) //not already in a copycat
				{
					bool skipself = rpos_handle.data() == cid;
					copycat_id = cmb.trigcopycat;
					for_every_rpos_in_region([&](const rpos_handle_t& cc_rpos_handle) {
						if (skipself && cc_rpos_handle.layer == rpos_handle.layer && cc_rpos_handle.rpos == rpos_handle.rpos) return;
						do_copycat_trigger(cc_rpos_handle);
					});
					if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
					{
						for_every_ffc_in_region(do_copycat_trigger_ffc);
					}
					copycat_id = 0;
				}
			}
			
			update_trig_group(timer.data, rpos_handle.data());
			timer.updateData(rpos_handle.data());
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if (w && used_bit)
		{
			w->rposes_checked.insert({rpos_handle.layer, rpos_handle.rpos});
		}
	}
	
	if(w)
		do_weapon_fx(w,cmb);
	
	if(dorun && cmb.trig_genscr)
		FFCore.runGenericFrozenEngine(cmb.trig_genscr);
	return true;
}

bool do_trigger_combo_ffc(const ffc_handle_t& ffc_handle, int32_t special, weapon* w)
{
	if (get_qr(qr_OLD_FFC_FUNCTIONALITY)) return false;

	ffcdata* ffc = ffc_handle.ffc;
	int32_t cid = ffc->getData();
	cpos_info& timer = ffc->info;
	int32_t ocs = ffc->cset;
	int32_t cx = ffc->x;
	int32_t cy = ffc->y;
	newcombo const& cmb = combobuf[cid];
	bool hasitem = false;
	if(w && (cmb.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,cmb);
		return true;
	}
	
	word ctramnt = game->get_counter(cmb.trigctr);
	bool onlytrigctr = !(cmb.triggerflags[1] & combotriggerCTRNONLYTRIG);
	
	int32_t flag2 = cmb.flag;
	
	bool check_bit = false;
	bool used_bit = false;
	
	uint32_t exflag = 0;
	if(cmb.exstate > -1)
	{
		exflag = 1<<cmb.exstate;
		if(force_ex_trigger_ffc(ffc_handle))
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
		check_bit = w->ffcs_checked.contains(ffc_handle.ffc);
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
						do_generic_combo_ffc(w, ffc_handle, cid, flag2);
					else do_generic_combo_ffc2(ffc_handle, cid, flag2);
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
						trigger_cswitch_block_ffc(ffc_handle);
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
						trigger_cuttable_ffc(ffc_handle);
						break;
						
					case cSTEP: case cSTEPSAME: case cSTEPALL:
						if(!trigger_step_ffc(ffc_handle))
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
						if(!trigger_chest_ffc(ffc_handle))
							return false;
						break;
					case cLOCKBLOCK: case cBOSSLOCKBLOCK:
						if(!trigger_lockblock_ffc(ffc_handle))
							return false;
						break;
					
					case cARMOS: case cBSGRAVE: case cGRAVE:
						if(!trigger_armos_grave_ffc(ffc_handle))
							return false;
						break;
					
					case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
					case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
						trigger_damage_combo(ffc_handle.screen, cid, ZSD_FFC, ffc_handle.id);
						break;
					
					case cSTEPSFX:
						trigger_stepfx_ffc(ffc_handle);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock_ffc(ffc_handle))
							return false;
						break;
					
					case cSHOOTER:
						if(!trigger_shooter(cmb,cx,cy))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb, ffc_handle.screen);
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
					trigger_secrets_for_screen(TriggerSource::GenericCombo, ffc_handle.screen_index, false);
					triggering_generic_secrets = false;
				}
				if(canPermSecret(currdmap, ffc_handle.screen_index) && !(ffc_handle.screen->flags5&fTEMPSECRETS))
					setmapflag(ffc_handle.screen_index, mSECRET);
				sfx(ffc_handle.screen->secretsfx);
			}
			
			if(cmb.trigchange)
			{
				used_bit = true;
				ffc->setData(cid+cmb.trigchange);
			}
			if(cmb.trigcschange)
			{
				used_bit = true;
				ffc->cset = (ocs+cmb.trigcschange) & 0xF;
			}
			
			if(cmb.triggerflags[0] & combotriggerRESETANIM)
			{
				newcombo& rcmb = combobuf[ffc->getData()];
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
				word numcreated = addenemy(ffc_handle.screen_index, cx, cy, cmb.spawnenemy, -10);
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
				setxmapflag(ffc_handle.screen_index, exflag);
			}
			
			if(cmb.trigcopycat) //has a copycat set
			{
				if(!copycat_id) //not already in a copycat
				{
					bool skipself = ffc->getData() == cid;
					copycat_id = cmb.trigcopycat;
					for(auto cclayer = 0; cclayer < 7; ++cclayer)
					{
						for(auto ccpos = 0; ccpos < 176; ++ccpos)
						{
							do_copycat_trigger(get_rpos_handle((rpos_t)ccpos, cclayer));
						}
					}
					if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
					{
						for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle_2) {
							if (skipself && &ffc_handle_2.ffc == &ffc_handle.ffc)
								return;

							do_copycat_trigger_ffc(ffc_handle_2);
						});
					}
					copycat_id = 0;
				}
			}
			
			update_trig_group(timer.data, ffc_handle.data());
			timer.updateData(ffc_handle.data());
			if(cmb.trigcooldown)
				timer.trig_cd = cmb.trigcooldown;
		}
		if (w && used_bit)
		{
			w->ffcs_checked.insert(ffc_handle.ffc);
		}
	}
	
	if(w)
		do_weapon_fx(w,cmb);
	
	if(dorun && cmb.trig_genscr)
		FFCore.runGenericFrozenEngine(cmb.trig_genscr);
	return true;
}


bool do_lift_combo(const rpos_handle_t& rpos_handle, int32_t gloveid)
{
	if(!Hero.can_lift(gloveid)) return false;
	if(Hero.lift_wpn) return false;

	int32_t cid = rpos_handle.data();
	int32_t cset = rpos_handle.cset();
	newcombo const& cmb = combobuf[cid];
	itemdata const& glove = itemsbuf[gloveid];

	if(cmb.liftlvl > glove.fam_type) return false;

	int32_t cx, cy;
	COMBOXY_REGION(rpos_handle.rpos, cx, cy);

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
	
	if(hasitem && (cmb.liftflags&LF_SPECIALITEM) && getmapflag(rpos_handle.screen_index, mSPECIALITEM))
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
	
	w->moveflags |= FLAG_OBEYS_GRAV;
	if(cmb.liftflags & LF_BREAKONSOLID)
		w->misc_wflags |= WFLAG_BREAK_ON_SOLID;
	
	w->death_sprite = cmb.liftbreaksprite;
	w->death_sfx = cmb.liftbreaksfx;
	
	Hero.lift(w, cmb.lifttime, cmb.lifthei);
	
	rpos_handle.set_data(cmb.liftundercmb);
	if(!(cmb.liftflags & LF_NOUCSET))
		rpos_handle.set_cset(cmb.liftundercs);
	rpos_handle.set_sflag(0);
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

void ffc_clear_cpos_info()
{
	for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
		ffc_handle.ffc->info.clear();
	});
}

void update_trig_group(int oldc, int newc)
{
	if(oldc == newc) return;
	newcombo const& newcmb = combobuf[newc];
	if(newcmb.triggerflags[3] & combotriggerTGROUP_CONTRIB)
		++trig_groups[newcmb.trig_group];
	newcombo const& oldcmb = combobuf[oldc];
	if(oldcmb.triggerflags[3] & combotriggerTGROUP_CONTRIB)
		--trig_groups[oldcmb.trig_group];
}

// void calculate_trig_groups()
// {

// }

void calculate_trig_groups()
{
	memset(trig_groups, 0, sizeof(trig_groups));

	for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
		cpos_info& timer = get_combo_posinfo(rpos_handle);
		int cid = rpos_handle.data();
		timer.updateData(cid);
		newcombo const& cmb = combobuf[cid];
		if(cmb.triggerflags[3] & combotriggerTGROUP_CONTRIB)
			++trig_groups[cmb.trig_group];
	});

	for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
		int cid = ffc_handle.data();
		ffc_handle.ffc->info.clear();
		ffc_handle.ffc->info.data = cid;
		newcombo const& cmb = combobuf[cid];
		if(cmb.triggerflags[3] & combotriggerTGROUP_CONTRIB)
			++trig_groups[cmb.trig_group];
	});
}

void trig_trigger_groups()
{
	ffc_clear_cpos_info();

	for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
		int cid = rpos_handle.data();
		cpos_info& timer = get_combo_posinfo(rpos_handle);
		const newcombo* cmb = &combobuf[cid];

		while (
			((cmb->triggerflags[3] & combotriggerTGROUP_LESS)
				&& trig_groups[cmb->trig_group] < cmb->trig_group_val)
			|| ((cmb->triggerflags[3] & combotriggerTGROUP_GREATER)
				&& trig_groups[cmb->trig_group] > cmb->trig_group_val)
			)
		{
			do_trigger_combo(rpos_handle);
			int cid2 = rpos_handle.data();
			// TODO z3 upstream ? https://discord.com/channels/876899628556091432/876908472728453161/1135099459005579375
			// update_trig_group(timer.data, cid2);
			// timer.updateData(cid2);

			if (cid != cid2)
			{
				cmb = &combobuf[cid2];
				cid = cid2;
			}
		}
	});

	for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
		int cid = ffc_handle.data();
		cpos_info& timer = ffc_handle.ffc->info;
		const newcombo* cmb = &combobuf[cid];
		
		while(
			((cmb->triggerflags[3] & combotriggerTGROUP_LESS)
				&& trig_groups[cmb->trig_group] < cmb->trig_group_val)
			|| ((cmb->triggerflags[3] & combotriggerTGROUP_GREATER)
				&& trig_groups[cmb->trig_group] > cmb->trig_group_val)
			)
		{
			do_trigger_combo_ffc(ffc_handle);
			int cid2 = ffc_handle.data();
			update_trig_group(cid,cid2);
			timer.updateData(cid2);
			cmb = &combobuf[cid2];
		}
	});
}

void init_combo_timers()
{
	// clear_combo_posinfo();
	ffc_clear_cpos_info();
}

void update_combo_timers()
{
	ffc_clear_cpos_info();

	for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
		cpos_info& timer = get_combo_posinfo(rpos_handle);
		int cid = rpos_handle.data();
		newcombo const& cmb = combobuf[cid];
		if(!timer.appeared)
		{
			int x, y;
			COMBOXY_REGION(rpos_handle.rpos, x, y);
			timer.appeared = true;
			if(cmb.sfx_appear)
				sfx(cmb.sfx_appear);
			if(cmb.spr_appear)
				decorations.add(new comboSprite(x, y, dCOMBOSPRITE, 0, cmb.spr_appear));
			if(timer.sfx_onchange) //last combo's sfx_disappear
				sfx(timer.sfx_onchange);
			if(timer.spr_onchange) //last combo's spr_disappear
				decorations.add(new comboSprite(x, y, dCOMBOSPRITE, 0, timer.spr_onchange));
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
				do_trigger_combo(rpos_handle);
				cid = rpos_handle.data();
				update_trig_group(timer.data,cid);
				timer.updateData(cid);
			}
		}
		if(timer.trig_cd) --timer.trig_cd;
		if(cmb.type == cSHOOTER)
			handle_shooter(cmb, timer, rpos_handle.rpos);
	});

	for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
		cpos_info& timer = ffc_handle.ffc->info;
		int cid = ffc_handle.data();
		update_trig_group(timer.data,cid);
		timer.updateData(cid);
		zfix wx = ffc_handle.ffc->x;
		zfix wy = ffc_handle.ffc->y;
		wx += (ffc_handle.screen->ffTileWidth(ffc_handle.i)-1)*8;
		wy += (ffc_handle.screen->ffTileHeight(ffc_handle.i)-1)*8;
		
		newcombo const& cmb = combobuf[cid];
		if(cmb.trigtimer)
		{
			if(++timer.clk >= cmb.trigtimer)
			{
				timer.clk = 0;
				do_trigger_combo_ffc(ffc_handle);
				cid = ffc_handle.data();
				update_trig_group(timer.data,cid);
				timer.updateData(cid);
			}
		}
		if(timer.trig_cd) --timer.trig_cd;
		if(cmb.type == cSHOOTER)
			handle_shooter(cmb, timer, wx, wy);
	});

	trig_trigger_groups();
}

bool on_cooldown(const rpos_handle_t& rpos_handle)
{
	int pos = rpos_handle.pos();
	if(unsigned(rpos_handle.layer) > 7 || unsigned(pos) > 176)
		return false;
	return get_combo_posinfo(rpos_handle).trig_cd != 0;
}
