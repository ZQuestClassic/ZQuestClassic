#include "core/qrs.h"
#include "core/qst.h"
#include "defdata.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern char *guy_string[eMAXGUYS];
extern const char *old_guy_string[OLDMAXGUYS];
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];
extern bool fixpolsvoice;

namespace {

void guy_update_firesfx(guydata& tempguy)
{
	tempguy.firesfx = 0;
	if (tempguy.type == eeWIZZ)
	{
		switch (tempguy.attributes[1])
		{
		case 0: // normal weapon
			tempguy.firesfx = WAV_WAND;
			break;
		case 1: // 8 shots
			if (get_qr(qr_8WAY_SHOT_SFX_DEP)) tempguy.firesfx = WAV_FIRE;
			else
			{
				switch (tempguy.weapon)
				{
				case ewFireTrail:
				case ewFlame:
				case ewFlame2Trail:
				case ewFlame2:
					tempguy.firesfx = WAV_FIRE;
					break;
				case ewWind:
				case ewMagic:
					tempguy.firesfx = WAV_WAND;
					break;
				case ewIce:
					tempguy.firesfx = WAV_ZN1ICE;
					break;
				case ewRock:
					tempguy.firesfx = WAV_ZN1ROCK;
					break;
				case ewFireball2:
				case ewFireball:
					tempguy.firesfx = WAV_ZN1FIREBALL;
					break;
				case ewBrang:
					tempguy.firesfx = WAV_BRANG;
					break;
				case ewBomb:case ewSBomb: case ewLitBomb:case ewLitSBomb:
					tempguy.firesfx = WAV_BOMB;
					break;
				default:
					tempguy.firesfx = 0;
					break;
				}
				break;
			}
		case 2: // Summon
			tempguy.firesfx = WAV_FIRE;
			break;
		case 3: // Summon Layer
			tempguy.firesfx = get_qr(qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
			break;
		}
	}
	else
	{
		if ((tempguy.type == eeWALK || tempguy.type == eePROJECTILE) && (tempguy.attributes[0] == e1tSUMMON || tempguy.attributes[0] == e1tSUMMONLAYER))
		{
			tempguy.firesfx = get_qr(qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
		}
		else
		{
			switch (tempguy.weapon)
			{
			case ewFireTrail:
			case ewFlame:
			case ewFlame2Trail:
			case ewFlame2:
				tempguy.firesfx = WAV_FIRE;
				break;
			case ewWind:
			case ewMagic:
				tempguy.firesfx = WAV_WAND;
				break;
			case ewIce:
				tempguy.firesfx = WAV_ZN1ICE;
				break;
			case ewRock:
				tempguy.firesfx = WAV_ZN1ROCK;
				break;
			case ewFireball2:
			case ewFireball:
				tempguy.firesfx = WAV_ZN1FIREBALL;
				break;
			case ewBrang:
				tempguy.firesfx = WAV_BRANG;
				break;
			case ewBomb:case ewSBomb: case ewLitBomb:case ewLitSBomb:
				tempguy.firesfx = WAV_BOMB;
				break;
			default:
				tempguy.firesfx = 0;
				break;
			}
		}
	}
}

void guy_update_weaponflags(guydata& tempguy)
{
	tempguy.weap_data.unblockable = 0;
	tempguy.weap_data.moveflags = move_none;
	if (tempguy.weapon == ewFlame || tempguy.weapon == ewFireTrail)
		tempguy.weap_data.moveflags |= (move_obeys_grav | move_can_pitfall);
	tempguy.weap_data.override_flags = 0;
	tempguy.weap_data.hxofs = 0;
	tempguy.weap_data.hyofs = 0;
	tempguy.weap_data.hxsz = 0;
	tempguy.weap_data.hysz = 0;
	tempguy.weap_data.hzsz = 0;
	tempguy.weap_data.xofs = 0;
	tempguy.weap_data.yofs = 0;
	tempguy.weap_data.tilew = 0;
	tempguy.weap_data.tileh = 0;
	switch (tempguy.weapon)
	{
	case ewFireball: case ewFireball2:
		tempguy.weap_data.step = 1.75_zf;
		break;
	case ewSword: case ewLitBomb: case ewLitSBomb:
	case ewRock:  case ewMagic: case ewWind:
		tempguy.weap_data.step = 3_zf;
		break;
	case ewArrow:
		tempguy.weap_data.step = 2_zf;
		break;
	case ewFlame: case ewFlame2:
		tempguy.weap_data.step = 1_zf;
		break;
	default:
		tempguy.weap_data.step = 0_zf;
		break;
	}
	for (int q = 0; q < WPNSPR_MAX; ++q)
	{
		tempguy.weap_data.burnsprs[q] = 0;
		tempguy.weap_data.light_rads[q] = 0;
	}
}

void guy_update_weaponspecialsfx(guydata& tempguy)
{
	switch (tempguy.weapon)
	{
	case ewBrang:
		tempguy.specialsfx = WAV_BRANG;
		break;
	case ewBomb: case ewSBomb: case ewLitBomb:case ewLitSBomb:
		tempguy.specialsfx = WAV_BOMB;
		break;
	default:
		tempguy.specialsfx = 0;
		break;
	}
}

void update_guy_1(guydata *tempguy) // November 2009
{
    bool doesntcount = false;
    tempguy->flags &= ~guy_weak_arrow; // Formerly 'weak to arrow' which wasn't implemented
    
    switch(tempguy->type)
    {
    case 1: //eeWALK
        switch(tempguy->attributes[9])
        {
        case 0: //Stalfos
            if(tempguy->attributes[0]==1)  // Fires four projectiles at once
                tempguy->attributes[0]=4;
                
            break;
            
        case 1: //Darknut
            goto darknuts;
            break;
        }
        
        tempguy->attributes[9] = 0;
        break;
        
    case 2: //eeSHOOT
        tempguy->type = eeWALK;
        
        switch(tempguy->attributes[9])
        {
        case 0: //Octorok
            if(tempguy->attributes[0]==1||tempguy->attributes[0]==2)
            {
                tempguy->attributes[0]=e1tFIREOCTO;
                tempguy->attributes[1]=e2tFIREOCTO;
            }
            else tempguy->attributes[0] = 0;
            
            tempguy->attributes[5]=tempguy->attributes[3];
            tempguy->attributes[3]=tempguy->attributes[2];
            tempguy->attributes[2]=0;
            break;
            
        case 1: // Moblin
            tempguy->attributes[0] = 0;
            break;
            
        case 2: //Lynel
            tempguy->attributes[5]=tempguy->attributes[0]+1;
            tempguy->attributes[0]=0;
            break;
            
        case 3: //Stalfos 2
            if(tempguy->attributes[0]==1)  // Fires four projectiles at once
                tempguy->attributes[0]=e1t4SHOTS;
            else tempguy->attributes[0] = 0;
            
            break;
            
        case 4: //Darknut 5
darknuts:
            tempguy->defense[edefFIRE] = edIGNORE;
            tempguy->defense[edefBRANG] = edSTUNORCHINK;
            tempguy->defense[edefHOOKSHOT] = 0;
            tempguy->defense[edefARROW] = tempguy->defense[edefBYRNA] = tempguy->defense[edefREFROCK] =
                                              tempguy->defense[edefMAGIC] = tempguy->defense[edefSTOMP] = edCHINK;
                                              
            if(tempguy->attributes[0]==1)
                tempguy->attributes[0]=2;
            else if(tempguy->attributes[0]==2)
            {
                tempguy->attributes[3]=tempguy->attributes[2];
                tempguy->attributes[2]=tempguy->attributes[1];
                tempguy->attributes[1]=e2tSPLIT;
                tempguy->attributes[0] = 0;
            }
            else tempguy->attributes[0] = 0;
            
            tempguy->flags |= guy_shield_front;
            
            if(!get_bit(deprecated_rules,qr_BRKBLSHLDS_DEP))
                tempguy->flags &= ~guy_bkshield;
			else
				tempguy->flags |= guy_bkshield;
                
            break;
        }
        
        tempguy->attributes[9] = 0;
        break;
        
        /*
        		case 9: //eeARMOS
        		tempguy->family = eeWALK;
        		break;
        */
    case 11: //eeGEL
    case 33: //eeGELTRIB
        if(tempguy->type==33)
        {
            tempguy->attributes[3] = 1;
            
            if(get_bit(deprecated_rules, qr_OLDTRIBBLES_DEP))  //Old Tribbles
                tempguy->attributes[2] = tempguy->attributes[1];
                
            tempguy->attributes[1] = e2tTRIBBLE;
        }
        else
        {
            tempguy->attributes[3] = 0;
            tempguy->attributes[2] = 0;
            tempguy->attributes[1] = 0;
        }
        
        tempguy->type = eeWALK;
        
        if(tempguy->attributes[0])
        {
            tempguy->attributes[0]=1;
            tempguy->weapon = ewFireTrail;
        }
        
        break;
        
    case 34: //eeZOLTRIB
    case 12: //eeZOL
        tempguy->attributes[3]=tempguy->attributes[2];
        tempguy->attributes[2]=tempguy->attributes[1];
        tempguy->type = eeWALK;
        tempguy->attributes[1]=e2tSPLITHIT;
        
        if(tempguy->attributes[0])
        {
            tempguy->attributes[0]=1;
            tempguy->weapon = ewFireTrail;
        }
        
        break;
        
    case 13: //eeROPE
        tempguy->type = eeWALK;
        tempguy->attributes[8] = e9tROPE;
        
        if(tempguy->attributes[0])
        {
            tempguy->attributes[3] = tempguy->attributes[2];
            tempguy->attributes[2] = tempguy->attributes[1];
            tempguy->attributes[1] = e2tBOMBCHU;
        }
        
        tempguy->attributes[0] = 0;
        break;
        
    case 14: //eeGORIYA
        tempguy->type = eeWALK;
        
        if(tempguy->attributes[0]!=2) tempguy->attributes[0] = 0;
        
        break;
        
    case 17: //eeBUBBLE
        tempguy->type = eeWALK;
        tempguy->attributes[7] = tempguy->attributes[1];
        tempguy->attributes[6] = tempguy->attributes[0] + 1;
        tempguy->attributes[0] = tempguy->attributes[1] = 0;
        
        //fallthrogh
    case eeTRAP:
    case eeROCK:
        doesntcount = true;
        break;
        
    case 35: //eeVIRETRIB
    case 18: //eeVIRE
        tempguy->type = eeWALK;
        tempguy->attributes[3]=tempguy->attributes[2];
        tempguy->attributes[2]=tempguy->attributes[1];
        tempguy->attributes[1]=e2tSPLITHIT;
        tempguy->attributes[8]=e9tVIRE;
        break;
        
    case 19: //eeLIKE
        tempguy->type = eeWALK;
        tempguy->attributes[6] = e7tEATITEMS;
        tempguy->attributes[7]=95;
        break;
        
    case 20: //eePOLSV
        tempguy->defense[edefBRANG] = edSTUNORCHINK;
        tempguy->defense[edefBOMB] = tempguy->defense[edefSBOMB] = tempguy->defense[edefFIRE] = edIGNORE;
        tempguy->defense[edefMAGIC] = tempguy->defense[edefBYRNA] = edCHINK;
        tempguy->defense[edefARROW] = ed1HKO;
        tempguy->defense[edefHOOKSHOT] = edSTUNONLY;
        tempguy->type = eeWALK;
        tempguy->attributes[8] = e9tPOLSVOICE;
        tempguy->rate = 4;
        tempguy->homing = 32;
        tempguy->hrate = 10;
        tempguy->grumble = 0;
        break;
        
    case eeWIZZ:
        if(tempguy->attributes[3])
        {
            for(int32_t i=0; i < edefLAST; i++)
                tempguy->defense[i] = (i != edefREFBEAM && i != edefREFMAGIC && i != edefQUAKE) ? edIGNORE : 0;
        }
        else
        {
            tempguy->defense[edefBRANG] = edSTUNORCHINK;
            tempguy->defense[edefMAGIC] = edCHINK;
            tempguy->defense[edefHOOKSHOT] = edSTUNONLY;
            tempguy->defense[edefARROW] = tempguy->defense[edefFIRE] =
                                              tempguy->defense[edefWAND] = tempguy->defense[edefBYRNA] = edIGNORE;
        }
        
        break;
        
    case eePEAHAT:
        tempguy->flags &= ~(guy_superman|guy_sbombonly);
        
        if(!(tempguy->flags & guy_bhit))
            tempguy->defense[edefBRANG] = edSTUNONLY;
            
        break;
        
    case eeLEV:
        tempguy->defense[edefSTOMP] = edCHINK;
        break;
    }
    
    // Old flags
    if(tempguy->flags & guy_superman)
    {
        for(int32_t i = 0; i < edefLAST; i++)
            if(!(i==edefSBOMB && (tempguy->flags & guy_sbombonly)))
                tempguy->defense[i] = (i==edefBRANG && tempguy->defense[i] != edIGNORE
                                       && tempguy->type != eeROCK && tempguy->type != eeTRAP
                                       && tempguy->type != eePROJECTILE) ? edSTUNORIGNORE : edIGNORE;
    }
    
    tempguy->flags &= ~(guy_superman|guy_sbombonly);
    
    if(doesntcount)
        tempguy->flags |= (guy_doesnt_count);
}

} // end namespace

void init_guys(int32_t guyversion)
{
    for(int32_t i=0; i<MAXGUYS; i++)
    {
        guysbuf[i] = default_guys[0];
    }
    
    for(int32_t i=0; i<OLDMAXGUYS; i++)
    {
        guysbuf[i] = default_guys[i];
        guysbuf[i].spr_shadow = (guysbuf[i].type==eeROCK && guysbuf[i].attributes[9] == 1) ? iwLargeShadow : iwShadow;
		guysbuf[i].spr_death = iwDeath;
		guysbuf[i].spr_spawn = iwSpawn;
        // Patra fix: 2.10 BSPatras used spDIG. 2.50 Patras use CSet 7.
        if(guyversion<=3 && i==ePATRABS)
        {
            guysbuf[i].bosspal=spDIG;
            guysbuf[i].cset=14;
            guysbuf[i].attributes[8] = 14;
        }
        
        if(guyversion<=3)
        {
            // Rope/Ghini Flash rules
            if(get_bit(deprecated_rules, qr_NOROPE2FLASH_DEP))
            {
                if(i==eROPE2)
                {
                    guysbuf[i].flags &= ~guy_flashing;
                }
            }
            
            if(get_bit(deprecated_rules, qr_NOBUBBLEFLASH_DEP))
            {
                if(i==eBUBBLEST || i==eBUBBLESP || i==eBUBBLESR || i==eBUBBLEIT || i==eBUBBLEIP || i==eBUBBLEIR)
                {
                    guysbuf[i].flags &= ~guy_flashing;
                }
            }
            
            if(i==eGHINI2)
            {
                if(get_bit(deprecated_rules, qr_GHINI2BLINK_DEP))
                {
                    guysbuf[i].flags |= guy_blinking;
                }
                
                if(get_bit(deprecated_rules, qr_PHANTOMGHINI2_DEP))
                {
                    guysbuf[i].flags |= guy_transparent;
                }
            }

			if (i == eDIG1 || i == eDIG3)
			{
				guysbuf[i].flags |= guy_ignore_kill_all;
			}
        }
        
        // Darknut fix
        if(i==eDKNUT1 || i==eDKNUT2 || i==eDKNUT3 || i==eDKNUT4 || i==eDKNUT5)
        {
            if(get_qr(qr_NEWENEMYTILES))
            {
                guysbuf[i].s_tile=guysbuf[i].e_tile+120;
                guysbuf[i].s_width=guysbuf[i].e_width;
                guysbuf[i].s_height=guysbuf[i].e_height;
            }
            else guysbuf[i].s_tile=860;
            
            if(!get_bit(deprecated_rules,qr_BRKBLSHLDS_DEP))
				guysbuf[i].flags &= ~guy_bkshield;
			else
				guysbuf[i].flags |= guy_bkshield;
        }
        
        if((i==eGELTRIB || i==eFGELTRIB) && get_bit(deprecated_rules,qr_OLDTRIBBLES_DEP))
        {
            guysbuf[i].attributes[2] = (i == eFGELTRIB ? eFZOL : eZOL);
        }

        guy_update_firesfx(guysbuf[i]);
		guy_update_weaponflags(guysbuf[i]);
		guy_update_weaponspecialsfx(guysbuf[i]);
    }
}

int32_t readguy_single(PACKFILE *f, word guyversion, word guy_cversion, zquestheader *Header, int32_t i, guydata& tempguy)
{
	byte tempbyte;
	word tempword;
	tempguy.clear();
	
	uint32_t flags1;
	uint32_t flags2;
	if(!p_igetl(&(flags1),f))
	{
		return qe_invalid;
	}
	if(!p_igetl(&(flags2),f))
	{
		return qe_invalid;
	}
	tempguy.flags = guy_flags(flags1) | guy_flags(uint64_t(flags2)<<32ULL);
		
	if ( guyversion >= 36 ) //expanded tiles
	{
		if(!p_igetl(&(tempguy.tile),f))
		{
		return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&(tempguy.tile),f))
		{
		return qe_invalid;
		}
	}    
		if(!p_getc(&(tempguy.width),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(tempguy.height),f))
		{
			return qe_invalid;
		}
		
	if ( guyversion >= 36 ) //expanded tiles
	{
		if(!p_igetl(&(tempguy.s_tile),f))
		{
		return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&(tempguy.s_tile),f))
		{
		return qe_invalid;
		}
	}
	
	if(!p_getc(&(tempguy.s_width),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(tempguy.s_height),f))
	{
		return qe_invalid;
	}
		
	if ( guyversion >= 36 ) //expanded tiles
	{
		if(!p_igetl(&(tempguy.e_tile),f))
		{
		return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&(tempguy.e_tile),f))
		{
		return qe_invalid;
		}
	}
	
	if(!p_getc(&(tempguy.e_width),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(tempguy.e_height),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.hp),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.type),f))
	{
		return qe_invalid;
	}
	
	if(guyversion < 9 && (i==eDKNUT1 || i==eDKNUT2 || i==eDKNUT3 || i==eDKNUT4 || i==eDKNUT5)) // Whoops, forgot about Darknuts...
	{
		if(get_qr(qr_NEWENEMYTILES))
		{
			tempguy.s_tile=tempguy.e_tile+120;
			tempguy.s_width=tempguy.e_width;
			tempguy.s_height=tempguy.e_height;
		}
		else tempguy.s_tile=860;
	}
	
	if(!p_igetw(&(tempguy.cset),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.anim),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.e_anim),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.frate),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.e_frate),f))
	{
		return qe_invalid;
	}
	
	if(guyversion < 13)  // April 2009
	{
		if(get_bit(deprecated_rules, qr_SLOWENEMYANIM_DEP))
		{
			tempguy.frate *= 2;
			tempguy.e_frate *= 2;
		}
	}
	
	if(guyversion < 14)  // May 1 2009
	{
		if(tempguy.anim==a2FRMSLOW)
		{
			tempguy.anim=a2FRM;
			tempguy.frate *= 2;
		}
		
		if(tempguy.e_anim==a2FRMSLOW)
		{
			tempguy.e_anim=a2FRM;
			tempguy.e_frate *= 2;
		}
		
		if(tempguy.anim==aFLIPSLOW)
		{
			tempguy.anim=aFLIP;
			tempguy.frate *= 2;
		}
		
		if(tempguy.e_anim==aFLIPSLOW)
		{
			tempguy.e_anim=aFLIP;
			tempguy.e_frate *= 2;
		}
		
		if(tempguy.anim == aNEWDWALK) tempguy.anim = a4FRM4DIR;
		
		if(tempguy.e_anim == aNEWDWALK) tempguy.e_anim = a4FRM4DIR;
		
		if(tempguy.anim == aNEWPOLV || tempguy.anim == a4FRM3TRAP)
		{
			tempguy.anim=a4FRM4DIR;
			tempguy.s_tile=(get_qr(qr_NEWENEMYTILES) ? tempguy.e_tile : tempguy.tile)+20;
		}
		
		if(tempguy.e_anim == aNEWPOLV || tempguy.e_anim == a4FRM3TRAP)
		{
			tempguy.e_anim=a4FRM4DIR;
			tempguy.s_tile=(get_qr(qr_NEWENEMYTILES) ? tempguy.e_tile : tempguy.tile)+20;
		}
	}
	
	if(!p_igetw(&(tempguy.dp),f))
	{
		return qe_invalid;
	}
	
	//correction for guy fire
	if(guyversion < 6)
	{
		if(i == gFIRE)
			tempguy.dp = 2;
	}
	
	if(!p_igetw(&(tempguy.wdp),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.weapon),f))
	{
		return qe_invalid;
	}
	
	//correction for bosses using triple, "rising" fireballs
	if(guyversion < 5)
	{
		if(i == eLAQUAM || i == eRAQUAM || i == eGOHMA1 || i == eGOHMA2 ||
				i == eGOHMA3 || i == eGOHMA4)
		{
			if(tempguy.weapon == ewFireball)
				tempguy.weapon = ewFireball2;
		}
	}
	
	if(!p_igetw(&(tempguy.rate),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.hrate),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.step),f))
	{
		return qe_invalid;
	}
	
	// HIGHLY UNORTHODOX UPDATING THING, part 2
	if(fixpolsvoice && tempguy.type==eePOLSV)
	{
		tempguy.step /= 2;
	}
	
	if(!p_igetw(&(tempguy.homing),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.grumble),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.item_set),f))
	{
		return qe_invalid;
	}
	
	if(guyversion>=22) // Version 22: Expand misc attributes to 32 bits
	{
		for (int q = 0; q < 10; ++q)
		{
			if (!p_igetl(&(tempguy.attributes[q]), f))
			{
				return qe_invalid;
			}
		}
	}
	else
	{
		int16_t tempMisc;
		
		for(int q=0;q<10;q++)
		{
			if (!p_igetw(&tempMisc, f))
			{
				return qe_invalid;
			}
			tempguy.attributes[q] = tempMisc;
		}
		
		if(guyversion < 13)  // April 2009 - a tiny Wizzrobe update
		{
			if(tempguy.type == eeWIZZ && !(tempguy.attributes[0]))
				tempguy.attributes[4] = 74;
		}

	}
	
	if(!p_igetw(&(tempguy.bgsfx),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.bosspal),f))
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(tempguy.extend),f))
	{
		return qe_invalid;
	}
	
//! Enemy Defences

//If a 2.50 quest, use only the 2.5 defences. 
	if(guyversion >= 16 )  // November 2009 - Super Enemy Editor
	{
		for(int32_t j=0; j<edefLAST; j++)
		{
			if(!p_getc(&(tempguy.defense[j]),f))
			{
				return qe_invalid;
			}
		}
//then copy the generic script defence to all the new script defences

	}



	
	if(guyversion >= 18)
	{
		if (guyversion < 55)
		{
			if(!p_getc(&tempbyte, f))
				return qe_invalid;
			tempguy.hitsfx = tempbyte;
			if(!p_getc(&tempbyte, f))
				return qe_invalid;
			tempguy.deadsfx = tempbyte;
		}
		else
		{
			if(!p_igetw(&(tempguy.hitsfx),f))
				return qe_invalid;
			if(!p_igetw(&(tempguy.deadsfx),f))
				return qe_invalid;
		}
	}
	
	if(guyversion >= 22)
	{
		if(!p_igetl(&(tempguy.attributes[10]), f))
		{
			return qe_invalid;
		}
		
		if(!p_igetl(&(tempguy.attributes[11]),f))
		{
			return qe_invalid;
		}
	}
	else if(guyversion >= 19)
	{
		int16_t tempMisc;
		
		if(!p_igetw(&tempMisc,f))
		{
			return qe_invalid;
		}
		
		tempguy.attributes[10] = tempMisc;
		
		if(!p_igetw(&tempMisc,f))
		{
			return qe_invalid;
		}
		
		tempguy.attributes[11] = tempMisc;
	}
	
	//If a 2.54 or later quest, use all of the defences. 
	if(guyversion > 24) // Add new guyversion conditional statement 
		{
	for(int32_t j=edefLAST; j<edefLAST255; j++)
			{
				if(!p_getc(&(tempguy.defense[j]),f))
				{
					return qe_invalid;
				}
			}
		}
	
	if(guyversion <= 24) // Port over generic script settings from old quests in the new editor. 
		{
	for(int32_t j=edefSCRIPT01; j<=edefSCRIPT10; j++)
			{
				tempguy.defense[j] = tempguy.defense[edefSCRIPT] ;
			}
		}
	
	//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
	if(guyversion > 25)
	{
		if(!p_igetl(&(tempguy.txsz),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.tysz),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.hxsz),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.hysz),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.hzsz),f))
				{
					return qe_invalid;
				}
		/* Is it safe to read a fixed with getl, or do I need to typecast it? -Z
		
		*/
	}
	//More Enemy Editor vars for 2.60
	if(guyversion > 26)
	{
		if(!p_igetl(&(tempguy.hxofs),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.hyofs),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.xofs),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.yofs),f))
				{
					return qe_invalid;
				}
		if(!p_igetl(&(tempguy.zofs),f))
				{
					return qe_invalid;
				}
	}
	
	if(guyversion <= 27) // Port over generic script settings from old quests in the new editor. 
		{
	tempguy.wpnsprite = 0;
		}
	
	if(guyversion > 27)
	{
		if(!p_igetl(&(tempguy.wpnsprite),f))
				{
					return qe_invalid;
				}
	}
	if(guyversion <= 28) // Port over generic script settings from old quests in the new editor. 
		{
	tempguy.SIZEflags = 0;
		}
	if(guyversion > 28)
	{
	if(!p_igetl(&(tempguy.SIZEflags),f))
		{
		return qe_invalid;
		}
	
	}
	if(guyversion < 30) // Port over generic script settings from old quests in the new editor. 
		{
	tempguy.frozentile = 0;
	tempguy.frozencset = 0;
	tempguy.frozenclock = 0;
	for ( int32_t q = 0; q < 10; q++ ) tempguy.frozenmisc[q] = 0;
		}
	if(guyversion >= 30)
	{
	if(!p_igetl(&(tempguy.frozentile),f))
	{
		return qe_invalid;
	}  
	if(!p_igetl(&(tempguy.frozencset),f))
	{
		return qe_invalid;
	}  
	if(!p_igetl(&(tempguy.frozenclock),f))
	{
		return qe_invalid;
	}  
	for ( int32_t q = 0; q < 10; q++ ) {
		if(!p_igetw(&(tempguy.frozenmisc[q]),f))
		{
			return qe_invalid;
		}
	}
	
	}
	
	if(guyversion >= 34)
	{
	if(!p_igetw(&(tempguy.firesfx),f))
	{
		return qe_invalid;
	}  
	for(int q=15;q<32;++q)
	{
		if(!p_igetl(&(tempguy.attributes[q]),f))
		{
			return qe_invalid;
		} 
	}
	
	for ( int32_t q = 0; q < 32; q++ ) {
		if(!p_igetl(&(tempguy.movement[q]),f))
		{
			return qe_invalid;
		}
	}
	for ( int32_t q = 0; q < 32; q++ ) {
		if(!p_igetl(&(tempguy.new_weapon[q]),f))
		{
			return qe_invalid;
		}
	}
	
	if (guyversion < 59)
	{
		if(!p_igetw(&(tempguy.scrconfig.script),f))
		{
			return qe_invalid;
		} 
		for ( int32_t q = 0; q < 8; q++ )
		{
			if(!p_igetl(&(tempguy.scrconfig.run_args[q]),f))
			{
				return qe_invalid;
			} 			
		}
		for ( int32_t q = 0; q < 2; q++ )
		{
			int32_t temp;
			if(!p_igetl(&temp,f))
			{
				return qe_invalid;
			} 			
		}
	}
	else
	{
		if (!p_getvar(&tempguy.scrconfig, f))
			return qe_invalid;
	}
	
	}
	
	if(guyversion >= 37)
	{
	if(!p_igetl(&(tempguy.editorflags),f))
	{
		return qe_invalid;
	}     
	}
	if ( guyversion < 37 ) { tempguy.editorflags = 0; }
	if(guyversion >= 38)
	{
	if(!p_igetl(&(tempguy.attributes[12]),f))
	{
		return qe_invalid;
	} 
	if(!p_igetl(&(tempguy.attributes[13]),f))
	{
		return qe_invalid;
	} 
	if(!p_igetl(&(tempguy.attributes[14]),f))
	{
		return qe_invalid;
	}  
		
	}
	if ( guyversion < 38 ) 
	{ 
	tempguy.attributes[12] = 0; 
	tempguy.attributes[13] = 0; 
	tempguy.attributes[14] = 0; 
	}
	
	if ( guyversion >= 39 )
	{
	for ( int32_t q = 0; q < 8; q++ )
	{
		byte dummybyte;
		if (guyversion < 59)
		{
			for (int32_t w = 0; w < 65; w++)
				if (!p_getc(&dummybyte,f))
					return qe_invalid;
		}
		if (guyversion < 54)
		{
			for (int32_t w = 0; w < 65; w++)
				if (!p_getc(&dummybyte,f))
					return qe_invalid;
		}
	}
		
		
	}
	if ( guyversion >= 40 && guyversion < 54)
		if(!p_igetw(&(tempguy.weap_data.scrconfig.script),f))
			return qe_invalid;
	if ( guyversion < 40 ) 
		tempguy.weap_data.scrconfig.clear();
	//eweapon script InitD
	if ( guyversion >= 41 )
	{
		if(guyversion < 54)
			for ( int32_t q = 0; q < 8; q++ )
				if(!p_igetl(&(tempguy.weap_data.scrconfig.run_args[q]),f))
					return qe_invalid;
		if ( guy_cversion < 4 )
		{
		if ( tempguy.type == eeKEESE )
		{

			if ( !tempguy.attributes[0] )
			{
				tempguy.attributes[15] = 120;
				tempguy.attributes[16] = 16;
				
			}
		}
		if ( tempguy.type == eePEAHAT )
		{	
			tempguy.attributes[15] = 80;
			tempguy.attributes[16] = 16;
		}

		if ( tempguy.type == eeGHINI )
		{	
			tempguy.attributes[15] = 120;
			tempguy.attributes[16] = 10;
		}			
			
		}
	}
	
	
	
	//default weapon sprites (quest version < 2.54)
	//port over old defaults -Z
	if(guyversion < 32)
	{
	if ( tempguy.wpnsprite <= 0 )
	{
		switch(tempguy.weapon)
		{
			case wNone:
				tempguy.wpnsprite = 0; break;
			
			case wSword:
			case wBeam:
			case wBrang:
			case wBomb:
			case wSBomb:
			case wLitBomb:
			case wLitSBomb:
			case wArrow:
			case wFire:
			case wWhistle:
			case wBait:
			case wWand:
			case wMagic:
			case wCatching:
			case wWind:
			case wRefMagic:
			case wRefFireball:
			case wRefRock:
			case wHammer:
			case wHookshot:
			case wHSHandle:
			case wHSChain:
			case wSSparkle:
			case wFSparkle:
			case wSmack:
			case wPhantom:
			case wCByrna:
			case wRefBeam:
			case wStomp:
			case lwMax:
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
			case wIce:
				//Cannot use any of these weapons yet. 
				tempguy.wpnsprite = -1;
				break;
			
			case wEnemyWeapons:
			case ewFireball: tempguy.wpnsprite = 17; break;
			
			case ewArrow: tempguy.wpnsprite = 19; break;
			case ewBrang: tempguy.wpnsprite = 4; break;
			case ewSword: tempguy.wpnsprite = 20; break;
			case ewRock: tempguy.wpnsprite = 18; break;
			case ewMagic: tempguy.wpnsprite = 21; break;
			case ewBomb: tempguy.wpnsprite = 78; break;
			case ewSBomb: tempguy.wpnsprite = 79; break;
			case ewLitBomb: tempguy.wpnsprite = 76; break;
			case ewLitSBomb: tempguy.wpnsprite = 77; break;
			case ewFireTrail: tempguy.wpnsprite = 80; break;
			case ewFlame: tempguy.wpnsprite = 35; break;
			case ewWind: tempguy.wpnsprite = 36; break;
			case ewFlame2: tempguy.wpnsprite = 81; break;
			case ewFlame2Trail: tempguy.wpnsprite = 82; break;
			case ewIce: tempguy.wpnsprite = 83; break;
			case ewFireball2: tempguy.wpnsprite = 17; break; //fireball (rising)
			
				
			default: break; //No assign.
		}
	}
	}
	
	//default weapon fire sound (quest version < 2.54)
	//port over old defaults and zero new data. -Z
	if(guyversion < 34)
	{
	for ( int32_t q = 0; q < 32; q++ )
	{
		tempguy.movement[q] = 0;
		tempguy.new_weapon[q] = 0;
		
	}
	
	//NPC Script attributes.
	tempguy.scrconfig.clear();
	
	for ( int32_t q = 15; q < 32; q++) tempguy.attributes[q] = 0; //misc 16-32

	//old default sounds
	if ( tempguy.firesfx <= 0 )
	{
		switch(tempguy.weapon)
		{
			case wNone:
				tempguy.firesfx = 0; break;
			
			case wSword:
			case wBeam:
			case wBrang:
			case wBomb:
			case wSBomb:
			case wLitBomb:
			case wLitSBomb:
			case wArrow:
			case wFire:
			case wWhistle:
			case wBait:
			case wWand:
			case wMagic:
			case wCatching:
			case wWind:
			case wRefMagic:
			case wRefFireball:
			case wRefRock:
			case wHammer:
			case wHookshot:
			case wHSHandle:
			case wHSChain:
			case wSSparkle:
			case wFSparkle:
			case wSmack:
			case wPhantom:
			case wCByrna:
			case wRefBeam:
			case wStomp:
			case lwMax:
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
			case wIce:
				//Cannot use any of these weapons yet. 
				tempguy.firesfx = -1;
				break;
			
			case wEnemyWeapons:
			case ewFireball: tempguy.firesfx = 40; break;
			
			case ewArrow: tempguy.firesfx = 1; break; //Ghost.zh has 0?
			case ewBrang: tempguy.firesfx = 4; break; //Ghost.zh has 0?
			case ewSword: tempguy.firesfx = 20; break; //Ghost.zh has 0?
			case ewRock: tempguy.firesfx = 51; break;
			case ewMagic: tempguy.firesfx = 32; break;
			case ewBomb: tempguy.firesfx = 3; break; //Ghost.zh has 0?
			case ewSBomb: tempguy.firesfx = 3; break; //Ghost.zh has 0?
			case ewLitBomb: tempguy.firesfx = 21; break; //Ghost.zh has 0?
			case ewLitSBomb: tempguy.firesfx = 21; break; //Ghost.zh has 0?
			case ewFireTrail: tempguy.firesfx = 13; break;
			case ewFlame: tempguy.firesfx = 13; break;
			case ewWind: tempguy.firesfx = 32; break;
			case ewFlame2: tempguy.firesfx = 13; break;
			case ewFlame2Trail: tempguy.firesfx = 13; break;
			case ewIce: tempguy.firesfx = 44; break;
			case ewFireball2: tempguy.firesfx = 40; break; //fireball (rising)
			
			//what about special attacks (e.g. summoning == 56)
			default: break; //No assign.
		}
	}
	}
	
	//Port hardcoded hit sound to the enemy hitsfx defaults for older quests. 
	if(Header->zelda_version <= 0x250 || ( Header->zelda_version > 0x250 && guyversion < 35 ))
	{
		if ( tempguy.hitsfx == 0 ) tempguy.hitsfx = 11;
	}
	//Keese and bat halt rates.
	if ( guyversion < 42 && guy_cversion < 4  ) 
	{
		
		if ( tempguy.type == eeKEESE )
		{

			if ( !tempguy.attributes[0] )
			{
				tempguy.attributes[15] = 120;
				tempguy.attributes[16] = 16;
				
			}
		}
		if ( tempguy.type == eePEAHAT )
		{	
			tempguy.attributes[15] = 80;
			tempguy.attributes[16] = 16;
		}
		if ( tempguy.type == eeGHINI )
		{	
			tempguy.attributes[15] = 120;
			tempguy.attributes[16] = 10;
		}			
			
		
	}
	
	
		//miscellaneous other corrections
		//fix the mirror wizzrobe -DD
		if(guyversion < 7)
		{
			if(i == eMWIZ)
			{
				tempguy.attributes[1] = 0;
				tempguy.attributes[3] = 1;
			}
		}
		
		if(guyversion < 8)
		{
			if(i == eGLEEOK1 || i == eGLEEOK2 || i == eGLEEOK3 || i == eGLEEOK4 || i == eGLEEOK1F || i == eGLEEOK2F || i == eGLEEOK3F || i == eGLEEOK4F)
			{
				// Some of these are deliberately different to NewDefault/defdata.cpp, by the way. -L
				tempguy.attributes[4] = 4; //neck length in segments
				tempguy.attributes[5] = 8; //neck offset from first body tile
				tempguy.attributes[6] = 40; //offset for each subsequent neck tile from the first neck tile
				tempguy.attributes[7] = 168; //head offset from first body tile
				tempguy.attributes[8] = 228; //flying head offset from first body tile
				
				if(i == eGLEEOK1F || i == eGLEEOK2F || i == eGLEEOK3F || i == eGLEEOK4F)
				{
					tempguy.attributes[5] += 10; //neck offset from first body tile
					tempguy.attributes[7] -= 12; //head offset from first body tile
				}
			}
		}
		
		if(guyversion < 10) // December 2007 - Dodongo CSet fix
		{
			if(get_bit(deprecated_rules,46) && tempguy.type==eeDONGO && tempguy.attributes[0]==0)
				tempguy.bosspal = spDIG;
		}
		
		if(guyversion < 11) // December 2007 - Spinning Tile fix
		{
			if(tempguy.type==eeSPINTILE)
			{
				tempguy.flags |= guy_superman;
				tempguy.item_set = 0; // Don't drop items
				tempguy.step = 300;
			}
		}
		
		if(guyversion < 12) // October 2008 - Flashing Bubble, Rope 2, and Ghini 2 fix
		{
			if(get_bit(deprecated_rules, qr_NOROPE2FLASH_DEP))
			{
				if(tempguy.type==eeROPE)
				{
					tempguy.flags &= ~guy_flashing;
				}
			}
			
			if(get_bit(deprecated_rules, qr_NOBUBBLEFLASH_DEP))
			{
				if(tempguy.type==eeBUBBLE)
				{
					tempguy.flags &= ~guy_flashing;
				}
			}
			
			if((tempguy.type==eeGHINI)&&(tempguy.attributes[0]))
			{
				if(get_bit(deprecated_rules, qr_GHINI2BLINK_DEP))
				{
					tempguy.flags |= guy_blinking;
				}
				
				if(get_bit(deprecated_rules, qr_PHANTOMGHINI2_DEP))
				{
					tempguy.flags |= guy_transparent;
				}
			}
		}
		
		if(guyversion < 15) // July 2009 - Guy Fire and Fairy fix
		{
			if(i==gFIRE)
			{
				tempguy.e_anim = aFLIP;
				tempguy.e_frate = 24;
			}
			
			if(i==gFAIRY)
			{
				tempguy.e_anim = a2FRM;
				tempguy.e_frate = 16;
			}
		}
		
		if(guyversion < 16)  // November 2009 - Super Enemy Editor part 1
		{
			if(i==0) Z_message("Updating guys to version 16...\n");
			
			update_guy_1(&tempguy);
			
			if(i==eMPOLSV)
			{
				tempguy.defense[edefARROW] = edCHINK;
				tempguy.defense[edefMAGIC] = ed1HKO;
				tempguy.defense[edefREFMAGIC] = ed1HKO;
			}
		}
		
		if(guyversion < 17)  // December 2009
		{
			if(tempguy.type==eePROJECTILE)
			{
				tempguy.attributes[0] = 0;
			}
		}
		
		if(guyversion < 18)  // January 2010
		{
			bool boss = (tempguy.type == eeAQUA || tempguy.type==eeDONGO || tempguy.type == eeMANHAN || tempguy.type == eeGHOMA || tempguy.type==eeDIG
							|| tempguy.type == eeGLEEOK || tempguy.type==eePATRA || tempguy.type == eeGANON || tempguy.type==eeMOLD);
							
			tempguy.hitsfx = (boss && tempguy.type != eeMOLD && tempguy.type != eeDONGO && tempguy.type != eeDIG) ? WAV_GASP : 0;
			tempguy.deadsfx = (boss && (tempguy.type != eeDIG || tempguy.attributes[9] == 0)) ? WAV_GASP : WAV_EDEAD;
			
			if(tempguy.type == eeAQUA)
				for(int32_t j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eRAQUAM].defense[j];
			else if(tempguy.type == eeMANHAN)
				for(int32_t j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eMANHAN].defense[j];
			else if(tempguy.type==eePATRA)
				for(int32_t j=0; j<edefLAST; j++) tempguy.defense[j] = default_guys[eGLEEOK1].defense[j];
			else if(tempguy.type==eeGHOMA)
			{
				for(int32_t j=0; j<edefLAST; j++)
					tempguy.defense[j] = default_guys[eGOHMA1].defense[j];
					
				tempguy.defense[edefARROW] = ((tempguy.attributes[0]==3) ? edCHINKL8 : (tempguy.attributes[0]==2) ? edCHINKL4 : 0);
				
				if(tempguy.attributes[0]==3 && !tempguy.weapon) tempguy.weapon = ewFlame;
				
				tempguy.attributes[0]--;
			}
			else if(tempguy.type == eeGLEEOK)
			{
				for(int32_t j=0; j<edefLAST; j++)
					tempguy.defense[j] = default_guys[eGLEEOK1].defense[j];
					
				if(tempguy.attributes[2]==1 && !tempguy.weapon) tempguy.weapon = ewFlame;
			}
			else if(tempguy.type == eeARMOS)
			{
				tempguy.type=eeWALK;
				tempguy.hrate = 0;
				tempguy.attributes[9] = tempguy.attributes[0];
				tempguy.attributes[0] = tempguy.attributes[1] = tempguy.attributes[2] = tempguy.attributes[3] = tempguy.attributes[4] = tempguy.attributes[5] = tempguy.attributes[6] = tempguy.attributes[7] = 0;
				tempguy.attributes[0] = tempguy.attributes[1] = tempguy.attributes[2] = tempguy.attributes[3] = tempguy.attributes[4] = tempguy.attributes[5] = tempguy.attributes[6] = tempguy.attributes[7] = 0;
				tempguy.attributes[8] = e9tARMOS;
			}
			else if(tempguy.type == eeGHINI && !tempguy.attributes[0])
			{
				tempguy.type=eeWALK;
				tempguy.hrate = 0;
				tempguy.attributes[0] = tempguy.attributes[1] = tempguy.attributes[2] = tempguy.attributes[3] = tempguy.attributes[4] = tempguy.attributes[5] =
													tempguy.attributes[6] = tempguy.attributes[7] = tempguy.attributes[8] = tempguy.attributes[9] = 0;
			}
			
			// Spawn animation flags
			if(tempguy.type == eeWALK && (tempguy.flags&guy_armos || tempguy.flags&guy_ghini))
				tempguy.flags |= guy_fade_flicker;
			else
				tempguy.flags &= (guy_flags)0x0F00000F; // Get rid of the unused flags!
		}
		
		if(guyversion < 20)  // April 2010
		{
			if(tempguy.type == eeTRAP)
			{
				tempguy.attributes[1] = tempguy.attributes[9];
				
				if(tempguy.attributes[9]>=1)
				{
					tempguy.attributes[0]++;
				}
				
				tempguy.attributes[9] = 0;
			}
			
			// Bomb Blast fix
			if(tempguy.weapon==ewBomb && tempguy.type!=eeROPE && (tempguy.type!=eeWALK || tempguy.attributes[1] != e2tBOMBCHU))
				tempguy.weapon = ewLitBomb;
			else if(tempguy.weapon==ewSBomb && tempguy.type!=eeROPE && (tempguy.type!=eeWALK || tempguy.attributes[1] != e2tBOMBCHU))
				tempguy.weapon = ewLitSBomb;
		}
		
		if(guyversion < 21)  // September 2011
		{
			if(tempguy.type == eeKEESE || tempguy.type == eeKEESETRIB)
			{
				if(tempguy.type == eeKEESETRIB)
				{
					tempguy.type = eeKEESE;
					tempguy.attributes[1] = e2tKEESETRIB;
					tempguy.attributes[0] = 0;
				}
				
				tempguy.rate = 2;
				tempguy.hrate = 8;
				tempguy.homing = 0;
				tempguy.step= (tempguy.type == eeKEESE && tempguy.attributes[0] ? 100:62);
			}
			else if(tempguy.type == eePEAHAT || tempguy.type==eePATRA)
			{
				if(tempguy.type == eePEAHAT)
				{
					tempguy.rate = 4;
					tempguy.step = 62;
				}
				else
					tempguy.step = 25;
					
				tempguy.hrate = 8;
				tempguy.homing = 0;
			}
			else if(tempguy.type == eeDIG || tempguy.type == eeMANHAN)
			{
				if(tempguy.type == eeMANHAN)
					tempguy.step=50;
					
				tempguy.hrate = 16;
				tempguy.homing = 0;
			}
			else if(tempguy.type == eeGLEEOK)
			{
				tempguy.rate = 2;
				tempguy.homing = 0;
				tempguy.hrate = 9;
				tempguy.step=89;
			}
			else if(tempguy.type == eeGHINI)
			{
				tempguy.rate = 4;
				tempguy.hrate = 12;
				tempguy.step=62;
				tempguy.homing = 0;
			}
			
			// Bigdig random rate fix
			if(tempguy.type==eeDIG && tempguy.attributes[9]==1)
			{
				tempguy.rate = 2;
			}
		}
		
		if(guyversion < 24) // November 2012
		{
			if(tempguy.type==eeLANM)
				tempguy.attributes[2] = 1;
			else if(tempguy.type==eeMOLD)
				tempguy.attributes[1] = 0;
		}
	
	if(guyversion < 33) //Whistle defence did not exist before this version of 2.54. -Z
	{
	if(tempguy.type!=eeDIG)
	{
		tempguy.defense[edefWhistle] = edIGNORE; //Might need to be ignore, universally. 
	}
		
	}
	// does not seem to solve the issue!
	if ( Header->zelda_version <= 0x210 ) 
	{
	al_trace("Detected version %d for dodongo patch.\n",Header->zelda_version);
	if ( tempguy.type == eeDONGO ) 
	{
		tempguy.deadsfx = 15; //In 2.10 and earlier, Dodongos used this as their death sound.
	}
	}
		
	if(guyversion >= 42)
	{
		if(guyversion >= 47)
		{
			if(!p_igetl(&(tempguy.moveflags),f))
			{
				return qe_invalid;
			}
		}
		else
		{
			byte fl;
			if(!p_getc(&fl,f))
			{
				return qe_invalid;
			}
			tempguy.moveflags = (move_flags)fl;
		}
	}
	else
	{
		switch(tempguy.type)
		{
			//No gravity; floats over pits
			case eeTEK: case eePEAHAT: case eeROCK: case eeTRAP:
			case eePROJECTILE: case eeSPINTILE: case eeKEESE: case eeFIRE:
			//Special (bosses, etc)
			case eeFAIRY: case eeGUY: case eeNONE: case eeZORA:
			case eeAQUA: case eeDIG: case eeGHOMA: case eeGANON:
			case eePATRA: case eeGLEEOK: case eeMOLD: case eeMANHAN:
				tempguy.moveflags = move_can_pitwalk;
				break;
			//No gravity, but falls in pits
			case eeLEV:
				tempguy.moveflags = move_can_pitfall;
				break;
			//Bosses that respect pits
			case eeDONGO:
				tempguy.moveflags = move_obeys_grav;
				break;
			case eeLANM:
				tempguy.moveflags = move_none;
				break;
			//Gravity, floats over pits
			case eeWIZZ: case eeWALLM: case eeGHINI:
				tempguy.moveflags = move_obeys_grav | move_can_pitwalk;
				break;
			//Gravity and falls in pits
			case eeWALK:
				if (tempguy.attributes[8]==e9tPOLSVOICE||tempguy.attributes[8]==e9tVIRE)
					break;
				[[fallthrough]];
			case eeOTHER:
			case eeSCRIPT01: case eeSCRIPT02: case eeSCRIPT03: case eeSCRIPT04: case eeSCRIPT05:
			case eeSCRIPT06: case eeSCRIPT07: case eeSCRIPT08: case eeSCRIPT09: case eeSCRIPT10:
			case eeSCRIPT11: case eeSCRIPT12: case eeSCRIPT13: case eeSCRIPT14: case eeSCRIPT15:
			case eeSCRIPT16: case eeSCRIPT17: case eeSCRIPT18: case eeSCRIPT19: case eeSCRIPT20:
			case eeFFRIENDLY01: case eeFFRIENDLY02: case eeFFRIENDLY03: case eeFFRIENDLY04: case eeFFRIENDLY05:
			case eeFFRIENDLY06: case eeFFRIENDLY07: case eeFFRIENDLY08: case eeFFRIENDLY09: case eeFFRIENDLY10:
				tempguy.moveflags = move_obeys_grav | move_can_pitfall;
		}
	}
	if(guyversion < 43)
	{
		switch(tempguy.type)
		{
			//No gravity; floats over pits
			case eeTEK: case eePEAHAT: case eeROCK: case eeTRAP:
			case eePROJECTILE: case eeSPINTILE: case eeKEESE: case eeFIRE:
			//Special (bosses, etc)
			case eeFAIRY: case eeGUY: case eeNONE: case eeZORA:
			case eeAQUA: case eeDIG: case eeGHOMA: case eeGANON:
			case eePATRA: case eeGLEEOK: case eeMOLD: case eeMANHAN:
			case eeWIZZ: case eeWALLM: case eeGHINI:
			//Gravity, floats over pits
				tempguy.moveflags |= move_can_waterwalk;
				tempguy.moveflags |= move_can_pitwalk;
				break;
		}
	}
	if (guyversion < 44)
	{
		if ( tempguy.type == eeGHOMA )
		{
			tempguy.flags |= guy_fade_instant;
		}
	}
	if (guyversion > 55)
	{
		if(!p_igetw(&(tempguy.spr_shadow),f))
			return qe_invalid;
		if(!p_igetw(&(tempguy.spr_death),f))
			return qe_invalid;
		if(!p_igetw(&(tempguy.spr_spawn),f))
			return qe_invalid;
	}
	else if (guyversion > 44)
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		tempguy.spr_shadow = tempbyte;
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		tempguy.spr_death = tempbyte;
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		tempguy.spr_spawn = tempbyte;
	}
	else
	{
		tempguy.spr_shadow = (tempguy.type==eeROCK && tempguy.attributes[9]==1) ? iwLargeShadow : iwShadow;
		tempguy.spr_death = iwDeath;
		tempguy.spr_spawn = iwSpawn;
	}
	
	if(guyversion < 46)
	{
		if(tempguy.type == eeWALK && tempguy.attributes[8] == e9tPOLSVOICE)
		{
			tempguy.moveflags |= move_can_waterwalk;
		}
	}

	if (guyversion < 47)
	{
		if (tempguy.type == eeDIG && tempguy.attributes[9]!=1)
		{
			tempguy.flags |= guy_ignore_kill_all;
		}
	}

	if (guyversion < 49)
	{
		if (tempguy.type == eeWALK && (tempguy.attributes[6]==e7tPERMJINX || tempguy.attributes[6]==e7tTEMPJINX || tempguy.attributes[6]==e7tUNJINX)) //BUBBLE CHECK
		{
			switch (tempguy.attributes[7]) {
				case 0: //Sword
					tempguy.attributes[7] = e8tSWORD;
					break;
				case 1:	//Item
					tempguy.attributes[7] = e8tITEM;
					break;
				case 2: //Both
					tempguy.attributes[7] = e8tSWORD|e8tITEM;
					break;
				default: //this can actually happen since Misc8 can be set to any number.
					tempguy.attributes[7] = 0;
					break;
			}
		}
	}

	//these could possible be combined but rather be safe...
	if (guyversion < 51) //reimport the firesfx, zoria ducked up.
	{
		guy_update_firesfx(tempguy);
	}
	if (guyversion < 52)
	{
		guy_update_weaponflags(tempguy);
	}
	else if(guyversion < 54)
	{
		if (!p_getc(&(tempguy.weap_data.unblockable), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.moveflags), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.override_flags), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.tilew), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.tileh), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.hxsz), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.hysz), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.hzsz), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.hxofs), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.hyofs), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.xofs), f))
			return qe_invalid;
		if (!p_igetl(&(tempguy.weap_data.yofs), f))
			return qe_invalid;
		int32_t temp_step;
		if (!p_igetl(&temp_step, f))
			return qe_invalid;
		tempguy.weap_data.step = zslongToFix(temp_step*100);
		for (int32_t q = 0; q < WPNSPR_MAX; ++q)
		{
			if (!p_igetw(&tempguy.weap_data.burnsprs[q], f))
				return qe_invalid;
			if (!p_igetw(&tempword, f))
				return qe_invalid;
			tempguy.weap_data.light_rads[q] = (byte)tempword;
		}
	}
	if (guyversion < 53)
	{
		guy_update_weaponspecialsfx(tempguy);
	}
	else
	{
		if (guyversion < 55)
		{
			if (!p_getc(&tempbyte, f))
				return qe_invalid;
			tempguy.specialsfx = tempbyte;
		}
		else
		{
			if (!p_igetw(&(tempguy.specialsfx), f))
				return qe_invalid;
		}
	}
	
	if(guyversion >= 54)
	{
		if(auto ret = read_weap_data(tempguy.weap_data, f))
			return ret;
	}
	else
	{
		SETFLAG(tempguy.weap_data.wflags, WFLAG_UPDATE_IGNITE_SPRITE, tempguy.flags & guy_burning_sprites);
		tempguy.weap_data.flags |= wdata_set_step;
		if(tempguy.weapon == ewRock)
			tempguy.weap_data.wflags |= WFLAG_BREAK_ON_SOLID;
	}

	if(guyversion >= 58)
	{
		if(!p_igetl(&(tempguy.viewport_suspend_range), f))
			return qe_invalid;
		if(!p_igetl(&(tempguy.viewport_despawn_range), f))
			return qe_invalid;
	}

	if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
	{
		tempguy.scrconfig.clear();
	}

	return 0;
}

int32_t readguys(PACKFILE *f, zquestheader *Header)
{
    bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_guys);
    if (should_skip) return 0;

    dword dummy;
    word guy_cversion;
    word guyversion=0;
    
    if(Header->zelda_version >= 0x193)
    {
        //section version info
        if(!p_igetw(&guyversion,f))
        {
            return qe_invalid;
        }

		if (guyversion > V_GUYS)
			return qe_version;
        
	FFCore.quest_format[vGuys] = guyversion;

		// Note: this is the only instance where "cversion" is ever used.
        if(!p_igetw(&guy_cversion,f))
        {
            return qe_invalid;
        }

        //section size
        if(!p_igetl(&dummy,f))
        {
            return qe_invalid;
        }
    }
    
    if(guyversion > 3)
    {
        for(int32_t i=0; i<MAXGUYS; i++)
        {
            char tempname[64];
            
            // rev. 1511 : guyversion = 23. upped to 512 editable enemies. -Gleeok
            // if guyversion < 23 then there is only 256 enemies in the packfile, so default the rest.
            if(guyversion < 23 && i >= OLDBETAMAXGUYS)
            {
                memset(tempname, 0, sizeof(char)*64);
                snprintf(tempname, sizeof(tempname), "e%03d", i);
                strcpy(guy_string[i], tempname);

                continue;
            }
            
            if(!pfread(tempname, 64, f))
            {
                return qe_invalid;
            }
            // pfread fills the whole buffer with raw file bytes; ensure it is
            // NUL-terminated before the strlen below to avoid an out-of-bounds read.
            tempname[63] = '\0';

            // Don't retain names of uneditable enemy entries!
			// for version upgrade to 2.5
			if(guyversion < 23 && i >= 177)
			{
				// some of the older builds have names such as 'zz123',
				// (this order gets messed up with some eXXX and some zzXXX)
				// so let's update to the newer naming convection. -Gleeok
				char tmpbuf[64];
				memset(tmpbuf, 0, sizeof(char)*64);
				snprintf(tmpbuf, sizeof(tmpbuf), "zz%03d", i);

				if(memcmp(tempname, tmpbuf, size_t(5)) == 0)
				{
					memset(tempname, 0, sizeof(char)*64);
					snprintf(tempname, sizeof(tempname), "e%03d", i);
				}
			}
			
			if(i >= OLDMAXGUYS || strlen(tempname)<1 || tempname[strlen(tempname)-1]!=' ')
			{
				guy_string[i][0] = '\0';
				strncat(guy_string[i], tempname, 64 - 1);
			}
			else
			{
				strcpy(guy_string[i],old_guy_string[i]);
			}
        }
    }
    else
    {
		for(int32_t i=0; i<eMAXGUYS; i++)
		{
			snprintf(guy_string[i], 64, "zz%03d",i);
		}
		
		for(int32_t i=0; i<OLDMAXGUYS; i++)
		{
			strcpy(guy_string[i],old_guy_string[i]);
		}
    }
    
    
    //finally...  section data
	init_guys(guyversion);                            //using default data for now...
	
	// Goriya guy fix
	if((Header->zelda_version < 0x211)||((Header->zelda_version == 0x211)&&(Header->build<7)))
	{
		if(get_qr(qr_NEWENEMYTILES))
		{
			guysbuf[gGORIYA].tile=130;
			guysbuf[gGORIYA].e_tile=130;
		}
	}
    
    if(Header->zelda_version < 0x193)
    {
        if(get_bit(deprecated_rules,46))
        {
            guysbuf[eDODONGO].cset=14;
            guysbuf[eDODONGO].bosspal=spDIG;
        }
    }
 // Not sure when this first changed, but it's necessary for 2.10, at least
    // @TODO: @BUG:1.92 - 1.84? Figure this out exactly for the final 2.50 release.
//2.10 Fixes  
    if((Header->zelda_version < 0x211)||((Header->zelda_version == 0x211)&&(Header->build<18)))
    {
	guysbuf[eWWIZ].editorflags |= ENEMY_FLAG5;
	guysbuf[eMOLDORM].editorflags |= ENEMY_FLAG6;
	guysbuf[eMANHAN].editorflags |= ENEMY_FLAG6;
	guysbuf[eCENT1].attributes[2] = 1;
	guysbuf[eCENT2].attributes[2] = 1;
    }
    if((Header->zelda_version <= 0x255) || (Header->zelda_version == 0x255 && Header->build < 47) )
    {
	guysbuf[eWPOLSV].defense[edefWhistle] = ed1HKO;
    }
    if(Header->zelda_version <= 0x210)
    {
        guysbuf[eGLEEOK1F].attributes[5] = 16;
        guysbuf[eGLEEOK2F].attributes[5] = 16;
        guysbuf[eGLEEOK3F].attributes[5] = 16;
        guysbuf[eGLEEOK4F].attributes[5] = 16;
	    
        guysbuf[eWIZ1].attributes[3] = 1; //only set the enemy that needs backward compat, not all of them.
        guysbuf[eBATROBE].attributes[3] = 1;
        //guysbuf[eSUMMONER].misc4 = 1;
        guysbuf[eWWIZ].attributes[3] = 1;
	    guysbuf[eDODONGO].deadsfx = 15; //In 2.10 and earlier, Dodongos used this as their death sound.
	guysbuf[eDODONGOBS].deadsfx = 15; //In 2.10 and earlier, Dodongos used this as their death sound.
    }
    if(Header->zelda_version == 0x190)
    {
	al_trace("Setting Tribble Properties for Version: %x", Header->zelda_version);
	guysbuf[eKEESETRIB].attributes[2] = eVIRE; //1.90 and earlier, keese and gel tribbles grew up into 
	guysbuf[eGELTRIB].attributes[2] = eZOL; //normal vires, and zols -Z (16th January, 2019 )
    }
    
    // The versions here may not be correct
    // zelda_version>=0x211 handled at guyversion<24
    if(Header->zelda_version <= 0x190)
    {
        guysbuf[eCENT1].attributes[2] = 0;
        guysbuf[eCENT2].attributes[2] = 0;
        guysbuf[eMOLDORM].attributes[1] = 0;
	//guysbuf[eKEESETRIB].misc3 = eVIRE; //1.90 and earlier, keese and gel tribbles grew up into 
	//guysbuf[eGELTRIB].misc3 = eZOL; //normal vires, and zols -Z (16th January, 2019 )
    }
    else if(Header->zelda_version <= 0x210)
    {
        guysbuf[eCENT1].attributes[2] = 1;
        guysbuf[eCENT2].attributes[2] = 1;
        guysbuf[eMOLDORM].attributes[1] = 0;
    }
    
    if ( Header->zelda_version < 0x211 ) //Default rest rates for phantom ghinis, peahats and keese in < 2.50 quests
    {
		guysbuf[eKEESE1].attributes[15] = 120;
		guysbuf[eKEESE2].attributes[15] = 120;
		guysbuf[eKEESE3].attributes[15] = 120;
		guysbuf[eKEESETRIB].attributes[15] = 120;
		guysbuf[eKEESE1].attributes[16] = 16;
		guysbuf[eKEESE2].attributes[16] = 16;
		guysbuf[eKEESE3].attributes[16] = 16;
		guysbuf[eKEESETRIB].attributes[16] = 16;
			
		guysbuf[ePEAHAT].attributes[15] = 80;
		guysbuf[ePEAHAT].attributes[16] = 16;
			
		guysbuf[eGHINI2].attributes[15] = 120;
		guysbuf[eGHINI2].attributes[16] = 10;
		
		if (replay_version_check(20))
		{
			guysbuf[eGHINI2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eMOLDORM].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eKEESETRIB].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eKEESE3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eKEESE2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eKEESE1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTEK1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTEK2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePEAHAT].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eROCK].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRAP].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eWALLM].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePOLSV].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eMANHAN].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK4].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIG1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIG3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIGPUP1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIGPUP2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIGPUP3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eDIGPUP4].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eRAQUAM].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eITEMFAIRY].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eFIRE].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eMANHAN2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRAP_H].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRAP_V].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRAP_LR].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRAP_UD].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePATRA1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePATRA2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePATRABS].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eBAT].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK1F].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK2F].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK3F].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGLEEOK4F].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eTRIGGER].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePATRAL2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[ePATRAL3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGOHMA1].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGOHMA2].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGOHMA3].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eGOHMA4].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eMPOLSV].moveflags |= (move_can_waterwalk|move_can_pitwalk);
			guysbuf[eWPOLSV].moveflags |= (move_can_waterwalk|move_can_pitwalk);
		}
    }
	
    
    if(guyversion<=2)
    {
        return readherosprites2(f, guyversion==2?0:-1);
    }
    
    if(guyversion > 3)
    {
        for(int32_t i=0; i<MAXGUYS; i++)
        {
            if(guyversion < 23 && i >= OLDBETAMAXGUYS)
            {
                guysbuf[i].clear();
                continue;
            }

            guydata tempguy;
            if(auto ret = readguy_single(f, guyversion, guy_cversion, Header, i, tempguy))
                return ret;
            guysbuf[i] = tempguy;
        }
    }
    
    return 0;
}
