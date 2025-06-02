#include <string>

#include "base/zdefs.h"
#include "ffc.h"
#include "zc/weapons.h"
#include "zc/guys.h"
#include "zc/zc_ffc.h"
#include "zc/zelda.h"
#include "base/zsys.h"
#include "zc/maps.h"
#include "tiles.h"
#include "pal.h"
#include "zc/hero.h"
#include "zc/ffscript.h"
#include "zc/decorations.h"
#include "drawing.h"
#include "zc/combos.h"
#include "base/zc_math.h"
#include "base/qrs.h"
#include "base/mapscr.h"
#include "base/misctypes.h"
#include "base/initdata.h"
#include "iter.h"
#include "zscriptversion.h"

#ifndef IS_EDITOR
#include "zc/render.h"
#endif

extern int32_t directWpn;
extern enemy Enemy;
extern bool show_hitboxes;

static bool CanComboTrigger(weapon *w)
{
	if(screenIsScrolling()) return false;
	int32_t wid = (w->useweapon > 0) ? w->useweapon : w->id;
	switch(wid)
	{
		case wSword: case wBeam: case wBrang: case wBomb: case wSBomb: case wLitBomb: case wLitSBomb:
		case wArrow: case wFire: case wWhistle: case wBait: case wWand: case wMagic: case wWind:
		case wRefMagic: case wRefFireball: case wRefRock: case wHammer: case wHookshot:
		case wFSparkle: case wSSparkle: case wCByrna: case wRefBeam: case wStomp:
		case wScript1: case wScript2: case wScript3: case wScript4: case wScript5:
		case wScript6: case wScript7: case wScript8: case wScript9: case wScript10:
		case ewFireball: case ewFireball2: case ewArrow: case ewBrang: case ewSword: case ewRock:
		case ewMagic: case ewBomb: case ewSBomb: case ewLitBomb: case ewLitSBomb: case ewFireTrail:
		case ewFlame: case ewWind: case ewFlame2: case wThrown:
		case wRefArrow: case wRefFire: case wRefFire2:
			return true;
	}
	return false;
}

bool weapon::no_triggers() const
{
	switch(id)
	{
		case wHammer: //Hammers don't trigger anything while in the air if item_flag1 is set!
		{
			if(parentitem < 0) break;
			itemdata const& itm = itemsbuf[parentitem];
			if(itm.family == itype_hammer && (itm.flags & item_flag1) && Hero.getHammerState() < 3)
				return true;
			break;
		}
	}
	return false;
}

static bool MatchComboTrigger(weapon *w, combo_trigger const& trig)
{
	if(screenIsScrolling()) return false;
	if(w->no_triggers()) return false;
	int32_t wid = (w->useweapon > 0) ? w->useweapon : w->id;
	
	if(w->z > 0 && (trig.triggerflags[3] & combotriggerONLY_GROUND_WPN))
		return false; // Air based weapon shouldn't trigger ground-only combo
	if(w->isLWeapon) //min/max level check
	{
		//Nothing should ever count as 'level 0'
		int lv = w->type, tlv = trig.triggerlevel;
		if(lv<1) lv=1;
		if(tlv<1) tlv=1;
		if(!((trig.triggerflags[0]&combotriggerINVERTMINMAX) ? lv <= tlv : lv >= tlv))
			return false;
	}
	bool ret = false;
	switch(wid)
	{
		case wSword: ret = (trig.triggerflags[0]&combotriggerSWORD); break;
		case wBeam: ret = (trig.triggerflags[0]&combotriggerSWORDBEAM); break;
		case wBrang: ret = (trig.triggerflags[0]&combotriggerBRANG); break;
		case wBomb: ret = (trig.triggerflags[0]&combotriggerBOMB); break;
		case wSBomb: ret = (trig.triggerflags[0]&combotriggerSBOMB); break;
		case wLitBomb: ret = (trig.triggerflags[0]&combotriggerLITBOMB); break;
		case wLitSBomb: ret = (trig.triggerflags[0]&combotriggerLITSBOMB); break;
		case wArrow: ret = (trig.triggerflags[0]&combotriggerARROW); break;
		case wFire: ret = (trig.triggerflags[0]&combotriggerFIRE); break;
		case wWhistle: ret = (trig.triggerflags[0]&combotriggerWHISTLE); break;
		case wBait: ret = (trig.triggerflags[0]&combotriggerBAIT); break;
		case wWand: ret = (trig.triggerflags[0]&combotriggerWAND); break;
		case wMagic: ret = (trig.triggerflags[0]&combotriggerMAGIC); break;
		case wWind: ret = (trig.triggerflags[0]&combotriggerWIND); break;
		case wRefMagic: ret = (trig.triggerflags[0]&combotriggerREFMAGIC); break;
		case wRefFireball: ret = (trig.triggerflags[0]&combotriggerREFFIREBALL); break;
		case wRefRock: ret = (trig.triggerflags[0]&combotriggerREFROCK); break;
		case wHammer: ret = (trig.triggerflags[0]&combotriggerHAMMER); break;
		case wHookshot: ret = (trig.triggerflags[1]&combotriggerHOOKSHOT); break;
		case wFSparkle: ret = (trig.triggerflags[1]&combotriggerSPARKLE); break;
		case wSSparkle: ret = (trig.triggerflags[1]&combotriggerSPARKLE); break;
		case wCByrna: ret = (trig.triggerflags[1]&combotriggerBYRNA); break;
		case wRefBeam: ret = (trig.triggerflags[1]&combotriggerREFBEAM); break;
		case wStomp: ret = (trig.triggerflags[1]&combotriggerSTOMP); break;
		case wThrown: ret = (trig.triggerflags[2]&combotriggerTHROWN); break;
		case wScript1: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT01) : (trig.triggerflags[2]&combotriggerEWSCRIPT01); break;
		case wScript2: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT02) : (trig.triggerflags[2]&combotriggerEWSCRIPT02); break;
		case wScript3: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT03) : (trig.triggerflags[2]&combotriggerEWSCRIPT03); break;
		case wScript4: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT04) : (trig.triggerflags[2]&combotriggerEWSCRIPT04); break;
		case wScript5: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT05) : (trig.triggerflags[2]&combotriggerEWSCRIPT05); break;
		case wScript6: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT06) : (trig.triggerflags[2]&combotriggerEWSCRIPT06); break;
		case wScript7: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT07) : (trig.triggerflags[2]&combotriggerEWSCRIPT07); break;
		case wScript8: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT08) : (trig.triggerflags[2]&combotriggerEWSCRIPT08); break;
		case wScript9: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT09) : (trig.triggerflags[2]&combotriggerEWSCRIPT09); break;
		case wScript10: ret = w->isLWeapon ? (trig.triggerflags[1]&combotriggerSCRIPT10) : (trig.triggerflags[2]&combotriggerEWSCRIPT10); break;
		case ewFireball: case ewFireball2: ret = (trig.triggerflags[0]&combotriggerEWFIREBALL); break;
		case ewArrow: ret = (trig.triggerflags[1]&combotriggerEWARROW); break;
		case ewBrang: ret = (trig.triggerflags[1]&combotriggerEWBRANG); break;
		case ewSword: ret = (trig.triggerflags[1]&combotriggerEWSWORD); break;
		case ewRock: ret = (trig.triggerflags[1]&combotriggerEWROCK); break;
		case ewMagic: ret = (trig.triggerflags[2]&combotriggerEWMAGIC); break;
		case ewBomb: ret = (trig.triggerflags[2]&combotriggerEWBBLAST); break;
		case ewSBomb: ret = (trig.triggerflags[2]&combotriggerEWSBBLAST); break;
		case ewLitBomb: ret = (trig.triggerflags[2]&combotriggerEWLITBOMB); break;
		case ewLitSBomb: ret = (trig.triggerflags[2]&combotriggerEWLITSBOMB); break;
		case ewFireTrail: ret = (trig.triggerflags[2]&combotriggerEWFIRETRAIL); break;
		case ewFlame: ret = (trig.triggerflags[2]&combotriggerEWFLAME); break;
		case ewWind: ret = (trig.triggerflags[2]&combotriggerEWWIND); break;
		case ewFlame2: ret = (trig.triggerflags[2]&combotriggerEWFLAME2); break;
		case wRefArrow: ret = (trig.triggerflags[3]&combotriggerLWREFARROW); break;
		case wRefFire: ret = (trig.triggerflags[3]&combotriggerLWREFFIRE); break;
		case wRefFire2: ret = (trig.triggerflags[3]&combotriggerLWREFFIRE2); break;
	}
	return ret;
}

int FindComboTriggerMatch(weapon *w, int combo_id, int start_idx)
{
	if(screenIsScrolling()) return false;
	if(w->no_triggers()) return false;
	newcombo const& cmb = combobuf[combo_id];
	for(size_t idx = start_idx; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig = cmb.triggers[idx];
		if(MatchComboTrigger(w, trig))
			return idx;
	}
	return -1;
}

static int32_t COMBOAT(int32_t x, int32_t y) 
{
	x = vbound(x,255,0);
	y = vbound(y,175,0);
	return (y & 240)+(x>>4);
}


void killgenwpn(weapon* w)
{
	switch(w->id)
	{
		case wSword:
		case wHammer:
			return;
		default:
			w->dead = 1;
			break;
	}
}

void do_generic_combo(const rpos_handle_t& rpos_handle, weapon *w, int32_t wid, 
	int32_t cid, int32_t flag, int32_t flag2, int32_t ft, bool single16) //WID currently is unused; if you add code relating to it, make sure to check if it's greater than 0
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	}

	int32_t pos = rpos_handle.pos;
	int32_t layer = rpos_handle.layer;
	mapscr* scr = rpos_handle.scr;
	auto [x, y] = rpos_handle.xy();

	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	bool checked = w->rposes_checked.contains({rpos_handle.layer, rpos_handle.rpos});
	if ( !checked || (combobuf[cid].usrflags&cflag5) ) 
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
			item* itm = (new item(x, y, 0, it, ipBIGRANGE + ipTIMER, 0));
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
			screen_combo_modify_postroutine(rpos_handle);
			if ( combobuf[cid].attribytes[2] > 0 )
				sfx(combobuf[cid].attribytes[2],x);
		}
		
		//loop next combo
		if((combobuf[cid].usrflags&cflag4))
		{
			do
			{
				if (layer) 
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
						scr->data[pos]=vbound(scr->data[pos]+1,0,MAXCOMBOS);
					}
					screen_combo_modify_postroutine(rpos_handle);
				}
				
				if ( combobuf[cid].usrflags&cflag8 ) w->dead = 1;
				if((combobuf[cid].usrflags&cflag12)) break; //No continuous for undercombo
				if ( (combobuf[cid].usrflags&cflag5) ) cid = ( layer ) ? MAPCOMBO2(layer,x,y) : MAPCOMBO(x,y);
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],x);
			
			
		}
		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(rpos_handle.screen,x,y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
		}
	}
	w->rposes_checked.insert({rpos_handle.layer, rpos_handle.rpos});
	
	if ( combobuf[cid].usrflags&cflag8 ) killgenwpn(w);
}

void do_generic_combo_ffc(weapon *w, const ffc_handle_t& ffc_handle, int32_t cid, int32_t ft)
{
	if ( combobuf[cid].type < cTRIGGERGENERIC && !(combobuf[cid].usrflags&cflag9 )  )  //Script combos need an 'Engine' flag
	{ 
		return;
	} 
	ft = vbound(ft, minSECRET_TYPE, maxSECRET_TYPE); //sanity guard to legal secret types. 44 to 127 are unused
	ffcdata* ffc = ffc_handle.ffc;
	if ( !w->ffcs_checked.contains(ffc) || (combobuf[cid].usrflags&cflag5) ) 
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
			else decorations.add(new comboSprite(ffc->x, ffc->y, 0, 0, combobuf[cid].attribytes[0]));
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
			item* itm = (new item(ffc->x, ffc->y,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
		
		//drop special room item
		if ( (combobuf[cid].usrflags&cflag6) && !getmapflag(ffc_handle.scr, mSPECIALITEM))
		{
			items.add(new item(ffc->x, ffc->y,
				(zfix)0,
				ffc_handle.scr->catchall,ipONETIME2|ipBIGRANGE|((itemsbuf[ffc_handle.scr->catchall].family==itype_triforcepiece ||
				(ffc_handle.scr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((ffc_handle.scr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
		}
		//screen secrets
		if ( combobuf[cid].usrflags&cflag7 )
		{
			screen_ffc_modify_preroutine(ffc_handle);
			ffc_handle.set_data(ffc_handle.scr->secretcombo[ft]);
			ffc->cset = ffc_handle.scr->secretcset[ft];
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
					ffc_handle.set_data(ffc_handle.scr->undercombo);
					ffc->cset = ffc_handle.scr->undercset;	
				}
				else
					ffc_handle.increment_data();
				
				screen_ffc_modify_postroutine(ffc_handle);
				
				if (combobuf[cid].usrflags&cflag8) w->dead = 1;
				if (combobuf[cid].usrflags&cflag12) break; //No continuous for undercombo
				if (combobuf[cid].usrflags&cflag5) cid = ffc_handle.data(); //cid needs to be set to data so continuous combos work
				
			} while((combobuf[cid].usrflags&cflag5) && (combobuf[cid].type == cTRIGGERGENERIC) && (cid < (MAXCOMBOS-1)));
			if ( (combobuf[cid].attribytes[2]) > 0 )
				sfx(combobuf[cid].attribytes[2],int32_t(ffc->x));
		}

		if((combobuf[cid].usrflags&cflag14)) //drop enemy
		{
			addenemy(ffc_handle.screen,ffc->x,ffc->y,(combobuf[cid].attribytes[4]),((combobuf[cid].usrflags&cflag13) ? 0 : -15));
		}
	}
	w->ffcs_checked.insert(ffc);
	
	if (combobuf[cid].usrflags&cflag8) killgenwpn(w);
}

//Checks if a weapon triggers a combo at a given bx/by
static void MatchComboTrigger2(weapon *w, int32_t bx, int32_t by, int32_t layer = 0/*, int32_t comboid, int32_t flag*/)
{
	if (screenIsScrolling()) return;
	if(w->weapon_dying_frame) return;
	if(unsigned(bx) > world_w-1 || unsigned(by) > world_h-1) return;
	if (!layer)
	{
		if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
		{
			for_every_ffc([&](const ffc_handle_t& ffc_handle) {
				if (ffcIsAt(ffc_handle, bx, by))
				{
					int trig_idx = -1;
					auto cid = ffc_handle.data();
					do
					{
						trig_idx = FindComboTriggerMatch(w, cid, trig_idx + 1);
						if(trig_idx >= 0)
							do_trigger_combo(ffc_handle, trig_idx, 0, w);
					} while(ffc_handle.data() == cid && trig_idx >= 0);
				}
			});
		}
	}

	//find out which combo row/column the coordinates are in
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h-1);
	bx=TRUNCATE_TILE(bx);
	by=TRUNCATE_TILE(by);
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, layer);
	int32_t cid = rpos_handle.data();
	int trig_idx = -1;
	do
	{
		trig_idx = FindComboTriggerMatch(w, cid, trig_idx + 1);
		if(trig_idx >= 0)
			do_trigger_combo(rpos_handle, trig_idx, 0, w);
	} while(rpos_handle.data() == cid && trig_idx >= 0);
}

static bool triggerfire(int x, int y, weapon* w, bool setflag, bool any, bool strong, bool magic, bool divine)
{
	if(w->no_triggers()) return false;
	int trigflags = (any?combotriggerANYFIRE:0)
		| (strong?combotriggerSTRONGFIRE:0)
		| (magic?combotriggerMAGICFIRE:0)
		| (divine?combotriggerDIVINEFIRE:0);
	if(!trigflags) return false;
	weapon* wptr = get_qr(qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS) ? nullptr : w;
	bool ret = false;
	if(any)
		ret = ret||trigger_secrets_if_flag(x,y,mfANYFIRE,setflag);
	if(strong)
		ret = ret||trigger_secrets_if_flag(x,y,mfSTRONGFIRE,setflag);
	if(magic)
		ret = ret||trigger_secrets_if_flag(x,y,mfMAGICFIRE,setflag);
	if(divine)
		ret = ret||trigger_secrets_if_flag(x,y,mfDIVINEFIRE,setflag);

	std::set<rpos_t> rposes({COMBOPOS_REGION_B(x,y),COMBOPOS_REGION_B(x,y+15),COMBOPOS_REGION_B(x+15,y),COMBOPOS_REGION_B(x+15,y+15)});
	for(int q = 0; q < 7; ++q)
	{
		for (rpos_t rpos : rposes)
		{
			if (rpos == rpos_t::None)
				continue;

			auto rpos_handle = get_rpos_handle(rpos, q);
			auto& cmb = rpos_handle.combo();

			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if (w->z > 0 && (trig.triggerflags[3] & combotriggerONLY_GROUND_WPN))
					continue; // Air based weapon shouldn't trigger ground-only combo

				if (trig.triggerflags[2] & trigflags)
				{
					auto oldcombo = rpos_handle.data();
					do_trigger_combo(rpos_handle, idx, 0, wptr);
					ret = true;
					if(rpos_handle.data() != oldcombo) break;
				}
			}
		}
	}

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		ffcdata& ffc = *ffc_handle.ffc;
		auto& cmb = ffc_handle.combo();
		for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
		{
			auto& trig = cmb.triggers[idx];
			if (w->z > 0 && (trig.triggerflags[3] & combotriggerONLY_GROUND_WPN))
				return; // Air based weapon shouldn't trigger ground-only combo

			if((trig.triggerflags[2] & trigflags) && ffc.collide(x,y,16,16))
			{
				auto oldcombo = ffc_handle.data();
				do_trigger_combo(ffc_handle, idx, 0, wptr);
				ret = true;
				if(ffc_handle.data() != oldcombo) break;
			}
		}
	});

	return ret;
}

byte boomframe[16] = {0,0,1,0,2,0,1,1,0,1,1,3,2,2,1,2};
byte bszboomflip[4] = {0,2,3,1};

//light up the screen if there is at least one "lit" weapon
//otherwise darken the screen
void checkLightSources(bool perm = false)
{
	int32_t isPerm = 0;
	if(perm)isPerm=3;
    for(int32_t i=0; i<Lwpns.Count(); i++)
    {
        if(((weapon *)Lwpns.spr(i))->isLit)
        {
            lighting(true, false, isPerm);
            return;
        }
    }
    
    for(int32_t i=0; i<Ewpns.Count(); i++)
    {
        if(((weapon *)Ewpns.spr(i))->isLit)
        {
            lighting(true, false, isPerm);
            return;
        }
    }
    
    lighting(false,false,isPerm);
}

void getdraggeditem(int32_t j)
{
    item *it=(item*)items.spr(j);
    
    if(it==NULL)
        return;
	
    it->x = HeroX();
    it->y = HeroY();
    it->fakez = HeroFakeZ();
    it->z = HeroZ();
    HeroCheckItems(j);
}

void weapon::setAngle(double angletoset)
{
    angular = true;
    angle = angletoset;
    doAutoRotate();
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>(PI/2))   dir=l_down;
    else                  dir=r_down;
}

void weapon::seekHero()
{
    angular = true;
    double _MSVC2022_tmp1, _MSVC2022_tmp2;
    angle = atan2_MSVC2022_FIX(double(HeroY()-y),double(HeroX()-x));
    doAutoRotate();
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>(PI/2))   dir=l_down;
    else                  dir=r_down;
    
    if(z>HeroZ()) z--;
    else if(z<HeroZ()) z++;
    if(fakez>HeroFakeZ()) fakez--;
    else if(fakez<HeroFakeZ()) fakez++;
}

void weapon::seekEnemy(int32_t j)
{
    angular = true;
    zfix mindistance=(zfix)1000000;
    zfix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int32_t i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(x,y,GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return;
    }

    double _MSVC2022_tmp1, _MSVC2022_tmp2;
    angle = atan2_MSVC2022_FIX(double(GuyY(j)-y),double(GuyX(j)-x));
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
}

int32_t weapon::seekEnemy2(int32_t j)
{
    angular = true;
    zfix mindistance=(zfix)1000000;
    zfix tempdistance;
    
    if((j==-1)||(j>=GuyCount()))
    {
        j=-1;
        
        for(int32_t i=0; i<GuyCount(); i++)
        {
            //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
            tempdistance=distance(dummy_fix[0],dummy_fix[1],GuyX(i),GuyY(i));
            
            if((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
            {
                mindistance=tempdistance;
                j=i;
            }
        }
    }
    
    if(j==-1)
    {
        return j;
    }
    
    double _MSVC2022_tmp1, _MSVC2022_tmp2;
    angle = atan2_MSVC2022_FIX(double(GuyY(j)-dummy_fix[1]),double(GuyX(j)-dummy_fix[0]));
    doAutoRotate();
    
    if(angle==-PI || angle==PI) dir=left;
    else if(angle==-PI/2) dir=up;
    else if(angle==PI/2)  dir=down;
    else if(angle==0)     dir=right;
    else if(angle<-PI/2)  dir=l_up;
    else if(angle<0)      dir=r_up;
    else if(angle>PI/2)   dir=l_down;
    else                  dir=r_down;
    
    return j;
}

void weapon::convertType(bool toLW)
{
	if((isLWeapon && toLW) || (!isLWeapon && !toLW)) return; //Already the right type
	//== here is unsafe!
	weaponscript = 0;
	for(int32_t q = 0; q < 8; ++q)
	{
		weap_initd[q] = 0;
	}
}

weapon::weapon(weapon const & other):
     //Struct Element			Type		Purpose
    sprite(other),
    power(other.power), 		//int32_t
    type(other.type), 			//int32_t
    dead(other.dead),			//int32_t
    clk2(other.clk2),			//int32_t
    misc2(other.misc2),			//int32_t
    ignorecombo(other.ignorecombo),	//int32_t
    isLit(other.isLit),			//bool		Does it light the screen?
    parentid(other.parentid),		//int32_t		Enemy that created it. -1 for none. This is the Enemy POINTER, not the Enemy ID. 
    parentitem(other.parentitem),	//int32_t		Item that created it. -1 for none. 
    dragging(other.dragging),		//int32_t draggong		?
    step(other.step),			//zfix		Speed of movement
    bounce(other.bounce),		//bool		Boomerang, or hookshot bounce. 
    ignoreHero(other.ignoreHero),	//bool		?
    flash(other.flash),			//word		Is it flashing?
    wid(other.wid),			//word		ID
    aframe(other.aframe),		//word		Anim frame
    csclk(other.csclk),			//word		CSet flash clk (?)
    o_speed(other.o_speed),		//int32_t		Original anim (?) speed.
    o_type(other.o_type),		//int32_t		The weapon ID (type)
    frames(other.frames),		//int32_t		Frames of the anim cycle
    o_flip(other.o_flip),		//int32_t		The original flip/orientationn
    ref_o_tile(other.ref_o_tile),	//int32_t
	script_wrote_otile(other.script_wrote_otile),
    temp1(other.temp1),			//int32_t		Misc var.
    behind(other.behind),		//bool		Should it be drawn behind Hero, NPC, and other sprites?
	autorotate(other.autorotate),
    linked_parent(other.linked_parent),	//int32_t		A flagset that determines of the weapon can collect an item.
	unblockable(other.unblockable),
	misc_wflags(other.misc_wflags),
	last_burnstate(other.last_burnstate),
    minX(other.minX),			//int32_t		How close can the weapon get tot he edge of the screen
    maxX(other.maxX),			//int32_t		...before being deleted or bouncing
    minY(other.minY),			//int32_t		...
    maxY(other.maxY),			//int32_t		...
	
    //! dimi Wand
    /*
    //!dimi: These 5 exist both here and in the header file. If you remove these, don't forget to
    remove them over there as well.
    */
    count1(other.count1), 		//int32_t		dimi Wand 
    count2(other.count2), 		//int32_t		dimi Wand 
    count3(other.count3), 		//int32_t		dimi Wand
    count4(other.count4), 		//int32_t		dimi Wand
    count5(other.count5), 		//int32_t		dimi Wand
	
    //Weapon Editor -Z
    useweapon(other.useweapon),		//byte		The weapon editor weapon type.
    usedefence(other.usedefence),	//byte		The defence type to evaluate in do_enemy_hit()
    weaprange(other.weaprange),		//int32_t		The range or distance of the weapon before removing it. 
    weapduration(other.weapduration),	//int32_t		The number of frames that must elapse before removing it
   	//word		The weapon action script. 
    tilemod(other.tilemod),		//int32_t		The Tile Mod to use when the weapon is active. 
    drawlayer(other.drawlayer),		//byte		The layer onto which we draw the weapon.
    family_class(other.family_class),	//byte		Item Class
    family_level(other.family_level),	//byte		Item Level
    flags(other.flags),			//word		A misc flagset. 
    collectflags(other.collectflags),	//int32_t		A flagset that determines of the weapon can collect an item.
    duplicates(other.duplicates),	//int32_t		A flagset that determines of the weapon can collect an item.
    quantity_iterator(other.quantity_iterator),	//int32_t		A flagset that determines of the weapon can collect an item.
//Enemy Editor Weapon Sprite
    wpnsprite(other.wpnsprite),
    specialinfo(other.specialinfo),
    ScriptGenerated(other.ScriptGenerated),
    isLWeapon(other.isLWeapon),
	linkedItem(other.linkedItem),
	//weaponscript(other.weaponscript),
	parent_uid(other.parent_uid), //Theoretical: Should the parent remain the same, or change to the weapon that spawned the copy?
	//If the cloned weapon is not getting an incremented UID for ZASM, then it needs one below.
	weapon_dying_frame(other.weapon_dying_frame),
	weap_timeout(other.weap_timeout),
	
	
	death_spawnitem(other.death_spawnitem),
	death_spawndropset(other.death_spawndropset),
	death_item_pflags(other.death_item_pflags),
	death_sprite(other.death_sprite),
	death_sfx(other.death_sfx),
	has_shadow(other.has_shadow),
	lift_level(other.lift_level),
	lift_time(other.lift_time),
	lift_height(other.lift_height)
    
	
	//End Weapon editor non-arrays. 

{
	weaponscript = other.weaponscript;
	for ( int32_t q = 0; q < 22; q++ ) wscreengrid[q] = 0;
	memset(wscreengrid_layer, 0, sizeof(wscreengrid_layer));
	for( int32_t q = 0; q < WPNSPR_MAX; q++ )
	{
		misc_wsprites[q] = other.misc_wsprites[q];
		light_rads[q] = other.light_rads[q];
	}
	for( int32_t q = 0; q < 8; q++ ) 
	{
		weap_initd[q] = other.weap_initd[q];
	}
	for(int32_t i=0; i<10; ++i)
	{
		dummy_int[i]=other.dummy_int[i];
		dummy_fix[i]=other.dummy_fix[i];
		dummy_float[i]=other.dummy_float[i];
		dummy_bool[i]=other.dummy_bool[i];
	}
    
    //Weapon Editor Arrays
    for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) 
    {
	weap_pattern[q] = other.weap_pattern[q];	//int32_t	The movement pattern and args.
    }
    for ( int32_t q = 0; q < WEAPON_CLOCKS; q++ ) 
    {
	clocks[q] = other.clocks[q];		//int32_t	An array of misc clocks. 
    }

    for ( int32_t q = 0; q < FFSCRIPT_MISC; q++ ) 
    {
	ffmisc[q] = other.ffmisc[q];		//int32_t -The base wpn->Misc[32] set from the editor
    }
    for ( int32_t q = 0; q < FFSCRIPT_MISC; q++ ) 
    {
	wpn_misc_d[q] = other.wpn_misc_d[q];		//int32_t -The base wpn->Misc[32] set from the editor
    }
    for ( int32_t q = 0; q < 128; q++ ) 
    {
	weapname[q] = 0;		//int32_t -The base wpn->Misc[32] set from the editor
    }
    script_wrote_otile = 0;
    
    doAutoRotate(true);
}

byte weapon::get_burnstate() const
{
	if(misc_wflags & WFLAG_BURN_DIVINEFIRE)
		return WPNSPR_IGNITE_DIVINE;
	if(misc_wflags & WFLAG_BURN_MAGICFIRE)
		return WPNSPR_IGNITE_MAGIC;
	if(misc_wflags & WFLAG_BURN_STRONGFIRE)
		return WPNSPR_IGNITE_STRONG;
	if(misc_wflags & WFLAG_BURN_ANYFIRE)
		return WPNSPR_IGNITE_ANY;
	return WPNSPR_BASE;
}

// Let's dispose of some sound effects!
void weapon::cleanup_sfx()
{
	//Check weapon id
    switch(id)
    {
		case wWind:
		case ewBrang:
		case wBrang:
		case wCByrna:
			break;
		case wSSparkle:
		case wFSparkle:
			if(parentitem>=0 && itemsbuf[parentitem].family==itype_cbyrna)
				break;
			return;
		default: return; //No repeating sfx
    }
    // First, check for the existence of weapons that don't have parentitems
    // but make looping sounds anyway.
    if(parentitem<0 && get_qr(qr_MORESOUNDS))
    {
        //I am reasonably confident that I fixed these expressions. ~pkmnfrk
			//No, you didn't. Now I have. -V
        if(id==ewBrang && Ewpns.idCount(ewBrang) > 1)
            return;
            
        if(id==wWind && Lwpns.idCount(wWind) > 1)
            return;
    }
    
    // Check each Lwpn to see if this weapon's sound is also allocated by it.
	int32_t use_sfx = 0;
	if(parentitem >= 0 && (itemsbuf[parentitem].family != itype_whistle || id != wWind)) use_sfx = itemsbuf[parentitem].usesound;
	else switch(id)
	{
		case ewBrang:
			use_sfx = specialsfx;
			break;
		case wBrang:
			use_sfx = WAV_BRANG;
			break;
		case wWind:
			use_sfx = WAV_ZN1WHIRLWIND;
			break;
		case wSSparkle:
		case wFSparkle:
		case wCByrna:
			use_sfx = WAV_ZN2CANE;
			break;
	}
	
    if(use_sfx)
    {
        for(int32_t i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = ((weapon *)Lwpns.spr(i));
            
            if(w->getUID() == getUID())  // Itself!
            {
                continue;
            }
            
            int32_t wparent = w->parentitem;
            
            if(wparent>=0 && (itemsbuf[wparent].family == itype_brang || itemsbuf[wparent].family == itype_divineprotection
                              || itemsbuf[wparent].family == itype_hookshot || itemsbuf[wparent].family == itype_cbyrna))
            {
                if(itemsbuf[wparent].usesound == use_sfx)
                    return;
            }
        }
    }
    
	stop_sfx(use_sfx);
	
    /*==switch(id)
    {
    case wWind:
        stop_sfx(WAV_ZN1WHIRLWIND);
        break;
        
    case ewBrang:
        //stop_sfx(WAV_BRANG); //causes a bug -L
        break;
        
    case wBrang:
    case wCByrna:
        if(parentitem>=0)
        {
            stop_sfx(itemsbuf[parentitem].usesound);
        }
        
        break;
        
    case wSSparkle:
    case wFSparkle:
        if(parentitem>=0 && itemsbuf[parentitem].family==itype_cbyrna)
        {
            stop_sfx(itemsbuf[parentitem].usesound);
        }
        
        break;
    }*/
}
void weapon::reset_wgrids()
{
	memset(wscreengrid,0,sizeof(wscreengrid));
	memset(wscreengrid_layer,0,sizeof(wscreengrid_layer));
}
weapon::~weapon()
{
	if(dragging > 0)
	{
		item* dragItem = (item*)items.spr(dragging);
		if(dragItem)
			dragItem->is_dragged = false;
	}
	FFCore.deallocateAllScriptOwned(isLWeapon ? ScriptType::Lwpn : ScriptType::Ewpn, getUID());
	cleanup_sfx();
}

void weapon::eweapon_overrides()
{
	enemy* e = (enemy*)guys.getByUID(parentid);
	if (e == NULL) return;
	if (e->weapoverrideFLAGS & OVERRIDE_TILE_WIDTH) { txsz = e->weap_tilew; }
	if (e->weapoverrideFLAGS & OVERRIDE_TILE_HEIGHT) { tysz = e->weap_tileh; }
	if (e->weapoverrideFLAGS & OVERRIDE_HIT_WIDTH) { hit_width = e->weap_hxsz; }
	if (e->weapoverrideFLAGS & OVERRIDE_HIT_HEIGHT) { hit_height = e->weap_hysz; }
	if (e->weapoverrideFLAGS & OVERRIDE_HIT_Z_HEIGHT) { hzsz = e->weap_hzsz; }
	if (e->weapoverrideFLAGS & OVERRIDE_HIT_X_OFFSET) { hxofs = e->weap_hxofs; }
	if (e->weapoverrideFLAGS & OVERRIDE_HIT_Y_OFFSET) { hyofs = e->weap_hyofs; }
	if (e->weapoverrideFLAGS & OVERRIDE_DRAW_X_OFFSET) { xofs = e->weap_xofs; }
	if (e->weapoverrideFLAGS & OVERRIDE_DRAW_Y_OFFSET) { yofs = e->weap_yofs + (get_qr(qr_OLD_DRAWOFFSET) ? playing_field_offset : original_playing_field_offset); }

	unblockable = e->wunblockable;
	moveflags = e->wmoveflags;
	//flames are complicated
	if (id == ewFlame || id == ewFlame2)
	{
		switch (e->family)
		{
		case eeGLEEOK:
			if (e->dmisc3 == 1) step = e->wstep * 2;
			break;
		case eeGHOMA:
			if (e->dmisc1 == 2) step = e->wstep * 2;
			break;
		case eePATRA: //absolutely not
			break;
		default:
			if (dir > right) step = e->wstep * .707;
		}
	}
	else step = e->wstep;
	specialsfx = e->specialsfx;
}

weapon::weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t Type,int32_t pow,int32_t Dir, int32_t Parentitem, int32_t prntid, bool isDummy, byte script_gen, byte isLW, byte special, int32_t Linked_Parent, int32_t use_sprite) : sprite(), parentid(prntid)
{
	x=X;
	y=Y;
	z=Z;
	screen_spawned=get_screen_for_world_xy(x.getInt(), y.getInt());
	id=Id;
	type=Type;
	power=pow;
	parentitem=Parentitem;
	parentid=prntid;
	dir=zc_max(Dir,0);
	clk=clk2=flip=misc=misc2=0;
	frames=flash=wid=aframe=csclk=0;
	ignorecombo=rpos_t::None;
	step=0;
	dead=-1;
	specialinfo = special;
	bounce=ignoreHero=false;
	yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) - 2;
	dragging=-1;
	hit_width=15;
	hit_height=15;
	hzsz=8;
	autorotate = false;
	do_animation = true;
	ref_o_tile = 0;
	useweapon = usedefence = useweapondummy = usedefencedummy = 0;
	weaprange = weapduration = 0;
	script_wrote_otile = 0;
	linked_parent = Linked_Parent;
	quantity_iterator = 0;
	weapon_dying_frame = false;
	weap_timeout = 0;
	parent_uid = 0;
	unblockable = 0;
	misc_wflags = WFLAG_NONE;
	last_burnstate = 0;
	death_spawnitem = -1;
	death_spawndropset = -1;
	death_sprite = -1;
	death_sfx = 0;
	lift_level = 0;
	lift_time = 16;
	lift_height = 8;
	death_item_pflags = 0;
	has_shadow = true;
	if ( Parentitem > -1 )
	{
		if(get_qr(qr_SPARKLES_INHERIT_PROPERTIES)
			|| (id != wSSparkle && id != wFSparkle))
		{
			//Sparkles shouldn't have these behaviors!
			weaponscript = itemsbuf[Parentitem].weaponscript;
			for ( int32_t q = 0; q < 8; q++ )
			{
				weap_initd[q] = itemsbuf[Parentitem].weap_initiald[q];
			}
			useweapon = itemsbuf[Parentitem].useweapon;
			usedefence = itemsbuf[Parentitem].usedefence;
		}
		if (id == wLitBomb || id == wLitSBomb) 
		{
			useweapondummy = useweapon;
			useweapon = 0;
			usedefencedummy = usedefence;
			usedefence = 0;
		}
		quantity_iterator = type; //wCByrna uses this for positioning.
		if ( id != wPhantom /*&& (id != wWind && !specialinfo)*/ && /*id != wFSparkle && id != wSSparkle &&*/ ( id < wEnemyWeapons || ( id >= wScript1 && id <= wScript10) ) )
			type = itemsbuf[Parentitem].fam_type; //the weapon level for real lweapons.
			//Note: eweapons use this for boss weapon block flags
			// Note: wInd uses type for special properties.
			//Note: wFire is bonkers. If it writes this, then red candle and above use the wrong sprites. 
	}
	
	if ( isLW ) goto skip_eweapon_script;
	if ( prntid > -1 && prntid != Hero.getUID()  ) //eweapon scripts
	{
		enemy *s = (enemy *)guys.getByUID(prntid);
		weaponscript = guysbuf[s->id & 0xFFF].weaponscript;
		parent_uid = s->getUID();
		for ( int32_t q = 0; q < 8; q++ )
		{
			weap_initd[q] = guysbuf[s->id & 0xFFF].weap_initiald[q];
		}
	}
	skip_eweapon_script:
	tilemod = 0;
	drawlayer = 0;
	family_class = family_level = 0;
	flags = 0;
	collectflags = 0;
	duplicates = 0;
	count1 = count2 = count3 = count4 = count5 = 0;
	temp1 = 0;
	scriptrange = blastsfx = wpnsprite = 0;
	for ( int32_t q = 0; q < FFSCRIPT_MISC; q++ ) ffmisc[q] = 0;
	for ( int32_t q = 0; q < 128; q++ ) weapname[q] = 0;
	
	for ( int32_t q = 0; q < FFSCRIPT_MISC; q++ ) wpn_misc_d[q] = 0;
	for ( int32_t q = 0; q < WEAPON_CLOCKS; q++ ) clocks[q] = 0;
	for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) 
	{
		weap_pattern[q] = 0; //int32_t	The movement pattern and args.
	}
	isLit = false;
	linkedItem = 0;
	for ( int32_t q = 0; q < 22; q++ ) wscreengrid[q] = 0;
		memset(wscreengrid_layer, 0, sizeof(wscreengrid_layer));
		
	ScriptGenerated = script_gen; //t/b/a for script generated swords and other HeroCLass items. 
	//This will need an input in the params! -Z
		
	isLWeapon = isLW;
	minX = minY = maxX = maxY = 0;
	rundeath = false;
	shd_aclk = shd_aframe = 0;
	
	int32_t itemid = parentitem;
	itemdata const& parent = itemsbuf[unsigned(parentitem) < MAXITEMS ? parentitem : -1];
	
	if(id>wEnemyWeapons)
	{
		canfreeze=true;
		
		if(id!=ewBrang)
		{
			if(Type&2)
			{
				misc=(Type>>3)-1;
				type &= ~2;
			}
			else
				misc=-1;
		}
	}
	
	switch(id) //Default Gravity
	{
		case ewFlame:
		case ewFireTrail:
			//weapons created by scripts need to fall too
			if(parentid >= 0)
				break;
		[[fallthrough]];
		case wFire:
			// Divine Fire shouldn't fall
			if(parentitem >= 0 && parent.family==itype_divinefire && !(parent.flags & item_flag3))
				break;
		[[fallthrough]];
		case wLitBomb:
		case wLitSBomb:
		case wBait:
		case wThrown:
			moveflags |= move_obeys_grav | move_can_pitfall;
	}
	
	switch(id) //flags
	{
		case wThrown:
			misc_wflags = WFLAG_BREAK_WHEN_LANDING;
			break;
		case wLitBomb:
		case wLitSBomb:
			if(parentitem >= 0)
			{
				if(parent.family==itype_bomb || parent.family==itype_sbomb)
				{
					if(parent.flags&item_flag3)
						misc_wflags |= WFLAG_STOP_WHEN_LANDING;
					if(parent.flags&item_flag5)
						misc_wflags |= WFLAG_STOP_WHEN_HIT_SOLID;
				}
			}
			break;
	}
	
	switch(id)
	{
		case wScript1: case wScript2: case wScript3: case wScript4: case wScript5:
		case wScript6: case wScript7: case wScript8: case wScript9: case wScript10:
		case wIce:
		{
			if(isLWeapon)
			{
				if(parentitem >-1)
				{
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
				}
			}
			break;
		}
		case wSword:
		{
			hit_width=hit_height=15;
			if ( parentitem > -1 )
			{
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_sword);
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 )
				{
					extend = 3; 
					
					switch(dir)
					{
						case up:
						{
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
							/* yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) == yofs+56.
							It is needed for the passive subscreen offset.
							*/
							yofs-=16;
							break;
						}
						case down:
						{
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
							/* yofs+playing_field_offset == yofs+56.
							It is needed for the passive subscreen offset.
							*/
							break;
						}
						case left:
						{
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tileh;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tilew;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hysz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hxsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hyofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hxofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_yofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_xofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
							/* yofs+playing_field_offset == yofs+56.
							It is needed for the passive subscreen offset.
							*/
							xofs-=16;
							break;
						}
						case right:
						{
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tileh;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tilew;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hysz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hxsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hyofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hxofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_yofs;}
							if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_xofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
							/* yofs+playing_field_offset == yofs+56.
							It is needed for the passive subscreen offset.
							*/
							break;
						}
					}
				}
			}
			break;
		}
		case wWand:
		{
			hit_width=hit_height=15;
			break;
		}
		case wHammer:
		{
			hit_width=15;
			hit_height=24;
			if ( parentitem > -1 )
			{
				if(isDummy || itemid<0)
					itemid = getCanonicalItemID(itemsbuf, itype_hammer);
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 )
				{
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			break;
		}
		case wCByrna: // The Cane's beam
		{
			if(isDummy || itemid<0)
				itemid = getCanonicalItemID(itemsbuf, itype_cbyrna);
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			int32_t speed = parentitem>-1 ? zc_max(parent.misc1,1) : 1;
			int32_t qty = parentitem>-1 ? zc_max(parent.misc3,1) : 1;
			clk = (int32_t)((((2*quantity_iterator*PI)/qty)
						 // Appear on top of the cane's hook
						 + (dir==right? 3*PI/2 : dir==left? PI/2 : dir==down ? 0 : PI))*speed);
			quantity_iterator = 0;
			
			if(parentitem>-1)
				cont_sfx(parent.usesound);
			
			break;
		}
		case wWhistle:
		{
			xofs=1000;                                            // don't show
			x=y=hxofs=hyofs=0;
			hit_width=hit_height=255;                                        // hit the whole screen
			//Port Item Editor Weapon Size Values
		
			if(isDummy || itemid<0)
				itemid = getCanonicalItemID(itemsbuf, itype_whistle);
			
			if ( parentitem > -1 )
			{
				//Whistle damage
				if ((parent.flags & item_flag2)!=0 ) //Flags[1]
				{
					power = parent.misc5; //Attributews[5]
				}
				
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			break;
		}
		case wWind:
		{
			if(isDummy || itemid<0)
				itemid = getCanonicalItemID(itemsbuf, itype_whistle);
			clk=-14;
			step=2;
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
				}
			}
			
			if(get_qr(qr_MORESOUNDS) && dead != 1 && dead != 2)
				cont_sfx(WAV_ZN1WHIRLWIND);
			break;
		}
		case wBeam: case wRefBeam:
		{
			step = 3;
			
			if(isDummy || itemid<0)
				itemid = getCanonicalItemID(itemsbuf, itype_sword);
			
			if(id==wRefBeam)
			{
				ignorecombo=COMBOPOS_REGION_B(x, y);
			}
			switch(dir)
			{
				case down:
				case up:
					hyofs=2;
					hit_height=12;
					break;
					
				case left:
				case right:
					hxofs=2;
					hit_width=12;
					yofs = (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+(BSZ ? 3 : 1);
					break;
			}
			break;
		}
		case wArrow:
		case wRefArrow:
		{
			if(isDummy || itemid<0)
				itemid = getCanonicalItemID(itemsbuf, itype_arrow);
			step = id == wRefArrow ? 2 : 3;
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}    
			if(itemid >-1)
				misc = itemsbuf[itemid].misc1;
			if ( do_animation ) 
			{
				switch(dir)
				{
					case down:
					case up:
						hyofs= ( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) ? parent.weap_hyofs : 2;
						hit_height= ( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) ? parent.weap_hysz : 12;
						break;
						
					case left:
					case l_down:
					case right:
						yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) + 1;
						hyofs= ( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) ? parent.weap_hyofs : 2;
						hit_height= ( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) ? parent.weap_hysz : 14;
						hxofs=( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) ? parent.weap_hxofs : 2;
						hit_width=( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ) ? parent.weap_hxsz : 12;
						break;
				}
			}
			break;
		}
		case wSSparkle: case wFSparkle:
		{
			step=0;
			break;
		}
		case wFire:
		case wRefFire:
		case wRefFire2:
		{
			glowRad = game->get_light_rad(); //Default light radius for fires
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( parent.weapoverrideFLAGS > 0 )
				{
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
				}
				
				switch(parent.family)
				{
					case itype_divinefire: // Divine Fire. This uses magicitem rather than itemid
						step = 0; 
						if(itemsbuf[magicitem].flags & item_flag2)
							glowRad = 0;
						break;
						
					case itype_book:
						step = 0;
						break;
						
					case itype_candle:
						if(parent.flags & item_flag2)
							glowRad = 0;
						hxofs = hyofs = 1;
						hit_width = hit_height = 14;
						step = zfix(parent.misc4)/100;
						break;
					default:
						step = 0;
				}
			}
			else step = 0.5;
			if(BSZ)
				yofs+=2;
			break;
		}
		case wLitBomb: case wBomb:
		{
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_bomb);
			hxofs=hyofs=4;
			hit_width=hit_height=8;
			
			if(itemid > -1)
			{
				misc = (id==wBomb ? 1 : itemsbuf[itemid].misc1);
				if(id == wLitBomb && itemsbuf[itemid].misc4)
				{
					lift_level = itemsbuf[itemid].misc4;
					lift_time = itemsbuf[itemid].misc5;
					lift_height = itemsbuf[itemid].misc6;
				}
			}
			else
			{
				misc = (id==wBomb ? 1 : 50);
			}
			//This may not work for bombs, as they need special size data. We need a 'Special Size' tab.
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 )
				{
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			break;
		}
		case wLitSBomb: case wSBomb:
		{
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_sbomb);
			hxofs=hyofs=4;
			hit_width=hit_height=8;
		
			if ( parentitem > -1 )
			{
				misc = (id==wSBomb ? 1 : itemsbuf[itemid].misc1);
				if(id == wLitSBomb && itemsbuf[itemid].misc4)
				{
					lift_level = itemsbuf[itemid].misc4;
					lift_time = itemsbuf[itemid].misc5;
					lift_height = itemsbuf[itemid].misc6;
				}
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			else
			{
				misc = (id==wSBomb ? 1 : 50);
			}
			break;
		}
		case wBait:
		{
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_bait);
			
			misc2 = itemsbuf[itemid].misc2;
			
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			break;
		}
		case wMagic:
		{
			itemid = current_item_id(itype_book);
			
			if(itemid < 0)
				itemid = directWpn > -1 ? directWpn : current_item_id(itype_wand);
			
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_wand);
			if ( itemid > -1 )
			{
				// Z_message("Flags: %d\n",itemsbuf[itemid].weapoverrideFLAGS);
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = itemsbuf[itemid].weap_tilew;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = itemsbuf[itemid].weap_tileh;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = itemsbuf[itemid].weap_hxsz;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = itemsbuf[itemid].weap_hysz;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itemsbuf[itemid].weap_hzsz;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = itemsbuf[itemid].weap_hxofs;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = itemsbuf[itemid].weap_hyofs;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = itemsbuf[itemid].weap_xofs;}
					if ( itemsbuf[itemid].weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = itemsbuf[itemid].weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			step = (BSZ ? 3 : 2.5);
			switch(dir)
			{
				case down:
				case up:
					hyofs = ( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) ? parent.weap_hyofs : 2;
					hit_height=( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) ? parent.weap_hysz : 12;
					break;
					
				case left:
				case right:
					hxofs=( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) ? parent.weap_hxofs : 2;
					hit_width=( (parentitem > -1) && parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ) ? parent.weap_hxsz : 12;
					break;
			}
			break;
		}
		case wBrang:
		{
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_brang);
			hxofs=4;
			hit_width=7;
			hyofs=2;
			hit_height=11;
			if ( parentitem > -1 )
			{
				//Port Item Editor Weapon Size Values
				if ( itemsbuf[itemid].weapoverrideFLAGS > 0 ) {
					extend = 3; 
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_WIDTH ) { txsz = parent.weap_tilew;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_TILE_HEIGHT ){  tysz = parent.weap_tileh;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_WIDTH ){  hit_width = parent.weap_hxsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_HEIGHT ) {  hit_height = parent.weap_hysz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Z_HEIGHT ) {  hzsz = parent.weap_hzsz;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_X_OFFSET ) {  hxofs = parent.weap_hxofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_HIT_Y_OFFSET ) { hyofs = parent.weap_hyofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_X_OFFSET ) { xofs = parent.weap_xofs;}
					if ( parent.weapoverrideFLAGS&OVERRIDE_DRAW_Y_OFFSET ) {  yofs = parent.weap_yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);}
					/* yofs+playing_field_offset == yofs+56.
					It is needed for the passive subscreen offset.
					*/
				}
			}
			dummy_bool[0]=false; //grenade armed?
			break;
		}
		case wHookshot:
		{
			hookshot_used=true;
			hs_switcher = family_class == itype_switchhook;
			
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, family_class);
			
			itemdata const& hshot = itemsbuf[parentitem>-1 ? parentitem : current_item_id(family_class)];
			step = 4;
			clk2=256;
			
			//Size Tab Settings here would be broken, IMO. -Z
			switch(dir)
			{
				case down:
					xofs+=4;
					yofs+=1;
					hyofs=2;
					hit_height=12;
					break;
					
				case up:
					yofs+=3;
					xofs-=5;
					hyofs=2;
					hit_height=12;
					break;
					
				case left:
					xofs+=2;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					hxofs=2;
					hit_width=12;
					break;
					
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					xofs-=2;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					hxofs=2;
					hit_width=12;
					break;
				//Diagonal Hookshot (1)
				case l_up:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case r_down:
					yofs+=3; //check numbers ater
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case l_down:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case r_up:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
			}
			break;
		}
		case wHSHandle:
		{
			step = 0;
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_hookshot);
			
			switch(dir)
			{
				case down:
					xofs+=4;
					yofs+=1;
					hyofs=2;
					hit_height=12;
					break;
					
				case up:
					yofs+=3;
					xofs-=5;
					hyofs=2;
					hit_height=12;
					break;
					
				case left:
					xofs+=2;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					hxofs=2;
					hit_width=12;
					break;
					
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					xofs-=2;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					hxofs=2;
					hit_width=12;
					break;
			
				//Diagonal Hookshot (5)
				case r_down:
					yofs+=3; //check numbers ater
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case l_down:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case r_up:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
				case l_up:
					yofs+=3;
					xofs-=3;
					hit_height=12;
					hit_width=12;
					break;
			}
			break;
		}
		case wHSChain:
		{
			if(isDummy || itemid < 0)
				itemid = getCanonicalItemID(itemsbuf, itype_hookshot);
			step = 0;
			switch(dir)
			{
				case down:
					xofs+=4;
					yofs-=7;
					break;
					
				case up:
					xofs-=5;
					yofs+=11;
					break;
					
				case left:
					xofs+=10;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					break;
					
				case right:
					xofs-=10;
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+4;
					break;
				
				case r_up:
					xofs-=10;
					yofs+=7;
					break;
				case r_down:
					xofs-=10;
					yofs-=7;
					break;
				case l_up:
					xofs+=10;
					yofs+=7;
					break;
				case l_down:
					xofs+=10;
					yofs-=7;
					break;
			}
			break;
		}
		
		//EWeapons
		case ewLitBomb: case ewBomb:
		{
			hxofs=0;
			hit_width=16;
			
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hyofs=0;
				hit_height=16;
			}
			
			if(id==ewBomb)
				misc=2;
			else
				step=3;
			specialsfx = WAV_BOMB;
			break;
		}
		case ewLitSBomb: case ewSBomb:
		{
			hxofs=0;
			hit_width=16;
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hyofs=0;
				hit_height=16;
			}
			
			if(id==ewSBomb)
				misc=2;
			else
				step=3;
			specialsfx = WAV_BOMB;
			break;
		}
		case ewBrang:
		{
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=0;
				hit_width=16;
				hyofs=0;
				hit_height=16;
			}
			else
			{
				hxofs=4;
				hit_width=8;
			}
			specialsfx = WAV_BRANG;
			break;
		}
		case ewFireball2:
		{
			if(dir < 8)
				misc = dir;
			else
				misc = -1;
		}
		[[fallthrough]];
		case ewFireball: case wRefFireball:
		{
			step=1.75;
			
			if(Type&2)
			{
				seekHero();
			}
			else misc=-1;
			break;
		}
		case ewRock:
		{
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=0;
				hit_width=16;
				hyofs=0;
				hit_height=16;
			}
			else
			{
				hxofs=4;
				hit_width=8;
			}
			
			step=3;
			break;
		}
		case ewArrow:
		{
			step=2;
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
				case r_up:
				case l_up:
				case up:
					xofs=-4;
					hit_width=8;
					break;
					
				case left:
				case right:
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+1;
					break;
			}
			break;
		}
		case ewSword:
		{
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=0;
				hit_width=16;
				hyofs=0;
				hit_height=16;
			}
			else
			{
				hxofs=4;
				hit_width=8;
			}
			
			step=3;
			switch(dir)
			{
				case left:
				case right:
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+1;
					break;
			}
			break;
		}
		case wRefMagic: case ewMagic:
		{
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=0;
				hit_width=16;
				hyofs=0;
				hit_height=16;
			}
			else
			{
				hxofs=4;
				hit_width=8;
			}
			
			step=3;
			switch(dir)
			{
				case left:
				case right:
					yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)+1;
					break;
			}
			if(id==wRefMagic)
				ignorecombo=COMBOPOS_REGION_B(x, y);
			break;
		}
		case ewFlame: case ewFlame2:
		{
			if(get_qr(qr_EW_FIRE_EMITS_LIGHT))
				glowRad = game->get_light_rad();
			if(dir==255)
			{
				step=2;
				seekHero();
			}
			else
			{
				if(dir>right)
					step = .707;
				else
					step = 1;
			}
			
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=hyofs=0;
				hit_width=hit_height=16;
			}
			else
			{
				hxofs = hyofs=1; // hof of 1 means that hero can use the 'half-tile trick'.
				hit_width = hit_height = 14;
			}
			
			if(BSZ)
				yofs+=2;
			break;
		}
		case ewFireTrail:
		{
			if (get_qr(qr_EW_FIRE_EMITS_LIGHT))
				glowRad = game->get_light_rad();
			step=0;
			dir=-1;
			
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=hyofs=0;
				hit_width=hit_height=16;
			}
			else
			{
				hxofs = hyofs=1; // hof of 1 means that hero can use the 'half-tile trick'.
				hit_width = hit_height = 15;
			}
			
			if(BSZ)
				yofs+=2;
			break;
		}
		case ewWind:
		{
			if(get_qr(qr_OFFSETEWPNCOLLISIONFIX))
			{
				hxofs=hyofs=0;
				hit_width=hit_height=16;
			}
			clk=0;
			step=3;
			break;
		}
		case wPhantom:
		{
			switch(type)
			{
				case pDIVINEFIREROCKET:
					if(get_qr(qr_MORESOUNDS))
						sfx(WAV_ZN1ROCKETUP,(int32_t)x);
					step = 4;
					break;
					
				case pDIVINEFIREROCKETRETURN:
					if(get_qr(qr_MORESOUNDS))
						sfx(WAV_ZN1ROCKETDOWN,(int32_t)x);
					step = 4;
					break;
					
				case pDIVINEPROTECTIONROCKET1:
					if(get_qr(qr_MORESOUNDS))
						sfx(WAV_ZN1ROCKETUP,(int32_t)x);
						
					step = 4;
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETRETURN1:
					if(get_qr(qr_MORESOUNDS))
						sfx(WAV_ZN1ROCKETDOWN,(int32_t)x);
						
					step = 4;
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETTRAIL1:
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETTRAILRETURN1:
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKET2:
					step = 4;
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETRETURN2:
					step = 4;
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETTRAIL2:
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEPROTECTIONROCKETTRAILRETURN2:
					drawstyle=parent.flags & item_flag2 ? 1 : 0;
					break;
					
				case pDIVINEFIREROCKETTRAIL:
				case pDIVINEFIREROCKETTRAILRETURN:
				case pMESSAGEMORE:
				default:
					break;
			}
			break;
		}
	}

	if (parentitem > -1)
	{
		//lweapon_overrides(); SOON
	}
	else if (parentid > -1 && !isLWeapon)
	{
		eweapon_overrides();
	}
	
	if(id>wEnemyWeapons && id!=ewBrang && (Type&4)!=0)  // Increase speed of Aquamentus 2 fireballs
	{
		step *=2;
	}

	for(int q = 0; q < WPNSPR_MAX; ++q)
	{
		misc_wsprites[q] = 0;
		light_rads[q] = 0;
	}
	
	optional<byte> wpnspr;
	
	switch(id) //burn sprites
	{
		case wSword:
		case wWand:
		case wHSHandle:
		case wHSChain:
		case wSSparkle:
		case wFSparkle:
			break; //These weapon types don't support burn sprites
		default:
			if(parentitem > -1) //weapons created by items
			{
				if(parent.flags & item_burning_sprites)
				{
					misc_wflags |= WFLAG_UPDATE_IGNITE_SPRITE;
					for (int q = 0; q < WPNSPR_MAX; ++q)
					{
						misc_wsprites[q] = parent.burnsprs[q];
						light_rads[q] = parent.light_rads[q];
					}
					last_burnstate = get_burnstate();
					wpnspr = _handle_loadsprite(misc_wsprites[last_burnstate]);
					glowRad = light_rads[last_burnstate];
				}
				else light_rads[WPNSPR_BASE] = glowRad;

			}
			else if (parentid > -1 && !isLWeapon) //weapons created by enemies
			{
				enemy* e = (enemy*)guys.getByUID(parentid);
				if (e->flags & guy_burning_sprites)
				{
					misc_wflags |= WFLAG_UPDATE_IGNITE_SPRITE;
					for (int q = 0; q < WPNSPR_MAX; ++q)
					{
						misc_wsprites[q] = e->burnsprs[q];
						light_rads[q] = e->light_rads[q];
					}
					last_burnstate = get_burnstate();
					wpnspr = _handle_loadsprite(misc_wsprites[last_burnstate]);
					glowRad = light_rads[last_burnstate];
				}
				else light_rads[WPNSPR_BASE] = glowRad;
			}
			else light_rads[WPNSPR_BASE] = glowRad;
			break;
	}
	if(!wpnspr)
	{
		if(use_sprite > -1)
			wpnspr = _handle_loadsprite(use_sprite, isDummy, true);
		else wpnspr = _handle_loadsprite(nullopt, isDummy, true);
	}
}
optional<byte> weapon::_ewpn_sprite(int parentid) const
{
	if(parentid > -1 && !isLWeapon)
		if(enemy *e = (enemy*)guys.getByUID(parentid))
			if(e->wpnsprite > 0)
				return e->wpnsprite;
	return nullopt;
}

optional<byte> weapon::_handle_loadsprite(optional<byte> spr, bool isDummy, bool force)
{
	optional<byte> ret;
	if(force) ret = 0;
	itemdata const& parent = itemsbuf[unsigned(parentitem) < MAXITEMS ? parentitem : -1];
	switch(id)
	{
		case wBugNet:
		{
			if(spr)
				ret = *spr;
			else ret = parent.wpn;
			LOADGFX(*ret);
			break;
		}
		case wScript1: case wScript2: case wScript3: case wScript4: case wScript5:
		case wScript6: case wScript7: case wScript8: case wScript9: case wScript10:
		case wIce:
		{
			if(isLWeapon)
			{
				if(spr)
					ret = *spr;
				else if(parentitem >-1)
					ret = parent.wpn;
				LOADGFX(*ret);
				if(parentitem > -1)
				{
					if(!(parent.flags & item_flag1))
					{
						switch(dir)
						{
							case up:
							{
								flip = 0; break;
							}
							case down:
							{
								flip = 2; break;
							}
							case left:
							{
								flip = 7; break;
							}
							case right:
							{
								flip = 4; break;
							}
							default: flip = 0; break;
						}
					}
				}
			}
			else
			{
				if(spr)
					ret = *spr;
				else if ( parentid > -1 )
				{
					enemy *e = (enemy*)guys.getByUID(parentid);
					if(e && e->wpnsprite > 0)
						ret = e->wpnsprite;
				}
				if(ret)
					LOADGFX(*ret);
			}
			break;
		}
		case wSword: // Hero's sword
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_sword);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else ret = wSWORD;
			}
			LOADGFX(*ret);
			break;
		}
		case wWand: // Hero's wand, as well as the Cane itself
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_wand);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else ret = wWAND;
			}
			LOADGFX(*ret);
			break;
		}
		case wHammer:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_hammer);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else ret = wHAMMER;
			}
			LOADGFX(*ret);
			break;
		}
		case wCByrna: // The Cane's beam
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_cbyrna);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn3;
				else ret = wCBYRNA;
			}
			LOADGFX(*ret);
			break;
		}
		case wWind:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_whistle);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else ret = wWIND;
			}
			LOADGFX(*ret);
			break;
		}
		case wBeam: case wRefBeam:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_whistle);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn3;
				else ret = ewSWORD;
			}
			LOADGFX(*ret);
			flash = 1;
			cs = 6;
			switch(dir)
			{
				case down:
					flip=get_qr(qr_SWORDWANDFLIPFIX)?3:2;
					break;
				case up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case wArrow:
		case wRefArrow:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_arrow);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else ret = wARROW;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
					flip=2;
					break;
				case up:
					break;
					
				case left:
				case l_down:
					flip=1;
				[[fallthrough]];
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case wSSparkle:
		{
			if(spr)
				ret = *spr;
			else ret = linked_parent ? linked_parent : wSSPARKLE;
			LOADGFX(*ret);
			break;
		}
		case wFSparkle:
		{
			if(spr)
				ret = *spr;
			else ret = linked_parent ? linked_parent : wFSPARKLE;
			LOADGFX(*ret);
			break;
		}
		case wFire:
		case wRefFire:
		case wRefFire2:
		{
			if(spr)
				ret = *spr;
			else if ( parentitem > -1 )
			{
				switch(parent.family)
				{
					case itype_divinefire: // Divine Fire. This uses magicitem rather than itemid
						if(magicitem >-1 && !isDummy)
							ret = itemsbuf[magicitem].wpn5;
						else ret = wFIRE;
						break;
						
					case itype_book:
						ret = isDummy ? wFIRE : parent.wpn2;
						break;
						
					case itype_candle:
						ret = (parentid > -1 && !isDummy) ? parent.wpn3 : wFIRE;
						break;
				}
			}
			else
				ret = wFIRE;
			LOADGFX(*ret);
			break;
		}
		case wLitBomb:
		case wBomb:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_bomb);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else
					ret = wBOMB;
			}
			LOADGFX(*ret);
			break;
		}
		case wLitSBomb:
		case wSBomb:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_sbomb);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else
					ret = wSBOMB;
			}
			LOADGFX(*ret);
			break;
		}
		case wBait:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_bait);
				if(itemid > -1)
					ret = itemsbuf[itemid].wpn;
				else
					ret = wBAIT;
			}
			LOADGFX(*ret);
			break;
		}
		case wMagic:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = current_item_id(itype_book);
				bool book = true;
				
				if(itemid < 0)
				{
					itemid = directWpn > -1 ? directWpn : current_item_id(itype_wand);
					book = false;
				}
				
				if(isDummy || itemid < 0)
				{
					itemid = getCanonicalItemID(itemsbuf, itype_wand);
					book = false;
				}
				
				if(itemid > -1)
					// Book Magic sprite is wpn, Wand Magic sprite is wpn3.
					ret = book ? itemsbuf[itemid].wpn : itemsbuf[itemid].wpn3;
				else
					ret = wMAGIC;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
					flip=2;
					break;
				case up:
					break;
				case left:
					flip=1;
				[[fallthrough]];
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case wBrang:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, itype_brang);
				if(itemid >-1)
					ret = itemsbuf[itemid].wpn;
				else
					ret = wBRANG;
			}
			LOADGFX(*ret);
			break;
		}
		case wHookshot:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, family_class);
				if(itemid >-1)
				{
					if(dir > 3 && dir < 8)
						ret = itemsbuf[itemid].wpn5; //diagonal
					else ret = itemsbuf[itemid].wpn;
				}
				else
					ret = wHSHEAD;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
					flip=2;
					break;
					
				case up:
					break;
					
				case left:
					flip=1;
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
					
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
				//Diagonal Hookshot (1)
				case l_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=0;
					break;
				case r_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=3;
					break;
				case l_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=2;
					break;
				case r_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=1;
					break;
			}
			break;
		}
		case wHSHandle:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, family_class);
				if(itemid >-1)
				{
					if(dir > 3 && dir < 8)
						ret = itemsbuf[itemid].wpn6; //diagonal
					else ret = itemsbuf[itemid].wpn4;
				}
				else
					ret = wHSHANDLE;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
					flip=2;
					break;
					
				case up:
					break;
					
				case left:
					flip=1;
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
					
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			
				//Diagonal Hookshot (5)
				case r_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=3;
					break;
				case l_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=2;
					break;
				case r_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=1;
					break;
				case l_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=0;
					break;
			}
			break;
		}
		case wHSChain:
		{
			if(spr)
				ret = *spr;
			else
			{
				int itemid = parentitem;
				if(isDummy || itemid < 0)
					itemid = getCanonicalItemID(itemsbuf, family_class);
				if(itemid >-1)
				{
					if(dir > 3 && dir < 8)
						ret = itemsbuf[itemid].wpn7; //diagonal
					else if(dir < left)
						ret = itemsbuf[itemid].wpn3;
					else ret = itemsbuf[itemid].wpn2;
				}
				else
					ret = dir < left ? wHSCHAIN_V : wHSCHAIN_H;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case up:
				case left:
				case right:
					break;
				
				case r_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=1;
					break;
				case r_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=3;
					break;
				case l_up:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=0;
					break;
				case l_down:
					update_weapon_frame(((frames>1)?frames:0),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					flip=2;
					break;
			}
			break;
		}
		//EWeapons
		case ewLitBomb: case ewBomb:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewBOMB;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
					flip=2;
					break;
				case l_up:
				case r_up:
				case up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case ewLitSBomb: case ewSBomb:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewSBOMB;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
					flip=2;
					break;
				case l_up:
				case r_up:
				case up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case ewBrang:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = zc_min(zc_max(current_item(itype_brang),1),3)-1+wBRANG;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewFireball: case ewFireball2: case wRefFireball:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewFIREBALL;
				if(id != wRefFireball)
					if(auto val = _ewpn_sprite(parentid))
						ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewRock:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewROCK;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewArrow:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewARROW;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
					flip=2;
					break;
				case r_up:
				case l_up:
				case up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case ewSword:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewSWORD;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
					flip=2;
					break;
				case up:
				case r_up:
				case l_up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right:
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case wRefMagic: case ewMagic:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewMAGIC;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
					flip=2;
					break;
				case up:
				case r_up:
				case l_up:
					break;
					
				case left:
					flip=1;
				[[fallthrough]];
				case right: /*tile=o_tile+((frames>1)?frames:1)*/
					update_weapon_frame(((frames>1)?frames:1),o_tile);
					if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
					break;
			}
			break;
		}
		case ewFlame:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewFLAME;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewFlame2:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewFLAME2;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewFireTrail:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewFIRETRAIL;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case ewWind:
		{
			if(spr)
				ret = *spr;
			else
			{
				ret = ewWIND;
				if(auto val = _ewpn_sprite(parentid))
					ret = *val;
			}
			LOADGFX(*ret);
			break;
		}
		case wPhantom:
		{
			if(spr)
				ret = *spr;
			else
			{
				switch(type)
				{
					case pDIVINEFIREROCKET:
					case pDIVINEPROTECTIONROCKET1:
						ret = parent.wpn;
						break;
						
					case pDIVINEFIREROCKETRETURN:
					case pDIVINEPROTECTIONROCKETRETURN1:
						ret = parent.wpn2;
						break;
						
					case pDIVINEFIREROCKETTRAIL:
					case pDIVINEPROTECTIONROCKETTRAIL1:
						ret = parent.wpn3;
						break;
						
					case pDIVINEFIREROCKETTRAILRETURN:
					case pDIVINEPROTECTIONROCKETTRAILRETURN1:
						ret = parent.wpn4;
						break;
						
					case pMESSAGEMORE:
						ret = iwMore;
						break;
						
					case pDIVINEPROTECTIONROCKET2:
						ret = parent.wpn6;
						break;
						
					case pDIVINEPROTECTIONROCKETRETURN2:
						ret = parent.wpn7;
						break;
						
					case pDIVINEPROTECTIONROCKETTRAIL2:
						ret = parent.wpn8;
						break;
						
					case pDIVINEPROTECTIONROCKETTRAILRETURN2:
						ret = parent.wpn9;
						break;
						
					default:
						tile = 0;
						ret = nullopt;
						break;
				}
			}
			if(ret)
				LOADGFX(*ret);
			break;
		}
		default:
			if(spr)
				ret = *spr;
			if(ret)
				LOADGFX(*ret);
			break;
	}
	return ret;
}

bool weapon::isHeroWeapon()
{
	if ( isLWeapon > 0 ) return true;
	if ( id < lwMax ) return true;
	if ( id < wEnemyWeapons && isLWeapon > 0 ) return true;
	return false;
}
bool weapon::isHeroMelee()
{
	int32_t family = itemsbuf[parentitem].family;
	if ( family == itype_sword && id != wBeam ) return true;
	//if ( id == wBeam )  return true;
	return false;
}


void weapon::LOADGFX(int32_t wpn)
{
    if(wpn<0)
        return;
        
    wid = wpn;
    flash = wpnsbuf[wid].misc&3;
    tile  = wpnsbuf[wid].tile;
    cs = wpnsbuf[wid].csets&15;
    o_tile = wpnsbuf[wid].tile;
    tile = o_tile;
    ref_o_tile = o_tile;
    o_cset = wpnsbuf[wid].csets;
    o_flip=(wpnsbuf[wid].misc>>2)&3;
    o_speed = wpnsbuf[wid].speed;
    o_type = wpnsbuf[wid].type;
    frames = wpnsbuf[wid].frames;
    temp1 = wpnsbuf[wFIRE].tile;
    behind = (wpnsbuf[wid].misc&WF_BEHIND)!=0;
}
void weapon::LOADGFX_CMB(int32_t cid, int32_t cset)
{
    if(unsigned(cid)>=MAXCOMBOS)
        return;
	newcombo const& cmb = combobuf[cid];
	
	flash = 0;
	cs = vbound(cset,0,15);
	o_tile = cmb.o_tile;
	tile = cmb.tile;
	ref_o_tile = o_tile;
	o_cset = cset;
	o_flip= cmb.flip;
	o_speed = cmb.speed;
	o_type = 0;
	frames = cmb.frames;
}

bool weapon::Dead()
{
    return dead!=-1;
}

bool weapon::isScriptGenerated()
{
	return (ScriptGenerated != 0);
}

// Returns true if the weapon is outside the world bounds, give or a
// take a few pixels depending on the weapon/if in a dungeon.
// Also, does a viewport bounds check for player weapons.
bool weapon::clip()
{
    int32_t c[4];
    int32_t d2=isdungeon(screen_spawned);
    int32_t nb1 = get_qr(qr_NOBORDER) ? 16 : 0;
    int32_t nb2 = get_qr(qr_NOBORDER) ? 8 : 0;

    if (id<wEnemyWeapons)
    {
        if (x+txsz*16<viewport.left()||y+tysz*16<viewport.top()||x>viewport.right()||y>viewport.bottom())
            return true;
    }

    if(id>wEnemyWeapons && id!=ewBrang)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = world_h - (d2?48:32-nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = world_w - (d2?48:32-nb1);
    }
    else if(id==wHookshot||id==wHSChain)
    {
        c[0] = d2?8:0;
        c[1] = world_h - (d2?24:16);
        c[2] = d2?8:0;
        c[3] = world_w - (d2?8:0);
    }
    else
    {
        c[0] = d2?18:2;
        c[1] = world_h - (d2?32:16);
        c[2] = d2?20:4;
        c[3] = world_w - (d2?36:20);
    }
    
    if(id==wSSparkle || id==wFSparkle)
    {
        c[0] = 0;
        c[1] = world_h;
        c[2] = 0;
        c[3] = world_w;
    }
    
    if(id==ewFlame)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    
    if(id==ewFireTrail)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    
    if(id==ewWind)
    {
        c[0] = d2?32:(16-nb1);
        c[1] = d2?128:(144+nb1);
        c[2] = d2?32:(16-nb1);
        c[3] = d2?208:(224+nb1);
    }
    /*
    if (id==wArrow && get_qr(qr_ARROWCLIP))
    {
        c[0] = d2?14:2;
        c[1] = d2?146:160;
        c[2] = d2?14:4;
        c[3] = d2?226:236;
    }*/
    
    if(x < c[2])
        if(dir==left || dir==l_up || dir==l_down)
            return true;
            
    if(x > c[3])
        if(dir==right || dir==r_up || dir==r_down)
            return true;
            
    if(y < c[0])
        if(dir==up || dir==l_up || dir==r_up)
            return true;
            
    if(y > c[1])
        if(dir==down || dir==l_down || dir==r_down)
            return true;
    
    if(id>wEnemyWeapons)
    {
        if((x<(8-nb2) && dir==left)
                || (y<(8-nb2) && dir==up)
                || (x>(world_w-24+nb2) && dir==right)
                || (y>(world_h-8+nb2) && dir==down))
            return true;
    }
    
    if(x<0||y<0||x>world_w-16||y>world_h)
        return true;
        
    return false;
}

bool weapon::blocked()
{
    return blocked(8, 8);
}

bool weapon::blocked(int32_t xOffset, int32_t yOffset)
{
    int32_t wx = x+xOffset;
    int32_t wy = y+yOffset;
    
    if(id == wPhantom || id == wHSHandle || id == wHSChain)  // Sanity check
    {
        return false;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
	    //Add lw->Level check here. -Z
        if(parentitem<0 || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
                            itemsbuf[parentitem].fam_type))
        {
            return true;
        }
    }
    
    switch(dir)
    {
    case up:
        wy-=8;
        break;
        
    case down:
        wy+=8;
        break;
        
    case left:
        wx-=8;
        break;
        
    case right:
        wx+=8;
        break;
    }
    
    if(get_bit(combo_class_buf[COMBOTYPE(wx,wy)].block_weapon,id)
            || get_bit(combo_class_buf[FFCOMBOTYPE(wx,wy)].block_weapon, id))
    {
        if(parentitem<0 || (combo_class_buf[COMBOTYPE(wx,wy)].block_weapon_lvl >=
                            itemsbuf[parentitem].fam_type))
        {
            return true;
        }
    }
    
    return false;
}


void collectitem_script(int32_t id);

static const int bombxoff[] = {-16, -8, -8, 0,  8,  8, 16};
static const int bombyoff[] = {  0,-16, 16, 0,-16, 16,  0};
static const int sbombxoff[] = { -8,  8,-24, -8,  8, 24,-32,-16,  0, 16, 32,-24, -8,  8, 24, -8,  8};
static const int sbombyoff[] = {-32,-32,-16,-16,-16,-16,  0,  0,  0,  0,  0, 16, 16, 16, 16, 32, 32};
static const int bombcount = 7;
static const int sbombcount = 17;
std::set<rpos_t> weapon::getBombPositions()
{
	std::set<rpos_t> rposes;
	#define CHECKED_INSERT(rpos) if (rpos != rpos_t::None) rposes.insert(rpos);
	int parentid = parentitem < 0 ? -1 : parentitem;
	itemdata const& itm = itemsbuf[parentid];
	if(parentid < 0 || itm.misc7 < 1) //standard pattern
	{
		bool sbomb = id == wSBomb || id == ewSBomb;
		for(int q = 0; q < (sbomb ? sbombcount : bombcount); ++q)
		{
			int tx = x+(sbomb?sbombxoff:bombxoff)[q];
			int ty = y-fakez+(sbomb?sbombyoff:bombyoff)[q];
			
			CHECKED_INSERT(COMBOPOS_REGION_B(tx,ty));
			CHECKED_INSERT(COMBOPOS_REGION_B(tx+15,ty));
			CHECKED_INSERT(COMBOPOS_REGION_B(tx,ty+15));
			CHECKED_INSERT(COMBOPOS_REGION_B(tx+15,ty+15));
		}
	}
	else //radius
	{
		int rad = itm.misc7;
		int tx = x, ty = y-fakez;
		CHECKED_INSERT(COMBOPOS_REGION_B(tx+8,ty+8)); //always hits at least 1 combo

		int cx = tx / 16;
		int cy = ty / 16;
		int rad_num_tiles = std::ceil(rad / 16.0);
		int x_min = cx - rad_num_tiles;
		int y_min = cy - rad_num_tiles;
		int x_max = cx + rad_num_tiles;
		int y_max = cy + rad_num_tiles;

		for (int y = y_min; y <= y_max; y++)
		{
			for (int x = x_min; x <= x_max; x++)
			{
				int x0 = x * 16;
				int y0 = y * 16;
				if (distance(tx, ty, x0, y0) <= rad)
					CHECKED_INSERT(COMBOPOS_REGION_B(x0, y0));
			}
		}
	}

	#undef CHECKED_INSERT
	return rposes;
}

static uint flame_count()
{
	static std::set<int32_t> flametypes = {wFire, wRefFire, ewFlame};
	return Lwpns.idCount(flametypes) + Ewpns.idCount(flametypes);
}

void weapon::limited_animate()
{
	switch(id)
	{
		case wLitBomb:
		case wBomb:
		case wLitSBomb:
		case wSBomb:
		{
			if(!misc)
				break;
			
			bool fixboom = !get_qr(qr_BROKEN_MOVING_BOMBS);
			bool canboom = (fixboom || step==0);
			if(clk==(misc-2) && canboom)
			{
				id = parentitem>-1 ? ((itemsbuf[parentitem].family==itype_sbomb) ? wSBomb:wBomb)
						  : (id==wLitSBomb||id==wSBomb ? wSBomb : wBomb);
				hxofs=2000;
				step = 0;
				rundeath = true;
				if(fixboom) moveflags &= ~move_obeys_grav;
			}
			
			if(clk==(misc-1) && canboom)
			{
				sfx(parentitem<0 ? WAV_BOMB : itemsbuf[parentitem].usesound,pan(int32_t(x)));
				
				if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
				{
					hxofs=hyofs=-16;
					hit_width=hit_height=48;
				}
				else
				{
					hxofs=hyofs=-8;
					hit_width=hit_height=32;
				}
				usedefence = usedefencedummy;
				useweapon = useweapondummy;
				hzsz=16;
				step = 0;
				if(fixboom) moveflags &= ~move_obeys_grav;
			}
			
			int32_t boomend = misc+((parentitem>-1 && itemsbuf[parentitem].flags & item_flag1) ? 35 : 31);
								  
			if(clk==boomend && canboom)
			{
				hxofs=2000;
				step = 0;
				if(fixboom) moveflags &= ~move_obeys_grav;
			}
			
			if(clk==(misc-1))
			{
				if(get_qr(qr_OLD_BOMB_HITBOXES))
				{
					int32_t f1 = (id==wSBomb || id==wLitSBomb) ? 16 : 0; // Large SBomb triggerbox
					
					for(int32_t tx=-f1; tx<=f1; tx+=8)  // -16,-8,0,8,16
					{
						int32_t f2 = 0;
						
						if(tx==-8 || tx==8)
							f2 = f1;
							
						for(int32_t ty=-f2; ty<=f2; ty+=32)
						{
							trigger_secrets_if_flag(x+tx,y+ty+(isSideViewGravity()?2:-3),mfBOMB,true);
							
							if(id==wSBomb || id==wLitSBomb)
							{
								trigger_secrets_if_flag(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSBOMB,true);
							}
							
							trigger_secrets_if_flag(x+tx,y+ty+(isSideViewGravity()?2:-3),mfSTRIKE,true);
						}
					}
				}
				else
				{
					bool sbomb = (id==wSBomb || id==wLitSBomb);
					std::set<rpos_t> rposes = getBombPositions();
					for (rpos_t rpos : rposes)
					{
						auto [x, y] = COMBOXY_REGION(rpos);
						trigger_secrets_if_flag(x,y,mfBOMB,true);
						if(sbomb) trigger_secrets_if_flag(x,y,mfSBOMB,true);
						trigger_secrets_if_flag(x,y,mfSTRIKE,true);
					}
				}
			}
			
			if(!get_qr(qr_NOBOMBPALFLASH) && !flash_reduction_enabled(false))
			{
				if(!usebombpal)
				{
					if(clk==misc || clk==misc+5)
					{
					
						usebombpal=true;
						memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
						
						//grayscale entire screen
						if(get_qr(qr_FADE))
						{
							for(int32_t i=CSET(0); i < CSET(15); i++)
							{
								int32_t g = zc_min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
								g = (g >> 1) + 32;
								g = _rgb_scale_6[g];
								RAMpal[i] = _RGB(g,g,g);
							}
						}
						else
						{
							// this is awkward. NES Z1 converts colors based on the global
							// NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
							for(int32_t i=CSET(0); i < CSET(15); i++)
							{
								RAMpal[i] = NESpal(reverse_NESpal(RAMpal[i]) & 0x30);
							}
						}
						
						refreshpal = true;
					}
				}
				
				if((clk==misc+4 || clk==misc+9) && usebombpal)
				{
					// undo grayscale
					usebombpal=false;
					memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
					refreshpal = true;
				}
			}
			
			if(clk==misc+30)
			{
				bombdoor(x,y);
			}
			
			if(clk==misc+34)
			{
				if(canboom)
				{
					dead=1;
				}
			}
			break;
		}
	}
	if(rundeath) do_death_fx();
}
bool weapon::animate(int32_t index)
{
	if(dead != 0) weapon_dying_frame = false; //reset dying frame if weapon revived
	if(switch_hooked)
	{
		if(isLWeapon)
		{
			//Run its script
			if (runscript_do_earlyret(run_script(MODE_NORMAL)))
			{
				return 0; //Avoid NULLPO if this object deleted itself
			}
		}
		solid_update(false);
		return false;
	}
	if(fallclk > 0)
	{
		if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		if(!--fallclk)
		{
			if(!weapon_dying_frame && get_qr(qr_WEAPONS_EXTRA_FRAME))
			{
				if(id==wSword || id==wBrang)
				{
					return true;
				}
				dead = 0;
				weapon_dying_frame = true;
				++fallclk;
				
				if(isLWeapon)
					run_script(MODE_NORMAL);
				
				solid_update(false);
				return false;
			}
			return true;
		}
		
		wpndata& spr = wpnsbuf[QMisc.sprites[sprFALL]];
		cs = spr.csets & 0xF;
		int32_t fr = spr.frames ? spr.frames : 1;
		int32_t spd = spr.speed ? spr.speed : 1;
		int32_t animclk = (PITFALL_FALL_FRAMES-fallclk);
		tile = spr.tile + zc_min(animclk / spd, fr-1);
		
		if(isLWeapon)
			run_script(MODE_NORMAL);
		
		solid_update(false);
		return false;
	}
	if(drownclk > 0)
	{
		//if(drownclk == WATER_DROWN_FRAMES && drownCombo) sfx(combobuf[drownCombo].attribytes[0], pan(x.getInt()));
		//!TODO: Drown SFX
		if(!--drownclk)
		{
			if(!weapon_dying_frame && get_qr(qr_WEAPONS_EXTRA_FRAME))
			{
				if(id==wSword || id==wBrang)
				{
					return true;
				}
				dead = 0;
				weapon_dying_frame = true;
				++drownclk;
				
				if(isLWeapon)
					run_script(MODE_NORMAL);
				
				solid_update(false);
				return false;
			}
			return true;
		}
		
		bool lava = (drownCombo && combobuf[drownCombo].usrflags&cflag1);
		wpndata &spr = wpnsbuf[QMisc.sprites[lava ? sprLAVADROWN : sprDROWN]];
		cs = spr.csets & 0xF;
		int32_t fr = spr.frames ? spr.frames : 1;
		int32_t spd = spr.speed ? spr.speed : 1;
		int32_t animclk = (WATER_DROWN_FRAMES-drownclk);
		tile = spr.tile + zc_min(animclk / spd, fr-1);
		
		if(isLWeapon)
			run_script(MODE_NORMAL);
		
		solid_update(false);
		return false;
	}
	// do special timing stuff
	bool hooked=false;
	if(misc_wflags & WFLAG_UPDATE_IGNITE_SPRITE)
	{
		auto burnstate = get_burnstate();
		if(last_burnstate != burnstate)
		{
			last_burnstate = burnstate;
			_handle_loadsprite(misc_wsprites[burnstate]);
			glowRad = light_rads[burnstate];
		}
	}
	if(misc_wflags & WFLAG_PICKUP_ITEMS) //Weapon grabs touched items, giving them to the player, similar to engine melee weapons.
	{
		zfix wx = x+hxofs;
		zfix wy = y+hyofs-fakez;
		auto wxsz = hit_width, wysz = hit_height;
		for(int32_t j=0; j<items.Count(); ++j)
		{
			item* ptr = (item*)items.spr(j);
			
			if((itemsbuf[ptr->id].family == itype_bottlefill) && !game->canFillBottle())
				continue; //No picking these up unless you have a bottle to fill!
			int32_t pickup = ptr->pickup;
			if((pickup & ipCANGRAB) || (pickup & ipTIMER))
			{
				if(((pickup & ipCANGRAB) || ptr->clk2 >= 32) && !ptr->fallclk && !ptr->drownclk)
				{
					if(ptr->hit(wx,wy,z,wxsz,wysz,1))
					{
						int screen = get_screen_for_world_xy(wx, wy);
						mapscr* scr = get_scr_for_world_xy(wx, wy);

						int32_t pickup = ptr->pickup;
						int32_t id2 = ptr->id;
						int32_t pstr = ptr->pstring;
						int32_t pstr_flags = ptr->pickup_string_flags;
						int32_t pstr_screen = ptr->screen_spawned;
						
						std::vector<int32_t> &ev = FFCore.eventData;
						ev.clear();
						ev.push_back(id2*10000);
						ev.push_back(pickup*10000);
						ev.push_back(pstr*10000);
						ev.push_back(pstr_flags*10000);
						ev.push_back(0);
						ev.push_back(ptr->getUID());
						ev.push_back((isLWeapon ? GENEVT_ICTYPE_MELEE : GENEVT_ICTYPE_MELEE_EW)*10000);
						ev.push_back(getUID());
						
						throwGenScriptEvent(GENSCR_EVENT_COLLECT_ITEM);
						bool nullify = ev[4] != 0;
						if(nullify) continue;
						id2 = ev[0]/10000;
						pickup = (pickup&(ipCHECK|ipDUMMY)) | (ev[1]/10000);
						pstr = ev[2] / 10000;
						pstr_flags = ev[3] / 10000;
						
						if(pickup&ipONETIME) // set mITEM for one-time-only items
							setmapflag(scr, mITEM);
						else if(pickup&ipONETIME2) // set mSPECIALITEM flag for other one-time-only items
							setmapflag(scr, (screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
						
						if(pickup&ipSECRETS)								// Trigger secrets if this item has the secret pickup
						{
							if(scr->flags9&fITEMSECRETPERM) setmapflag(scr, mSECRET);
							trigger_secrets_for_screen(TriggerSource::ItemsSecret, scr, false);
						}
						//!DIMI
						
						collectitem_script(id2);
						
						getitem(id2, false, true);
						if(ptr->pickupexstate > -1 && ptr->pickupexstate < 32)
							setxmapflag(screen, 1<<ptr->pickupexstate);
						items.del(j);
						
						for(int32_t i=0; i<Lwpns.Count(); i++)
						{
							weapon *w2 = (weapon*)Lwpns.spr(i);
							
							if(w2->dragging==j)
							{
								w2->dragging=-1;
							}
							else if(w2->dragging>j)
							{
								w2->dragging-=1;
							}
						}
						
						if ( (pstr > 0 && pstr < msg_count) )
						{
							if ( ( (!(pstr_flags&itemdataPSTRING_IP_HOLDUP)) && ( pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(id2))) ) ) )
							{
								if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) )
									FFCore.SetItemMessagePlayed(id2);
								donewmsg(get_scr(pstr_screen), pstr);
								break;
							}
						}
						
						--j;
					}
				}
			}
		}
	}
	if(misc_wflags & WFLAG_BURNFLAGS)
		triggerfire(x,y,this,true,
			misc_wflags&WFLAG_BURN_ANYFIRE,
			misc_wflags&WFLAG_BURN_STRONGFIRE,
			misc_wflags&WFLAG_BURN_MAGICFIRE,
			misc_wflags&WFLAG_BURN_DIVINEFIRE);
	//Only lweapons
	if ( id < wEnemyWeapons || ( id >= wScript1 && id <= wScript10 && isLWeapon ) ) 
	{
		/*
		if ( useweapon == wSword )
		{
		if(dir==up && ((int32_t(x)&15)==0))
		{
			Hero.check_slash_block2((int32_t)x,(int32_t)y);
			Hero.check_slash_block2((int32_t)x,(int32_t)y+8);
		}
		else if(dir==up && ((int32_t(x)&15)==8||Hero.diagonalMovement))
		{
			Hero.check_slash_block2((int32_t)x,(int32_t)y);
			Hero.check_slash_block2((int32_t)x,(int32_t)y+8);
			Hero.check_slash_block2((int32_t)x+8,(int32_t)y);
			Hero.check_slash_block2((int32_t)x+8,(int32_t)y+8);
		}
		
		if(dir==down && ((int32_t(x)&15)==0))
		{
			Hero.check_slash_block2((int32_t)x,(int32_t)y+(int32_t)hysz-8);
			Hero.check_slash_block2((int32_t)x,(int32_t)y+(int32_t)hysz);
		}
		else if(dir==down && ((int32_t(x)&15)==8||Hero.diagonalMovement))
		{
			Hero.check_slash_block2((int32_t)x,(int32_t)y+hysz-8);
			Hero.check_slash_block2((int32_t)x,(int32_t)y+hysz);
			Hero.check_slash_block2((int32_t)x+8,(int32_t)y+hysz-8);
			Hero.check_slash_block2((int32_t)x+8,(int32_t)y+hysz);
		}
		
		if(dir==left)
		{
			Hero.check_slash_block2((int32_t)x,(int32_t)y+8);
			Hero.check_slash_block2((int32_t)x+8,(int32_t)y+8);
		}
		
		if(dir==right)
		{
			Hero.check_slash_block2((int32_t)x+hxsz,(int32_t)y+8);
			Hero.check_slash_block2((int32_t)x+hxsz-8,(int32_t)y+8);
		}
		
		}*/

		// https://discord.com/channels/876899628556091432/976887183518625883/976887186454618152
		byte temp_screengrid[22];
		byte temp_screengrid_layer[2][22];
		std::map<ffcdata*, bool> temp_recently_hit;
		memcpy(temp_screengrid, screengrid, sizeof(screengrid));
		memcpy(temp_screengrid_layer[0], screengrid_layer[0], sizeof(screengrid_layer[0]));
		memcpy(temp_screengrid_layer[1], screengrid_layer[1], sizeof(screengrid_layer[1]));
		
		for(int32_t q=0; q<22; q++)
		{
			screengrid[q] = 0;
			screengrid_layer[0][q] = 0;
			screengrid_layer[1][q] = 0;
		}

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			temp_recently_hit[ffc_handle.ffc] = ffc_handle.ffc->recently_hit;
		});
		
		bool pound = useweapon == wHammer && id != wHammer;
		
		for(int32_t dx = 0; dx < hit_width; dx += 16)
		{
			for(int32_t dy = 0; dy < hit_height; dy += 16)
			{
				Hero.check_slash_block2((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this);
				//Layers
				//1
				Hero.check_slash_block_layer2((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this,1);
				//2
				Hero.check_slash_block_layer2((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this,2);
				
				Hero.check_wand_block2((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this);
				if(pound) Hero.check_pound_block((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this);
				Hero.check_wpn_triggers((int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, this);
			}
			Hero.check_slash_block2((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this);
			Hero.check_slash_block_layer2((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this,1);
			Hero.check_slash_block_layer2((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this,2);
			Hero.check_wand_block2((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this);
			if(pound) Hero.check_pound_block((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this);
			Hero.check_wpn_triggers((int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, this);
		}
		for(int32_t dy = 0; dy < hit_height; dy += 16)
		{
			Hero.check_slash_block2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this);
			Hero.check_slash_block_layer2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this,1);
			Hero.check_slash_block_layer2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this,2);
			Hero.check_wand_block2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this);
			if(pound) Hero.check_pound_block((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this);
			Hero.check_wpn_triggers((int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, this);
		}
		Hero.check_slash_block2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this);
		Hero.check_slash_block_layer2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this,1);
		Hero.check_slash_block_layer2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this,2);
		Hero.check_wand_block2((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this);
		if(pound) Hero.check_pound_block((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this);
		Hero.check_wpn_triggers((int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, this);
		
		findcombotriggers();
		
		memcpy(screengrid, temp_screengrid, sizeof(screengrid));
		memcpy(screengrid_layer[0], temp_screengrid_layer[0], sizeof(screengrid_layer[0]));
		memcpy(screengrid_layer[1], temp_screengrid_layer[1], sizeof(screengrid_layer[1]));
		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			auto it = temp_recently_hit.find(ffc_handle.ffc);
			if (it != temp_recently_hit.end())
				ffc_handle.ffc->recently_hit = it->second;
		});
	}
	else findcombotriggers();
	
	// fall down
	if ( moveflags & move_obeys_grav ) // from above, or if scripted
	{
		if(isSideViewGravity())
		{
			if((!get_qr(qr_BROKEN_SIDEVIEW_SPRITE_JUMP) && ((fall>=0&&!_walkflag(x,y+16,0))||fall<0)) ||
				(get_qr(qr_BROKEN_SIDEVIEW_SPRITE_JUMP) && !_walkflag(x, y+16, 0)))
			{
				y+=fall/100;
				
				if(fall <= (int32_t)zinit.terminalv)
				{
					fall += (zinit.gravity/100);
				}
			}
			else
			{
				if(fall!=0 && !(step>0 && dir==up))  // Don't fix pos if still moving through solidness
				{
					y.doFloor();
					y-=(int32_t)y%8; // Fix position
				}
					
				fall = 0;
				
				if(misc_wflags & WFLAG_BREAK_WHEN_LANDING) //Die
				{
					collision_check();
					dead = 0;
				}
				if(misc_wflags & WFLAG_STOP_WHEN_LANDING) //Stop movement
					step = 0;
			}
			
			if (y > world_h + 16) dead=0;  // Out of bounds
		}
		else
		{
			if (!(moveflags & move_no_fake_z))
			{
				bool didfall = fakez > 0;
				fakez-=fakefall/100;
			
				if(fakez <= 0)
				{
					fakez = fakefall = 0;
					if(didfall)
					{
						if(misc_wflags & WFLAG_BREAK_WHEN_LANDING) //Die
						{
							collision_check();
							dead = 0;
						}
						if(misc_wflags & WFLAG_STOP_WHEN_LANDING) //Stop movement
							step = 0;
					}
				}
				else if(fakefall <= (int32_t)zinit.terminalv)
				{
					fakefall += (zinit.gravity/100);
				}
			}
			if (!(moveflags & move_no_real_z))
			{
				bool didfall = z > 0;
				z-=fall/100;
				
				if(z <= 0)
				{
					z = fall = 0;
					if(didfall)
					{
						if(misc_wflags & WFLAG_BREAK_WHEN_LANDING) //Die
						{
							collision_check();
							dead = 0;
						}
						if(misc_wflags & WFLAG_STOP_WHEN_LANDING) //Stop movement
							step = 0;
					}
				}
				else if(fall <= (int32_t)zinit.terminalv)
				{
					fall += (zinit.gravity/100);
				}
			}
		}
	}
	if(moveflags & move_can_pitfall)
	{
		switch(id)
		{
			case wSword:
			case wWand:
			case wCByrna:
			case wHammer:
			case wHookshot:
			case wWhistle:
			case wFSparkle:
			case wHSChain:
			case wHSHandle:
			case wSSparkle:
			case wStomp:
			case wSmack:
				break;
			default:
				if(z <= 0 && fakez <= 0)
				{
					fallCombo = check_pits();
				}
		}
	}
	if(moveflags & move_can_waterdrown)
	{
		switch(id)
		{
			case wSword:
			case wWand:
			case wCByrna:
			case wHammer:
			case wHookshot:
			case wWhistle:
			case wFSparkle:
			case wHSChain:
			case wHSHandle:
			case wSSparkle:
			case wStomp:
			case wSmack:
				break;
			default:
				if(z <= 0 && fakez <= 0)
				{
					drownCombo = check_water();
				}
		}
	}
	
	if(id>wEnemyWeapons && id!=ewBrang && id != ewFireball2 && id != ewBomb && id != ewSBomb)
		switch(misc)
		{
			case up:
				y-=.5;
				break;
				
			case down:
				y+=.5;
				break;
				
			case left:
				x-=.5;
				break;
				
			case right:
				x+=.5;
				break;
				
			case 4:
				y-=1;
				break;
				
			case 5:
				y+=1;
				break;
				
			case 6:
				x-=1;
				break;
				
			case 7:
				x+=1;
				break;
			//case l_up:  y-=.354; x-=.354; break;
			//case r_up:  y-=.354; x+=.354; break;
			//case l_down:y+=.354; x-=.354; break;
			//case r_down:y+=.354; x+=.354; break;
		}
	
	bool AngleReflect = (this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS) && !get_qr(qr_ANGULAR_REFLECT_BROKEN));
	
	switch(id)
	{
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
		{
			if ( ScriptGenerated && !isLWeapon ) break; //Return early for eweapons. We handle those elsewhere. 
			if ( parentitem > -1 || (isLWeapon && ScriptGenerated) )
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			
			
			break;
		}
		// Hero's weapons
		case wThrown:
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			break;
		case wSword:
			if ( itemsbuf[parentitem].misc10 == 50 )
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
		[[fallthrough]];
		case wWand:
		case wHammer:
		case wBugNet:
		{
			if (HeroAction() != attacking && HeroAction() != sideswimattacking && HeroAction() != ischarging && !HeroCharged())
			{
				dead = 0;
			}

			break;
		}
		
		case wCByrna:
		{
			if(blocked())
			{
				dead=0;
			}
			
			int32_t speed = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc1,1) : 1;
			int32_t radius = parentitem>-1 ? zc_max(itemsbuf[parentitem].misc2,8) : 8;
			double xdiff = -(zc::math::Sin((double)clk/speed) * radius);
			double ydiff = (zc::math::Cos((double)clk/speed) * radius);
			
			double ddir=atan2(double(ydiff),double(xdiff));
			
			if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
				dir=r_up;
			else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
				dir=right;
			else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
				dir=r_down;
			else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
				dir=down;
			else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
				dir=l_down;
			else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
				dir=left;
			else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
				dir=l_up;
			else
				dir=up;
			
			doAutoRotate(true);
				
			x = (zfix)((double)HeroX() + xdiff);
			y = (zfix)((double)HeroY() + ydiff);
			fakez = HeroFakeZ();
			z = HeroZ();
			
			if(parentitem>-1 && dead != 1) //Perhaps don't play the sound if the weapon is dead?
			{
				sfx(itemsbuf[parentitem].usesound,pan(int32_t(x)),true,false);
			}
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;

			break;
		}
		
		case wBeam:
		case wRefBeam:
		{
			for(int32_t i2=0; i2<=zc_min(type-1,3) && dead!=23; i2++)
			{
				if(trigger_secrets_if_flag(x,y,mfSWORDBEAM+i2,true)) dead=23;
			}
			
			if(blocked())
			{
				dead=23;
			}		 
			
		}
		[[fallthrough]];
		case ewSword:
		{
			if (blocked())
			{
				dead = 0;
			}

			if (id == ewSword && get_qr(qr_SWORDMIRROR) || id != ewSword && (parentitem > -1 ? itemsbuf[parentitem].flags & item_flag9 : get_qr(qr_SWORDMIRROR))) //TODO: First qr_SWORDMIRROR port to enemy weapon flag, second qr_SWORDMIRROR port to script default flag -V
			{
				zfix checkx = 0, checky = 0;
				int32_t check_x_ofs = 0, check_y_ofs = 0;

				if (get_qr(qr_MIRRORS_USE_WEAPON_CENTER))
				{
					checkx = (x + hxofs + (hit_width * 0.5));
					checky = (y + hyofs + (hit_height * 0.5) - fakez);
					check_x_ofs = x - (checkx - 8);
					check_y_ofs = y - (checky - 8);
				}
				else
				{
					switch (dir)
					{
					case up:
						checkx = x + 7;
						checky = y + 8;
						break;

					case down:
						checkx = x + 7;
						checky = y;
						break;

					case left:
						checkx = x + 8;
						checky = y + 7;
						break;

					case right:
						checkx = x;
						checky = y + 7;
						break;
					}
					checky -= fakez;
				}

				if (ignorecombo != rpos_t::None && ignorecombo == COMBOPOS_REGION_B(checkx, checky))
					break;

				int32_t posx, posy;
				if (get_qr(qr_OLDMIRRORCOMBOS))//Replace this conditional with an ER; true if the ER is checked. This will use the old (glitchy) behavior for sword beams.
				{
					posx = x;
					posy = y;
				}
				else
				{
					posx = checkx;
					posy = checky;
				}
				byte layers = get_qr(qr_MIRROR_PRISM_LAYERS) ? 0b1111111 : 0b0000001;
				bool fix_mirror_anim = false;
				if (hitcombo(checkx, checky, cMIRROR, layers))
				{
					id = wRefBeam;
					dir ^= 1;
					if (dir & 2)
						flip ^= 1;
					else
						flip ^= 2;
						
					ignoreHero = false;
					ignorecombo = COMBOPOS_REGION_B(checkx, checky);
					x = (int32_t)TRUNCATE_TILE(posx) + check_x_ofs;
					y = (int32_t)TRUNCATE_TILE(posy) + check_y_ofs;
				}

				if (hitcombo(checkx, checky, cMIRRORSLASH, layers))
				{
					id = wRefBeam;
					doAutoRotate(true);
					dir = 3 - dir;
					fix_mirror_anim = true;
					ignoreHero = false;
					ignorecombo = COMBOPOS_REGION_B(checkx, checky);
					x = (int32_t)TRUNCATE_TILE(posx) + check_x_ofs;
					y = (int32_t)TRUNCATE_TILE(posy) + check_y_ofs;
				}

				if (hitcombo(checkx, checky, cMIRRORBACKSLASH, layers))
				{
					id = wRefBeam;
					dir ^= 2;
					fix_mirror_anim = true;
					ignoreHero = false;
					ignorecombo = COMBOPOS_REGION_B(checkx, checky);
					x = (int32_t)TRUNCATE_TILE(posx) + check_x_ofs;
					y = (int32_t)TRUNCATE_TILE(posy) + check_y_ofs;
				}

				if (hitcombo(checkx, checky, cMAGICPRISM, layers))
				{
					int32_t newx, newy;
					newx = (int32_t)TRUNCATE_TILE(posx) + check_x_ofs;
					newy = (int32_t)TRUNCATE_TILE(posy) + check_y_ofs;
					
					for (int32_t tdir = 0; tdir < 4; tdir++)
					{
						//AngleToDir(double ddir)
						//This didn't check for these before with the angle reflect rule... -Deedee
						if ((dir != (tdir ^ 1) && !AngleReflect) || (tdir != 2 && AngleReflect))
						{
							weapon* w = new weapon(*this);
							w->dir = tdir;
							w->doAutoRotate(true);
							//jesus fuck Zoria, this is blatantly wrong...
							//In your next job, don't code while drunk you dumbass. -Deedee
							if (this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS))
							{
								double newangle = this->angle + DegreesToRadians(90 * tdir);
								w->angle = WrapAngle(newangle);
								if (AngleReflect)
								{
									//Zoria, you need to turn on angular.
									w->angular = true;
									//Zoria, you need to set the dir... *sigh*
									w->dir = AngleToDir(WrapAngle(newangle));
									w->doAutoRotate();
									//That's not to mention that the scope above checked for direction... on an angular weapon. Come on, that'll result in buggy behavior.
									//Did you even fucking test this?
									//No, of course you didn't. -Deedee
								}
							}
							w->o_tile = ref_o_tile;
							w->tile = ref_o_tile;
							w->x = newx;
							w->y = newy;
							w->fakez = fakez;
							w->z = z;
							w->id = wRefBeam;
							w->parentid = parentid;
							w->parentitem = parentitem;
							w->ignorecombo = COMBOPOS_REGION_B(checkx, checky);
							w->hyofs = w->hxofs = 0;
							//also set up the magic's correct animation -DD
							w->flip = 0;
							if (do_animation)
							{
								switch (w->dir)
								{
								case down:
									w->flip = 2;

								case up:
									w->tile = w->o_tile;
									w->hyofs = 2;
									w->hit_height = 12;
									break;

								case left:
									w->flip = 1;

								case right:
									w->tile = w->o_tile + ((w->frames > 1) ? w->frames : 1);
									w->hxofs = 2;
									w->hit_width = 12;
									break;

								default: break;
								}
							}
							Lwpns.add(w);
						}
					}

					dead = 0;
				}

				if (hitcombo(checkx, checky, cMAGICPRISM4, layers))
				{
					int32_t newx, newy;
					newx = (int32_t)TRUNCATE_TILE(posx) + check_x_ofs;
					newy = (int32_t)TRUNCATE_TILE(posy) + check_y_ofs;
					
					for (int32_t tdir = 0; tdir < 4; tdir++)
					{
						weapon* w = new weapon(*this);
						w->dir = tdir;
						w->doAutoRotate(true);
						if (this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS))
						{
							double newangle = this->angle + DegreesToRadians(90 * tdir);
							w->angle = WrapAngle(newangle);
							if (AngleReflect)
							{
								w->angular = true;
								w->doAutoRotate();
								w->dir = AngleToDir(WrapAngle(newangle));
							}
						}
						w->o_tile = ref_o_tile;
						w->tile = ref_o_tile;
						w->x = newx;
						w->y = newy;
						w->z = z;
						w->fakez = fakez;
						w->id = wRefBeam;
						w->parentid = parentid;
						w->parentitem = parentitem;
						w->hyofs = w->hxofs = 0;
						w->ignorecombo = COMBOPOS_REGION_B(checkx, checky);
						//also set up the magic's correct animation -DD
						w->flip = 0;
						if (do_animation)
						{
							switch (w->dir)
							{
							case down:
								w->flip = 2;

							case up:
								w->tile = w->o_tile;
								w->hyofs = 2;
								w->hit_height = 12;
								break;

							case left:
								w->flip = 1;

							case right:
								w->tile = w->o_tile + ((w->frames > 1) ? w->frames : 1);
								w->hxofs = 2;
								w->hit_width = 12;
								break;

							default: break;
							}
						}
						Lwpns.add(w);
					}

					dead = 0;
				}

				auto newmirror = gethitcombo(checkx, checky, cMIRRORNEW, layers);
				if (newmirror > -1)
				{
					newcombo const& cmb = combobuf[newmirror];
					id = wRefBeam;
					byte newdir = cmb.attribytes[NORMAL_DIR(dir)];
					if (newdir > 7)
					{
						dead = 0;
						break;
					}
					dir = newdir;
					fix_mirror_anim = true;
					ignoreHero = false;
					ignorecombo = COMBOPOS_REGION_B(checkx, checky);
					
					x = TRUNCATE_TILE(posx) + check_x_ofs;
					y = TRUNCATE_TILE(posy) + check_y_ofs;
				}

				if (fix_mirror_anim)
				{
					if (dir == right)
						flip &= ~1; // not horiz
					else if (dir == left)
						flip |= 1;  // horiz
					else if (dir == up)
						flip &= ~2; // not vert
					else if (dir == down)
						flip |= 2;  // vert
					tile = ref_o_tile;

					if (dir & 2)
					{
						if (frames > 1)
							tile += frames;
						else ++tile;
					}
				}
			}


			if ((id == wRefBeam && ScriptGenerated) || id == wBeam)
			{
				if (runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			if (id == ewSword)
			{
				//eweqapon script here, later

			}

			break;
		}
		
		case wWhistle:
		{
			if(clk)
			{
				dead=1;
			}
			
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			break;
		}
		case wWind:
		{
			if(blocked())
			{
				dead=1;
			}

			mapscr* scr = get_scr(screen_spawned);
			
			int32_t wrx;
			if(get_qr(qr_NOARRIVALPOINT))
				wrx=scr->warpreturnx[0];
			else wrx=scr->warparrivalx;
			
			int32_t wry;
			if(get_qr(qr_NOARRIVALPOINT))
				wry=scr->warpreturny[0];
			else wry=scr->warparrivaly;

			std::tie(wrx, wry) = translate_screen_coordinates_to_world(screen_spawned, wrx, wry);

			if(specialinfo==1 && dead==-1 && x==(int32_t)wrx && y==(int32_t)wry)
			{
				stop_sfx(WAV_ZN1WHIRLWIND);
				dead=2;
			}
			else if(HeroAction() !=inwind && ((dir==right && x>=viewport.right()-16) || (dir==down && y>=viewport.bottom()-16) || (dir==left && x<=viewport.left()) || (dir==up && y<=viewport.top())))
			{
				stop_sfx(WAV_ZN1WHIRLWIND);
				dead=1;
			}
			else if(get_qr(qr_MORESOUNDS) && dead < 1)
				sfx(WAV_ZN1WHIRLWIND,pan(int32_t(x)),true,false);
				
			if((parentitem==-1 && get_qr(qr_WHIRLWINDMIRROR)) || (parentitem > -1 && itemsbuf[parentitem].flags & item_flag3))
				goto mirrors;
				
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			break;
		}
		
		case wFire:
		case wRefFire:
		case wRefFire2:
		{
			if(blocked())
			{
				dead=1;
			}
			
			itemdata const& parent = itemsbuf[parentitem];
			if(id != wFire)
			{
				if(clk==32)
				{
					step=0;
					
					if(id != wRefFire2)
					{
						isLit = true;
						checkLightSources(!get_qr(qr_TEMPCANDLELIGHT));
					}
				}
				
				if(clk==94)
				{
					dead=1;
					
					if(id != wRefFire2 && get_qr(qr_TEMPCANDLELIGHT) && flame_count()==1)
					{
						isLit = false;
						checkLightSources();
					}
				}
				
				if(clk==94 || get_qr(qr_INSTABURNFLAGS))
				{
					triggerfire(x,y,this,true,true,false,false,false);
				}
			}
			else if(parentitem<0 || (parentitem>-1 && parent.family!=itype_book))
			{
				if(clk==32)
				{
					step=0;
					
					if(parentitem<0 || !(parent.flags & item_flag2))
					{
						isLit = true;
						if(parentitem==-1 ? get_qr(qr_TEMPCANDLELIGHT)
							: (parent.flags & item_flag5))
						{
							checkLightSources();
						}
						else
						{
							checkLightSources(true);
						}
					}
				}
				
				if(clk==94)
				{
					dead=1;
					
					if((parentitem==-1 ? (get_qr(qr_TEMPCANDLELIGHT))
					    : (!(parent.flags & item_flag2)
							&&(parent.flags & item_flag5))) && flame_count()==1)
					{
						isLit = false;
						checkLightSources();
					}
				}
				
				if(clk==94 || get_qr(qr_INSTABURNFLAGS))
				{
					triggerfire(x,y,this,true,
						true,parentitem < 0 ? type > 1 : (parent.flags & item_flag9),
						parent.flags & item_flag10,parent.flags & item_flag11);
				}
			}
			else //book+wand fire
			{
				if(clk==1)
				{
					isLit = true;
					if(parent.flags & item_flag5)
					{
						checkLightSources();
					} 
					else
					{
						checkLightSources(true);
					}
				}
				
				if(clk==80)
				{
					dead=1;
					triggerfire(x,y,this,true,
						true,parent.flags & item_flag9,
						parent.flags & item_flag10,parent.flags & item_flag11);
					
					if((parentitem==-1 ? (get_qr(qr_TEMPCANDLELIGHT))
						: ((parent.flags & item_flag5))) && flame_count()==1)
					{
						isLit=false;
						checkLightSources();
					}
				}
			}
			
			// Killed by script?
			if(dead==0 && (parentitem==-1 ? get_qr(qr_TEMPCANDLELIGHT)
				: (parent.flags & item_flag5)) && flame_count()==1)
			{
				isLit=false;
				checkLightSources();
			}
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			break;
		}
		
		case wLitBomb:
		case wBomb:
		case wLitSBomb:
		case wSBomb:
		{
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			limited_animate();
			break;
		}
		
		case ewLitBomb:
		case ewBomb:
		case ewLitSBomb:
		case ewSBomb:
		{
			if(!misc)
			{
				break;
			}
			
			// Naaah.
			/*if (blocked())
			{
			  dead=1;
			}*/
			if(clk==(misc-2) && step==0)
			{
				id = (id==ewLitSBomb||id==ewSBomb ? ewSBomb : ewBomb);
				hxofs=2000;
			}
			
			if(clk==(misc-1) && step==0)
			{
				sfx(specialsfx,pan(int32_t(x)));
					
				if(id==wSBomb || id==wLitSBomb || id==ewSBomb || id==ewLitSBomb)
				{
					hxofs=hyofs=-16;
					hit_width=hit_height=48;
				}
				else
				{
					hxofs=hyofs=-8;
					hit_width=hit_height=32;
				}
				
				hzsz=16;
			}
			
			int32_t boomend = (misc+31);
								  
			if(clk==boomend && step==0)
			{
				hxofs=2000;
			}
			
			if(clk==misc+34)
			{
				if(step==0)
				{
					dead=1;
				}
			}
			break;
		}
		
		case wArrow:
		case wRefArrow:
		{
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			if(weapon_dying_frame)
			{
				if(dead != 0)
					weapon_dying_frame = false;
				else break;
			}
			if(dead>0 && !get_qr(qr_ARROWCLIP))
			{
				break;
			}
			
			if(id == wArrow && misc>0 && clk > misc)
			{
				dead=4;
			}
			
			if(trigger_secrets_if_flag(x,y,mfSTRIKE,true))
			{
				if (dead < 0) dead=4;
			}
			
			if(id == wArrow)
			{
				if(trigger_secrets_if_flag(x,y,mfARROW,true))
				{
					if (dead < 0) dead=4;
				}
				
				if(current_item(itype_arrow)>1)
				{
					if(trigger_secrets_if_flag(x,y,mfSARROW,true))
					{
						if (dead < 0) dead=4;
					}
				}
				
				if(current_item(itype_arrow)>=3)
				{
					if(trigger_secrets_if_flag(x,y,mfGARROW,true))
					{
						if (dead < 0) dead=4;
					}
				}
			}
			
			if(blocked())
			{
				if (dead < 0) dead=4;
			}
			
			break;
		}
		
		case wSSparkle:
		{
			if(blocked())  //no spakle area?
			{
				dead=1;
			}
			
			if(clk>=frames*o_speed-1) //(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1))
			{
				dead=1;
			}
		
			if ( isLWeapon )
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			
			break;
		}
			
		case wFSparkle:
		{
			if(blocked())  //no sparkle area?
			{
				dead=1;
			}
			
			if(clk>=frames*o_speed-1) //(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1))
			{
				dead=1;
			}
		
			if ( isLWeapon )
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			
			break;
		}
		case wBait:
		{
			if(blocked())  //no bait area?
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
				dead=23;
				goto skip_second_bait_script;
			}
			
			if(parentitem>-1 && clk>=itemsbuf[parentitem].misc1)
			{
				dead=1;
			}
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			skip_second_bait_script:
			break;
		}
		
		case wBrang:
		{
			//run first? brang scripts were being killed on WDS_BOUNCE, so this may fix that.
		
			if(dead==0)  // Set by ZScript
			{
				stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
				break;
			}
			
			else if(dead==1) // Set by ZScript
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
				onhit(false);
			}
			else
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			
			
			int32_t deadval=(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & item_flag3)?-2:1;
			
			/* This causes brang weapons with a level > 3 to hit OTHER flags. 
			// e.g., a brang with a level of 5 would trigger mfBRANG through mfGARROW! -Z
				for(int32_t i=0; i<current_item(itype_brang); i++)
				{
					if(trigger_secrets_if_flag(x,y,mfBRANG+i,true)) dead=deadval;
				}
			*/
			
			int32_t branglevel = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].fam_type;
			
			switch ( branglevel )
			{
				case 0:
				case 1:
				{
					if(trigger_secrets_if_flag(x,y,mfBRANG,true)) dead=deadval; break;
				}
				case 2: 
				{
					if(trigger_secrets_if_flag(x,y,mfBRANG,true)) dead=deadval;
					if(trigger_secrets_if_flag(x,y,mfMBRANG,true)) dead=deadval;
					break;
				}
				case 3:
				{
					goto brang_level_3_or_higher;
				}
				default: //level higher than 3
				{
					goto brang_level_3_or_higher;
				}
				brang_level_3_or_higher: 
				{
					if(trigger_secrets_if_flag(x,y,mfBRANG,true)) dead=deadval;
					if(trigger_secrets_if_flag(x,y,mfMBRANG,true)) dead=deadval;
					if(trigger_secrets_if_flag(x,y,mfFBRANG,true)) dead=deadval;
					break;
				}
			}
			
			if(trigger_secrets_if_flag(x,y,mfSTRIKE,true)) dead=deadval;
			itemdata const& brangitm = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)];
			if(triggerfire(x,y,this,true,
				brangitm.flags & item_flag8,brangitm.flags & item_flag9,
				brangitm.flags & item_flag10,brangitm.flags & item_flag11))
				dead=deadval;
			
			if(blocked())
			{
				dead=deadval;
				onhit(false);
			}
			
			++clk2;
			int32_t range = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].misc1;
			
			if(range && clk2>=range)
			{
				if(deadval==1)
				{
					misc=1;
				}
				else
				{
					dead=deadval;
				}
			}
			
			if(range && clk2>range-18 && clk2<range+16)
			{
				step=1;
			}
			else if(misc)
			{
				step=2;
			}
			else
			{
				step=3;
			}
			
			if(clk==0)                                            // delay a frame
			{
				++clk;
				sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int32_t(x)),true);
				return false;
			}
			
			if(clk==1)                                            // then check directional input
			{
				if(Up())
				{
					dir=up;
					
					if(Left())  dir=l_up;
					
					if(Right()) dir=r_up;
				}
				else if(Down())
				{
					dir=down;
					
					if(Left())  dir=l_down;
					
					if(Right()) dir=r_down;
				}
				else if(Left())
				{
					dir=left;
				}
				else if(Right())
				{
					dir=right;
				}
				doAutoRotate(true);
			}
			
			if(dead==1)
			{
				dead=-1;
				misc=1;
			}
			
			if(dead==-2)
			{
				misc=1;
			}
			
			if(misc==1)                                           // returning
			{
				if((abs(HeroY()-y)<7 && abs(HeroX()-x)<7)||dead==-2)
				{
					if(dead!=-2)
					{
						CatchBrang();
					}
					
					if(Lwpns.idCount(wBrang)<=1 && (!get_qr(qr_MORESOUNDS) || !Ewpns.idCount(ewBrang)))
						stop_sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound);
						
					/*if (dummy_bool[0])
					{
						add_grenade(x,y,z,0,-1); //TODO: Super bombs as well?
						dummy_bool[0]=false;
					}*/
					if(dragging!=-1)
					{
						getdraggeditem(dragging);
					}
					
					return true;
				}
				
				seekHero();
			}
			sfx(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].usesound,pan(int32_t(x)),true,false);
			
			break;
		}
		
		case wHookshot:
		{
			if(dead==0)  // Set by ZScript
			{
				hookshot_used = false;
				
				for(int32_t i=0; i<chainlinks.Count(); i++)
				{
					chainlinks.del(chainlinks.idFirst(wHSChain));
				}
			}
			//Diagonal Hookshot (8)
			itemdata const& hshot = itemsbuf[parentitem>-1 ? parentitem : current_item_id(family_class)];
			byte allow_diagonal = (hshot.flags & item_flag2) ? 1 : 0;
			bool sw = family_class == itype_switchhook;
			//if ( allow_diagonal && misc2 == 0 ) 
			if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
			{
				++clk;
				return false;
			}
			//Diagonal Hookshot (10)
			//Sprites wpn5: Head, diagonal
			//	  wpn6: handle, diagonal
			//	  wpn7: chainlink, diagonal
			//This sets the direction for digaonals based on controller input. 
			if(clk==1 && allow_diagonal)    
			{
				if(Up())
				{
					if(Left() )  
					{
						LOADGFX(hshot.wpn5);
						dir=l_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=0;
						switch((int32_t)(Hero.dir))
						{
							case up:
								yofs += 7;
								xofs -= 2;
								break;
							case left:
								yofs -= 5;
								xofs += 5;
								break;
						}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn5);
						dir=r_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=1;
						
						switch((int32_t)(Hero.dir))
						{
							case up:
								yofs += 7;
								xofs -= 0;
								break;
							case right:
								yofs -= 8;
								xofs -= 3;
								break;
						}
						
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
				else if(Down())
				{
					//dir=down; //Up would already have been set if facing down.
					
					if(Left() )  
					{
						LOADGFX(hshot.wpn5);
						dir=l_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=2;
						switch((int32_t)(Hero.dir))
						{
							case down:
								yofs -= 5;
								xofs -= 2;
								break;
							case left:
								yofs -= 2;
								xofs += 5;
								break;
						}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn5);
						dir=r_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=3;
						switch((int32_t)(Hero.dir))
						{
							case down:
								yofs -= 8;
								xofs += 1;
								break;
							case right:
								yofs += 2;
								xofs -= 3;
								break;
						}
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
				doAutoRotate(true);
			}
			
			// Hookshot grab and retract code 
			//Diagonal Hookshot (2)
			
			rpos_t rpos = rpos_t::None;
			ffcdata* ffc = nullptr;
			
			if(misc==0)
			{
				int32_t maxlength=16*hshot.misc1;
				//If the hookshot has extended to maxlength, retract it.
				//Needa an option to measure in pixels, instead of tiles. -Z
				if((abs(HeroX()-x)>maxlength)||(abs(HeroY()-y)>maxlength))
				{
					dead=1;
				}
				//If it hits a block object, retract it.
				if(trigger_secrets_if_flag(x,y,mfSTRIKE,true)) dead=1;
				
				if(trigger_secrets_if_flag(x,y,mfHOOKSHOT,true)) dead=1;
			
				//Look for grab combos based on direction.
				int32_t tx = -1, ty = -1, tx2 = -1, ty2 = -1, ty3 = -1;
				//ty3 is for the old hookshot collision. Hookshot blocks would block the hookshot but not grab them in certain scenarios.
				bool oldshot = (get_qr(qr_OLDHOOKSHOTGRAB) && !sw);
				switch(Y_DIR(dir))
				{
					case up:
						tx2 = x + 2;
						ty2 = y + 7;
						ty3 = y + 7;
						break;
					case down:
						tx2 = x + 12;
						ty2 = y + 12;
						ty3 = y + 12;
						break;
				}
				switch(X_DIR(dir))
				{
					case left:
						tx = x + 6;
						ty = y + (oldshot?7:13);
						ty3 = y + 13;
						break;
					case right:
						tx = x + 9;
						ty = y + (oldshot?7:13);
						ty3 = y + 13;
						break;
				}
				
				bool hitsolid = false;
				int32_t maxlayer = 0;
				if (get_qr(qr_HOOKSHOTALLLAYER)) maxlayer = 6;
				else if (get_qr(qr_HOOKSHOTLAYERFIX)) maxlayer = 2;
				
				if(tx > -1)
				{
					hooked = check_hshot(0,tx, ty, sw, &rpos, &ffc);
					
					for(auto lyr = 1; !hooked && lyr <= maxlayer; ++lyr)
						hooked = check_hshot(lyr,tx,ty,sw, &rpos);
						
					if(_walkflag(tx,ty3,1) && !ishookshottable(tx,ty3))
						hitsolid = true;
				}
				if(tx2 > -1 && !hooked)
				{
					hooked = check_hshot(0,tx2, ty2, sw, &rpos, &ffc);
					
					for(auto lyr = 1; !hooked && lyr <= maxlayer; ++lyr)
						hooked = check_hshot(lyr,tx2,ty2,sw, &rpos);
						
					if(_walkflag(tx2,ty3,1) && !ishookshottable(tx2,ty3))
						hitsolid=true;
				}
				
				if (hitsolid && !hooked) 
					dead = 1;
			}
			
			if(hooked)
			{
				if (rpos != rpos_t::None)
					hooked_comborpos = rpos;
				misc=sw?2:1;
				step=0;
				pull_hero=true;
				if(sw)
				{
					if (ffc)
					{
						ffc->hooked = true;
						ffc->switch_hooked = true;
						switching_object = ffc;
					}
					Hero.doSwitchHook(hshot.misc5);
					sfx(hshot.usesound2,pan(int32_t(x)));
					stop_sfx(hshot.usesound);
					hs_switcher = true;
				}
				else
				{
					if (ffc)
					{
						ffc->hooked = true;
					}
				}
			}
			
			++clk2;
			
			if(clk==0)                                            // delay a frame
			{
				++clk;
				
				if(misc < 2) sfx(hshot.usesound,pan(int32_t(x)),true);
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
				return false;
			}
			
			if(dead==1)
			{
				dead=-1;
				if(step > 0)
					step = -step;
					
				if(!misc) misc=1;
			}
			
			if(misc==1)                                           // returning
			{
				if((dir<left && abs(HeroY()-y)<9) || (dir >= left && abs(HeroX()-x)<9))
				{
					hookshot_used=false;
					
					if(pull_hero)
					{
						hs_fix=true;
					}
					
					pull_hero=false;
					chainlinks.clear();
					CatchBrang();
					
					if(parentitem>-1)
					{
						stop_sfx(itemsbuf[parentitem].usesound);
					}
					
					if(dragging!=-1)
					{
						getdraggeditem(dragging);
					}
					return true;
				}
			}
			
			if(misc < 2) sfx(hshot.usesound,pan(int32_t(x)),true,false);
			
			if(blocked())
			{
				//not on the return!! -DD
				if(dead != -1)
					dead=1;
			}
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
		}
		break;
		case wHSHandle:
		{
			if(hookshot_used==false)
			{
				dead=0;
			}
			
			if(blocked())  //no hookshot handle area?
			{
				hookshot_used=false;
				dead=0;
			}
			//Diagonal Hookshot Handle
			itemdata const& hshot = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)];
			byte allow_diagonal = (hshot.flags & item_flag2) ? 1 : 0; 
			//if ( allow_diagonal && misc2 == 0 ) 
			if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
			{
				++clk;
				return false;
			}
			//Diagonal Hookshot (10)
			//This sets the direction for digaonals based on controller input. 
			if(clk==1 && allow_diagonal)    
			{
				if(Up())
				{
					if(Left() )  
					{
						LOADGFX(hshot.wpn6);
						dir=l_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=0;
						switch((int32_t)(Hero.dir))
						{
							case up:
								yofs += 7;
								xofs += 2;
								break;
							case left:
								yofs -= 1;
								xofs += 6;
								break;
						}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn6);
						dir=r_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=1;
						
						switch((int32_t)(Hero.dir))
						{
							case up:
								yofs += 5;
								xofs -= 3;
								break;
							case right:
								yofs -= 0;
								xofs -= 8;
								break;
						}
						
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
			
				else if(Down())
				{
					//dir=down; //Up would already have been set if facing down.
					
					if(Left() )  
					{
						LOADGFX(hshot.wpn6);
						dir=l_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=2;
						switch((int32_t)(Hero.dir))
						{
							case down:
								yofs -= 8;
								xofs -= 0;
								break;
							case left:
								yofs -= 6;
								xofs += 5;
								break;
						}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn6);
						dir=r_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						flip=3;
						switch((int32_t)(Hero.dir))
						{
							case down:
								yofs -= 8;
								xofs -= 0;
								break;
							case right:
								yofs -= 3;
								xofs -= 5;
								break;
						}
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
				doAutoRotate(true);
			}
			break;
		}
		case wHSChain:
		{
			
			//Diagonal Hookshot Handle
			itemdata const& hshot = itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_hookshot)];
			byte allow_diagonal = (hshot.flags & item_flag2) ? 1 : 0; 
			//if ( allow_diagonal && misc2 == 0 ) 
			if(clk==0 && allow_diagonal)                                            // delay a frame ere setting a dir
			{
				++clk;
				return false;
			}
			//Diagonal Hookshot (10)
			//This sets the direction for digaonals based on controller input. 
			if(clk==1 && allow_diagonal)    
			{
				if(Up())
				{
					if(Left() )  
					{
						LOADGFX(hshot.wpn7);
						dir=l_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						//flip=0;
						//switch((int32_t)(Hero.dir))
						//{
						//	case up:
						//		yofs += 7;
						//		xofs += 2;
						//		break;
						//	case left:
						//		yofs -= 1;
						//		xofs += 6;
						//		break;
						//}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn7);
						dir=r_up;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						//flip=1;
						
						//switch((int32_t)(Hero.dir))
						//{
						//	case up:
						//		yofs += 5;
						//		xofs -= 3;
						//		break;
						//	case right:
						//		yofs -= 0;
						//		xofs -= 8;
						//		break;
						//}
						
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
			
				else if(Down())
				{
					//dir=down; //Up would already have been set if facing down.
					
					if(Left() )  
					{
						LOADGFX(hshot.wpn7);
						dir=l_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						//flip=2;
						//switch((int32_t)(Hero.dir))
						//{
						//	case down:
						//		yofs -= 8;
						//		xofs -= 0;
						//		break;
						//	case left:
						//		yofs -= 6;
						//		xofs += 5;
						//		break;
						//}
						
					}
					
					else if(Right() ) 
					{
						LOADGFX(hshot.wpn7);
						dir=r_down;
						update_weapon_frame(((frames>1)?frames:0),o_tile);
						if (!get_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM)) o_tile = tile;
						//flip=3;
						//switch((int32_t)(Hero.dir))
						//{
						///	case down:
						//		yofs -= 8;
						//		xofs -= 0;
						//		break;
						//	case right:
						//		yofs -= 3;
						//		xofs -= 5;
						//		break;
						//}
						
					}
					misc2 = 1; //to prevent wagging it all over the screen, we set it once. 
				}
			}
			break;
		}
		
		case wPhantom:
		{
			switch(type)
			{
				case pDIVINEFIREROCKET:
					if(y <= -200)
						dead = 1;
						
					break;
					
				case pDIVINEFIREROCKETRETURN:                                             //Divine Fire Rocket return
					if(y>=casty)
					{
						dead=1;
						castnext=true;
					}
					
					break;
					
				case pDIVINEFIREROCKETTRAIL:                                             //Divine Fire Rocket trail
					if(clk>=(((wpnsbuf[wDIVINEFIRES1A].frames) * (wpnsbuf[wDIVINEFIRES1A].speed))-1))
					{
						dead=0;
					}
					
					break;
					
				case pDIVINEFIREROCKETTRAILRETURN:                                             //Divine Fire Rocket return trail
					if(clk>=(((wpnsbuf[wDIVINEFIRES1B].frames) * (wpnsbuf[wDIVINEFIRES1B].speed))-1))
					{
						dead=0;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETRETURN1:                                             //Divine Protection Rocket return
					if(x>=castx)
					{
						dead=1;
						castnext=true;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETTRAIL1:                                             //Divine Protection Rocket trail
					if(clk>=(((wpnsbuf[wDIVINEPROTECTIONS1A].frames) * (wpnsbuf[wDIVINEPROTECTIONS1A].speed))-1))
					{
						dead=0;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETTRAILRETURN1:                                             //Divine Protection Rocket return trail
					if(clk>=(((wpnsbuf[wDIVINEPROTECTIONS1B].frames) * (wpnsbuf[wDIVINEPROTECTIONS1B].speed))-1))
					{
						dead=0;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETRETURN2:                                             //Divine Protection Rocket return
					if(x<=castx)
					{
						dead=0;
						castnext=true;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETTRAIL2:                                             //Divine Protection Rocket trail
					if(clk>=(((wpnsbuf[wDIVINEPROTECTIONS2A].frames) * (wpnsbuf[wDIVINEPROTECTIONS2A].speed))-1))
					{
						dead=0;
					}
					
					break;
					
				case pDIVINEPROTECTIONROCKETTRAILRETURN2:                                             //Divine Protection Rocket return trail
					if(clk>=(((wpnsbuf[wDIVINEPROTECTIONS2B].frames) * (wpnsbuf[wDIVINEPROTECTIONS2B].speed))-1))
					{
						dead=0;
					}
					
					break;
					
			}
			
			if(blocked()) //not really sure this is needed
			{
				dead=1;
			}
			
			break;
		}
		
		case wRefMagic:
		case wMagic:
		{
			if (isLWeapon && linkedItem)
			{
				itemdata const& book = itemsbuf[linkedItem];
				if(book.flags&item_flag6)
					step = zslongToFix(book.misc3*100);
			}
			
			if(trigger_secrets_if_flag(x,y,id==wMagic ? mfWANDMAGIC : mfREFMAGIC,true))
				dead=0;
			
			if(trigger_secrets_if_flag(x,y,mfSTRIKE,true))
				dead=0;
		   
			bool brokebook = get_qr(qr_BROKENBOOKCOST);
			itemdata const& book = itemsbuf[brokebook ? (parentitem>-1 ? parentitem : current_item_id(itype_book)) : linkedItem];
			if((id==wMagic && (brokebook ? current_item(itype_book) : (linkedItem && book.family == itype_book)) &&
				book.flags&item_flag1) && get_qr(qr_INSTABURNFLAGS))
			{
				triggerfire(x,y,this,true,
					true,book.flags & item_flag9,
					book.flags & item_flag10,book.flags & item_flag11);
			}
			
			
			//mirrors: //the latter instance should suffice
			zfix checkx=0, checky=0;
			int32_t check_x_ofs=0, check_y_ofs=0;
			if (get_qr(qr_MIRRORS_USE_WEAPON_CENTER))
			{
				checkx = (x+hxofs+(hit_width*0.5));
				checky = (y+hyofs+(hit_height*0.5)-fakez);
				check_x_ofs = x - (checkx-8);
				check_y_ofs = y - (checky-8);
			}
			else
			{
				switch(dir)
				{
					case up:
						checkx=x+7;
						checky=y+8;
						break;
						
					case down:
						checkx=x+7;
						checky=y;
						break;
						
					case left:
						checkx=x+8;
						checky=y+7;
						break;
						
					case right:
						checkx=x;
						checky=y+7;
						break;
				}
				checky-=fakez;
			}
			
			if (ignorecombo == rpos_t::None || ignorecombo != COMBOPOS_REGION_B(checkx, checky))
			{
				byte layers = get_qr(qr_MIRROR_PRISM_LAYERS) ? 0b1111111 : 0b0000001;
				if(hitcombo(checkx, checky, cMIRROR, layers))
				{
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					w->dir ^= 1;
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						
						if(w->dir&2)
							w->flip ^= 1;
						else
							w->flip ^= 2;
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMIRRORSLASH, layers))
				{
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this; //Oh, look, a memory leak. The new instruction is making something on the heap, but this circumvents removing it. 
					}
					
					w->o_tile = ref_o_tile;
					w->tile = ref_o_tile;
					w->dir = 3-w->dir;
					w->doAutoRotate(true);
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						if ( do_animation ) 
						{
							if((w->dir==1)||(w->dir==2))
							w->flip ^= 3;
						}
					}
					if ( do_animation ) 
					{
						w->tile=w->o_tile;
						
						if(w->dir&2)
						{
							if(w->frames>1)
							{
							w->tile+=w->frames;
							}
							else
							{
							++w->tile;
							}
						}
					}
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMIRRORBACKSLASH, layers))
				{
					weapon *w = NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					w->o_tile = ref_o_tile;
					w->tile = ref_o_tile;
					w->dir ^= 2;
					w->doAutoRotate(true);
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						if ( do_animation ) 
						{
							if(w->dir&1)
								w->flip ^= 2;
							else
								w->flip ^= 1;
						}
					}
					if ( do_animation ) 
					{
						w->tile=w->o_tile;
						
						if(w->dir&2)
						{
							if(w->frames>1)
							{
							w->tile+=w->frames;
							}
							else
							{
							++w->tile;
							}
						}
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMAGICPRISM, layers) && (id != wWind))
				{
					int32_t newx, newy;
					newy=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
					newx=(int32_t(checkx)&0xF0)+check_x_ofs;
					
					for(int32_t tdir=0; tdir<4; tdir++)
					{
						if((dir!=(tdir^1) && !AngleReflect) || (tdir != 2 && AngleReflect))
						{
							weapon *w=new weapon(*this);
							w->dir=tdir;
							w->doAutoRotate(true);
							if ( this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS) )
							{
								double newangle = this->angle + DegreesToRadians(90*tdir);
								w->angle = WrapAngle(newangle);
								if (AngleReflect)
								{
									w->angular = true;
									w->doAutoRotate();
									w->dir = AngleToDir(WrapAngle(newangle));
								}
							}
							w->o_tile = ref_o_tile;
							w->tile = ref_o_tile;
							w->x=newx;
							w->y=newy;
							w->fakez=fakez;
							w->z=z;
							w->id=wRefMagic; w->convertType(true);
							w->parentid=parentid;
							w->parentitem=parentitem;
							w->flip = 0;
							w->ignoreHero = false;
							w->hyofs = w->hxofs = 0;
							w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
							if ( do_animation ) 
							{
								//also set up the magic's correct animation -DD
								switch(w->dir)
								{
									case down:
										w->flip=2;
										
									case up:
										w->tile = w->o_tile;
										w->hyofs=2;
										w->hit_height=12;
										break;
										
									case left:
										w->flip=1;
										
									case right:
										w->tile=w->o_tile+((w->frames>1)?w->frames:1);
										w->hxofs=2;
										w->hit_width=12;
										break;
									
									default: break;
								}
							}
							Lwpns.add(w);
						}
					}
					
					dead=0;
				}
				
				if(hitcombo(checkx, checky, cMAGICPRISM4, layers) && (id != wWind))
				{
					int32_t newx, newy;
					newy=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
					newx=(int32_t(checkx)&0xF0)+check_x_ofs;
					
					for(int32_t tdir=0; tdir<4; tdir++)
					{
						weapon *w=new weapon(*this);
						w->dir=tdir;
						w->doAutoRotate(true);
						if ( this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS) )
						{
							double newangle = this->angle + DegreesToRadians(90*tdir);
							w->angle = WrapAngle(newangle);
							if (AngleReflect)
							{
								w->angular = true;
								w->doAutoRotate();
								w->dir = AngleToDir(WrapAngle(newangle));
							}
						}
						w->o_tile = ref_o_tile;
						w->tile = ref_o_tile;
						w->x=newx;
						w->y=newy;
						w->fakez=fakez;
						w->z=z;
						w->id=wRefMagic; w->convertType(true);
						w->parentid=parentid;
						w->parentitem=parentitem;
						w->flip = 0;
						w->ignoreHero = false;
						w->hyofs = w->hxofs = 0;
						w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
						
						if ( do_animation ) 
						{
							//also set up the magic's correct animation -DD
							switch(w->dir)
							{
								case down:
									w->flip=2;
								
								case up:
									w->tile = w->o_tile;
									w->hyofs=2;
									w->hit_height=12;
									break;
								
								case left:
									w->flip=1;
								
								case right:
									w->tile=w->o_tile+((w->frames>1)?w->frames:1);
									w->hxofs=2;
									w->hit_width=12;
									break;
								
								default: break;
							}
						}
						Lwpns.add(w);
					}
					
					dead=0;
				}
				
				auto newmirror = gethitcombo(checkx, checky, cMIRRORNEW, layers);
				if(newmirror > -1)
				{
					newcombo const& cmb = combobuf[newmirror];
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					byte newdir = cmb.attribytes[NORMAL_DIR(w->dir)];
					if(newdir > 7)
					{
						dead = 0;
						break;
					}
					w->dir = newdir;
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						
						tile = ref_o_tile;
						int tile_inc = zc_max(frames,1);
						switch(dir)
						{
							case up:
								flip = 0;
								break;
							case right:
								flip = 0;
								tile += tile_inc;
								break;
							case left:
								flip = 1; //horz
								tile += tile_inc;
								break;
							case down:
								flip = 2; //vert
								break;
							case l_up:
								flip = 0;
								tile += tile_inc*2;
								break;
							case l_down:
								flip = 2; //vert
								tile += tile_inc*2;
								break;
							case r_up:
								flip = 1; //horz
								tile += tile_inc*2;
								break;
							case r_down:
								flip = 3; //horz+vert
								tile += tile_inc*2;
								break;
						}
						o_tile = tile;
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(blocked(0, 0))
				{
					dead=0;
				}
			}
			if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
		}
		break;
		
		case ewMagic:
		{
		mirrors: //jumped to by wWind
			zfix checkx=0, checky=0;
			int32_t check_x_ofs=0, check_y_ofs=0;
			
			if (get_qr(qr_MIRRORS_USE_WEAPON_CENTER))
			{
				checkx = (x+hxofs+(hit_width*0.5));
				checky = (y+hyofs+(hit_height*0.5)-fakez);
				check_x_ofs = x - (checkx-8);
				check_y_ofs = y - (checky-8);
			}
			else
			{
				switch(dir)
				{
					case up:
						checkx=x+7;
						checky=y+8;
						break;
						
					case down:
						checkx=x+7;
						checky=y;
						break;
						
					case left:
						checkx=x+8;
						checky=y+7;
						break;
						
					case right:
						checkx=x;
						checky=y+7;
						break;
				}
				checky-=fakez;
			}
			
			if (ignorecombo == rpos_t::None || ignorecombo != COMBOPOS_REGION_B(checkx, checky))
			{
				byte layers = get_qr(qr_MIRROR_PRISM_LAYERS) ? 0b1111111 : 0b0000001;
				if(hitcombo(checkx, checky, cMIRROR, layers))
				{
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					
					w->dir ^= 1;
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						
						if(w->dir&2)
							w->flip ^= 1;
						else
							w->flip ^= 2;
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMIRRORSLASH, layers))
				{
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					w->o_tile = ref_o_tile;
					w->tile = ref_o_tile;
					w->dir = 3-w->dir;
					w->doAutoRotate(true);
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						if ( do_animation ) 
						{
							if((w->dir==1)||(w->dir==2))
							w->flip ^= 3;
						}
					}
					if ( do_animation ) 
					{
						w->tile=w->o_tile;
						
						if(w->dir&2)
						{
							if(w->frames>1)
							{
								w->tile+=w->frames;
							}
							else
							{
								++w->tile;
							}
						}
					}
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMIRRORBACKSLASH, layers))
				{
					weapon *w=NULL;
					
					if(id==ewMagic)
					{
						w=new weapon(*this);
						dead=0;
						if (!Lwpns.add(w)) break;
					}
					else
					{
						w=this;
					}
					
					w->o_tile = ref_o_tile;
					w->tile = ref_o_tile;
					w->dir ^= 2;
					w->doAutoRotate(true);
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						if ( do_animation ) 
						{
							if(w->dir&1)
								w->flip ^= 2;
							else
								w->flip ^= 1;
						}
					}
					if ( do_animation ) 
					{
						w->tile=w->o_tile;
						
						if(w->dir&2)
						{
							if(w->frames>1)
							{
								w->tile+=w->frames;
							}
							else
							{
								++w->tile;
							}
						}
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(hitcombo(checkx, checky, cMAGICPRISM, layers) && (id != wWind))
				{
					int32_t newx, newy;
					newy=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
					newx=(int32_t(checkx)&0xF0)+check_x_ofs;
					
					for(int32_t tdir=0; tdir<4; tdir++)
					{
						if((dir!=(tdir^1) && !AngleReflect) || (tdir != 2 && AngleReflect))
						{
							weapon *w=new weapon(*this);
							w->dir=tdir;
							w->doAutoRotate(true);
							if ( this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS) )
							{
								double newangle = this->angle + DegreesToRadians(90*tdir);
								w->angle = WrapAngle(newangle);
								if (AngleReflect)
								{
									w->angular = true;
									w->doAutoRotate();
									w->dir = AngleToDir(WrapAngle(newangle));
								}
							}
							w->o_tile = ref_o_tile;
							w->tile = ref_o_tile;
							w->x=newx;
							w->y=newy;
							w->fakez=fakez;
							w->z=z;
							w->id=wRefMagic; w->convertType(true);
							w->parentid=parentid;
							w->parentitem=parentitem;
							w->flip = 0;
							w->ignoreHero = false;
							w->hyofs = w->hxofs = 0;
							w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
							if ( do_animation ) 
							{
								//also set up the magic's correct animation -DD
								switch(w->dir)
								{
									case down:
										w->flip=2;
										
									case up:
										w->tile = w->o_tile;
										w->hyofs=2;
										w->hit_height=12;
										break;
										
									case left:
										w->flip=1;
										
									case right:
										w->tile=w->o_tile+((w->frames>1)?w->frames:1);
										w->hxofs=2;
										w->hit_width=12;
										break;
									
									default: break;
								}
							}
							Lwpns.add(w);
						}
					}
					
					dead=0;
				}
				
				if(hitcombo(checkx, checky, cMAGICPRISM4, layers) && (id != wWind))
				{
					int32_t newx, newy;
					newy=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
					newx=(int32_t(checkx)&0xF0)+check_x_ofs;
					
					for(int32_t tdir=0; tdir<4; tdir++)
					{
						weapon *w=new weapon(*this);
						w->dir=tdir;
						w->doAutoRotate(true);
						if ( this->angular && get_qr(qr_ANGULAR_REFLECTED_WEAPONS) )
						{
							double newangle = this->angle + DegreesToRadians(90*tdir);
							w->angle = WrapAngle(newangle);
							if (AngleReflect)
							{
								w->angular = true;
								w->doAutoRotate();
								w->dir = AngleToDir(WrapAngle(newangle));
							}
						}
						w->o_tile = ref_o_tile;
						w->tile = ref_o_tile;
						w->x=newx;
						w->y=newy;
						w->fakez=fakez;
						w->z=z;
						w->id=wRefMagic; w->convertType(true);
						w->parentid=parentid;
						w->parentitem=parentitem;
						w->flip = 0;
						w->ignoreHero = false;
						w->hyofs = w->hxofs = 0;
						w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
						
						if ( do_animation ) 
						{
							//also set up the magic's correct animation -DD
							switch(w->dir)
							{
								case down:
									w->flip=2;
								
								case up:
									w->tile = w->o_tile;
									w->hyofs=2;
									w->hit_height=12;
									break;
								
								case left:
									w->flip=1;
								
								case right:
									w->tile=w->o_tile+((w->frames>1)?w->frames:1);
									w->hxofs=2;
									w->hit_width=12;
									break;
								
								default: break;
							}
						}
						Lwpns.add(w);
					}
					
					dead=0;
				}
				
				auto newmirror = gethitcombo(checkx, checky, cMIRRORNEW, layers);
				if(newmirror > -1)
				{
					newcombo const& cmb = combobuf[newmirror];
					weapon *w = this;
					
					byte newdir = cmb.attribytes[NORMAL_DIR(w->dir)];
					if(newdir > 7)
					{
						dead = 0;
						break;
					}

					if (id == ewMagic)
					{
						w = new weapon(*this);
						dead = 0;
						if (!Lwpns.add(w)) break;
					}

					w->dir = newdir;
					
					if(w->id != wWind)
					{
						w->id = wRefMagic; w->convertType(true);
						
						tile = ref_o_tile;
						int tile_inc = zc_max(frames,1);
						switch(dir)
						{
							case up:
								flip = 0;
								break;
							case right:
								flip = 0;
								tile += tile_inc;
								break;
							case left:
								flip = 1; //horz
								tile += tile_inc;
								break;
							case down:
								flip = 2; //vert
								break;
							case l_up:
								flip = 0;
								tile += tile_inc*2;
								break;
							case l_down:
								flip = 2; //vert
								tile += tile_inc*2;
								break;
							case r_up:
								flip = 1; //horz
								tile += tile_inc*2;
								break;
							case r_down:
								flip = 3; //horz+vert
								tile += tile_inc*2;
								break;
						}
						o_tile = tile;
					}
					
					w->ignoreHero=false;
					w->ignorecombo=COMBOPOS_REGION_B(checkx, checky);
					w->x=TRUNCATE_TILE(checkx.getInt())+check_x_ofs;
					w->y=TRUNCATE_TILE(checky.getInt())+check_y_ofs;
				}
				
				if(blocked(0, 0))
				{
					dead=0;
				}
			}
		}
		break;
		
		//  enemy weapons
		case ewFireball2:
			switch(misc)
			{
				case up:
					y-=.5;
					break;
					
				case down:
					y+=.5;
					break;
					
				case left:
					x-=.5;
					break;
					
				case right:
					x+=.5;
					break;
					
				case l_up:
					y-=.354;
					x-=.354;
					break;
					
				case r_up:
					y-=.354;
					x+=.354;
					break;
					
				case l_down:
					y+=.354;
					x-=.354;
					break;
					
				case r_down:
					y+=.354;
					x+=.354;
					break;
			}
			
			[[fallthrough]];
		case wRefFireball:
		case ewFireball:
		{
			if((id==wRefFireball)&&(trigger_secrets_if_flag(x,y,mfREFFIREBALL,true))) dead=0;
			
			if((id==wRefFireball)&&(trigger_secrets_if_flag(x,y,mfSTRIKE,true))) dead=0;
			
			if(blocked())
			{
				dead=0;
			}
			
			if(clk<16)
			{
				++clk;
				
				if(dead>0)
					--dead;
				
				if(dead == 0 && !weapon_dying_frame && get_qr(qr_WEAPONS_EXTRA_FRAME))
				{
					weapon_dying_frame = true;
					return false;
				}
				
				return dead==0;
			}
			if ( id == wRefFireball && ScriptGenerated )
			{
				if(runscript_do_earlyret(run_script(MODE_NORMAL))) return false;
			}
			break;
		}
		
		case ewFlame:
		{
			if(clk==32)
			{
				step=0;
				misc = -1; // Don't drift diagonally anymore
				isLit=true;
				if(get_qr(qr_TEMPCANDLELIGHT))
				{
					checkLightSources();
				}
				else
				{
					checkLightSources(true);
				}
			}
			
			if(clk==126)
			{
				dead=1;
				
				if(get_qr(qr_TEMPCANDLELIGHT))
				{
					isLit=false;
					checkLightSources();
				}
			}
			
			if(blocked())
			{
				dead=1;
			}
			
			// Killed by script?
			if(dead==0 && get_qr(qr_TEMPCANDLELIGHT) && flame_count()==1)
			{
				isLit=false;
				checkLightSources();
			}
			
			break;
		}
		
		case ewFireTrail:
		{
			if(clk==32)
			{
				step=0;  //should already be 0, but still...
				isLit=true;
				if(get_qr(qr_TEMPCANDLELIGHT))
				{
					checkLightSources();
				}
				else
				{
					checkLightSources(true);
				}
			}
			
			if((get_qr(qr_OLD_FLAMETRAIL_DURATION) && clk==640) || (!get_qr(qr_OLD_FLAMETRAIL_DURATION) && clk == 180))
			{
				dead=1;
				
				if(get_qr(qr_TEMPCANDLELIGHT))
				{
					isLit=false;
					checkLightSources();
				}
			}
			
			if(blocked())
			{
				dead=1;
			}
			
			
			break;
		}
		
		case ewBrang:
		{
			if(clk==0)
			{
				misc2=(dir<left)?y:x;                               // save home position
				
				if(dummy_bool[0]==true)
				{
					seekHero();
				}
				
				if(get_qr(qr_MORESOUNDS))
				{
					//if (step!=0)
					sfx(specialsfx, pan(int32_t(x)), true);
					//else
					;//stop_sfx(WAV_BRANG);
				}
			}
			
			++clk2;
			
			if(clk2==45&&!dummy_bool[0])
			{
				misc=1;
				dir^=1;
			}
			
			if(dummy_bool[0])
			{
				step=5;
			}
			else
			{
				if(clk2>27 && clk2<61)
				{
					step=1;
				}
				else if(misc)
				{
					step=2;
				}
				else
				{
					step=3;
				}
			}
			
			bool killrang = false;
			
			if(misc==1)                                           // returning
			{
				if(dummy_bool[0]==true)
				{
					//no parent
					if(parentid < 0)
						return true;
						
					//find parent
					int32_t index = -1;
					
					for(int32_t i=0; i<guys.Count(); i++)
						if(guys.spr(i)->getUID()==parentid)
						{
							index = i;
							break;
						}
						
					//parent is not a guy
					if(index < 0)
						return true;
						
					seekEnemy(index);
					
					if((abs(x-GuyX(index))<7)&&(abs(y-GuyY(index))<7))
					{
						if(get_qr(qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
						{
							stop_sfx(WAV_BRANG);
						}
						
						return true;
					}
				}
				else
				{
					switch(dir)
					{
						case up:
							if(y<misc2) killrang=true;
							
							break;
							
						case down:
							if(y>misc2) killrang=true;
							
							break;
							
						case left:
							if(x<misc2) killrang=true;
							
							break;
							
						case right:
							if(x>misc2) killrang=true;
							
							break;
					}
				}
			}
			else if(blocked())
			{
				//only obey block flags before the bounce
				dead=1;
			}
			
			//if the boomerang is not on its way home, it bounces
			if(dead==1 && misc != 1)
			{
				dead=-1;
				misc=1;
				dir^=1;
				clk2=zc_max(46,90-clk2);
			}
			
			//otherwise it disappears
			if(killrang || dead==1)
			{
				if(get_qr(qr_MORESOUNDS) && !Lwpns.idCount(wBrang) && Ewpns.idCount(ewBrang)<=1)
				{
					stop_sfx(WAV_BRANG);
				}
				
				dead = 1;
			}
		}
	}
	
	// move sprite, check clipping
	if(dead==-1 && clk>=0)
	{
		move(step);
		
		if(clip())
		{
			onhit(true);
		}
		else if(id==ewRock)
		{
			if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
			{
				onhit(true);
			}
		}
	}
	else if(dead==-10) // Scripting hack thing related to weapon->DeadState
	{
		if(clip())
		{
			onhit(true);
		}
	}
	
	if(misc_wflags & (WFLAG_BREAK_ON_SOLID|WFLAG_STOP_WHEN_HIT_SOLID))
	{
		if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
		{
			if(misc_wflags & WFLAG_BREAK_ON_SOLID)
				dead = 0;
			if(misc_wflags & WFLAG_STOP_WHEN_HIT_SOLID)
				step = 0;
			findcombotriggers(); //Hit solid triggers
		}
	}
	
	if(bounce)
	{
		switch(dir)
		{
			case up:
				x-=1;
				y+=2;
				break;
				
			case down:
				x+=1;
				y-=2;
				break;
				
			case left:
				x+=2;
				y-=1;
				break;
				
			case right:
				x-=2;
				y-=1;
				break;
		}
	}
	
	// update clocks
	++clk;
	//if ( id >= wScript1 && id <= wScript10 ) Z_scripterrlog("Updated Script Weapon Clock\n");
	
	if(dead>0)
		--dead;
	if(weap_timeout > 0)
	{
		if(!--weap_timeout)
			dead = 0;
	}
	
	bool ret = dead==0;
	if(ret && !weapon_dying_frame && get_qr(qr_WEAPONS_EXTRA_FRAME))
	{
		if(id!=wSword)
		{
			weapon_dying_frame = true;
			ret = false;
		}
	}
	if(ret || rundeath) do_death_fx();
	return ret;
}

void weapon::do_death_fx()
{
	if(death_spawnitem > -1)
	{
		item* itm = (new item(x, y, z, death_spawnitem, death_item_pflags, 0));
		itm->fakez = fakez;
		items.add(itm);
	}
	if(death_spawndropset > -1)
	{
		auto itid = select_dropitem(death_spawndropset);
		if(itid > -1)
		{
			item* itm = (new item(x, y, z, itid, death_item_pflags, 0));
			itm->fakez = fakez;
			itm->from_dropset = death_spawndropset;
			items.add(itm);
		}
	}
	switch(death_sprite)
	{
		case -2: decorations.add(new dBushLeaves(x, y-(z+fakez), dBUSHLEAVES, 0, 0)); break;
		case -3: decorations.add(new dFlowerClippings(x, y-(z+fakez), dFLOWERCLIPPINGS, 0, 0)); break;
		case -4: decorations.add(new dGrassClippings(x, y-(z+fakez), dGRASSCLIPPINGS, 0, 0)); break;
		default:
			if(death_sprite < 0) break;
			decorations.add(new comboSprite(x, y-(z+fakez), dCOMBOSPRITE, 0, death_sprite));
	}
	if(death_sfx > 0)
		sfx(death_sfx, pan(int32_t(x)));
	
	death_spawnitem = -1;
	death_item_pflags = 0;
	death_spawndropset = -1;
	death_sprite = -1;
	death_sfx = 0;
	rundeath = false;
}

void weapon::collision_check()
{
	findcombotriggers();
	if(isLWeapon)
	{
		check_enemy_lweapon_collision(this);
		(void)Hero.try_lwpn_hit(this);
	}
	else
	{
		(void)Hero.try_ewpn_hit(this);
	}
}

void weapon::onhit(bool clipped, enemy* e, int32_t ehitType)
{
    onhit(clipped, 0, -1, e, ehitType);
}

void weapon::onhit(bool clipped, int32_t special, int32_t linkdir, enemy* e, int32_t ehitType)
{
	if(!scriptcoldet || fallclk || drownclk)
	{
		// These won't hit anything, but they can still go too far offscreen...
		// Unless the compatibility rule is set.
		if(get_qr(qr_OFFSCREENWEAPONS) || !clipped)
			return;
		goto offscreenCheck;
	}
	
	if(special==2)                                            // hit Hero's mirror shield
	{
		bool reflect = false;
		switch(id)
		{
			case ewArrow:
				if(get_qr(qr_BROKEN_FLAME_ARROW_REFLECTING))
					break;
				id = wRefArrow;
				reflect = true;
				break;
			case ewFlame:
				if(get_qr(qr_BROKEN_FLAME_ARROW_REFLECTING))
					break;
				id = wRefFire;
				reflect = true;
				break;
			case ewFlame2:
				if(get_qr(qr_BROKEN_FLAME_ARROW_REFLECTING))
					break;
				id = wRefFire2;
				reflect = true;
				break;
				
			case ewFireball2:
			case ewFireball:
				id = wRefFireball;
				reflect = true;
				break;
				
			case ewRock:
				id = wRefRock;
				reflect = true;
				break;
			case ewSword:
				id = wRefBeam;
				reflect = true;
				break;
			case ewMagic:
				id = wRefMagic;
				reflect = true;
				break;
			case wRefBeam:
			case wRefMagic:
			case wRefArrow:
			case wRefFire:
			case wRefFire2:
				reflect = true;
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
				reflect = true;
				
				//prevents the block below from moving it to 'Lwpns'... needed for compat -Em
				isLWeapon = true;
				break;
		}
		if(reflect)
		{
			ignoreHero = true;
			if(!(replay_is_active()&&replay_get_meta_str("sav")=="link_to_the_heavens_16_of_17.sav"))
				ignorecombo = rpos_t::None;
			if(!isLWeapon)
			{
				if(Ewpns.remove(this))
					Lwpns.add(this);
				isLWeapon = true;
			}
			if(angular)
			{
				switch(linkdir)
				{
					case up:
						angle += (PI - angle) * 2.0;
						doAutoRotate();
						break;
						
					case down:
						angle = -angle;
						doAutoRotate();
						break;
						
					case left:
						angle += ((-PI/2) - angle) * 2.0;
						doAutoRotate();
						break;
						
					case right:
						angle += ((PI/2) - angle) * 2.0;
						doAutoRotate();
						break;
						
					default:
						angle += PI;
						doAutoRotate();
						break;
				}
			}
			else
			{
				dir ^= 1;
				
				if(dir&2)
					flip ^= 1;
				else
					flip ^= 2;
			}
			return;
		}
	}
	
	if(special>=1)                                            // hit Hero's shield
	{
		switch(id)
		{
			case ewRock:
			case ewMagic:
			case ewArrow:
			case ewSword:
				bounce=true;
				dead=16;
				return;
				
			case ewBrang:
				if(misc==0)
				{
					clk2=256;
					misc=1;
					dir^=1;
				}
				return;
			case wRefMagic:
				ignoreHero=true;
				ignorecombo=rpos_t::None;
				break;
		}
	}
    
offscreenCheck:
    
    if ( get_qr(qr_WEAPONSMOVEOFFSCREEN) || (screenedge&SPRITE_MOVESOFFSCREEN) ) goto skip_offscreenCheck;
    switch(id)
    {
    case wSword:
    case wWand:
    case wHammer:
    case wBugNet:
        break;
        
    case ewBomb:
        step=0;
        break;
        
    case ewLitBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-7;
        hit_width=hit_height=30;
        break;
        
    case ewSBomb:
        step=0;
        break;
        
    case ewLitSBomb:
        step=0;
        misc=50;
        clk=misc-3;
        hxofs=hyofs=-16;
        hit_width=hit_height=48;
        break;
        
    case wLitBomb:
        if(!clipped) dead=1;
        
    case wLitSBomb:
        if(!clipped) dead=1;
        
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wHSHandle:
    case wPhantom:
        break;                                   // don't worry about clipping or hits with these
        
    case ewFireTrail:
        if(!clipped) dead=1;
        
        break;
    
    case ewFlame:
    case wRefFire:
        if(!clipped) dead=1;
        break;
    case ewFlame2:
    case wRefFire2:
        dead=1;
        break;
        
    case wRefBeam:
    case wBeam:
        dead=23;
        break;
        
    case wArrow:
        dead=4;
        break;                           //trigger_secrets_if_flag(x,y,mfARROW,true); break;
        
    case ewArrow:
    case wRefArrow:
        dead=clipped?4:1;
        break;
        
    case wCByrna:
    
        // byrna keeps going
        if(parentitem<0 || !(itemsbuf[parentitem].flags&item_flag1))
            dead=0;
            
        break;
        
    case wWind:
        if(x>=240)
            dead=2;
            
        break;
        
	case wBrang:
	{
		if(e && e->switch_hooked && ehitType == 1)
		{
			dead = 0;
			break;
		}
		if(misc==0)
		{
			int32_t deadval=(itemsbuf[parentitem>-1 ? parentitem : current_item_id(itype_brang)].flags & item_flag3)?-2:4;
			clk2=256;
			if(clipped)
			{
				dead=deadval;
			}
			else
			{
				if(deadval==-2)
				{
					dead=deadval;
				}
				
				misc=1;
				/*
				  if (current_item(itype_brang)>1) {
				  if (dummy_bool[0]) {
				  add_grenade(x,y,z,current_item(itype_brang)>2);
				  dummy_bool[0]=false;
				  }
				  }
				  */
			}
		}
	}
	break;
        
    case wHookshot:
        if(misc==0)
        {
			if(family_class==itype_switchhook)
			{
				if(e && !switching_object && ehitType == -1)
				{
					switch(e->family)
					{
						case eeAQUA: case eeMOLD: case eeDONGO: case eeMANHAN: case eeGLEEOK:
						case eeDIG: case eeGHOMA: case eeLANM: case eePATRA: case eeGANON:
							break; //No segmented/z1boss allowed!
						default:
							switching_object = e;
					}
					if(switching_object)
					{
						switching_object->switch_hooked = true;
						hooked_comborpos = rpos_t::None;
						hooked_layerbits = 0;
						misc=2;
						step=0;
						Hero.doSwitchHook(parentitem>-1 ? (itemsbuf[parentitem].misc5) : game->get_switchhookstyle());
						if(parentitem > -1)
						{
							if(itemsbuf[parentitem].usesound2)
								sfx(itemsbuf[parentitem].usesound2,pan(int32_t(x)));
							else if(QMisc.miscsfx[sfxSWITCHED])
								sfx(QMisc.miscsfx[sfxSWITCHED],int32_t(x));
							stop_sfx(itemsbuf[parentitem].usesound);
						}
						else if(QMisc.miscsfx[sfxSWITCHED])
							sfx(QMisc.miscsfx[sfxSWITCHED],int32_t(x));
						break;
					}
				}
			}
			
			clk2=256;
			
			if(clipped)
				dead=4;
			else
				dead=1;
        }
        
        break;
        
    case ewBrang:
        if(misc==0)
        {
            clk2=256;
            dead=4;
        }
        
        break;
        
	case wRefMagic:
	case wMagic:
		dead=1; //remove the dead part to make the wand only die when clipped
		
		if ( get_qr(qr_BROKENBOOKCOST) )
		{
			//Create an ER to sue this in older quests -V
			//used a QR. -Z
			if(((id==wMagic && current_item(itype_book) &&
				(itemsbuf[current_item_id(itype_book)].flags&item_flag1))) && Lwpns.idCount(wFire)<2)
			{
				Lwpns.add(new weapon(x,y-fakez,z,wFire,2,zc_max(1, itemsbuf[current_item_id(itype_book)].misc4)*game->get_hero_dmgmult(),0,current_item_id(itype_book),-1,false,0,1));
				if ( FFCore.getQuestHeaderInfo(vZelda) < 0x255 ) 
				{
					sfx(WAV_FIRE,pan(x));
				}
				
				else sfx(itemsbuf[linkedItem].usesound > 0 ? itemsbuf[linkedItem].usesound : WAV_FIRE,pan(x));
			}
		}
		else
		{
			if(((id==wMagic && linkedItem && itemsbuf[linkedItem].family==itype_book &&
				(itemsbuf[linkedItem].flags&item_flag1))) && Lwpns.idCount(wFire)<2)
			{
				Lwpns.add(new weapon(x,y-fakez,z,wFire,2,zc_max(1, itemsbuf[linkedItem].misc4)*game->get_hero_dmgmult(),0,linkedItem,-1,false,0,1));
				if ( FFCore.getQuestHeaderInfo(vZelda) < 0x255 ) 
				{
					sfx(WAV_FIRE,pan(x));
				}
				else sfx(itemsbuf[linkedItem].usesound > 0 ? itemsbuf[linkedItem].usesound : WAV_FIRE,pan(x));
			}
        }
        break;
        
    case ewWind:
        if(clipped)
        {
            if(misc==999)                                       // in enemy wind
            {
                ewind_restart=true;
            }
            
            dead=1;
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
	{
		
		if(!clipped || !get_qr(qr_CHECKSCRIPTWEAPONOFFSCREENCLIP) ) dead=1;
		break;
	}
    default:
        dead=1;
    }
    skip_offscreenCheck:
    
    {
	//do not remove these braces!
    }
}

// override hit detection to check for invicibility, etc
bool weapon::hit()
{
    if(!scriptcoldet || fallclk || drownclk) return false;
    
	if(id==wBugNet) return false;
    if(id==ewBrang && misc)
        return false;
	if(Dead()&&dead!=-10) return false;
	return sprite::hit();
}
bool weapon::hit(sprite *s)
{
    if(!hit() || !s->hit()) return false;
	if(!get_qr(qr_OLD_BOMB_HITBOXES) && (id == wBomb || id == wSBomb || id == ewBomb || id == ewSBomb))
	{
		if(z+zofs >= s->z+s->zofs+s->hzsz || z+zofs+hzsz < s->z+s->zofs)
			return false;
		if(parentitem < 0 || itemsbuf[parentitem].misc7 < 1)
		{
			bool sbomb = id == wSBomb;
			for(int q = 0; q < (sbomb ? sbombcount : bombcount); ++q)
			{
				int tx = x+(sbomb?sbombxoff:bombxoff)[q];
				int ty = y-fakez+(sbomb?sbombyoff:bombyoff)[q];
				if(s->hit(tx,ty,16,16))
					return true;
			}
			return false;
		}
		else
		{
			int rad = itemsbuf[parentitem].misc7;
			return (distance(x+8,y+8-fakez,s->x+s->hit_width/2,s->y+s->hit_height/2) <= rad);
		}
	}
    return sprite::hit(s);
}

bool weapon::hit(int32_t tx2,int32_t ty2,int32_t tz2,int32_t txsz2,int32_t tysz2,int32_t tzsz2)
{
    if(!hit()) return false;
	if(!get_qr(qr_OLD_BOMB_HITBOXES) && (id == wBomb || id == wSBomb || id == ewBomb || id == ewSBomb))
	{
		if(z+zofs >= tz2+tzsz2 || z+zofs+hzsz < tz2)
			return false;
		if(parentitem < 0 || itemsbuf[parentitem].misc7 < 1)
		{
			bool sbomb = id == wSBomb;
			for(int q = 0; q < (sbomb ? sbombcount : bombcount); ++q)
			{
				int tx = x+(sbomb?sbombxoff:bombxoff)[q];
				int ty = y-fakez+(sbomb?sbombyoff:bombyoff)[q];
				if(tx >= tx2+txsz2 || ty >= ty2+tysz2
					|| tx+15 < tx2 || ty+15 < ty2)
					continue;
				return true;
			}
			return false;
		}
		else
		{
			int rad = itemsbuf[parentitem].misc7;
			return (distance(x+8,y+8-fakez,tx2+txsz2/2,ty2+tysz2/2) <= rad);
		}
	}
    return sprite::hit(tx2,ty2,tz2,txsz2,tysz2,tzsz2);
}
bool weapon::hit(int32_t tx2,int32_t ty2,int32_t txsz2,int32_t tysz2)
{
    if(!hit()) return false;
    if(!get_qr(qr_OLD_BOMB_HITBOXES) && (id == wBomb || id == wSBomb || id == ewBomb || id == ewSBomb))
	{
		if(parentitem < 0 || itemsbuf[parentitem].misc7 < 1)
		{
			bool sbomb = id == wSBomb;
			for(int q = 0; q < (sbomb ? sbombcount : bombcount); ++q)
			{
				int tx = x+(sbomb?sbombxoff:bombxoff)[q];
				int ty = y-fakez+(sbomb?sbombyoff:bombyoff)[q];
				if(tx >= tx2+txsz2 || ty >= ty2+tysz2
					|| tx+15 < tx2 || ty+15 < ty2)
					continue;
				return true;
			}
			return false;
		}
		else
		{
			int rad = itemsbuf[parentitem].misc7;
			return (distance(x+8,y+8-fakez,tx2+txsz2/2,ty2+tysz2/2) <= rad);
		}
	}
	return sprite::hit(tx2,ty2,txsz2,tysz2);
}

void weapon::update_weapon_frame(int32_t change, int32_t orig)
{
    if(extend > 2)
    {
        byte extra_rows=(tysz-1)*(((orig%TILES_PER_ROW)+(txsz*change))/TILES_PER_ROW);
        tile=orig+(txsz*change)+(extra_rows*TILES_PER_ROW);
    }
    else
        tile=orig+change;
}

void weapon::doAutoRotate(bool dodir, bool doboth)
{
	if (autorotate)
	{
		if (angular && (!dodir || doboth))
		{
			rotation = angle*(180.0 / PI);
		}
		else if (dodir || doboth)
		{
			rotation = DirToDegrees(dir);
		}
	}
}

void weapon::draw(BITMAP *dest)
{
	if(weapon_dying_frame) return;
	if(fallclk || drownclk)
	{
		sprite::draw(dest);
		return;
	}
	bool suspt = (FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon) || (FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon);
	if(!suspt)
	{
		if(flash==1)
		{
			if(!BSZ)
			{
				cs = (id==wBeam || id==wRefBeam) ? 6 : o_cset&15;
				cs += frame&3;
			}
			else
			{
				if(id==wBeam || id==wRefBeam)
					cs = ((frame>>2)&1)+7;
				else
				{
					cs = o_cset&15;
					
					if(++csclk >= 12)
						csclk = 0;
						
					cs += csclk>>2;
				}
			}
		}
		
		if(flash>1)
		{
			if(++csclk >= (o_speed<<1))
				csclk = 0;
				
			cs = o_cset&15;
			
			if(csclk >= o_speed)
				cs = o_cset>>4;
		}
		
		if(frames)
		{
			if(id!=wBrang && id!=ewBrang)  // Brangs use clk2 for adjusting boomerang's flight range.
			{
				if(++clk2 >= o_speed)
				{
					clk2 = 0;
					
					if(frames>1 && ++aframe >= frames)
					{
						aframe = 0;
					}
				}
			}
			else
			{
				if((clk2 % zc_max(1, o_speed))==0)
				{
					if(frames>1 && ++aframe >= frames)
					{
						aframe = 0;
					}
				}
			}
			
			//tile = o_tile+aframe;
			if ( do_animation ) 
			{
				update_weapon_frame(aframe,o_tile);
			}
		}
	}
	
	
	// do special case stuff
	switch(id)
	{
		case wSword:
		case wHammer:
			if(Hero.is_hitflickerframe() ||
					Hero.getDontDraw() || hero_scr->flags3&fINVISHERO)
				return;
				
		case wBeam:
		case wRefBeam:
		{
			if(dead==-1) break;
			if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
			{
				// draw the beam shards
				int32_t ofs=23-dead;
				int32_t f = frame&3;
				int32_t type2 = o_type;
				//tile = o_tile+((frames?frames:1)*2);
				update_weapon_frame(((frames?frames:1)*2),ref_o_tile);
				
				if(type2)
					cs = o_cset>>4;
				if ( do_animation ) 
				{
					if(type2==3 || type2 == 4 && (f&2))
						++tile;
				}
				
				int beam_x = x - viewport.x;
				int beam_y = y + playing_field_offset - viewport.y;
				if(!type2 || type2 == 4 || f==0 || (type2>1 && f==3)) overtile16(dest,tile,beam_x-2-ofs,beam_y-2-ofs-(z+zofs)-fakez,cs,0);
				
				if(!type2 || type2 == 4 || f==2 || (type2>1 && f==1)) overtile16(dest,tile,beam_x+2+ofs,beam_y-2-ofs-(z+zofs)-fakez,cs,1);
				
				if(!type2 || type2 == 4 || f==1 || (type2>1 && f==2)) overtile16(dest,tile,beam_x-2-ofs,beam_y+2+ofs-(z+zofs)-fakez,cs,2);
				
				if(!type2 || type2 == 4 || f==3 || (type2>1 && f==0)) overtile16(dest,tile,beam_x+2+ofs,beam_y+2+ofs-(z+zofs)-fakez,cs,3);
			}
		}
		
		return;											   // don't draw sword
		
		case wBomb:
		case wSBomb:
		case ewBomb:
		case ewSBomb:
		{
			if(!misc || clk<misc-2) break;
			if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
			{
				// draw the explosion
				int32_t id2=0;
				int32_t boomframes = 0;
				
				switch(id)
				{
				case wBomb:
				{
					id2=wBOOM;
					
					if(parentitem>-1)
					{
						id2=itemsbuf[parentitem].wpn2;
					}
					
					break;
				}
				
				case wSBomb:
				{
					id2=wSBOOM;
					
					if(parentitem>-1)
					{
						id2=itemsbuf[parentitem].wpn2;
					}
					
					break;
				}
				
				case ewBomb:
					id2=ewBOOM;
					break;
					
				case ewSBomb:
					id2=ewSBOOM;
					break;
				}
				
				tile = wpnsbuf[id2].tile;
				cs = wpnsbuf[id2].csets&15;
				boomframes = wpnsbuf[id2].frames;
				
				if(boomframes != 0)
				{
					//equally divide up the explosion time among the frames
					int32_t perframe = (boomframes >= 34 ? 1 : (boomframes + 34)/boomframes);
					if ( do_animation ) 
					{
						if(clk > misc)
							tile += (clk-misc)/perframe;
					}
					//update_weapon_frame((clk-misc)/perframe,tile);
				}
				else if(clk>misc+22)
				{
					if ( do_animation ) ++tile;
					//update_weapon_frame(1,tile);
				}
			}
			
			int x0 = x - viewport.x;
			int y0 = y - viewport.y;
			overtile16(dest,tile,x0+((clk&1)?7:-7),y0+yofs-fakez-13-(z+zofs),cs,0);
			overtile16(dest,tile,x0,y0+yofs-fakez-(z+zofs),cs,0);
			overtile16(dest,tile,x0+((clk&1)?-14:14),y0+yofs-fakez-(z+zofs),cs,0);
			overtile16(dest,tile,x0+((clk&1)?-7:7),y0+yofs+14-fakez-(z+zofs),cs,0);
			
			if(id==wSBomb||id==ewSBomb)
			{
				overtile16(dest,tile,x0+((clk&1)?7:-7),y0+yofs-27-fakez-(z+zofs),cs,0);
				overtile16(dest,tile,x0+((clk&1)?-21:21),y0+yofs-13-fakez-(z+zofs),cs,0);
				overtile16(dest,tile,x0+((clk&1)?-28:28),y0+yofs-fakez-(z+zofs),cs,0);
				overtile16(dest,tile,x0+((clk&1)?21:-21),y0+yofs+14-fakez-(z+zofs),cs,0);
				overtile16(dest,tile,x0+((clk&1)?-7:7),y0+yofs+28-fakez-(z+zofs),cs,0);
			}
			else
			{
				;
			}
			
			if(get_debug() && zc_getkey(KEY_O))
				rectfill(dest,x0+hxofs,y0+hyofs+yofs-(z+zofs)-fakez,
						 x0+hxofs+hit_width-1,y0+hyofs+hit_height-1+yofs-fakez,vc(id));
			
			if(show_hitboxes)
				draw_hitbox();
			return;											   // don't draw bomb
		}
		
		case wArrow:
		case wRefArrow:
		case ewArrow:
			if(dead>0 && !bounce)
			{
				if ( do_animation ) 
				{
					if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
					{
						cs=7;
						tile=54;
						flip=0;
					}
				}
			}
			
			break;
			
		case ewFireTrail:
		case ewFlame:
		case wFire:
		
			//if no animation, flip tile
			if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
			{
				if(frames==0 && do_animation ) //do_animation is a ZScript setting. -Z
					flip = o_flip & (clk>>2);
			}
			break;
			
		case ewBrang:
		case wBrang:
			if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
			{
				cs = o_cset&15;
				
				if((id == wBrang && (parentitem<0 || !(itemsbuf[parentitem].flags & item_flag1)))
					|| (id == ewBrang && !get_qr(qr_CORRECTED_EW_BRANG_ANIM)))
				{
					if ( do_animation ) 
					{
						tile = o_tile;
						
						if(BSZ)
						{
							flip = bszboomflip[(clk>>2)&3];
						}
						else
						{
							//Okay, so since this code confused me, lemme explain it.
							//that &0xE will cause it to A: loop to a range of 0-15, and B: truncate any odd numbers 
							//to the even number below it (because it is not &0xF, the least significant bit is not 
							//being saved, which means it's only treating it as an even number).
							
							//Basically, boomframe[] is an array in which even numbers are the frame offset from o_tile for that frame,
							//and the odd number above it is the flip for that frame; and each frame lasts two... well, frames. This results
							//in a 16 frame animation using only an A.Speed of 2, and only 3 tiles. It's used to save tile space in older versions.
							update_weapon_frame(boomframe[clk&0xE],o_tile);
							flip = boomframe[(clk&0xE)+1];
						}
						
						if(parentitem>=0 && itemsbuf[parentitem].flags & item_flag2)
						{
						update_weapon_frame((BSZ?1:4)*dir,tile);
						}
					}
				}
				else
				{
					if(parentitem>=0 && itemsbuf[parentitem].flags & item_flag2)
					{
						if ( do_animation )update_weapon_frame(zc_max(frames,1)*dir,tile);
					}
				}
				
				if(dead>0)
				{
					if(parentitem>=0 && itemsbuf[parentitem].flags & item_flag1)
					{
						if ( do_animation ) 
						{
							tile=o_tile+(frames*(itemsbuf[parentitem].flags & item_flag2)?8:1);
						}
					}
					else
					{
						if ( do_animation ) 
						{
							cs=7;
							tile=54;
							flip=0;
						}
					}
				}
			}
			
			break;
			
		case wHookshot:
			break;
			
		case wWind:
			if ( !FFCore.system_suspend[susptLWEAPONS] && this->isLWeapon || !FFCore.system_suspend[susptEWEAPONS] && !this->isLWeapon)
			{
				if(frames==0 && do_animation)
					flip ^= o_flip;
					
				if(Dead() && !BSZ && do_animation)
					tile = temp1;//wpnsbuf[wFIRE].tile;
			}
				
			break;
			
		case ewWind:
			/*
			  if(wpnsbuf[wid].frames==0)
			  flip ^= (wpnsbuf[wid].misc>>2)&3;
			  */
			break;
			
		case wPhantom:
			switch(type)
			{
			case pDIVINEPROTECTIONROCKET1:
			case pDIVINEPROTECTIONROCKETRETURN1:
			case pDIVINEPROTECTIONROCKETTRAIL1:
			case pDIVINEPROTECTIONROCKETTRAILRETURN1:
			case pDIVINEPROTECTIONROCKET2:
			case pDIVINEPROTECTIONROCKETRETURN2:
			case pDIVINEPROTECTIONROCKETTRAIL2:
			case pDIVINEPROTECTIONROCKETTRAILRETURN2:
				if(parentitem>=0 && (itemsbuf[parentitem].flags & item_flag1 ? 1 : 0)&&!(frame&1))
				{
					return;
				}
				
				break;
			}
			
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
		{
			if ( do_animation )
			{
				//Bugfix script weapons not animating:
				//Let's see if this works, and failstobreakanything. -Z
				//This also will need a QR, if it works!
				/* Bugged, disabling.
				if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && get_qr(qr_ANIMATECUSTOMWEAPONS) )
				{
					if(frames>1 && ++aframe >= frames)
					{
						aframe = 0;
					}
					//update_weapon_frame(aframe,o_tile);
					update_weapon_frame(aframe,o_tile);
				}
				*/
				//if ( ScriptGenerated && script_wrote_otile && aframe > 0 ) 
				//{ 
				//	script_wrote_otile = 0; // NOTES and ISSUES
								// I honestly do not recall when or why I added this. I think that it was
								// in an attempt to fix Tile not being reset when writing to OTile. 
								// 
								// PROBLEM
								// Doing any of this on the first frame of a weapon will overwrite a script-set
								// tile with the original tile if the scripter does this in a script:
								//
								// this->OriginalTile = 6;
								// this->Tile = 12345; 	// will be overwritten by o_tile because of the 
								//			// script_wrote_otile FLAG being checked by the engine
								//			// after the script writes to tile! -Z 26th October, 2019
					
					//tile = o_tile; //This will overwrite the tile on the calls above, so we can't do it. Fuck it. 
				//}
			}
		}
		break;
	}
	
	// draw it
	
	if (has_shadow && (z > 0||fakez > 0) && get_qr(qr_WEAPONSHADOWS) )
	{
		wpndata const& spr = wpnsbuf[spr_shadow];
		if(!suspt)
		{
			if(++shd_aclk >= zc_max(spr.speed,1))
			{
				shd_aclk = 0;
				if(++shd_aframe >= zc_max(spr.frames,1))
				{
					shd_aframe = 0;
				}
			}
		}
		shadowtile = spr.tile+shd_aframe;
		sprite::drawshadow(dest,get_qr(qr_TRANSSHADOWS) != 0);
	}
	sprite::draw(dest);
}

void putweapon(BITMAP *dest,int32_t x,int32_t y,int32_t weapon_id, int32_t type, int32_t dir, int32_t &aclk, int32_t &aframe, int32_t parentid)
{
    weapon temp((zfix)x,(zfix)y,(zfix)0,weapon_id,type,0,dir,-1,parentid,true);
    temp.ignorecombo=COMBOPOS_REGION_B(dir==left?x+8:x, dir==up?y+8:y); // Lens hints can sometimes create real weapons without this
    temp.ignoreHero=true;
    temp.yofs=0;
    temp.clk2=aclk;
    temp.aframe=aframe;
    temp.script = 0; //Can not have script data.
    temp.animate(0); //Scripts run in this function. Call after forcing script data to 0.
    temp.draw(dest);
    aclk=temp.clk2;
    aframe=temp.aframe;
}

void weapon::findcombotriggers()
{
	if(!CanComboTrigger(this)) return;
	int32_t layercount = get_qr(qr_CUSTOMCOMBOS_EVERY_LAYER) ?
		7 : ((get_qr(qr_CUSTOMCOMBOSLAYERS1AND2)) ? 3 : 1);
	if(!get_qr(qr_OLD_BOMB_HITBOXES) && (id == wBomb || id == wSBomb || id == ewBomb || id == ewSBomb))
	{
		std::set<rpos_t> rposes = getBombPositions();
		for (rpos_t rpos : rposes)
		{
			auto [x, y] = COMBOXY_REGION(rpos);

			for (int32_t ly = 0; ly < layercount; ++ly )
				MatchComboTrigger2(this, x, y, ly);
			if(misc_wflags & WFLAG_BURNFLAGS)
				triggerfire(x, y, this, true,
					misc_wflags&WFLAG_BURN_ANYFIRE,
					misc_wflags&WFLAG_BURN_STRONGFIRE,
					misc_wflags&WFLAG_BURN_MAGICFIRE,
					misc_wflags&WFLAG_BURN_DIVINEFIRE);
		}
		return;
	}
	for(int32_t dx = 0; dx < hit_width; dx += 16)
	{
		for(int32_t dy = 0; dy < hit_height; dy += 16)
		{
			for (int32_t ly = 0; ly < layercount; ++ly )
			{
				MatchComboTrigger2(this, (int32_t)x+dx+hxofs, (int32_t)y+dy+hyofs-fakez, ly);
			}
		}
		for (int32_t ly = 0; ly < layercount; ++ly )
		{
			MatchComboTrigger2(this, (int32_t)x+dx+hxofs, (int32_t)y+hyofs+(hit_height-1)-fakez, ly);
		}
	}
	for(int32_t dy = 0; dy < hit_height; dy += 16)
	{
		for (int32_t ly = 0; ly < layercount; ++ly )
		{
			MatchComboTrigger2(this, (int32_t)x+hxofs+(hit_width-1), (int32_t)y+dy+hyofs-fakez, ly);
		}
	}
	for (int32_t ly = 0; ly < layercount; ++ly )
	{
		MatchComboTrigger2(this, (int32_t)x+hxofs+(hit_width-1), (int32_t)y+hyofs+(hit_height-1)-fakez, ly);
	}
}

int32_t weapon::run_script(int32_t mode)
{
	if(switch_hooked && !get_qr(qr_SWITCHOBJ_RUN_SCRIPT)) return RUNSCRIPT_OK;
	if (weaponscript <= 0 || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[isLWeapon ? susptLWEAPONSCRIPTS : susptEWEAPONSCRIPTS])
		return RUNSCRIPT_OK;
	auto scrty = *get_scrtype();
	auto uid = getUID();
	if(!FFCore.doscript(scrty,uid))
		return RUNSCRIPT_OK;
	int32_t ret = RUNSCRIPT_OK;
	bool& waitdraw = FFCore.waitdraw(scrty, uid);
	switch(mode)
	{
		case MODE_NORMAL:
			return ZScriptVersion::RunScript(scrty, weaponscript, uid);
		case MODE_WAITDRAW:
			if(waitdraw)
			{
				ret = ZScriptVersion::RunScript(scrty, weaponscript, uid);
				waitdraw = false;
			}
			break;
	}
    return ret;
}
ALLEGRO_COLOR weapon::hitboxColor(byte opacity) const
{
	return al_map_rgba(isLWeapon ? 0 : 255,255,0,opacity);
}
void weapon::draw_hitbox()
{
	if(hide_hitbox) return;
	if(!get_qr(qr_OLD_BOMB_HITBOXES) && (id == wBomb || id == wSBomb || id == ewBomb || id == ewSBomb))
	{
#ifndef IS_EDITOR
		start_info_bmp();
		if(parentitem < 0 || itemsbuf[parentitem].misc7 < 1)
		{
			bool sbomb = id == wSBomb || id == ewSBomb;
			for(int q = 0; q < (sbomb ? sbombcount : bombcount); ++q)
			{
				int tx = x+(sbomb?sbombxoff:bombxoff)[q];
				int ty = y-fakez+(sbomb?sbombyoff:bombyoff)[q]+playing_field_offset;
				al_draw_rectangle(tx,ty,tx+15,ty+15,hitboxColor(info_opacity),1);
			}
		}
		else
		{
			al_draw_circle(x+8, y+8-fakez+playing_field_offset, itemsbuf[parentitem].misc7, hitboxColor(info_opacity), 1);
		}
		end_info_bmp();
#endif
		return;
	}
	sprite::draw_hitbox();
}
//Dummy weapon for visual effects.
weapon::weapon(zfix X,zfix Y,zfix Z,int32_t Id,int32_t usesprite, int32_t Dir, int32_t step, int32_t prntid, int32_t height, int32_t width, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g) : sprite(), parentid(prntid)
{
	screen_spawned=get_screen_for_world_xy(x.getInt(), y.getInt());
	unblockable = 0;
	misc_wflags = WFLAG_NONE;
	death_spawnitem = -1;
	death_spawndropset = -1;
	death_sprite = -1;
	death_sfx = 0;
	lift_level = 0;
	lift_time = 16;
	lift_height = 8;
	has_shadow = true;
	death_item_pflags = 0;
    x=X;
    y=Y;
    z=Z;
	txsz = width > 0 ? width : 1;
	tysz = height > 0 ? height : 1;
    id=Id;
    type=0;
    power=0;
    specialinfo = 0;
    parentitem=-1;
    dir=zc_max(Dir,0);
    clk=clk2=flip=misc=misc2=0;
    frames=flash=wid=aframe=csclk=0;
    ignorecombo=rpos_t::None;
    step=0;
    dead=-1;
    ref_o_tile = 0;
    bounce= false;
	ignoreHero=true;
    yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) - 2;
    dragging=-1;
    width=1;
    height=1;
    hzsz=1;
	hyofs = -32768;
    useweapon = usedefence = 0;
    weaprange = weapduration = 0;
    script_wrote_otile = 0;
    isLWeapon = 0;
    ScriptGenerated = 0;
    LOADGFX(usesprite);
    step=0;
    weaponscript = 0;
	weapon_dying_frame = false;
	rundeath = false;
	shd_aclk = shd_aframe = 0;
	last_burnstate = 0;
}

