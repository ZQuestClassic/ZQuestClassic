#include "base/handles.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "base/zdefs.h"
#include "ffc.h"
#include "zc/zc_ffc.h"
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
#include "zscriptversion.h"

extern sprite_list items, decorations;

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

static void do_generic_combo2(int32_t bx, int32_t by, int32_t cid, int32_t flag, int32_t flag2, int32_t ft, const rpos_handle_t& rpos_handle, bool single16)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	}

	mapscr* scr = rpos_handle.scr;
	int pos = rpos_handle.pos;
	auto [x, y] = rpos_handle.xy();

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
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(scr, mSPECIALITEM))
		{
			items.add(new item(x, y, 0,
				scr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[scr->catchall].family==itype_triforcepiece ||
				(scr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_combo_modify_preroutine(rpos_handle);
			scr->data[pos] = scr->secretcombo[ft];
			scr->cset[pos] = scr->secretcset[ft];
			scr->sflag[pos] = scr->secretflag[ft];
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
						scr->data[pos] = scr->undercombo;
						scr->cset[pos] = scr->undercset;
						scr->sflag[pos] = 0;	
					}
					else
						++scr->data[pos];
					
					screen_combo_modify_postroutine(rpos_handle);
				}
				else
				{
					screen_combo_modify_preroutine(rpos_handle);
					//undercombo or next?
					if((combobuf[cid].usrflags&cflag12))
					{
						scr->data[pos] = scr->undercombo;
						scr->cset[pos] = scr->undercset;
						scr->sflag[pos] = 0;	
					}
					else
					{
						scr->data[pos]=vbound(rpos_handle.data()+1,0,MAXCOMBOS);
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
			addenemy(rpos_handle.screen,x,y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
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
	mapscr* scr = ffc_handle.scr;
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
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(scr, mSPECIALITEM))
		{
			items.add(new item(ffc->x, ffc->y,(zfix)0,
				scr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[scr->catchall].family==itype_triforcepiece ||
				(scr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_ffc_modify_preroutine(ffc_handle);
			ffc_handle.set_data(scr->secretcombo[ft]);
			ffc->cset = scr->secretcset[ft];
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
					ffc_handle.set_data(scr->undercombo);
					ffc->cset = scr->undercset;
				}
				else
				{
					ffc_handle.increment_data();
				}
				screen_ffc_modify_postroutine(ffc_handle);
				
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ffc_handle.data();
				
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc->x));
			
			
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(ffc_handle.screen,ffc->x,ffc->y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
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
	int pos = rpos_handle.pos;
	if (rpos_handle.ctype() != cCSWITCHBLOCK) return;

	auto& cmb = rpos_handle.combo();
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	rpos_handle.modify_data(cmbofs);
	rpos_handle.set_cset((rpos_handle.cset() + csofs) & 15);

	int newcid = rpos_handle.data();
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
		combo_caches::drawing.refresh(newcid);
	}
	for (int lyr = 0; lyr <= 6; lyr++)
	{
		if(lyr == rpos_handle.layer) continue;
		if(!(cmb.usrflags&(1<<lyr))) continue;

		auto rpos_handle_2 = get_rpos_handle(rpos_handle.rpos, lyr);
		int cid = rpos_handle_2.data();
		mapscr* scr_2 = rpos_handle_2.scr;
		if (!cid) //Don't increment empty space
			continue;
		
		rpos_handle_2.modify_data(cmbofs);
		int newcid2 = rpos_handle_2.data();
		scr_2->cset[pos] = (scr_2->cset[pos] + csofs) & 15;
		if(combobuf[newcid2].animflags & AF_CYCLE)
		{
			combobuf[newcid2].tile = combobuf[newcid2].o_tile;
			combobuf[newcid2].cur_frame=0;
			combobuf[newcid2].aclk = 0;
			combo_caches::drawing.refresh(newcid2);
		}
	}
	if (cmb.usrflags&cflag11)
	{
		auto [bx, by] = rpos_handle.xy();
		bx += 8;
		by += 8;

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by))
			{
				ffcdata* ffc = ffc_handle.ffc;
				ffc_handle.modify_data(cmbofs);
				ffc->cset = (ffc->cset + csofs) & 15;
				int32_t newcid2 = ffc_handle.data();
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
					combo_caches::drawing.refresh(newcid2);
				}
			}
		});
	}
}

static void trigger_cswitch_block_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	auto& cmb = ffc_handle.combo();
	if(cmb.type != cCSWITCHBLOCK) return;
	
	int32_t cmbofs = (cmb.attributes[0]/10000L);
	int32_t csofs = (cmb.attributes[1]/10000L);
	ffc_handle.modify_data(cmbofs);
	ffc->cset = (ffc->cset + csofs) & 15;
	auto newcid = ffc_handle.data();
	if(combobuf[newcid].animflags & AF_CYCLE)
	{
		combobuf[newcid].tile = combobuf[newcid].o_tile;
		combobuf[newcid].cur_frame=0;
		combobuf[newcid].aclk = 0;
		combo_caches::drawing.refresh(newcid);
	}

	rpos_t rpos = COMBOPOS_REGION_B(ffc->x+8, ffc->y+8);
	for (int lyr = 0; lyr <= 6; lyr++)
	{
		if (rpos == rpos_t::None) break;
		if(!(cmb.usrflags&(1<<lyr))) continue;

		auto rpos_handle = get_rpos_handle(rpos, lyr + 1);
		if (!rpos_handle.scr->is_valid())
			continue;
		if(!rpos_handle.data()) //Don't increment empty space
			continue;

		rpos_handle.modify_data(cmbofs);
		rpos_handle.set_cset((rpos_handle.cset() + csofs) & 15);
		int32_t newcid2 = rpos_handle.data();
		if(combobuf[newcid2].animflags & AF_CYCLE)
		{
			combobuf[newcid2].tile = combobuf[newcid2].o_tile;
			combobuf[newcid2].cur_frame=0;
			combobuf[newcid2].aclk = 0;
			combo_caches::drawing.refresh(newcid2);
		}
	}
	if (cmb.usrflags&cflag11)
	{
		for_every_ffc([&](const ffc_handle_t& ffc_handle_2) {
			if (&ffc_handle_2.ffc == &ffc_handle.ffc) return;

			if (ffcIsAt(ffc_handle_2, ffc->x+8, ffc->y+8))
			{
				ffcdata* ffc2 = ffc_handle_2.ffc;
				ffc_handle_2.modify_data(cmbofs);
				ffc2->cset = (ffc2->cset + csofs) & 15;
				int32_t newcid2 = ffc_handle_2.data();
				if(combobuf[newcid2].animflags & AF_CYCLE)
				{
					combobuf[newcid2].tile = combobuf[newcid2].o_tile;
					combobuf[newcid2].cur_frame=0;
					combobuf[newcid2].aclk = 0;
					combo_caches::drawing.refresh(newcid2);
				}
			}
		});
	}
}

void spawn_decoration_xy(newcombo const& cmb, zfix x, zfix y, zfix cx, zfix cy)
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
			decorations.add(new comboSprite(cx-8, cy-8, dCOMBOSPRITE, 0, cmb.attribytes[0]));
			break;
		case 1: decorations.add(new dBushLeaves(x, y, dBUSHLEAVES, 0, 0)); break;
		case 2: decorations.add(new dFlowerClippings(x, y, dFLOWERCLIPPINGS, 0, 0)); break;
		case 3: decorations.add(new dGrassClippings(x, y, dGRASSCLIPPINGS, 0, 0)); break;
	}
}

void spawn_decoration(newcombo const& cmb, const rpos_handle_t& rpos_handle)
{
	auto [x, y] = rpos_handle.xy();
	spawn_decoration_xy(cmb, x, y, x + 8, y + 8);
}

void trigger_cuttable(const rpos_handle_t& rpos_handle)
{
	int pos = rpos_handle.pos;
	mapscr* scr = rpos_handle.scr;
	auto& cmb = rpos_handle.combo();	
	auto type = cmb.type;
	if(!isCuttableType(type)) return;
	auto flag = scr->sflag[pos];
	auto flag2 = cmb.flag;
	auto [x, y] = rpos_handle.xy();
	
	bool skipSecrets = isNextType(type) && !get_qr(qr_OLD_SLASHNEXT_SECRETS);
	bool done = false;
	if(!skipSecrets)
	{
		done = true;
		if((flag >= 16 && flag <= 31))
		{  
			scr->data[pos] = scr->secretcombo[flag-16+4];
			scr->cset[pos] = scr->secretcset[flag-16+4];
			scr->sflag[pos] = scr->secretflag[flag-16+4];
		}
		else if(flag == mfARMOS_SECRET)
		{
			scr->data[pos] = scr->secretcombo[sSTAIRS];
			scr->cset[pos] = scr->secretcset[sSTAIRS];
			scr->sflag[pos] = scr->secretflag[sSTAIRS];
			sfx(scr->secretsfx);
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
			scr->data[pos] = scr->secretcombo[(scr->sflag[pos])-16+4];
			scr->cset[pos] = scr->secretcset[(scr->sflag[pos])-16+4];
			scr->sflag[pos] = scr->secretflag[(scr->sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			scr->data[pos] = scr->secretcombo[sSTAIRS];
			scr->cset[pos] = scr->secretcset[sSTAIRS];
			scr->sflag[pos] = scr->secretflag[sSTAIRS];
			sfx(scr->secretsfx);
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
			scr->data[pos]++;
		}
		else
		{
			scr->data[pos] = scr->undercombo;
			scr->cset[pos] = scr->undercset;
			scr->sflag[pos] = 0;
		}
	}
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(scr, (rpos_handle.screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (scr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, scr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(scr->secretsfx);
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
	mapscr* scr = ffc_handle.scr;
	auto& cmb = ffc_handle.combo();
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
			ffc_handle.set_data(ffc_handle.scr->secretcombo[(ffc_handle.scr->sflag[pos])-16+4]);
			ffc.cset = ffc_handle.scr->secretcset[(ffc_handle.scr->sflag[pos])-16+4];
		}
		else if(flag2 == mfARMOS_SECRET)
		{
			ffc_handle.set_data(ffc_handle.scr->secretcombo[sSTAIRS]);
			ffc.cset = ffc_handle.scr->secretcset[sSTAIRS];
			sfx(ffc_handle.scr->secretsfx);
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
			zc_ffc_modify(ffc, 1);
		}
		else
		{
			zc_ffc_set(ffc, scr->undercombo);
			ffc.cset = scr->undercset;
		}
	}
	
	if((flag2==mfARMOS_ITEM) && (!getmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (scr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)x, (zfix)y,(zfix)0, scr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(scr->secretsfx);
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
	spawn_decoration_xy(cmb, x, y, ffc.x+(ffc.hit_width / 2), ffc.y+(ffc.hit_height / 2));
}

bool trigger_step(const rpos_handle_t& rpos_handle)
{
	auto& cmb = rpos_handle.combo();	
	if(!isStepType(cmb.type) || cmb.type == cSTEPCOPY) return false;
	if(cmb.attribytes[1] && !game->item[cmb.attribytes[1]])
		return false; //lacking required item
	if((cmb.usrflags & cflag1) && !Hero.HasHeavyBoots())
		return false;
	if(cmb.attribytes[0])
		sfx(cmb.attribytes[0], pan(COMBOX_REGION(rpos_handle.rpos)));
	switch(cmb.type)
	{
		case cSTEP:
			rpos_handle.increment_data();
			break;
		case cSTEPSAME:
		{
			// Increment all combos of the same id as the triggered combo on the base screen.
			// If the trigger is on a layer screen, that will be the only combo on that layer incremented.
			int32_t id = rpos_handle.data();
			every_combo_opts opts{};
			opts.include_rposes_base_screen_only = true;
			for_every_combo([&](const auto& handle) {
				if (handle.data() == id)
					handle.increment_data();
			}, opts);

			if (rpos_handle.layer > 0) rpos_handle.increment_data();
			break;
		}
		case cSTEPALL:
		{
			every_combo_opts opts{};
			opts.include_rposes_base_screen_only = true;
			for_every_combo([&](const auto& handle) {
				if (isStepType(handle.ctype()))
					handle.increment_data();
			}, opts);

			if (rpos_handle.layer > 0) rpos_handle.increment_data();
			break;
		}
	}
	return true;
}

bool trigger_step_ffc(const ffc_handle_t& ffc_handle)
{
	ffcdata* ffc = ffc_handle.ffc;
	auto& cmb = ffc_handle.combo();
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
			int32_t id = ffc_handle.data();
			every_combo_opts opts{};
			opts.include_rposes_base_screen_only = true;
			for_every_combo([&](const auto& handle) {
				if (handle.data() == id)
					handle.increment_data();
			}, opts);

			ffc_handle.increment_data();
			break;
		}
		case cSTEPALL:
		{
			every_combo_opts opts{};
			opts.include_rposes_base_screen_only = true;
			for_every_combo([&](const auto& handle) {
				if (isStepType(handle.ctype()))
					handle.increment_data();
			}, opts);

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

bool play_combo_string(int str, int screen)
{
	mapscr* scr = get_scr(screen);
	switch(str)
	{
		case -1: //Special case: Use Screen String
			str = scr->str;
			break;
		case -2: //Special case: Use Screen Catchall
			str = scr->catchall;
			break;
		case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17: //Special case: Screen->D[]
			int32_t di = (cur_dmap<<7) + screen-(DMaps[cur_dmap].type==dmOVERW ? 0 : DMaps[cur_dmap].xoff);
			str = game->screen_d[di][abs(str)-10] / 10000L;
			break;
	}
	if(!str || unsigned(str) >= MAXMSGS)
		return false;

	donewmsg(scr, str);
	return true;
}

bool play_combo_string(int str)
{
	return play_combo_string(str, cur_screen);
}

void trigger_string(combo_trigger const& trig, bool success)
{
	play_combo_string((success ? trig.trig_msgstr : trig.fail_msgstr)/10000L, cur_screen);
}

void trigger_sign(newcombo const& cmb, int screen)
{
	play_combo_string(cmb.attributes[0]/10000L, screen);
}

void trigger_sign(newcombo const& cmb)
{
	play_combo_string(cmb.attributes[0]/10000L, cur_screen);
}

bool trigger_warp(const combined_handle_t& handle)
{
	auto& cmb = handle.combo();
	if(!isWarpType(cmb.type)) return false;

	mapscr* scr = handle.base_scr();
	mapscr* wscr = cur_screen >= 128 ? special_warp_return_scr : scr;
	auto index = getWarpLetter(cmb.type);
	if(index == 4) index = zc_oldrand()%4; //Random warp
	auto wtype = wscr->tilewarptype[index];
	auto stype = simplifyWarpType(cmb.type);
	if(wtype==wtNOWARP && stype != cAWARPA)
		return false;
	auto wsfx = cmb.attribytes[0];
	auto tdm = wscr->tilewarpdmap[index];
	auto tscr = wscr->tilewarpscr[index];
	auto wrindex=(wscr->warpreturnc>>(index*2))&3;
	int32_t ws = map_screen_index(DMaps[tdm].map, wscr->tilewarpscr[index] + DMaps[tdm].xoff);
	mapscr* wscr2=&TheMaps[ws];
	int wx = wscr2->warpreturnx[wrindex];
	int wy = wscr2->warpreturny[wrindex];
	auto weff = warpEffectNONE;
	if (stype == cAWARPA) //these use sidewarps
	{
		wtype = wscr->sidewarptype[index];
		if (wtype == wtNOWARP)
			return false;
		wsfx = cmb.attribytes[0];
		tdm = wscr->sidewarpdmap[index];
		tscr = wscr->sidewarpscr[index];
		ws = map_screen_index(DMaps[tdm].map, wscr->sidewarpscr[index] + DMaps[tdm].xoff);
		wscr2 = &TheMaps[ws];
		wx = wscr2->warpreturnx[wrindex];
		wy = wscr2->warpreturny[wrindex];
	}
	if (stype == cPIT || (stype == cAWARPA && (scr->flags5 & fDIRECTAWARP)) || (stype == cSWARPA && (scr->flags5 & fDIRECTSWARP)))
	{
		wx = Hero.getX().getInt();
		wy = Hero.getY().getInt();
	}
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
	if(scr->flags3&fIWARP_SPRITE_CARRYOVER) wflag |= warpFlagDONTCLEARSPRITES;
	//Queue the warp for the next frame, as doing anything else breaks terribly
	FFCore.queueWarp(wtype, tdm, tscr, wx, wy, weff, wsfx, wflag, -1);
	return true;
}

bool trigger_chest(const rpos_handle_t& rpos_handle)
{
	mapscr* base_scr = rpos_handle.base_scr();
	int screen = rpos_handle.screen;

	auto& cmb = rpos_handle.combo();	
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				
				remove_xstatecombos(create_screen_handles(rpos_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(base_scr, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(rpos_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(base_scr, mCHEST);
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(rpos_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(base_scr, mBOSSCHEST);
			break;
	}
	
	sfx(cmb.attribytes[3]); //opening sfx
	
	bool itemflag = false;
	for(int32_t i=0; i<3; i++)
	{
		mapscr* layer_scr = get_scr_layer_valid(rpos_handle.screen, i);
		if (!layer_scr)
			continue;

		if(layer_scr->sflag[rpos_handle.pos]==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
		if(combobuf[layer_scr->data[rpos_handle.pos]].flag==mfARMOS_ITEM)
		{
			itemflag = true; break;
		}
	}
	bool itemstate = false;
	int32_t ipflag = 0;
	if(cmb.usrflags & cflag7)
	{
		itemstate = getmapflag(rpos_handle.scr, (rpos_handle.screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (rpos_handle.screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((base_scr->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((cur_dmap)<<7) + rpos_handle.screen-(DMaps[cur_dmap].type==dmOVERW ? 0 : DMaps[cur_dmap].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = base_scr->catchall;
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
	int screen = ffc_handle.screen;

	auto& cmb = ffc_handle.combo();
	int32_t cid = ffc_handle.data();
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(ffc_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.scr, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			if(cmb.usrflags&cflag16)
			{
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(ffc_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.scr, mCHEST);
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(ffc_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.scr, mBOSSCHEST);
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
		itemstate = getmapflag(screen, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		ipflag = (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? ipONETIME : ipONETIME2;
	}
	if(itemflag && !itemstate)
	{
		int32_t pflags = ipflag | ipBIGRANGE | ipHOLDUP | ((ffc_handle.scr->flags8&fITEMSECRET) ? ipSECRETS : 0);
		int32_t itid = cmb.attrishorts[2];
		switch(itid)
		{
			case -10: case -11: case -12: case -13:
			case -14: case -15: case -16: case -17:
			{
				int32_t di = ((get_currdmap())<<7) + ffc_handle.screen-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
				itid = game->screen_d[di][abs(itid)-10] / 10000L;
				break;
			}
			case -1:
				itid = ffc_handle.scr->catchall;
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
	mapscr* base_scr = rpos_handle.base_scr();
	int screen = rpos_handle.screen;
	
	auto& cmb = rpos_handle.combo();	
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(base_scr), 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(base_scr, mLOCKBLOCK);
			remove_lockblocks(create_screen_handles(base_scr));
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
				setxmapflag(screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(base_scr), 1<<cmb.attribytes[5], false);
				break;
			}
			setmapflag(base_scr, mBOSSLOCKBLOCK);
			remove_bosslockblocks(create_screen_handles(base_scr));
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
	auto& cmb = ffc_handle.combo();
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
				setxmapflag(ffc_handle.screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(ffc_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.scr, mLOCKBLOCK);
			remove_lockblocks(create_screen_handles(ffc_handle.scr));
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
				setxmapflag(ffc_handle.screen, 1<<cmb.attribytes[5]);
				remove_xstatecombos(create_screen_handles(ffc_handle.scr), 1<<cmb.attribytes[5]);
				break;
			}
			setmapflag(ffc_handle.scr, mBOSSLOCKBLOCK);
			remove_bosslockblocks(create_screen_handles(ffc_handle.scr));
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
	
	int pos = rpos_handle.pos;
	//!TODO Expand 'activation_counters' stuff to account for layers, so that layers >0 can be used
	if (activation_counters[(int)rpos_handle.rpos]) return false;
	int32_t gc = 0;
	for(int32_t i=0; i<guys.Count(); ++i)
	{
		if(((enemy*)guys.spr(i))->mainguy)
		{
			++gc;
		}
	}
	if(gc > 10) return false; //Unsure what this purpose is

	auto& cmb = rpos_handle.combo();
	int32_t eclk = -14;
	int32_t id2 = 0;
	auto [tx, ty] = rpos_handle.xy();
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
				activation_counters[(int)rpos_handle.rpos] = 61;
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
				if ((guysbuf[id2].SIZEflags&OVERRIDE_TILE_HEIGHT) != 0) armosxsz = guysbuf[id2].txsz;
				if ((guysbuf[id2].SIZEflags&OVERRIDE_TILE_WIDTH) != 0) armosysz = guysbuf[id2].tysz;
				
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
								chy -= 16;
								if ( ty + chy < 0 ) break; //don't go out of bounds
								
								if (get_rpos_handle_for_world_xy(tx, ty + chy, rpos_handle.layer).ctype() == cARMOS)
								{
									ypos -=16;
								}
								else searching = 2;
							}
							while(searching == 2) //find the left edge of an armos block
							{
								chx -= 16;
								if ( tx + chx < 0 ) break; //don't go out of bounds

								if (get_rpos_handle_for_world_xy(tx + chx, ty, rpos_handle.layer).ctype() == cARMOS)
								{
									xpos -=16;
								}
								else searching = 3;
							}
							
							break;
						}
						case down: //touched armos from above
						{
							while(searching == 1) //find the left edge of an armos block
							{
								chx -= 16;
								if ( tx + chx < 0 ) break; //don't go out of bounds
								
								if (get_rpos_handle_for_world_xy(tx + chx, ty, rpos_handle.layer).ctype() == cARMOS)
								{
									xpos -=16;
								}
								else searching = 3;
							}
						}
						[[fallthrough]];
						case left: //touched right edge of armos
						{
							while(searching == 1) //find the top edge of an armos block
							{
								chy -= 16;
								if ( ty + chy < 0 ) break; //don't go out of bounds

								if (get_rpos_handle_for_world_xy(tx, ty + chy, rpos_handle.layer).ctype() == cARMOS)
								{
									ypos -=16;
								}
								else searching = 2;
							}
							while(searching == 2) //find the left edge of an armos block
							{
								chx -= 16;
								if ( tx + chx < 0 ) break; //don't go out of bounds

								if (get_rpos_handle_for_world_xy(tx + chx, ty, rpos_handle.layer).ctype() == cARMOS)
								{
									xpos -=16;
								}
								else searching = 3;
							}
							break;
						}
							
						case right: //touched left edge of armos
						{
							while(searching == 1) //find the top edge of an armos block
							{
								chy -= 16;
								if ( ty + chy < 0 ) break; //don't go out of bounds

								if (get_rpos_handle_for_world_xy(tx, ty + chy, rpos_handle.layer).ctype() == cARMOS)
								{
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
				for (int32_t n = 0; n < armosysz; n++)
				{
					for (int32_t m = 0; m < armosxsz; m++) 
					{
						rpos_t rpos = COMBOPOS_REGION_B(xpos2 + m*16, ypos2 + n*16);
						if (rpos != rpos_t::None)
							activation_counters[(int)rpos] = 61;
					}
				}
				if (guysbuf[id2].family == eeGHOMA) 
				{
					if ( COMBOTYPE(tx + chx + 1, ty) == cARMOS ) xpos += 16;
				}
				if(addenemy(rpos_handle.screen,tx+xpos,ty+1+ypos,id2,0))
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
				rpos_handle.increment_data();
			break;
		}
		default: return false;
	}
	activation_counters[(int)rpos_handle.rpos] = 61;
	if(addenemy(rpos_handle.screen,tx,ty+3,id2,eclk))
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
	auto& cmb = ffc_handle.combo();
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
				ffc_handle.increment_data();
			break;
		}
		default: return false;
	}
	activation_counters_ffc[ffc_handle.id] = 61;
	if(addenemy(ffc_handle.screen,tx,ty+3,id2,eclk))
	{
		((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
		((enemy*)guys.spr(guys.Count()-1))->ffcactivated=ffc_handle;
	}
	else return false;
	return true;
}

bool trigger_damage_combo(const rpos_handle_t& rpos_handle, int type, int ptrval, int32_t hdir, bool force_solid)
{
	return trigger_damage_combo(rpos_handle.base_scr(), rpos_handle.data(), type, ptrval, hdir, force_solid);
}

bool trigger_damage_combo(mapscr* scr, int32_t cid, int type, int ptrval, int32_t hdir, bool force_solid)
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

	bool current_ring = (scr->flags6&fTOGGLERINGDAMAGE) != 0;
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
		if(itemid < 0 || ignoreBoots || (scr->flags5&fDAMAGEWITHBOOTS)
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

static void trigger_stepfx_add_weapon_and_load_gfx(sprite_list& list, const newcombo& cmb, weapon* wpn)
{
	if (!list.add(wpn))
		return;

	if (cmb.attribytes[3] > 0)
		wpn->LOADGFX(cmb.attribytes[3]);
	
	if(cmb.usrflags & cflag10)
		wpn->load_weap_data(cmb.misc_weap_data);
}

bool trigger_stepfx(const rpos_handle_t& rpos_handle, bool stepped)
{
	auto [tx, ty] = rpos_handle.xy();

	auto& cmb = rpos_handle.combo();

	int32_t thesfx = cmb.attribytes[0];
	sfx_no_repeat(thesfx,pan(tx));

	tx += 8;
	ty += 8;

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
				trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false));
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
				trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0));
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
						trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
					}
					break;
				}
				else //wscript ewpn
				{
					trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false));
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
					trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
				}
				break;
			
			default: //enemy bomb
				trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,damg,up, -1,-1,false));
				break;
			
			//x,y,z, wpn, 0, dmisc4, dir,-1,getUID(),false);
		}
		if (!(cmb.usrflags&cflag3) ) //Don't Advance
		{
			rpos_handle.increment_data();
		}
	}
	return true;
}

bool trigger_stepfx_ffc(const ffc_handle_t& ffc_handle, bool stepped)
{
	ffcdata* ffc = ffc_handle.ffc;
	auto& cmb = ffc_handle.combo();
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
				trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false));
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
				trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem,Hero.getUID(),false,0,1,0));
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
						trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
					}
					break;
				}
				else //wscript ewpn
				{
					trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,0,damg,wpdir, -1,-1,false));
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
					trigger_stepfx_add_weapon_and_load_gfx(Lwpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,wpn,wlvl,damg,wpdir,parentitem, Hero.getUID(),false,0,1,0));
				}
				break;
			
			default: //enemy bomb
				trigger_stepfx_add_weapon_and_load_gfx(Ewpns, cmb, new weapon((zfix)tx,(zfix)ty,(zfix)0,ewLitBomb,0,damg,up, -1,-1,false));
				break;
			
			//x,y,z, wpn, 0, dmisc4, dir,-1,getUID(),false);
		}
		if (!(cmb.usrflags&cflag3)) //Don't Advance
		{
			ffc_handle.increment_data();
		}
	}
	return true;
}


bool trigger_switchhookblock(const rpos_handle_t& rpos_handle)
{
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
		
		wpn = new weapon((zfix)wx,(zfix)wy,(zfix)0,weapid,plvl,damage,wdir,pitem, Hero.getUID(),false,0,1,0,0,weapspr,autorot);
		if (!Lwpns.add(wpn)) return nullptr;
	}
	else
	{
		wpn = new weapon((zfix)wx,(zfix)wy,(zfix)0,weapid,0,damage,wdir, -1,-1,false,0,0,0,0,weapspr,autorot);
		if (!Ewpns.add(wpn)) return nullptr;
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
	
	wpn->script = cmb.attribytes[5];
	if(cmb.usrflags & cflag10)
		wpn->load_weap_data(cmb.misc_weap_data);
	
	if(!((lw ? lwpnmap : ewpnmap)[wpn->script-1].hasScriptData())) // validate script
		wpn->script = 0;
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
bool trigger_shooter(const rpos_handle_t& rpos_handle)
{
	auto [x, y] = rpos_handle.xy();
	return trigger_shooter(rpos_handle.combo(), x, y);
}

void trigger_save(newcombo const& cmb, mapscr* scr)
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
	save_game((scr->flags4&fSAVEROOM) != 0, save_type);
}

static byte copycat_id = 0;
static bool do_copycat_trigger(const rpos_handle_t& rpos_handle)
{
	if(!copycat_id) return false;

	auto cid = rpos_handle.data();
	auto& cmb = rpos_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig = cmb.triggers[idx];
		if(trig.trigcopycat == copycat_id && !(trig.triggerflags[4] & combotriggerNO_COPYCAT_CAUSE))
		{
			do_trigger_combo(rpos_handle, idx);
			ret = true;
			if(rpos_handle.data() != cid) break;
		}
	}
	return ret;
}

static bool do_copycat_trigger_ffc(const ffc_handle_t& ffc_handle)
{
	if(!copycat_id) return false;

	auto cid = ffc_handle.data();
	auto& cmb = ffc_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig = cmb.triggers[idx];
		if(trig.trigcopycat == copycat_id && !(trig.triggerflags[4] & combotriggerNO_COPYCAT_CAUSE))
		{
			do_trigger_combo(ffc_handle, idx);
			ret = true;
			if(ffc_handle.data() != cid) break;
		}
	}
	return ret;
}

static int copycat_skip_lyr = -1, copycat_skip_ffc = -1;
static rpos_t copycat_skip_rpos = rpos_t::None;

void trig_copycat(byte copyid)
{
	if(copycat_id)
		return;
	copycat_id = copyid;

	for_every_rpos([&](const rpos_handle_t& cc_rpos_handle) {
		if (cc_rpos_handle.layer == copycat_skip_lyr && cc_rpos_handle.rpos == copycat_skip_rpos)
			return;
		do_copycat_trigger(cc_rpos_handle);
	});

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if (ffc_handle.i != copycat_skip_ffc)
				do_copycat_trigger_ffc(ffc_handle);
		});
	}
	copycat_id = 0;
}

static void trig_copycat(int cid, const rpos_handle_t& rpos_handle, byte trigcopyid)
{
	if(copycat_id)
		return;
	if (rpos_handle.data() == cid) //skip self
	{
		copycat_skip_lyr = rpos_handle.layer;
		copycat_skip_rpos = rpos_handle.rpos;
	}
	trig_copycat(trigcopyid);
	copycat_skip_lyr = -1;
	copycat_skip_rpos = rpos_t::None;
}
static void trig_copycat(int cid, const ffc_handle_t& ffc_handle, byte trigcopyid)
{
	if(copycat_id)
		return;
	if (ffc_handle.data() == cid) //skip self
	{
		copycat_skip_ffc = ffc_handle.i;
	}
	trig_copycat(trigcopyid);
	copycat_skip_ffc = -1;
}

void do_ex_trigger(const rpos_handle_t& rpos_handle, size_t idx)
{
	int32_t cid = rpos_handle.data();
	int32_t ocs = rpos_handle.cset();
	auto& cmb = rpos_handle.combo();
	if(cmb.triggers.size() <= idx) return;
	auto& trig = cmb.triggers[idx];
	if(trig.trigchange)
	{
		rpos_handle.set_data(cid+trig.trigchange);
	}
	if(trig.trigcschange)
	{
		rpos_handle.set_cset((ocs+trig.trigcschange) & 0xF);
	}
	if(trig.triggerflags[0] & combotriggerRESETANIM)
	{
		auto& rcmb = rpos_handle.combo();
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
		combo_caches::drawing.refresh(rpos_handle.data());
	}
	
	if(trig.trigcopycat && is_in_current_region(rpos_handle.base_scr())) //has a copycat set
		trig_copycat(cid, rpos_handle, trig.trigcopycat);
}

void do_ex_trigger_ffc(const ffc_handle_t& ffc_handle, size_t idx)
{
	ffcdata* ffc = ffc_handle.ffc;
	int32_t cid = ffc_handle.data();
	int32_t ocs = ffc->cset;
	auto& cmb = ffc_handle.combo();
	if(cmb.triggers.size() <= idx) return;
	auto& trig = cmb.triggers[idx];
	if(trig.trigchange)
	{
		ffc_handle.set_data(cid+trig.trigchange);
	}
	if(trig.trigcschange)
	{
		ffc->cset = (ocs+trig.trigcschange) & 0xF;
	}
	if(trig.triggerflags[0] & combotriggerRESETANIM)
	{
		newcombo& rcmb = combobuf[ffc_handle.data()];
		rcmb.tile = rcmb.o_tile;
		rcmb.cur_frame=0;
		rcmb.aclk = 0;
		combo_caches::drawing.refresh(ffc_handle.data());
	}
	
	if(trig.trigcopycat) //has a copycat set
		trig_copycat(cid, ffc_handle, trig.trigcopycat);
}

bool force_ex_trigger(const rpos_handle_t& rpos_handle, size_t idx, char xstate)
{
	auto& cmb = rpos_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];
	if(trig.triggerflags[4] & combotriggerUNSETEXSTATE)
		return false;
	if(trig.exstate > -1 && (xstate < 0 || xstate == trig.exstate))
	{
		if(xstate >= 0 || getxmapflag(rpos_handle.screen, 1<<trig.exstate))
		{
			do_ex_trigger(rpos_handle, idx);
			return true;
		}
	}
	return false;
}

bool force_ex_trigger_ffc(const ffc_handle_t& ffc_handle, size_t idx, char xstate)
{
	auto& cmb = ffc_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];
	if(trig.triggerflags[4] & combotriggerUNSETEXSTATE)
		return false;
	if(trig.exstate > -1 && (xstate < 0 || xstate == trig.exstate))
	{
		if(xstate >= 0 || getxmapflag(ffc_handle.screen, 1<<trig.exstate))
		{
			do_ex_trigger_ffc(ffc_handle, idx);
			return true;
		}
	}
	return false;
}

bool force_ex_trigger_any(const rpos_handle_t& rpos_handle, char xstate)
{
	auto cid = rpos_handle.data();
	auto& cmb = rpos_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		if(force_ex_trigger(rpos_handle, idx, xstate))
		{
			ret = true;
			if(rpos_handle.data() != cid) break;
		}
	}
	return ret;
}

bool force_ex_trigger_ffc_any(const ffc_handle_t& ffc_handle, char xstate)
{
	auto cid = ffc_handle.data();
	auto& cmb = ffc_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		if(force_ex_trigger_ffc(ffc_handle, idx, xstate))
		{
			ret = true;
			if(ffc_handle.data() != cid) break;
		}
	}
	return ret;
}

bool force_ex_door_trigger(const rpos_handle_t& rpos_handle, size_t idx, int dir, uint ind)
{
	if (dir > 3 || ind > 7) return false;

	auto& cmb = rpos_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];
	if(trig.triggerflags[4] & combotriggerUNSETEXDOOR)
		return false;
	if(trig.exdoor_dir > -1 && (dir < 0 || (dir == trig.exdoor_dir && ind == trig.exdoor_ind)))
	{
		if(dir >= 0 || getxdoor(rpos_handle.screen, trig.exdoor_dir, trig.exdoor_ind))
		{
			do_ex_trigger(rpos_handle, idx);
			return true;
		}
	}
	return false;
}
bool force_ex_door_trigger_ffc(const ffc_handle_t& ffc_handle, size_t idx, int dir, uint ind)
{
	if (dir > 3 || ind > 7) return false;

	auto& cmb = ffc_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];
	if(trig.triggerflags[4] & combotriggerUNSETEXDOOR)
		return false;
	if(trig.exdoor_dir > -1 && (dir < 0 || (dir == trig.exdoor_dir && ind == trig.exdoor_ind)))
	{
		if(dir >= 0 || getxdoor(ffc_handle.screen, trig.exdoor_dir, trig.exdoor_ind))
		{
			do_ex_trigger_ffc(ffc_handle, idx);
			return true;
		}
	}
	return false;
}

bool force_ex_door_trigger_any(const rpos_handle_t& rpos_handle, int dir, uint ind)
{
	if (dir > 3 || ind > 7) return false;

	auto cid = rpos_handle.data();
	auto& cmb = rpos_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		if(force_ex_door_trigger(rpos_handle, idx, dir, ind))
		{
			ret = true;
			if(rpos_handle.data() != cid) break;
		}
	}
	return ret;
}
bool force_ex_door_trigger_ffc_any(const ffc_handle_t& ffc_handle, int dir, uint ind)
{
	if (dir > 3 || ind > 7) return false;

	auto cid = ffc_handle.data();
	auto& cmb = ffc_handle.combo();
	bool ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		if(force_ex_door_trigger_ffc(ffc_handle, idx, dir, ind))
		{
			ret = true;
			if(ffc_handle.data() != cid) break;
		}
	}
	return ret;
}

static bool triggering_generic_secrets = false;
static bool triggering_generic_switchstate = false;

void do_weapon_fx(weapon* w, combo_trigger const& trig)
{
	if(!w) return;
	if(trig.triggerflags[0] & combotriggerKILLWPN)
		killgenwpn(w);
	if(trig.triggerflags[3] & combotriggerIGNITE_ANYFIRE)
		w->misc_wflags |= WFLAG_BURN_ANYFIRE;
	if(trig.triggerflags[3] & combotriggerIGNITE_STRONGFIRE)
		w->misc_wflags |= WFLAG_BURN_STRONGFIRE;
	if(trig.triggerflags[3] & combotriggerIGNITE_MAGICFIRE)
		w->misc_wflags |= WFLAG_BURN_MAGICFIRE;
	if(trig.triggerflags[3] & combotriggerIGNITE_DIVINEFIRE)
		w->misc_wflags |= WFLAG_BURN_DIVINEFIRE;
}

int32_t get_cmb_trigctrcost(combo_trigger const& trig)
{
	int32_t ctrcost = trig.trigctramnt;
	if(!ctrcost) return ctrcost;
	if(trig.triggerflags[3] & combotriggerCOUNTERDISCOUNT)
	{
		auto wmedal_id = current_item_id(itype_wealthmedal);
		if(wmedal_id > -1)
		{
			itemdata const& wmedal = itemsbuf[wmedal_id];
			if(wmedal.flags & item_flag1)
				ctrcost = (ctrcost * wmedal.misc1) / 100;
			else
				ctrcost += wmedal.misc1; // will be negative for discount
			if(trig.trigctramnt > 0 && ctrcost < 0)
				ctrcost = 1; // matching the behavior on room types
		}
	}
	return ctrcost;
}

// Return true if the condition passes.
static bool check_dark_trigger_conditional(bool is_active_screen, mapscr* scr, bool expect_lit)
{
	if (get_qr(qr_INVERTED_DARK_COMBO_TRIGGERS)) expect_lit = !expect_lit;
	bool lit = is_active_screen ? get_lights(scr) : !(scr->flags&fDARK);
	return expect_lit == lit;
}

static bool handle_trigger_conditionals(combined_handle_t const& comb_handle, size_t idx, bool& hasitem, bool side_effects = true)
{
	auto& cmb = comb_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	
	auto& trig = cmb.triggers[idx];
	auto [cx, cy] = comb_handle.xy();
	auto [center_x, center_y] = comb_handle.center_xy();
	mapscr* scr = comb_handle.base_scr();
	
	bool is_active_screen = is_in_current_region(scr);

	if(trig.triggeritem) //Item requirement
	{
		hasitem = game->get_item(trig.triggeritem) && !item_disabled(trig.triggeritem)
			&& checkbunny(trig.triggeritem);
		if(trig.triggerflags[1] & combotriggerINVERTITEM)
		{
			if(hasitem) return false;
		}
		else if(!hasitem) return false;
	}
	if(trig.trigprox) //Proximity requirement
	{
		zfix cx2 = cx + FFCore.ScrollingData[SCROLLDATA_NX];
		zfix cy2 = cy + FFCore.ScrollingData[SCROLLDATA_NY];

		word d = dist(Hero.getX(), Hero.getY(), cx2, cy2).getInt();
		if(trig.triggerflags[0] & combotriggerINVERTPROX) //trigger outside the radius
		{
			if(d < trig.trigprox || !is_active_screen) //inside, cancel
				return false;
		}
		else //trigger inside the radius
		{
			if(d >= trig.trigprox && is_active_screen) //outside, cancel
				return false;
		}
	}
	
	if(trig.triggerflags[4] & (combotriggerREQ_X_LE|combotriggerREQ_X_GE|
		combotriggerREQ_Y_LE|combotriggerREQ_Y_GE|
		combotriggerREQ_JUMP_LE|combotriggerREQ_JUMP_GE))
	{
		auto targ_x = trig.req_player_x;
		auto targ_y = trig.req_player_y;
		
		if(trig.triggerflags[4] & combotriggerREQ_X_REL)
			targ_x += center_x - 8;
		if(trig.triggerflags[4] & combotriggerREQ_Y_REL)
			targ_y += center_y - 8;
		
		if(trig.triggerflags[4] & combotriggerREQ_X_LE)
		{
			if(Hero.getX() > targ_x)
				return false;
		}
		if(trig.triggerflags[4] & combotriggerREQ_X_GE)
		{
			if(Hero.getX() < targ_x)
				return false;
		}
		
		if(trig.triggerflags[4] & combotriggerREQ_Y_LE)
		{
			if(Hero.getY() > targ_y)
				return false;
		}
		if(trig.triggerflags[4] & combotriggerREQ_Y_GE)
		{
			if(Hero.getY() < targ_y)
				return false;
		}
		
		if(trig.triggerflags[4] & combotriggerREQ_JUMP_LE)
		{
			if(Hero.getJump() > trig.req_player_jump)
				return false;
		}
		if(trig.triggerflags[4] & combotriggerREQ_JUMP_GE)
		{
			if(Hero.getJump() < trig.req_player_jump)
				return false;
		}
	}
	
	if(trig.triggerflags[4] & combotriggerINVERT_PLAYER_Z)
	{
		if(Hero.z >= trig.req_player_z)
			return false;
	}
	else
	{
		if(Hero.z < trig.req_player_z)
			return false;
	}
	
	if(trig.triggerflags[4] & combotriggerREQ_JUMP_LE)
	{
		if(Hero.getJump() > trig.req_player_jump)
			return false;
	}
	if(trig.triggerflags[4] & combotriggerREQ_JUMP_GE)
	{
		if(Hero.getJump() < trig.req_player_jump)
			return false;
	}
	
	if(trig.req_player_dir && !(trig.req_player_dir & (1 << Hero.dir)))
		return false;
	
	if(trig.triggerflags[4] & combotriggerPLAYER_STANDING)
		if(!Hero.isStanding(true))
			return false;
	if(trig.triggerflags[4] & combotriggerPLAYER_NOTSTANDING)
		if(Hero.isStanding(true))
			return false;
	if((trig.triggerflags[3] & combotriggerCOND_DARK))
		if(!check_dark_trigger_conditional(is_active_screen, scr, false))
			return false;
	if((trig.triggerflags[3] & combotriggerCOND_NODARK))
		if(!check_dark_trigger_conditional(is_active_screen, scr, true))
			return false;

	if (is_active_screen || trig.trigdmlevel > -1)
	{
		auto dmap_level = trig.trigdmlevel > -1 ? trig.trigdmlevel : dlevel;
		if(trig.triggerflags[3] & combotriggerLITEM_COND)
			if((trig.trig_levelitems & game->lvlitems[dmap_level]) != trig.trig_levelitems)
				return false;
		if(trig.triggerflags[3] & combotriggerLITEM_REVCOND)
			if((trig.trig_levelitems & game->lvlitems[dmap_level]) == trig.trig_levelitems)
				return false;
		if((trig.req_level_state & game->lvlswitches[dmap_level]) != trig.req_level_state)
			return false; // missing at least 1 required state
		if(trig.unreq_level_state & game->lvlswitches[dmap_level])
			return false; // has at least 1 un-required state
		for(int q = 0; q < 256; ++q)
		{
			if(trig.req_global_state.get(q) && !game->gswitch_timers[q])
				return false;
			if(trig.unreq_global_state.get(q) && game->gswitch_timers[q])
				return false;
		}
	}
	
	if(zfix ctrcost = get_cmb_trigctrcost(trig))
	{
		bool inf_ctr = false;
		bool perc_ctr = (trig.triggerflags[4] & combotriggerCOUNTER_PERCENT);
		zfix ctrval = get_ssc_ctr(trig.trigctr, &inf_ctr);
		zfix ctrmax = get_ssc_ctrmax(trig.trigctr);
		if(perc_ctr)
			ctrcost = (ctrcost / 100) * ctrmax;
		
		if(side_effects && is_active_screen && (trig.triggerflags[1] & combotriggerCTRNONLYTRIG) && (trig.triggerflags[1] & combotriggerCOUNTEREAT))
		{
			if(ctrval >= ctrcost && !inf_ctr)
				modify_ssc_ctr(trig.trigctr, -ctrcost, trig.triggerflags[4] & combotriggerCOUNTER_GRADUAL);
		}
		if(trig.triggerflags[1] & combotriggerCOUNTERGE)
		{
			if(ctrval < ctrcost && !inf_ctr)
				return false;
		}
		if(trig.triggerflags[1] & combotriggerCOUNTERLT)
		{
			if(ctrval >= ctrcost || inf_ctr)
				return false;
		}
	}
	return true;
}
void handle_trigger_results(mapscr* scr, combo_trigger const& trig, int32_t cx, int32_t cy, bool& hasitem, bool& used_bit,
	int32_t special)
{
	int screen = scr->screen;
	if(trig.triggerflags[3]&combotriggerTOGGLEDARK)
	{
		toggle_lights(pal_litOVERRIDE);
	}
	if (trig.triggerflags[1]&combotriggerSECRETS)
	{
		used_bit = true;
		if(!(special & ctrigSECRETS) && !triggering_generic_secrets)
		{
			triggering_generic_secrets = true;
			trigger_secrets_for_screen(TriggerSource::GenericCombo, scr, false);
			triggering_generic_secrets = false;
			if(scr->secretsfx)
				sfx(scr->secretsfx);
		}
		if(canPermSecret(cur_dmap, screen) && !(scr->flags5&fTEMPSECRETS) && !getmapflag(screen, mSECRET))
			setmapflag(scr, mSECRET);
	}
	
	if (trig.triggerflags[3] & combotriggerLEVELSTATE)
	{
		used_bit = true;
		if(!(special & ctrigSWITCHSTATE) && !triggering_generic_switchstate)
		{
			triggering_generic_switchstate = true;
			game->lvlswitches[dlevel] ^= 1<<trig.trig_lstate;
			toggle_switches(1<<trig.trig_lstate, false);
			triggering_generic_switchstate = false;
		}
	}
	if (trig.triggerflags[3] & combotriggerGLOBALSTATE)
	{
		used_bit = true;
		if(!(special & ctrigSWITCHSTATE) && !triggering_generic_switchstate)
		{
			int tmr = trig.trig_statetime, pair = trig.trig_gstate;
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
	
	if(trig.trigsfx)
		sfx(trig.trigsfx, pan(cx));
	
	if(trig.triggerflags[3] & combotriggerKILLENEMIES)
		kill_em_all();
	if(trig.triggerflags[3] & combotriggerCLEARENEMIES)
		guys.clear(true);
	if(trig.triggerflags[3] & combotriggerCLEARLWEAPONS)
		Lwpns.clear(true);
	if(trig.triggerflags[3] & combotriggerCLEAREWEAPONS)
		Ewpns.clear(true);
	
	if(trig.triggeritem && hasitem && (trig.triggerflags[1] & combotriggerCONSUMEITEM))
	{
		takeitem(trig.triggeritem);
	}
	if(!(trig.triggerflags[1] & combotriggerCTRNONLYTRIG) && (trig.triggerflags[1] & combotriggerCOUNTEREAT))
	{
		if(zfix ctrcost = get_cmb_trigctrcost(trig))
		{
			bool inf_ctr = false;
			bool perc_ctr = (trig.triggerflags[4] & combotriggerCOUNTER_PERCENT);
			zfix ctrval = get_ssc_ctr(trig.trigctr, &inf_ctr);
			zfix ctrmax = get_ssc_ctrmax(trig.trigctr);
			if(perc_ctr)
				ctrcost = (ctrcost / 100) * ctrmax;
			if(ctrval >= ctrcost && !inf_ctr)
				modify_ssc_ctr(trig.trigctr, -ctrcost, trig.triggerflags[4] & combotriggerCOUNTER_GRADUAL);
		}
	}
	bool trigexstate = true;
	if(trig.spawnenemy)
	{
		enemy* enm = nullptr;
		bool enm_ex = (trig.triggerflags[2] & combotriggerEXSTENEMY);
		word numcreated = addenemy(screen, cx, cy, trig.spawnenemy, -10);
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
				enm->deathexstate = trig.exstate;
			}
		}
	}
	if(trig.spawnitem)
	{
		bool itm_ex = (trig.triggerflags[2] & combotriggerEXSTITEM);
		bool specitem = (trig.triggerflags[2] & combotriggerSPCITEM);
		if(specitem && getmapflag(screen, mSPECIALITEM))
		{
			//already collected
			if(itm_ex) trigexstate = true;
		}
		else
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			int32_t pflags = trig.spawnip & allowed_pflags;
			SETFLAG(pflags, ipONETIME2, specitem);
			int32_t item_id = trig.spawnitem;
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
				if(itm) itm->pickupexstate = trig.exstate;
			}
			if(trig.triggerflags[2] & combotriggerAUTOGRABITEM)
			{
				if(itm) itm->set_forcegrab(true);
			}
		}
	}
	
	//Level Item stuff
	{
		auto dmap_level = trig.trigdmlevel > -1 ? trig.trigdmlevel : dlevel;
		bool _set = trig.triggerflags[3] & combotriggerLITEM_SET,
			_unset = trig.triggerflags[3] & combotriggerLITEM_UNSET;
		if(_set && _unset) //toggle
			game->lvlitems[dmap_level] ^= trig.trig_levelitems;
		else if(_set)
			game->lvlitems[dmap_level] |= trig.trig_levelitems;
		else if(_unset)
			game->lvlitems[dmap_level] &= ~trig.trig_levelitems;
	}
	
	//Graphical stuff
	{
		if(trig.triggerflags[3] & combotriggerTINT_CLEAR)
			doClearTint();
		if(trig.trigtint[0] || trig.trigtint[1] || trig.trigtint[2])
			doTint(trig.trigtint[0], trig.trigtint[1], trig.trigtint[2]);
		if(trig.triglvlpalette > -1)
			loadlvlpal(trig.triglvlpalette);
		else if(trig.triglvlpalette == -2)
			loadlvlpal(scr->color);
		if(trig.trigbosspalette > -1)
			loadpalset(csBOSS,pSprite(trig.trigbosspalette));
		if(trig.trigquaketime > -1)
			quakeclk = trig.trigquaketime;
		if(trig.trigwavytime > -1)
			wavy = trig.trigwavytime;
	}
	
	//Status Effects
	{
		if(trig.trig_swjinxtime > -2)
			Hero.setSwordClk(trig.trig_swjinxtime);
		if(trig.trig_itmjinxtime > -2)
			Hero.setItemClk(trig.trig_itmjinxtime);
		if(trig.trig_shieldjinxtime > -2)
			Hero.setShieldClk(trig.trig_shieldjinxtime);
		if(trig.trig_stuntime > -2)
			Hero.setStunClock(trig.trig_stuntime);
		if(trig.trig_bunnytime > -2)
			Hero.setBunnyClock(trig.trig_bunnytime);
	}
	
	if(trig.exstate > -1 && (trig.triggerflags[4] & combotriggerUNSETEXSTATE))
		unsetxmapflag(screen, 1<<trig.exstate);
	else if(trig.exstate > -1 && trigexstate)
		setxmapflag(screen, 1<<trig.exstate);
	if(trig.exdoor_dir > -1 && (trig.triggerflags[4] & combotriggerUNSETEXDOOR))
		set_xdoorstate(screen, trig.exdoor_dir, trig.exdoor_ind, false);
	else if(trig.exdoor_dir > -1)
		set_xdoorstate(screen, trig.exdoor_dir, trig.exdoor_ind);
}

// Triggers a combo at a given position.
// TODO: combine w/ the ffc function into a single function, via combined_handle_t.
bool do_trigger_combo(const rpos_handle_t& rpos_handle, size_t idx, int32_t special, weapon* w)
{
	auto& cmb = rpos_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];
	
	int32_t cid = rpos_handle.data();
	auto [cx, cy] = rpos_handle.xy();
	auto [center_x, center_y] = rpos_handle.center_xy();

	int32_t ocs = rpos_handle.cset();
	mapscr* base_scr = rpos_handle.base_scr();
	bool hasitem = false;
	if(w && (trig.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,trig);
		return true;
	}
	
	int32_t flag = rpos_handle.sflag();
	int32_t flag2 = cmb.flag;
	
	bool check_bit = false;
	bool used_bit = false;
	
	bool force_ex_ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& ex_trig = cmb.triggers[idx];
		if(ex_trig.exstate > -1 && !(ex_trig.triggerflags[4] & combotriggerUNSETEXSTATE))
		{
			if (force_ex_trigger(rpos_handle, idx))
			{
				force_ex_ret = true;
				if(rpos_handle.data() != cid) break;
			}
		}
		if(ex_trig.exdoor_dir > -1 && !(ex_trig.triggerflags[4] & combotriggerUNSETEXDOOR))
		{
			if(force_ex_door_trigger(rpos_handle, idx))
			{
				force_ex_ret = true;
				if(rpos_handle.data() != cid) break;
			}
		}
	}
	if(force_ex_ret)
		return true;

	if(!handle_trigger_conditionals(rpos_handle, idx, hasitem))
	{
		trigger_string(trig, false);
		return false;
	}
	
	if (w)
	{
		check_bit = w->rposes_checked.contains({rpos_handle.layer, rpos_handle.rpos});
	}
	
	static cpos_info null_info;
	bool is_active_screen = is_in_current_region(base_scr);
	cpos_info& timer = is_active_screen ? rpos_handle.info() : null_info;

	bool dorun = !timer.trig_data[idx].cooldown;
	if(dorun)
	{
		if (is_active_screen && ((trig.triggerflags[0] & combotriggerCMBTYPEFX) || alwaysCTypeEffects(cmb.type)))
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
						trigger_warp(rpos_handle);
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
						if(!trigger_shooter(rpos_handle))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb, rpos_handle.scr);
						break;
					default:
						used_bit = false;
				}
			}
		}
		
		if(!check_bit)
		{
			if (is_active_screen)
				handle_trigger_results(base_scr, trig, cx, cy, hasitem, used_bit, special);
			
			if(trig.trigchange)
			{
				used_bit = true;
				rpos_handle.set_data(cid+trig.trigchange);
			}
			if(trig.trigcschange)
			{
				used_bit = true;
				rpos_handle.set_cset((ocs+trig.trigcschange) & 0xF);
			}
			
			if (is_active_screen)
			{
				if(trig.triggerflags[0] & combotriggerRESETANIM)
				{
					newcombo& rcmb = rpos_handle.combo();
					rcmb.tile = rcmb.o_tile;
					rcmb.cur_frame=0;
					rcmb.aclk = 0;
					combo_caches::drawing.refresh(rpos_handle.data());
				}

				if(trig.trigcopycat) //has a copycat set
					trig_copycat(cid, rpos_handle, trig.trigcopycat);

				timer.updateData(rpos_handle.data());

				if (trig.trigcooldown)
					timer.trig_data[idx].cooldown = trig.trigcooldown;
				
				if (trig.player_bounce)
					Hero.setJump(trig.player_bounce);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_X_ABS)
					Hero.setXfix(trig.dest_player_x);
				else if (trig.triggerflags[4] & combotriggerSETPLAYER_X_REL_CMB)
					Hero.setXfix(trig.dest_player_x + center_x - 8);
				else if(trig.dest_player_x)
					Hero.setXfix(Hero.getX() + trig.dest_player_x);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_Y_ABS)
					Hero.setYfix(trig.dest_player_y);
				else if (trig.triggerflags[4] & combotriggerSETPLAYER_Y_REL_CMB)
					Hero.setYfix(trig.dest_player_y + center_y - 8);
				else if(trig.dest_player_y)
					Hero.setYfix(Hero.getY() + trig.dest_player_y);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_Z_ABS)
					Hero.setZfix(trig.dest_player_z);
				else if(trig.dest_player_z)
					Hero.setZfix(Hero.getZ() + trig.dest_player_z);
			}
		}
		if (w && used_bit)
		{
			w->rposes_checked.insert({rpos_handle.layer, rpos_handle.rpos});
		}
	}
	
	if(w)
		do_weapon_fx(w,trig);
	
	if (is_active_screen && dorun && !check_bit)
	{
		if(trig.trig_genscr)
			FFCore.runGenericFrozenEngine(trig.trig_genscr);
		trigger_string(trig, true);
	}
	return true;
}

bool do_trigger_combo(const ffc_handle_t& ffc_handle, size_t idx, int32_t special, weapon* w)
{
	if (get_qr(qr_OLD_FFC_FUNCTIONALITY)) return false;
	auto& cmb = ffc_handle.combo();
	if(cmb.triggers.size() <= idx) return false;
	auto& trig = cmb.triggers[idx];

	ffcdata* ffc = ffc_handle.ffc;
	if (ffc->flags & ffc_changer)
		return false; //Changers can't trigger!

	int32_t cid = ffc_handle.data();
	int32_t ocs = ffc->cset;
	auto [cx, cy] = ffc_handle.xy();
	auto [center_x, center_y] = ffc_handle.center_xy();
	mapscr* base_scr = ffc_handle.scr;
	bool hasitem = false;
	if(w && (trig.triggerflags[3] & combotriggerSEPARATEWEAPON))
	{
		do_weapon_fx(w,trig);
		return true;
	}
	
	int32_t flag2 = cmb.flag;
	
	bool check_bit = false;
	bool used_bit = false;
	
	bool force_ex_ret = false;
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& ex_trig = cmb.triggers[idx];
		if(ex_trig.exstate > -1 && (ex_trig.triggerflags[4] & combotriggerUNSETEXSTATE))
		{
			if(force_ex_trigger_ffc(ffc_handle, idx))
			{
				force_ex_ret = true;
				if(ffc_handle.data() != cid) break;
			}
		}
		if(ex_trig.exdoor_dir > -1 && (ex_trig.triggerflags[4] & combotriggerUNSETEXDOOR))
		{
			if(force_ex_door_trigger_ffc(ffc_handle, idx))
			{
				force_ex_ret = true;
				if(ffc_handle.data() != cid) break;
			}
		}
	}
	if(force_ex_ret)
		return true;

	if(!handle_trigger_conditionals(ffc_handle, idx, hasitem))
	{
		trigger_string(trig, false);
		return false;
	}
	
	if(w)
	{
		check_bit = w->ffcs_checked.contains(ffc);
	}

	static cpos_info null_info;
	bool is_active_screen = is_in_current_region(base_scr);
	cpos_info& timer = is_active_screen ? ffc_handle.info() : null_info;

	bool dorun = !timer.trig_data[idx].cooldown;
	if(dorun)
	{
		if (is_active_screen && ((trig.triggerflags[0] & combotriggerCMBTYPEFX) || alwaysCTypeEffects(cmb.type)))
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
						trigger_warp(ffc_handle);
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
						trigger_damage_combo(ffc_handle.scr, cid, ZSD_FFC, ffc_handle.id);
						break;
					
					case cSTEPSFX:
						trigger_stepfx_ffc(ffc_handle);
						break;
					
					case cSWITCHHOOK:
						if(!trigger_switchhookblock_ffc(ffc_handle))
							return false;
						break;
					
					case cSHOOTER:
						if(!trigger_shooter(ffc_handle.combo(), cx, cy))
							return false;
						break;
					case cSAVE: case cSAVE2:
						trigger_save(cmb, ffc_handle.scr);
						break;
					default:
						used_bit = false;
				}
			}
		}
		
		if(!check_bit)
		{
			if (is_active_screen)
				handle_trigger_results(ffc_handle.scr, trig, cx, cy, hasitem, used_bit, special);
			
			if(trig.trigchange)
			{
				used_bit = true;
				ffc_handle.set_data(cid+trig.trigchange);
			}
			if(trig.trigcschange)
			{
				used_bit = true;
				ffc->cset = (ocs+trig.trigcschange) & 0xF;
			}

			if (is_active_screen)
			{
				if(trig.triggerflags[0] & combotriggerRESETANIM)
				{
					newcombo& rcmb = combobuf[ffc_handle.data()];
					rcmb.tile = rcmb.o_tile;
					rcmb.cur_frame=0;
					rcmb.aclk = 0;
					combo_caches::drawing.refresh(ffc_handle.data());
				}
				
				if(trig.trigcopycat) //has a copycat set
					trig_copycat(cid, ffc_handle, trig.trigcopycat);
				
				if (ffc_handle.ffc->flags & ffc_changer)
					timer.updateData(-1);
				else timer.updateData(ffc_handle.data());

				if (trig.trigcooldown)
					timer.trig_data[idx].cooldown = trig.trigcooldown;
				
				if (trig.player_bounce)
					Hero.setJump(trig.player_bounce);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_X_ABS)
					Hero.setXfix(trig.dest_player_x);
				else if (trig.triggerflags[4] & combotriggerSETPLAYER_X_REL_CMB)
					Hero.setXfix(trig.dest_player_x + center_x - 8);
				else if(trig.dest_player_x)
					Hero.setXfix(Hero.getX() + trig.dest_player_x);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_Y_ABS)
					Hero.setYfix(trig.dest_player_y);
				else if (trig.triggerflags[4] & combotriggerSETPLAYER_Y_REL_CMB)
					Hero.setYfix(trig.dest_player_y + center_y - 8);
				else if(trig.dest_player_y)
					Hero.setYfix(Hero.getY() + trig.dest_player_y);
				
				if (trig.triggerflags[4] & combotriggerSETPLAYER_Z_ABS)
					Hero.setZfix(trig.dest_player_z);
				else if(trig.dest_player_z)
					Hero.setZfix(Hero.getZ() + trig.dest_player_z);
			}
		}
		if (w && used_bit)
		{
			w->ffcs_checked.insert(ffc_handle.ffc);
		}
	}
	
	if(w)
		do_weapon_fx(w,trig);
	
	if (is_active_screen && dorun && !check_bit)
	{
		if(trig.trig_genscr)
			FFCore.runGenericFrozenEngine(trig.trig_genscr);
		trigger_string(trig, true);
	}
	return true;
}

bool do_trigger_combo(const combined_handle_t& comb_handle, size_t idx, int32_t special, weapon* w)
{
	if(comb_handle.is_rpos())
		return do_trigger_combo(comb_handle.get_rpos(), idx, special, w);
	else if(comb_handle.is_ffc())
		return do_trigger_combo(comb_handle.get_ffc(), idx, special, w);
	assert(false);
	return false;
}

bool do_trigger_ctype_causes(const combined_handle_t& comb_handle)
{
	auto cid = comb_handle.data();
	auto& cmb = comb_handle.combo();
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig = cmb.triggers[idx];
		if (trig.triggerflags[4]&combotriggerCMBTYPECAUSES)
		{
			do_trigger_combo(comb_handle, idx);
			if(comb_handle.data() != cid) return true;
		}
	}
	return false;
}

bool check_trig_conditions(const combined_handle_t& comb_handle, size_t idx)
{
	bool hasitem = false;
	return handle_trigger_conditionals(comb_handle, idx, hasitem, false);
}

bool do_lift_combo(const rpos_handle_t& rpos_handle, int32_t gloveid)
{
	if(!Hero.can_lift(gloveid)) return false;
	if(Hero.lift_wpn) return false;

	int32_t cid = rpos_handle.data();
	int32_t cset = rpos_handle.cset();
	auto& cmb = rpos_handle.combo();
	itemdata const& glove = itemsbuf[gloveid];

	if(cmb.liftlvl > glove.fam_type) return false;

	auto [cx, cy] = rpos_handle.xy();

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
	
	if(hasitem && (cmb.liftflags&LF_SPECIALITEM) && getmapflag(rpos_handle.screen, mSPECIALITEM))
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
	
	w->death_sprite = cmb.liftbreaksprite;
	w->death_sfx = cmb.liftbreaksfx;
	
	w->load_weap_data(cmb.lift_weap_data);
	
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

static void handle_shooter(newcombo const& cmb, cpos_info& timer, rpos_t rpos)
{
	auto [x, y] = COMBOXY_REGION(rpos);
	handle_shooter(cmb, timer, x, y);
}

static void trigger_crumble(newcombo const& cmb, cpos_info& timer, word& cid)
{
	++cid;
	timer.updateData(cid);
	//Continue crumbling through the change
	newcombo const& ncmb = combobuf[cid];
	if(ncmb.type == cCRUMBLE && (cmb.usrflags&cflag1)
		&& (ncmb.attribytes[0] == CMBTY_CRUMBLE_INEVITABLE))
		timer.crumbling = true;
}

static bool handle_crumble(newcombo const& cmb, cpos_info& timer, word& cid, zfix x, zfix y, zfix w, zfix h)
{
	bool breaking = false;
	byte ty = cmb.attribytes[0];
	if(ty == CMBTY_CRUMBLE_INEVITABLE)
		if(timer.crumbling)
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
	else if(ty == CMBTY_CRUMBLE_RESET && timer.crumbling)
	{
		timer.type_clk = 0;
		if(int16_t diff = cmb.attrishorts[1])
		{
			cid += diff;
			timer.updateData(cid);
		}
	}
	timer.crumbling = breaking;
	return false;
}

void trig_trigger_groups()
{
	auto& combo_cache = combo_caches::trigger_group;

	bool include_ffcs = true;
	for_every_combo([&](const auto& handle) {
		// changers don't contribute
		if constexpr (requires { handle.ffc; })
		{
			if (handle.ffc->flags & ffc_changer)
				return;
		}

		int cid = handle.data();

		bool recheck = false;
		do
		{
			if (!combo_cache.minis[cid].tgroup)
				break;
			auto& cmb = combobuf[cid];
			recheck = false;
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				bool ok = false;
				if(trig.triggerflags[3] & combotriggerTGROUP_LESS)
					ok = cpos_trig_group_count(trig.trig_group) < trig.trig_group_val;
				if(!ok && (trig.triggerflags[3] & combotriggerTGROUP_GREATER))
					ok = cpos_trig_group_count(trig.trig_group) > trig.trig_group_val;
				if(!ok) continue;
				
				do_trigger_combo(handle, idx);
				int cid2 = handle.data();
				cpos_info& timer = handle.info();

				if constexpr (requires { handle.ffc; })
				{
					if (handle.ffc->flags & ffc_changer)
						timer.updateData(-1);
					else timer.updateData(cid2);
				}
				else timer.updateData(cid2);

				if (cid != cid2)
				{
					cid = cid2;
					recheck = true;
					break;
				}
			}
		}
		while(recheck);
	}, include_ffcs);
}

//COMBOTYPE POS STUFF

void handle_cpos_type(byte combo_type, cpos_info& timer, const rpos_handle_t& rpos_handle)
{
	switch(combo_type)
	{
		case cSHOOTER:
			handle_shooter(rpos_handle.combo(), timer, rpos_handle.rpos);
			break;
		case cCRUMBLE:
		{
			word cid = rpos_handle.data();
			auto [x, y] = rpos_handle.xy();
			handle_crumble(rpos_handle.combo(), timer, cid, x, y, 16, 16);
			rpos_handle.set_data(cid);
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
static std::vector<cpos_info> combo_posinfos;

cpos_info& cpos_get(const rpos_handle_t& rpos_handle)
{
	int index = rpos_handle.layer * region_num_rpos + (int)rpos_handle.rpos;
	return combo_posinfos[index];
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
	
	for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig = cmb.triggers[idx];
		if(trig.triggerflags[3] & combotriggerTGROUP_CONTRIB)
			trig_groups[trig.trig_group] += add;
	}
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
	copycat_skip_rpos = rpos_t::None;
	copycat_skip_ffc = -1;
	//
	combo_posinfos.resize(region_num_rpos * 7);
	for (int i = 0; i < region_num_rpos * 7; i++)
	{
		combo_posinfos[i].clear();
	}
	
	cpos_reset_cache();
}
void cpos_force_update() //updates without side-effects
{
	for_every_rpos([&](const rpos_handle_t& rpos_handle) {
		rpos_handle.info().updateData(rpos_handle.data());
	});

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		if (ffc_handle.ffc->flags & ffc_changer)
			ffc_handle.ffc->info.updateData(-1);
		else ffc_handle.ffc->info.updateData(ffc_handle.data());
	});
}

void cpos_update() //updates with side-effects
{
	auto& combo_cache = combo_caches::cpos_update;

	for_every_rpos([&](const rpos_handle_t& rpos_handle) {
		cpos_info& timer = rpos_handle.info();
		int cid = rpos_handle.data();

		// Even though `updateData` already does this check, avoiding a function call
		// has been observed to be a performance boost.
		if (cid != timer.data)
			timer.updateData(cid);

		if(!timer.appeared)
		{
			auto& cmb = combobuf[cid];
			auto [x, y] = rpos_handle.xy();
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

		auto& mini_cmb = combo_cache.minis[cid];
		if (mini_cmb.sfx_loop)
			sfx_no_repeat(mini_cmb.sfx_loop);
		
		bool do_trigger_timer = mini_cmb.trigtimer;
		newcombo const* cmb = nullptr;
		if(do_trigger_timer)
			cmb = &combobuf[cid];
		size_t max_trig = zc_max(cmb ? cmb->triggers.size() : 0, timer.trig_data.capacity());
		for(size_t idx = 0; idx < max_trig; ++idx)
		{
			if(do_trigger_timer && idx < cmb->triggers.size())
			{
				auto& trig = cmb->triggers[idx];
				if (trig.trigtimer)
				{
					cpos_trig_info& trig_info = timer.trig_data[idx];
					if(++trig_info.clk >= trig.trigtimer)
					{
						trig_info.clk = 0;
						do_trigger_combo(rpos_handle, idx);
						timer.updateData(rpos_handle.data());
						if(rpos_handle.data() != cid)
							do_trigger_timer = false; // 'break', but only out of the trigtimer part of the loop
					}
				}
			}
			if (idx < timer.trig_data.capacity())
			{
				cpos_trig_info& trig_info = timer.trig_data[idx];
				if (trig_info.cooldown) --trig_info.cooldown;
			}
		}
		handle_cpos_type(mini_cmb.type,timer,rpos_handle);
	});

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		if (ffc_handle.ffc->flags & ffc_changer)
			return; //changers don't contribute

		ffcdata& f = *ffc_handle.ffc;
		cpos_info& timer = f.info;
		if (f.flags & ffc_changer)
		{
			//changers don't contribute
			timer.updateData(-1);
			return;
		}
		int cid = f.data;
		timer.updateData(cid);
		zfix wx = f.x + (f.txsz-1)*8;
		zfix wy = f.y + (f.tysz-1)*8;
		
		if(!timer.appeared)
		{
			auto& cmb = combobuf[cid];
			zfix wx = ffc_handle.ffc->x;
			zfix wy = ffc_handle.ffc->y;
			wx += (ffc_handle.scr->ffTileWidth(ffc_handle.i)-1)*8;
			wy += (ffc_handle.scr->ffTileHeight(ffc_handle.i)-1)*8;
			timer.appeared = true;
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

		auto& mini_cmb = combo_cache.minis[cid];
		if (mini_cmb.sfx_loop)
			sfx_no_repeat(mini_cmb.sfx_loop);
		
		bool do_trigger_timer = mini_cmb.trigtimer;
		newcombo const* cmb;
		if(do_trigger_timer)
			cmb = &combobuf[cid];
		
		for(size_t idx = 0; idx < MAX_COMBO_TRIGGERS; ++idx)
		{
			if (do_trigger_timer && idx < cmb->triggers.size())
			{
				auto& trig = cmb->triggers[idx];
				if (trig.trigtimer)
				{
					cpos_trig_info& trig_info = timer.trig_data[idx];
					if (++trig_info.clk >= trig.trigtimer)
					{
						trig_info.clk = 0;
						do_trigger_combo(ffc_handle, idx);
						timer.updateData(f.data);
						if (f.data != cid)
							do_trigger_timer = false; // 'break', but only out of the trigtimer part of the loop
					}
				}
			}
			if (idx < timer.trig_data.capacity())
			{
				cpos_trig_info& trig_info = timer.trig_data[idx];
				if (trig_info.cooldown) --trig_info.cooldown;
			}
		}
		if(mini_cmb.type == cSHOOTER)
			handle_shooter(combobuf[cid], timer, wx, wy);
	});

	trig_trigger_groups();
}

namespace combo_caches
{
	combo_cache<minicombo_type> type;
	combo_cache<minicombo_flag> flag;
	combo_cache<minicombo_cpos_update> cpos_update;
	combo_cache<minicombo_trigger_group> trigger_group;
	combo_cache<minicombo_can_cycle> can_cycle;
	combo_cache<minicombo_shutter> shutter;
	combo_cache<minicombo_gswitch> gswitch;
	combo_cache<minicombo_script> script;
	combo_cache<minicombo_drawing> drawing;
	combo_cache<minicombo_lens> lens;
	combo_cache<minicombo_spotlight> spotlight;
}
