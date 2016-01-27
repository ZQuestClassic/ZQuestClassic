//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  guys.cc
//
//  "Guys" code (and other related stuff) for zelda.cc
//
//--------------------------------------------------------

// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "precompiled.h" //always first

#include <string.h>
#include <stdio.h>
#include "zc_alleg.h"
#include "guys.h"
#include "zelda.h"
#include "zc_sys.h"
#include "zsys.h"
#include "maps.h"
#include "link.h"
#include "subscr.h"
#include "ffscript.h"
#include "gamedata.h"
#include "defdata.h"
#include "mem_debug.h"
#include "zscriptversion.h"
#include "messageManager.h"
#include "room.h"
#include "sound.h"

#include "sfxManager.h"
#include "angelscript/aszc.h"
#include "angelscript/scriptData.h"

extern LinkClass   Link;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern zinitdata    zinit;
extern MessageManager messageMgr;
extern SFXManager sfxMgr;

int wallm_load_clk=0;
int sle_x,sle_y,sle_cnt,sle_clk;
int vhead=0;
int guycarryingitem=0;

char *guy_string[eMAXGUYS];

BITMAP* scriptDrawingTarget;
static bool scriptShadowTrans;

void never_return(int index);
void playLevelMusic();

// If an enemy is this far out of the playing field, just remove it.
#define OUTOFBOUNDS ((int)y>((tmpscr->flags7&fSIDEVIEW && canfall(id))?192:352) || y<-176 || x<-256 || x > 512)

namespace
{
    int trapConstantHorizontalID;
    int trapConstantVerticalID;
    int trapLOSHorizontalID;
    int trapLOSVerticalID;
    int trapLOS4WayID;
    
    int cornerTrapID;
    int centerTrapID;
    
    int rockID;
    int zoraID;
    int statueID;
}

void identifyCFEnemies()
{
    trapConstantHorizontalID=-1;
    trapConstantVerticalID=-1;
    trapLOSHorizontalID=-1;
    trapLOSVerticalID=-1;
    trapLOS4WayID=-1;
    cornerTrapID=-1;
    centerTrapID=-1;
    rockID=-1;
    zoraID=-1;
    statueID=-1;
    
    for(int i=0; i<eMAXGUYS; i++)
    {
        if((guysbuf[i].flags2&cmbflag_trph) && trapLOSHorizontalID==-1)
            trapLOSHorizontalID=i;
        if((guysbuf[i].flags2&cmbflag_trpv) && trapLOSVerticalID==-1)
            trapLOSVerticalID=i;
        if((guysbuf[i].flags2&cmbflag_trp4) && trapLOS4WayID==-1)
            trapLOS4WayID=i;
        if((guysbuf[i].flags2&cmbflag_trplr) && trapConstantHorizontalID==-1)
            trapConstantHorizontalID=i;
        if((guysbuf[i].flags2&cmbflag_trpud) && trapConstantVerticalID==-1)
            trapConstantVerticalID=i;
        
        if((guysbuf[i].flags2&eneflag_trap) && cornerTrapID==-1)
            cornerTrapID=i;
        if((guysbuf[i].flags2&eneflag_trp2) && centerTrapID==-1)
            centerTrapID=i;
        
        if((guysbuf[i].flags2&eneflag_rock) && rockID==-1)
            rockID=i;
        if((guysbuf[i].flags2&eneflag_zora) && zoraID==-1)
            zoraID=i;
        
        if((guysbuf[i].flags2 & eneflag_fire) && statueID==-1)
            statueID=i;
    }
}

bool tooclose(int x,int y,int d)
{
    return (abs(int(LinkX())-x)<d && abs(int(LinkY())-y)<d);
}

// Returns true iff a combo type or flag precludes enemy movement.
bool groundblocked(int dx, int dy)
{
    return COMBOTYPE(dx,dy)==cPIT || COMBOTYPE(dx,dy)==cPITB || COMBOTYPE(dx,dy)==cPITC ||
           COMBOTYPE(dx,dy)==cPITD || COMBOTYPE(dx,dy)==cPITR ||
           // Block enemies type and block enemies flags
           combo_class_buf[COMBOTYPE(dx,dy)].block_enemies&1 ||
           MAPFLAG(dx,dy)==mfNOENEMY || MAPCOMBOFLAG(dx,dy)==mfNOENEMY ||
           MAPFLAG(dx,dy)==mfNOGROUNDENEMY || MAPCOMBOFLAG(dx,dy)==mfNOGROUNDENEMY ||
           // Check for ladder-only combos which aren't dried water
           (combo_class_buf[COMBOTYPE(dx,dy)].ladder_pass&1 && !iswater_type(COMBOTYPE(dx,dy))) ||
           // Check for drownable water
           (get_bit(quest_rules,qr_DROWN) && !(tmpscr->flags7&fSIDEVIEW) && (iswater(MAPCOMBO(dx,dy))));
}

// Returns true iff enemy is floating and blocked by a combo type or flag.
bool flyerblocked(int dx, int dy, int special)
{
    return ((special==spw_floater)&&
            ((COMBOTYPE(dx,dy)==cNOFLYZONE)||
             (combo_class_buf[COMBOTYPE(dx,dy)].block_enemies&4)||
             (MAPFLAG(dx,dy)==mfNOENEMY)||
             (MAPCOMBOFLAG(dx,dy)==mfNOENEMY)));
}

bool m_walkflag(int dx,int dy,int special, int x, int y)
{
    int yg = (special==spw_floater)?8:0;
    int nb = get_bit(quest_rules, qr_NOBORDER) ? 16 : 0;
    
    if(dx<16-nb || dy<zc_max(16-yg-nb,0) || dx>=240+nb || dy>=160+nb)
        return true;
        
    if(isdungeon() || special==spw_wizzrobe)
    {
        if((x>=32 && dy<32-yg) || (y>-1000 && y<=144 && dy>=144))
            return true;
            
        if((x>=32 && dx<32) || (x>-1000 && x<224 && dx>=224))
            if(special!=spw_door)                                 // walk in door way
                return true;
    }
    
    switch(special)
    {
    case spw_clipbottomright:
        if(dy>=128 || dx>=208) return true;
        
    case spw_clipright:
        break; //if(x>=208) return true; break;
        
    case spw_wizzrobe:
    case spw_floater:
        return false;
    }
    
    dx&=(special==spw_halfstep)?(~7):(~15);
    dy&=(special==spw_halfstep || tmpscr->flags7&fSIDEVIEW)?(~7):(~15);
    
    if(special==spw_water)
        return (water_walkflag(dx,dy+8,1) || water_walkflag(dx+8,dy+8,1));
        
    return _walkflag(dx,dy+8,1) || _walkflag(dx+8,dy+8,1) ||
           groundblocked(dx,dy+8) || groundblocked(dx+8,dy+8);
}


/**********************************/
/*******  Enemy Base Class  *******/
/**********************************/

/* ROM data flags

  */

enemy::enemy(fix X,fix Y,int Id,int Clk):
    sprite()
{
    d = guysbuf + (Id & 0xFFF);
    
    bgSFX=sfxMgr.getSFX(d->bgsfx);
    hitSFX=sfxMgr.getSFX(d->hitsfx);
    deathSFX=sfxMgr.getSFX(d->deadsfx);
    
    x=X;
    y=Y;
    id=Id;
    clk=Clk;
    floor_y=y;
    ceiling=false;
    fading = misc = clk2 = stunclk = hclk = sclk = superman = 0;
    grumble = 0;
    yofs = playing_field_offset - ((tmpscr->flags7&fSIDEVIEW) ? 0 : 2);
    did_armos=true;
    script_spawned=false;
    
    hp = d->hp;
//  cs = d->cset;
//d variables

    flags=d->flags;
    flags2=d->flags2;
    s_tile=d->s_tile; //secondary (additional) tile(s)
    family=d->family;
    dcset=d->cset;
    cs=dcset;
    anim=get_bit(quest_rules,qr_NEWENEMYTILES)?d->e_anim:d->anim;
    dp=d->dp;
    wdp=d->wdp;
    wpn=d->weapon;
    
    rate=d->rate;
    hrate=d->hrate;
    dstep=d->step;
    homing=d->homing;
    dmisc1=d->misc1;
    dmisc2=d->misc2;
    dmisc3=d->misc3;
    dmisc4=d->misc4;
    dmisc5=d->misc5;
    dmisc6=d->misc6;
    dmisc7=d->misc7;
    dmisc8=d->misc8;
    dmisc9=d->misc9;
    dmisc10=d->misc10;
    dmisc11=d->misc11;
    dmisc12=d->misc12;
    dmisc13=d->misc13;
    dmisc14=d->misc14;
    dmisc15=d->misc15;
    
    for(int i=0; i<edefLAST; i++)
        defense[i]=d->defense[i];
    
    bgsfx=d->bgsfx;
    bosspal=d->bosspal;
    
    if(bosspal>-1)
    {
        loadpalset(csBOSS,pSprite(bosspal));
    }
    
    bgSFX.startLooping();
    
    if(get_bit(quest_rules,qr_NEWENEMYTILES))
    {
        o_tile=d->e_tile;
        frate = d->e_frate;
    }
    else
    {
        o_tile=d->tile;
        frate = d->frate;
    }
    
    tile=0;
    
    // If they forgot the invisibility flag, here's another failsafe:
    if(o_tile==0 && family!=eeSPINTILE)
        flags |= guy_invisible;
        
//  step = d->step/100.0;
    // To preserve the odd step values for Keese & Gleeok heads. -L
    if(dstep==62.0) dstep+=fix(0.5);
    else if(dstep==89) dstep-=fix(1)/fix(9);
    
    step = dstep/100.0;
    
    
    item_set = d->item_set;
    grumble = d->grumble;
    
    if(frate == 0)
        frate = 256;
        
    leader = itemguy = dying = scored = false;
    canfreeze = count_enemy = true;
    mainguy = !(flags & guy_doesntcount);
    dir = rand()&3;
    
    if((wpn==ewBomb || wpn==ewSBomb) && family!=eeOTHER && family!=eeFIRE && (family!=eeWALK || dmisc2 != e2tBOMBCHU))
        wpn = 0;
    
    dummy_int1=0;
}

enemy::~enemy() {}

// Will this only be used by scripts?
void enemy::setBGSFX(SFX newBG)
{
    bgSFX=newBG;
    bgSFX.startLooping();
}

void enemy::setAttack(EnemyAttack* att)
{
    attack.reset(att);
    att->setOwner(this);
}

void enemy::leave_item()
{
    int drop_item = select_dropitem(item_set, x, y);
    
    if(drop_item!=-1&&((itemsbuf[drop_item].family!=itype_fairy)||!m_walkflag(x,y,0)))
    {
        if(extend >= 3) items.add(new item(x+(txsz-1)*8,y+(tysz-1)*8,(fix)0,drop_item,ipBIGRANGE+ipTIMER,0));
        else items.add(new item(x,y,(fix)0,drop_item,ipBIGRANGE+ipTIMER,0));
    }
}

// auomatically kill off enemy (for rooms with ringleaders)
void enemy::kickbucket()
{
    if(!superman)
        hp=-1000;                                               // don't call death_sfx()
}

// Hit the shield(s)?
// Apparently, this function is only used for hookshots...
bool enemy::hitshield(int wpnx, int wpny, int xdir)
{
    if(!(family==eeWALK || family==eeFIRE || family==eeOTHER))
        return false;
        
    bool ret = false;
    
    // TODO: There must be some bitwise operations that can simplify this...
    if(wpny > y) ret = ((flags&inv_front && xdir==down) || (flags&inv_back && xdir==up) || (flags&inv_left && xdir==left) || (flags&inv_right && xdir==right));
    else if(wpny < y) ret = ((flags&inv_front && xdir==up) || (flags&inv_back && xdir==down) || (flags&inv_left && xdir==right) || (flags&inv_right && xdir==left));
    
    if(wpnx < x) ret = ret || ((flags&inv_front && xdir==left) || (flags&inv_back && xdir==right) || (flags&inv_left && xdir==up) || (flags&inv_right && xdir==down));
    else if(wpnx > x) ret = ret || ((flags&inv_front && xdir==right) || (flags&inv_back && xdir==left) || (flags&inv_left && xdir==down) || (flags&inv_right && xdir==up));
    
    return ret;
}

// Check defenses without actually acting on them.
bool enemy::candamage(int power, int edef)
{
    switch(defense[edef])
    {
    case edSTUNORCHINK:
    case edSTUNORIGNORE:
    case edSTUNONLY:
    case edCHINK:
    case edIGNORE:
        return false;
        
    case edIGNOREL1:
    case edCHINKL1:
        return power >= 1*DAMAGE_MULTIPLIER;
        
    case edCHINKL2:
        return power >= 2*DAMAGE_MULTIPLIER;
        
    case edCHINKL4:
        return power >= 4*DAMAGE_MULTIPLIER;
        
    case edCHINKL6:
        return power >= 6*DAMAGE_MULTIPLIER;
        
    case edCHINKL8:
        return power >= 8*DAMAGE_MULTIPLIER;
    }
    
    return true;
}

// Do we do damage?
// 0: takehit returns 0
// 1: takehit returns 1
// -1: do damage
int enemy::defend(int wpnId, int *power, int edef)
{
    if(shieldCanBlock)
    {
        switch(defense[edef])
        {
        case edIGNORE:
            return 0;
        case edIGNOREL1:
        case edSTUNORIGNORE:
            if(*power <= 0)
                return 0;
        }
        
        sfx(WAV_CHINK,pan(int(x)));
        return 1;
    }
    
    switch(defense[edef])
    {
    case edSTUNORCHINK:
        if(*power <= 0)
        {
            sfx(WAV_CHINK,pan(int(x)));
            return 1;
        }
        
    case edSTUNORIGNORE:
        if(*power <= 0)
            return 0;
            
    case edSTUNONLY:
        if((wpnId==wFire || wpnId==wBomb || wpnId==wSBomb || wpnId==wHookshot || wpnId==wSword) && stunclk>=159)
            return 1;
            
        stunclk=160;
        sfx(WAV_EHIT,pan(int(x)));
        return 1;
        
    case edCHINKL1:
        if(*power >= 1*DAMAGE_MULTIPLIER) break;
        
    case edCHINKL2:
        if(*power >= 2*DAMAGE_MULTIPLIER) break;
        
    case edCHINKL4:
        if(*power >= 4*DAMAGE_MULTIPLIER) break;
        
    case edCHINKL6:
        if(*power >= 6*DAMAGE_MULTIPLIER) break;
        
    case edCHINKL8:
        if(*power >= 8*DAMAGE_MULTIPLIER) break;
        
    case edCHINK:
        sfx(WAV_CHINK,pan(int(x)));
        return 1;
        
    case edIGNOREL1:
        if(*power > 0)  break;
        
    case edIGNORE:
        return 0;
        
    case ed1HKO:
        *power = hp;
        return -2;
        
    case edQUARTDAMAGE:
        *power = zc_max(1,*power/2);
        
        //fallthrough
    case edHALFDAMAGE:
        *power = zc_max(1,*power/2);
        break;
    }
    
    return -1;
}

// Defend against a particular item class.
int enemy::defenditemclass(int wpnId, int *power)
{
    int def=-1;
    
    switch(wpnId)
    {
        // These first 2 are only used by Gohma... enemy::takehit() has complicated stun-calculation code for these.
    case wBrang:
        def = defend(wpnId, power, edefBRANG);
        break;
        
    case wHookshot:
        def = defend(wpnId, power, edefHOOKSHOT);
        break;
        
        // Anyway...
    case wBomb:
        def = defend(wpnId, power, edefBOMB);
        break;
        
    case wSBomb:
        def = defend(wpnId, power, edefSBOMB);
        break;
        
    case wArrow:
        def = defend(wpnId, power, edefARROW);
        break;
        
    case wFire:
        def = defend(wpnId, power, edefFIRE);
        break;
        
    case wWand:
        def = defend(wpnId, power, edefWAND);
        break;
        
    case wMagic:
        def = defend(wpnId, power, edefMAGIC);
        break;
        
    case wHammer:
        def = defend(wpnId, power, edefHAMMER);
        break;
        
    case wSword:
        def = defend(wpnId, power, edefSWORD);
        break;
        
    case wBeam:
        def = defend(wpnId, power, edefBEAM);
        break;
        
    case wRefBeam:
        def = defend(wpnId, power, edefREFBEAM);
        break;
        
    case wRefMagic:
        def = defend(wpnId, power, edefREFMAGIC);
        break;
        
    case wRefFireball:
        def = defend(wpnId, power, edefREFBALL);
        break;
        
    case wRefRock:
        def = defend(wpnId, power, edefREFROCK);
        break;
        
    case wStomp:
        def = defend(wpnId, power, edefSTOMP);
        break;
        
    case wCByrna:
        def = defend(wpnId, power, edefBYRNA);
        break;
        
    default:
        if(wpnId>=wScript1 && wpnId<=wScript10)
        {
            def = defend(wpnId, power, edefSCRIPT);
        }
        
        break;
    }
    
    return def;
}

// take damage or ignore it
// -1: damage (if any) dealt
// 1: blocked
// 0: weapon passes through unhindered
int enemy::takehit(weapon *w)
{
    int wpnId = w->id;
    int power = w->power;
    int wpnx = w->x;
    int wpny = w->y;
    int enemyHitWeapon = w->parentitem;
    int wpnDir;
    
    // If it's a boomerang that just bounced, use the opposite direction;
    // otherwise, it might bypass a shield. This probably won't handle
    // every case correctly, but it's better than having shields simply
    // not work against boomerangs.
    if(w->id==wBrang && w->misc==1 && w->clk2>=256 && w->clk2<264)
        wpnDir = oppositeDir[w->dir];
    else
        wpnDir = w->dir;
        
    if(dying || clk<0 || hclk>0 || superman)
    {
        return 0;
    }
    
    int ret = -1;
    
    // This obscure quest rule...
    if(get_bit(quest_rules,qr_BOMBDARKNUTFIX) && (wpnId==wBomb || wpnId==wSBomb))
    {
        double ddir=atan2(double(wpny-y),double(x-wpnx));
        wpnDir=rand()&3;
        
        if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
        {
            wpnDir=down;
        }
        else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
        {
            wpnDir=right;
        }
        else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/4)))
        {
            wpnDir=up;
        }
        else
        {
            wpnDir=left;
        }
    }
    
    int xdir = dir;
    shieldCanBlock=false;
    
    //if (family==eeFLOAT && flags&(inv_front|inv_back_inv_left|inv_right)) xdir=down;
    if((wpnId==wHookshot && hitshield(wpnx, wpny, xdir))
            || ((flags&inv_front && wpnDir==(xdir^down)) || (flags&inv_back && wpnDir==(xdir^up)) || (flags&inv_left && wpnDir==(xdir^left)) || (flags&inv_right && wpnDir==(xdir^right)))
      )
        // The hammer should already be dealt with by subclasses (Walker etc.)
    {
        switch(wpnId)
        {
            // Weapons which shields protect against
        case wSword:
        case wWand:
            Link.onMeleeWeaponHit();
                
            //fallthrough
        case wHookshot:
        case wHSHandle:
        case wBrang:
            shieldCanBlock=true;
            break;
            
        case wBeam:
        case wRefBeam:
        case wRefRock:
        case wRefFireball:
        case wMagic:
            if(wpnId>wEnemyWeapons)
                return 0;
                
            //fallthrough
        default:
            shieldCanBlock=true;
            break;
            
            // Bombs
        case wSBomb:
        case wBomb:
            goto hitclock;
            
            // Weapons which ignore shields
        case wWhistle:
        case wHammer:
            break;
            
            // Weapons which shouldn't be removed by shields
        case wLitBomb:
        case wLitSBomb:
        case wWind:
        case wPhantom:
        case wSSparkle:
        case wBait:
            return 0;
            
        case wFire:
            ;
        }
    }
    
    switch(wpnId)
    {
    case wWhistle:
        return 0;
        
    case wPhantom:
        return 0;
        
    case wLitBomb:
    case wLitSBomb:
    case wBait:
    case wWind:
    case wSSparkle:
        return 0;
        
    case wFSparkle:
    
        // Only take sparkle damage if the sparkle's parent item is not
        // defended against.
        if(enemyHitWeapon > -1)
        {
            int p = 0;
            int f = itemsbuf[enemyHitWeapon].family;
            
            switch(f)
            {
            case itype_arrow:
                if(!candamage(p, edefARROW)) return 0;
                
                break;
                
            case itype_cbyrna:
                if(!candamage(p, edefBYRNA)) return 0;
                
                break;
                
            case itype_brang:
                if(!candamage(p, edefBRANG)) return 0;
                
                break;
                
            default:
                return 0;
            }
        }
        
        wpnId = wSword;
        power = DAMAGE_MULTIPLIER>>1;
        goto fsparkle;
        break;
        
    case wBrang:
    {
        int def = defend(wpnId, &power, edefBRANG);
        
        if(def >= 0) return def;
        
        // Not hurt by 0-damage weapons
        if(!(flags & guy_bhit))
        {
            stunclk=160;
            
            if(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_brang))
            {
                hp -= (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_brang))*DAMAGE_MULTIPLIER;
                goto hitclock;
            }
            
            break;
        }
        
        if(!power)
            hp-=(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].fam_type : current_item(itype_brang))*DAMAGE_MULTIPLIER;
        else
            hp-=power;
            
        goto hitclock;
    }
    
    case wHookshot:
    {
        int def = defend(wpnId, &power, edefHOOKSHOT);
        
        if(def >= 0) return def;
        
        if(!(flags & guy_bhit))
        {
            stunclk=160;
            
            if(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot))
            {
                hp -= (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot))*DAMAGE_MULTIPLIER;
                goto hitclock;
            }
            
            break;
        }
        
        if(!power) hp-=(enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].fam_type : current_item(itype_hookshot))*DAMAGE_MULTIPLIER;
        else
            hp-=power;
            
        goto hitclock;
    }
    break;
    
    case wHSHandle:
    {
        if(itemsbuf[enemyHitWeapon>-1 ? enemyHitWeapon : current_item_id(itype_hookshot)].flags & ITEM_FLAG1)
            return 0;
            
        bool ignorehookshot = ((defense[edefHOOKSHOT] == edIGNORE) || ((defense[edefHOOKSHOT] == edIGNOREL1 || defense[edefHOOKSHOT] == edSTUNORIGNORE)
                               && (enemyHitWeapon>-1 ? itemsbuf[enemyHitWeapon].power : current_item_power(itype_hookshot)) <= 0));
                               
        // Peahats, Darknuts, Aquamentuses, Pols Voices, Wizzrobes, Manhandlas
        if(!(family==eePEAHAT || family==eeAQUA || family==eeMANHAN || (family==eeWIZZ && !ignorehookshot)
                || (family==eeWALK && dmisc9==e9tPOLSVOICE) || (family==eeWALK && flags&(inv_back|inv_front|inv_left|inv_right))))
            return 0;
            
        power = DAMAGE_MULTIPLIER;
        //fallthrough
    }
    
fsparkle:

    default:
        // Work out the defenses!
    {
        int def = defenditemclass(wpnId, &power);
        
        if(def >= 0)
            return def;
        else if(def == -2)
        {
            ret = 0;
        }
    }
    
    if(!power)
    {
        if(flags & guy_bhit)
            hp-=1;
        else
        {
            // Don't make a long chain of 'stun' hits
            if((wpnId==wFire || wpnId==wBomb || wpnId==wSBomb || wpnId==wSword) && stunclk>0)
                return 1;
                
            stunclk=160;
            break;
        }
    }
    else hp-=power;
    
hitclock:
    hclk=33;
    
    // Use w->dir instead of wpnDir to make sure boomerangs don't push enemies the wrong way
    if((dir&2)==(w->dir&2))
    {
        sclk=(w->dir<<8)+16;
    }
    }
    
    if(((wpnId==wBrang) || (get_bit(quest_rules,qr_NOFLASHDEATH))) && hp<=0)
    {
        fading=fade_blue_poof;
    }
    
    sfx(WAV_EHIT, pan(int(x)));
    
    hitSFX.play(x);
        
    if(family==eeGUY)
        sfx(WAV_EDEAD, pan(int(x)));
        
    // Penetrating weapons
    if((wpnId==wArrow || wpnId==wBeam) && !cannotpenetrate())
    {
        int item=enemyHitWeapon;
        
        if(wpnId==wArrow)
        {
            if(item<0)
                item=current_item_id(itype_arrow);
                
            if(item>=0 && (itemsbuf[item].flags&ITEM_FLAG1))
                return 0;
        }
        
        else
        {
            if(item<0)
                item=current_item_id(itype_sword);
                
            if(item>=0 && (itemsbuf[item].flags&ITEM_FLAG3))
                return 0;
        }
    }
    
    return ret;
}

bool enemy::dont_draw()
{
    if(fading==fade_invisible || (((flags2&guy_blinking)||(fading==fade_flicker)) && (clk&1)))
        return true;
        
    if(flags&guy_invisible)
        return true;
        
    if(flags&lens_only && !lensclk)
        return true;
        
    return false;
}

// base drawing function to be used by all derived classes instead of
// sprite::draw()
void enemy::draw(BITMAP *dest)
{
    if(dont_draw())
        return;
        
    int cshold=cs;
    
    if(dying)
    {
        if(clk2>=19)
        {
            if(!(clk2&2))
                sprite::draw(dest);
                
            return;
        }
        
        flip = 0;
        tile = wpnsbuf[iwDeath].tile;
        
        if(BSZ)
            tile += zc_min((15-clk2)/3,4);
        else if(clk2>6 && clk2<=12)
            ++tile;
            
        /* trying to get more death frames here
          if(wpnsbuf[wid].frames)
          {
          if(++clk2 >= wpnsbuf[wid].speed)
          {
          clk2 = 0;
          if(++aframe >= wpnsbuf[wid].frames)
          aframe = 0;
          }
          tile = wpnsbuf[wid].tile + aframe;
          }
          */
        
        if(BSZ || fading==fade_blue_poof)
            cs = wpnsbuf[iwDeath].csets&15;
        else
            cs = (((clk2+5)>>1)&3)+6;
    }
    else if(hclk>0)
    {
        if(family==eeGANON)
            cs=(((hclk-1)>>1)&3)+6;
        else if(hclk<33 && !get_bit(quest_rules,qr_ENEMIESFLICKER))
            cs=(((hclk-1)>>1)&3)+6;
    }
    
    if((tmpscr->flags3&fINVISROOM) &&
            !(current_item(itype_amulet)) &&
            !(get_bit(quest_rules,qr_LENSSEESENEMIES) &&
              lensclk) && family!=eeGANON)
    {
        sprite::drawcloaked(dest);
    }
    else
    {
        if(family !=eeGANON && hclk>0 && get_bit(quest_rules,qr_ENEMIESFLICKER))
        {
            if((frame&1)==1)
                sprite::draw(dest);
        }
        else
            sprite::draw(dest);
    }
    
    cs=cshold;
}

// similar to the overblock function--can do up to a 32x32 sprite
void enemy::drawblock(BITMAP *dest)
{
    int thold=tile;
    int t1=tile;
    int t2=tile+20;
    int t3=tile+1;
    int t4=tile+21;
    
    if(flip&1)
    {
        zc_swap(t1,t3);
        zc_swap(t2,t4);
    }
    
    if(flip&2)
    {
        zc_swap(t1,t2);
        zc_swap(t3,t4);
    }
    
    tile=t1;
    enemy::draw(dest);
    tile=t2;
    yofs+=16;
    enemy::draw(dest);
    yofs-=16;
    tile=t3;
    xofs+=16;
    enemy::draw(dest);
    tile=t4;
    yofs+=16;
    enemy::draw(dest);
    xofs-=16;
    yofs-=16;
    
    tile=thold;
}

void enemy::drawshadow(BITMAP *dest, bool translucent)
{
    if(dont_draw() || tmpscr->flags7&fSIDEVIEW)
    {
        return;
    }
    
    if(dying)
    {
        return;
    }
    
    if(((tmpscr->flags3&fINVISROOM)&& !(current_item(itype_amulet)))||
            (darkroom))
    {
        return;
    }
    else
    {
        /*   if (canfall(id) && z>0)
             shadowtile = wpnsbuf[iwShadow].tile;
           sprite::drawshadow(dest,translucent);
           if (z==0)
             shadowtile = 0;*/
        // a bad idea, as enemies do their own setting of the shadow tile (since some use the
        // 2x2 tiles, shadows animate, etc.) -DD
        
        //this hack is in place as not all enemies that should use the z axis while in the air
        //(ie rocks, boulders) actually do. To be removed when the enemy revamp is complete -DD
        if(canfall(id) && shadowtile == 0)
            shadowtile = wpnsbuf[iwShadow].tile;
            
        if(z>0 || !canfall(id))
            sprite::drawshadow(dest,translucent);
    }
}

void enemy::masked_draw(BITMAP *dest,int mx,int my,int mw,int mh)
{
    BITMAP *sub=create_sub_bitmap(dest,mx,my,mw,mh);
    
    if(sub!=NULL)
    {
        xofs-=mx;
        yofs-=my;
        enemy::draw(sub);
        xofs+=mx;
        yofs+=my;
        destroy_bitmap(sub);
    }
    else
        enemy::draw(dest);
}

// override hit detection to check for invicibility, stunned, etc
bool enemy::hit(sprite *s)
{
    if(!(s->scriptcoldet&1)) return false;
    
    return (dying || hclk>0) ? false : sprite::hit(s);
}

bool enemy::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    return (dying || hclk>0) ? false : sprite::hit(tx,ty,tz,txsz2,tysz2,tzsz2);
}

bool enemy::hit(weapon *w)
{
    if(!(w->scriptcoldet&1)) return false;
    
    return (dying || hclk>0) ? false : sprite::hit(w);
}

//                         --==**==--

//   Movement routines that can be used by derived classes as needed

//                         --==**==--

bool enemy::cannotpenetrate()
{
    return (family == eeAQUA || family == eeMANHAN || family == eeGHOMA);
}

// returns true if next step is ok, false if there is something there
bool enemy::canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2)
{
    bool ok;
    int dx = 0, dy = 0;
    int sv = 8;
    s += 0.5; // Make the ints round; doesn't seem to cause any problems.
    
    switch(ndir)
    {
    case 8:
    case up:
        if(canfall(id) && tmpscr->flags7&fSIDEVIEW)
            return false;
            
        dy = dy1-s;
        special = (special==spw_clipbottomright)?spw_none:special;
        ok = !m_walkflag(x,y+dy,special, x, y) && !flyerblocked(x,y+dy, special);
        break;
        
    case 12:
    case down:
        if(canfall(id) && tmpscr->flags7&fSIDEVIEW)
            return false;
            
        dy = dy2+s;
        ok = !m_walkflag(x,y+dy,special, x, y) && !flyerblocked(x,y+dy, special);
        break;
        
    case 14:
    case left:
        dx = dx1-s;
        sv = ((tmpscr->flags7&fSIDEVIEW)?7:8);
        special = (special==spw_clipbottomright||special==spw_clipright)?spw_none:special;
        ok = !m_walkflag(x+dx,y+sv,special, x, y) && !flyerblocked(x+dx,y+8, special);
        break;
        
    case 10:
    case right:
        dx = dx2+s;
        sv = ((tmpscr->flags7&fSIDEVIEW)?7:8);
        ok = !m_walkflag(x+dx,y+sv,special, x, y) && !flyerblocked(x+dx,y+8, special);
        break;
        
    case 9:
    case r_up:
        dx = dx2+s;
        dy = dy1-s;
        ok = !m_walkflag(x,y+dy,special, x, y) && !m_walkflag(x+dx,y+sv,special, x, y) &&
             !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
        break;
        
    case 11:
    case r_down:
        dx = dx2+s;
        dx = dy2+s;
        ok = !m_walkflag(x,y+dy,special, x, y) && !m_walkflag(x+dx,y+sv,special, x, y) &&
             !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
        break;
        
    case 13:
    case l_down:
        dx = dx1-s;
        dy = dy2+s;
        ok = !m_walkflag(x,y+dy,special, x, y) && !m_walkflag(x+dx,y+sv,special, x, y) &&
             !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
        break;
        
    case 15:
    case l_up:
        dx = dx1-s;
        dy = dy1-s;
        ok = !m_walkflag(x,y+dy,special, x, y) && !m_walkflag(x+dx,y+sv,special, x, y) &&
             !flyerblocked(x,y+dy, special) && !flyerblocked(x+dx,y+8, special);
        break;
        
    default:
        db=99;
        return true;
    }
    
    return ok;
}


bool enemy::canmove(int ndir,fix s,int special)
{
    return canmove(ndir,s,special,0,-8,15,15);
}

bool enemy::canmove(int ndir,int special)
{
    bool dodongo_move=true; //yes, it's an ugly hack, but we're going to rewrite everything later anyway - DN
    
    if(special==spw_clipright&&ndir==right)
    {
        dodongo_move=canmove(ndir,(fix)1,special,0,-8,31,15);
    }
    
    return canmove(ndir,(fix)1,special,0,-8,15,15)&&dodongo_move;
}

bool enemy::canmove(int ndir)
{
    return canmove(ndir,(fix)1,spw_none,0,-8,15,15);
}

int enemy::pos(int newx,int newy)
{
    return (newy<<8)+newx;
}

int wpnsfx(int wpn)
{
    switch(wpn)
    {
    case ewFireTrail:
    case ewFlame:
    case ewFlame2Trail:
    case ewFlame2:
        return WAV_FIRE;
        
    case ewWind:
    case ewMagic:
        return WAV_WAND;
        
    case ewIce:
        return WAV_ZN1ICE;
        
    case ewRock:
        if(get_bit(quest_rules,qr_MORESOUNDS)) return WAV_ZN1ROCK;
        
    case ewFireball2:
    case ewFireball:
        if(get_bit(quest_rules,qr_MORESOUNDS)) return WAV_ZN1FIREBALL;
    }
    
    return -1;
}

/**********************************************************/
/*********  AngelScript enemy class (temporary)  **********/
/**********************************************************/

// scriptFlags
#define asf_ALWAYSDIE    0x0001 // Invincibility doesn't prevent death by ringleader
#define asf_PATRADEFENSE 0x0002 // Center eye is invincible

ASEnemy::ASEnemy(const char* scriptName, fix x, fix y, int id, int clk):
    enemy(x, y, id, clk),
    haslink(false),
    scriptFlags(0)
{
    assignEnemyScript(this, scriptName);
    asUpdate=scriptData->getFunction("bool Update(int)");
    asDraw=scriptData->getFunction("void Draw()");
    asDraw2=scriptData->getFunction("void Draw2()");
    asDrawShadow=scriptData->getFunction("void DrawShadow()");
    asTakeHit=scriptData->getFunction("int takehit(weapon@)");
    asHit=scriptData->getFunction("bool Hit(weapon@)");
    asBreakShield=scriptData->getFunction("void break_shield()");
    asEatLink=scriptData->getFunction("void eatlink()");
}

bool ASEnemy::animate(int index)
{
    if(asUpdate)
    {
        scriptData->runFunction(asUpdate, index);
        return scriptData->getLastResult<bool>();
    }
    
    return enemy::animate(index);
}

void ASEnemy::drawshadow(BITMAP *dest, bool translucent)
{
    if(asDrawShadow)
    {
        scriptDrawingTarget=dest;
        scriptShadowTrans=translucent;
        scriptData->runFunction(asDrawShadow);
    }
    else
        enemy::drawshadow(dest, translucent);
}

void ASEnemy::draw(BITMAP *dest)
{
    if(asDraw)
    {
        scriptDrawingTarget=dest;
        scriptData->runFunction(asDraw);
    }
    else
        enemy::draw(dest);
}

void ASEnemy::draw2(BITMAP *dest)
{
    if(asDraw2)
    {
        scriptDrawingTarget=dest;
        scriptData->runFunction(asDraw2);
    }
    else
        enemy::draw2(dest);
}

void ASEnemy::stopBGSFX()
{
    bgSFX.stopLooping();
}

void ASEnemy::playDeathSFX()
{
    deathSFX.play(x);
}

void ASEnemy::playHitSFX(int pan)
{
    hitSFX.play(pan);
}

bool ASEnemy::hit(weapon *w)
{
    if(asHit)
    {
        scriptData->runFunction(asHit, w->getScriptObject());
        return scriptData->getLastResult<bool>();
    }
    else
        return enemy::hit(w);
}

int ASEnemy::takehit(weapon *w)
{
    if(asTakeHit)
    {
        scriptData->runFunction(asTakeHit, w->getScriptObject());
        return scriptData->getLastResult<int>();
    }
    else
        return enemy::takehit(w);
}

int ASEnemy::defend(int wpnId, int *power, int edef)
{
    int ret = enemy::defend(wpnId, power, edef);
    if((scriptFlags&asf_PATRADEFENSE)!=0)
        return 0;
        
    return ret;
}

void ASEnemy::break_shield()
{
    if(asBreakShield)
        scriptData->runFunction(asBreakShield);
}

void ASEnemy::eatlink()
{
    if(asEatLink)
        scriptData->runFunction(asEatLink);
}

void ASEnemy::fireWeapon()
{
    if(attack)
        attack->activate();
}

bool ASEnemy::isFiring() const
{
    return attack && attack->isActive();
}

void ASEnemy::updateFiring()
{
    if(attack && attack->isActive())
        attack->update();
}

void ASEnemy::setBreathTimer(int time)
{
    if(attack)
        attack->setBreathTimer(time);
}

void ASEnemy::setAttackOwner(int index)
{
    if(attack)
        attack->setOwner((enemy*)guys.spr(index));
}

void ASEnemy::activateDeathAttack()
{
    if(deathAttack)
    {
        deathAttack->activate();
        deathAttack.reset();
    }
}

void ASEnemy::scriptDraw()
{
    enemy::draw(scriptDrawingTarget);
}

void ASEnemy::scriptDrawShadow()
{
    enemy::drawshadow(scriptDrawingTarget, scriptShadowTrans);
}

void ASEnemy::scriptDrawBlock()
{
    drawblock(scriptDrawingTarget);
}

void ASEnemy::scriptMaskedDraw(int mx, int my, int mw, int mh)
{
    masked_draw(scriptDrawingTarget, mx, my, mw, mh);
}

void ASEnemy::overTileCloaked16(int tile, int x, int y, int flip)
{
    overtilecloaked16(scriptDrawingTarget, tile, x, y, flip);
}

void ASEnemy::overTile16(int tile, int x, int y, int cset, int flip)
{
    overtile16(scriptDrawingTarget, tile, x, y, cset, flip);
}

bool ASEnemy::defaultAnimate(int index)
{
    return enemy::animate(index);
}

int ASEnemy::defaultTakeHit(weapon* w)
{
    int ret=enemy::takehit(w);
    return ret;
}

void ASEnemy::kickbucket()
{
    if((scriptFlags&asf_ALWAYSDIE) || !superman)
        hp=-1000;
}

// This is really ugly...
int ASEnemy::scriptDefendItemClass(int wpnId, int power)
{
    wpnPower=power;
    return defenditemclass(wpnId, &wpnPower);
}

int ASEnemy::getDefendedItemPower()
{
    return wpnPower;
}

void ASEnemy::setDeathAttack(EnemyAttack* att)
{
    deathAttack.reset(att);
    deathAttack->setOwner(this);
}

/**********************************/
/**********  Misc Code  ***********/
/**********************************/

int hit_enemy(int index, int wpnId,int power,int wpnx,int wpny,int dir, int enemyHitWeapon)
{
    weapon w((fix)wpnx,(fix)wpny,(fix)0,wpnId,0,power,dir,enemyHitWeapon,-1,false);
    int ret= ((enemy*)guys.spr(index))->takehit(&w);
    return ret;
}

void enemy_scored(int index)
{
    ((enemy*)guys.spr(index))->scored=true;
}

void addguy(int x,int y,int id,int clk,bool mainguy)
{
    if(mainguy) // Slightly ugly hack here
        id|=0x8000;
    ASEnemy* g = new ASEnemy("Guy", (fix)x,(fix)(y+(isdungeon()?1:0)),id,get_bit(quest_rules,qr_NOGUYPOOF)?0:clk);
    guys.add(g);
}

item* additem(int x,int y,int id,int pickup)
{
    item *i = new item(fix(x), fix(y - get_bit(quest_rules, qr_NOITEMOFFSET)), fix(0), id, pickup, 0);
    items.add(i);
    return i;
}

item* additem(int x,int y,int id,int pickup,int clk)
{
    item *i = new item((fix)x,(fix)y-(get_bit(quest_rules, qr_NOITEMOFFSET)),(fix)0,id,pickup,clk);
    items.add(i);
    return i;
}

void kill_em_all()
{
    for(int i=0; i<guys.Count(); i++)
    {
        enemy *e = ((enemy*)guys.spr(i));
        
        if(e->flags&(1<<3) && !(e->family == eeGHINI && e->dmisc1 == 1)) continue;
        
        e->kickbucket();
    }
}

// For Link's hit detection. Don't count them if they are stunned or are guys.
int GuyHit(int tx,int ty,int tz,int txsz,int tysz,int tzsz)
{
    for(int i=0; i<guys.Count(); i++)
    {
        if(guys.spr(i)->hit(tx,ty,tz,txsz,tysz,tzsz))
        {
            if(((enemy*)guys.spr(i))->stunclk==0 && (!get_bit(quest_rules, qr_SAFEENEMYFADE) || ((enemy*)guys.spr(i))->fading != fade_flicker)
                    &&(((enemy*)guys.spr(i))->d->family != eeGUY || ((enemy*)guys.spr(i))->dmisc1))
            {
                return i;
            }
        }
    }
    
    return -1;
}

// For Link's hit detection. Count them if they are dying.
int GuyHit(int index,int tx,int ty,int tz,int txsz,int tysz,int tzsz)
{
    enemy *e = (enemy*)guys.spr(index);
    
    if(e->hp > 0)
        return -1;
        
    bool d = e->dying;
    int hc = e->hclk;
    e->dying = false;
    e->hclk = 0;
    bool hit = e->hit(tx,ty,tz,txsz,tysz,tzsz);
    e->dying = d;
    e->hclk = hc;
    
    return hit ? index : -1;
}

bool hasMainGuy()
{
    for(int i=0; i<guys.Count(); i++)
    {
        if(((enemy*)guys.spr(i))->mainguy)
        {
            return true;
        }
    }
    
    return false;
}

void EatLink(int index)
{
    ((ASEnemy*)guys.spr(index))->eatlink();
}

void GrabLink(int index)
{
    // Move to scripts when possible...
    
    ASEnemy* wm=(ASEnemy*)guys.spr(index);
    wm->haslink=true;
    wm->superman=1;
}

bool CarryLink()
{
    for(int i=0; i<guys.Count(); i++)
    {
        if(((enemy*)(guys.spr(i)))->family==eeWALLM)
        {
            if(((ASEnemy*)guys.spr(i))->haslink)
            {
                Link.x=guys.spr(i)->x;
                Link.y=guys.spr(i)->y;
                return ((ASEnemy*)guys.spr(i))->misc > 0;
            }
        }
    }
    
    return false;
}

// Move item with guy
void movefairy(fix &x,fix &y,int misc)
{
    int i = guys.idFirst(eITEMFAIRY+0x1000*misc);
    
    if(i!=-1)
    {
        x = guys.spr(i)->x;
        y = guys.spr(i)->y;
    }
}

// Move guy with item (used by FFC scripts and hookshot-dragged fairies)
void movefairy2(fix x,fix y,int misc)
{
    int i = guys.idFirst(eITEMFAIRY+0x1000*misc);
    
    if(i!=-1)
    {
        guys.spr(i)->x = x;
        guys.spr(i)->y = y;
    }
}

void killfairy(int misc)
{
    int i = guys.idFirst(eITEMFAIRY+0x1000*misc);
    guys.del(i);
}

int addenemy(int x,int y,int id,int clk)
{
    return addenemy(x,y,0,id,clk);
}

static void setStandardAttack(enemy* en, const guydata& data)
{
    fireType ft;
    int weapon=data.weapon;
    int wType=2;
    int power=data.wdp;
    int sfx=wpnsfx(data.weapon);
    
    switch(data.misc1)
    {
    case e1tFAST:
        wType=6;
        // Fall through
    case e1tNORMAL:
    case e1tCONSTANT: // Also e1tHOMINGBRANG
    case e1tEACHTILE:
        ft=ft_1Shot;
        break;
    case e1tSLANT:
        ft=ft_1ShotSlanted;
        break;
    case e1t3SHOTSFAST:
        wType=6;
        // Fall through
    case e1t3SHOTS:
        ft=ft_3Shots;
        break;
    case e1t5SHOTS:
        ft=ft_5Shots;
        break;
    case e1t4SHOTS:
        ft=ft_4Shots;
        wType=0;
        break;
    case e1t8SHOTS:
        ft=ft_8Shots;
        wType=0;
        break;
    case e1tFIREOCTO:
        if(data.weapon!=ewFireTrail)
            ft=ft_breath;
        else
            ft=ft_1Shot;
        break;
        
    case e1tSUMMON:
        ft=ft_summon;
        weapon=data.misc3;
        power=zc_max(1, data.misc4);
        sfx=get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
        break;
        
    case e1tSUMMONLAYER:
        ft=ft_summonLayer;
        power=zc_max(1, data.misc4);
        sfx=get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
        break;
        
    default:
        return;
    }
    
    EnemyAttack* atk;
    atk=new EnemyAttack(weapon, wType, power, ft, sfxMgr.getSFX(sfx));
    en->setAttack(atk);
}

static void setWalkerDeathAttack(ASEnemy* en, const guydata& data)
{
    fireType ft;
    int wType;
    
    switch(data.misc2)
    {
    case e2tFIREOCTO:
        ft=ft_8Shots;
        wType=0;
        break;
        
    case e2tBOMBCHU:
        ft=ft_1Shot;
        wType=2;
        break;
        
    default:
        return;
    }
    
    EnemyAttack* atk;
    atk=new EnemyAttack(data.weapon+data.misc3, 0, data.misc4, ft,
      sfxMgr.getSFX(data.weapon+data.misc3));
    en->setDeathAttack(atk);
}

static void setWizzrobeAttack(enemy* en, const guydata& data)
{
    fireType ft;
    int weapon;
    int power=data.wdp;
    int sfx=WAV_FIRE;
    
    switch(data.misc2)
    {
    case 0: // Single shot
        ft=ft_1Shot;
        weapon=data.weapon;
        sfx=WAV_WAND;
        break;
        
    case 1: // 8-way
        ft=ft_8Shots;
        weapon=data.weapon;
        break;
        
    case 2: // Summon misc. 3
        ft=ft_summon;
        weapon=data.misc3;
        power=3;
        break;
        
    case 3: // Summon from layer
        ft=ft_summonLayer;
        weapon=-1;
        power=3;
        sfx=get_bit(quest_rules,qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
        break;
        
    default:
        return;
    }
    
    if(weapon==0) // wNone==0
        return;
    EnemyAttack* atk=new EnemyAttack(weapon, 0, power, ft,
      sfxMgr.getSFX(sfx));
    en->setAttack(atk);
}

static void setAquamentusAttack(enemy* en, const guydata& data)
{
    if(data.weapon==wNone)
        return;
    EnemyAttack* atk=new EnemyAttack(data.weapon, 2, data.wdp,
      ft_aquamentus, sfxMgr.getSFX(wpnsfx(data.weapon)));
    en->setAttack(atk);
}

static void setGohmaAttack(enemy* en, const guydata& data)
{
    if(data.weapon==wNone)
        return;
    
    fireType ft;
    int wpnType;
    switch(data.misc1)
    {
    case 0: // Single shot
        ft=ft_gohma1Shot;
        wpnType=3;
        break;
        
    case 1: // Triple shot
        ft=ft_gohma3Shots;
        wpnType=3;
        break;
        
    case 2: // Breath
        ft=ft_breathAimed;
        wpnType=2;
        break;
    }
    
    EnemyAttack* atk=new EnemyAttack(data.weapon, wpnType,
      data.wdp, ft, sfxMgr.getSFX(wpnsfx(data.weapon)));
    if(data.misc1!=2) // Single or triple shot
        atk->setYOffset(2);
    else // Breath
        atk->setBreathTimer(104);
    en->setAttack(atk);
}

static void setSimpleAttack(enemy* en, const guydata& data, int type)
{
    if(data.weapon==wNone)
        return;
    EnemyAttack* atk=new EnemyAttack(data.weapon, type, data.wdp,
      ft_1Shot, sfxMgr.getSFX(wpnsfx(data.weapon)));
    en->setAttack(atk);
}

static void setGleeokBreathAttack(enemy* en, const guydata& data)
{
    if(data.weapon==wNone)
        return;
    EnemyAttack* atk=new EnemyAttack(data.weapon, 2, data.wdp,
      ft_breathAimed, sfxMgr.getSFX(wpnsfx(data.weapon)));
    en->setAttack(atk);
}

// Returns number of enemies/segments created
int addenemy(int x,int y,int z,int id,int clk)
{
    if(id <= 0) return 0;
    
    int ret = 0;
    sprite *e=NULL;
    const guydata& data=guysbuf[id&0xFFF];
    
    switch(data.family)
    {
        //Fixme: possible enemy memory leak. (minor)
    case eeWALK:
        e = new ASEnemy("WalkingEnemy", (fix)x,(fix)y,id,clk);
        setStandardAttack((enemy*)e, data);
        setWalkerDeathAttack((ASEnemy*)e, data);
        break;
        
    case eeLEV:
        e = new ASEnemy("Leever", (fix)x,(fix)y,id,clk);
        break;
        
    case eeTEK:
        //e = new eTektite((fix)x,(fix)y,id,clk);
        e = new ASEnemy("Tektite", (fix)x,(fix)y,id,clk);
        break;
        
    case eePEAHAT:
        //e = new ePeahat((fix)x,(fix)y,id,clk);
        e = new ASEnemy("Peahat", (fix)x,(fix)y,id,clk);
        break;
        
    case eeZORA:
        e = new ASEnemy("Zora", (fix)x,(fix)y,id,clk);
        setSimpleAttack((enemy*)e, data, 2);
        break;
        
    case eeGHINI:
        e = new ASEnemy("Ghini", (fix)x,(fix)y,id,clk);
        break;
        
    case eeKEESE:
        e = new ASEnemy("Keese", (fix)x,(fix)y,id,clk);
        break;
        
    case eeWIZZ:
        e = new ASEnemy("Wizzrobe", (fix)x,(fix)y,id,clk);
        setWizzrobeAttack((enemy*)e, data);
        break;
        
    case eePROJECTILE:
        e = new ASEnemy("ProjectileShooter", (fix)x,(fix)y,id,clk);
        setStandardAttack((enemy*)e, data);
        break;
        
    case eeWALLM:
        e = new ASEnemy("WallMaster", (fix)x,(fix)y,id,clk);
        break;
        
    case eeAQUA:
    {
        e = new ASEnemy("Aquamentus", (fix)x,(fix)y,id,clk);
        setAquamentusAttack((enemy*)e, data);
        break;
    }
        
    case eeMOLD:
        e = new ASEnemy("MoldormController", (fix)x,(fix)y,id,zc_max(1,zc_min(254,data.misc1)));
        break;
        
    case eeMANHAN:
        e = new ASEnemy("Manhandla", (fix)x,(fix)y,id,clk);
        break;
        
    case eeGLEEOK:
        e = new ASEnemy("Gleeok", (fix)x,(fix)y,id,zc_max(1,zc_min(254,data.misc1)));
        setSimpleAttack((enemy*)e, data, 3);
        break;
        
    case eeGHOMA:
        e = new ASEnemy("Gohma", (fix)x,(fix)y,id,clk);
        setGohmaAttack((enemy*)e, data);
        break;
        
    case eeLANM:
        e = new ASEnemy("LanmolaController", (fix)x,(fix)y,id,zc_max(1,zc_min(253,data.misc1)));
        break;
        
    case eeGANON:
        e = new ASEnemy("Ganon", (fix)x,(fix)y,id,clk);
        setSimpleAttack((enemy*)e, data, 3);
        break;
        
    case eeFAIRY:
        e = new ASEnemy("Fairy", (fix)x,(fix)y,id+0x1000*clk,clk);
        break;
        
    case eeFIRE:
        e = new ASEnemy("Fire", (fix)x,(fix)y,id,clk);
        break;
        
    case eeOTHER:
        e = new ASEnemy("Other", (fix)x,(fix)y,id,clk);
        break;
        
    case eeSPINTILE:
        e = new ASEnemy("SpinningTile", (fix)x,(fix)y,id,clk);
        break;
        
        // and these enemies use the misc10/misc2 value
    case eeROCK:
        e = new ASEnemy("FallingRock", (fix)x,(fix)y,id,0);
        break;
    
    case eeTRAP:
    {
        switch(data.misc2)
        {
        case 1:
            e = new ASEnemy("TrapConstant", (fix)x,(fix)y,id,clk);
            break;
            
        case 0:
        default:
            e = new ASEnemy("TrapLOS", (fix)x,(fix)y,id,clk);
            break;
        }
        
        break;
    }
    
    case eeDONGO:
        e = new ASEnemy("Dodongo", (fix)x,(fix)y,id,clk);
        break;
    
    case eeDIG:
    {
        switch(data.misc10)
        {
        case 1:
            e = new ASEnemy("DigdoggerKid", (fix)x,(fix)y,id,clk);
            break;
            
        case 0:
        default:
            e = new ASEnemy("Digdogger", (fix)x,(fix)y,id,clk);
            break;
        }
        
        break;
    }
    
    case eePATRA:
    {
        switch(data.misc10)
        {
        case 1:
            e = new ASEnemy("PatraBS", (fix)x,(fix)y,id,clk);
            break;
            
        case 0:
        default:
            e = new ASEnemy("Patra", (fix)x,(fix)y,id,clk);
            break;
        }
        // This part's stupid.
        if(data.misc5>0)
            setSimpleAttack((enemy*)e, data, 3);
        break;
    }
    
    case eeGUY:
    {
        switch(data.misc10)
        {
        case 1:
            e = new ASEnemy("Trigger", (fix)x,(fix)y,id,clk);
            break;
            
        case 0:
        default:
            e = new ASEnemy("NPC", (fix)x,(fix)y,id,clk);
            break;
        }
        
        break;
    }
    
    case eeNONE:
        if(data.misc10 ==1)
        {
            e = new ASEnemy("Trigger", (fix)x,(fix)y,id,clk);
            break;
        }
        
    default:
    
        return 0;
    }
    
    ret++; // Made one enemy.
    
    if(z && canfall(id))
    {
        e->z = (fix)z;
    }
    
    ((enemy*)e)->ceiling = (z && canfall(id));
    
    if(!guys.add(e))
    {
        return 0;
    }
    
    // add segments of segmented enemies
    int c=0;
    
    switch(data.family)
    {
    case eeMOLD:
    {
        byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
        id &= 0xFFF;
        
        for(int i=0; i<zc_max(1,zc_min(254,guysbuf[id].misc1)); i++)
        {
            //christ this is messy -DD
            int segclk = -i*((int)(8.0/(fix(data.step/100.0))));
            
            if(!guys.add(new ASEnemy("MoldormSegment", (fix)x,(fix)y,id+0x1000,segclk)))
            {
                al_trace("Moldorm segment %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1; j++)
                    guys.del(guys.Count()-1);
                    
                return 0;
            }
            
            if(i>0)
                ((enemy*)guys.spr(guys.Count()-1))->item_set=is;
                
            ret++;
        }
        
        break;
    }
    
    case eeLANM:
    {
        id &= 0xFFF;
        int shft = guysbuf[id].misc2;
        byte is=((enemy*)guys.spr(guys.Count()-1))->item_set;
        
        if(!guys.add(new ASEnemy("LanmolaSegment", (fix)x,(fix)y,id+0x1000,0)))
        {
            al_trace("Lanmola segment 1 could not be created!\n");
            guys.del(guys.Count()-1);
            return 0;
        }
        
        ret++;
        
        for(int i=1; i<zc_max(1,zc_min(253,data.misc1)); i++)
        {
            if(!guys.add(new ASEnemy("LanmolaSegment", (fix)x,(fix)y,id+0x2000,-(i<<shft))))
            {
                al_trace("Lanmola segment %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1; j++)
                    guys.del(guys.Count()-1);
                    
                return 0;
            }
            
            ((enemy*)guys.spr(guys.Count()-1))->item_set=is;
            ret++;
        }
    }
    break;
    
    case eeMANHAN:
        id &= 0xFFF;
        
        for(int i=0; i<((!(data.misc2))?4:8); i++)
        {
            enemy* en=new ASEnemy("ManhandlaHead", (fix)x,(fix)y,id+0x1000,i);
            if(!guys.add(en))
            {
                al_trace("Manhandla head %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1; j++)
                {
                    guys.del(guys.Count()-1);
                }
                
                delete en;
                return 0;
            }
            
            ret++;
            setSimpleAttack(en, data, 3);
            en->frate=guysbuf[id].misc1;
        }
        
        break;
        
    case eeGLEEOK:
    {
        id &= 0xFFF;
        
        for(int i=0; i<zc_max(1,zc_min(254,data.misc1)); i++)
        {
            enemy* en=new ASEnemy("GleeokHead", (fix)x,(fix)y,id+0x1000,c);
            if(!guys.add(en))
            {
                al_trace("Gleeok head %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1; j++)
                {
                    guys.del(guys.Count()-1);
                }
                
                delete en;
                return false;
            }
            setGleeokBreathAttack(en, data);
            c-=guysbuf[id].misc4;
            ret++;
        }
    }
    break;
    
    
    case eePATRA:
    {
        id &= 0xFFF;
        int outeyes = 0;
        
        for(int i=0; i<zc_min(254,data.misc1); i++)
        {
            if(!(guysbuf[id].misc10?guys.add(new ASEnemy("PatraOrbiterBS", (fix)x,(fix)y,id+0x1000,i)):guys.add(new ASEnemy("PatraOrbiter", (fix)x,(fix)y,id+0x1000,i))))
            {
                al_trace("Patra outer eye %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1; j++)
                    guys.del(guys.Count()-1);
                    
                return 0;
            }
            else
                outeyes++;
                
            ret++;
        }
        
        for(int i=0; i<zc_min(254,data.misc2); i++)
        {
            if(!guys.add(new ASEnemy("PatraOrbiter", (fix)x,(fix)y,id+0x1000,i)))
            {
                al_trace("Patra inner eye %d could not be created!\n",i+1);
                
                for(int j=0; j<i+1+zc_min(254,outeyes); j++)
                    guys.del(guys.Count()-1);
                    
                return 0;
            }
            
            ret++;
        }
        
        break;
    }
    }
    
    return ret;
}

bool isjumper(int id)
{
    switch(guysbuf[id&0xFFF].family)
    {
    case eeROCK:
    case eeTEK:
        return true;
        
    case eeWALK:
        if(guysbuf[id&0xFFF].misc9==e9tVIRE || guysbuf[id&0xFFF].misc9==e9tPOLSVOICE) return true;
    }
    
    return false;
}


bool isfixedtogrid(int id)
{
    switch(guysbuf[id&0xFFF].family)
    {
    case eeWALK:
    case eeLEV:
    case eeZORA:
    case eeDONGO:
    case eeGANON:
    case eeROCK:
    case eeGLEEOK:
    case eeAQUA:
    case eeLANM:
        return true;
    }
    
    return false;
}

// Can't fall, can have Z value.
bool isflier(int id)
{
    switch(guysbuf[id&0xFFF].family) //id&0x0FFF)
    {
    case eePEAHAT:
    case eeKEESE:
    case eePATRA:
    case eeFAIRY:
    case eeGHINI:
    
        // Could theoretically have their Z set by a script
    case eeFIRE:
        return true;
        break;
    }
    
    return false;
}

// Can't have Z position
bool never_in_air(int id)
{
    switch(guysbuf[id&0xFFF].family)
    {
    case eeMANHAN:
    case eeMOLD:
    case eeLANM:
    case eeGLEEOK:
    case eeZORA:
    case eeLEV:
    case eeAQUA:
    case eeROCK:
    case eeGANON:
    case eeTRAP:
    case eePROJECTILE:
    case eeSPINTILE:
        return true;
    }
    
    return false;
}

bool canfall(int id)
{
    switch(guysbuf[id&0xFFF].family)
    {
    case eeGUY:
    {
        if(id < eOCTO1S)
            return false;
            
        switch(guysbuf[id&0xFFF].misc10)
        {
        case 1:
        case 2:
            return true;
            
        case 0:
        case 3:
        default:
            return false;
        }
        
        case eeGHOMA:
        case eeDIG:
            return false;
        }
    }
    
    return !never_in_air(id) && !isflier(id) && !isjumper(id);
}

void addfires()
{
    if(!get_bit(quest_rules,qr_NOGUYFIRES))
    {
        int bs = get_bit(quest_rules,qr_BSZELDA);
        addguy(bs? 64: 72,64,gFIRE,-17,false);
        addguy(bs?176:168,64,gFIRE,-18,false);
    }
}

void loadguys()
{
    if(loaded_guys)
        return;
        
    loaded_guys=true;
    
    byte Guy=0;
    // When in caves/item rooms, use mBELOW and ipONETIME2
    // Else use mITEM and ipONETIME
    int mf = (currscr>=128) ? mBELOW : mITEM;
    int onetime = (currscr>=128) ? ipONETIME2 : ipONETIME;
    
    hasitem=0;
    
    if(currscr>=128 && DMaps[currdmap].flags&dmfGUYCAVES)
    {
        if(DMaps[currdmap].flags&dmfCAVES)
        {
            Guy=tmpscr[1].guy;
        }
    }
    else
    {
        Guy=tmpscr->guy;
        
        if(DMaps[currdmap].flags&dmfVIEWMAP)
            game->maps[(currmap*MAPSCRSNORMAL)+currscr] |= mVISITED;          // mark as visited
    }
    
    // The Guy appears if 'Link is in cave' equals 'Guy is in cave'.
    if(Guy && ((currscr>=128) == !!(DMaps[currdmap].flags&dmfGUYCAVES)))
    {
        if(tmpscr->room==rZELDA)
        {
            addguy(120,72,Guy,-15,true);
            guys.spr(0)->hxofs=1000;
            addenemy(128,96,eFIRE,-15);
            addenemy(112,96,eFIRE,-15);
            addenemy(96,120,eFIRE,-15);
            addenemy(144,120,eFIRE,-15);
            return;
        }
        
        if(Guy!=gFAIRY || !get_bit(quest_rules,qr_NOFAIRYGUYFIRES))
            addfires();
            
        if(currscr>=128)
            if(getmapflag())
                Guy=0;
                
        switch(tmpscr->room)
        {
        case rSP_ITEM:
        case rMONEY:
        case rGRUMBLE:
        case rBOMBS:
        case rARROWS:
        case rSWINDLE:
        case rMUPGRADE:
        case rLEARNSLASH:
        case rTAKEONE:
        case rREPAIR:
        case rRP_HC:
            if(getmapflag())
                Guy=0;
                
            break;
            
        case rTRIFORCE:
        {
            int tc = TriforceCount();
            
            if(get_bit(quest_rules,qr_4TRI))
            {
                if((get_bit(quest_rules,qr_3TRI) && tc>=3) || tc>=4)
                    Guy=0;
            }
            else
            {
                if((get_bit(quest_rules,qr_3TRI) && tc>=6) || tc>=8)
                    Guy=0;
            }
        }
        break;
        }
        
        if(Guy)
        {
            if(Guy!=gFAIRY || !get_bit(quest_rules,qr_NOFAIRYGUYFIRES))
                blockpath=true;
                
            if(currscr<128)
                sfx(WAV_SCALE);
                
            addguy(120,64,Guy, (dlevel||BSZ)?-15:startguy[rand()&7], true);
            Link.Freeze();
        }
    }
    else if(Guy==gFAIRY)  // The only Guy that somewhat ignores the "Guys In Caves Only" DMap flag
    {
        sfx(WAV_SCALE);
        addguy(120,62,gFAIRY,-14,false);
    }
    
    loaditem();
    
    // Collecting a rupee in a '10 Rupees' screen sets the mITEM screen state if
    // it doesn't appear in a Cave/Item Cellar, and the mBELOW screen state if it does.
    if(tmpscr->room==r10RUPIES && !getmapflag(mf))
    {
        //setmapflag();
        for(int i=0; i<10; i++)
            additem(ten_rupies_x[i],ten_rupies_y[i],0,ipBIGRANGE+onetime,-14);
    }
}

void loaditem()
{
    byte Item = 0;
    
    if(currscr<128)
    {
        Item=tmpscr->item;
        
        if(!getmapflag(mITEM) && (tmpscr->hasitem != 0))
        {
            if(tmpscr->flags&fITEM)
                hasitem=1;
            else if(tmpscr->enemyflags&efCARRYITEM)
                hasitem=4; // Will be set to 2 by roaming_item
            else
                items.add(new item((fix)tmpscr->itemx,
                                   (tmpscr->flags7&fITEMFALLS && tmpscr->flags7&fSIDEVIEW) ? (fix)-170 : (fix)tmpscr->itemy+(get_bit(quest_rules, qr_NOITEMOFFSET)?0:1),
                                   (tmpscr->flags7&fITEMFALLS && !(tmpscr->flags7&fSIDEVIEW)) ? (fix)170 : (fix)0,
                                   Item,ipONETIME+ipBIGRANGE+((itemsbuf[Item].family==itype_triforcepiece ||
                                           (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0),0));
        }
    }
    else if(!(DMaps[currdmap].flags&dmfCAVES))
    {
        if(!getmapflag() && tmpscr[1].room==rSP_ITEM
                && (currscr==128 || !get_bit(quest_rules,qr_ITEMSINPASSAGEWAYS)))
        {
            Item=tmpscr[1].catchall;
            
            if(Item)
                items.add(new item((fix)tmpscr->itemx,
                                   (tmpscr->flags7&fITEMFALLS && tmpscr->flags7&fSIDEVIEW) ? (fix)-170 : (fix)tmpscr->itemy+(get_bit(quest_rules, qr_NOITEMOFFSET)?0:1),
                                   (tmpscr->flags7&fITEMFALLS && !(tmpscr->flags7&fSIDEVIEW)) ? (fix)170 : (fix)0,
                                   Item,ipONETIME2|ipBIGRANGE|ipHOLDUP,0));
        }
    }
}

void never_return(int index)
{
    if(!get_bit(quest_rules,qr_KILLALL))
        goto doit;
        
    for(int i=0; i<guys.Count(); i++)
        if(((((enemy*)guys.spr(i))->d->flags)&guy_neverret) && i!=index)
        {
            goto dontdoit;
        }
        
doit:
    setmapflag(mNEVERRET);
dontdoit:
    return;
}

bool slowguy(int id)
{
    switch(id)
    {
    case eOCTO1S:
    case eOCTO2S:
    case eOCTO1F:
    case eOCTO2F:
    case eLEV1:
    case eLEV2:
    case eROCK:
    case eBOULDER:
        return true;
    }
    
    return false;
}

bool ok2add(int id)
{
    if(getmapflag(mNEVERRET) && (guysbuf[id].flags & guy_neverret))
        return false;
        
    switch(guysbuf[id].family)
    {
        // I added a special case for shooters because having traps on the same screen
        // was preventing them from spawning due to TMPNORET. This means they will
        // never stay dead, though, so it may not be the best solution. - Saf
    case eePROJECTILE:
        return true;
        
    case eeGANON:
    case eeTRAP:
        return false;
        
    case eeDIG:
    {
        switch(guysbuf[id].misc10)
        {
        case 1:
            if(!get_bit(quest_rules,qr_NOTMPNORET))
                return !getmapflag(mTMPNORET);
                
            return true;
            
        case 0:
        default:
            return true;
        }
    }
    }
    
    if(!get_bit(quest_rules,qr_NOTMPNORET))
        return !getmapflag(mTMPNORET);
        
    return true;
}

void activate_fireball_statue(int pos)
{
    if(!(tmpscr->enemyflags&efFIREBALLS) || statueID<0)
    {
        return;
    }
    
    int cx=-1000, cy=-1000;
    int x = (pos&15)<<4;
    int y = pos&0xF0;
    
    int ctype = combobuf[MAPCOMBO(x,y)].type;
    
    if(!isfixedtogrid(statueID))
    {
        if(ctype==cL_STATUE)
        {
            cx=x+4;
            cy=y+7;
        }
        else if(ctype==cR_STATUE)
        {
            cx=x-8;
            cy=y-1;
        }
        else if(ctype==cC_STATUE)
        {
            cx=x;
            cy=y;
        }
    }
    else if(ctype==cL_STATUE || ctype==cR_STATUE || ctype==cC_STATUE)
    {
        cx=x;
        cy=y;
    }
    
    if(cx!=-1000)  // No point creating it if this is false
    {
        for(int j=0; j<guys.Count(); j++)
        {
            if((int(guys.spr(j)->x)==cx)&&(int(guys.spr(j)->y)==cy))
            {
                if((guys.spr(j)->id&0xFFF) == statueID)  // There's already a matching enemy here!
                    return; // No point deleting it. A script might be toying with it in some way.
                else
                    guys.del(j);
            }
        }
        
        addenemy(cx, cy, statueID, !isfixedtogrid(statueID) ? 24 : 0);
    }
}

void activate_fireball_statues()
{
    if(!(tmpscr->enemyflags&efFIREBALLS))
    {
        return;
    }
    
    for(int i=0; i<176; i++)
    {
        activate_fireball_statue(i);
    }
}

void load_default_enemies()
{
    wallm_load_clk=frame-80;
    int Id=0;
    
    if(tmpscr->enemyflags&efZORA)
    {
        if(zoraID>=0)
            addenemy(-16, -16, zoraID, 0);
    }
    
    if(tmpscr->enemyflags&efTRAP4)
    {
        if(cornerTrapID>=0)
        {
            addenemy(32, 32, cornerTrapID, -14);
            addenemy(208, 32, cornerTrapID, -14);
            addenemy(32, 128, cornerTrapID, -14);
            addenemy(208, 128, cornerTrapID, -14);
        }
    }
    
    for(int y=0; y<176; y+=16)
    {
        for(int x=0; x<256; x+=16)
        {
            int ctype = combobuf[MAPCOMBO(x,y)].type;
            int cflag = MAPFLAG(x, y);
            int cflag2 = MAPCOMBOFLAG(x, y);
            
            if(ctype==cTRAP_H || cflag==mfTRAP_H || cflag2==mfTRAP_H)
            {
                if(trapLOSHorizontalID>=0)
                    addenemy(x, y, trapLOSHorizontalID, -14);
            }
            else if(ctype==cTRAP_V || cflag==mfTRAP_V || cflag2==mfTRAP_V)
            {
                if(trapLOSVerticalID>=0)
                    addenemy(x, y, trapLOSVerticalID, -14);
            }
            else if(ctype==cTRAP_4 || cflag==mfTRAP_4 || cflag2==mfTRAP_4)
            {
                if(trapLOS4WayID>=0)
                {
                    if(addenemy(x, y, trapLOS4WayID, -14))
                        static_cast<enemy*>(guys.spr(guys.Count()-1))->dummy_int1=2;
                }
            }
            
            else if(ctype==cTRAP_LR || cflag==mfTRAP_LR || cflag2==mfTRAP_LR)
            {
                if(trapConstantHorizontalID>=0)
                    addenemy(x, y, trapConstantHorizontalID, -14);
            }
            else if(ctype==cTRAP_UD || cflag==mfTRAP_UD || cflag2==mfTRAP_UD)
            {
                if(trapConstantVerticalID>=0)
                    addenemy(x, y, trapConstantVerticalID, -14);
            }
            
            if(ctype==cSPINTILE1)
            {
                // Awaken spinning tile
                awaken_spinning_tile(tmpscr,COMBOPOS(x,y));
            }
        }
    }
    
    if(tmpscr->enemyflags&efTRAP2)
    {
        if(centerTrapID>=-1)
        {
            if(addenemy(64, 80, centerTrapID, -14))
                static_cast<enemy*>(guys.spr(guys.Count()-1))->dummy_int1=1;
            
            if(addenemy(176, 80, centerTrapID, -14))
                static_cast<enemy*>(guys.spr(guys.Count()-1))->dummy_int1=1;
        }
    }
    
    if(tmpscr->enemyflags&efROCKS)
    {
        if(rockID>=0)
        {
            addenemy(rand()&0xF0, 0, rockID, 0);
            addenemy(rand()&0xF0, 0, rockID, 0);
            addenemy(rand()&0xF0, 0, rockID, 0);
        }
    }
    
    activate_fireball_statues();
}


// Everything that must be done before we change a screen's combo to another combo, or a combo's type to another type.
// There's 2 routines because it's unclear if combobuf or tmpscr->data gets modified. -L
void screen_combo_modify_preroutine(mapscr *s, int pos)
{
    delete_fireball_shooter(s, pos);
}

// Everything that must be done after we change a screen's combo to another combo. -L
void screen_combo_modify_postroutine(mapscr *s, int pos)
{
    activate_fireball_statue(pos);
    
    if(combobuf[s->data[pos]].type==cSPINTILE1)
    {
        // Awaken spinning tile
        awaken_spinning_tile(s,pos);
    }
}

void awaken_spinning_tile(mapscr *s, int pos)
{
    addenemy((pos&15)<<4,pos&0xF0,(s->cset[pos]<<12)+eSPINTILE1,animated_combo_table[s->data[pos]][1]+zc_max(1,combobuf[s->data[pos]].frames));
}


// It stands for next_side_pos
void nsp(bool random)
// moves sle_x and sle_y to the next position
{
    if(random)
    {
        if(rand()%2)
        {
            sle_x = (rand()%2) ? 0 : 240;
            sle_y = (rand()%10)*16;
        }
        else
        {
            sle_y = (rand()%2) ? 0 : 160;
            sle_x = (rand()%15)*16;
        }
        
        return;
    }
    
    if(sle_x==0)
    {
        if(sle_y<160)
            sle_y+=16;
        else
            sle_x+=16;
    }
    else if(sle_y==160)
    {
        if(sle_x<240)
            sle_x+=16;
        else
            sle_y-=16;
    }
    else if(sle_x==240)
    {
        if(sle_y>0)
            sle_y-=16;
        else
            sle_x-=16;
    }
    else if(sle_y==0)
    {
        if(sle_x>0)
            sle_x-=16;
        else
            sle_y+=16;
    }
}

int next_side_pos(bool random)
// moves sle_x and sle_y to the next available position
// returns the direction the enemy should face
{
    bool blocked;
    int c=0;
    
    do
    {
        nsp(c>35 ? false : random);
        blocked = _walkflag(sle_x,sle_y,2) || _walkflag(sle_x,sle_y+8,2) ||
                  (combo_class_buf[COMBOTYPE(sle_x,sle_y)].block_enemies ||
                   MAPFLAG(sle_x,sle_y) == mfNOENEMY || MAPCOMBOFLAG(sle_x,sle_y)==mfNOENEMY ||
                   MAPFLAG(sle_x,sle_y) == mfNOGROUNDENEMY || MAPCOMBOFLAG(sle_x,sle_y)==mfNOGROUNDENEMY ||
                   iswater(MAPCOMBO(sle_x,sle_y)));
                   
        if(++c>50)
            return -1;
    }
    while(blocked);
    
    int dir=0;
    
    if(sle_x==0)    dir=right;
    
    if(sle_y==0)    dir=down;
    
    if(sle_x==240)  dir=left;
    
    if(sle_y==168)  dir=up;
    
    return dir;
}

bool can_side_load(int id)
{
    switch(guysbuf[id].family) //id&0x0FFF)
    {
    case eeTEK:
    case eeLEV:
    case eeAQUA:
    case eeDONGO:
    case eeMANHAN:
    case eeGLEEOK:
    case eeDIG:
    case eeGHOMA:
    case eeLANM:
    case eePATRA:
    case eeGANON:
    case eePROJECTILE:
        return false;
        break;
    }
    
    return true;
}


void side_load_enemies()
{
    if(sle_clk==0)
    {
        sle_cnt = 0;
        int guycnt = 0;
        short s = (currmap<<7)+currscr;
        bool beenhere=false;
        bool reload=true;
        
        load_default_enemies();
        
        for(int i=0; i<6; i++)
            if(visited[i]==s)
                beenhere=true;
                
        if(!beenhere)
        {
            visited[vhead]=s;
            vhead = (vhead+1)%6;
        }
        else if(game->guys[s]==0)
        {
            sle_cnt=0;
            reload=false;
        }
        
        if(reload)
        {
            sle_cnt = game->guys[s];
            
            if(sle_cnt==0)
            {
                while(sle_cnt<10 && tmpscr->enemy[sle_cnt]!=0)
                    ++sle_cnt;
            }
        }
        
        if((get_bit(quest_rules,qr_ALWAYSRET)) || (tmpscr->flags3&fENEMIESRETURN))
        {
            sle_cnt = 0;
            
            while(sle_cnt<10 && tmpscr->enemy[sle_cnt]!=0)
                ++sle_cnt;
        }
        
        for(int i=0; i<sle_cnt; i++)
            ++guycnt;
                
        game->guys[s] = guycnt;
    }
    
    if((++sle_clk+8)%24 == 0)
    {
        int dir = next_side_pos(tmpscr->pattern==pSIDESR);
        
        if(dir==-1 || tooclose(sle_x,sle_y,32))
        {
            return;
        }
        
        int enemy_slot=guys.Count();
        
        while(sle_cnt > 0 && !ok2add(tmpscr->enemy[sle_cnt-1]))
            sle_cnt--;
            
        if(sle_cnt > 0)
        {
            if(addenemy(sle_x,sle_y,tmpscr->enemy[--sle_cnt],0))
                guys.spr(enemy_slot)->dir = dir;
        }
    }
    
    if(sle_cnt<=0)
        loaded_enemies=true;
}

bool is_starting_pos(int i, int x, int y, int t)
{
    // No corner enemies
    if((x==0 || x==240) && (y==0 || y==160))
        return false;
        
    // No enemies in dungeon walls
    if(isdungeon() && (x<32 || x>=224 || y<32 || y>=144))
        return false;
        
    // Too close
    if(tooclose(x,y,40) && t<11)
        return false;
        
    // Can't fly onto it?
    if(isflier(tmpscr->enemy[i])&&
            (flyerblocked(x+8,y+8,spw_floater)||
             (_walkflag(x,y+8,2)&&!get_bit(quest_rules,qr_WALLFLIERS))))
        return false;
        
    // Can't jump onto it?
    if(guysbuf[tmpscr->enemy[i]].family==eeTEK &&
            (COMBOTYPE(x+8,y+8)==cNOJUMPZONE||
             COMBOTYPE(x+8,y+8)==cNOENEMY||
             MAPFLAG(x+8,y+8)==mfNOENEMY||
             MAPCOMBOFLAG(x+8,y+8)==mfNOENEMY))
        return false;
        
    // Other off-limit combos
    if((!isflier(tmpscr->enemy[i])&& guysbuf[tmpscr->enemy[i]].family!=eeTEK &&
            (_walkflag(x,y+8,2) || groundblocked(x+8,y+8))) &&
            guysbuf[tmpscr->enemy[i]].family!=eeZORA)
        return false;
        
    // Don't ever generate enemies on these combos!
    if(COMBOTYPE(x+8,y+8)==cARMOS||COMBOTYPE(x+8,y+8)==cBSGRAVE)
        return false;
        
    //BS Dodongos need at least 2 spaces.
    if((guysbuf[tmpscr->enemy[i]].family==eeDONGO)&&(guysbuf[tmpscr->enemy[i]].misc10==1))
    {
        if(((x<16) ||_walkflag(x-16,y+8, 2))&&
                ((x>224)||_walkflag(x+16,y+8, 2))&&
                ((y<16) ||_walkflag(x,   y-8, 2))&&
                ((y>144)||_walkflag(x,   y+24,2)))
        {
            return false;
        }
    }
    
    return true;
}

bool is_ceiling_pattern(int i)
{
    return (i==pCEILING || i==pCEILINGR);
}

int placeenemy(int i)
{
    std::map<int, int> freeposcache;
    int frees = 0;
    
    for(int y=0; y<176; y+=16)
    {
        for(int x=0; x<256; x+=16)
        {
            if(is_starting_pos(i,x,y,0))
            {
                freeposcache[frees++] = (y&0xF0)+(x>>4);
            }
        }
    }
    
    if(frees > 0)
        return freeposcache[rand()%frees];
        
    return -1;
}

void loadenemies()
{
    if(loaded_enemies)
        return;
        
    // check if it's the dungeon boss and it has been beaten before
    if(tmpscr->enemyflags&efBOSS && game->lvlitems[dlevel]&liBOSS)
    {
        loaded_enemies = true;
        return;
    }
    
    if(tmpscr->pattern==pSIDES || tmpscr->pattern==pSIDESR)
    {
        side_load_enemies();
        return;
    }
    
    loaded_enemies=true;
    
    // do enemies that are always loaded
    load_default_enemies();
    
    // dungeon basements
    
    static byte dngn_enemy_x[4] = {32,96,144,208};
    
    if(currscr>=128)
    {
        if(DMaps[currdmap].flags&dmfCAVES) return;
        
        for(int i=0; i<4; i++)
            addenemy(dngn_enemy_x[i],96,tmpscr->enemy[i]?tmpscr->enemy[i]:(int)eKEESE1,-14-i);
            
        return;
    }
    
    // check if it's been long enough to reload all enemies
    
    int loadcnt = 10;
    short s = (currmap<<7)+currscr;
    bool beenhere = false;
    bool reload = true;
    
    for(int i=0; i<6; i++)
        if(visited[i]==s)
            beenhere = true;
            
    if(!beenhere)
    {
        visited[vhead]=s;
        vhead = (vhead+1)%6;
    }
    else if(game->guys[s]==0)
    {
        loadcnt = 0;
        reload  = false;
    }
    
    if(reload)
    {
        loadcnt = game->guys[s];
        
        if(loadcnt==0)
            loadcnt = 10;
    }
    
    if((get_bit(quest_rules,qr_ALWAYSRET)) || (tmpscr->flags3&fENEMIESRETURN))
        loadcnt = 10;
        
    // load enemies
    
    //if(true)                    // enemies appear at random places
    //{
    //int set=loadside*9;
    int pos=rand()%9;
    int clk=-15,x=0,y=0,fastguys=0;
    int i=0,guycnt=0;
    
    for(; i<loadcnt && tmpscr->enemy[i]>0; i++)             /* i=0 */
    {
        bool placed=false;
        int t=-1;
        
        // First: enemy combo flags
        for(int sy=0; sy<176; sy+=16)
        {
            for(int sx=0; sx<256; sx+=16)
            {
                int cflag = MAPFLAG(sx, sy);
                int cflag2 = MAPCOMBOFLAG(sx, sy);
                
                if(((cflag==mfENEMY0+i)||(cflag2==mfENEMY0+i)) && (!placed))
                {
                    if(!ok2add(tmpscr->enemy[i]))
                        ++loadcnt;
                    else
                    {
                        addenemy(sx,
                                 (is_ceiling_pattern(tmpscr->pattern) && tmpscr->flags7&fSIDEVIEW) ? -(150+50*guycnt) : sy,
                                 (is_ceiling_pattern(tmpscr->pattern) && !(tmpscr->flags7&fSIDEVIEW)) ? 150+50*guycnt : 0,tmpscr->enemy[i],-15);
                                 
                        ++guycnt;
                            
                        placed=true;
                        goto placed_enemy;
                    }
                }
            }
        }
        
        // Next: enemy pattern
        if((tmpscr->pattern==pRANDOM || tmpscr->pattern==pCEILING) && !(tmpscr->flags7&fSIDEVIEW))
        {
            do
            {
                // NES positions
                pos%=9;
                x=stx[loadside][pos];
                y=sty[loadside][pos];
                ++pos;
                ++t;
            }
            while((t< 20) && !is_starting_pos(i,x,y,t));
        }
        
        if(t<0 || t >= 20) // above enemy pattern failed
        {
            // Final chance: find a random position anywhere onscreen
            int randpos = placeenemy(i);
            
            if(randpos>-1)
            {
                x=(randpos&15)<<4;
                y= randpos&0xF0;
            }
            else // All opportunities failed - abort
            {
                --clk;
                continue;
            }
        }
        
        {
            int c=0;
            c=clk;
            
            if(!slowguy(tmpscr->enemy[i]))
                ++fastguys;
            else if(fastguys>0)
                c=-15*(i-fastguys+2);
            else
                c=-15*(i+1);
                
            if(BSZ)
            {
                // Special case for blue leevers
                if(guysbuf[tmpscr->enemy[i]].family==eeLEV && guysbuf[tmpscr->enemy[i]].misc1==1)
                    c=-15*(i+1);
                else
                    c=-15;
            }
            
            if(!ok2add(tmpscr->enemy[i]))
                ++loadcnt;
            else
            {
                addenemy(x,(is_ceiling_pattern(tmpscr->pattern) && tmpscr->flags7&fSIDEVIEW) ? -(150+50*guycnt) : y,
                         (is_ceiling_pattern(tmpscr->pattern) && !(tmpscr->flags7&fSIDEVIEW)) ? 150+50*guycnt : 0,tmpscr->enemy[i],c);
                         
                ++guycnt;
            }
            
            placed=true;
        }                                                     // if(t < 20)
        
placed_enemy:
        
        // I don't like this, but it seems to work...
        static bool foundCarrier;
        
        if(i==0)
            foundCarrier=false;
        
        if(placed)
        {
            if(i==0 && tmpscr->enemyflags&efLEADER)
            {
                int index = guys.idFirst(tmpscr->enemy[i],0xFFF);
                
                if(index!=-1)
                {
                    ((enemy*)guys.spr(index))->leader = true;
                }
            }
            
            if(!foundCarrier && (hasitem)>1)
            {
                int index = guys.idFirst(tmpscr->enemy[i],0xFFF);
                
                if(index!=-1 && (((enemy*)guys.spr(index))->flags&guy_doesntcount)==0)
                {
                    ((enemy*)guys.spr(index))->itemguy = true;
                    foundCarrier=true;
                }
            }
        }
        
        --clk;
    }                                                       // for
    
    game->guys[s] = guycnt;
    //} //if(true)
}

// Setting up special rooms
// Also called when the Letter is used successfully.
void setupscreen()
{
    boughtsomething=false;
    setUpRoom(tmpscr[currscr<128?0:1]);
}

/***  Collision detection & handling  ***/

void check_collisions()
{
    for(int i=0; i<Lwpns.Count(); i++)
    {
        weapon *w = (weapon*)Lwpns.spr(i);
        
        if(!(w->Dead()) && w->id!=wSword && w->id!=wHammer && w->id!=wWand)
        {
            for(int j=0; j<guys.Count(); j++)
            {
                enemy *e = (enemy*)guys.spr(j);
                
                if(e->hit(w))
                {
                    int h = e->takehit(w);
                    
                    if(h)
                    {
                        w->onhit(false);
                    }
                    
                    if(h==2)
                    {
                        break;
                    }
                }
                
                if(w->Dead())
                {
                    break;
                }
            }
            
            if(get_bit(quest_rules,qr_Z3BRANG_HSHOT))
            {
                if(w->id == wBrang || w->id==wHookshot)
                {
                    for(int j=0; j<items.Count(); j++)
                    {
                        if(items.spr(j)->hit(w))
                        {
                            bool priced = ((item*)items.spr(j))->PriceIndex >-1;
                            
                            if((((item*)items.spr(j))->pickup & ipTIMER && ((item*)items.spr(j))->clk2 >= 32)
                                    || (get_bit(quest_rules,qr_BRANGPICKUP) && !priced && !(((item*)items.spr(j))->pickup & ipDUMMY)))
                            {
                                if(w->id == wBrang)
                                {
                                    w->onhit(false);
                                }
                                
                                if(w->dragging==-1)
                                {
                                    w->dead=1;
                                    ((item*)items.spr(j))->clk2=256;
                                    w->dragging=j;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if(w->id == wBrang || w->id == wArrow || w->id==wHookshot)
                {
                    for(int j=0; j<items.Count(); j++)
                    {
                        if(items.spr(j)->hit(w))
                        {
                            bool priced = ((item*)items.spr(j))->PriceIndex >-1;
                            
                            if((((item*)items.spr(j))->pickup & ipTIMER && ((item*)items.spr(j))->clk2 >= 32)
                                    || (get_bit(quest_rules,qr_BRANGPICKUP) && !priced))
                            {
                                if(itemsbuf[items.spr(j)->id].collect_script)
                                {
                                    ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[items.spr(j)->id].collect_script, items.spr(j)->id & 0xFFF);
                                }
                                
                                //getitem(items.spr(j)->id);
                                //items.del(j);
                                Link.checkitems(j);
                                //--j;
                            }
                        }
                    }
                }
            }
        }
    }
}

void dragging_item()
{
    if(get_bit(quest_rules,qr_Z3BRANG_HSHOT))
    {
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = (weapon*)Lwpns.spr(i);
            
            if(w->id == wBrang || w->id==wHookshot)
            {
                if(w->dragging>=0 && w->dragging<items.Count())
                {
                    items.spr(w->dragging)->x=w->x;
                    items.spr(w->dragging)->y=w->y;
                    
                    // Drag the Fairy enemy as well as the Fairy item
                    int id = items.spr(w->dragging)->id;
                    
                    if(itemsbuf[id].family ==itype_fairy && itemsbuf[id].misc3)
                    {
                        movefairy2(w->x,w->y,items.spr(w->dragging)->misc);
                    }
                }
            }
        }
    }
}

int more_carried_items()
{
    int hasmorecarries = 0;
    
    for(int i=0; i<items.Count(); i++)
    {
        if(((item*)items.spr(i))->pickup & ipENEMY)
        {
            hasmorecarries++;
        }
    }
    
    return hasmorecarries;
}

// messy code to do the enemy-carrying-the-item thing
void roaming_item()
{
    if(hasitem<2 || !loaded_enemies)
        return;
    
    // All enemies already dead upon entering a room?
    if(guys.Count()==0)
    {
        return;
    }
    
    // Lost track of the carrier?
    if(guycarryingitem<0 || guycarryingitem>=guys.Count() ||
       !((enemy*)guys.spr(guycarryingitem))->itemguy)
    {
        guycarryingitem=-1;
        for(int j=0; j<guys.Count(); j++)
        {
            if(((enemy*)guys.spr(j))->itemguy)
            {
                guycarryingitem=j;
                break;
            }
        }
    }
    
    if(hasitem&4)
    {
        guycarryingitem = -1;
        
        for(int i=0; i<guys.Count(); i++)
        {
            if(((enemy*)guys.spr(i))->itemguy)
            {
                guycarryingitem = i;
            }
        }
        
        if(guycarryingitem == -1)                                      //This happens when "default enemies" such as
        {
            return;                                               //eSHOOTFBALL are alive but enemies from the list
        }                                                       //are not. Defer to LinkClass::checkspecial().
        
        int Item=tmpscr->item;
        
        hasitem &= ~4;
        
        if(!getmapflag(mITEM) && (tmpscr->hasitem != 0))
        {
            additem(0,0,Item,ipENEMY+ipONETIME+ipBIGRANGE
                    + (((tmpscr->flags3&fHOLDITEM) || (itemsbuf[Item].family==itype_triforcepiece)) ? ipHOLDUP : 0)
                   );
            hasitem |= 2;
        }
        else
        {
            return;
        }
    }
    
    for(int i=0; i<items.Count(); i++)
    {
        if(((item*)items.spr(i))->pickup&ipENEMY)
        {
            if(get_bit(quest_rules,qr_HIDECARRIEDITEMS))
            {
                items.spr(i)->x = -128; // Awfully inelegant, innit?
                items.spr(i)->y = -128;
            }
            else if(guycarryingitem>=0 && guycarryingitem<guys.Count())
            {
                items.spr(i)->x = guys.spr(guycarryingitem)->x;
                items.spr(i)->y = guys.spr(guycarryingitem)->y - 2;
            }
        }
    }
}

const char *old_guy_string[OLDMAXGUYS] =
{
    "(None)","Abei","Ama","Merchant","Moblin","Fire","Fairy","Goriya","Zelda","Abei 2","Empty","","","","","","","","","",
    // 020
    "Octorok (L1, Slow)","Octorok (L2, Slow)","Octorok (L1, Fast)","Octorok (L2, Fast)","Tektite (L1)",
    // 025
    "Tektite (L2)","Leever (L1)","Leever (L2)","Moblin (L1)","Moblin (L2)",
    // 030
    "Lynel (L1)","Lynel (L2)","Peahat (L1)","Zora","Rock",
    // 035
    "Ghini (L1, Normal)","Ghini (L1, Phantom)","Armos","Keese (CSet 7)","Keese (CSet 8)",
    // 040
    "Keese (CSet 9)","Stalfos (L1)","Gel (L1, Normal)","Zol (L1, Normal)","Rope (L1)",
    // 045
    "Goriya (L1)","Goriya (L2)","Trap (4-Way)","Wall Master","Darknut (L1)",
    // 050
    "Darknut (L2)","Bubble (Sword, Temporary Disabling)","Vire (Normal)","Like Like","Gibdo",
    // 055
    "Pols Voice (Arrow)","Wizzrobe (Teleporting)","Wizzrobe (Floating)","Aquamentus (Facing Left)","Moldorm",
    // 060
    "Dodongo","Manhandla (L1)","Gleeok (1 Head)","Gleeok (2 Heads)","Gleeok (3 Heads)",
    // 065
    "Gleeok (4 Heads)","Digdogger (1 Kid)","Digdogger (3 Kids)","Digdogger Kid (1)","Digdogger Kid (2)",
    // 070
    "Digdogger Kid (3)","Digdogger Kid (4)","Gohma (L1)","Gohma (L2)","Lanmola (L1)",
    // 075
    "Lanmola (L2)","Patra (L1, Big Circle)","Patra (L1, Oval)","Ganon","Stalfos (L2)",
    // 080
    "Rope (L2)","Bubble (Sword, Permanent Disabling)","Bubble (Sword, Re-enabling)","Shooter (Fireball)","Item Fairy ",
    // 085
    "Fire","Octorok (Magic)", "Darknut (Death Knight)", "Gel (L1, Tribble)", "Zol (L1, Tribble)",
    // 090
    "Keese (Tribble)", "Vire (Tribble)", "Darknut (Splitting)", "Aquamentus (Facing Right)", "Manhandla (L2)",
    // 095
    "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Wizzrobe (Fire)",
    // 100
    "Wizzrobe (Wind)", "Ceiling Master ", "Floor Master ", "Patra (BS Zelda)", "Patra (L2)",
    // 105
    "Patra (L3)", "Bat", "Wizzrobe (Bat)", "Wizzrobe (Bat 2) ", "Gleeok (Fire, 1 Head)",
    // 110
    "Gleeok (Fire, 2 Heads)",  "Gleeok (Fire, 3 Heads)","Gleeok (Fire, 4 Heads)", "Wizzrobe (Mirror)", "Dodongo (BS Zelda)",
    // 115
    "Dodongo (Fire) ","Trigger", "Bubble (Item, Temporary Disabling)", "Bubble (Item, Permanent Disabling)", "Bubble (Item, Re-enabling)",
    // 120
    "Stalfos (L3)", "Gohma (L3)", "Gohma (L4)", "NPC 1 (Standing) ", "NPC 2 (Standing) ",
    // 125
    "NPC 3 (Standing) ", "NPC 4 (Standing) ", "NPC 5 (Standing) ", "NPC 6 (Standing) ", "NPC 1 (Walking) ",
    // 130
    "NPC 2 (Walking) ", "NPC 3 (Walking) ", "NPC 4 (Walking) ", "NPC 5 (Walking) ", "NPC 6 (Walking) ",
    // 135
    "Boulder", "Goriya (L3)", "Leever (L3)", "Octorok (L3, Slow)", "Octorok (L3, Fast)",
    // 140
    "Octorok (L4, Slow)", "Octorok (L4, Fast)", "Trap (8-Way) ", "Trap (Diagonal) ", "Trap (/, Constant) ",
    // 145
    "Trap (/, Line of Sight) ", "Trap (\\, Constant) ", "Trap (\\, Line of Sight) ", "Trap (CW, Constant) ", "Trap (CW, Line of Sight) ",
    // 150
    "Trap (CCW, Constant) ", "Trap (CCW, Line of Sight) ", "Wizzrobe (Summoner)", "Wizzrobe (Ice) ", "Shooter (Magic)",
    // 155
    "Shooter (Rock)", "Shooter (Spear)", "Shooter (Sword)", "Shooter (Fire)", "Shooter (Fire 2)",
    // 160
    "Bombchu", "Gel (L2, Normal)", "Zol (L2, Normal)", "Gel (L2, Tribble)", "Zol (L2, Tribble)",
    // 165
    "Tektite (L3) ", "Spinning Tile (Combo)", "Spinning Tile (Enemy Sprite)", "Lynel (L3) ", "Peahat (L2) ",
    // 170
    "Pols Voice (Magic) ", "Pols Voice (Whistle) ", "Darknut (Mirror) ", "Ghini (L2, Fire) ", "Ghini (L2, Magic) ",
    // 175
    "Grappler Bug (HP) ", "Grappler Bug (MP) "
};

/*** end of guys.cc ***/

