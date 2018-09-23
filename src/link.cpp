//--------------------------------------------------------
//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  link.cc
//
//  Link's class: LinkClass
//  Handles a lot of game play stuff as well as Link's
//  movement, attacking, etc.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <set>
#include <stdio.h>

#include "link.h"
#include "guys.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "decorations.h"
#include "gamedata.h"
#include "zc_custom.h"
#include "title.h"
#include "ffscript.h"
extern FFScript FFCore;
extern LinkClass Link;
#include "mem_debug.h"
#include "zscriptversion.h"

using std::set;

extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;
//extern bool draw_screen_clip_rect_show_link;
extern bool global_wait;

int link_count = -1;
int link_animation_speed = 1; //lower is faster animation
int z3step = 2;
bool did_scripta=false;
bool did_scriptb=false;
bool did_scriptl=false;
byte lshift = 0;
int dowpn = -1;
int directItem = -1; //Is set if Link is currently using an item directly
int directItemA = -1;
int directItemB = -1;
int directWpn = -1;
int whistleitem=-1;
extern word g_doscript;

void playLevelMusic();

const byte lsteps[8] = { 1, 1, 2, 1, 1, 2, 1, 1 };


static inline bool isSideview()
{
    return (((tmpscr->flags7&fSIDEVIEW)!=0 || DMaps[currdmap].sideview != 0) && !ignoreSideview); //DMap Enable Sideview on All Screens -Z //2.54 Alpha 27
}

int LinkClass::DrunkClock()
{
    return drunkclk;
}
void LinkClass::setDrunkClock(int newdrunkclk)
{
    drunkclk=newdrunkclk;
}

int LinkClass::StunClock()
{
    return link_is_stunned;
}
void LinkClass::setStunClock(int v)
{
    link_is_stunned=v;
}

LinkClass::LinkClass() : sprite()
{
    init();
}

//2.6

//Stop the subscreen from falling. -Z

bool LinkClass::stopSubscreenFalling(){
	return preventsubscreenfalling;
}

void LinkClass::stopSubscreenFalling(bool v){
	preventsubscreenfalling = v;
}


//Set the button items by brute force

void LinkClass::setAButtonItem(int itmslot){
	game->awpn = itmslot;
}

void LinkClass::setBButtonItem(int itmslot){
	game->bwpn = itmslot;
}

bool LinkClass::getCanLinkFlicker(){
	return flickerorflash; //enable or disable flicker or flash
}
void LinkClass::setCanLinkFlicker(bool v){
	flickerorflash = v;
}

void LinkClass::ClearhitLinkUIDs()
{ 		//Why the flidd doesn't this work?! Clearing this to 0 in a way that doesn't demolish script access is impossible. -Z
		//All I want, is to clear it at the end of a frame, or at the start of a frame, so that if it changes to non-0
		//that a script can read it before Waitdraw(). --I want it to go stale at the end of a frame.
		//I suppose I will need to do this inside the script engine, and not the game_loop() ? -Z
		//THis started out as a simple clear to 0 of lastHitBy[n], but that did not work:
		//I added the second element to this, so that I could store the frame on which the hit is recorded, and 
		//clear it on the next frame, but that had the SAME outcome.
		//Where and how can I clear a value at the end of every frame, so that:
		// 1. If set by internal mecanics, it has its value that you can read by script, before waitdraw--this part works at present.
		// 2. FFCs can read it before Waitframe. --same.
		// 3. After Waitframe(), it is wiped by the ZC Engine to 0. --I cannot get this to happen without breaking 1 and 2. 
	for ( int q = 0; q < NUM_HIT_TYPES_USED_LINK; q++ ) 	
	{
		/*
		if ( lastHitBy[q][1] == (frame-1) ) //Verify if this is needed at all now. 
		{
			//Z_scripterrlog("frame is: %d\n", frame);
			//Z_scripterrlog("Link->HitBy frame is: %d\n", lastHitBy[q][1]);
			lastHitBy[q][0] = 0;
		}
		*/
		lastHitBy[q][0] = 0;
	}
}

void LinkClass::sethitLinkUID(int type, int screen_index)
{
	lastHitBy[type][0] = screen_index;
	//lastHitBy[type][1] = frame;
	/* Let's figure out how to clear this...
	if ( global_wait ) lastHitBy[type] = screen_index;
	else lastHitBy[type] = 0;
	//No, we clear it in Zelda.cpp, with this:
	    if(global_wait)
	    {
		ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_GAME);
		global_wait=false;
	    }
	    
	    
	    draw_screen(tmpscr);
	    
	    //clear Link's last hits 
	    //for ( int q = 0; q < 4; q++ ) Link.sethitLinkUID(q, 0);
	    
	*/
	
}

int LinkClass::gethitLinkUID(int type)
{
	return lastHitBy[type][0];
}

void LinkClass::set_defence(int type, int v)
{
	defence[type] = v;
}

int LinkClass::get_defence(int type)
{
	return defence[type];
}


//Set Link;s hurt sfx
void LinkClass::setHurtSFX(int sfx){
	hurtsfx = sfx;
}	
int LinkClass::getHurtSFX() {
	return hurtsfx;
}

bool  LinkClass::getDiagMove()
{
    return diagonalMovement;
}
void LinkClass::setDiagMove(bool newdiag)
{
    diagonalMovement=newdiag;
}
bool  LinkClass::getBigHitbox()
{
    return bigHitbox;
}
void LinkClass::setBigHitbox(bool newbighitbox)
{
    bigHitbox=newbighitbox;
}


//void LinkClass::linkstep() { lstep = lstep<(BSZ?27:11) ? lstep+1 : 0; }
void LinkClass::linkstep()
{
    lstep = lstep<((zinit.linkanimationstyle==las_bszelda)?27:11) ? lstep+1 : 0;
}

bool is_moving()
{
    return DrunkUp()||DrunkDown()||DrunkLeft()||DrunkRight();
}

// called by ALLOFF()
void LinkClass::resetflags(bool all)
{
    refilling=REFILL_NONE;
    inwallm=false;
    inlikelike=blowcnt=whirlwind=specialcave=hclk=fairyclk=refill_why=didstuff=0;
    
    if(swordclk>0 || all)
        swordclk=0;
        
    if(itemclk>0 || all)
        itemclk=0;
        
    if(all)
    {
        NayrusLoveShieldClk=0;
        
        if(nayruitem != -1)
        {
            stop_sfx(itemsbuf[nayruitem].usesound);
            stop_sfx(itemsbuf[nayruitem].usesound+1);
        }
        
        nayruitem = -1;
        hoverclk=jumping=0;
    }
    
    hopclk=0;
    hopdir=-1;
    attackclk=0;
    stomping=false;
    reset_swordcharge();
    diveclk=drownclk=0;
    action=none; FFCore.setLinkAction(none);
    conveyor_flags=0;
    magiccastclk=0;
    magicitem=-1;
}

//Can use this for Link->Stun. -Z
void LinkClass::Freeze()
{
    if (action != inwind)
    {
        action=freeze; FFCore.setLinkAction(freeze);
        // also cancel Link's attack
        attackclk = 0;
    }
}
void LinkClass::unfreeze()
{
    if(action==freeze && fairyclk<1) { action=none; FFCore.setLinkAction(none); }
}

void LinkClass::Drown()
{
    // Link should never drown if the ladder is out
    if(ladderx+laddery)
        return;
        
    action=drowning; FFCore.setLinkAction(drowning);
    attackclk=0;
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    drownclk=64;
    z=fall=0;
}

void LinkClass::finishedmsg()
{
    //these are to cancel out any keys that Link may
    //be pressing so he doesn't attack at the end of
    //a message if he was scrolling through it quickly.
    rAbtn();
    rBbtn();
    unfreeze();
    
    if(action == landhold1 ||
            action == landhold2 ||
            action == waterhold1 ||
            action == waterhold2)
    {
        holdclk = 1;
    }
}
void LinkClass::setEaten(int i)
{
    inlikelike=i;
}
int LinkClass::getEaten()
{
    return inlikelike;
}
fix  LinkClass::getX()
{
    return x;
}
fix  LinkClass::getY()
{
    return y;
}
fix  LinkClass::getZ()
{
    return z;
}
fix  LinkClass::getFall()
{
    return fall;
}
fix  LinkClass::getXOfs()
{
    return xofs;
}
fix  LinkClass::getYOfs()
{
    return yofs;
}
void LinkClass::setXOfs(int newxofs)
{
    xofs=newxofs;
}
void LinkClass::setYOfs(int newyofs)
{
    yofs=newyofs;
}
int  LinkClass::getHXOfs()
{
    return hxofs;
}
int  LinkClass::getHYOfs()
{
    return hyofs;
}
int  LinkClass::getHXSz()
{
    return hxsz;
}
int  LinkClass::getHYSz()
{
    return hysz;
}
fix  LinkClass::getClimbCoverX()
{
    return climb_cover_x;
}
fix  LinkClass::getClimbCoverY()
{
    return climb_cover_y;
}
int  LinkClass::getLadderX()
{
    return ladderx;
}
int  LinkClass::getLadderY()
{
    return laddery;
}

void LinkClass::setX(int new_x)
{
    fix dx=new_x-x;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=dx;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=dx;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->x+=dx;
        }
	}
    
    x=new_x;
    
    // A kludge
    if(!diagonalMovement && dir<=down)
        is_on_conveyor=true;
}

void LinkClass::setY(int new_y)
{
    fix dy=new_y-y;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=dy;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=dy;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->y+=dy;
        }
	}
    
    y=new_y;
    
    // A kludge
    if(!diagonalMovement && dir>=left)
        is_on_conveyor=true;
}

void LinkClass::setZ(int new_z)
{
    if(isSideview())
        return;
        
    if(z==0 && new_z > 0)
    {
        switch(action)
        {
        case swimming:
	{
            diveclk=0;
            action=walking; FFCore.setLinkAction(walking);
            break;
	}
            
        case waterhold1:
	{
            action=landhold1; FFCore.setLinkAction(landhold1);
            break;
	}
            
        case waterhold2:
	{
            action=landhold2; FFCore.setLinkAction(landhold2);
            break;
	}
            
        default:
            if(charging)
            {
                reset_swordcharge();
                attackclk=0;
            }
            
            break;
        }
    }
    
    z=(new_z>0 ? new_z : 0);
}
void LinkClass::setFall(fix new_fall)
{
    fall=new_fall;
    jumping=-1;
}
void LinkClass::setClimbCoverX(int new_x)
{
    climb_cover_x=new_x;
}
void LinkClass::setClimbCoverY(int new_y)
{
    climb_cover_y=new_y;
}
int  LinkClass::getLStep()
{
    return lstep;
}
int  LinkClass::getCharging()
{
    return charging;
}
bool LinkClass::isCharged()
{
    return spins>0;
}
int  LinkClass::getAttackClk()
{
    return attackclk;
}
void  LinkClass::setAttackClk(int new_clk)
{
    attackclk=new_clk;
}
void LinkClass::setCharging(int new_charging)
{
    charging=new_charging;
}
int  LinkClass::getSwordClk()
{
    return swordclk;
}
int  LinkClass::getItemClk()
{
    return itemclk;
}
void LinkClass::setSwordClk(int newclk)
{
    swordclk=newclk;
}
void LinkClass::setItemClk(int newclk)
{
    itemclk=newclk;
}
fix  LinkClass::getModifiedX()
{
    fix tempx=x;
    
    if(screenscrolling&&(dir==left))
    {
        tempx=tempx+256;
    }
    
    return tempx;
}

fix  LinkClass::getModifiedY()
{
    fix tempy=y;
    
    if(screenscrolling&&(dir==up))
    {
        tempy=tempy+176;
    }
    
    return tempy;
}

int  LinkClass::getDir()
{
    return dir;
}
void LinkClass::setDir(int newdir)
{
    dir=newdir;
    reset_hookshot();
}
int  LinkClass::getHitDir()
{
    return hitdir;
}
void LinkClass::setHitDir(int newdir)
{
    hitdir = newdir;
}
int  LinkClass::getClk()
{
    return clk;
}
int  LinkClass::getPushing()
{
    return pushing;
}
void LinkClass::Catch()
{
    if(!inwallm && (action==none || action==walking))
    {
        action=attacking; FFCore.setLinkAction(attacking);
        attackclk=0;
        attack=wCatching;
    }
}

bool LinkClass::getClock()
{
    return superman;
}
void LinkClass::setClock(bool state)
{
    superman=state;
}
int  LinkClass::getAction() // Used by ZScript
{
    if(spins > 0)
        return isspinning;
    else if(charging > 0)
        return ischarging;
    else if(diveclk > 0)
        return isdiving;
    //else if (pushing > 0) return ispushing; //Needs a QR? -Z or make it an instruction as Link->Pushing? Probably better, as that has a clk?? 
        
    return action;
}

int  LinkClass::getAction2() // Used by ZScript new FFCore.actions
{
    if(spins > 0)
        return isspinning;
    else if(charging > 0)
        return ischarging;
    else if(diveclk > 0)
        return isdiving;
    //else if (pushing > 0) return ispushing; //Needs a QR? -Z or make it an instruction as Link->Pushing? Probably better, as that has a clk?? 
        
    return -1;
}

void LinkClass::setAction(actiontype new_action) // Used by ZScript
{
    if(new_action==dying || new_action==won || new_action==scrolling ||
       new_action==inwind || new_action==rafting || new_action==ischarging ||
       new_action==hopping)
        return; // Can't use these actions.
    
    if(magicitem>-1 && itemsbuf[magicitem].family==itype_faroreswind)
    {
        // Using Farore's Wind
        if(magiccastclk<96)
        {
            // Not cast yet; cancel it
            magicitem=-1;
            magiccastclk=0;
        }
        else
            // Already activated; don't do anything
            return;
    }
    
    if(action==inwind) // Remove from whirlwind
    {
        xofs=0;
        whirlwind=0;
        lstep=0;
        if ( dontdraw < 2 ) { dontdraw=0; }
    }
    else if(action==freeze) // Might be in enemy wind
    {
        sprite* wind=0;
        bool foundWind=false;
        for(int i=0; i<Ewpns.Count(); i++)
        {
            wind=Ewpns.spr(i);
            if(wind->id==ewWind && wind->misc==999)
            {
                foundWind=true;
                break;
            }
        }
        
        if(foundWind)
        {
            xofs=0;
            if ( dontdraw < 2 ) { dontdraw=false; }
            wind->misc=-1;
            x=wind->x;
            y=wind->y;
        }
    }
    
    if(new_action != attacking)
    {
        attackclk=0;
        
        if(attack==wHookshot)
            reset_hookshot();
    }
    if(new_action != isspinning)
    {
        charging = 0;
        spins = 0;
    }
    
    switch(new_action)
    {
    case isspinning:
        if(attack==wSword)
        {
            attackclk = SWORDCHARGEFRAME+1;
            charging = 0;
            
            if(spins==0)
                spins = 5;
        }
        return;
        
    case isdiving:
        if(action==swimming && diveclk==0)
            diveclk = 80; // Who cares about qr_NODIVING? It's the questmaker's business.
        return;
        
    case drowning:
	//I would add a sanity check to see if Link is in water, but I *KNOW* that quests have used this 
	// INTENTIONALLY while Link is on Land, as a blink-out effect. :( -Z
        if(!drownclk)
            Drown();
            
        break;
        
    case gothit:
    case swimhit:
        if(!hclk)
            hclk=48;
            
        break;
        
    case landhold1:
    case landhold2:
    case waterhold1:
    case waterhold2:
        if(!holdclk)
            holdclk=130;
            
        attack=none;
        break;
        
    case attacking:
        attack=none;
        break;
        
    default:
        break;
    }
    
    action=new_action; FFCore.setLinkAction(new_action);
}

void LinkClass::setHeldItem(int newitem)
{
    holditem=newitem;
}
int LinkClass::getHeldItem()
{
    return holditem;
}
bool LinkClass::isDiving()
{
    return (diveclk>30);
}
bool LinkClass::isSwimming()
{
    return ((action==swimming)||(action==swimhit)||
            (action==waterhold1)||(action==waterhold2)||
            (hopclk==0xFF));
}

void LinkClass::setDontDraw(byte new_dontdraw)
{
    dontdraw=new_dontdraw;
}

byte LinkClass::getDontDraw()
{
    return dontdraw;
}

void LinkClass::setHClk(int newhclk)
{
    hclk=newhclk;
}

int LinkClass::getHClk()
{
    return hclk;
}

int LinkClass::getSpecialCave()
{
    return specialcave;    // used only by maps.cpp
}

void LinkClass::init()
{
    setMonochrome(false);
    if ( dontdraw < 2 ) {  dontdraw = 0; } //scripted dontdraw == 2, normal == 1, draw link == 0
    hookshot_used=false;
    hookshot_frozen=false;
    dir = up;
    shiftdir = -1;
    holddir = -1;
    landswim = 0;
    sdir = up;
    ilswim=true;
    walkable=false;
    
    if(get_bit(quest_rules,qr_NOARRIVALPOINT))
    {
        x=tmpscr->warpreturnx[0];
        y=tmpscr->warpreturny[0];
    }
    else
    {
        x=tmpscr->warparrivalx;
        y=tmpscr->warparrivaly;
    }
    
    z=fall=0;
    hzsz = 12; // So that flying peahats can still hit him.
    
    if(x==0)   dir=right;
    
    if(x==240) dir=left;
    
    if(y==0)   dir=down;
    
    if(y==160) dir=up;
    
    lstep=0;
    skipstep=0;
    autostep=false;
    attackclk=holdclk=hoverclk=jumping=0;
    attack=wNone;
    attackid=-1;
    action=none; FFCore.setLinkAction(none); tempaction=none;
    xofs=0;
    yofs=playing_field_offset;
    cs=6;
    pushing=fairyclk=0;
    id=0;
    inlikelike=0;
    superman=inwallm=false;
    scriptcoldet=1;
    blowcnt=whirlwind=specialcave=0;
    hopclk=diveclk=0;
    hopdir=-1;
    conveyor_flags=0;
    drunkclk=0;
    link_is_stunned = 0;
    drawstyle=3;
    ffwarp = false;
    stepoutindex=stepoutwr=stepoutdmap=stepoutscr=0;
    stepnext=stepsecret=-1;
    ffpit = false;
    entry_x=x;
    entry_y=y;
    falling_oldy = y;
    magiccastclk=0;
    magicitem = nayruitem = -1;
	last_lens_id = 0;
    
    for(int i=0; i<32; i++) miscellaneous[i] = 0;
    
    bigHitbox=get_bit(quest_rules, qr_LTTPCOLLISION);
    diagonalMovement=get_bit(quest_rules,qr_LTTPWALK);
    
    //2.6
	preventsubscreenfalling = false;  //-Z
	hurtsfx = 19; //getHurtSFX(); //Set the default sound. 
	flickerorflash = true; //flicker or flash unless disabled externally.
	walkspeed = 0; //not used, yet. -Z
	for ( int q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][0] = 0; 
	for ( int q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][1] = 0; 
	for ( int q = 0; q < wMax; q++ ) defence[q] = 0; //we will need to have a Link section in the quest load/save code! -Z
}

void LinkClass::draw_under(BITMAP* dest)
{
    int c_raft=current_item_id(itype_raft);
    int c_ladder=current_item_id(itype_ladder);
    
    if(action==rafting && c_raft >-1)
    {
        if(((dir==left) || (dir==right)) && (get_bit(quest_rules,qr_RLFIX)))
        {
            overtile16(dest, itemsbuf[c_raft].tile, x, y+playing_field_offset+4,
                       itemsbuf[c_raft].csets&15, rotate_value((itemsbuf[c_raft].misc>>2)&3)^3);
        }
        else
        {
            overtile16(dest, itemsbuf[c_raft].tile, x, y+playing_field_offset+4,
                       itemsbuf[c_raft].csets&15, (itemsbuf[c_raft].misc>>2)&3);
        }
    }
    
    if(ladderx+laddery && c_ladder >-1)
    {
        if((ladderdir>=left) && (get_bit(quest_rules,qr_RLFIX)))
        {
            overtile16(dest, itemsbuf[c_ladder].tile, ladderx, laddery+playing_field_offset,
                       itemsbuf[c_ladder].csets&15, rotate_value((itemsbuf[iRaft].misc>>2)&3)^3);
        }
        else
        {
            overtile16(dest, itemsbuf[c_ladder].tile, ladderx, laddery+playing_field_offset,
                       itemsbuf[c_ladder].csets&15, (itemsbuf[c_ladder].misc>>2)&3);
        }
    }
}

void LinkClass::drawshadow(BITMAP* dest, bool translucent)
{
    int tempy=yofs;
    yofs+=8;
    shadowtile = wpnsbuf[iwShadow].newtile;
    sprite::drawshadow(dest,translucent);
    yofs=tempy;
}

// The Stone of Agony reacts to these flags.
bool LinkClass::agonyflag(int flag)
{
    switch(flag)
    {
    case mfWHISTLE:
    case mfBCANDLE:
    case mfARROW:
    case mfBOMB:
    case mfSBOMB:
    case mfBRANG:
    case mfMBRANG:
    case mfFBRANG:
    case mfSARROW:
    case mfGARROW:
    case mfRCANDLE:
    case mfWANDFIRE:
    case mfDINSFIRE:
    case mfWANDMAGIC:
    case mfREFMAGIC:
    case mfREFFIREBALL:
    case mfSWORD:
    case mfWSWORD:
    case mfMSWORD:
    case mfXSWORD:
    case mfSWORDBEAM:
    case mfWSWORDBEAM:
    case mfMSWORDBEAM:
    case mfXSWORDBEAM:
    case mfHOOKSHOT:
    case mfWAND:
    case mfHAMMER:
    case mfSTRIKE:
        return true;
    }
    
    return false;
}


// Find the attack power of the current melee weapon.
// The Whimsical Ring is applied on a target-by-target basis.
int LinkClass::weaponattackpower()
{
    int power = directWpn>-1 ? itemsbuf[directWpn].power : (current_item_power(attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword));
    
    // Multiply it by the power of the spin attack/quake hammer, if applicable.
    power *= (spins>0 ? itemsbuf[current_item_id(attack==wHammer ? itype_quakescroll : (spins>5 || current_item_id(itype_spinscroll) < 0) ? itype_spinscroll2 : itype_spinscroll)].power : 1);
    return power;
}

// Must only be called once per frame!
void LinkClass::positionSword(weapon *w, int itemid)
{
	//if ( w->ScriptGenerated ) return; //t/b/a for script-generated swords.
	//if ( itemsbuf[itemid].ScriptGenerated ) return; //t/b/a for script-generated swords.
    itemid=vbound(itemid, 0, MAXITEMS-1);
    // Place a sword weapon at the right spot.
    int wy=1;
    int wx=1;
    int f=0,t,cs2;
    
    t = w->o_tile;
    cs2 = w->o_cset;
    slashxofs=0;
    slashyofs=0;
    
    switch(dir)
    {
    case up:
        wx=-1;
        wy=-12;
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & ITEM_FLAG4 && charging==0)
        {
            if(attackclk>10) //extended stab
            {
                slashyofs-=3;
                wy-=2;
            }
            
            if(attackclk>=14) //retracting stab
            {
                slashyofs+=3;
                wy+=2;
            }
        }
        else
        {
            if(attackclk==SWORDCHARGEFRAME)
            {
                wy+=4;
            }
            else if(attackclk==13)
            {
                wy+=4;
            }
            else if(attackclk==14)
            {
                wy+=8;
            }
        }
        
        break;
        
    case down:
        f=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2;
        wy=11;
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & ITEM_FLAG4 && charging==0)
        {
            if(attackclk>10) //extended stab
            {
                slashyofs+=3;
                wy+=2;
            }
            
            if(attackclk>=14) //retracting stab
            {
                slashyofs-=3;
                wy-=2;
            }
        }
        else
        {
            if(attackclk==SWORDCHARGEFRAME)
            {
                wy-=2;
            }
            else if(attackclk==13)
            {
                wy-=4;
            }
            else if(attackclk==14)
            {
                wy-=8;
            }
        }
        
        break;
        
    case left:
        f=1;
        wx=-11;
        ++t;
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & ITEM_FLAG4 && charging==0)
        {
            if(attackclk>10)  //extended stab
            {
                slashxofs-=4;
                wx-=7;
            }
            
            if(attackclk>=14) //retracting stab
            {
                slashxofs+=3;
                wx+=7;
            }
        }
        else
        {
            if(attackclk==SWORDCHARGEFRAME)
            {
                wx+=2;
            }
            else if(attackclk==13)
            {
                wx+=4;
            }
            else if(attackclk==14)
            {
                wx+=8;
            }
        }
        
        break;
        
    case right:
        wx=11;
        ++t;
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & ITEM_FLAG4 && charging==0)
        {
            if(attackclk>10) //extended stab
            {
                slashxofs+=4;
                wx+=7;
            }
            
            if(attackclk>=14) //retracting stab
            {
                slashxofs-=3;
                wx-=7;
            }
        }
        else
        {
            if(attackclk==SWORDCHARGEFRAME)
            {
                wx-=2;
            }
            else if(attackclk==13)
            {
                wx-=4;
            }
            else if(attackclk==14)
            {
                wx-=8;
            }
        }
        
        break;
    }
    
    if(game->get_canslash() && itemsbuf[itemid].flags & ITEM_FLAG4 && attackclk<11)
    {
        int wpn2=itemsbuf[itemid].wpn2;
        wpn2=vbound(wpn2, 0, MAXWPNS);
        
        //slashing tiles
        switch(dir)
        {
        case up:
            wx=15;
            wy=-3;
            ++t;
            f=0;                                     //starts pointing right
            
            if(attackclk>=7)
            {
                wy-=9;
                wx-=3;
                t = wpnsbuf[wpn2].newtile;
                cs2 = wpnsbuf[wpn2].csets&15;
                f=0;
            }
            
            break;
            
        case down:
            wx=-13;
            wy=-1;
            ++t;
            f=1;                                     //starts pointing left
            
            if(attackclk>=7)
            {
                wy+=15;
                wx+=2;
                t = wpnsbuf[wpn2].newtile;
                cs2 = wpnsbuf[wpn2].csets&15;
                ++t;
                f=0;
            }
            
            break;
            
        case left:
            wx=3;
            wy=-15;
            --t;
            f=0;                                     //starts pointing up
            
            if(attackclk>=7)
            {
                wx-=15;
                wy+=3;
                slashxofs-=1;
                t = wpnsbuf[wpn2].newtile;
                cs2 = wpnsbuf[wpn2].csets&15;
                t+=2;
                f=0;
            }
            
            break;
            
        case right:
            --t;
            
            if(spins>0 || get_bit(quest_rules, qr_SLASHFLIPFIX))
            {
                wx=1;
                wy=13;
                f=2;
            }
            else
            {
                wx=3;
                wy=-15;
                f=0;
            }
            
            if(attackclk>=7)
            {
                wx+=15;
                slashxofs+=1;
                t = wpnsbuf[wpn2].newtile;
                cs2 = wpnsbuf[wpn2].csets&15;
                
                if(spins>0 || get_bit(quest_rules, qr_SLASHFLIPFIX))
                {
                    wx-=1;
                    wy-=2;
                }
                else
                {
                    t+=3;
                    f=0;
                    wy+=3;
                }
            }
            
            break;
        }
    }
    
    int itemid2 = current_item_id(itype_chargering);
    
    if(charging>(itemid2>=0 ? itemsbuf[itemid2].misc1 : 64))
    {
        cs2=(BSZ ? (frame&3)+6 : ((frame>>2)&1)+7);
    }
    
    /*if(BSZ || ((isdungeon() && currscr<128) && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)))
    {
      wy+=2;
    }*/
    w->x = x+wx;
    w->y = y+wy-(54-(yofs+slashyofs));
    w->z = (z+zofs);
    w->tile = t;
    w->flip = f;
    w->power = weaponattackpower();
    w->dir = dir;
}

void LinkClass::draw(BITMAP* dest)
{
    /*{
    	char buf[36];
    	//sprintf(buf,"%d %d %d %d %d %d %d",dir, action, attack, attackclk, charging, spins, tapping);
    	textout_shadowed_ex(framebuf,font, buf, 2,72,WHITE,BLACK,-1);
    }*/
    int oxofs, oyofs;
    bool shieldModify = false;
    
    bool invisible=(dontdraw>0) || (tmpscr->flags3&fINVISLINK);
    
    if(action==dying)
    {
        if(!invisible)
        {
            sprite::draw(dest);
        }
        
        return;
    }
    
    bool useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);
    
    oxofs=xofs;
    oyofs=yofs;
    
    if(!invisible)
        yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
        
    // Stone of Agony
    bool agony=false;
    int agonyid = current_item_id(itype_agony);
    
    if(invisible)
        goto attack;
        
    if(agonyid>-1)
    {
        int power=itemsbuf[agonyid].power;
        int left=static_cast<int>(x+8-power)&0xF0; // Check top-left pixel of each tile
        int right=(static_cast<int>(x+8+power)&0xF0)+16;
        int top=static_cast<int>(y+(bigHitbox ? 8 : 12)-power)&0xF0;
        int bottom=(static_cast<int>(y+(bigHitbox ? 8 : 12)+power)&0xF0)+16;
        
        for(int x=left; x<right; x+=16)
        {
            for(int y=top; y<bottom; y+=16)
            {
                if(agonyflag(MAPFLAG(x, y)) || agonyflag(MAPCOMBOFLAG(x, y)))
                {
                    agony=true;
                    x=right; // Break out of outer loop
                    break;
                }
            }
        }
    }
    
    cs = 6;
    
     if(!get_bit(quest_rules,qr_LINKFLICKER))
    {
        if(superman && getCanLinkFlicker())
        {
            cs += (((~frame)>>1)&3);
        }
        else if(hclk&&(NayrusLoveShieldClk<=0) && getCanLinkFlicker())
        {
            cs += ((hclk>>1)&3);
        }
    }
    
attack:

    if(attackclk || action==attacking)
    {
        /* Spaghetti code constants!
        * - Link.attack contains a weapon type...
        * - which must be converted to an itype...
        * - which must be converted to an item ID...
        * - which is used to acquire a wpn ID! Aack!
        */
        int itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
        int itemid = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
        itemid=vbound(itemid, 0, MAXITEMS-1);
        // if ( itemsbuf[itemid].ScriptGenerated ) return; //t/b/a for script-generated swords.
        if(attackclk>4||(attack==wSword&&game->get_canslash()))
        {
            if((attack==wSword || attack==wWand || ((attack==wFire || attack==wCByrna) && itemsbuf[itemid].wpn)) && wpnsbuf[itemsbuf[itemid].wpn].newtile)
            {
                // Create a sword weapon at the right spot.
                weapon *w=NULL;
                bool found = false;
                
                // Look for pre-existing sword
                for(int i=0; i<Lwpns.Count(); i++)
                {
                    w = (weapon*)Lwpns.spr(i);
                    
                    if(w->id == (attack==wSword ? wSword : wWand))
                    {
                        found = true;
                        break;
                    }
                }
                
                if(!found)  // Create one if sword nonexistant
                {
			
		    //check magic cost for sword -Z	
			
		    //if(!checkmagiccost(itemid)) return;
		    //else paymagiccost(itemid);
			//placing this here causes the sword to use magic more than one time per use, and when Link
			//is out of MP, he flickers and the sword slash sound still plays. 
                    Lwpns.add(new weapon((fix)0,(fix)0,(fix)0,(attack==wSword ? wSword : wWand),0,0,dir,itemid,getUID()));
                    w = (weapon*)Lwpns.spr(Lwpns.Count()-1);
                    
                    positionSword(w,itemid);
                    
                    // Stone of Agony
                    if(agony)
                    {
                        w->y-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,2));
                    }
                }
                
                // These are set by positionSword(), above or in checkstab()
                yofs += slashyofs;
                xofs += slashxofs;
                slashyofs = slashxofs = 0;
            }
        }
        
        if(attackclk<7
                || (attack==wSword &&
                    (attackclk<(game->get_canslash()?15:13) || (charging>0 && attackclk!=SWORDCHARGEFRAME)))
                || ((attack==wWand || attack==wFire || attack==wCByrna) && attackclk<13)
                || (attack==wHammer && attackclk<=30))
        {
            if(!invisible)
            {
                linktile(&tile, &flip, &extend, ls_stab, dir, zinit.linkanimationstyle);
                
                if((game->get_canslash() && (attack==wSword || attack==wWand || attack==wFire || attack==wCByrna)) && itemsbuf[itemid].flags&ITEM_FLAG4 && (attackclk<7))
                {
                    linktile(&tile, &flip, &extend, ls_slash, dir, zinit.linkanimationstyle);
                }
                
                if((attack==wHammer) && (attackclk<13))
                {
                    linktile(&tile, &flip, &extend, ls_pound, dir, zinit.linkanimationstyle);
                }
                
                if(useltm)
                {
                    if ( script_link_sprite <= 0 ) tile+=item_tile_mod(shieldModify);
                }
                
                if ( script_link_sprite <= 0 ) tile+=dmap_tile_mod();
                
                // Stone of Agony
                if(agony)
                {
                    yofs-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3));
                }
                
                 //Probably what makes Link flicker, except for the QR check. What makes him flicker when that rule is off?! -Z
                    if (!(get_bit(quest_rules, qr_LINKFLICKER) && ((superman || hclk) && (frame & 1))))
                    {
                        masked_draw(dest);
                    }

                    //Prevent flickering -Z
                    if (!getCanLinkFlicker()) masked_draw(dest);
            }
            
            if(attack!=wHammer)
            {
                xofs=oxofs;
                yofs=oyofs;
                return;
            }
        }
        
        if(attack==wHammer) // To do: possibly abstract this out to a positionHammer routine?
        {
            int wy=1;
            int wx=1;
            int f=0,t,cs2;
            weapon *w=NULL;
            bool found = false;
            
            for(int i=0; i<Lwpns.Count(); i++)
            {
                w = (weapon*)Lwpns.spr(i);
                
                if(w->id == wHammer)
                {
                    found = true;
                    break;
                }
            }
            
            if(!found)
            {
                Lwpns.add(new weapon((fix)0,(fix)0,(fix)0,wHammer,0,0,dir,itemid,getUID()));
                w = (weapon*)Lwpns.spr(Lwpns.Count()-1);
                found = true;
            }
            
            t = w->o_tile;
            cs2 = w->o_cset;
            
            switch(dir)
            {
            case up:
                wx=-1;
                wy=-15;
                
                if(attackclk>=13)
                {
                    wx-=1;
                    wy+=1;
                    ++t;
                }
                
                if(attackclk>=15)
                {
                    ++t;
                }
                
                break;
                
            case down:
                wx=3;
                wy=-14;
                t+=3;
                
                if(attackclk>=13)
                {
                    wy+=16;
                    ++t;
                }
                
                if(attackclk>=15)
                {
                    wx-=1;
                    wy+=12;
                    ++t;
                }
                
                break;
                
            case left:
                wx=0;
                wy=-14;
                t+=6;
                f=1;
                
                if(attackclk>=13)
                {
                    wx-=7;
                    wy+=8;
                    ++t;
                }
                
                if(attackclk>=15)
                {
                    wx-=8;
                    wy+=8;
                    ++t;
                }
                
                break;
                
            case right:
                wx=0;
                wy=-14;
                t+=6;
                
                if(attackclk>=13)
                {
                    wx+=7;
                    wy+=8;
                    ++t;
                }
                
                if(attackclk>=15)
                {
                    wx+=8;
                    wy+=8;
                    ++t;
                }
                
                break;
            }
            
            if(BSZ || ((isdungeon() && currscr<128) && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)))
            {
                wy+=2;
            }
            
            // Stone of Agony
            if(agony)
            {
                wy-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3));
            }
            
            w->x = x+wx;
            w->y = y+wy-(54-yofs);
            w->z = (z+zofs);
            w->tile = t;
            w->flip = f;
            w->hxsz=20;
            w->hysz=20;
            
            if(dir>down)
            {
                w->hysz-=6;
            }
            else
            {
                w->hxsz-=6;
                w->hyofs=4;
            }
            
            w->power = weaponattackpower();
            
            if(attackclk==15 && z==0 && (sideviewhammerpound() || !isSideview()))
            {
                sfx(((iswater(MAPCOMBO(x+wx+8,y+wy)) || COMBOTYPE(x+wx+8,y+wy)==cSHALLOWWATER) && get_bit(quest_rules,qr_MORESOUNDS)) ? WAV_ZN1SPLASH : itemsbuf[itemid].usesound,pan(int(x)));
            }
            
            xofs=oxofs;
            yofs=oyofs;
            return;
        }
    }
    else if(!charging && !spins)  // remove the sword
    {
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = (weapon*)Lwpns.spr(i);
            
            if(w->id == wSword || w->id == wHammer || w->id==wWand)
                w->dead=1;
        }
    }
    
    if(invisible)
    {
        xofs=oxofs;
        yofs=oyofs;
        return;
    }
    
    if(action != casting)
    {
        // Keep this consistent with checkspecial2, line 7800-ish...
        bool inwater = iswater(MAPCOMBO(x+4,y+9))  && iswater(MAPCOMBO(x+4,y+15)) &&  iswater(MAPCOMBO(x+11,y+9)) && iswater(MAPCOMBO(x+11,y+15));
        
        int jumping2 = int(jumping*(zinit.gravity/16.0));
        
        //if (jumping!=0) al_trace("%d %d %f %d\n",jumping,zinit.gravity,zinit.gravity/16.0,jumping2);
        switch(zinit.linkanimationstyle)
        {
        case las_original:                                               //normal
            if(action==drowning)
            {
                if(inwater)
                {
                    linktile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
                }
                else
                {
                    xofs=oxofs;
                    yofs=oyofs;
                    return;
                }
            }
            else if(action==swimming || action==swimhit || hopclk==0xFF)
            {
                linktile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.linkanimationstyle);
                
                if(lstep>=6)
                {
                    if(dir==up)
                    {
                        if ( script_link_sprite <= 0 ) ++flip;
                    }
                    else
                    {
                        if ( script_link_sprite <= 0 ) extend==2?tile+=2:++tile;
                    }
                }
                
                if(diveclk>30)
                {
                    linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
                }
            }
            else if(charging > 0 && attack != wHammer)
            {
                linktile(&tile, &flip, &extend, ls_charge, dir, zinit.linkanimationstyle);
                
                if(lstep>=6)
                {
                    if(dir==up)
                    {
                        if ( script_link_sprite <= 0 ) ++flip;
                    }
                    else
                    {
                        if ( script_link_sprite <= 0 ) extend==2?tile+=2:++tile;
                    }
                }
            }
            else if((z>0 || isSideview()) && jumping2>0 && jumping2<24 && game->get_life()>0 && action!=rafting)
            {
                linktile(&tile, &flip, &extend, ls_jump, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile+=((int)jumping2/8)*(extend==2?2:1);
            }
            else
            {
                linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
                
                if(dir>up)
                {
                    useltm=true;
                    shieldModify=true;
                }
                
                if(lstep>=6)
                {
                    if(dir==up)
                    {
                        if ( script_link_sprite <= 0 ) ++flip;
                    }
                    else
                    {
                        if ( script_link_sprite <= 0 ) extend==2?tile+=2:++tile;
                    }
                }
            }
            
            break;
            
        case las_bszelda:                                               //BS
            if(action==drowning)
            {
                if(inwater)
                {
                    linktile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                }
                else
                {
                    xofs=oxofs;
                    yofs=oyofs;
                    return;
                }
            }
            else if(action==swimming || action==swimhit || hopclk==0xFF)
            {
                linktile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                
                if(diveclk>30)
                {
                    linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                }
            }
            else if(charging > 0 && attack != wHammer)
            {
                linktile(&tile, &flip, &extend, ls_charge, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
            }
            else if((z>0 || isSideview()) && jumping2>0 && jumping2<24 && game->get_life()>0)
            {
                linktile(&tile, &flip, &extend, ls_jump, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile+=((int)jumping2/8)*(extend==2?2:1);
            }
            else
            {
                linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
                
                if(dir>up)
                {
                    useltm=true;
                    shieldModify=true;
                }
                
                /*
                else if (dir==up)
                {
                useltm=true;
                }
                */
                if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
            }
            
            break;
            
        case las_zelda3slow:                                           //8-frame Zelda 3 (slow)
        case las_zelda3:                                               //8-frame Zelda 3
            if(action == drowning)
            {
                if(inwater)
                {
                    linktile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                }
                else
                {
                    xofs=oxofs;
                    yofs=oyofs;
                    return;
                }
            }
            else if(action == swimming || action==swimhit || hopclk==0xFF)
            {
                linktile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                
                if(diveclk>30)
                {
                    linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
                    if ( script_link_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
                }
            }
            else if(charging > 0 && attack != wHammer)
            {
                linktile(&tile, &flip, &extend, ls_charge, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile+=(extend==2?2:1);
//          int l=link_count/link_animation_speed;
                int l=(link_count/link_animation_speed)&15;
                //int l=((p[lt_clock]/link_animation_speed)&15);
                l-=((l>3)?1:0)+((l>12)?1:0);
                if ( script_link_sprite <= 0 ) tile+=(l/2)*(extend==2?2:1);
            }
            else if((z>0 || isSideview()) && jumping2>0 && jumping2<24 && game->get_life()>0)
            {
                linktile(&tile, &flip, &extend, ls_jump, dir, zinit.linkanimationstyle);
                if ( script_link_sprite <= 0 ) tile+=((int)jumping2/8)*(extend==2?2:1);
            }
            else
            {
                linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
                
                if(action == walking || action == climbcoverbottom || action == climbcovertop)
                {
                    if ( script_link_sprite <= 0 ) tile += (extend == 2 ? 2 : 1);
                }
                
                if(dir>up)
                {
                    useltm=true;
                    shieldModify=true;
                }
                
                if(action == walking || action == hopping || action == climbcoverbottom || action == climbcovertop)
                {
                    //tile+=(extend==2?2:1);
                    //tile+=(((active_count>>2)%8)*(extend==2?2:1));
                    int l = link_count / link_animation_speed;
                    l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
                    if ( script_link_sprite <= 0 ) tile += (l / 2) * (extend == 2 ? 2 : 1);
                }
            }
            
            break;
            
        default:
            break;
        }
    }
    
    yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
    
    if(action==won)
    {
        yofs=playing_field_offset - 2;
    }
    
    if(action==landhold1 || action==landhold2)
    {
        useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);
        yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
        linktile(&tile, &flip, &extend, (action==landhold1)?ls_landhold1:ls_landhold2, dir, zinit.linkanimationstyle);
    }
    else if(action==waterhold1 || action==waterhold2)
    {
        useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);
        linktile(&tile, &flip, &extend, (action==waterhold1)?ls_waterhold1:ls_waterhold2, dir, zinit.linkanimationstyle);
    }
    
    if(action!=casting)
    {
        if(useltm)
        {
            if ( script_link_sprite <= 0 ) tile+=item_tile_mod(shieldModify);
        }
    }
    
    if ( script_link_sprite <= 0 ) tile+=dmap_tile_mod();
    
    // Stone of Agony
    if(agony)
    {
        yofs-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3));
    }
    
    if(!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
    {
        masked_draw(dest);
    }
    
    //draw held items after Link so they don't go behind his head
    if(action==landhold1 || action==landhold2)
    {
        if(holditem > -1)
        {
            if(get_bit(quest_rules,qr_HOLDITEMANIMATION))
            {
                putitem2(dest,x-((action==landhold1)?4:0),y+yofs-16-(get_bit(quest_rules, qr_NOITEMOFFSET)),holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
            }
            else
            {
                putitem(dest,x-((action==landhold1)?4:0),y+yofs-16-(get_bit(quest_rules, qr_NOITEMOFFSET)),holditem);
            }
        }
    }
    else if(action==waterhold1 || action==waterhold2)
    {
        if(holditem > -1)
        {
            if(get_bit(quest_rules,qr_HOLDITEMANIMATION))
            {
                putitem2(dest,x-((action==waterhold1)?4:0),y+yofs-12-(get_bit(quest_rules, qr_NOITEMOFFSET)),holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
            }
            else
            {
                putitem(dest,x-((action==waterhold1)?4:0),y+yofs-12-(get_bit(quest_rules, qr_NOITEMOFFSET)),holditem);
            }
        }
    }
    
    if(fairyclk==0||(get_bit(quest_rules,qr_NOHEARTRING)))
    {
        xofs=oxofs;
        yofs=oyofs;
        return;
    }
    
    double a2 = fairyclk*2*PI/80 + (PI/2);
    int hearts=0;
    //  int htile = QHeader.dat_flags[ZQ_TILES] ? 2 : 0;
    int htile = 2;
    
    do
    {
        int nx=125;
        
        if(get_bit(quest_rules,qr_HEARTRINGFIX))
        {
            nx=x;
        }
        
        int ny=88;
        
        if(get_bit(quest_rules,qr_HEARTRINGFIX))
        {
            ny=y;
        }
        
        double tx = cos(a2)*53  +nx;
        double ty = -sin(a2)*53 +ny+playing_field_offset;
        overtile8(dest,htile,int(tx),int(ty),1,0);
        a2-=PI/4;
        ++hearts;
    }
    while(a2>PI/2 && hearts<8);
    
    xofs=oxofs;
    yofs=oyofs;
}

void LinkClass::masked_draw(BITMAP* dest)
{
    if(isdungeon() && currscr<128 && (x<16 || x>224 || y<18 || y>146) && !get_bit(quest_rules,qr_FREEFORM))
    {
        // clip under doorways
        BITMAP *sub=create_sub_bitmap(dest,16,playing_field_offset+16,224,144);
        
        if(sub!=NULL)
        {
            yofs -= (playing_field_offset+16);
            xofs -= 16;
            sprite::draw(sub);
            xofs=0;
            yofs += (playing_field_offset+16);
            destroy_bitmap(sub);
        }
    }
    else
    {
        sprite::draw(dest);
    }
    
    return;
}

// separate case for sword/wand/hammer/slashed weapons only
// the main weapon checking is in the global function check_collisions()
void LinkClass::checkstab()
{
    if(action!=attacking || (attack!=wSword && attack!=wWand && attack!=wHammer && attack!=wCByrna && attack!=wFire)
            || (attackclk<=4))
        return;
        
    weapon *w=NULL;
    
    int wx=0,wy=0,wz=0,wxsz=0,wysz=0;
    bool found = false;
    int melee_weapon_index;
    
    for(int i=0; i<Lwpns.Count(); i++)
    {
        w = (weapon*)Lwpns.spr(i);
        
        if(w->id == (attack==wCByrna || attack==wFire ? wWand : attack))  // Kludge: Byrna and Candle sticks are wWand-type.
        {
            found = true;
            melee_weapon_index = i+1;
            // Position the sword as Link slashes with it.
            if(w->id!=wHammer)
                positionSword(w,w->parentitem);
                
            wx=w->x;
            wy=w->y;
            wz=w->z;
            wxsz = w->hxsz;
            wysz = w->hysz;
            break;
        }
    }
    
    if(attack==wSword && attackclk>=14 && charging==0)
        return;
        
    if(!found)
        return;
        
    if(attack==wCByrna||attack==wFire)
        return;
        
    if(attack==wHammer)
    {
        if(attackclk<15)
        {
            switch(w->dir)
            {
            case up:
                wx=x-1;
                wy=y-4;
                break;
                
            case down:
                wx=x+8;
                wy=y+28;
                break; // This is consistent with 2.10
                
            case left:
                wx=x-13;
                wy=y+14;
                break;
                
            case right:
                wx=x+21;
                wy=y+14;
                break;
            }
            
            if(attackclk==12 && z==0 && sideviewhammerpound())
            {
                switch(dir)
                {
                case up:
                    decorations.add(new dHammerSmack(x-1, y-4, dHAMMERSMACK, 0));
                    break;
                    
                case down:
                    decorations.add(new dHammerSmack(x+8, y+28, dHAMMERSMACK, 0));
                    break;
                    
                case left:
                    decorations.add(new dHammerSmack(x-13, y+14, dHAMMERSMACK, 0));
                    break;
                    
                case right:
                    decorations.add(new dHammerSmack(x+21, y+14, dHAMMERSMACK, 0));
                    break;
                }
            }
            
            return;
        }
        else if(attackclk==15)
        {
            // Hammer's reach needs adjusted slightly for backward compatibility
            if(w->dir==up)
                w->hyofs-=1;
            else if(w->dir==left)
                w->hxofs-=2;
        }
    }
    
    // The return of Spaghetti Code Constants!
    int itype = (attack==wWand ? itype_wand : itype_sword);
    int itemid = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
    itemid = vbound(itemid, 0, MAXITEMS-1);
    
    // The sword offsets aren't based on anything other than what felt about right
    // compared to the NES game and what mostly kept it from hitting things that
    // should clearly be out of range. They could probably still use more tweaking.
    // Don't use 2.10 for reference; it's pretty far off.
    // - Saf
    
    if(game->get_canslash() && (attack==wSword || attack==wWand) && itemsbuf[itemid].flags & ITEM_FLAG4)
    {
        switch(w->dir)
        {
        case up:
            if(attackclk<8)
            {
                wy-=4;
            }
            
            break;
            
        case down:
            //if(attackclk<8)
        {
            wy-=2;
        }
        break;
        
        case left:
        
            //if(attackclk<8)
        {
            wx+=2;
        }
        
        break;
        
        case right:
        
            //if(attackclk<8)
        {
            wx-=3;
            //wy+=((spins>0 || get_bit(quest_rules, qr_SLASHFLIPFIX)) ? -4 : 4);
        }
        
        break;
        }
    }
    
    switch(w->dir)
    {
    case up:
        wx+=2;
        break;
        
    case down:
        break;
        
    case left:
        wy-=3;
        break;
        
    case right:
        wy-=3;
        break;
    }
    
    wx+=w->hxofs;
    wy+=w->hyofs;
    
    for(int i=0; i<guys.Count(); i++)
    {
        // So that Link can actually hit peahats while jumping, his weapons' hzsz becomes 16 in midair.
        if((guys.spr(i)->hit(wx,wy,wz,wxsz,wysz,wz>0?16:8) && (attack!=wWand || !get_bit(quest_rules,qr_NOWANDMELEE)))
                || (attack==wWand && guys.spr(i)->hit(wx,wy-8,z,16,24,z>8) && !get_bit(quest_rules,qr_NOWANDMELEE))
                || (attack==wHammer && guys.spr(i)->hit(wx,wy-8,z,16,24,z>0?16:8)))
        {
            // Checking the whimsical ring for every collision check causes
            // an odd bug. It's much more likely to activate on a 0-damage
            // weapon, since a 0-damage hit won't make the enemy invulnerable
            // to damaging hits in the following frames.
            
            int whimsyid = current_item_id(itype_whimsicalring);
            int whimsypower = 0;
            
            if(whimsyid>-1)
            {
                whimsypower = rand()%zc_max(itemsbuf[current_item_id(itype_whimsicalring)].misc1,1) ?
                              0 : current_item_power(itype_whimsicalring);
            }
            
            int h = hit_enemy(i,attack,(weaponattackpower() + whimsypower)*DAMAGE_MULTIPLIER,wx,wy,dir,directWpn);
	    enemy *e = (enemy*)guys.spr(i);
	    if (h == -1) { e->hitby[HIT_BY_LWEAPON] = melee_weapon_index; } //temp_hit = true; }
	    //melee weapons and non-melee weapons both writing to this index may be a problem. It needs to be cleared by something earlier than this check.
            
            if(h<0 && whimsypower)
            {
                sfx(itemsbuf[whimsyid].usesound);
            }
            
            if(h && charging>0)
            {
                attackclk = SWORDTAPFRAME;
                spins=0;
            }
            
            if(h && hclk==0 && inlikelike != 1)
            {
                if(GuyHit(i,x+7,y+7,z,2,2,hzsz)!=-1)
                {
                    hitlink(i);
                }
            }
            
            if(h==2)
                break;
        }
    }
    
    if(!get_bit(quest_rules,qr_NOITEMMELEE))
    {
        for(int j=0; j<items.Count(); j++)
        {
            if(((item*)items.spr(j))->pickup & ipTIMER)
            {
                if(((item*)items.spr(j))->clk2 >= 32)
                {
                    if(items.spr(j)->hit(wx,wy,z,wxsz,wysz,1) || (attack==wWand && items.spr(j)->hit(x,y-8,z,wxsz,wysz,1))
                            || (attack==wHammer && items.spr(j)->hit(x,y-8,z,wxsz,wysz,1)))
                    {
                        int pickup = ((item*)items.spr(j))->pickup;
                        
                        if(pickup&ipONETIME) // set mITEM for one-time-only items
                            setmapflag(mITEM);
                        else if(pickup&ipONETIME2) // set mBELOW flag for other one-time-only items
                            setmapflag();
                            
                        if(itemsbuf[items.spr(j)->id].collect_script)
                        {
                            ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[items.spr(j)->id].collect_script, items.spr(j)->id & 0xFFF);
                        }
                        
                        getitem(items.spr(j)->id);
                        items.del(j);
                        
                        for(int i=0; i<Lwpns.Count(); i++)
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
                        
                        --j;
                    }
                }
            }
        }
    }
    
    if(attack==wSword)
    {
        if(attackclk == 6)
        {
            for(int q=0; q<176; q++)
            {
                set_bit(screengrid,q,0);
            }
            
            for(int q=0; q<32; q++)
                set_bit(ffcgrid, q, 0);
        }
        
        if(dir==up && ((int(x)&15)==0))
        {
            check_slash_block(wx,wy);
            check_slash_block(wx,wy+8);
        }
        else if(dir==up && ((int(x)&15)==8||diagonalMovement))
        {
            check_slash_block(wx,wy);
            check_slash_block(wx,wy+8);
            check_slash_block(wx+8,wy);
            check_slash_block(wx+8,wy+8);
        }
        
        if(dir==down && ((int(x)&15)==0))
        {
            check_slash_block(wx,wy+wysz-8);
            check_slash_block(wx,wy+wysz);
        }
        else if(dir==down && ((int(x)&15)==8||diagonalMovement))
        {
            check_slash_block(wx,wy+wysz-8);
            check_slash_block(wx,wy+wysz);
            check_slash_block(wx+8,wy+wysz-8);
            check_slash_block(wx+8,wy+wysz);
        }
        
        if(dir==left)
        {
            check_slash_block(wx,wy+8);
            check_slash_block(wx+8,wy+8);
        }
        
        if(dir==right)
        {
            check_slash_block(wx+wxsz,wy+8);
            check_slash_block(wx+wxsz-8,wy+8);
        }
    }
    else if(attack==wWand)
    {
        if(attackclk == 5)
        {
            for(int q=0; q<176; q++)
            {
                set_bit(screengrid,q,0);
            }
            
            for(int q=0; q<32; q++)
                set_bit(ffcgrid,q, 0);
        }
        
        // cutable blocks
        if(dir==up && (int(x)&15)==0)
        {
            check_wand_block(wx,wy);
            check_wand_block(wx,wy+8);
        }
        else if(dir==up && ((int(x)&15)==8||diagonalMovement))
        {
            check_wand_block(wx,wy);
            check_wand_block(wx,wy+8);
            check_wand_block(wx+8,wy);
            check_wand_block(wx+8,wy+8);
        }
        
        if(dir==down && (int(x)&15)==0)
        {
            check_wand_block(wx,wy+wysz-8);
            check_wand_block(wx,wy+wysz);
        }
        else if(dir==down && ((int(x)&15)==8||diagonalMovement))
        {
            check_wand_block(wx,wy+wysz-8);
            check_wand_block(wx,wy+wysz);
            check_wand_block(wx+8,wy+wysz-8);
            check_wand_block(wx+8,wy+wysz);
        }
        
        if(dir==left)
        {
            check_wand_block(wx,y+8);
            check_wand_block(wx+8,y+8);
        }
        
        if(dir==right)
        {
            check_wand_block(wx+wxsz,y+8);
            check_wand_block(wx+wxsz-8,y+8);
        }
    }
    else if((attack==wHammer) && (attackclk==15))
    {
        // poundable blocks
        for(int q=0; q<176; q++)
        {
            set_bit(screengrid,q,0);
        }
        
        for(int q=0; q<32; q++)
            set_bit(ffcgrid, q, 0);
            
        if(dir==up && (int(x)&15)==0)
        {
            check_pound_block(wx,wy);
            check_pound_block(wx,wy+8);
        }
        else if(dir==up && ((int(x)&15)==8||diagonalMovement))
        {
            check_pound_block(wx,wy);
            check_pound_block(wx,wy+8);
            check_pound_block(wx+8,wy);
            check_pound_block(wx+8,wy+8);
        }
        
        if(dir==down && (int(x)&15)==0)
        {
            check_pound_block(wx,wy+wysz-8);
            check_pound_block(wx,wy+wysz);
        }
        else if(dir==down && ((int(x)&15)==8||diagonalMovement))
        {
            check_pound_block(wx,wy+wysz-8);
            check_pound_block(wx,wy+wysz);
            check_pound_block(wx+8,wy+wysz-8);
            check_pound_block(wx+8,wy+wysz);
        }
        
        if(dir==left)
        {
            check_pound_block(wx,y+8);
            check_pound_block(wx+8,y+8);
        }
        
        if(dir==right)
        {
            check_pound_block(wx+wxsz,y+8);
            check_pound_block(wx+wxsz-8,y+8);
        }
    }
    
    return;
}

void LinkClass::check_slash_block(int bx, int by)
{
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, 255);
    by=vbound(by, 0, 176);
    int fx=vbound(bx, 0, 255);
    int fy=vbound(by, 0, 176);
    
    //first things first
    if(attack!=wSword)
        return;
        
    if(z>8 || attackclk==SWORDCHARGEFRAME  // is not charging>0, as tapping a wall reduces attackclk but retains charging
            || (attackclk>SWORDTAPFRAME && tapping))
        return;
        
    //find out which combo row/column the coordinates are in
    bx &= 0xF0;
    by &= 0xF0;
    
    int type = COMBOTYPE(bx,by);
    int type2 = FFCOMBOTYPE(fx,fy);
    int flag = MAPFLAG(bx,by);
    int flag2 = MAPCOMBOFLAG(bx,by);
    int flag3 = MAPFFCOMBOFLAG(fx,fy);
    int i = (bx>>4) + by;
    
    if(i > 175)
        return;
        
    bool ignorescreen=false;
    bool ignoreffc=false;
    
    if(get_bit(screengrid, i) != 0)
    {
        ignorescreen = true;
    }
    
    int current_ffcombo = getFFCAt(fx,fy);
    
    if(current_ffcombo == -1 || get_bit(ffcgrid, current_ffcombo) != 0)
    {
        ignoreffc = true;
    }
    
    if(!isCuttableType(type) &&
            (flag<mfSWORD || flag>mfXSWORD) &&  flag!=mfSTRIKE && (flag2<mfSWORD || flag2>mfXSWORD) && flag2!=mfSTRIKE)
    {
        ignorescreen = true;
    }
    
    if(!isCuttableType(type2) &&
            (flag3<mfSWORD || flag3>mfXSWORD) && flag3!=mfSTRIKE)
    {
        ignoreffc = true;
    }
    
    mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);
    
    int sworditem = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? itemsbuf[directWpn].fam_type : current_item(itype_sword);
    
    if(!ignorescreen)
    {
        if((flag >= 16)&&(flag <= 31))
        {
            s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
            s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
            s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        }
        else if(flag == mfARMOS_SECRET)
        {
            s->data[i] = s->secretcombo[sSTAIRS];
            s->cset[i] = s->secretcset[sSTAIRS];
            s->sflag[i] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else if(((flag>=mfSWORD&&flag<=mfXSWORD)||(flag==mfSTRIKE)))
        {
            for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                findentrance(bx,by,mfSWORD+i2,true);
            }
            
            findentrance(bx,by,mfSTRIKE,true);
        }
        else if(((flag2 >= 16)&&(flag2 <= 31)))
        {
            s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
            s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
            s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        }
        else if(flag2 == mfARMOS_SECRET)
        {
            s->data[i] = s->secretcombo[sSTAIRS];
            s->cset[i] = s->secretcset[sSTAIRS];
            s->sflag[i] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else if(((flag2>=mfSWORD&&flag2<=mfXSWORD)||(flag2==mfSTRIKE)))
        {
            for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                findentrance(bx,by,mfSWORD+i2,true);
            }
            
            findentrance(bx,by,mfSTRIKE,true);
        }
        else
        {
            if(isCuttableNextType(type))
            {
                s->data[i]++;
            }
            else
            {
                s->data[i] = s->undercombo;
                s->cset[i] = s->undercset;
                s->sflag[i] = 0;
            }
            
            //pausenow=true;
        }
    }
    
    if(((flag3>=mfSWORD&&flag3<=mfXSWORD)||(flag3==mfSTRIKE)) && !ignoreffc)
    {
        for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
        {
            findentrance(bx,by,mfSWORD+i2,true);
        }
        
        findentrance(fx,fy,mfSTRIKE,true);
    }
    else if(!ignoreffc)
    {
        if(isCuttableNextType(type2))
        {
            s->ffdata[current_ffcombo]++;
        }
        else
        {
            s->ffdata[current_ffcombo] = s->undercombo;
            s->ffcset[current_ffcombo] = s->undercset;
        }
    }
    
    if(!ignorescreen)
    {
        if(!isTouchyType(type)) set_bit(screengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && !getmapflag())
        {
            items.add(new item((fix)bx, (fix)by,(fix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP, 0));
            sfx(tmpscr->secretsfx);
        }
        else if(isCuttableItemType(type))
        {
            int it = select_dropitem(12, bx, by);
            
            if(it!=-1)
            {
                items.add(new item((fix)bx, (fix)by,(fix)0, it, ipBIGRANGE + ipTIMER, 0));
            }
        }
        
        putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
        
        if(isBushType(type) || isFlowersType(type) || isGrassType(type))
        {
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                sfx(WAV_ZN1GRASSCUT,int(bx));
            }
            
            if(isBushType(type))
            {
                decorations.add(new dBushLeaves((fix)fx, (fix)fy, dBUSHLEAVES, 0));
            }
            else if(isFlowersType(type))
            {
                decorations.add(new dFlowerClippings((fix)fx, (fix)fy, dFLOWERCLIPPINGS, 0));
            }
            else if(isGrassType(type))
            {
                decorations.add(new dGrassClippings((fix)fx, (fix)fy, dGRASSCLIPPINGS, 0));
            }
        }
    }
    
    if(!ignoreffc)
    {
        if(!isTouchyType(type)) set_bit(ffcgrid, current_ffcombo, 1);
        
        if(isCuttableItemType(type2))
        {
            int it=-1;
            int r=rand()%100;
            
            if(r<15)
            {
                it=iHeart;                                // 15%
            }
            else if(r<35)
            {
                it=iRupy;                                 // 20%
            }
            
            if(it!=-1 && itemsbuf[it].family != itype_misc) // Don't drop non-gameplay items
            {
                items.add(new item((fix)fx, (fix)fy,(fix)0, it, ipBIGRANGE + ipTIMER, 0));
            }
        }
        
        if(isBushType(type2) || isFlowersType(type2) || isGrassType(type2))
        {
            if(get_bit(quest_rules,qr_MORESOUNDS))
            {
                sfx(WAV_ZN1GRASSCUT,int(bx));
            }
            
            if(isBushType(type2))
            {
                decorations.add(new dBushLeaves((fix)fx, (fix)fy, dBUSHLEAVES, 0));
            }
            else if(isFlowersType(type2))
            {
                decorations.add(new dFlowerClippings((fix)fx, (fix)fy, dFLOWERCLIPPINGS, 0));
            }
            else if(isGrassType(type2))
            {
                decorations.add(new dGrassClippings((fix)fx, (fix)fy, dGRASSCLIPPINGS, 0));
            }
        }
    }
}

//TODO: Boomerang that cuts bushes. -L
/*void LinkClass::slash_bush()
{

}*/

void LinkClass::check_wand_block(int bx, int by)
{
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, 255);
    by=vbound(by, 0, 176);
    int fx=vbound(bx, 0, 255);
    int fy=vbound(by, 0, 176);
    
    //first things first
    if(z>8) return;
    
    //find out which combo row/column the coordinates are in
    bx &= 0xF0;
    by &= 0xF0;
    
    int flag = MAPFLAG(bx,by);
    int flag2 = MAPCOMBOFLAG(bx,by);
    int flag3=0;
    int flag31 = MAPFFCOMBOFLAG(fx,fy);
    int flag32 = MAPFFCOMBOFLAG(fx,fy);
    int flag33 = MAPFFCOMBOFLAG(fx,fy);
    int flag34 = MAPFFCOMBOFLAG(fx,fy);
    
    if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
        flag3=mfWAND;
        
    if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
        flag3=mfSTRIKE;
        
    int i = (bx>>4) + by;
    
    if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
        return;
        
    if(i > 175)
        return;
        
    //mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);
    
    //findentrance(bx,by,mfWAND,true);
    //findentrance(bx,by,mfSTRIKE,true);
    if((findentrance(bx,by,mfWAND,true)==false)&&(findentrance(bx,by,mfSTRIKE,true)==false))
    {
        if(flag3==mfWAND||flag3==mfSTRIKE)
        {
            findentrance(fx,fy,mfWAND,true);
            findentrance(fx,fy,mfSTRIKE,true);
        }
    }
    
    //putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
}

void LinkClass::check_pound_block(int bx, int by)
{
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, 255);
    by=vbound(by, 0, 176);
    int fx=vbound(bx, 0, 255);
    int fy=vbound(by, 0, 176);
    
    //first things first
    if(z>8) return;
    
    //find out which combo row/column the coordinates are in
    bx &= 0xF0;
    by &= 0xF0;
    
    int type = COMBOTYPE(bx,by);
    int type2 = FFCOMBOTYPE(fx,fy);
    int flag = MAPFLAG(bx,by);
    int flag2 = MAPCOMBOFLAG(bx,by);
    int flag3 = MAPFFCOMBOFLAG(fx,fy);
    int i = (bx>>4) + by;
    
    if(i > 175)
        return;
        
    bool ignorescreen=false;
    bool ignoreffc=false;
    bool pound=false;
    
    if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
        ignorescreen = true; // Affect only FFCs
        
    if(get_bit(screengrid, i) != 0)
        ignorescreen = true;
        
    int current_ffcombo = getFFCAt(fx,fy);
    
    if(current_ffcombo == -1 || get_bit(ffcgrid, current_ffcombo) != 0)
        ignoreffc = true;
        
    if(type2!=cPOUND && flag3!=mfSTRIKE && flag3!=mfHAMMER)
        ignoreffc = true;
        
    if(ignorescreen && ignoreffc)  // Nothing to do.
        return;
        
    mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);
    
    if(!ignorescreen)
    {
        if(flag==mfHAMMER||flag==mfSTRIKE)  // Takes precedence over Secret Tile and Armos->Secret
        {
            findentrance(bx,by,mfHAMMER,true);
            findentrance(bx,by,mfSTRIKE,true);
        }
        else if(flag2==mfHAMMER||flag2==mfSTRIKE)
        {
            findentrance(bx,by,mfHAMMER,true);
            findentrance(bx,by,mfSTRIKE,true);
        }
        else if((flag >= 16)&&(flag <= 31))
        {
            s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
            s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
            s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        }
        else if(flag == mfARMOS_SECRET)
        {
            s->data[i] = s->secretcombo[sSTAIRS];
            s->cset[i] = s->secretcset[sSTAIRS];
            s->sflag[i] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else if((flag2 >= 16)&&(flag2 <= 31))
        {
            s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
            s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
            s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        }
        else if(flag2 == mfARMOS_SECRET)
        {
            s->data[i] = s->secretcombo[sSTAIRS];
            s->cset[i] = s->secretcset[sSTAIRS];
            s->sflag[i] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else pound = true;
    }
    
    if(!ignoreffc)
    {
        if(flag3==mfHAMMER||flag3==mfSTRIKE)
        {
            findentrance(fx,fy,mfHAMMER,true);
            findentrance(fx,fy,mfSTRIKE,true);
        }
        else
        {
            s->ffdata[current_ffcombo]+=1;
        }
    }
    
    if(!ignorescreen)
    {
        if(pound)
            s->data[i]+=1;
            
        set_bit(screengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && !getmapflag())
        {
            items.add(new item((fix)bx, (fix)by, (fix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP, 0));
            sfx(tmpscr->secretsfx);
        }
        
        if(type==cPOUND && get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1HAMMERPOST,int(bx));
            
        putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
    }
    
    if(!ignoreffc)
    {
        set_bit(ffcgrid,current_ffcombo,1);
        
        if(type2==cPOUND && get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1HAMMERPOST,int(bx));
    }
    
    return;
}

int LinkClass::EwpnHit()
{
    for(int i=0; i<Ewpns.Count(); i++)
    {
        if(Ewpns.spr(i)->hit(x+7,y+7,z,2,2,1))
        {
            weapon *ew = (weapon*)(Ewpns.spr(i));
            bool hitshield=false;
            
            if((ew->ignoreLink)==true)
                break;
                
            if(ew->id==ewWind)
            {
                xofs=1000;
                action=freeze; FFCore.setLinkAction(freeze);
                ew->misc=999;                                         // in enemy wind
                attackclk=0;
                return -1;
            }
            
            switch(dir)
            {
            case up:
                if(ew->dir==down || ew->dir==l_down || ew->dir==r_down)
                    hitshield=true;
                    
                break;
                
            case down:
                if(ew->dir==up || ew->dir==l_up || ew->dir==r_up)
                    hitshield=true;
                    
                break;
                
            case left:
                if(ew->dir==right || ew->dir==r_up || ew->dir==r_down)
                    hitshield=true;
                    
                break;
                
            case right:
                if(ew->dir==left || ew->dir==l_up || ew->dir==l_down)
                    hitshield=true;
                    
                break;
            }
            
            switch(ew->id)
            {
            case ewLitBomb:
            case ewBomb:
            case ewLitSBomb:
            case ewSBomb:
                return i;
            }
            
            if(!hitshield || action==attacking || action==swimming || charging > 0 || spins > 0 || hopclk==0xFF)
            {
                return i;
            }
            
            int itemid = current_item_id(itype_shield);
            
            if(itemid<0 || !checkmagiccost(itemid)) return i;
            
            paymagiccost(itemid);
            
            bool reflect = false;
            
            switch(ew->id)
            {
            case ewFireball2:
            case ewFireball:
                if(ew->type & 1) //Boss fireball
                {
                    if(!(itemsbuf[itemid].misc1 & (shFIREBALL2)))
                        return i;
                        
                    reflect = ((itemsbuf[itemid].misc2 & shFIREBALL2) != 0);
                }
                else
                {
                    if(!(itemsbuf[itemid].misc1 & (shFIREBALL)))
                        return i;
                        
                    reflect = ((itemsbuf[itemid].misc2 & shFIREBALL) != 0);
                }
                
                break;
                
            case ewMagic:
                if(!(itemsbuf[itemid].misc1 & shMAGIC))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shMAGIC) != 0);
                break;
                
            case ewSword:
                if(!(itemsbuf[itemid].misc1 & shSWORD))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shSWORD) != 0);
                break;
                
            case ewFlame:
                if(!(itemsbuf[itemid].misc1 & shFLAME))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shFLAME) != 0); // Actually isn't reflected.
                break;
                
            case ewRock:
                if(!(itemsbuf[itemid].misc1 & shROCK))
                    return i;
                    
                reflect = (itemsbuf[itemid].misc2 & shROCK);
                break;
                
            case ewArrow:
                if(!(itemsbuf[itemid].misc1 & shARROW))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shARROW) != 0); // Actually isn't reflected.
                break;
                
            case ewBrang:
                if(!(itemsbuf[itemid].misc1 & shBRANG))
                    return i;
                    
                break;
                
            default: // Just throw the script weapons in here...
                if(ew->id>=wScript1 && ew->id<=wScript10)
                {
                    if(!(itemsbuf[itemid].misc1 & shSCRIPT))
                        return i;
                        
                    reflect = ((itemsbuf[itemid].misc2 & shSCRIPT) != 0);
                }
                
                break;
            }
            
            int oldid = ew->id;
            ew->onhit(false, reflect ? 2 : 1, dir);
            
            if(ew->id != oldid)                                     // changed type from ewX to wX
            {
                //        ew->power*=DAMAGE_MULTIPLIER;
                Lwpns.add(ew);
                Ewpns.remove(ew);
            }
            
            if(ew->id==wRefMagic)
            {
                ew->ignoreLink=true;
                ew->ignorecombo=-1;
            }
            
            sfx(itemsbuf[itemid].usesound,pan(int(x)));
        }
    }
    
    return -1;
}

int LinkClass::LwpnHit()                                    //only here to check magic hits
{
    for(int i=0; i<Lwpns.Count(); i++)
        if(Lwpns.spr(i)->hit(x+7,y+7,z,2,2,1))
        {
            weapon *lw = (weapon*)(Lwpns.spr(i));
            bool hitshield=false;
            
            if((lw->ignoreLink)==true)
                break;
                
            switch(dir)
            {
            case up:
                if(lw->dir==down || lw->dir==l_down || lw->dir==r_down)
                    hitshield=true;
                    
                break;
                
            case down:
                if(lw->dir==up || lw->dir==l_up || lw->dir==r_up)
                    hitshield=true;
                    
                break;
                
            case left:
                if(lw->dir==right || lw->dir==r_up || lw->dir==r_down)
                    hitshield=true;
                    
                break;
                
            case right:
                if(lw->dir==left || lw->dir==l_up || lw->dir==l_down)
                    hitshield=true;
                    
                break;
            }
            
            int itemid = current_item_id(itype_shield);
            bool reflect = false;
            
            switch(lw->id)
            {
            case wRefFireball:
                if(itemid<0)
                    return i;
                    
                if(lw->type & 1)  //Boss fireball
                    return i;
                    
                if(!(itemsbuf[itemid].misc1 & (shFIREBALL)))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shFIREBALL) != 0);
                break;
                
            case wRefMagic:
                if(itemid<0)
                    return i;
                    
                if(!(itemsbuf[itemid].misc1 & shMAGIC))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shMAGIC) != 0);
                break;
                
            case wRefBeam:
                if(itemid<0)
                    return i;
                    
                if(!(itemsbuf[itemid].misc1 & shSWORD))
                    return i;
                    
                reflect = ((itemsbuf[itemid].misc2 & shSWORD) != 0);
                break;
                
            case wRefRock:
                if(itemid<0)
                    return i;
                    
                if(!(itemsbuf[itemid].misc1 & shROCK))
                    return i;
                    
                reflect = (itemsbuf[itemid].misc2 & shROCK);
                break;
                
            default:
                return -1;
            }
            
            if(!hitshield || action==attacking || action==swimming || hopclk==0xFF)
                return i;
                
            if(itemid<0 || !checkmagiccost(itemid)) return i;
            
            paymagiccost(itemid);
            
            lw->onhit(false, 1+reflect, dir);
            lw->ignoreLink=true;
            lw->ignorecombo=-1;
            sfx(itemsbuf[itemid].usesound,pan(int(x)));
        }
        
    return -1;
}

void LinkClass::checkhit()
{
    if(checklink==true)
    {
        if(hclk>0)
        {
            --hclk;
        }
        
        if(NayrusLoveShieldClk>0)
        {
            --NayrusLoveShieldClk;
            
            if(NayrusLoveShieldClk == 0 && nayruitem != -1)
            {
                stop_sfx(itemsbuf[nayruitem].usesound);
                stop_sfx(itemsbuf[nayruitem].usesound+1);
                nayruitem = -1;
            }
            else if(get_bit(quest_rules,qr_MORESOUNDS) && !(NayrusLoveShieldClk&0xF00) && nayruitem != -1)
            {
                stop_sfx(itemsbuf[nayruitem].usesound);
                cont_sfx(itemsbuf[nayruitem].usesound+1);
            }
        }
    }
    
    if(hclk<39 && action==gothit)
    {
        action=none; FFCore.setLinkAction(none);
    }
        
    if(hclk<39 && action==swimhit)
    {
        action=swimming; FFCore.setLinkAction(swimming);
    }
        
    if(hclk>=40 && action==gothit)
    {
        if(((ladderx+laddery) && ((hitdir&2)==ladderdir))||(!(ladderx+laddery)))
        {
            for(int i=0; i<4; i++)
            {
                switch(hitdir)
                {
                case up:
                    if(hit_walkflag(x,y+(bigHitbox?-1:7),2)||(int(x)&7?hit_walkflag(x+16,y+(bigHitbox?-1:7),1):0))    
		    {
			    action=none; FFCore.setLinkAction(none);
		    }
                    else --y;
                    
                    break;
                    
                case down:
                    if(hit_walkflag(x,y+16,2)||(int(x)&7?hit_walkflag(x+16,y+16,1):0))   
		    {
			    action=none; FFCore.setLinkAction(none);
		    }
                    else ++y;
                    
                    break;
                    
                case left:
                    if(hit_walkflag(x-1,y+(bigHitbox?0:8),1)||hit_walkflag(x-1,y+8,1)||(int(y)&7?hit_walkflag(x-1,y+16,1):0))  
		    {
			    action=none; FFCore.setLinkAction(none);
		    }
                    else --x;
                    
                    break;
                    
                case right:
                    if(hit_walkflag(x+16,y+(bigHitbox?0:8),1)||hit_walkflag(x+16,y+8,1)||(int(y)&7?hit_walkflag(x+16,y+16,1):0))
		    {
			    action=none; FFCore.setLinkAction(none);
		    }
                    else ++x;
                    
                    break;
                }
            }
        }
    }
    
    if(hclk>0 || inlikelike == 1 || action==inwind || action==drowning || inwallm || diveclk>30 || (action==hopping && hopclk<255))
    {
        return;
    }
    
    for(int i=0; i<Lwpns.Count(); i++)
    {
        sprite *s = Lwpns.spr(i);
	int itemid = ((weapon*)(Lwpns.spr(i)))->parentitem;
        //if ( itemdbuf[parentitem].flags&ITEM_FLAGS3 ) //can damage Link
	    //if ( itemsbuf[parentitem].misc1 > 0 ) //damages Link by this amount. 
        if((!((itemid==-1&&get_bit(quest_rules,qr_FIREPROOFLINK)||((itemid>-1&&itemsbuf[itemid].family==itype_candle||itemsbuf[itemid].family==itype_book)&&(itemsbuf[itemid].flags & ITEM_FLAG3))) || get_bit(quest_rules,qr_FIREPROOFLINK)) && (scriptcoldet&1) && (!superman || !get_bit(quest_rules,qr_FIREPROOFLINK2)))
        {
            if(s->id==wFire && (superman ? (diagonalMovement?s->hit(x+4,y+4,z,7,7,1):s->hit(x+7,y+7,z,2,2,1)) : s->hit(this))&&
                        (itemid < 0 || itemsbuf[itemid].family!=itype_dinsfire))
            {
                if(NayrusLoveShieldClk<=0)
                {
                    int ringpow = ringpower(lwpn_dp(i));
                    game->set_life(zc_max(game->get_life()-ringpow,0));
                }
                
                hitdir = s->hitdir(x,y,16,16,dir);
                
                if (action != rafting && action != freeze)
                {
                    if (action == swimming || hopclk == 0xFF)
		    {
                        action=swimhit; FFCore.setLinkAction(swimhit);
		    }
                    else
		    {
                        action=gothit; FFCore.setLinkAction(gothit);
		    }
                }
                    
                if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
                {
                    spins = charging = attackclk = 0;
                    attack=none;
                    tapping = false;
                }
                
                hclk=48;
                sfx(getHurtSFX(),pan(int(x)));
                return;
            }
        }
        
        //   check enemy weapons true, 1, -1
        //
        if(itemsbuf[itemid].flags & ITEM_FLAG6)
        {
            if(s->id==wBrang || s->id==wHookshot)
            {
                int itemid = ((weapon*)s)->parentitem>-1 ? ((weapon*)s)->parentitem :
                             directWpn>-1 ? directWpn : current_item_id(s->id==wHookshot ? itype_hookshot : itype_brang);
                itemid = vbound(itemid, 0, MAXITEMS-1);
                
                for(int j=0; j<Ewpns.Count(); j++)
                {
                    sprite *t = Ewpns.spr(j);
                    
                    if(s->hit(t->x+7,t->y+7,t->z,2,2,1))
                    {
                        bool reflect = false;
                       // sethitLinkUID(HIT_BY_EWEAPON,j); //set that Link was hit by a specific eweapon index. 
                        switch(t->id)
                        {
                        case ewBrang:
                            if(!(itemsbuf[itemid].misc3 & shBRANG)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shBRANG) != 0);
                            goto killweapon;
                            
                        case ewArrow:
                            if(!(itemsbuf[itemid].misc3 & shARROW)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shARROW) != 0);
                            goto killweapon;
                            
                        case ewRock:
                            if(!(itemsbuf[itemid].misc3 & shROCK)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shROCK) != 0);
                            goto killweapon;
                            
                        case ewFireball2:
                        case ewFireball:
                        {
                            int mask = (((weapon*)t)->type&1 ? shFIREBALL2 : shFIREBALL);
                            
                            if(!(itemsbuf[itemid].misc3 & mask)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & mask) != 0);
                            goto killweapon;
                        }
                        
                        case ewSword:
                            if(!(itemsbuf[itemid].misc3 & shSWORD)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shSWORD) != 0);
                            goto killweapon;
                            
                        case wRefMagic:
                        case ewMagic:
                            if(!(itemsbuf[itemid].misc3 & shMAGIC)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shMAGIC) != 0);
                            goto killweapon;
                            
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
                            if(!(itemsbuf[itemid].misc3 & shSCRIPT)) break;
                            
                            reflect = ((itemsbuf[itemid].misc4 & shSCRIPT) != 0);
                            goto killweapon;
                            
                        case ewLitBomb:
                        case ewLitSBomb:
killweapon:
                            ((weapon*)s)->dead=1;
                            weapon *ew = ((weapon*)t);
                            int oldid = ew->id;
                            ew->onhit(true, reflect ? 2 : 1, s->dir);
                            
                            /*if (s->dummy_bool[0])
                            {
                              add_grenade(s->x,s->y,s->z,0,-1);
                              s->dummy_bool[0]=false;
                            }*/
                            if(ew->id != oldid || (ew->id>=wScript1 && ew->id<=wScript10)) // changed type from ewX to wX... Except for script weapons
                            {
                                Lwpns.add(ew);
                                Ewpns.remove(ew);
                            }
                            
                            if(ew->id==wRefMagic)
                            {
                                ew->ignoreLink=true;
                                ew->ignorecombo=-1;
                            }
                            
                            break;
                        }
                        
                        break;
                    }
                }
            }
        }
        
        if((itemsbuf[itemid].flags & ITEM_FLAG2)||(itemid==-1&&get_bit(quest_rules,qr_OUCHBOMBS)))
        {
            //     if(((s->id==wBomb)||(s->id==wSBomb)) && (superman ? s->hit(x+7,y+7,z,2,2,1) : s->hit(this)))
            if(((s->id==wBomb)||(s->id==wSBomb)) && s->hit(this) && !superman && (scriptcoldet&1))
            {
                if(NayrusLoveShieldClk<=0)
                {
                    int ringpow = ringpower(((((weapon*)s)->parentitem>-1 ? itemsbuf[((weapon*)s)->parentitem].misc3 : ((weapon*)s)->power) *HP_PER_HEART));
                    game->set_life(zc_min(game->get_maxlife(), zc_max(game->get_life()-ringpow,0)));
                }
                
                hitdir = s->hitdir(x,y,16,16,dir);
                
                if (action != rafting && action != freeze)
                {
                    if (action == swimming || hopclk == 0xFF)
		    {
                        action=swimhit; FFCore.setLinkAction(swimhit);
		    }
                    else
		    {
                        action=gothit; FFCore.setLinkAction(gothit);
		    }
                }
                    
                if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
                {
                    spins = charging = attackclk = 0;
                    attack=none;
                    tapping = false;
                }
                
                hclk=48;
                sfx(getHurtSFX(),pan(int(x)));
                return;
            }
        }
        
        if(hclk==0 && s->id==wWind && s->hit(x+7,y+7,z,2,2,1) && !fairyclk)
        {
            reset_hookshot();
            xofs=1000;
            action=inwind; FFCore.setLinkAction(inwind);
            dir=s->dir;
            spins = charging = attackclk = 0;
            
            // In case Link used two whistles in a row, summoning two whirlwinds,
            // check which whistle's whirlwind picked him up so the correct
            // warp ring will be used
            int whistle=((weapon*)s)->parentitem;
            
            if(whistle>-1 && itemsbuf[whistle].family==itype_whistle)
                whistleitem=whistle;
                
            return;
        }
    }
    
    if(action==rafting || action==freeze ||
            action==casting || action==drowning || superman || !(scriptcoldet&1))
        return;
        
    int hit2 = diagonalMovement?GuyHit(x+4,y+4,z,8,8,hzsz):GuyHit(x+7,y+7,z,2,2,hzsz);
    
    
    
    if(hit2!=-1)
    {
        hitlink(hit2);
        return;
    }
    
    hit2 = LwpnHit();
    
    if(hit2!=-1)
    {
        if(NayrusLoveShieldClk<=0)
        {
            int ringpow = ringpower(lwpn_dp(hit2));
            game->set_life(zc_max(game->get_life()-ringpow,0));
	    sethitLinkUID(HIT_BY_LWEAPON,(hit2+1));  //this is first readable after waitdraw. 
		//Z_scripterrlog("lweapon hit2 is: %d\n", hit2*10000);
		//Z_scripterrlog("Link->HitBy[LWPN] is: %d\n", gethitLinkUID(HIT_BY_LWEAPON));
            
        }
        
        hitdir = Lwpns.spr(hit2)->hitdir(x,y,16,16,dir);
        ((weapon*)Lwpns.spr(hit2))->onhit(false);
        
        if(action==swimming || hopclk==0xFF)
	{
            action=swimhit; FFCore.setLinkAction(swimhit);
	}
        else
	{
            action=gothit; FFCore.setLinkAction(gothit);
	}
            
        hclk=48;
        
        if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
        {
            spins = charging = attackclk = 0;
            attack=none;
            tapping = false;
        }
        
        sfx(getHurtSFX(),pan(int(x)));
        return;
    }
    
    //else  { sethitLinkUID(HIT_BY_LWEAPON,(0));  //fails to clear
    
    hit2 = EwpnHit();
    
    if(hit2!=-1)
    {
        if(NayrusLoveShieldClk<=0)
        {
            int ringpow = ringpower(ewpn_dp(hit2));
            game->set_life(zc_max(game->get_life()-ringpow,0));
	    sethitLinkUID(HIT_BY_EWEAPON,(hit2+1)); //this is first readable after waitdraw. 
		//Z_scripterrlog("wweapon hit2 is: %d\n", hit2*10000);
		//Z_scripterrlog("Link->HitBy[EWPN] is: %d\n", gethitLinkUID(HIT_BY_EWEAPON));
        }
        
        hitdir = Ewpns.spr(hit2)->hitdir(x,y,16,16,dir);
        ((weapon*)Ewpns.spr(hit2))->onhit(false);
        
        if(action==swimming || hopclk==0xFF)
	{
            action=swimhit; FFCore.setLinkAction(swimhit);
	}
        else
	{
            action=gothit; FFCore.setLinkAction(gothit);
	}
            
        hclk=48;
        
        if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
        {
            spins = charging = attackclk = 0;
            attack=none;
            tapping = false;
        }
        
        sfx(getHurtSFX(),pan(int(x)));
        return;
    }
    //else { sethitLinkUID(HIT_BY_EWEAPON,(0)); } //fails to clear
    
    // The rest of this method deals with damage combos, which can be jumped over.
    if(z>0 && !(tmpscr->flags2&fAIRCOMBOS)) return;
    
    int dx1 = (int)x+8-(tmpscr->csensitive);
    int dx2 = (int)x+8+(tmpscr->csensitive-1);
    int dy1 = (int)y+(bigHitbox?8:12)-(bigHitbox?tmpscr->csensitive:(tmpscr->csensitive+1)/2);
    int dy2 = (int)y+(bigHitbox?8:12)+(bigHitbox?tmpscr->csensitive-1:((tmpscr->csensitive+1)/2)-1);
    
    for(int i=get_bit(quest_rules, qr_DMGCOMBOLAYERFIX) ? 1 : -1; i>=-1; i--)  // Layers 0, 1 and 2!!
        (void)checkdamagecombos(dx1,dx2,dy1,dy2,i);
}

bool LinkClass::checkdamagecombos(int dx, int dy)
{
    return checkdamagecombos(dx,dx,dy,dy);
}

bool LinkClass::checkdamagecombos(int dx1, int dx2, int dy1, int dy2, int layer, bool solid) //layer = -1, solid = false
{
    if(hclk || superman)
        return false;
        
    int hp_mod[4];
    
    hp_mod[0]=combo_class_buf[combobuf[layer>-1?MAPCOMBO2(layer,dx1,dy1):MAPCOMBO(dx1,dy1)].type].modify_hp_amount;
    hp_mod[1]=combo_class_buf[combobuf[layer>-1?MAPCOMBO2(layer,dx1,dy2):MAPCOMBO(dx1,dy2)].type].modify_hp_amount;
    hp_mod[2]=combo_class_buf[combobuf[layer>-1?MAPCOMBO2(layer,dx2,dy1):MAPCOMBO(dx2,dy1)].type].modify_hp_amount;
    hp_mod[3]=combo_class_buf[combobuf[layer>-1?MAPCOMBO2(layer,dx2,dy2):MAPCOMBO(dx2,dy2)].type].modify_hp_amount;
    
    int hp_modtotal=0;
    
    for(int i=0; i<4; i++)
    {
        if(get_bit(quest_rules,qr_DMGCOMBOPRI))
        {
            if(hp_modtotal >= 0)
                hp_modtotal = zc_min(hp_modtotal, hp_mod[i]);
            else if(hp_mod[i] < 0)
                hp_modtotal = zc_max(hp_modtotal, hp_mod[i]);
        }
        else
            hp_modtotal = zc_min(hp_modtotal, hp_mod[i]);
    }
    
    hp_mod[0]=combo_class_buf[combobuf[MAPFFCOMBO(dx1,dy1)].type].modify_hp_amount;
    hp_mod[1]=combo_class_buf[combobuf[MAPFFCOMBO(dx1,dy2)].type].modify_hp_amount;
    hp_mod[2]=combo_class_buf[combobuf[MAPFFCOMBO(dx2,dy1)].type].modify_hp_amount;
    hp_mod[3]=combo_class_buf[combobuf[MAPFFCOMBO(dx2,dy2)].type].modify_hp_amount;
    int hp_modtotalffc = 0;
    
    for(int i=0; i<4; i++)
    {
        if(get_bit(quest_rules,qr_DMGCOMBOPRI))
        {
            if(hp_modtotalffc >= 0)
                hp_modtotalffc = zc_min(hp_modtotalffc, hp_mod[i]);
            else if(hp_mod[i] < 0)
                hp_modtotalffc = zc_max(hp_modtotalffc, hp_mod[i]);
        }
        else
            hp_modtotalffc = zc_min(hp_modtotalffc, hp_mod[i]);
    }
    
    int hp_modmin = zc_min(hp_modtotal, hp_modtotalffc);
    
    bool global_ring = (get_bit(quest_rules,qr_RINGAFFECTDAMAGE) != 0);
    bool current_ring = ((tmpscr->flags6&fTOGGLERINGDAMAGE) != 0);
    
    int itemid = current_item_id(itype_boots);
    
    bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & ITEM_FLAG1);
    
    if(hp_modmin<0)
    {
        if((itemid<0) || (tmpscr->flags5&fDAMAGEWITHBOOTS) || (4<<current_item_power(itype_boots)<(abs(hp_modmin))) || (solid && bootsnosolid) || !checkmagiccost(itemid))
        {
            if(NayrusLoveShieldClk<=0)
            {
                int ringpow = ringpower(-hp_modmin);
                game->set_life(zc_max(game->get_life()-(global_ring!=current_ring ? ringpow:-hp_modmin),0));
            }
            
            hitdir = (dir^1);
            
            if (action != rafting && action != freeze)
            {
                if (action == swimming || hopclk == 0xFF)
		{
                    action=swimhit; FFCore.setLinkAction(swimhit);
		}
                else
		{
                    action=gothit; FFCore.setLinkAction(gothit);
		}
            }
                
                
            hclk=48;
            
            if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
            {
                spins = charging = attackclk = 0;
                attack=none;
                tapping = false;
            }
            
            sfx(getHurtSFX(),pan(int(x)));
            return true;
        }
        else paymagiccost(itemid); // Boots are successful
    }
    
    return false;
}

void LinkClass::hitlink(int hit2)
{
//printf("Stomp check: %d <= 12, %d < %d\n", int((y+16)-(((enemy*)guys.spr(hit2))->y)), (int)falling_oldy, (int)y);
    if(current_item(itype_stompboots) && checkmagiccost(current_item(itype_stompboots)) && (stomping ||
            (z > (((enemy*)guys.spr(hit2))->z)) ||
            ((isSideview() && (y+16)-(((enemy*)guys.spr(hit2))->y)<=14) && falling_oldy<y)))
    {
        int itemid = current_item_id(itype_stompboots);
        paymagiccost(itemid);
        hit_enemy(hit2,wStomp,itemsbuf[itemid].power*DAMAGE_MULTIPLIER,x,y,0,itemid);
        
        if(itemsbuf[itemid].flags & ITEM_DOWNGRADE)
            game->set_item(itemid,false);
            
        // Stomp Boots script
        if(itemsbuf[itemid].script != 0)
        {
            ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[itemid].script, itemid & 0xFFF);
        }
        
        return;
    }
    else if(superman || !(scriptcoldet&1))
        return;
    else if(NayrusLoveShieldClk<=0)
    {
        int ringpow = ringpower(enemy_dp(hit2));
        game->set_life(zc_max(game->get_life()-ringpow,0));
	sethitLinkUID(HIT_BY_NPC,(hit2+1)); //this is first readable after waitdraw. 
	    //Z_scripterrlog("lweapon hit2 is: %d\n", hit2*10000);
		//Z_scripterrlog("Link->HitBy[NPC] is: %d\n", gethitLinkUID(HIT_BY_NPC));
	    
    }
    
    hitdir = guys.spr(hit2)->hitdir(x,y,16,16,dir);
    if(action==swimming || hopclk==0xFF)
    {
        action=swimhit; FFCore.setLinkAction(swimhit);
    }
    else
    {
        action=gothit; FFCore.setLinkAction(gothit);
    }
        
    hclk=48;
    sfx(getHurtSFX(),pan(int(x)));
    
    if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
    {
        spins = charging = attackclk = 0;
        attack=none;
        tapping = false;
    }
    
    enemy_scored(hit2);
    int dm7 = ((enemy*)guys.spr(hit2))->dmisc7;
    int dm8 = ((enemy*)guys.spr(hit2))->dmisc8;
    
    switch(((enemy*)guys.spr(hit2))->family)
    {
    case eeWALLM:
        if(((enemy*)guys.spr(hit2))->hp>0)
        {
            GrabLink(hit2);
            inwallm=true;
            action=none; FFCore.setLinkAction(none);
        }
        
        break;
        
        //case eBUBBLEST:
        //case eeBUBBLE:
    case eeWALK:
    {
        int itemid = current_item_id(itype_whispring);
        //I can only assume these are supposed to be int, not bool ~pkmnfrk
        int sworddivisor = ((itemid>-1 && itemsbuf[itemid].misc1 & 1) ? itemsbuf[itemid].power : 1);
        int itemdivisor = ((itemid>-1 && itemsbuf[itemid].misc1 & 2) ? itemsbuf[itemid].power : 1);
        
        switch(dm7)
        {
        case e7tTEMPJINX:
            if(dm8==0 || dm8==2)
                if(swordclk>=0 && !(sworddivisor==0))
                    swordclk=150;
                    
            if(dm8==1 || dm8==2)
                if(itemclk>=0 && !(itemdivisor==0))
                    itemclk=150;
                    
            break;
            
        case e7tPERMJINX:
            if(dm8==0 || dm8==2)
                if(sworddivisor) swordclk=(itemid >-1 && itemsbuf[itemid].flags & ITEM_FLAG1)? int(150/sworddivisor) : -1;
                
            if(dm8==1 || dm8==2)
                if(itemdivisor) itemclk=(itemid >-1 && itemsbuf[itemid].flags & ITEM_FLAG1)? int(150/itemdivisor) : -1;
                
            break;
            
        case e7tUNJINX:
            if(dm8==0 || dm8==2)
                swordclk=0;
                
            if(dm8==1 || dm8==2)
                itemclk=0;
                
            break;
            
        case e7tTAKEMAGIC:
            game->change_dmagic(-dm8*game->get_magicdrainrate());
            break;
            
        case e7tTAKERUPEES:
            game->change_drupy(-dm8);
            break;
            
        case e7tDRUNK:
            drunkclk += dm8;
            break;
        }
        
        if(dm7 >= e7tEATITEMS)
        {
            EatLink(hit2);
            inlikelike=(dm7 == e7tEATHURT ? 2:1);
            action=none; FFCore.setLinkAction(none);
        }
    }
    }
}

void LinkClass::addsparkle(int wpn)
{
    weapon *w = (weapon*)Lwpns.spr(wpn);
    int itemid = w->parentitem;
    
    if(itemid<0)
        return;
        
    int itemtype = itemsbuf[itemid].family;
    
    if(itemtype!=itype_cbyrna && frame%4)
        return;
        
    int wpn2 = (itemtype==itype_cbyrna) ? itemsbuf[itemid].wpn4 : itemsbuf[itemid].wpn2;
    int wpn3 = (itemtype==itype_cbyrna) ? itemsbuf[itemid].wpn5 : itemsbuf[itemid].wpn3;
    // Either one (wpn2) or the other (wpn3). If both are present, randomise.
    int sparkle_type = (!wpn2 ? (!wpn3 ? 0 : wpn3) : (!wpn3 ? wpn2 : (rand()&1 ? wpn2 : wpn3)));
    int direction=w->dir;
    
    if(sparkle_type)
    {
        int h=0;
        int v=0;
        
        if(w->dir==right||w->dir==r_up||w->dir==r_down)
        {
            h=-1;
        }
        
        if(w->dir==left||w->dir==l_up||w->dir==l_down)
        {
            h=1;
        }
        
        if(w->dir==down||w->dir==l_down||w->dir==r_down)
        {
            v=-1;
        }
        
        if(w->dir==up||w->dir==l_up||w->dir==r_up)
        {
            v=1;
        }
        
        // Damaging boomerang sparkle?
        if(wpn3 && itemtype==itype_brang)
        {
            // If the boomerang just bounced, flip the sparkle direction so it doesn't hit
            // whatever it just bounced off of if it's shielded from that direction.
            if(w->misc==1 && w->clk2>256 && w->clk2<272)
                direction=oppositeDir[direction];
        }
        
        Lwpns.add(new weapon((fix)(w->x+(itemtype==itype_cbyrna ? 2 : rand()%4)+(h*4)),
                             (fix)(w->y+(itemtype==itype_cbyrna ? 2 : rand()%4)+(v*4)),
                             w->z,sparkle_type==wpn3 ? wFSparkle : wSSparkle,sparkle_type,0,direction,itemid,getUID()));
    }
}

// For wPhantoms
void LinkClass::addsparkle2(int type1, int type2)
{
    if(frame%4) return;
    
    int arrow = -1;
    
    for(int i=0; i<Lwpns.Count(); i++)
    {
        weapon *w = (weapon*)Lwpns.spr(i);
        
        if(w->id == wPhantom && w->type == type1)
        {
            arrow = i;
            break;
        }
    }
    
    if(arrow==-1)
    {
        return;
    }
    
    Lwpns.add(new weapon((fix)((Lwpns.spr(arrow)->x-3)+(rand()%7)),
                         (fix)((Lwpns.spr(arrow)->y-3)+(rand()%7)),
                         Lwpns.spr(arrow)->z, wPhantom, type2,0,0,((weapon*)Lwpns.spr(arrow))->parentitem,-1));
}

// returns true when game over
bool LinkClass::animate(int)
{
    int lsave=0;
    
    if(do_cheat_goto)
    {
        didpit=true;
        pitx=x;
        pity=y;
        dowarp(3,0);
        do_cheat_goto=false;
        return false;
    }
    
    if(do_cheat_light)
    {
        naturaldark = !naturaldark;
        lighting(false, false);
        do_cheat_light = false;
    }
    
    if(action!=climbcovertop&&action!=climbcoverbottom)
    {
        climb_cover_x=-1000;
        climb_cover_y=-1000;
    }
    
    if(isGrassType(COMBOTYPE(x,y+15)) && isGrassType(COMBOTYPE(x+15,y+15))&& z<=8)
    {
        if(decorations.idCount(dTALLGRASS)==0)
        {
            decorations.add(new dTallGrass(x, y, dTALLGRASS, 0));
        }
    }
    
    if((COMBOTYPE(x,y+15)==cSHALLOWWATER)&&(COMBOTYPE(x+15,y+15)==cSHALLOWWATER) && z==0)
    {
        if(decorations.idCount(dRIPPLES)==0)
        {
            decorations.add(new dRipples(x, y, dRIPPLES, 0));
        }
    }
    
    if(stomping)
        stomping = false;
        
    if(isSideview())  // Sideview gravity
    {
        // Fall, unless on a ladder, rafting, using the hookshot, drowning or cheating.
        if(!(toogam && Up()) && !drownclk && action!=rafting && !pull_link && !((ladderx || laddery) && fall>0))
        {
            int ydiff = fall/(spins && fall<0 ? 200:100);
            falling_oldy = y; // Stomp Boots-related variable
            y+=ydiff;
            hs_starty+=ydiff;
            
            for(int j=0; j<chainlinks.Count(); j++)
            {
                chainlinks.spr(j)->y+=ydiff;
            }
            
            if(Lwpns.idFirst(wHookshot)>-1)
            {
                Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=ydiff;
            }
            
            if(Lwpns.idFirst(wHSHandle)>-1)
            {
                Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=ydiff;
            }
        }
        
        // Stop hovering/falling if you land on something.
        if(ON_SIDEPLATFORM && !(pull_link && dir==down) && action!=rafting)
        {
            stop_item_sfx(itype_hoverboots);
            fall = hoverclk = jumping = 0;
            y-=(int)y%8; //fix position
            
            if(y>=160 && currscr>=0x70 && !(tmpscr->flags2&wfDOWN))  // Landed on the bottommost screen.
                y = 160;
        }
        // Stop hovering if you press down.
        else if((hoverclk || ladderx || laddery) && DrunkDown())
        {
            stop_item_sfx(itype_hoverboots);
            hoverclk = 0;
            reset_ladder();
            fall = zinit.gravity;
        }
        // Continue falling.
        else if(fall <= (int)zinit.terminalv)
        {
            if(fall != 0 || hoverclk)
                jumping++;
                
            // Bump head if: hit a solid combo from beneath, or hit a solid combo in the screen above this one.
            if((_walkflag(x+4,y-(bigHitbox?9:1),0)
                || (y<=(bigHitbox?9:1) &&
                // Extra checks if Smart Screen Scrolling is enabled
                 (nextcombo_wf(up) || ((get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)) &&
                                               !(tmpscr->flags2&wfUP)) && (nextcombo_solid(up))))))
                    && fall < 0)
            {
                fall = 0; // Bumped his head
                
                // ... maybe on spikes
                checkdamagecombos(x+4, x+12, y-1, y-1);
            }
            
            if(hoverclk)
            {
                if(hoverclk > 0)
                {
                    hoverclk--;
                }
                
                if(!hoverclk && !ladderx && !laddery)
                {
                    fall += zinit.gravity;
                }
            }
            else if(fall+int(zinit.gravity) > 0 && fall<=0 && can_use_item(itype_hoverboots,i_hoverboots) && !ladderx && !laddery)
            {
                fall = jumping = 0;
                int itemid = current_item_id(itype_hoverboots);
                hoverclk = itemsbuf[itemid].misc1 ? itemsbuf[itemid].misc1 : -1;
                
                if(itemsbuf[itemid].wpn)
                    decorations.add(new dHover(x, y, dHOVER, 0));
                    
                sfx(itemsbuf[itemid].usesound,pan(int(x)));
            }
            else if(!ladderx && !laddery)
            {
                fall += zinit.gravity;
            }
        }
    }
    else // Topdown gravity
    {
        z-=fall/(spins && fall>0 ? 200:100);
        
        if(z>0)
        {
            switch(action)
            {
            case swimming:
	    {
                diveclk=0;
                action=walking; FFCore.setLinkAction(walking);
		    
                break;
	    }
            case waterhold1:
	    {
                action=landhold1; FFCore.setLinkAction(landhold1);
                break;
	    }
                
            case waterhold2:
	    {
                action=landhold2; FFCore.setLinkAction(landhold2);
                break;
	    }
                
            default:
                break;
            }
        }
        
        for(int j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->z=z;
        }
        
        if(Lwpns.idFirst(wHookshot)>-1)
        {
            Lwpns.spr(Lwpns.idFirst(wHookshot))->z=z;
        }
        
        if(Lwpns.idFirst(wHSHandle)>-1)
        {
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->z=z;
        }
        
        if(z<=0)
        {
            if(fall > 0)
            {
                if((iswater(MAPCOMBO(x,y+8)) && ladderx<=0 && laddery<=0) || COMBOTYPE(x,y+8)==cSHALLOWWATER)
                    sfx(WAV_ZN1SPLASH,int(x));
                    
                stomping = true;
            }
            
            z = fall = jumping = hoverclk = 0;
        }
        else if(fall <= (int)zinit.terminalv)
        {
            if(fall != 0 || hoverclk)
                jumping++;
                
            if(hoverclk)
            {
                if(hoverclk > 0)
                {
                    hoverclk--;
                }
                
                if(!hoverclk)
                {
                    fall += zinit.gravity;
                }
            }
            else if(fall+(int)zinit.gravity > 0 && fall<=0 && can_use_item(itype_hoverboots,i_hoverboots))
            {
                fall = 0;
                int itemid = current_item_id(itype_hoverboots);
                hoverclk = itemsbuf[itemid].misc1 ? itemsbuf[itemid].misc1 : -1;
                decorations.add(new dHover(x, y, dHOVER, 0));
                sfx(itemsbuf[current_item_id(itype_hoverboots)].usesound,pan(int(x)));
            }
            else fall += zinit.gravity;
        }
    }
    
    if(drunkclk)
    {
        --drunkclk;
    }
    
    if(link_is_stunned > 0)
    {
	    actiontype lastaction = action; //cache the last action so that we can compare against it
	    
	    if( lastaction != freeze )  //stun sets freeze, so we only want to store a tempaction that was not freeze
	    {
		    tempaction=lastaction; //update so future checks won't do this
		    //action=freeze; //setting this somehow makes the FFCore link action 'swimming' ?!
		    FFCore.setLinkAction(stunned);
		    Z_scripterrlog("The stunned action is: %d\n", (int)stunned);
		    Z_scripterrlog("The present FFCore action is: %d\n", FFCore.getLinkAction());
	    }
	    --link_is_stunned;
    }
    //if the stun action is still set in FFCore, but he isn't stunned, then the timer reached 0
    //, so we unfreeze him here, and return him to the action that he had when he was stunned. 
    if ( FFCore.getLinkAction() == stunned && !link_is_stunned )
    {
	action=tempaction; FFCore.setLinkAction(tempaction);
	     Z_scripterrlog("Unfreezing link to action: %d\n", (int)tempaction);
       
	//action=none; FFCore.setLinkAction(none);
	    
    }
    
    if(!is_on_conveyor && !diagonalMovement && (fall==0 || z>0) && charging==0 && spins<=5
            && action != gothit)
    {
        switch(dir)
        {
        case up:
        case down:
            x=(int(x)+4)&0xFFF8;
            break;
            
        case left:
        case right:
            y=(int(y)+4)&0xFFF8;
            break;
        }
    }
    
    if((watch==true) && clockclk)
    {
        --clockclk;
        
        if(!clockclk)
        {
            if(cheat_superman==false)
            {
                setClock(false);
            }
            
            watch=false;
            
            for(int i=0; i<eMAXGUYS; i++)
            {
                for(int zoras=0; zoras<clock_zoras[i]; zoras++)
                {
                    addenemy(0,0,i,0);
                }
            }
        }
    }
    
    if(hookshot_frozen==true)
    {
        if(hookshot_used==true)
        {
            action=freeze; FFCore.setLinkAction(freeze); //could be LA_HOOKSHOT for FFCore. -Z
            
            if(pull_link==true)
            {
                sprite *t;
                int i;
                
                for(i=0; i<Lwpns.Count() && (Lwpns.spr(i)->id!=wHSHandle); i++)
                {
                    /* do nothing */
                }
                
                t = Lwpns.spr(i);
                
                for(i=0; i<Lwpns.Count(); i++)
                {
                    sprite *s = Lwpns.spr(i);
                    
                    if(s->id==wHookshot)
                    {
                        if((s->y)>y)
                        {
                            y+=4;
                            
                            if(Lwpns.idFirst(wHSHandle)!=-1)
                            {
                                t->y+=4;
                            }
                            
                            hs_starty+=4;
                        }
                        
                        if((s->y)<y)
                        {
                            y-=4;
                            
                            if(Lwpns.idFirst(wHSHandle)!=-1)
                            {
                                t->y-=4;
                            }
                            
                            hs_starty-=4;
                        }
                        
                        if((s->x)>x)
                        {
                            x+=4;
                            
                            if(Lwpns.idFirst(wHSHandle)!=-1)
                            {
                                t->x+=4;
                            }
                            
                            hs_startx+=4;
                        }
                        
                        if((s->x)<x)
                        {
                            x-=4;
                            
                            if(Lwpns.idFirst(wHSHandle)!=-1)
                            {
                                t->x-=4;
                            }
                            
                            hs_startx-=4;
                        }
                    }
                }
            }
        }
        else
        {
            Lwpns.del(Lwpns.idFirst(wHSHandle));
            reset_hookshot();
        }
        
        if(hs_fix)
        {
            if(dir==up)
            {
                y=int(y+7)&0xF0;
            }
            
            if(dir==down)
            {
                y=int(y+7)&0xF0;
            }
            
            if(dir==left)
            {
                x=int(x+7)&0xF0;
            }
            
            if(dir==right)
            {
                x=int(x+7)&0xF0;
            }
            
            hs_fix=false;
        }
        
    }
    
    if(DrunkrLbtn() && !get_bit(quest_rules,qr_SELECTAWPN))
        selectNextBWpn(SEL_LEFT);
    else if(DrunkrRbtn() && !get_bit(quest_rules,qr_SELECTAWPN))
        selectNextBWpn(SEL_RIGHT);
        
    if(rPbtn())
    
        // #define PBUTTONDEBUG
        
#ifndef PBUTTONDEBUG
        onViewMap();
        
#else
        /* This is here to allow me to output something to allegro.log on demand. */
    {
        al_trace("**********\n");
    }
#endif
        
    for(int i=0; i<Lwpns.Count(); i++)
    {
        weapon *w = ((weapon*)Lwpns.spr(i));
        
        if(w->id == wArrow || w->id == wBrang || w->id == wCByrna)
            addsparkle(i);
    }
    
    if(Lwpns.idCount(wPhantom))
    {
        addsparkle2(pDINSFIREROCKET,pDINSFIREROCKETTRAIL);
        addsparkle2(pDINSFIREROCKETRETURN,pDINSFIREROCKETTRAILRETURN);
        addsparkle2(pNAYRUSLOVEROCKET1,pNAYRUSLOVEROCKETTRAIL1);
        addsparkle2(pNAYRUSLOVEROCKET2,pNAYRUSLOVEROCKETTRAIL2);
        addsparkle2(pNAYRUSLOVEROCKETRETURN1,pNAYRUSLOVEROCKETTRAILRETURN1);
        addsparkle2(pNAYRUSLOVEROCKETRETURN2,pNAYRUSLOVEROCKETTRAILRETURN2);
    }
    
    // Pay magic cost for Byrna beams
    
    //Byrna needs a secondary timer, for 254+, as do all items that reduce MP on a per-frae basis. Essentially, we will do % divisor == 0 for that. -Z
    if(Lwpns.idCount(wCByrna))
    {
        weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wCByrna)));
        int itemid = ew->parentitem;
        
        if(!checkmagiccost(itemid))
        {
            for(int i=0; i<Lwpns.Count(); i++)
            {
                weapon *w = ((weapon*)Lwpns.spr(i));
                
                if(w->id==wCByrna)
                    w->dead=1;
            }
        }
	else paymagiccost(itemid);
	
    }
    
    
    ClearhitLinkUIDs(); //clear them before we advance. 
    checkhit();
    
    if(game->get_life()<=0)
    {
        // So scripts can have one frame to handle hp zero events
        if(false == (last_hurrah = !last_hurrah))
        {
            drunkclk=0;
	    link_is_stunned = 0;
	    FFCore.setLinkAction(dying);
            gameover();
            
            return true;
        }
    }
    else last_hurrah=false;
    
    if(swordclk>0)
        --swordclk;
        
    if(itemclk>0)
        --itemclk;
        
    if(inwallm)
    {
        attackclk=0;
        linkstep();
        
        if(CarryLink()==false)
            restart_level();
            
        return false;
    }
    
    if(ewind_restart)
    {
        attackclk=0;
        restart_level();
        xofs=0;
        action=none; FFCore.setLinkAction(none);
        ewind_restart=false;
        return false;
    }
    
    
    if(hopclk)
    {
        action=hopping; FFCore.setLinkAction(hopping);
    }
        
    // get user input or do other animation
    freeze_guys=false;                                        // reset this flag, set it again if holding
    
    if(action != landhold1 && action != landhold2 && action != waterhold1 && action != waterhold2 && fairyclk==0 && holdclk>0)
    {
        holdclk=0;
    }
    
    switch(action)
    {
    case gothit:
        if(attackclk)
            if(!doattack())
            {
                attackclk=spins=0;
                tapping=false;
            }
            
        break;
        
    case drowning:
    {
        linkstep(); // maybe this line should be elsewhere?
        
        if(--drownclk==0)
        {
            action=none; FFCore.setLinkAction(none);
            x=entry_x;
            y=entry_y;
            warpx=x;
            warpy=y;
            hclk=48;
            game->set_life(zc_max(game->get_life()-(HP_PER_HEART/4),0));
        }
        
        break;
    }  
    case swimhit:
    case freeze:
    case scrolling:
        break;
        
    case casting:
    {
        if(magicitem==-1)
        {
            action=none; FFCore.setLinkAction(none);
        }
        
        break;
    }
    case landhold1:
    case landhold2:
    {
        if(--holdclk <= 0)
        {
            //restart music
            if(get_bit(quest_rules, qr_HOLDNOSTOPMUSIC) == 0 && (specialcave < GUYCAVE))
                playLevelMusic();
                
            action=none; FFCore.setLinkAction(none);
        }
        else
            freeze_guys=true;
            
        break;
    }
    case waterhold1:
    case waterhold2:
    {
        diveclk=0;
        
        if(--holdclk <= 0)
        {
            //restart music
            if(get_bit(quest_rules, qr_HOLDNOSTOPMUSIC) == 0  && (specialcave < GUYCAVE))
                playLevelMusic();
                
            action=swimming; FFCore.setLinkAction(swimming);
        }
        else
            freeze_guys=true;
            
        break;
    }
    case hopping:
    {
        if(DRIEDLAKE)
        {
            action=none; FFCore.setLinkAction(none);
            hopclk = 0;
            diveclk = 0;
            break;
        }
        
        do_hopping();
        break;
    }
    case inwind:
    {
        int i=Lwpns.idFirst(wWind);
        
        if(i<0)
        {
            bool exit=false;
            
            if(whirlwind==255)
            {
                exit=true;
            }
            else if(y<=0 && dir==up) y=-1;
            else if(y>=160 && dir==down) y=161;
            else if(x<=0 && dir==left) x=-1;
            else if(x>=240 && dir==right) x=241;
            else exit=true;
            
            if(exit)
            {
                action=none; FFCore.setLinkAction(none);
                xofs=0;
                whirlwind=0;
                lstep=0;
                if ( dontdraw < 2 ) dontdraw=0;
                entry_x=x;
                entry_y=y;
            }
        }
        /*
              else if (((weapon*)Lwpns.spr(i))->dead==1)
              {
                whirlwind=255;
              }
        */
        else
        {
            x=Lwpns.spr(i)->x;
            y=Lwpns.spr(i)->y;
            dir=Lwpns.spr(i)->dir;
        }
    }
    break;
    
    case swimming:
	    
        if(DRIEDLAKE)
        {
            action=none; FFCore.setLinkAction(none);
            hopclk=0;
            break;
        }
        
        if(frame&1)
            linkstep();
            
        // fall through
        
    default:
        movelink();                                           // call the main movement routine
    }
    
    // check for ladder removal
    if(diagonalMovement)
    {
        if(ladderx+laddery)
        {
            if(ladderdir==up)
            {
                if((laddery-int(y)>=(16+(ladderstart==dir?ladderstart==down?1:0:0))) || (laddery-int(y)<=(-16-(ladderstart==dir?ladderstart==up?1:0:0))) || (abs(ladderx-int(x))>8))
                {
                    reset_ladder();
                }
            }
            else
            {
                if((abs(laddery-int(y))>8) || (ladderx-int(x)>=(16+(ladderstart==dir?ladderstart==right?1:0:0))) || (ladderx-int(x)<=(-16-(ladderstart==dir?ladderstart==left?1:0:0))))
                {
                    reset_ladder();
                }
            }
        }
    }
    else
    {
        if((abs(laddery-int(y))>=16) || (abs(ladderx-int(x))>=16))
        {
            reset_ladder();
        }
    }
    
    if(ilswim)
        landswim++;
    else landswim=0;
    
    if(hopclk!=0xFF) ilswim=false;
    
    if((!loaded_guys) && (frame - newscr_clk >= 1))
    {
        if(tmpscr->room==rGANON)
        {
            ganon_intro();
        }
        else
        {
            loadguys();
        }
    }
    
    if((!loaded_enemies) && (frame - newscr_clk >= 2))
    {
        loadenemies();
    }
    
    // check lots of other things
    checkscroll();
    
    if(action!=inwind && action!=drowning)
    {
        checkspecial();
        checkitems();
        checklocked(); //This has issues if Link's action is WALKING, in 8-way moveent. 
        checklockblock();
        checkbosslockblock();
        checkchest(cCHEST);
        checkchest(cLOCKEDCHEST);
        checkchest(cBOSSCHEST);
        checkpushblock();
        checkswordtap();
        
        if(hookshot_frozen==false)
        {
            checkspecial2(&lsave);
        }
        
        if(action==won)
        {
            return true;
        }
    }
    
    // Somehow Link was displaced from the fairy flag...
    if(fairyclk && action != freeze)
    {
        fairyclk = holdclk = refill_why = 0;
    }
    
    if((!activated_timed_warp) && (tmpscr->timedwarptics>0))
    {
        tmpscr->timedwarptics--;
        
        if(tmpscr->timedwarptics==0)
        {
            activated_timed_warp=true;
            
            if(tmpscr->flags4 & fTIMEDDIRECT)
            {
                didpit=true;
                pitx=x;
                pity=y;
            }
            
            int index2 = 0;
            
            if(tmpscr->flags5 & fRANDOMTIMEDWARP) index2=rand()%4;
            
            sdir = dir;
            dowarp(1,index2);
        }
    }
    
    bool awarp = false;
    
    for(int i=0; i<176; i++)
    {
    
        int ind=0;
        
        if(!awarp)
        {
            if(combobuf[tmpscr->data[i]].type==cAWARPA)
            {
                awarp=true;
                ind=0;
            }
            else if(combobuf[tmpscr->data[i]].type==cAWARPB)
            {
                awarp=true;
                ind=1;
            }
            else if(combobuf[tmpscr->data[i]].type==cAWARPC)
            {
                awarp=true;
                ind=2;
            }
            else if(combobuf[tmpscr->data[i]].type==cAWARPD)
            {
                awarp=true;
                ind=3;
            }
            else if(combobuf[tmpscr->data[i]].type==cAWARPR)
            {
                awarp=true;
                ind=rand()%4;
            }
            
            if(awarp)
            {
                if(tmpscr->flags5&fDIRECTAWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir = dir;
                dowarp(1,ind);
            }
        }
        
    }
    
    awarp=false;
    
    for(int i=0; i<32; i++)
    {
        int ind=0;
        
        if(!awarp)
        {
            if(combobuf[tmpscr->ffdata[i]].type==cAWARPA)
            {
                awarp=true;
                ind=0;
            }
            else if(combobuf[tmpscr->ffdata[i]].type==cAWARPB)
            {
                awarp=true;
                ind=1;
            }
            else if(combobuf[tmpscr->ffdata[i]].type==cAWARPC)
            {
                awarp=true;
                ind=2;
            }
            else if(combobuf[tmpscr->ffdata[i]].type==cAWARPD)
            {
                awarp=true;
                ind=3;
            }
            else if(combobuf[tmpscr->ffdata[i]].type==cAWARPR)
            {
                awarp=true;
                ind=rand()%4;
            }
            
            if(awarp)
            {
                if(tmpscr->flags5&fDIRECTAWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir = dir;
                dowarp(1,ind);
            }
        }
        
    }
    
    if(ffwarp)
    {
        if(ffpit)
        {
            ffpit=false;
            didpit=true;
            pitx=x;
            pity=y;
        }
        
        ffwarp=false;
        dowarp(1,0);
    }
    
    // walk through bombed doors and fake walls
    bool walk=false;
    int dtype=dBOMBED;
    
    if(pushing>=24) dtype=dWALK;
    
    if(isdungeon() && action!=freeze && loaded_guys && !inlikelike && !diveclk && action!=rafting)
    {
        if(((dtype==dBOMBED)?DrunkUp():dir==up) && (diagonalMovement?x>112&&x<128:x==120) && y<=32 && tmpscr->door[0]==dtype)
        {
            walk=true;
            dir=up;
        }
        
        if(((dtype==dBOMBED)?DrunkDown():dir==down) && (diagonalMovement?x>112&&x<128:x==120) && y>=128 && tmpscr->door[1]==dtype)
        {
            walk=true;
            dir=down;
        }
        
        if(((dtype==dBOMBED)?DrunkLeft():dir==left) && x<=32 && (diagonalMovement?y>72&&y<88:y==80) && tmpscr->door[2]==dtype)
        {
            walk=true;
            dir=left;
        }
        
        if(((dtype==dBOMBED)?DrunkRight():dir==right) && x>=208 && (diagonalMovement?y>72&&y<88:y==80) && tmpscr->door[3]==dtype)
        {
            walk=true;
            dir=right;
        }
    }
    
    if(walk)
    {
        hclk=0;
        drawguys=false;
        
        if(dtype==dWALK)
        {
            sfx(tmpscr->secretsfx);
        }
        
        action=none; FFCore.setLinkAction(none);
        stepforward(29, true);
        action=scrolling; FFCore.setLinkAction(scrolling);
        pushing=false;
    }
    
    if(game->get_life()<=(HP_PER_HEART) && !(game->get_maxlife()<=(HP_PER_HEART)))
    {
        if(heart_beep)
        {
            cont_sfx(WAV_ER);
        }
        else
        {
            if(heart_beep_timer==-1)
            {
                heart_beep_timer=70;
            }
            
            if(heart_beep_timer>0)
            {
                --heart_beep_timer;
                cont_sfx(WAV_ER);
            }
            else
            {
                stop_sfx(WAV_ER);
            }
        }
    }
    else
    {
        heart_beep_timer=-1;
        stop_sfx(WAV_ER);
    }
    
    if(rSbtn())
    {
        int tmp_subscr_clk = frame;
        
        switch(lsave)
        {
        case 0:
            if ( !stopSubscreenFalling() ){
		    conveyclk=3;
		    dosubscr(&QMisc);
		    newscr_clk += frame - tmp_subscr_clk;
		    break;
		}
		else break;
            
            
        case 1:
            save_game((tmpscr->flags4&fSAVEROOM) != 0, 0);
            break;
            
        case 2:
            save_game((tmpscr->flags4&fSAVEROOM) != 0, 1);
            break;
        }
    }
    
    checkstab();
    
    check_conveyor();
    return false;
}

// A routine used exclusively by startwpn,
// to switch Link's weapon if his current weapon (bombs) was depleted.
void LinkClass::deselectbombs(int super)
{
    if(getItemFamily(itemsbuf,Bwpn&0x0FFF)==(super? itype_sbomb : itype_bomb) && (directWpn<0 || Bwpn==directWpn))
    {
        int temp = selectWpn_new(SEL_VERIFY_LEFT, game->bwpn, game->awpn);
        Bwpn = Bweapon(temp);
        directItemB = directItem;
        game->bwpn = temp;
    }
    else
    {
        int temp = selectWpn_new(SEL_VERIFY_LEFT, game->awpn, game->bwpn);
        Awpn = Bweapon(temp);
        directItemA = directItem;
        game->awpn = temp;
    }
}

int potion_life=0;
int potion_magic=0;

bool LinkClass::startwpn(int itemid)
{
    if(itemid < 0) return false;
    
    if(((dir==up && y<24) || (dir==down && y>128) ||
            (dir==left && x<32) || (dir==right && x>208)) && !(get_bit(quest_rules,qr_ITEMSONEDGES) || inlikelike))
        return false;
        
    int wx=x;
    int wy=y;
    int wz=z;
    bool ret = true;
    
    switch(dir)
    {
    case up:
        wy-=16;
        break;
        
    case down:
        wy+=16;
        break;
        
    case left:
        wx-=16;
        break;
        
    case right:
        wx+=16;
        break;
    }
    
    bool use_hookshot=true;
    
    switch(itemsbuf[itemid].family)
    {
    case itype_potion:
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        
        if(itemsbuf[itemid].misc1 || itemsbuf[itemid].misc2)
        {
            refill_what=REFILL_ALL;
            refill_why=itemid;
            StartRefill(REFILL_ALL);
            potion_life = game->get_life();
            potion_magic = game->get_magic();
            
            //add a quest rule or an item option that lets you specify whether or not to pause music during refilling
            //music_pause();
            while(refill())
            {
                put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,sspUP);
                advanceframe(true);
            }
            
            //add a quest rule or an item option that lets you specify whether or not to pause music during refilling
            //music_resume();
            ret = false;
        }
        
        break;
        
    case itype_rocs:
    {
        if(!inlikelike && z==0 && charging==0 && !(isSideview() && !ON_SIDEPLATFORM && !ladderx && !laddery) && hoverclk==0)
        {
            if(!checkmagiccost(itemid))
                return false;
                
            paymagiccost(itemid);
            fall -= FEATHERJUMP*(itemsbuf[itemid].power+2);
            
            // Reset the ladder, unless on an unwalkable combo
            if((ladderx || laddery) && !(_walkflag(ladderx,laddery,0)))
                reset_ladder();
                
            sfx(itemsbuf[itemid].usesound,pan(int(x)));
        }
        
        ret = false;
    }
    break;
    
    case itype_letter:
    {
        if(current_item(itype_letter)==i_letter &&
                tmpscr[currscr<128?0:1].room==rP_SHOP &&
                tmpscr[currscr<128?0:1].guy &&
                ((currscr<128&&!(DMaps[currdmap].flags&dmfGUYCAVES))||(currscr>=128&&DMaps[currdmap].flags&dmfGUYCAVES))
          )
        {
            int usedid = getItemID(itemsbuf, itype_letter,i_letter+1);
            
            if(usedid != -1)
                getitem(usedid, true);
                
            sfx(tmpscr[currscr<128?0:1].secretsfx);
            setupscreen();
            action=none; FFCore.setLinkAction(none);
        }
        
        ret = false;
    }
    break;
    
    case itype_whistle:
    {
        bool whistleflag;
        
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        sfx(itemsbuf[itemid].usesound);
        
        if(dir==up || dir==right)
            ++blowcnt;
        else
            --blowcnt;
            
        while(sfx_allocated(itemsbuf[itemid].usesound))
        {
            advanceframe(true);
            
            if(Quit)
                return false;
        }
        
        Lwpns.add(new weapon(x,y,z,wWhistle,0,0,dir,itemid,getUID()));
        
        if(whistleflag=findentrance(x,y,mfWHISTLE,false))
            didstuff |= did_whistle;
            
        if((didstuff&did_whistle && itemsbuf[itemid].flags&ITEM_FLAG1) || currscr>=128)
            return false;
            
        if(itemsbuf[itemid].flags&ITEM_FLAG1) didstuff |= did_whistle;
        
        if((tmpscr->flags&fWHISTLE) || (tmpscr->flags7 & fWHISTLEWATER)
                || (tmpscr->flags7&fWHISTLEPAL))
        {
            whistleclk=0;                                       // signal to start drying lake or doing other stuff
        }
        else
        {
            int where = itemsbuf[itemid].misc1;
            
            if(where>right) where=dir^1;
            
            if(((DMaps[currdmap].flags&dmfWHIRLWIND && TriforceCount()) || DMaps[currdmap].flags&dmfWHIRLWINDRET) &&
                    itemsbuf[itemid].misc2 >= 0 && itemsbuf[itemid].misc2 <= 8 && !whistleflag)
                Lwpns.add(new weapon((fix)(where==left?240:where==right?0:x),(fix)(where==down?0:where==up?160:y),
                                     (fix)0,wWind,0,0,where,itemid,getUID()));
                                     
            whistleitem=itemid;
        }
        
        ret = false;
    }
    break;
    
    case itype_bomb:
    {
        //Remote detonation
        if(Lwpns.idCount(wLitBomb) >= zc_max(itemsbuf[itemid].misc2,1))
        {
            weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitBomb)));
            
            while(Lwpns.idCount(wLitBomb) && ew->misc == 0)
            {
                ew->misc=50;
                ew->clk=ew->misc-3;
                ew->id=wBomb;
                ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitBomb)));
            }
            
            deselectbombs(false);
            return false;
        }
        
        //Remote bombs:
        //Even if you have no bombs, the icon remains so that you can detonate laid bombs.
        //But the remaining code requires at least one bomb.
        if(!game->get_bombs() && !current_item_power(itype_bombbag))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        
        if(!get_debug() && !current_item_power(itype_bombbag))
            game->change_bombs(-1);
            
        if(itemsbuf[itemid].misc1>0) // If not remote bombs
            deselectbombs(false);
            
        if(isdungeon())
        {
            wy=zc_max(wy,16);
        }
        
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wLitBomb,itemsbuf[itemid].fam_type,
                             itemsbuf[itemid].power*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
        sfx(WAV_PLACE,pan(wx));
    }
    break;
    
    case itype_sbomb:
    {
        //Remote detonation
        if(Lwpns.idCount(wLitSBomb) >= zc_max(itemsbuf[itemid].misc2,1))
        {
            weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitSBomb)));
            
            while(Lwpns.idCount(wLitSBomb) && ew->misc == 0)
            {
                ew->misc=50;
                ew->clk=ew->misc-3;
                ew->id=wSBomb;
                ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitSBomb)));
            }
            
            deselectbombs(true);
            return false;
        }
        
        //Remote bombs:
        //Even if you have no bombs, the icon remains so that you can detonate laid bombs.
        //But the remaining code requires at least one bomb.
        bool magicbag = (current_item_power(itype_bombbag)
                         && itemsbuf[current_item_id(itype_bombbag)].flags & ITEM_FLAG1);
                         
        if(!game->get_sbombs() && !magicbag)
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        
        if(!get_debug() && !magicbag)
            game->change_sbombs(-1);
            
        if(itemsbuf[itemid].misc1>0) // If not remote bombs
            deselectbombs(true);
            
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wLitSBomb,itemsbuf[itemid].fam_type,itemsbuf[itemid].power*DAMAGE_MULTIPLIER,dir, itemid,getUID()));
        sfx(WAV_PLACE,pan(wx));
    }
    break;
    
    case itype_wand:
    {
        if(Lwpns.idCount(wMagic))
            return false;
            
        int bookid = current_item_id(itype_book);
        bool paybook = (bookid>-1 && checkmagiccost(bookid));
        
        if(!(itemsbuf[itemid].flags&ITEM_FLAG1) && !paybook)  //Can the wand shoot without the book?
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        if(Lwpns.idCount(wBeam))
            Lwpns.del(Lwpns.idFirst(wBeam));
        
	//This needs an ER -V
	//Patched with QRs
	int type, pow;
	if ( get_bit(quest_rules,qr_BROKENBOOKCOST) )
	{
		type = bookid != -1 ? current_item(itype_book) : itemsbuf[itemid].fam_type;
		pow = (bookid != -1 ? current_item_power(itype_book) : itemsbuf[itemid].power)*DAMAGE_MULTIPLIER;
	}
	else
	{
		//@Venrob:
		//Wait, so, why are you using current_item(itype_book) and not itemsbuf[bookid].whatever? 
		//There is an actual field on the magic book and/or wand for the damage modification!! -Z
		type = (bookid != -1 && paybook) ? current_item(itype_book) : itemsbuf[itemid].fam_type;
		pow = ((bookid != -1 && paybook) ? current_item_power(itype_book) : itemsbuf[itemid].power)*DAMAGE_MULTIPLIER;
	}
        for(int i=(spins==1?up:dir); i<=(spins==1 ? right:dir); i++)
            if(dir!=(i^1))
	    {
		weapon *magic = new weapon((fix)wx,(fix)wy,(fix)wz,wMagic,type,pow,i, itemid,getUID());
		if(paybook)magic->miscellaneous[31] = bookid;
                Lwpns.add(magic);
	    }
                
        paymagiccost(itemid);
        
        if(paybook)
            paymagiccost(current_item_id(itype_book));
            
        if(bookid != -1)
            sfx(itemsbuf[bookid].usesound,pan(wx));
        else
            sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    /*
    //    Fireball Wand
    Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wRefFireball,0,2*DAMAGE_MULTIPLIER,dir));
    switch (dir)
    {
    case up:
      Lwpns.spr(Lwpns.Count()-1)->angle=-PI/2;
      Lwpns.spr(Lwpns.Count()-1)->dir=up;
      break;
    case down:
      Lwpns.spr(Lwpns.Count()-1)->angle=PI/2;
      Lwpns.spr(Lwpns.Count()-1)->dir=down;
      break;
    case left:
      Lwpns.spr(Lwpns.Count()-1)->angle=PI;
      Lwpns.spr(Lwpns.Count()-1)->dir=left;
      break;
    case right:
      Lwpns.spr(Lwpns.Count()-1)->angle=0;
      Lwpns.spr(Lwpns.Count()-1)->dir=right;
      break;
    }
    Lwpns.spr(Lwpns.Count()-1)->clk=16;
    ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step=3.5;
    Lwpns.spr(Lwpns.Count()-1)->dummy_bool[0]=true; //homing
    */
    break;
    
    case itype_sword:
    {
        if(!checkmagiccost(itemid))
            return false;
            
        if((Lwpns.idCount(wBeam) && spins==0)||Lwpns.idCount(wMagic))
            return false;
            
        paymagiccost(itemid);
        float temppower;
        
        if(itemsbuf[itemid].flags & ITEM_FLAG2)
        {
            temppower=DAMAGE_MULTIPLIER*itemsbuf[itemid].power;
            temppower=temppower*itemsbuf[itemid].misc2;
            temppower=temppower/100;
        }
        else
        {
            temppower = DAMAGE_MULTIPLIER*itemsbuf[itemid].misc2;
        }
        
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wBeam,itemsbuf[itemid].fam_type,int(temppower),dir,itemid,getUID()));
        sfx(WAV_BEAM,pan(wx));
    }
    break;
    
    case itype_candle:
    {
        if(itemsbuf[itemid].flags&ITEM_FLAG1 && didstuff&did_candle)
        {
            return false;
        }
        
        if(Lwpns.idCount(wFire)>=2)
        {
            return false;
        }
        
        if(!checkmagiccost(itemid))
        {
            return false;
        }
        
        paymagiccost(itemid);
        
        if(itemsbuf[itemid].flags&ITEM_FLAG1) didstuff|=did_candle;
        
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wFire,
                             (itemsbuf[itemid].fam_type > 1), //To do with combo flags
                             itemsbuf[itemid].power*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
        sfx(itemsbuf[itemid].usesound,pan(wx));
        attack=wFire;
    }
    break;
    
    case itype_script1:
    {
        if(Lwpns.idCount(wScript1))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript1,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script2:
    {
        if(Lwpns.idCount(wScript2))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript2,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script3:
    {
        if(Lwpns.idCount(wScript3))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript3,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script4:
    {
        if(Lwpns.idCount(wScript4))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript4,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script5:
    {
        if(Lwpns.idCount(wScript5))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript5,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script6:
    {
        if(Lwpns.idCount(wScript6))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript6,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script7:
    {
        if(Lwpns.idCount(wScript7))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript7,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script8:
    {
        if(Lwpns.idCount(wScript8))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript8,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script9:
    {
        if(Lwpns.idCount(wScript9))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript9,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_script10:
    {
        if(Lwpns.idCount(wScript10))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wScript10,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_icerod:
    {
        if(Lwpns.idCount(wIce))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
	
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wIce,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itemsbuf[itemid].misc1;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    
    break;
    
    case itype_arrow:
    {
        if(Lwpns.idCount(wArrow) > itemsbuf[itemid].misc2)
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
        
        if(get_bit(quest_rules,qr_TRUEARROWS) && !current_item_power(itype_quiver))
        {
            if(game->get_arrows()<=0)
                return false;
                
            game->change_arrows(-1);
        }
        else if(!current_item_power(itype_quiver) && !current_item_power(itype_wallet))
        {
            if(game->get_drupy()+game->get_rupies()<=0)
                return false;
                
            game->change_drupy(-1);
        }
        
        paymagiccost(itemid);
        
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wArrow,itemsbuf[itemid].fam_type,DAMAGE_MULTIPLIER*itemsbuf[itemid].power,dir,itemid,getUID()));
        ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step*=(current_item_power(itype_bow)+1)/2;
        sfx(itemsbuf[itemid].usesound,pan(wx));
    }
    break;
    
    case itype_bait:
        if(Lwpns.idCount(wBait)) //TODO: More than one Bait per screen?
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        sfx(itemsbuf[itemid].usesound,pan(wx));
        
        if(tmpscr->room==rGRUMBLE && !getmapflag())
        {
            items.add(new item((fix)wx,(fix)wy,(fix)0,iBait,ipDUMMY+ipFADE,0));
            fadeclk=66;
            dismissmsg();
            clear_bitmap(pricesdisplaybuf);
            set_clip_state(pricesdisplaybuf, 1);
            //    putscr(scrollbuf,0,0,tmpscr);
            setmapflag();
            removeItemsOfFamily(game,itemsbuf,itype_bait);
            verifyBothWeapons();
            sfx(tmpscr->secretsfx);
            return false;
        }
        
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wBait,0,0,dir,itemid,getUID()));
        break;
        
    case itype_brang:
    {
        if(Lwpns.idCount(wBrang) > itemsbuf[itemid].misc2)
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        current_item_power(itype_brang);
        Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wBrang,itemsbuf[itemid].fam_type,(itemsbuf[itemid].power*DAMAGE_MULTIPLIER),dir,itemid,getUID()));
    }
    break;
    
    case itype_hookshot:
        if(inlikelike || Lwpns.idCount(wHookshot))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        
        for(int i=-1; i<2; i++)
        {
            if(dir==up)
            {
                if((combobuf[MAPCOMBO2(i,x,y-7)].type==cHSGRAB)||
                        (_walkflag(x+2,y+4,1) && !ishookshottable(int(x),int(y+4))))
                {
                    use_hookshot=false;
                }
            }
            else if(dir==down)
            {
                if((combobuf[MAPCOMBO2(i,x+12,y+23)].type==cHSGRAB))
                {
                    use_hookshot=false;
                }
            }
            else if(dir==left)
            {
                if((combobuf[MAPCOMBO2(i,x-7,y+12)].type==cHSGRAB))
                {
                    use_hookshot=false;
                }
            }
            else if(dir==right)
            {
                if((combobuf[MAPCOMBO2(i,x+23,y+12)].type==cHSGRAB))
                {
                    use_hookshot=false;
                }
            }
        }
        
        if(use_hookshot)
        {
            int hookitem = itemsbuf[itemid].fam_type;
            int hookpower = itemsbuf[itemid].power;
            
            if(Lwpns.Count()>=SLMAX)
            {
                Lwpns.del(0);
            }
            
            if(Lwpns.Count()>=SLMAX-1)
            {
                Lwpns.del(0);
            }
            
            if(dir==up)
            {
                hookshot_used=true;
                Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                Lwpns.add(new weapon((fix)wx,(fix)wy-4,(fix)wz,wHookshot,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                hs_startx=wx;
                hs_starty=wy-4;
            }
            
            if(dir==down)
            {
                int offset=get_bit(quest_rules,qr_HOOKSHOTDOWNBUG)?4:0;
                hookshot_used=true;
                Lwpns.add(new weapon((fix)wx,(fix)wy+offset,(fix)wz,wHSHandle,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                Lwpns.add(new weapon((fix)wx,(fix)wy+offset,(fix)wz,wHookshot,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                hs_startx=wx;
                hs_starty=wy;
            }
            
            if(dir==left)
            {
                hookshot_used=true;
                Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                Lwpns.add(new weapon((fix)(wx-4),(fix)wy,(fix)wz,wHookshot,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                hs_startx=wx-4;
                hs_starty=wy;
            }
            
            if(dir==right)
            {
                hookshot_used=true;
                Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wHSHandle,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                Lwpns.add(new weapon((fix)(wx+4),(fix)wy,(fix)wz,wHookshot,hookitem,
                                     hookpower*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
                hs_startx=wx+4;
                hs_starty=wy;
            }
            
            hookshot_frozen=true;
        }
        
        break;
        
    case itype_dinsfire:
        if(z!=0 || (isSideview() && !ON_SIDEPLATFORM))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        action=casting; FFCore.setLinkAction(casting);
        magicitem=itemid;
        break;
        
    case itype_faroreswind:
        if(z!=0 || (isSideview() && !ON_SIDEPLATFORM))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        action=casting; FFCore.setLinkAction(casting);
        magicitem=itemid;
        break;
        
    case itype_nayruslove:
        if(z!=0 || (isSideview() && !ON_SIDEPLATFORM))
            return false;
            
        if(!checkmagiccost(itemid))
            return false;
            
        paymagiccost(itemid);
        action=casting; FFCore.setLinkAction(casting);
        magicitem=itemid;
        break;
        
    case itype_cbyrna:
    {
        //Beams already deployed
        if(Lwpns.idCount(wCByrna))
        {
            stopCaneOfByrna();
            return false;
        }
        
        if(!checkmagiccost(itemid))
            return false;
	paymagiccost(itemid);
        
        for(int i=0; i<itemsbuf[itemid].misc3; i++)
            Lwpns.add(new weapon((fix)wx,(fix)wy,(fix)wz,wCByrna,i,itemsbuf[itemid].power*DAMAGE_MULTIPLIER,dir,itemid,getUID()));
    }
    break;
    
    default:
        ret = false;
    }
    
    if(itemsbuf[itemid].flags & ITEM_DOWNGRADE)
    {
        game->set_item(itemid,false);
        
        // Maybe Item Override has allowed the same item in both slots?
        if(Bwpn == itemid)
        {
            Bwpn = 0;
            verifyBWpn();
        }
        
        if(Awpn == itemid)
        {
            Awpn = 0;
            verifyAWpn();
        }
    }
    
    return ret;
}


bool LinkClass::doattack()
{
    //int s = BSZ ? 0 : 11;
    int s = (zinit.linkanimationstyle==las_bszelda) ? 0 : 11;
    
    // Abort attack if attackclk has run out and:
    // * the attack is not Hammer, Sword with Spin Scroll, Candle, or Wand, OR
    // * you aren't holding down the A button, you're not charging, and/or you're still spinning
    
    if(attackclk>=(spins>0?8:14) && attack!=wHammer &&
            (((attack!=wSword || !current_item(itype_spinscroll) || inlikelike) && attack!=wWand && attack!=wFire && attack!=wCByrna) || !((attack==wSword && isWpnPressed(itype_sword) && spins==0) || charging>0)))
    {
        tapping=false;
        return false;
    }
    
    if(attackclk>29)
    {
        tapping=false;
        return false;
    }
    
    int candleid = (directWpn>-1 && itemsbuf[directWpn].family==itype_candle) ? directWpn : current_item_id(itype_candle);
    int byrnaid = (directWpn>-1 && itemsbuf[directWpn].family==itype_cbyrna) ? directWpn : current_item_id(itype_cbyrna);
    
    // An attack can be "walked out-of" after 8 frames, unless it's:
    // * a sword stab
    // * a hammer pound
    // * a wand thrust
    // * a candle thrust
    // * a cane thrust
    // In which case it should continue.
    if((attack==wCatching && attackclk>4)||(attack!=wWand && attack!=wSword && attack!=wHammer
                                            && (attack!=wFire || (candleid!=-1 && !(itemsbuf[candleid].wpn)))
                                            && (attack!=wCByrna || (byrnaid!=-1 && !(itemsbuf[byrnaid].wpn))) && attackclk>7))
    {
        if(DrunkUp()||DrunkDown()||DrunkLeft()||DrunkRight())
        {
            lstep = s;
            return false;
        }
    }
    
    if(charging==0)
    {
        lstep=0;
    }
    
    // Work out the sword charge-up delay
    int magiccharge = 192, normalcharge = 64;
    int itemid = current_item_id(itype_chargering);
    
    if(itemid>=0)
    {
        normalcharge = itemsbuf[itemid].misc1;
        magiccharge = itemsbuf[itemid].misc2;
    }
    
    itemid = current_item_id(attack==wHammer ? itype_quakescroll : itype_spinscroll);
    
    bool doCharge=true;
    if(z!=0)
        doCharge=false;
    if(attack==wSword)
    {
        if(!(attackclk==SWORDCHARGEFRAME && isWpnPressed(itype_sword)))
            doCharge=false;
        else if(charging<=normalcharge)
        {
            if(itemid<0 || !checkmagiccost(itemid))
                doCharge=false;
        }
    }
    else if(attack==wHammer)
    {
        if(!(attackclk==HAMMERCHARGEFRAME && isWpnPressed(itype_hammer)))
            doCharge=false;
        else if(charging<=normalcharge)
        {
            if(itemid<0 || !checkmagiccost(itemid))
                doCharge=false;
        }
    }
    else
        doCharge=false;
    
    // Now work out the magic cost
    itemid = current_item_id(attack==wHammer ? itype_quakescroll : itype_spinscroll);
    
    // charging up weapon...
    //
    if(doCharge)
    {
        // Increase charging while holding down button.
        if(spins==0 && charging<magiccharge)
            charging++;
            
        // Once a charging threshold is reached, play the sound.
        if(charging==normalcharge)
        {
            paymagiccost(itemid);
            sfx(WAV_ZN1CHARGE,pan(int(x)));
        }
        else if(charging==magiccharge)
        {
            itemid = current_item_id(attack==wHammer ? itype_quakescroll2 : itype_spinscroll2);
            
            if(itemid>-1 && checkmagiccost(itemid))
            {
                paymagiccost(itemid);
                charging++; // charging>magiccharge signifies a successful supercharge.
                sfx(WAV_ZN1CHARGE2,pan(int(x)));
            }
        }
    }
    else if(attack==wCByrna && byrnaid!=-1)
    {
        if(!(itemsbuf[byrnaid].wpn))
        {
            attack = wNone;
            return startwpn(attackid); // Beam if the Byrna stab animation WASN'T used.
        }
        
        bool beamcount = false;
        
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = ((weapon*)Lwpns.spr(i));
            
            if(w->id==wCByrna)
            {
                beamcount = true;
                break;
            }
        }
        
        // If beams already deployed, remove them
        if(!attackclk && beamcount)
        {
            return startwpn(attackid); // Remove beams instantly
        }
        
        // Otherwise, continue
        ++attackclk;
    }
    else
    {
        ++attackclk;
        
        if(attackclk==SWORDCHARGEFRAME && charging>0 && !tapping)  //Signifies a tapped enemy
        {
            ++attackclk; // Won't continue charging
            charging=0;
        }
        
        // Faster if spinning.
        if(spins>0)
            ++attackclk;
            
        // Even faster if hurricane spinning.
        if(spins>5)
            attackclk+=2;
            
        // If at a charging threshold, do a charged attack.
        if(charging>=normalcharge && (attack!=wSword || attackclk>=SWORDCHARGEFRAME) && !tapping)
        {
            if(attack==wSword)
            {
                spins=(charging>magiccharge ? (itemsbuf[current_item_id(itype_spinscroll2)].misc1*4)-3
                       : (itemsbuf[current_item_id(itype_spinscroll)].misc1*4)+1);
                attackclk=1;
                sfx(itemsbuf[current_item_id(spins>5 ? itype_spinscroll2 : itype_spinscroll)].usesound,pan(int(x)));
            }
            /*
            else if(attack==wWand)
            {
                //Not reachable.. yet
                spins=1;
            }
            */
            else if(attack==wHammer && sideviewhammerpound())
            {
                spins=1; //signifies the quake hammer
                bool super = (charging>magiccharge && current_item(itype_quakescroll2));
                sfx(itemsbuf[current_item_id(super ? itype_quakescroll2 : itype_quakescroll)].usesound,pan(int(x)));
                quakeclk=(itemsbuf[current_item_id(super ? itype_quakescroll2 : itype_quakescroll)].misc1);
                
                // general area stun
                for(int i=0; i<GuyCount(); i++)
                {
                    if(!isflier(GuyID(i)))
                    {
                        StunGuy(i,(itemsbuf[current_item_id(super ? itype_quakescroll2 : itype_quakescroll)].misc2)-
                                distance(x,y,GuyX(i),GuyY(i)));
                    }
                }
            }
        }
        else if(tapping && attackclk<SWORDCHARGEFRAME && charging<magiccharge)
            charging++;
            
        if(!isWpnPressed(attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword))
            charging=0;
            
        if(attackclk>=SWORDCHARGEFRAME)
            tapping = false;
    }
    
    if(attackclk==1 && attack==wFire && candleid!=-1 && !(itemsbuf[candleid].wpn))
    {
        return startwpn(attackid); // Flame if the Candle stab animation WASN'T used.
    }
    
    int crossid = current_item_id(itype_crossscroll);  //has Cross Beams scroll
    
    if(attackclk==13 || (attackclk==7 && spins>1 && crossid >=0 && checkmagiccost(crossid)))
    {
    
        int wpnid = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? directWpn : current_item_id(itype_sword);
        long long templife = wpnid>=0? itemsbuf[wpnid].misc1 : 0;
        
        if(wpnid>=0 && itemsbuf[wpnid].flags & ITEM_FLAG1)
        {
            templife=templife*game->get_maxlife();
            templife=templife/100;
        }
        else
        {
            templife*=HP_PER_HEART;
        }
        
        bool normalbeam = (game->get_life()+(get_bit(quest_rules,qr_QUARTERHEART)?((HP_PER_HEART/4)-1):((HP_PER_HEART/2)-1))>=templife);
        int perilid = current_item_id(itype_perilscroll);
        bool perilbeam = (perilid>=0 && wpnid>=0 && game->get_life()<=itemsbuf[perilid].misc1*HP_PER_HEART
                          && checkmagiccost(perilid)
                          // Must actually be able to shoot sword beams
                          && ((itemsbuf[wpnid].flags & ITEM_FLAG1)
                              || itemsbuf[wpnid].misc1 <= game->get_maxlife()/HP_PER_HEART));
                              
        if(attack==wSword && !tapping && (perilbeam || normalbeam))
        {
            if(attackclk==7)
                paymagiccost(crossid); // Pay the Cross Beams magic cost.
                
            if(perilbeam && !normalbeam)
                paymagiccost(perilid); // Pay the Peril Beam magic cost.
                
            // TODO: Something that would be cheap but disgraceful to hack in at this point is
            // a way to make the peril/cross beam item's power stat influence the strength
            // of the peril/cross beam...
            startwpn(attackid);
        }
        
        if(attack==wWand)
            startwpn(attackid); // Flame if the Wand stab animation WAS used (it always is).
            
        if(attack==wFire && candleid!=-1 && itemsbuf[candleid].wpn) // Flame if the Candle stab animation WAS used.
            startwpn(attackid);
            
        if(attack==wCByrna && byrnaid!=-1 && itemsbuf[byrnaid].wpn) // Beam if the Byrna stab animation WAS used.
            startwpn(attackid);
    }
    
    if(attackclk==14)
        lstep = s;
        
    return true;
}

bool LinkClass::can_attack()
{
	int currentSwordOrWand = (itemsbuf[dowpn].family == itype_wand || itemsbuf[dowpn].family == itype_sword)?dowpn:-1;
    if(action==hopping || action==swimming || action==freeze || link_is_stunned > 0 ||
            (action==attacking && ((attack!=wSword && attack!=wWand) || !(get_bit(quest_rules,qr_QUICKSWORD)||(itemsbuf[currentSwordOrWand].flags & ITEM_FLAG5))) && charging!=0) || spins>0)
    {
        return false;
    }
    
    int r = (isdungeon()) ? 16 : 0;
    int r2 = get_bit(quest_rules, qr_NOBORDER) ? 0 : 8;
    
    if(!get_bit(quest_rules, qr_ITEMSONEDGES)) switch(dir)
        {
        case up:
        case down:
            return !(y<(r2+r) || y>(160-r-r2));
            
        case left:
        case right:
            return !(x<(r2+r) || x>(240-r-r2));
        }
        
    return true;
}

bool isRaftFlag(int flag)
{
    return (flag==mfRAFT || flag==mfRAFT_BRANCH || flag==mfRAFT_BOUNCE);
}

void do_lens()
{
	
	if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x250 )
	{
		int wpnPressed = getWpnPressed(itype_lens);
	    int itemid = lensid >= 0 ? lensid : wpnPressed>0 ? wpnPressed : Link.getLastLensID()>0 ? Link.getLastLensID() : current_item_id(itype_lens);
	    //printf("Item ID read:%d\nLastLensID:%d\nlensid:%d\ngetWpnPressed:%d\ndefault:%d\n\n",itemid,Link.getLastLensID(),lensid,getWpnPressed(itype_lens),current_item_id(itype_lens));
	    if(itemid<0)
		return;
		
	    if(isWpnPressed(itype_lens) && !LinkItemClk() && !lensclk && checkmagiccost(itemid))
	    {
		if(lensid<0)
		{
		    lensid=itemid;
		    if(itemsbuf[itemid].family == itype_lens)
			Link.setLastLensID(itemid);
		    if(get_bit(quest_rules,qr_MORESOUNDS)) sfx(itemsbuf[itemid].usesound);
		}
		
		paymagiccost(itemid);
		
		if(itemid>=0 && itemsbuf[itemid].script != 0 && !did_scriptl)
		{
		    ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[itemid].script, itemid & 0xFFF);
		    did_scriptl=true;
		}
		
		lensclk = 12;
	    }
	    else
	    {
		did_scriptl=false;
		
		if(lensid>-1 && !(isWpnPressed(itype_lens) && !LinkItemClk() && checkmagiccost(itemid)))
		{
		    lensid=-1;
		    lensclk = 0;
		    
		    if(get_bit(quest_rules,qr_MORESOUNDS)) sfx(WAV_ZN1LENSOFF);
		}
	    }
	}
	else //2.10 or earlier
	{
		do_210_lens();
	}
}
//Add 2.10 version check to call this
void do_210_lens()
{
    int itemid = lensid >= 0 ? lensid : directWpn>-1 ? directWpn : current_item_id(itype_lens);
    
    if(itemid<0)
        return;
        
    if(isWpnPressed(itype_lens) && !LinkItemClk() && !lensclk && checkmagiccost(itemid))
    {
        if(lensid<0)
        {
            lensid=itemid;
            
            if(get_bit(quest_rules,qr_MORESOUNDS)) sfx(itemsbuf[itemid].usesound);
        }
        
        paymagiccost(itemid);
        
        if(itemid>=0 && itemsbuf[itemid].script != 0 && !did_scriptl)
        {
            ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[itemid].script, itemid & 0xFFF);
            did_scriptl=true;
        }
        
        lensclk = 12;
    }
    else
    {
        did_scriptl=false;
        
        if(lensid>-1 && !(isWpnPressed(itype_lens) && !LinkItemClk() && checkmagiccost(itemid)))
        {
            lensid=-1;
            lensclk = 0;
            
            if(get_bit(quest_rules,qr_MORESOUNDS)) sfx(WAV_ZN1LENSOFF);
        }
    }
}

void LinkClass::do_hopping()
{
    do_lens();
    
    if(hopclk==0xFF) //|| (diagonalMovement && hopclk >= 0xFF) ))                                         // swimming
			//Possible fix for exiting water in diagonal movement. -Z
    {
        if(diveclk>0)
            --diveclk;
        else if(DrunkrAbtn())
        {
            bool global_diving=(get_bit(quest_rules,qr_NODIVING)>0 || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG1)>0);
            bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
            
            if(global_diving==screen_diving)
                diveclk=80;
        }
        
        if((!(int(x)&7) && !(int(y)&7)) || diagonalMovement)
        {
            action=swimming; FFCore.setLinkAction(swimming);
            hopclk = 0;
            charging = attackclk = 0;
            tapping = false;
        }
        else
        {
            linkstep();
            
            if(diveclk<=30 || (frame&1))
            {
                switch(dir)
                {
                case up:
                    y -= 1;
                    break;
                    
                case down:
                    y += 1;
                    break;
                    
                case left:
                    x -= 1;
                    break;
                    
                case right:
                    x += 1;
                    break;
                }
            }
        }
    }
    else                                                      // hopping in or out (need to separate the cases...)
    {
        if(diagonalMovement)
        {
            if(hopclk==1) //hopping out
		    //>= 1 possible fix for getting stuck on land edges. 
		//No, this is not a clock. it's a type. 1 == out, 2 == in. 
            {
                if(hopdir!=-1) dir=hopdir;
                
                landswim=0;
                
                if(dir==up)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(iswater(MAPCOMBO(x,y+(bigHitbox?0:8)-1)) && !iswater(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1)) && !iswater(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1)))
                        sidestep=1;
                    else if(!iswater(MAPCOMBO(x,y+(bigHitbox?0:8)-1)) && !iswater(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1)) && iswater(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1)))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y--;
                    
                    if(!iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&!iswater(MAPCOMBO(int(x),int(y)+15)))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setLinkAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==down)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(iswater(MAPCOMBO(x,y+16)) && !iswater(MAPCOMBO(x+8,y+16)) && !iswater(MAPCOMBO(x+15,y+16)))
                        sidestep=1;
                    else if(!iswater(MAPCOMBO(x,y+16)) && !iswater(MAPCOMBO(x+7,y+16)) && iswater(MAPCOMBO(x+15,y+16)))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y++;
                    
                    if(!iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&!iswater(MAPCOMBO(int(x),int(y)+15)))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setLinkAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==left)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(iswater(MAPCOMBO(x-1,y+(bigHitbox?0:8))) && !iswater(MAPCOMBO(x-1,y+(bigHitbox?8:12))) && !iswater(MAPCOMBO(x-1,y+15)))
                        sidestep=1;
                    else if(!iswater(MAPCOMBO(x-1,y+(bigHitbox?0:8))) && !iswater(MAPCOMBO(x-1,y+(bigHitbox?7:11))) && iswater(MAPCOMBO(x-1,y+15)))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x--;
                    
                    if(!iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&!iswater(MAPCOMBO(int(x)+15,int(y)+8)))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setLinkAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==right)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(iswater(MAPCOMBO(x+16,y+(bigHitbox?0:8))) && !iswater(MAPCOMBO(x+16,y+(bigHitbox?8:12))) && !iswater(MAPCOMBO(x+16,y+15)))
                        sidestep=1;
                    else if(!iswater(MAPCOMBO(x+16,y+(bigHitbox?0:8))) && !iswater(MAPCOMBO(x+16,y+(bigHitbox?7:11))) && iswater(MAPCOMBO(x+16,y+15)))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x++;
                    
                    if(!iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&!iswater(MAPCOMBO(int(x)+15,int(y)+8)))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setLinkAction(none);
                        hopdir=-1;
                    }
                }
            }
            
            if(hopclk==2) //hopping in
            {
                landswim=0;
                
                if(dir==up)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(!iswater(MAPCOMBO(x,y+(bigHitbox?0:8)-1)) && iswater(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1)) && iswater(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1)))
                        sidestep=1;
                    else if(iswater(MAPCOMBO(x,y+(bigHitbox?0:8)-1)) && iswater(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1)) && !iswater(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1)))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y--;
                    
                    if(iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&iswater(MAPCOMBO(int(x),int(y)+15)))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        action=swimming; FFCore.setLinkAction(swimming);
                    }
                }
                
                if(dir==down)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(!iswater(MAPCOMBO(x,y+16)) && iswater(MAPCOMBO(x+8,y+16)) && iswater(MAPCOMBO(x+15,y+16)))
                        sidestep=1;
                    else if(iswater(MAPCOMBO(x,y+16)) && iswater(MAPCOMBO(x+7,y+16)) && !iswater(MAPCOMBO(x+15,y+16)))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y++;
                    
                    if(iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&iswater(MAPCOMBO(int(x),int(y)+15)))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        reset_swordcharge();
                        action=swimming; FFCore.setLinkAction(swimming);
                    }
                }
                
                if(dir==left)
                {
                    linkstep();
                    linkstep();
                    int sidestep=0;
                    
                    if(!iswater(MAPCOMBO(x-1,y+(bigHitbox?0:8))) && iswater(MAPCOMBO(x-1,y+(bigHitbox?8:12))) && iswater(MAPCOMBO(x-1,y+15)))
                        sidestep=1;
                    else if(iswater(MAPCOMBO(x-1,y+(bigHitbox?0:8))) && iswater(MAPCOMBO(x-1,y+(bigHitbox?7:11))) && !iswater(MAPCOMBO(x-1,y+15)))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x--;
                    
                    if(iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&iswater(MAPCOMBO(int(x)+15,int(y)+8)))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        action=swimming; FFCore.setLinkAction(swimming);
                    }
                }
                
                if(dir==right)
                {
                    linkstep();
                    linkstep();
                    
                    int sidestep=0;
                    
                    if(!iswater(MAPCOMBO(x+16,y+(bigHitbox?0:8))) && iswater(MAPCOMBO(x+16,y+(bigHitbox?8:12))) && iswater(MAPCOMBO(x+16,y+15)))
                        sidestep=1;
                    else if(iswater(MAPCOMBO(x+16,y+(bigHitbox?0:8))) && iswater(MAPCOMBO(x+16,y+(bigHitbox?7:11))) && !iswater(MAPCOMBO(x+16,y+15)))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x++;
                    
                    if(iswater(MAPCOMBO(int(x),int(y)+(bigHitbox?0:8)))&&iswater(MAPCOMBO(int(x)+15,int(y)+8)))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        action=swimming; FFCore.setLinkAction(swimming);
                    }
                }
            }
            
        }
        else
        {
            if(dir<left ? !(int(x)&7) && !(int(y)&15) : !(int(x)&15) && !(int(y)&7))
            {
                action=none; FFCore.setLinkAction(none);
                hopclk = 0;
                diveclk = 0;
                
                if(iswater(MAPCOMBO(int(x),int(y)+8)))
                {
                    // hopped in
                    attackclk = charging = spins = 0;
                    action=swimming; FFCore.setLinkAction(swimming);
                }
            }
            else
            {
                linkstep();
                linkstep();
                
                if(++link_count>(16*link_animation_speed))
                    link_count=0;
                    
                int xofs2 = int(x)&15;
                int yofs2 = int(y)&15;
                int s = 1 + (frame&1);
                
                switch(dir)
                {
                case up:
                    if(yofs2<3 || yofs2>13) --y;
                    else y-=s;
                    
                    break;
                    
                case down:
                    if(yofs2<3 || yofs2>13) ++y;
                    else y+=s;
                    
                    break;
                    
                case left:
                    if(xofs2<3 || xofs2>13) --x;
                    else x-=s;
                    
                    break;
                    
                case right:
                    if(xofs2<3 || xofs2>13) ++x;
                    else x+=s;
                    
                    break;
                }
            }
        }
    }
}

void LinkClass::do_rafting()
{

    if(toogam)
    {
        action=none; FFCore.setLinkAction(none);
        return;
    }
    
    FFCore.setLinkAction(rafting);
    
    do_lens();
    
    linkstep();
    
    if(!(int(x)&15) && !(int(y)&15))
    {
        // this sections handles switching to raft branches
        if((MAPFLAG(x,y)==mfRAFT_BRANCH||MAPCOMBOFLAG(x,y)==mfRAFT_BRANCH))
        {
            if(dir!=down && DrunkUp() && (isRaftFlag(nextflag(x,y,up,false))||isRaftFlag(nextflag(x,y,up,true))))
            {
                dir = up;
                goto skip;
            }
            
            if(dir!=up && DrunkDown() && (isRaftFlag(nextflag(x,y,down,false))||isRaftFlag(nextflag(x,y,down,true))))
            {
                dir = down;
                goto skip;
            }
            
            if(dir!=right && DrunkLeft() && (isRaftFlag(nextflag(x,y,left,false))||isRaftFlag(nextflag(x,y,left,true))))
            {
                dir = left;
                goto skip;
            }
            
            if(dir!=left && DrunkRight() && (isRaftFlag(nextflag(x,y,right,false))||isRaftFlag(nextflag(x,y,right,true))))
            {
                dir = right;
                goto skip;
            }
        }
        else if((MAPFLAG(x,y)==mfRAFT_BOUNCE||MAPCOMBOFLAG(x,y)==mfRAFT_BOUNCE))
        {
            if(dir == left) dir = right;
            else if(dir == right) dir = left;
            else if(dir == up) dir = down;
            else if(dir == down) dir = up;
        }
        
        
        if(!isRaftFlag(nextflag(x,y,dir,false))&&!isRaftFlag(nextflag(x,y,dir,true)))
        {
            if(dir<left) //going up or down
            {
                if((isRaftFlag(nextflag(x,y,right,false))||isRaftFlag(nextflag(x,y,right,true))))
                    dir=right;
                else if((isRaftFlag(nextflag(x,y,left,false))||isRaftFlag(nextflag(x,y,left,true))))
                    dir=left;
                else if(y>0 && y<160) 
		{
                    action=none; FFCore.setLinkAction(none);
		}
            }
            else //going left or right
            {
                if((isRaftFlag(nextflag(x,y,down,false))||isRaftFlag(nextflag(x,y,down,true))))
                    dir=down;
                else if((isRaftFlag(nextflag(x,y,up,false))||isRaftFlag(nextflag(x,y,up,true))))
                    dir=up;
                else if(x>0 && x<240)
		{
                    action=none; FFCore.setLinkAction(none);
		}
            }
        }
    }
    
skip:

    switch(dir)
    {
    case up:
        if(int(x)&15)
        {
            if(int(x)&8)
                x++;
            else x--;
        }
        else	--y;
        
        break;
        
    case down:
        if(int(x)&15)
        {
            if(int(x)&8)
                x++;
            else x--;
        }
        else ++y;
        
        break;
        
    case left:
        if(int(y)&15)
        {
            if(int(y)&8)
                y++;
            else y--;
        }
        else --x;
        
        break;
        
    case right:
        if(int(y)&15)
        {
            if(int(y)&8)
                y++;
            else y--;
        }
        else ++x;
        
        break;
    }
}

void LinkClass::movelink()
{
    int xoff=int(x)&7;
    int yoff=int(y)&7;
    int push=pushing;
    int oldladderx=-1000, oldladdery=-1000; // moved here because linux complains "init crosses goto ~Koopa
    pushing=0;
    
    if(diveclk>0)
    {
        --diveclk;
    }
    else if(action==swimming)
    {
        bool global_diving=(get_bit(quest_rules,qr_NODIVING)>0 || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG1)>0);
        bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
        
        if(DrunkrAbtn()&&(global_diving==screen_diving))
        {
            diveclk=80;
        }
    }
    
    if(action==rafting)
    {
        do_rafting();
        
        if(action==rafting)
        {
            return;
        }
        
        setEntryPoints(x,y);
    }
    
    int olddirectwpn = directWpn; // To be reinstated if startwpn() fails
    int btnwpn = -1;
    
    //&0xFFF removes the "bow & arrows" bitmask
    //The Quick Sword is allowed to interrupt attacks.
    int currentSwordOrWand = (itemsbuf[dowpn].family == itype_wand || itemsbuf[dowpn].family == itype_sword)?dowpn:-1;
    if((!attackclk && action!=attacking) || ((attack==wSword || attack==wWand) && (get_bit(quest_rules,qr_QUICKSWORD)||(itemsbuf[currentSwordOrWand].flags & ITEM_FLAG5))))
    {
        if(DrunkrBbtn())
        {
            btnwpn=getItemFamily(itemsbuf,Bwpn&0xFFF);
            dowpn = Bwpn&0xFFF;
            directWpn = directItemB;
        }
        else if(DrunkrAbtn())
        {
            btnwpn=getItemFamily(itemsbuf,Awpn&0xFFF);
            dowpn = Awpn&0xFFF;
            directWpn = directItemA;
        }
        
        if(directWpn > 255) directWpn = 0;
        
        // The Quick Sword only allows repeated sword or wand swings.
        if(action==attacking && ((attack==wSword && btnwpn!=itype_sword) || (attack==wWand && btnwpn!=itype_wand)))
            btnwpn=-1;
    }
    
    if(can_attack() && (directWpn>-1 ? itemsbuf[directWpn].family==itype_sword : current_item(itype_sword)) && swordclk==0 && btnwpn==itype_sword && charging==0)
    {
	attackid=directWpn>-1 ? directWpn : current_item_id(itype_sword);
	if(checkmagiccost(attackid) || (get_bit(quest_rules, qr_MELEEMAGICCOST) == 0) ) //what about wands and canes?
		//2.50.2 quests may have had a magic cost only on sword beams. Need to add this to the Item Editor in 2.54+ 
		//as a flag on sword class items (Beams Use Magic, Sword Blade Uses Magic)
	{
		    //else paymagiccost(itemid);
		action=attacking; FFCore.setLinkAction(attacking);
		attack=wSword;
		
		attackclk=0;
		sfx(itemsbuf[directWpn>-1 ? directWpn : current_item_id(itype_sword)].usesound, pan(int(x)));
		
		if(dowpn>-1 && itemsbuf[dowpn].script!=0 && !did_scripta && checkmagiccost(dowpn))
		{
		    ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[dowpn].script, dowpn & 0xFFF);
		    did_scripta=true;
		}
	}
    }
    else
    {
        did_scripta=false;
    }
    
    int wx=x;
    int wy=y;
    
    switch(dir)
    {
    case up:
        wy-=16;
        break;
        
    case down:
        wy+=16;
        break;
        
    case left:
        wx-=16;
        break;
        
    case right:
        wx+=16;
        break;
    }
    
    do_lens();
    
    WalkflagInfo info;
    
    if(can_attack() && itemclk==0 && btnwpn>itype_sword && charging==0 && btnwpn!=itype_rupee) // This depends on item 0 being a rupee...
    {
        bool paidmagic = false;
        
        if(btnwpn==itype_wand && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_wand : false) : current_item(itype_wand)))
        {
            action=attacking; FFCore.setLinkAction(attacking);
            attack=wWand;
            attackid=directWpn>-1 ? directWpn : current_item_id(itype_wand);
            attackclk=0;
        }
        else if((btnwpn==itype_hammer)&&!(action==attacking && attack==wHammer)
                && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_hammer : false) : current_item(itype_hammer)) && checkmagiccost(dowpn))
        {
            paymagiccost(dowpn);
            paidmagic = true;
            action=attacking; FFCore.setLinkAction(attacking);
            attack=wHammer;
            attackid=directWpn>-1 ? directWpn : current_item_id(itype_hammer);
            attackclk=0;
        }
        else if((btnwpn==itype_candle)&&!(action==attacking && attack==wFire)
                && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_candle : false) : current_item(itype_candle)))
        {
            action=attacking; FFCore.setLinkAction(attacking);
            attack=wFire;
            attackid=directWpn>-1 ? directWpn : current_item_id(itype_candle);
            attackclk=0;
        }
        else if((btnwpn==itype_cbyrna)&&!(action==attacking && attack==wCByrna)
                && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_cbyrna : false) : current_item(itype_cbyrna)))
        {
            action=attacking; FFCore.setLinkAction(attacking);
            attack=wCByrna;
            attackid=directWpn>-1 ? directWpn : current_item_id(itype_cbyrna);
            attackclk=0;
        }
        else
        {
            paidmagic = startwpn(directWpn>-1 ? directWpn : current_item_id(btnwpn));
            
            if(paidmagic)
            {
                if(action==casting || action==drowning)
                {
                    ;
                }
                else
                {
                    action=attacking; FFCore.setLinkAction(attacking);
                    attackclk=0;
                    attack=none;
                    
                    if(btnwpn==itype_brang)
                    {
                        attack=wBrang;
                    }
                }
            }
            else
            {
                // Weapon not started: directWpn should be reset to prev. value.
                directWpn = olddirectwpn;
            }
        }
        
        if(dowpn>-1 && itemsbuf[dowpn].script!=0 && !did_scriptb && (paidmagic || checkmagiccost(dowpn)))
        {
            // Only charge for magic if item's magic cost wasn't already charged
            // for the item's main use.
            if(!paidmagic && attack!=wWand)
                paymagiccost(dowpn);
                
            ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[dowpn].script, dowpn & 0xFFF);
            did_scriptb=true;
        }
        
        if(action==casting || action==drowning)
        {
            return;
        }
    }
    else
    {
        did_scriptb=false;
    }
    
    if(attackclk || action==attacking)
    {
        bool attacked = doattack();
        
        // This section below interferes with script-setting Link->Dir, so it comes after doattack
        if(!inlikelike && attackclk>4 && (attackclk&3)==0 && charging==0 && spins==0)
        {
            if((xoff==0)||diagonalMovement)
            {
                if(DrunkUp()) dir=up;
                
                if(DrunkDown()) dir=down;
            }
            
            if((yoff==0)||diagonalMovement)
            {
                if(DrunkLeft()) dir=left;
                
                if(DrunkRight()) dir=right;
            }
        }
        
        if(attacked && (charging==0 && spins<=5) && jumping<1)
        {
            return;
        }
        else if(!(attacked))
        {
            // Spin attack - change direction
            if(spins>1)
            {
                spins--;
                
                if(spins%5==0)
                    sfx(itemsbuf[current_item_id(spins >5 ? itype_spinscroll2 : itype_spinscroll)].usesound,pan(int(x)));
                    
                attackclk=1;
                
                switch(dir)
                {
                case up:
                    dir=left;
                    break;
                    
                case right:
                    dir=up;
                    break;
                    
                case down:
                    dir=right;
                    break;
                    
                case left:
                    dir=down;
                    break;
                }
                
                return;
            }
            else
            {
                spins=0;
            }
            
            action=none; FFCore.setLinkAction(none);
            attackclk=0;
            charging=0;
        }
    }
    
    if(action==walking) //still walking
    {
        if(!DrunkUp() && !DrunkDown() && !DrunkLeft() && !DrunkRight() && !autostep)
        {
            action=(attackclk>0 ? attacking : none);
		FFCore.setLinkAction(attackclk>0 ? attacking : none);
            link_count=-1;
            return;
        }
        
        autostep=false;
        
        if(!diagonalMovement)
        {
            if(dir==up&&yoff)
            {
                info = walkflag(x,y+(bigHitbox?0:8)-int(lsteps[int(y)&7]),2,up);
                if(blockmoving)
                    info = info || walkflagMBlock(x+8,y+(bigHitbox?0:8)-int(lsteps[int(y)&7]));
                execute(info);
                
                if(!info.isUnwalkable())
                {
                    move(up);
                }
                else
                {
                    action=none; FFCore.setLinkAction(none);
                }
                
                return;
            }
            
            if(dir==down&&yoff)
            {
                info = walkflag(x,y+15+int(lsteps[int(y)&7]),2,down);
                if(blockmoving)
                    info = info || walkflagMBlock(x+8,y+15+int(lsteps[int(y)&7]));
                execute(info);
                
                if(!info.isUnwalkable())
                {
                    move(down);
                }
                else
                {
                    action=none; FFCore.setLinkAction(none);
                }
                
                return;
            }
            
            if(dir==left&&xoff)
            {
                info = walkflag(x-int(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,left) || walkflag(x-int(lsteps[int(x)&7]),y+8,1,left);
                execute(info);
                
                if(!info.isUnwalkable())
                {
                    move(left);
                }
                else
                {
                    action=none; FFCore.setLinkAction(none);
                }
                
                return;
            }
            
            if(dir==right&&xoff)
            {
                info = walkflag(x+15+int(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,right) || walkflag(x+15+int(lsteps[int(x)&7]),y+8,1,right);
                execute(info);
                
                if(!info.isUnwalkable())
                {
                    move(right);
                }
                else
                {
                    action=none; FFCore.setLinkAction(none);
                }
                
                return;
            }
        }
	
    } // endif (action==walking)
    
    if((action!=swimming)&&(action!=casting)&&(action!=drowning) && charging==0 && spins==0 && jumping<1)
    {
        action=none; FFCore.setLinkAction(none);
    }
    
    if(diagonalMovement)
    {
        switch(holddir)
        {
        case up:
            if(!Up())
            {
                holddir=-1;
            }
            
            break;
            
        case down:
            if(!Down())
            {
                holddir=-1;
            }
            
            break;
            
        case left:
            if(!Left())
            {
                holddir=-1;
            }
            
            break;
            
        case right:
            if(!Right())
            {
                holddir=-1;
            }
            
            break;
            
        default:
            break;
        } //end switch
        
        if(DrunkUp()&&(holddir==-1||holddir==up))
        {
            if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
            {
            }
            else
            {
                if(charging==0 && spins==0)
                {
                    dir=up;
                }
                
                holddir=up;
                
                if(DrunkRight()&&shiftdir!=left)
                {
                    shiftdir=right;
                }
                else if(DrunkLeft()&&shiftdir!=right)
                {
                    shiftdir=left;
                }
                else
                {
                    shiftdir=-1;
                }
                
                //walkable if Ladder can be placed or is already placed vertically
                if(isSideview() && !toogam && !(can_deploy_ladder() || (ladderx && laddery && ladderdir==up)))
                {
                    walkable=false;
                }
                else
                {
                    info = walkflag(x,y+(bigHitbox?0:8)-z3step,2,up);
                    
                    if(int(x) & 7)
                        info = info || walkflag(x+16,y+(bigHitbox?0:8)-z3step,1,up);
                    else if(blockmoving)
                        info = info || walkflagMBlock(x+16, y+(bigHitbox?0:8)-z3step);
                        
                    execute(info);
                    
                    if(info.isUnwalkable())
                    {
                        if(z3step==2)
                        {
                            z3step=1;
                            info = walkflag(x,y+(bigHitbox?0:8)-z3step,2,up);
                            
                            if(int(x)&7)
                                info = info || walkflag(x+16,y+(bigHitbox?0:8)-z3step,1,up);
                            else if(blockmoving)
                                info = info || walkflagMBlock(x+16, y+(bigHitbox?0:8)-z3step);
                                
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                walkable = false;
                            }
                            else
                            {
                                walkable=true;
                            }
                        }
                        else
                        {
                            walkable=false;
                        }
                    }
                    else
                    {
                        walkable = true;
                    }
                }
                
                int s=shiftdir;
                
                if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
                {
                    shiftdir=-1;
                }
                else
                {
                    if(s==left)
                    {
                        info = (walkflag(x-1,y+(bigHitbox?0:8),1,left)||walkflag(x-1,y+15,1,left));
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x-1,y+(bigHitbox?0:8)-1,1,left);
                            execute(info);
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                    else if(s==right)
                    {
                        info = walkflag(x+16,y+(bigHitbox?0:8),1,right)||walkflag(x+16,y+15,1,right);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x+16,y+(bigHitbox?0:8)-1,1,right);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                }
                
                move(up);
                shiftdir=s;
                
                if(!walkable)
                {
                    if(shiftdir==-1)
                    {
                        if(!_walkflag(x,   y+(bigHitbox?0:8)-1,1) &&
                                !_walkflag(x+8, y+(bigHitbox?0:8)-1,1) &&
                                _walkflag(x+15,y+(bigHitbox?0:8)-1,1))
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+15,y+(bigHitbox?0:8)-1))
                                sprite::move((fix)-1,(fix)0);
                        }
                        else
                        {
                            if(_walkflag(x,   y+(bigHitbox?0:8)-1,1) &&
                                    !_walkflag(x+7, y+(bigHitbox?0:8)-1,1) &&
                                    !_walkflag(x+15,y+(bigHitbox?0:8)-1,1))
                            {
                                if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x,y+(bigHitbox?0:8)-1))
                                    sprite::move((fix)1,(fix)0);
                            }
                            else
                            {
                                pushing=push+1;
                            }
                        }
                        
                        z3step=2;
                    }
                    else
                    {
                        pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
                        z3step=2;
                    }
                }
                
                return;
            }
        }
        
        if(DrunkDown()&&(holddir==-1||holddir==down))
        {
            if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
            {
            }
            else
            {
                if(charging==0 && spins==0)
                {
                    dir=down;
                }
                
                holddir=down;
                
                if(DrunkRight()&&shiftdir!=left)
                {
                    shiftdir=right;
                }
                else if(DrunkLeft()&&shiftdir!=right)
                {
                    shiftdir=left;
                }
                else
                {
                    shiftdir=-1;
                }
                
                //bool walkable;
                if(isSideview() && !toogam)
                {
                    walkable=false;
                }
                else
                {
                    info = walkflag(x,y+15+z3step,2,down);
                    
                    if(int(x)&7)
                        info = info || walkflag(x+16,y+15+z3step,1,down);
                    else if(blockmoving)
                        info = info || walkflagMBlock(x+16, y+15+z3step);
                    
                    execute(info);
                    
                    if(info.isUnwalkable())
                    {
                        if(z3step==2)
                        {
                            z3step=1;
                            info = walkflag(x,y+15+z3step,2,down);
                            
                            if(int(x)&7)
                                info = info || walkflag(x+16,y+15+z3step,1,down);
                            else if(blockmoving)
                                info = info || walkflagMBlock(x+16, y+15+z3step);
                                
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                walkable = false;
                            }
                            else
                            {
                                walkable=true;
                            }
                        }
                        else
                        {
                            walkable=false;
                        }
                    }
                    else
                    {
                        walkable = true;
                    }
                }
                
                int s=shiftdir;
                
                if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
                {
                    shiftdir=-1;
                }
                else
                {
                    if(s==left)
                    {
                        info = walkflag(x-1,y+(bigHitbox?0:8),1,left)||walkflag(x-1,y+15,1,left);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x-1,y+16,1,left);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                    else if(s==right)
                    {
                        info = walkflag(x+16,y+(bigHitbox?0:8),1,right)||walkflag(x+16,y+15,1,right);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x+16,y+16,1,right);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                }
                
                move(down);
                shiftdir=s;
                
                if(!walkable)
                {
                    if(shiftdir==-1)
                    {
                        if(!_walkflag(x,   y+15+1,1)&&
                                !_walkflag(x+8, y+15+1,1)&&
                                _walkflag(x+15,y+15+1,1))
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+15,y+15+1))
                                sprite::move((fix)-1,(fix)0);
                        }
                        else if(_walkflag(x,   y+15+1,1)&&
                                !_walkflag(x+7, y+15+1,1)&&
                                !_walkflag(x+15,y+15+1,1))
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x,y+15+1))
                                sprite::move((fix)1,(fix)0);
                        }
                        else //if(shiftdir==-1)
                        {
                            pushing=push+1;
                            
                            if(action!=swimming)
                            {
                            }
                        }
                        
                        z3step=2;
                    }
                    else
                    {
                        pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
                        
                        if(action!=swimming)
                        {
                        }
                        
                        z3step=2;
                    }
                }
                
                return;
            }
        }
        
        if(DrunkLeft()&&(holddir==-1||holddir==left))
        {
            if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
            {
            }
            else
            {
                if(charging==0 && spins==0)
                {
                    dir=left;
                }
                
                holddir=left;
                
                if(DrunkUp()&&shiftdir!=down)
                {
                    shiftdir=up;
                }
                else if(DrunkDown()&&shiftdir!=up)
                {
                    shiftdir=down;
                }
                else
                {
                    shiftdir=-1;
                }
                
                //bool walkable;
                info = walkflag(x-z3step,y+(bigHitbox?0:8),1,left)||walkflag(x-z3step,y+8,1,left);
                
                if(int(y)&7)
                    info = info || walkflag(x-z3step,y+16,1,left);
                    
                execute(info);
                
                if(info.isUnwalkable())
                {
                    if(z3step==2)
                    {
                        z3step=1;
                        info = walkflag(x-z3step,y+(bigHitbox?0:8),1,left)||walkflag(x-z3step,y+8,1,left);
                        
                        if(int(y)&7)
                            info = info || walkflag(x-z3step,y+16,1,left);
                            
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            walkable = false;
                        }
                        else
                        {
                            walkable=true;
                        }
                    }
                    else
                    {
                        walkable=false;
                    }
                }
                else
                {
                    walkable = true;
                }
                
                int s=shiftdir;
                
                if((isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM)) || isSideview())
                {
                    shiftdir=-1;
                }
                else
                {
                    if(s==up)
                    {
                        info = walkflag(x,y+(bigHitbox?0:8)-1,2,up)||walkflag(x+15,y+(bigHitbox?0:8)-1,1,up);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x-1,y+(bigHitbox?0:8)-1,1,up);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                    else if(s==down)
                    {
                        info = walkflag(x,y+16,2,down)||walkflag(x+15,y+16,1,down);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x-1,y+16,1,down);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                }
                
                move(left);
                shiftdir=s;
                
                if(!walkable)
                {
                    if(shiftdir==-1)
                    {
                        int v1=bigHitbox?0:8;
                        int v2=bigHitbox?8:12;
                        
                        if(!_walkflag(x-1,y+v1,1)&&
                                !_walkflag(x-1,y+v2,1)&&
                                _walkflag(x-1,y+15,1))
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+15))
                                sprite::move((fix)0,(fix)-1);
                        }
                        else if(_walkflag(x-1,y+v1,  1)&&
                                !_walkflag(x-1,y+v2-1,1)&&
                                !_walkflag(x-1,y+15,  1))
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1))
                                sprite::move((fix)0,(fix)1);
                        }
                        else //if(shiftdir==-1)
                        {
                            pushing=push+1;
                            
                            if(action!=swimming)
                            {
                            }
                        }
                        
                        z3step=2;
                    }
                    else
                    {
                        pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
                        
                        if(action!=swimming)
                        {
                        }
                        
                        z3step=2;
                    }
                }
                
                return;
            }
        }
        
        if(DrunkRight()&&(holddir==-1||holddir==right))
        {
            if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
            {
            }
            else
            {
                if(charging==0 && spins==0)
                {
                    dir=right;
                }
                
                holddir=right;
                
                if(DrunkUp()&&shiftdir!=down)
                {
                    shiftdir=up;
                }
                else if(DrunkDown()&&shiftdir!=up)
                {
                    shiftdir=down;
                }
                else
                {
                    shiftdir=-1;
                }
                
                //bool walkable;
                info = walkflag(x+15+z3step,y+(bigHitbox?0:8),1,right)||walkflag(x+15+z3step,y+8,1,right);
                
                if(int(y)&7)
                    info = info || walkflag(x+15+z3step,y+16,1,right);
                    
                execute(info);
                
                if(info.isUnwalkable())
                {
                    if(z3step==2)
                    {
                        z3step=1;
                        info = walkflag(x+15+z3step,y+(bigHitbox?0:8),1,right)||walkflag(x+15+z3step,y+8,1,right);
                        
                        if(int(y)&7)
                            info = info || walkflag(x+15+z3step,y+16,1,right);
                            
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            walkable = false;
                        }
                        else
                        {
                            walkable=true;
                        }
                    }
                    else
                    {
                        walkable=false;
                    }
                }
                else
                {
                    walkable = true;
                }
                
                int s=shiftdir;
                
                if((isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM)) || isSideview())
                {
                    shiftdir=-1;
                }
                else
                {
                    if(s==up)
                    {
                        info = walkflag(x,y+(bigHitbox?0:8)-1,2,up)||walkflag(x+15,y+(bigHitbox?0:8)-1,1,up);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x+16,y+(bigHitbox?0:8)-1,1,up);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                    else if(s==down)
                    {
                        info = walkflag(x,y+16,2,down)||walkflag(x+15,y+16,1,down);
                        execute(info);
                        
                        if(info.isUnwalkable())
                        {
                            shiftdir=-1;
                        }
                        else if(walkable)
                        {
                            info = walkflag(x+16,y+16,1,down);
                            execute(info);
                            
                            if(info.isUnwalkable())
                            {
                                shiftdir=-1;
                            }
                        }
                    }
                }
                
                move(right);
                shiftdir=s;
                
                if(!walkable)
                {
                    if(shiftdir==-1)
                    {
                        int v1=bigHitbox?0:8;
                        int v2=bigHitbox?8:12;
                        
                        info = !walkflag(x+16,y+v1,1,right)&&
                               !walkflag(x+16,y+v2,1,right)&&
                               walkflag(x+16,y+15,1,right);
                               
                        //do NOT execute these
                        if(info.isUnwalkable())
                        {
                            if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+15))
                                sprite::move((fix)0,(fix)-1);
                        }
                        else
                        {
                            info = walkflag(x+16,y+v1,  1,right)&&
                                   !walkflag(x+16,y+v2-1,1,right)&&
                                   !walkflag(x+16,y+15,  1,right);
                                   
                            if(info.isUnwalkable())
                            {
                                if(hclk || (z>0 && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1))
                                    sprite::move((fix)0,(fix)1);
                            }
                            else //if(shiftdir==-1)
                            {
                                pushing=push+1;
                                z3step=2;
                                
                                if(action!=swimming)
                                {
                                }
                            }
                        }
                        
                        z3step=2;
                    }
                    else
                    {
                        pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
                        
                        if(action!=swimming)
                        {
                        }
                        
                        z3step=2;
                    }
                }
                
                return;
            }
        }
        
        bool wtry  = iswater(MAPCOMBO(x,y+15));
        bool wtry8 = iswater(MAPCOMBO(x+15,y+15));
        bool wtrx = iswater(MAPCOMBO(x,y+(bigHitbox?0:8)));
        bool wtrx8 = iswater(MAPCOMBO(x+15,y+(bigHitbox?0:8)));
        
        if(can_use_item(itype_flippers,i_flippers)&&!(ladderx+laddery)&&z==0)
        {
            if(wtrx&&wtrx8&&wtry&&wtry8 && !DRIEDLAKE)
            {
                //action=swimming;
                if(action !=none && action != swimming)
                {
                    hopclk = 0xFF;
                }
            }
        }
        
        return;
    } //endif (LTTPWALK)
    
    if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
    {
        goto LEFTRIGHT;
    }
    
    // make it easier to get in left & right doors
    
    //ignore ladder for this part. sigh sigh sigh -DD
    oldladderx = ladderx;
    oldladdery = laddery;
    info = walkflag(x-int(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,left) ||
           walkflag(x-int(lsteps[int(x)&7]),y+8,1,left);
    
    if(isdungeon() && DrunkLeft() && x==32 && y==80 && !info.isUnwalkable())
    {
        //ONLY process the side-effects of the above walkflag if Link will actually move
        //sigh sigh sigh... walkflag is a horrible mess :-/ -DD
        execute(info);
        move(left);
        return;
    }
    
    info = walkflag(x+15+int(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,right) ||
           walkflag(x+15+int(lsteps[int(x)&7]),y+8,1,right);
    
    if(isdungeon() && DrunkRight() && x==208 && y==80 && !info.isUnwalkable())
    {
        execute(info);
        move(right);
        return;
    }
    
    ladderx = oldladderx;
    laddery = oldladdery;
    
    if(DrunkUp())
    {
        if(xoff && !is_on_conveyor && action != swimming && jumping<1)
        {
            if(dir!=up && dir!=down)
            {
                if(xoff>2&&xoff<6)
                {
                    move(dir);
                }
                else if(xoff>=6)
                {
                    move(right);
                }
                else if(xoff>=1)
                {
                    move(left);
                }
            }
            else
            {
                if(xoff>=4)
                {
                    move(right);
                }
                else if(xoff<4)
                {
                    move(left);
                }
            }
        }
        else
        {
            if(action==swimming)
            {
                info = walkflag(x,y+(bigHitbox?0:8)-int(lsteps[int(y)&7]),2,up);
                
                if(_walkflag(x+15, y+(bigHitbox?0:8)-int(lsteps[int(y)&7]), 1) &&
                        !(iswater(MAPCOMBO(x, y+(bigHitbox?0:8)-int(lsteps[int(y)&7]))) &&
                          iswater(MAPCOMBO(x+15, y+(bigHitbox?0:8)-int(lsteps[int(y)&7])))))
                    info.setUnwalkable(true);
            }
            else
            {
                info = walkflag(x,y+(bigHitbox?0:8)-int(lsteps[int(y)&7]),2,up);
                if(blockmoving)
                    info = info || walkflagMBlock(x+8,y+(bigHitbox?0:8)-int(lsteps[int(y)&7]));
            }
            
            execute(info);
            
            if(!info.isUnwalkable())
            {
                move(up);
                return;
            }
            
            if(!DrunkLeft() && !DrunkRight())
            {
                pushing=push+1;
                
                if(charging==0 && spins==0)
                {
                    dir=up;
                }
                
                if(action!=swimming)
                {
                    linkstep();
                }
                
                return;
            }
            else
            {
                goto LEFTRIGHT;
            }
        }
        
        return;
    }
    
    if(DrunkDown())
    {
        if(xoff && !is_on_conveyor && action != swimming && jumping<1)
        {
            if(dir!=up && dir!=down)
            {
                if(xoff>2&&xoff<6)
                {
                    move(dir);
                }
                else if(xoff>=6)
                {
                    move(right);
                }
                else if(xoff>=1)
                {
                    move(left);
                }
            }
            else
            {
                if(xoff>=4)
                {
                    move(right);
                }
                else if(xoff<4)
                {
                    move(left);
                }
            }
        }
        else
        {
            if(action==swimming)
            {
                info=walkflag(x,y+15+int(lsteps[int(y)&7]),2,down);
                
                if(_walkflag(x+15, y+15+int(lsteps[int(y)&7]), 1) &&
                        !(iswater(MAPCOMBO(x, y+15+int(lsteps[int(y)&7]))) &&
                          iswater(MAPCOMBO(x+15, y+15+int(lsteps[int(y)&7])))))
                    info.setUnwalkable(true);
            }
            else
            {
                info=walkflag(x,y+15+int(lsteps[int(y)&7]),2,down);
                if(blockmoving)
                     info = info || walkflagMBlock(x+8,y+15+int(lsteps[int(y)&7]));
            }
            
            execute(info);
            
            if(!info.isUnwalkable())
            {
                move(down);
                return;
            }
            
            if(!DrunkLeft() && !DrunkRight())
            {
                pushing=push+1;
                
                if(charging==0 && spins==0)
                {
                    dir=down;
                }
                
                if(action!=swimming)
                {
                    linkstep();
                }
                
                return;
            }
            else goto LEFTRIGHT;
        }
        
        return;
    }
    
LEFTRIGHT:

    if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM) && !toogam)
    {
        return;
    }
    
    if(DrunkLeft())
    {
        if(yoff && !is_on_conveyor && action != swimming && jumping<1)
        {
            if(dir!=left && dir!=right)
            {
                if(yoff>2&&yoff<6)
                {
                    move(dir);
                }
                else if(yoff>=6)
                {
                    move(down);
                }
                else if(yoff>=1)
                {
                    move(up);
                }
            }
            else
            {
                if(yoff>=4)
                {
                    move(down);
                }
                else if(yoff<4)
                {
                    move(up);
                }
            }
        }
        else
        {
            info = walkflag(x-int(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,left) ||
                   walkflag(x-int(lsteps[int(x)&7]),y+(isSideview() ?0:8), 1,left);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                move(left);
                return;
            }
            
            if(!DrunkUp() && !DrunkDown())
            {
                pushing=push+1;
                
                if(charging==0 && spins==0)
                {
                    dir=left;
                }
                
                if(action!=swimming)
                {
                    linkstep();
                }
                
                return;
            }
        }
        
        return;
    }
    
    if(DrunkRight())
    {
        if(yoff && !is_on_conveyor && action != swimming && jumping<1)
        {
            if(dir!=left && dir!=right)
            {
                if(yoff>2&&yoff<6)
                {
                    move(dir);
                }
                else if(yoff>=6)
                {
                    move(down);
                }
                else if(yoff>=1)
                {
                    move(up);
                }
            }
            else
            {
                if(yoff>=4)
                {
                    move(down);
                }
                else if(yoff<4)
                {
                    move(up);
                }
            }
        }
        else
        {
            info = walkflag((int)x+15+(lsteps[int(x)&7]),y+(bigHitbox?0:8),1,right) || walkflag((int)x+15+(lsteps[int(x)&7]),y+(isSideview() ?0:8),             1,right);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                move(right);
                return;
            }
            
            if(!DrunkUp() && !DrunkDown())
            {
                pushing=push+1;
                
                if(charging==0 && spins==0)
                {
                    dir=right;
                }
                
                if(action!=swimming)
                {
                    linkstep();
                }
                
                return;
            }
        }
    }
}

void LinkClass::move(int d2)
{
//al_trace("%s\n",d2==up?"up":d2==down?"down":d2==left?"left":d2==right?"right":"?");
    static bool totalskip = false;
    
    if( inlikelike || link_is_stunned > 0 )
        return;
        
    int dx=0,dy=0;
    int xstep=lsteps[int(x)&7];
    int ystep=lsteps[int(y)&7];
    int z3skip=0;
    int z3diagskip=0;
    bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && (z==0 || tmpscr->flags2&fAIRCOMBOS)) ||
                     (isSideview() && ON_SIDEPLATFORM && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement);
    bool slowcharging = get_bit(quest_rules,qr_SLOWCHARGINGWALK) && charging>0;
    bool is_swimming = (action == swimming);
    
    //slow walk combo, or charging, moves at 2/3 speed
    if(
        (!is_swimming && (slowcharging ^ slowcombo))||
        (is_swimming && (zinit.link_swim_speed>60))
    )
    {
        totalskip = false;
        
        if(diagonalMovement)
        {
            skipstep=(skipstep+1)%6;
            
            if(skipstep%2==0) z3skip=1;
            else z3skip=0;
            
            if(skipstep%3==0) z3diagskip=1;
            else z3diagskip=0;
        }
        else
        {
            if(d2<left)
            {
                if(ystep>1)
                {
                    skipstep^=1;
                    ystep=skipstep;
                }
            }
            else
            {
                if(xstep>1)
                {
                    skipstep^=1;
                    xstep=skipstep;
                }
            }
        }
    }
//  else if(is_swimming || (slowcharging && slowcombo))
    else if(
        (is_swimming && (zinit.link_swim_speed<60))||
        (slowcharging && slowcombo)
    )
    {
        //swimming, or charging on a slow combo, moves at 1/2 speed
        totalskip = !totalskip;
        
        if(diagonalMovement)
        {
            skipstep=0;
        }
    }
    else
    {
        totalskip = false;
        
        if(diagonalMovement)
        {
            skipstep=0;
        }
    }
    
    if(!totalskip)
    {
        if(diagonalMovement)
        {
            switch(d2)
            {
            case up:
                if(shiftdir==left)
                {
                    if(walkable)
                    {
                        dy-=1-z3diagskip;
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                }
                else if(shiftdir==right)
                {
                    if(walkable)
                    {
                        dy-=1-z3diagskip;
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                }
                else
                {
                    if(walkable)
                    {
                        dy-=z3step-z3skip;
                        z3step=(z3step%2)+1;
                    }
                }
                
                break;
                
            case down:
                if(shiftdir==left)
                {
                    if(walkable)
                    {
                        dy+=1-z3diagskip;
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                }
                else if(shiftdir==right)
                {
                    if(walkable)
                    {
                        dy+=1-z3diagskip;
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                }
                else
                {
                    if(walkable)
                    {
                        dy+=z3step-z3skip;
                        z3step=(z3step%2)+1;
                    }
                }
                
                break;
                
            case right:
                if(shiftdir==up)
                {
                    if(walkable)
                    {
                        dy-=1-z3diagskip;
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dy-=1-z3diagskip;
                        z3step=2;
                    }
                }
                else if(shiftdir==down)
                {
                    if(walkable)
                    {
                        dy+=1-z3diagskip;
                        dx+=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dy+=1-z3diagskip;
                        z3step=2;
                    }
                }
                else
                {
                    if(walkable)
                    {
                        dx+=z3step-z3skip;
                        z3step=(z3step%2)+1;
                    }
                }
                
                break;
                
            case left:
                if(shiftdir==up)
                {
                    if(walkable)
                    {
                        dy-=1-z3diagskip;
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dy-=1-z3diagskip;
                        z3step=2;
                    }
                }
                else if(shiftdir==down)
                {
                    if(walkable)
                    {
                        dy+=1-z3diagskip;
                        dx-=1-z3diagskip;
                        z3step=2;
                    }
                    else
                    {
                        dy+=1-z3diagskip;
                        z3step=2;
                    }
                }
                else
                {
                    if(walkable)
                    {
                        dx-=z3step-z3skip;
                        z3step=(z3step%2)+1;
                    }
                }
                
                break;
            }
        }
        else
        {
            switch(d2)
            {
            case up:
                if(!isSideview() || (ladderx && laddery && ladderdir==up)) dy-=ystep;
                
                break;
                
            case down:
                if(!isSideview() || (ladderx && laddery && ladderdir==up)) dy+=ystep;
                
                break;
                
            case left:
                dx-=xstep;
                break;
                
            case right:
                dx+=xstep;
                break;
            }
        }
    }
    
    if((charging==0 || attack==wHammer) && spins==0 && attackclk!=HAMMERCHARGEFRAME)
    {
        dir=d2;
    }
    
    if(action != swimming)
    {
        linkstep();
        
        //ack... don't walk if in midair! -DD
        if(charging==0 && spins==0 && z==0 && !(isSideview() && !ON_SIDEPLATFORM))
	{
            action=walking; FFCore.setLinkAction(walking);
	}
            
        if(++link_count > (16*link_animation_speed))
            link_count=0;
    }
    else if(!(frame & 1))
    {
        linkstep();
    }
    
    if(charging==0 || attack!=wHammer)
    {
        sprite::move((fix)dx,(fix)dy);
    }
}

LinkClass::WalkflagInfo LinkClass::walkflag(int wx,int wy,int cnt,byte d2)
{
    WalkflagInfo ret;
    
    if(toogam)
    {
        ret.setUnwalkable(false);
        return ret;
    }
    
    if(blockpath && wy<(bigHitbox?80:88))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    if(blockmoving && mblock2.hit(wx,wy,0,1,1,1))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    if(isdungeon() && currscr<128 && wy<(bigHitbox?32:40) && ((diagonalMovement?(x<=112||x>=128):x!=120) || _walkflag(120,24,2))
            && !get_bit(quest_rules,qr_FREEFORM))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    bool wf = _walkflag(wx,wy,cnt);
    
    if(isdungeon() && currscr<128 && !get_bit(quest_rules,qr_FREEFORM))
    {
        if(diagonalMovement)
        {
            if(wx>=112&&wx<120&&wy<40&&wy>=32) wf=true;
            
            if(wx>=136&&wx<144&&wy<40&&wy>=32) wf=true;
        }
    }
    //All problems related to exiting water are probably here. -Z
    if(action==swimming)
    {
        if(!wf)
        {
            if(landswim>= (get_bit(quest_rules,qr_DROWN) && isSwimming() ? 1
                           : (!diagonalMovement) ? 1 : 22))
            {
                //Check for out of bounds for swimming
                bool changehop = true;
                
                if(diagonalMovement)
                {
                    if(wx<0||wy<0)
                        changehop = false;
                    else if(wx>248)
                        changehop = false;
                    else if(wx>240&&cnt==2)
                        changehop = false;
                    else if(wy>168)
                        changehop = false;
                }
                //This may be where the hang-up for exiting water exists. -Z
                // hop out of the water
                if(changehop)
                    ret.setHopClk(1);
            }
            else
            {
                if(dir==d2 || shiftdir==d2)
                {
                    //int vx=((int)x+4)&0xFFF8;
                    //int vy=((int)y+4)&0xFFF8;
                    if(d2==left)
                    {
                        if(!iswater(MAPCOMBO(x-1,y+(bigHitbox?6:11))) &&
                           !iswater(MAPCOMBO(x-1,y+(bigHitbox?9:12))) &&
                           !_walkflag(x-1,y+(bigHitbox?6:11),1) &&
                           !_walkflag(x-1,y+(bigHitbox?9:12),1))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==right)
                    {
                        if(!iswater(MAPCOMBO(x+16,y+(bigHitbox?6:11))) &&
                           !iswater(MAPCOMBO(x+16,y+(bigHitbox?9:12))) &&
                           !_walkflag(x+16,y+(bigHitbox?6:11),1) &&
                           !_walkflag(x+16,y+(bigHitbox?9:12),1))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==up)
                    {
                        if(!iswater(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1)) &&
                           !iswater(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1)) &&
                           !_walkflag(x+7,y+(bigHitbox?0:8)-1,1) &&
                           !_walkflag(x+8,y+(bigHitbox?0:8)-1,1))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==down)
                    {
                        if(!iswater(MAPCOMBO(x+7,y+16)) &&
                           !iswater(MAPCOMBO(x+8,y+16)) &&
                           !_walkflag(x+7,y+16,1) &&
                           !_walkflag(x+8,y+16,1))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                }
                
                if(wx<0||wy<0);
                else if(wx>248);
                else if(wx>240&&cnt==2);
                else if(wy>168);
                else if(get_bit(quest_rules, qr_DROWN) && !ilswim);
                else if(iswater(MAPCOMBO(wx,wy)) && iswater(MAPCOMBO(wx,wy)))
                {
                    ret.setUnwalkable(false);
                    return ret;
                }
                else
                {
                    ret.setUnwalkable(true);
                    return ret;
                }
            }
        }
        else
        {
            bool wtrx  = iswater(MAPCOMBO(wx,wy));
            bool wtrx8 = iswater(MAPCOMBO(x+8,wy));
            
            if((d2>=left && wtrx) || (d2<=down && wtrx && wtrx8))
            {
                ret.setUnwalkable(false);
                return ret;
            }
        }
    }
    else if(ladderx+laddery)                                  // ladder is being used
    {
        int lx = !(get_bit(quest_rules, qr_DROWN)&&iswater(MAPCOMBO(x+4,y+11))&&!_walkflag(x+4,y+11,1)) ? wx : x;
        int ly = !(get_bit(quest_rules, qr_DROWN)&&iswater(MAPCOMBO(x+4,y+11))&&!_walkflag(x+4,y+11,1)) ? wy : y;
        
        if(diagonalMovement)
        {
            if(ladderdir==up)
            {
                if(abs(ly-(laddery+8))<=8) // ly is between laddery (laddery+8-8) and laddery+16 (laddery+8+8)
                {
                    bool temp = false;
                    
                    if(!(abs(lx-(ladderx+8))<=8))
                        temp = true;
                        
                    if(cnt==2)
                        if(!(abs((lx+8)-(ladderx+8))<=8))
                            temp=true;
                            
                    if(!temp)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                    
                    if(current_item_power(itype_ladder)<2 && (d2==left || d2==right) && !isSideview())
                    {
                        ret.setUnwalkable(true);
                        return ret;
                    }
                }
            }
            else
            {
                if(abs(lx-(ladderx+8))<=8)
                {
                    if(abs(ly-(laddery+(bigHitbox?8:12)))<=(bigHitbox?8:4))
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                    
                    if(current_item_power(itype_ladder)<2 && (d2==up || d2==down))
                    {
                        ret.setUnwalkable(true);
                        return ret;
                    }
                    
                    if((abs(ly-laddery+8)<=8) && d2<=down)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                }
            }
        } // diagonalMovement
        else
        {
            if((d2&2)==ladderdir)                                    // same direction
            {
                switch(d2)
                {
                case up:
                    if(int(y)<=laddery)
                    {
                        ret.setUnwalkable(_walkflag(ladderx,laddery-8,1) ||
                                          _walkflag(ladderx+8,laddery-8,1));
                        return ret;
                        
                    }
                    
                    // no break
                case down:
                    if((wy&0xF0)==laddery)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                    
                    break;
                    
                default:
                    if((wx&0xF0)==ladderx)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                }
                
                if(d2<=down)
                {
                    ret.setUnwalkable(_walkflag(ladderx,wy,1) || _walkflag(ladderx+8,wy,1));
                    return ret;
                }
                
                ret.setUnwalkable(_walkflag((wx&0xF0),wy,1) || _walkflag((wx&0xF0)+8,wy,1));
                return ret;
            }
            
            // different dir
            if(current_item_power(itype_ladder)<2 && !(isSideview() && (d2==left || d2==right)))
            {
                ret.setUnwalkable(true);
                return ret;
            }
            
            if(wy>=laddery && wy<=laddery+16 && d2<=down)
            {
                ret.setUnwalkable(false);
                return ret;
            }
        }
    }
    else if(wf || isSideview() || get_bit(quest_rules, qr_DROWN))
    {
        // see if it's a good spot for the ladder or for swimming
        bool unwalkablex  = _walkflag(wx,wy,1); //will be used later for the ladder -DD
        bool unwalkablex8 = _walkflag(x+8,wy,1);
        
        if(get_bit(quest_rules, qr_DROWN))
        {
            // Drowning changes the following attributes:
            // * Dangerous water is also walkable, so ignore the previous
            // definitions of unwalkablex and unwalkablex8.
            // * Instead, prevent the ladder from being used in the
            // one frame where Link has landed on water before drowning.
            unwalkablex = unwalkablex8 = !iswater(MAPCOMBO(x+4,y+11));
        }
        
        // check if he can swim
        if(current_item(itype_flippers) && z==0)
        {
            bool wtrx  = iswater(MAPCOMBO(wx,wy));
            bool wtrx8 = iswater(MAPCOMBO(x+8,wy));
            //ladder ignores water combos that are now walkable thanks to flippers -DD
            unwalkablex = unwalkablex && !wtrx;
            unwalkablex8 = unwalkablex8 && !wtrx8;
            
            if(landswim >= 22)
            {
                ret.setHopClk(2);
                ret.setUnwalkable(false);
                return ret;
            }
            else if((d2>=left && wtrx) || (d2<=down && wtrx && wtrx8))
            {
                if(!diagonalMovement)
                {
                    ret.setHopClk(2);
                    
                    if(charging || spins>5)
                    {
                        //if Link is charging, he might be facing the wrong direction (we want him to
                        //hop into the water, not in the facing direction)
                        ret.setDir(d2);
                        //moreover Link can't charge in the water -DD
                        ret.setChargeAttack();
                    }
                    
                    ret.setUnwalkable(false);
                    return ret;
                }
                else if(dir==d2)
                {
                    ret.setIlswim(true);
                    ladderx = 0;
                    laddery = 0;
                }
            }
        }
        
        // check if he can use the ladder
        // "Allow Ladder Anywhere" is toggled by fLADDER
        if(can_deploy_ladder())
            // laddersetup
        {
            // Check if there's water to use the ladder over
            bool wtrx = iswater(MAPCOMBO(wx,wy));
            bool wtrx8 = iswater(MAPCOMBO(x+8,wy));
            
            if(wtrx || wtrx8)
            {
                if(isSideview())
                {
                    wtrx  = !_walkflag(wx, wy+8, 1) && !_walkflag(wx, wy, 1) && dir!=down;
                    wtrx8 = !_walkflag(wx+8, wy+8, 1) && !_walkflag(wx+8, wy, 1) && dir!=down;
                }
                // * walk on half-water using the ladder instead of using flippers.
                // * otherwise, walk on ladder(+hookshot) combos.
                else if(wtrx==wtrx8 && (isstepable(MAPCOMBO(wx, wy)) || isstepable(MAPCOMBO(wx+8,wy)) || wtrx==true))
                {
                    //if Link could swim on a tile instead of using the ladder,
                    //refuse to use the ladder to step over that tile. -DD
                    wtrx  = isstepable(MAPCOMBO(wx, wy)) && unwalkablex;
                    wtrx8 = isstepable(MAPCOMBO(wx+8,wy)) && unwalkablex8;
                }
            }
            // No water; how about ladder combos?
            else
            {
                int combo=combobuf[MAPCOMBO(wx, wy)].type;
                wtrx=(combo==cLADDERONLY || combo==cLADDERHOOKSHOT);
                combo=combobuf[MAPCOMBO(wx+8, wy)].type;
                wtrx8=(combo==cLADDERONLY || combo==cLADDERHOOKSHOT);
            }
            
            bool walkwater = get_bit(quest_rules, qr_DROWN) && !iswater(MAPCOMBO(wx,wy));
            
            if(diagonalMovement)
            {
                if(d2==dir)
                {
                    int c = walkwater ? 0:8;
                    int b = walkwater ? 8:0;
                    
                    if(d2>=left)
                    {
                        // If the difference between wy and y is small enough
                        if(abs((wy)-(int(y+c)))<=(b) && wtrx)
                        {
                            // Don't activate the ladder if it would be entirely
                            // over water and Link has the flippers. This isn't
                            // a good way to do this, but it's too risky
                            // to make big changes to this stuff.
                            bool deployLadder=true;
                            int lx=wx&0xF0;
                            if(current_item(itype_flippers) && z==0)
                            {
                                if(iswater(MAPCOMBO(lx, y)) && iswater(MAPCOMBO(lx+15, y)) &&
                                  iswater(MAPCOMBO(lx, y+15)) && iswater(MAPCOMBO(lx+15, y+15)))
                                    deployLadder=false;
                            }
                            if(deployLadder)
                            {
                                ladderx = wx&0xF0;
                                laddery = y;
                                ladderdir = left;
                                ladderstart = d2;
                                ret.setUnwalkable(laddery!=int(y));
                                return ret;
                            }
                        }
                    }
                    else if(d2<=down)
                    {
                        // If the difference between wx and x is small enough
                        if(abs((wx)-(int(x+c)))<=(b) && wtrx)
                        {
                            ladderx = x;
                            laddery = wy&0xF0;
                            ladderdir = up;
                            ladderstart = d2;
                            ret.setUnwalkable(ladderx!=int(x));
                            return ret;
                        }
                        
                        if(cnt==2)
                        {
                            if(abs((wx+8)-(int(x+c)))<=(b) && wtrx8)
                            {
                                ladderx = x;
                                laddery = wy&0xF0;
                                ladderdir = up;
                                ladderstart = d2;
                                ret.setUnwalkable(ladderx!=int(x));
                                return ret;
                            }
                        }
                    }
                }
            }
            else
            {
                bool flgx  = _walkflag(wx,wy,1) && !wtrx; // Solid, and not steppable
                bool flgx8 = _walkflag(x+8,wy,1) && !wtrx8; // Solid, and not steppable
                
                if((d2>=left && wtrx)
                        // Deploy the ladder vertically even if Link is only half on water.
                        || (d2<=down && ((wtrx && !flgx8) || (wtrx8 && !flgx))))
                {
                    if(((int(y)+15) < wy) || ((int(y)+8) > wy))
                        ladderdir = up;
                    else
                        ladderdir = left;
                        
                    if(ladderdir==up)
                    {
                        ladderx = int(x)&0xF8;
                        laddery = wy&0xF0;
                    }
                    else
                    {
                        ladderx = wx&0xF0;
                        laddery = int(y)&0xF8;
                    }
                    
                    ret.setUnwalkable(false);
                    return ret;
                }
            }
        }
    }
    
    ret.setUnwalkable(wf);
    return ret;
}

// Only checks for moving blocks. Apparently this is a thing we need.
LinkClass::WalkflagInfo LinkClass::walkflagMBlock(int wx,int wy)
{
    WalkflagInfo ret;
    ret.setUnwalkable(blockmoving && mblock2.hit(wx,wy,0,1,1,1));
    return ret;
}

void LinkClass::checkpushblock()
{
    if(toogam) return;
    
    if(z!=0) return;
    
    // Return early in some cases..
    bool earlyReturn=false;
    
    if(!diagonalMovement || dir==left)
        if(int(x)&15) earlyReturn=true;
        
    // if(y<16) return;
    if(isSideview() && !ON_SIDEPLATFORM) return;
    
    int bx = int(x)&0xF0;
    int by = (int(y)&0xF0);
    
    switch(dir)
    {
    case up:
        if(y<16)
        {
            earlyReturn=true;
            break;
        }
        
        if(!((int)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
        
        if((int)x&8) bx+=16;
        
        break;
        
    case down:
        if(y>128)
        {
            earlyReturn=true;
            break;
        }
        else
        {
            by+=16;
            
            if((int)x&8) bx+=16;
        }
        
        break;
        
    case left:
        if(x<32)
        {
            earlyReturn=true;
            break;
        }
        else
        {
            bx-=16;
            
            if(int(y)&8)
            {
                by+=16;
            }
        }
        
        break;
        
    case right:
        if(x>208)
        {
            earlyReturn=true;
            break;
        }
        else
        {
            bx+=16;
            
            if(int(y)&8)
            {
                by+=16;
            }
        }
        
        break;
    }
    
    // In sideview, we still have to check for damage combos.
    if(earlyReturn && !isSideview())
        return;
    
    int f = MAPFLAG(bx,by);
    int f2 = MAPCOMBOFLAG(bx,by);
    int t = combobuf[MAPCOMBO(bx,by)].type;
    
    // Unlike push blocks, damage combos should be tested on layers 2 and under
    for(int i=(get_bit(quest_rules,qr_DMGCOMBOLAYERFIX) ? 2 : 0); i>=0; i--)
    {
        t = combobuf[i==0 ? MAPCOMBO(bx,by) : MAPCOMBO2(i-1,bx,by)].type;
        
        // Solid damage combos use pushing>0, hence the code is here.
        if(combo_class_buf[t].modify_hp_amount && _walkflag(bx,by,1) && pushing>0 && hclk<1 && action!=casting && !get_bit(quest_rules, qr_NOSOLIDDAMAGECOMBOS))
        {
            // Bite Link
            checkdamagecombos(bx+8-(tmpscr->csensitive),
                              bx+8+(zc_max(tmpscr->csensitive-1,0)),
                              by+(bigHitbox?8:12)-(bigHitbox?tmpscr->csensitive:(tmpscr->csensitive+1)/2),
                              by+zc_max((bigHitbox?tmpscr->csensitive:(tmpscr->csensitive+1)/2)-1,0), i-1, true);
            return;
        }
        if(isSideview() && // Check for sideview damage combos
                hclk<1 && action!=casting) // ... but only if Link could be hurt
        {
            if(checkdamagecombos(x+4, x+12, y+16, y+24))
                return;
        }
    }
    
    if(earlyReturn)
        return;
    
    int itemid=current_item_id(itype_bracelet);
    
    if((t==cPUSH_WAIT || t==cPUSH_HW || t==cPUSH_HW2) && (pushing<16 || hasMainGuy())) return;
    
    if((t==cPUSH_HW || t==cPUSH_HEAVY || t==cPUSH_HEAVY2 || t==cPUSH_HW2)
            && (itemid<0 || itemsbuf[itemid].power<((t==cPUSH_HEAVY2 || t==cPUSH_HW2)?2:1) ||
                ((itemid>=0 && itemsbuf[itemid].flags & ITEM_FLAG1) && (didstuff&did_glove)))) return;
                
    if(get_bit(quest_rules,qr_HESITANTPUSHBLOCKS)&&(pushing<4)) return;
    
    bool doit=false;
    bool changeflag=false;
    bool changecombo=false;
    
    if(((f==mfPUSHUD || f==mfPUSHUDNS|| f==mfPUSHUDINS) && dir<=down) ||
            ((f==mfPUSHLR || f==mfPUSHLRNS|| f==mfPUSHLRINS) && dir>=left) ||
            ((f==mfPUSHU || f==mfPUSHUNS || f==mfPUSHUINS) && dir==up) ||
            ((f==mfPUSHD || f==mfPUSHDNS || f==mfPUSHDINS) && dir==down) ||
            ((f==mfPUSHL || f==mfPUSHLNS || f==mfPUSHLINS) && dir==left) ||
            ((f==mfPUSHR || f==mfPUSHRNS || f==mfPUSHRINS) && dir==right) ||
            f==mfPUSH4 || f==mfPUSH4NS || f==mfPUSH4INS)
    {
        changeflag=true;
        doit=true;
    }
    
    if((((f2==mfPUSHUD || f2==mfPUSHUDNS|| f2==mfPUSHUDINS) && dir<=down) ||
            ((f2==mfPUSHLR || f2==mfPUSHLRNS|| f2==mfPUSHLRINS) && dir>=left) ||
            ((f2==mfPUSHU || f2==mfPUSHUNS || f2==mfPUSHUINS) && dir==up) ||
            ((f2==mfPUSHD || f2==mfPUSHDNS || f2==mfPUSHDINS) && dir==down) ||
            ((f2==mfPUSHL || f2==mfPUSHLNS || f2==mfPUSHLINS) && dir==left) ||
            ((f2==mfPUSHR || f2==mfPUSHRNS || f2==mfPUSHRINS) && dir==right) ||
            f2==mfPUSH4 || f2==mfPUSH4NS || f2==mfPUSH4INS)&&(f!=mfPUSHED))
    {
        changecombo=true;
        doit=true;
    }
    
    if(get_bit(quest_rules,qr_SOLIDBLK))
    {
        switch(dir)
        {
        case up:
            if(_walkflag(bx,by-8,2)&&!(MAPFLAG(bx,by-8)==mfBLOCKHOLE||MAPCOMBOFLAG(bx,by-8)==mfBLOCKHOLE))    doit=false;
            
            break;
            
        case down:
            if(_walkflag(bx,by+24,2)&&!(MAPFLAG(bx,by+24)==mfBLOCKHOLE||MAPCOMBOFLAG(bx,by+24)==mfBLOCKHOLE))   doit=false;
            
            break;
            
        case left:
            if(_walkflag(bx-16,by+8,2)&&!(MAPFLAG(bx-16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG(bx-16,by+8)==mfBLOCKHOLE)) doit=false;
            
            break;
            
        case right:
            if(_walkflag(bx+16,by+8,2)&&!(MAPFLAG(bx+16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG(bx+16,by+8)==mfBLOCKHOLE)) doit=false;
            
            break;
        }
    }
    
    switch(dir)
    {
    case up:
        if((MAPFLAG(bx,by-8)==mfNOBLOCKS||MAPCOMBOFLAG(bx,by-8)==mfNOBLOCKS))       doit=false;
        
        break;
        
    case down:
        if((MAPFLAG(bx,by+24)==mfNOBLOCKS||MAPCOMBOFLAG(bx,by+24)==mfNOBLOCKS))     doit=false;
        
        break;
        
    case left:
        if((MAPFLAG(bx-16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG(bx-16,by+8)==mfNOBLOCKS)) doit=false;
        
        break;
        
    case right:
        if((MAPFLAG(bx+16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG(bx+16,by+8)==mfNOBLOCKS)) doit=false;
        
        break;
    }
    
    if(doit)
    {
        if(itemid>=0 && itemsbuf[itemid].flags & ITEM_FLAG1) didstuff|=did_glove;
        
        //   for(int i=0; i<1; i++)
        if(!blockmoving)
        {
            if(changeflag)
            {
                tmpscr->sflag[(by&0xF0)+(bx>>4)]=0;
            }
            
            //if (changecombo)
            //{
            //++tmpscr->data[(by&0xF0)+(bx>>4)];
            //}
            if(mblock2.clk<=0)
            {
                mblock2.push((fix)bx,(fix)by,dir,f);
                
                if(get_bit(quest_rules,qr_MORESOUNDS))
                    sfx(WAV_ZN1PUSHBLOCK,(int)x);
                    
                //       break;
            }
        }
    }
}

bool usekey()
{
    int itemid = current_item_id(itype_magickey);
    
    if(itemid<0 ||
            (itemsbuf[itemid].flags & ITEM_FLAG1 ? itemsbuf[itemid].power<dlevel
             : itemsbuf[itemid].power!=dlevel))
    {
        if(game->lvlkeys[dlevel]!=0)
        {
            game->lvlkeys[dlevel]--;
            return true;
        }
        
        if(game->get_keys()==0)
            return false;
            
        game->change_keys(-1);
    }
    
    return true;
}

bool islockeddoor(int x, int y, int lock)
{
    int mc = (y&0xF0)+(x>>4);
    bool ret = (((mc==7||mc==8||mc==23||mc==24) && tmpscr->door[up]==lock)
                || ((mc==151||mc==152||mc==167||mc==168) && tmpscr->door[down]==lock)
                || ((mc==64||mc==65||mc==80||mc==81) && tmpscr->door[left]==lock)
                || ((mc==78||mc==79||mc==94||mc==95) && tmpscr->door[right]==lock));
    return ret;
}

void LinkClass::checklockblock()
{
    if(toogam) return;
    
    int bx = int(x)&0xF0;
    int bx2 = int(x+8)&0xF0;
    int by = int(y)&0xF0;
    
    switch(dir)
    {
    case up:
        if(!((int)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
        
        break;
        
    case down:
        by+=16;
        break;
        
    case left:
        if((((int)x)&0x0F)<8)
            bx-=16;
        
        if(int(y)&8)
        {
            by+=16;
        }
        
        bx2=bx;
        break;
        
    case right:
        bx+=16;
        
        if(int(y)&8)
        {
            by+=16;
        }
        
        bx2=bx;
        break;
    }
    
    bool found=false;
    
    // Layer 0 is overridden by Locked Doors
    if((combobuf[MAPCOMBO(bx,by)].type==cLOCKBLOCK && !islockeddoor(bx,by,dLOCKED))||
            (combobuf[MAPCOMBO(bx2,by)].type==cLOCKBLOCK && !islockeddoor(bx2,by,dLOCKED)))
    {
        found=true;
    }
    
    // Layers
    if(!found)
    {
        for(int i=0; i<2; i++)
        {
            if((combobuf[MAPCOMBO2(i,bx,by)].type==cLOCKBLOCK)||
                    (combobuf[MAPCOMBO2(i,bx2,by)].type==cLOCKBLOCK))
            {
                found=true;
                break;
            }
        }
    }
    
    if(!found || pushing<8)
    {
        return;
    }
    
    if(!usekey()) return;
    
    setmapflag(mLOCKBLOCK);
    remove_lockblocks((currscr>=128)?1:0);
    sfx(WAV_DOOR);
}

void LinkClass::checkbosslockblock()
{
    if(toogam) return;
    
    int bx = int(x)&0xF0;
    int bx2 = int(x+8)&0xF0;
    int by = int(y)&0xF0;
    
    switch(dir)
    {
    case up:
        if(!((int)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
        
        break;
        
    case down:
        by+=16;
        break;
        
    case left:
        if((((int)x)&0x0F)<8)
            bx-=16;
        
        if(int(y)&8)
        {
            by+=16;
        }
        
        bx2=bx;
        break;
        
    case right:
        bx+=16;
        
        if(int(y)&8)
        {
            by+=16;
        }
        
        bx2=bx;
        break;
    }
    
    bool found=false;
    
    if((combobuf[MAPCOMBO(bx,by)].type==cBOSSLOCKBLOCK && !islockeddoor(bx,by,dBOSS))||
            (combobuf[MAPCOMBO(bx2,by)].type==cBOSSLOCKBLOCK && !islockeddoor(bx,by,dBOSS)))
    {
        found=true;
    }
    
    if(!found)
    {
        for(int i=0; i<2; i++)
        {
            if((combobuf[MAPCOMBO2(i,bx,by)].type==cBOSSLOCKBLOCK)||
                    (combobuf[MAPCOMBO2(i,bx2,by)].type==cBOSSLOCKBLOCK))
            {
                found=true;
                break;
            }
        }
    }
    
    if(!found || pushing<8)
    {
        return;
    }
    
    if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
    
    setmapflag(mBOSSLOCKBLOCK);
    remove_bosslockblocks((currscr>=128)?1:0);
    sfx(WAV_DOOR);
}

void LinkClass::checkchest(int type)
{
    // chests aren't affected by tmpscr->flags2&fAIRCOMBOS
    if(toogam || z>0) return;
    
    int bx = int(x)&0xF0;
    int bx2 = int(x+8)&0xF0;
    int by = int(y)&0xF0;
    
    switch(dir)
    {
    case up:
        if(isSideview()) return;
        
        if(!((int)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
        
        break;
        
    case left:
    case right:
        if(isSideview()) break;
        
    case down:
        return;
    }
    
    bool found=false;
    bool itemflag=false;
    
    if((combobuf[MAPCOMBO(bx,by)].type==type)||
            (combobuf[MAPCOMBO(bx2,by)].type==type))
    {
        found=true;
    }
    
    if(!found)
    {
        for(int i=0; i<2; i++)
        {
            if((combobuf[MAPCOMBO2(i,bx,by)].type==type)||
                    (combobuf[MAPCOMBO2(i,bx2,by)].type==type))
            {
                found=true;
                break;
            }
        }
    }
    
    if(!found || pushing<8)
    {
        return;
    }
    
    switch(type)
    {
    case cLOCKEDCHEST:
        if(!usekey()) return;
        
        setmapflag(mLOCKEDCHEST);
        break;
        
    case cCHEST:
        setmapflag(mCHEST);
        break;
        
    case cBOSSCHEST:
        if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
        
        setmapflag(mBOSSCHEST);
        break;
    }
    
    itemflag |= MAPCOMBOFLAG(bx,by)==mfARMOS_ITEM;
    itemflag |= MAPCOMBOFLAG(bx2,by)==mfARMOS_ITEM;
    itemflag |= MAPFLAG(bx,by)==mfARMOS_ITEM;
    itemflag |= MAPFLAG(bx2,by)==mfARMOS_ITEM;
    itemflag |= MAPCOMBOFLAG(bx,by)==mfARMOS_ITEM;
    itemflag |= MAPCOMBOFLAG(bx2,by)==mfARMOS_ITEM;
    
    if(!itemflag)
    {
        for(int i=0; i<2; i++)
        {
            itemflag |= MAPFLAG2(i,bx,by)==mfARMOS_ITEM;
            itemflag |= MAPFLAG2(i,bx2,by)==mfARMOS_ITEM;
            itemflag |= MAPCOMBOFLAG2(i,bx,by)==mfARMOS_ITEM;
            itemflag |= MAPCOMBOFLAG2(i,bx2,by)==mfARMOS_ITEM;
        }
    }
    
    if(itemflag && !getmapflag())
    {
        items.add(new item(x, y,(fix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP, 0));
    }
}

void LinkClass::checklocked()
{
    if(toogam) return; //Walk through walls. 
    
    if(!isdungeon()) return;
    
    if( !diagonalMovement && pushing!=8) return;
	/*This is required to allow the player to open a door, while sliding along a wall (pressing in the direction of the door, and sliding left or right)
	*/
	if ( diagonalMovement && pushing < 8 ) return; //Allow wall walking Should I add a quest rule for this? -Z
    
	
	bool found;
	for ( int q = 0; q < 4; q++ ) {
		if ( tmpscr->door[q] == dLOCKED || tmpscr->door[q] == dBOSS ) { found = true; }
	}
	
    if ( !found ) return;
    
    int si = (currmap<<7) + currscr;
    int di;
    
    
	
	if ( diagonalMovement ) 
	{
		//Up door
		if ( y <= 32 && x >= 112 && x <= 128 )
		{
			if (
				dir == up || dir == l_up || dir == r_up //|| Up() || ( Up()&&Left()) || ( Up()&&Right()) 
				
			)
			{
				di = nextscr(up);
				if(tmpscr->door[0]==dLOCKED)
				{
				    if(usekey())
				    {
					putdoor(scrollbuf,0,up,dUNLOCKED);
					tmpscr->door[0]=dUNLOCKED;
					setmapflag(si, mDOOR_UP);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_DOWN);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
				else if(tmpscr->door[0]==dBOSS)
				{
				    if(game->lvlitems[dlevel]&liBOSSKEY)
				    {
					putdoor(scrollbuf,0,up,dOPENBOSS);
					tmpscr->door[0]=dOPENBOSS;
					setmapflag(si, mDOOR_UP);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_DOWN);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;

				}
					
			}
		}
		//Down
		if ( y >= 128 && x >= 112 && x <= 128 ) 
		{
			if ( dir == down || dir == l_down || dir == r_down ) //|| Down() || ( Down()&&Left()) || ( Down()&&Right()))
			{
				di = nextscr(down);
				if(tmpscr->door[1]==dLOCKED)
				{
				    if(usekey())
				    {
					putdoor(scrollbuf,0,down,dUNLOCKED);
					tmpscr->door[1]=dUNLOCKED;
					setmapflag(si, mDOOR_DOWN);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_UP);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
				else if(tmpscr->door[1]==dBOSS)
				{
				    if(game->lvlitems[dlevel]&liBOSSKEY)
				    {
					putdoor(scrollbuf,0,down,dOPENBOSS);
					tmpscr->door[1]=dOPENBOSS;
					setmapflag(si, mDOOR_DOWN);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_UP);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
			}
		}
		//left
		if ( y > 72 && y < 88 && x <= 32 )
		{
			if ( dir == left || dir == l_up || dir == l_down )//|| Left()  || ( Up()&&Left()) || ( Down()&&Left() ) )
			{
				di = nextscr(left);
				if(tmpscr->door[2]==dLOCKED)
				{
				    if(usekey())
				    {
					putdoor(scrollbuf,0,left,dUNLOCKED);
					tmpscr->door[2]=dUNLOCKED;
					setmapflag(si, mDOOR_LEFT);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_RIGHT);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
				else if(tmpscr->door[2]==dBOSS)
				{
				    if(game->lvlitems[dlevel]&liBOSSKEY)
				    {
					putdoor(scrollbuf,0,left,dOPENBOSS);
					tmpscr->door[2]=dOPENBOSS;
					setmapflag(si, mDOOR_LEFT);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_RIGHT);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
			}
		}
		
		
		//right
		if ( ( y > 72 && y < 88 ) && x >= 208 )
			//!( (y<=72||y>=88) && x<206 ) )
			//y<=72||y>=88):y!=80) || x<208)
		{
			if ( dir == right || dir == r_up || dir == r_down ) //|| Right()  || ( Down()&&Right() ) || ( Up()&&Right()))
			{
				di  = nextscr(right);
				if(tmpscr->door[right]==dLOCKED)
				{
				    if(usekey())
				    {
					putdoor(scrollbuf,0,right,dUNLOCKED);
					tmpscr->door[3]=dUNLOCKED;
					setmapflag(si, mDOOR_RIGHT);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_LEFT);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
				else if(tmpscr->door[right]==dBOSS)
				{
				    if(game->lvlitems[dlevel]&liBOSSKEY)
				    {
					putdoor(scrollbuf,0,right,dOPENBOSS);
					tmpscr->door[3]=dOPENBOSS;
					setmapflag(si, mDOOR_RIGHT);
					
					if(di != 0xFFFF)
					    setmapflag(di, mDOOR_LEFT);
					sfx(WAV_DOOR);
					markBmap(-1);
				    }
				    else return;
				}
			
			}
		}
	}
	else
	{
		//orthogonal movement
		//Up door
		if ( y<=32 && x == 120 )
			//!( y>32 && (x!=120) ))
		{
			switch ( dir ) 
			{
				case up:
				case r_up:
				case l_up:
				{
					di  = nextscr(up);
					if(tmpscr->door[0]==dLOCKED)
					{
					    if(usekey())
					    {
						putdoor(scrollbuf,0,up,dUNLOCKED);
						tmpscr->door[0]=dUNLOCKED;
						setmapflag(si, mDOOR_UP);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_DOWN);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					else if(tmpscr->door[0]==dBOSS)
					{
					    if(game->lvlitems[dlevel]&liBOSSKEY)
					    {
						putdoor(scrollbuf,0,up,dOPENBOSS);
						tmpscr->door[0]=dOPENBOSS;
						setmapflag(si, mDOOR_UP);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_DOWN);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					break;
				}
				default: break;
					
			}
		}
		//Down
		if ( y >= 128 && x == 120 )
			//!(y<128 && (x!=120) ) )
		{
			switch(dir)
			{
				case down:
				case l_down:
				case r_down:
				{
					di  = nextscr(down);
					
					if(tmpscr->door[1]==dLOCKED)
					{
					    if(usekey())
					    {
						putdoor(scrollbuf,0,down,dUNLOCKED);
						tmpscr->door[1]=dUNLOCKED;
						setmapflag(si, mDOOR_DOWN);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_UP);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					else if(tmpscr->door[1]==dBOSS)
					{
					    if(game->lvlitems[dlevel]&liBOSSKEY)
					    {
						putdoor(scrollbuf,0,down,dOPENBOSS);
						tmpscr->door[1]=dOPENBOSS;
						setmapflag(si, mDOOR_DOWN);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_UP);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					break;
				}
				default: break;
			}
		}
		//left
		if ( y == 80 && x <= 32 )
			//!( (y!=80) && x>32 ) )
		{
			switch(dir)
			{
				case left:
				case l_up:
				case l_down:
				{
					di  = nextscr(left);
					if(tmpscr->door[2]==dLOCKED)
					{
					    if(usekey())
					    {
						putdoor(scrollbuf,0,left,dUNLOCKED);
						tmpscr->door[2]=dUNLOCKED;
						setmapflag(si, mDOOR_LEFT);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_RIGHT);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					else if(tmpscr->door[2]==dBOSS)
					{
					    if(game->lvlitems[dlevel]&liBOSSKEY)
					    {
						putdoor(scrollbuf,0,left,dOPENBOSS);
						tmpscr->door[2]=dOPENBOSS;
						setmapflag(si, mDOOR_LEFT);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_RIGHT);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					
					break;	
					
				}
				default: break;
			}
		}
		//right
		if ( y == 80 && x >= 208 )
			//!((y!=80) && x<208 ) )
		{
			switch(dir)
			{
				case right:
				case r_down:
				case r_up:
				{
					di  = nextscr(right);
					if(tmpscr->door[3]==dLOCKED)
					{
					    if(usekey())
					    {
						putdoor(scrollbuf,0,right,dUNLOCKED);
						tmpscr->door[3]=dUNLOCKED;
						setmapflag(si, mDOOR_RIGHT);
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_LEFT);
						sfx(WAV_DOOR);
						markBmap(-1);
					    }
					    else return;
					}
					else if(tmpscr->door[3]==dBOSS)
					{
					    if(game->lvlitems[dlevel]&liBOSSKEY)
					    {
						putdoor(scrollbuf,0,right,dOPENBOSS);
						tmpscr->door[3]=dOPENBOSS;
						setmapflag(si, mDOOR_RIGHT);
						
						
						if(di != 0xFFFF)
						    setmapflag(di, mDOOR_LEFT);
						sfx(WAV_DOOR);
						markBmap(-1);

					    }
					    else return;
					}
					
					
					break;
				}
				default: break;
				
			}
		}
	}
	
	
		

    
    
}

void LinkClass::checkswordtap()
{
    if(attack!=wSword || charging<=0 || pushing<8) return;
    
    int bx=x;
    int by=y+8;
    
    switch(dir)
    {
    case up:
        if(!Up()) return;
        
        by-=16;
        break;
        
    case down:
        if(!Down()) return;
        
        by+=16;
        bx+=8;
        break;
        
    case left:
        if(!Left()) return;
        
        bx-=16;
        by+=8;
        break;
        
    case right:
        if(!Right()) return;
        
        bx+=16;
        by+=8;
        break;
    }
    
    if(!_walkflag(bx,by,0)) return;
    
    attackclk=SWORDTAPFRAME;
    pushing=-8; //16 frames between taps
    tapping=true;
    
    int type = COMBOTYPE(bx,by);
    
    if(!isCuttableType(type))
    {
        bool hollow = (MAPFLAG(bx,by) == mfBOMB || MAPCOMBOFLAG(bx,by) == mfBOMB ||
                       MAPFLAG(bx,by) == mfSBOMB || MAPCOMBOFLAG(bx,by) == mfSBOMB);
                       
        // Layers
        for(int i=0; i < 6; i++)
            hollow = (hollow || MAPFLAG2(i,bx,by) == mfBOMB || MAPCOMBOFLAG2(i,bx,by) == mfBOMB ||
                      MAPFLAG2(i,bx,by) == mfSBOMB || MAPCOMBOFLAG2(i,bx,by) == mfSBOMB);
                      
        for(int i=0; i<4; i++)
            if(tmpscr->door[i]==dBOMB && i==dir)
                switch(i)
                {
                case up:
                case down:
                    if(bx>=112 && bx<144 && (by>=144 || by<=32)) hollow=true;
                    
                    break;
                    
                case left:
                case right:
                    if(by>=72 && by<104 && (bx>=224 || bx<=32)) hollow=true;
                    
                    break;
                }
                
        sfx(hollow ? WAV_ZN1TAP2 : WAV_ZN1TAP,pan(int(x)));
    }
    
}

void LinkClass::fairycircle(int type)
{
    if(fairyclk==0)
    {
        switch(type)
        {
        case REFILL_LIFE:
            if(didstuff&did_fairy) return;
            
            didstuff|=did_fairy;
            break;
            
        case REFILL_MAGIC:
            if(didstuff&did_magic) return;
            
            didstuff|=did_magic;
            break;
            
        case REFILL_ALL:
            if(didstuff&did_all) return;
            
            didstuff|=did_all;
        }
        
        refill_what=type;
        refill_why=REFILL_FAIRY;
        StartRefill(type);
        action=freeze; FFCore.setLinkAction(freeze);
        holdclk=0;
        hopclk=0;
    }
    
    ++fairyclk;
    
    if(refilling!=REFILL_FAIRYDONE)
    {
        if(!refill())
            refilling=REFILL_FAIRYDONE;
    }
    
    else if(++holdclk>80)
    {
        reset_swordcharge();
        attackclk=0;
        action=none; FFCore.setLinkAction(none);
        fairyclk=0;
        holdclk=0;
        refill_why = 0;
        refilling=REFILL_NONE;
        map_bkgsfx(true);
    }
}

int touchcombo(int x,int y)
{
    switch(combobuf[MAPCOMBO(x,y)].type)
    {
    case cBSGRAVE:
    case cGRAVE:
        if(MAPFLAG(x,y)||MAPCOMBOFLAG(x,y))
        {
            break;
        }
        
        // fall through
    case cARMOS:
    {
        return combobuf[MAPCOMBO(x,y)].type;
    }
    }
    
    return 0;
}

void LinkClass::checktouchblk()
{
    if(toogam) return;
    
    if(!pushing)
        return;
        
    int tdir = dir; //Bad hack #2. _L_, your welcome to fix this properly. ;)
    
    if(charging > 0 || spins > 0) //if not I probably will at some point...
    {
        if(Up()&&Left())tdir = (charging%2)*2;
        else if(Up()&&Right())tdir = (charging%2)*3;
        else if(Down()&&Left())tdir = 1+(charging%2)*1;
        else if(Down()&&Right())tdir = 1+(charging%2)*2;
        else
        {
            if(Up())tdir=0;
            else if(Down())tdir=1;
            else if(Left())tdir=2;
            else if(Right())tdir=3;
        }
    }
    
    int tx=0,ty=-1;
    
    switch(tdir)
    {
    case up:
        if(touchcombo(x,y+(bigHitbox?0:7)))
        {
            tx=x;
            ty=y+(bigHitbox?0:7);
        }
        else if(touchcombo(x+8,y+(bigHitbox?0:7)))
        {
            tx=x+8;
            ty=y+(bigHitbox?0:7);
        }
        
        break;
        
    case down:
        if(touchcombo(x,y+16))
        {
            tx=x;
            ty=y+16;
        }
        else if(touchcombo(x+8,y+16))
        {
            tx=x+8;
            ty=y+16;
        }
        
        break;
        
    case left:
        if(touchcombo(x-1,y+15))
        {
            tx=x-1;
            ty=y+15;
        }
        
        break;
        
    case right:
        if(touchcombo(x+16,y+15))
        {
            tx=x+16;
            ty=y+15;
        }
        
        break;
    }
    
    if(ty>=0)
    {
        ty&=0xF0;
        tx&=0xF0;
        int di = ty+(tx>>4);
        int gc=0;
        int eclk=-14;
        
        for(int i=0; i<guys.Count(); ++i)
        {
            if(((enemy*)guys.spr(i))->mainguy)
            {
                ++gc;
            }
        }
        
        if(di<176 && !guygrid[di] && gc<11)
        {
            if((getAction() != hopping || isSideview()))
            {
                guygrid[di]=61; //Note: not 60.
                int id2=0;
                
                switch(combobuf[MAPCOMBO(tx,ty)].type)
                {
                case cARMOS: //id2=eARMOS; break;
                    for(int i=0; i<eMAXGUYS; i++)
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
                    
                    break;
                    
                case cBSGRAVE:
                    tmpscr->data[di]++;
                    
                    //fall through
                case cGRAVE:
                    for(int i=0; i<eMAXGUYS; i++)
                    {
                        if(guysbuf[i].flags2&cmbflag_ghini)
                        {
                            id2=i;
                            eclk=0; // This is mostly for backwards-compatability
                            break;
                        }
                    }
                    
                    //id2=eGHINI2;
                    break;
                }
                
                addenemy(tx,ty+3,id2,eclk);
                ((enemy*)guys.spr(guys.Count()-1))->did_armos=false;
            }
        }
    }
}

int LinkClass::nextcombo(int cx, int cy, int cdir)
{
    switch(cdir)
    {
    case up:
        cy-=16;
        break;
        
    case down:
        cy+=16;
        break;
        
    case left:
        cx-=16;
        break;
        
    case right:
        cx+=16;
        break;
    }
    
    // off the screen
    if(cx<0 || cy<0 || cx>255 || cy>175)
    {
        int ns = nextscr(cdir);
        
        if(ns==0xFFFF) return 0;
        
        // want actual screen index, not game->maps[] index
        ns = (ns&127) + (ns>>7)*MAPSCRS;
        
        switch(cdir)
        {
        case up:
            cy=160;
            break;
            
        case down:
            cy=0;
            break;
            
        case left:
            cx=240;
            break;
            
        case right:
            cx=0;
            break;
        }
        
        // from MAPCOMBO()
        int cmb = (cy&0xF0)+(cx>>4);
        
        if(cmb>175)
            return 0;
            
        return TheMaps[ns].data[cmb];                           // entire combo code
    }
    
    return MAPCOMBO(cx,cy);
}

int LinkClass::nextflag(int cx, int cy, int cdir, bool comboflag)
{
    switch(cdir)
    {
    case up:
        cy-=16;
        break;
        
    case down:
        cy+=16;
        break;
        
    case left:
        cx-=16;
        break;
        
    case right:
        cx+=16;
        break;
    }
    
    // off the screen
    if(cx<0 || cy<0 || cx>255 || cy>175)
    {
        int ns = nextscr(cdir);
        
        if(ns==0xFFFF) return 0;
        
        // want actual screen index, not game->maps[] index
        ns = (ns&127) + (ns>>7)*MAPSCRS;
        
        switch(cdir)
        {
        case up:
            cy=160;
            break;
            
        case down:
            cy=0;
            break;
            
        case left:
            cx=240;
            break;
            
        case right:
            cx=0;
            break;
        }
        
        // from MAPCOMBO()
        int cmb = (cy&0xF0)+(cx>>4);
        
        if(cmb>175)
            return 0;
            
        if(!comboflag)
        {
            return TheMaps[ns].sflag[cmb];                          // flag
        }
        else
        {
            return combobuf[TheMaps[ns].data[cmb]].flag;                          // flag
        }
    }
    
    if(comboflag)
    {
        return MAPCOMBOFLAG(cx,cy);
    }
    
    return MAPFLAG(cx,cy);
}

bool did_secret;

void LinkClass::checkspecial()
{
    checktouchblk();
    
    bool hasmainguy = hasMainGuy();                           // calculate it once
    
    if(!(loaded_enemies && !hasmainguy))
        did_secret=false;
    else
    {
        // after beating enemies
        
        // if room has traps, guys don't come back
        for(int i=0; i<eMAXGUYS; i++)
        {
            if(guysbuf[i].family==eeTRAP&&guysbuf[i].misc2)
                if(guys.idCount(i))
                    setmapflag(mTMPNORET);
        }
        
        // item
        if(hasitem)
        {
            int Item=tmpscr->item;
            
            //if(getmapflag())
            //  Item=0;
            if(!getmapflag(mITEM) && (tmpscr->hasitem != 0))
            {
                if(hasitem==1)
                    sfx(WAV_CLEARED);
                    
                items.add(new item((fix)tmpscr->itemx,
                                   (tmpscr->flags7&fITEMFALLS && isSideview()) ? (fix)-170 : (fix)tmpscr->itemy+1,
                                   (tmpscr->flags7&fITEMFALLS && !isSideview()) ? (fix)170 : (fix)0,
                                   Item,ipONETIME+ipBIGRANGE+((itemsbuf[Item].family==itype_triforcepiece ||
                                           (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0),0));
            }
            
            hasitem=0;
        }
        
        // clear enemies and open secret
        if(!did_secret && (tmpscr->flags2&fCLEARSECRET))
        {
            hidden_entrance(0,true,true,-2);
            
            if(tmpscr->flags4&fENEMYSCRTPERM && !isdungeon())
            {
                if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
            }
            
            sfx(tmpscr->secretsfx);
            did_secret=true;
        }
    }
    
    // doors
    for(int i=0; i<4; i++)
        if(tmpscr->door[i]==dSHUTTER)
        {
            if(opendoors==0 && loaded_enemies)
            {
                if(!(tmpscr->flags&fSHUTTERS) && !hasmainguy)
                    opendoors=12;
            }
            else if(opendoors<0)
                ++opendoors;
            else if((--opendoors)==0)
                openshutters();
                
            break;
        }
        
    // set boss flag when boss is gone
    if(loaded_enemies && tmpscr->enemyflags&efBOSS && !hasmainguy)
    {
        game->lvlitems[dlevel]|=liBOSS;
        stop_sfx(tmpscr->bosssfx);
    }
    
    if(getmapflag(mCHEST))              // if special stuff done before
    {
        remove_chests((currscr>=128)?1:0);
    }
    
    if(getmapflag(mLOCKEDCHEST))              // if special stuff done before
    {
        remove_lockedchests((currscr>=128)?1:0);
    }
    
    if(getmapflag(mBOSSCHEST))              // if special stuff done before
    {
        remove_bosschests((currscr>=128)?1:0);
    }
}

void LinkClass::checkspecial2(int *ls)
{
    if(get_bit(quest_rules,qr_OLDSTYLEWARP) && !diagonalMovement)
    {
        if(int(y)&7)
            return;
            
        if(int(x)&7)
            return;
    }
    
    if(toogam) return;
    
    bool didstrig = false;
    
    for(int i=bigHitbox?0:8; i<16; i+=bigHitbox?15:7)
    {
        for(int j=0; j<16; j+=15) for(int k=0; k<2; k++)
        {
            int stype = combobuf[k>0 ? MAPFFCOMBO(x+j,y+i) : MAPCOMBO(x+j,y+i)].type;
            
            if(stype==cSWARPA)
            {
                if(tmpscr->flags5&fDIRECTSWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir=dir;
                dowarp(0,0);
                return;
            }
            
            if(stype==cSWARPB)
            {
                if(tmpscr->flags5&fDIRECTSWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir=dir;
                dowarp(0,1);
                return;
            }
            
            if(stype==cSWARPC)
            {
                if(tmpscr->flags5&fDIRECTSWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir=dir;
                dowarp(0,2);
                return;
            }
            
            if(stype==cSWARPD)
            {
                if(tmpscr->flags5&fDIRECTSWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir=dir;
                dowarp(0,3);
                return;
            }
            
            if(stype==cSWARPR)
            {
                if(tmpscr->flags5&fDIRECTSWARP)
                {
                    didpit=true;
                    pitx=x;
                    pity=y;
                }
                
                sdir=dir;
                dowarp(0,rand()%4);
                return;
            }
            
            if((stype==cSTRIGNOFLAG || stype==cSTRIGFLAG) && stepsecret!=MAPCOMBO(x+j,y+i))
            {
                if(stype==cSTRIGFLAG && !isdungeon())
                {
                    if(!didstrig)
                    {
                        stepsecret = ((int)(y+i)&0xF0)+((int)(x+j)>>4);
                        
                        if(!(tmpscr->flags5&fTEMPSECRETS))
                        {
                            setmapflag(mSECRET);
                        }
                        
                        hidden_entrance(0,true,false);
                        didstrig = true;
                    }
                }
                else
                {
                    if(!didstrig)
                    {
                        stepsecret = ((int)(y+i)&0xF0)+((int)(x+j)>>4);
                        hidden_entrance(0,true,true);
                        didstrig = true;
                    }
                }
            }
        }
    }
    
    // check if he's standing on a warp he just came out of
    if(((int)y>=warpy-8&&(int)y<=warpy+7)&&warpy!=-1)
    {
        if(((int)x>=warpx-8&&(int)x<=warpx+7)&&warpx!=-1)
        {
            return;
        }
    }
    
    warpy=255;
    int tx=x;
    int ty=y;
    
    int flag=0;
    int flag2=0;
    int flag3=0;
    int type=0;
    bool water=false;
    int index = 0;
    
    //bool gotpit=false;
    
    int x1,x2,y1,y2;
    x1 = tx;
    x2 = tx+15;
    y1 = ty;
    y2 = ty+15;
    
    if(diagonalMovement)
    {
        x1 = tx+4;
        x2 = tx+11;
        y1 = ty+4;
        y2 = ty+11;
    }
    
    int types[4];
    types[0]=types[1]=types[2]=types[3]=-1;
    
    //
    // First, let's find flag1 (combo flag), flag2 (inherent flag) and flag3 (FFC flag)...
    //
    types[0] = MAPFLAG(x1,y1);
    types[1] = MAPFLAG(x1,y2);
    types[2] = MAPFLAG(x2,y1);
    types[3] = MAPFLAG(x2,y2);
    
    if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
        flag = types[0];
        
    // 2.10 compatibility...
    else if(int(y)%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
        flag = types[0];
        
        
    types[0] = MAPCOMBOFLAG(x1,y1);
    types[1] = MAPCOMBOFLAG(x1,y2);
    types[2] = MAPCOMBOFLAG(x2,y1);
    types[3] = MAPCOMBOFLAG(x2,y2);
    
    if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
        flag2 = types[0];
        
    types[0] = MAPFFCOMBOFLAG(x1,y1);
    types[1] = MAPFFCOMBOFLAG(x1,y2);
    types[2] = MAPFFCOMBOFLAG(x2,y1);
    types[3] = MAPFFCOMBOFLAG(x2,y2);
    
    if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
        flag3 = types[0];
        
    //
    // Now, let's check for warp combos...
    //
    types[0] = COMBOTYPE(x1,y1);
    
    if(MAPFFCOMBO(x1,y1))
        types[0] = FFCOMBOTYPE(x1,y1);
        
    types[1] = COMBOTYPE(x1,y2);
    
    if(MAPFFCOMBO(x1,y2))
        types[1] = FFCOMBOTYPE(x1,y2);
        
    types[2] = COMBOTYPE(x2,y1);
    
    if(MAPFFCOMBO(x2,y1))
        types[2] = FFCOMBOTYPE(x2,y1);
        
    types[3] = COMBOTYPE(x2,y2);
    
    if(MAPFFCOMBO(x2,y2))
        types[3] = FFCOMBOTYPE(x2,y2);
        
    // Change B, C and D warps into A, for the comparison below...
    for(int i=0; i<4; i++)
    {
        if(types[i]==cCAVE)
        {
            index=0;
        }
        else if(types[i]==cCAVEB)
        {
            types[i]=cCAVE;
            index=1;
        }
        else if(types[i]==cCAVEC)
        {
            types[i]=cCAVE;
            index=2;
        }
        else if(types[i]==cCAVED)
        {
            types[i]=cCAVE;
            index=3;
        }
        
        if(types[i]==cPIT) index=0;
        else if(types[i]==cPITB)
        {
            types[i]=cPIT;
            index=1;
        }
        else if(types[i]==cPITC)
        {
            types[i]=cPIT;
            index=2;
        }
        else if(types[i]==cPITD)
        {
            types[i]=cPIT;
            index=3;
        }
        else if(types[i]==cPITR)
        {
            types[i]=cPIT;
            index=rand()%4;
        }
        
        if(types[i]==cSTAIR)
        {
            index=0;
        }
        else if(types[i]==cSTAIRB)
        {
            types[i]=cSTAIR;
            index=1;
        }
        else if(types[i]==cSTAIRC)
        {
            types[i]=cSTAIR;
            index=2;
        }
        else if(types[i]==cSTAIRD)
        {
            types[i]=cSTAIR;
            index=3;
        }
        else if(types[i]==cSTAIRR)
        {
            types[i]=cSTAIR;
            index=rand()%4;
        }
        
        if(types[i]==cCAVE2)
        {
            index=0;
        }
        else if(types[i]==cCAVE2B)
        {
            types[i]=cCAVE2;
            index=1;
        }
        else if(types[i]==cCAVE2C)
        {
            types[i]=cCAVE2;
            index=2;
        }
        else if(types[i]==cCAVE2D)
        {
            types[i]=cCAVE2;
            index=3;
        }
        
        if(types[i]==cSWIMWARP) index=0;
        else if(types[i]==cSWIMWARPB)
        {
            types[i]=cSWIMWARP;
            index=1;
        }
        else if(types[i]==cSWIMWARPC)
        {
            types[i]=cSWIMWARP;
            index=2;
        }
        else if(types[i]==cSWIMWARPD)
        {
            types[i]=cSWIMWARP;
            index=3;
        }
        
        if(types[i]==cDIVEWARP) index=0;
        else if(types[i]==cDIVEWARPB)
        {
            types[i]=cDIVEWARP;
            index=1;
        }
        else if(types[i]==cDIVEWARPC)
        {
            types[i]=cDIVEWARP;
            index=2;
        }
        else if(types[i]==cDIVEWARPD)
        {
            types[i]=cDIVEWARP;
            index=3;
        }
        
        if(types[i]==cSTEP) ;
        else if(types[i]==cSTEPSAME) types[i]=cSTEP;
        else if(types[i]==cSTEPALL) types[i]=cSTEP;
    }
    
    // Special case for step combos; otherwise, they act oddly in some cases
    if((types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])||(types[1]==cSTEP&&types[3]==cSTEP))
    {
        if(action!=freeze&&(!msg_active || !get_bit(quest_rules,qr_MSGFREEZE)))
            type = types[1];
    }
    
    //
    // Now, let's check for Save combos...
    //
    x1 = tx+4;
    x2 = tx+11;
    y1 = ty+4;
    y2 = ty+11;
    
    types[0] = COMBOTYPE(x1,y1);
    
    if(MAPFFCOMBO(x1,y1))
        types[0] = FFCOMBOTYPE(x1,y1);
        
    types[1] = COMBOTYPE(x1,y2);
    
    if(MAPFFCOMBO(x1,y2))
        types[1] = FFCOMBOTYPE(x1,y2);
        
    types[2] = COMBOTYPE(x2,y1);
    
    if(MAPFFCOMBO(x2,y1))
        types[2] = FFCOMBOTYPE(x2,y1);
        
    types[3] = COMBOTYPE(x2,y2);
    
    if(MAPFFCOMBO(x2,y2))
        types[3] = FFCOMBOTYPE(x2,y2);
        
    bool setsave=false;
    
    for(int i=0; i<4; i++)
    {
        if(types[i]==cSAVE) setsave=true;
        
        if(types[i]==cSAVE2) setsave=true;
    }
    
    if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
        if(setsave)
        {
            type = types[0];
        }
        
        
    //
    // Now, let's check for Drowning combos...
    //
    if(get_bit(quest_rules,qr_DROWN))
    {
        y1 = ty+9;
        y2 = ty+15;
        types[0] = COMBOTYPE(x1,y1);
        
        if(MAPFFCOMBO(x1,y1))
            types[0] = FFCOMBOTYPE(x1,y1);
            
        types[1] = COMBOTYPE(x1,y2);
        
        if(MAPFFCOMBO(x1,y2))
            types[1] = FFCOMBOTYPE(x1,y2);
            
        types[2] = COMBOTYPE(x2,y1);
        
        if(MAPFFCOMBO(x2,y1))
            types[2] = FFCOMBOTYPE(x2,y1);
            
        types[3] = COMBOTYPE(x2,y2);
        
        if(MAPFFCOMBO(x2,y2))
            types[3] = FFCOMBOTYPE(x2,y2);
            
        if(combo_class_buf[types[0]].water && combo_class_buf[types[1]].water &&
                combo_class_buf[types[2]].water && combo_class_buf[types[3]].water)
            water = true;
    }
    
    // Pits have a bigger 'hitbox' than stairs...
    x1 = tx+7;
    x2 = tx+8;
    y1 = ty+7+(bigHitbox?0:4);
    y2 = ty+8+(bigHitbox?0:4);
    
    types[0] = COMBOTYPE(x1,y1);
    
    if(MAPFFCOMBO(x1,y1))
        types[0] = FFCOMBOTYPE(x1,y1);
        
    types[1] = COMBOTYPE(x1,y2);
    
    if(MAPFFCOMBO(x1,y2))
        types[1] = FFCOMBOTYPE(x1,y2);
        
    types[2] = COMBOTYPE(x2,y1);
    
    if(MAPFFCOMBO(x2,y1))
        types[2] = FFCOMBOTYPE(x2,y1);
        
    types[3] = COMBOTYPE(x2,y2);
    
    if(MAPFFCOMBO(x2,y2))
        types[3] = FFCOMBOTYPE(x2,y2);
        
    for(int i=0; i<4; i++)
    {
        if(types[i]==cPIT) index=0;
        else if(types[i]==cPITB)
        {
            types[i]=cPIT;
            index=1;
        }
        else if(types[i]==cPITC)
        {
            types[i]=cPIT;
            index=2;
        }
        else if(types[i]==cPITD)
        {
            types[i]=cPIT;
            index=3;
        }
    }
    
    if(types[0]==cPIT||types[1]==cPIT||types[2]==cPIT||types[3]==cPIT)
        if(action!=freeze&& (!msg_active || !get_bit(quest_rules,qr_MSGFREEZE)))
            type=cPIT;
            
    //
    // Time to act on our results for type, flag, flag2 and flag3...
    //
    if(type==cSAVE&&currscr<128)
        *ls=1;
        
    if(type==cSAVE2&&currscr<128)
        *ls=2;
        
    if(refilling==REFILL_LIFE || flag==mfFAIRY||flag2==mfFAIRY||flag3==mfFAIRY)
    {
        fairycircle(REFILL_LIFE);
        
        if(fairyclk!=0) return;
    }
    
    if(refilling==REFILL_MAGIC || flag==mfMAGICFAIRY||flag2==mfMAGICFAIRY||flag3==mfMAGICFAIRY)
    {
        fairycircle(REFILL_MAGIC);
        
        if(fairyclk!=0) return;
    }
    
    if(refilling==REFILL_ALL || flag==mfALLFAIRY||flag2==mfALLFAIRY||flag3==mfALLFAIRY)
    {
        fairycircle(REFILL_ALL);
        
        if(fairyclk!=0) return;
    }
    
    // Just in case Link was moved off of the fairy flag
    if(refilling==REFILL_FAIRYDONE)
    {
        fairycircle(REFILL_NONE);
        
        if(fairyclk!=0) return;
    }
    
    if(flag==mfZELDA||flag2==mfZELDA||flag3==mfZELDA || combo_class_buf[type].win_game)
    {
        saved_Zelda();
        return;
    }
    
    if(z>0 && !(tmpscr->flags2&fAIRCOMBOS))
        return;
        
    if((type==cTRIGNOFLAG || type==cTRIGFLAG))
    {
        if((((ty+8)&0xF0)+((tx+8)>>4))!=stepsecret || get_bit(quest_rules,qr_TRIGGERSREPEAT))
        {
            stepsecret = (((ty+8)&0xF0)+((tx+8)>>4));
            
            if(type==cTRIGFLAG && !isdungeon())
            {
                if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
                
                hidden_entrance(0,true,false);
            }
            else
                hidden_entrance(0,true,true);
        }
    }
    else if(!didstrig)
    {
        stepsecret = -1;
    }
    
    // Drown if:
    // * Water (obviously walkable),
    // * Quest Rule allows it,
    // * Not on stepladder,
    // * Not jumping,
    // * Not hovering,
    // * Not rafting,
    // * Not swimming,
    // * Not swallowed,
    // * Not a dried lake.
    if(water && get_bit(quest_rules,qr_DROWN) && z==0  && fall>=0 && !ladderx && !hoverclk && action!=rafting && !isSwimming() && !inlikelike && !DRIEDLAKE)
    {
        if(!current_item(itype_flippers))
        {
            Drown();
        }
        else
        {
            attackclk = charging = spins = 0;
            action=swimming; FFCore.setLinkAction(swimming);
            landswim=0;
        }
        
        return;
    }
    
    if(type==cSTEP)
    {
        if((((ty+8)&0xF0)+((tx+8)>>4))!=stepnext)
        {
            stepnext=((ty+8)&0xF0)+((tx+8)>>4);
            
            if(COMBOTYPE(tx+8,ty+8)==cSTEP)
            {
                tmpscr->data[stepnext]++;
            }
            
            if(COMBOTYPE(tx+8,ty+8)==cSTEPSAME)
            {
                int stepc = tmpscr->data[stepnext];
                
                for(int k=0; k<176; k++)
                {
                    if(tmpscr->data[k]==stepc)
                    {
                        tmpscr->data[k]++;
                    }
                }
            }
            
            if(COMBOTYPE(tx+8,ty+8)==cSTEPALL)
            {
                for(int k=0; k<176; k++)
                {
                    if(
                        (combobuf[tmpscr->data[k]].type==cSTEP)||
                        (combobuf[tmpscr->data[k]].type==cSTEPSAME)||
                        (combobuf[tmpscr->data[k]].type==cSTEPALL)||
                        (combobuf[tmpscr->data[k]].type==cSTEPCOPY)
                    )
                    {
                        tmpscr->data[k]++;
                    }
                }
            }
        }
    }
    else stepnext = -1;
    
    detail_int[0]=tx;
    detail_int[1]=ty;
    
    if(!((type==cCAVE || type==cCAVE2) && z==0) && type!=cSTAIR &&
            type!=cPIT && type!=cSWIMWARP && type!=cRESET &&
            !(type==cDIVEWARP && diveclk>30))
    {
        switch(flag)
        {
        case mfDIVE_ITEM:
            if(diveclk>30 && !getmapflag())
            {
                additem(x, y, tmpscr->catchall,
                        ipONETIME2 + ipBIGRANGE + ipHOLDUP + ipNODRAW);
                sfx(tmpscr->secretsfx);
            }
            
            return;
            
        case mfRAFT:
        case mfRAFT_BRANCH:
        
            //        if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
            if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && z==0 && combo_class_buf[type].dock)
            {
                if(isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true)))
                {
                    reset_swordcharge();
                    action=rafting; FFCore.setLinkAction(rafting);
                    sfx(tmpscr->secretsfx);
                }
            }
            
            return;
            
        default:
            break;
            //return;
        }
        
        switch(flag2)
        {
        case mfDIVE_ITEM:
            if(diveclk>30 && !getmapflag())
            {
                additem(x, y, tmpscr->catchall,
                        ipONETIME2 + ipBIGRANGE + ipHOLDUP + ipNODRAW);
                sfx(tmpscr->secretsfx);
            }
            
            return;
            
        case mfRAFT:
        case mfRAFT_BRANCH:
        
            //        if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
            if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && z==0 && combo_class_buf[type].dock)
            {
                if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
                {
                    reset_swordcharge();
                    action=rafting; FFCore.setLinkAction(rafting);
                    sfx(tmpscr->secretsfx);
                }
            }
            
            return;
            
        default:
            break;
            //return;
        }
        
        switch(flag3)
        {
        case mfDIVE_ITEM:
            if(diveclk>30 && !getmapflag())
            {
                additem(x, y, tmpscr->catchall,
                        ipONETIME2 + ipBIGRANGE + ipHOLDUP + ipNODRAW);
                sfx(tmpscr->secretsfx);
            }
            
            return;
            
        case mfRAFT:
        case mfRAFT_BRANCH:
        
            //      if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
            if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && z==0 && combo_class_buf[type].dock)
            {
                if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
                {
                    reset_swordcharge();
                    action=rafting; FFCore.setLinkAction(rafting);
                    sfx(tmpscr->secretsfx);
                }
            }
            
            return;
            
        default:
            return;
        }
    }
    
    int t=(currscr<128)?0:1;
    
    if((type==cCAVE || type==cCAVE2) && (tmpscr[t].tilewarptype[index]==wtNOWARP)) return;
    
    //don't do this for canceled warps -DD
    //I have no idea why we do this skip, but I'll dutifully propagate it to all cases below...
    /*if(tmpscr[t].tilewarptype[index] != wtNOWARP)
    {
    	draw_screen(tmpscr);
    	advanceframe(true);
    }*/
    
    bool skippedaframe=false;
    
    if(type==cCAVE || type==cCAVE2 || type==cSTAIR)
    {
        // Stop music only if:
        // * entering a Guy Cave
        // * warping to a DMap whose music is different.
        
        int tdm = tmpscr[t].tilewarpdmap[index];
        
        if(tmpscr[t].tilewarptype[index]<=wtPASS)
        {
            if((DMaps[currdmap].flags&dmfCAVES) && tmpscr[t].tilewarptype[index] == wtCAVE)
                music_stop();
        }
        else
        {
            if(zcmusic!=NULL)
            {
                if(strcmp(zcmusic->filename, DMaps[tdm].tmusic) != 0 ||
                        zcmusic->track!=DMaps[tdm].tmusictrack)
                    music_stop();
            }
            else if(DMaps[tmpscr->tilewarpdmap[index]].midi != (currmidi-ZC_MIDI_COUNT+4) &&
                    TheMaps[(DMaps[tdm].map*MAPSCRS + (tmpscr[t].tilewarpscr[index] + DMaps[tdm].xoff))].screen_midi != (currmidi-ZC_MIDI_COUNT+4))
                music_stop();
        }
        
        stop_sfx(WAV_ER);
        bool opening = (tmpscr[t].tilewarptype[index]<=wtPASS && !(DMaps[currdmap].flags&dmfCAVES && tmpscr[t].tilewarptype[index]==wtCAVE)
                        ? false : COOLSCROLL);
                        
        draw_screen(tmpscr);
        advanceframe(true);
        
        skippedaframe=true;
        
        if(type==cCAVE2) walkup2(opening);
        else if(type==cCAVE) walkdown(opening);
    }
    
    if(type==cPIT)
    {
        didpit=true;
        pitx=x;
        pity=y;
    }
    
    if(DMaps[currdmap].flags&dmf3STAIR && (currscr==129 || !(DMaps[currdmap].flags&dmfGUYCAVES))
            && tmpscr[specialcave > 0 && DMaps[currdmap].flags&dmfGUYCAVES ? 1:0].room==rWARP && type==cSTAIR)
    {
        if(!skippedaframe)
        {
            draw_screen(tmpscr);
            advanceframe(true);
        }
        
        // "take any road you want"
        int dw = x<112 ? 1 : (x>136 ? 3 : 2);
        int code = WARPCODE(currdmap,homescr,dw);
        
        if(code>-1)
        {
            currdmap = code>>8;
            dlevel  = DMaps[currdmap].level;
            currmap = DMaps[currdmap].map;
            homescr = (code&0xFF) + DMaps[currdmap].xoff;
            init_dmap();
            
            if(!isdungeon())
                setmapflag(mSECRET);
        }
        
        if(specialcave==STAIRCAVE) exitcave();
        
        return;
    }
    
    if(type==cRESET)
    {
        if(!skippedaframe)
        {
            draw_screen(tmpscr);
            advanceframe(true);
        }
        
        if(!(tmpscr->noreset&mSECRET)) unsetmapflag(mSECRET);
        
        if(!(tmpscr->noreset&mITEM)) unsetmapflag(mITEM);
        
        if(!(tmpscr->noreset&mBELOW)) unsetmapflag(mBELOW);
        
        if(!(tmpscr->noreset&mNEVERRET)) unsetmapflag(mNEVERRET);
        
        if(!(tmpscr->noreset&mCHEST)) unsetmapflag(mCHEST);
        
        if(!(tmpscr->noreset&mLOCKEDCHEST)) unsetmapflag(mLOCKEDCHEST);
        
        if(!(tmpscr->noreset&mBOSSCHEST)) unsetmapflag(mBOSSCHEST);
        
        if(!(tmpscr->noreset&mLOCKBLOCK)) unsetmapflag(mLOCKBLOCK);
        
        if(!(tmpscr->noreset&mBOSSLOCKBLOCK)) unsetmapflag(mBOSSLOCKBLOCK);
        
        if(isdungeon())
        {
            if(!(tmpscr->noreset&mDOOR_LEFT)) unsetmapflag(mDOOR_LEFT);
            
            if(!(tmpscr->noreset&mDOOR_RIGHT)) unsetmapflag(mDOOR_RIGHT);
            
            if(!(tmpscr->noreset&mDOOR_DOWN)) unsetmapflag(mDOOR_DOWN);
            
            if(!(tmpscr->noreset&mDOOR_UP)) unsetmapflag(mDOOR_UP);
        }
        
        didpit=true;
        pitx=x;
        pity=y;
        sdir=dir;
        dowarp(4,0);
    }
    else
    {
        if(!skippedaframe && (tmpscr[t].tilewarptype[index]!=wtNOWARP))
        {
            draw_screen(tmpscr);
            advanceframe(true);
        }
        
        sdir = dir;
        dowarp(0,index);
    }
}

int selectWlevel(int d)
{
    if(TriforceCount()==0)
        return 0;
        
    word l = game->get_wlevel();
    
    do
    {
        if(d==0 && (game->lvlitems[l+1] & liTRIFORCE))
            break;
        else if(d<0)
            l = (l==0) ? 7 : l-1;
        else
            l = (l==7) ? 0 : l+1;
    }
    while(!(game->lvlitems[l+1] & liTRIFORCE));
    
    game->set_wlevel(l);
    return l;
}

// Would someone tell the Dodongos to shut their yaps?!
void kill_enemy_sfx()
{
    for(int i=0; i<guys.Count(); i++)
    {
        if(((enemy*)guys.spr(i))->bgsfx)
            stop_sfx(((enemy*)guys.spr(i))->bgsfx);
    }
    
    if(tmpscr->room==rGANON)
        stop_sfx(WAV_ROAR);
}

void LinkClass::setEntryPoints(int x2, int y2)
{
    mapscr *m = &tmpscr[0];
	m->entry_x = x2;
	m->entry_y = y2;
	//m->entry_x = entry_x;
	//m->entry_y = entry_y;
    entry_x=warpx=x2;
    entry_y=warpy=y2;
}

const char *roomtype_string[rMAX] =
{
    "(None)","Special Item","Pay for Info","Secret Money","Gamble",
    "Door Repair","Red Potion or Heart Container","Feed the Goriya","Level 9 Entrance",
    "Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
    "3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash", "More Arrows","Take One Item"
};

bool LinkClass::dowarp(int type, int index)
{
    if(index<0)
        return false;
        
    word wdmap=0;
    byte wscr=0,wtype=0,t=0;
    bool overlay=false;
    t=(currscr<128)?0:1;
    int wrindex = 0;
    //int lastent_org = lastentrance;
    //int lastdmap_org = lastentrance_dmap;
    bool wasSideview = (tmpscr[t].flags7 & fSIDEVIEW)!=0 && !ignoreSideview;
    
    // Drawing commands probably shouldn't carry over...
    script_drawing_commands.Clear();
    
    switch(type)
    {
    case 0:                                                 // tile warp
        wtype = tmpscr[t].tilewarptype[index];
        wdmap = tmpscr[t].tilewarpdmap[index];
        wscr = tmpscr[t].tilewarpscr[index];
        overlay = get_bit(&tmpscr[t].tilewarpoverlayflags,index)?1:0;
        wrindex=(tmpscr->warpreturnc>>(index*2))&3;
        break;
        
    case 1:                                                 // side warp
        wtype = tmpscr[t].sidewarptype[index];
        wdmap = tmpscr[t].sidewarpdmap[index];
        wscr = tmpscr[t].sidewarpscr[index];
        overlay = get_bit(&tmpscr[t].sidewarpoverlayflags,index)?1:0;
        wrindex=(tmpscr->warpreturnc>>(8+(index*2)))&3;
        break;
        
    case 2:                                                 // whistle warp
    {
        wtype = wtWHISTLE;
        int wind = whistleitem>-1 ? itemsbuf[whistleitem].misc2 : 8;
        int level=0;
        
        if(blowcnt==0)
            level = selectWlevel(0);
        else
        {
            for(int i=0; i<abs(blowcnt); i++)
                level = selectWlevel(blowcnt);
        }
        
        if(level > QMisc.warp[wind].size && QMisc.warp[wind].size>0)
        {
            level %= QMisc.warp[wind].size;
            game->set_wlevel(level);
        }
        
        wdmap = QMisc.warp[wind].dmap[level];
        wscr = QMisc.warp[wind].scr[level];
    }
    break;
    
    case 3:
        wtype = wtIWARP;
        wdmap = cheat_goto_dmap;
        wscr = cheat_goto_screen;
        break;
        
    case 4:
        wtype = wtIWARP;
        wdmap = currdmap;
        wscr = homescr-DMaps[currdmap].xoff;
        break;
    }
    
    bool intradmap = (wdmap == currdmap);
    rehydratelake(type!=wtSCROLL);
    
    switch(wtype)
    {
    case wtCAVE:
    {
        // cave/item room
        ALLOFF();
        homescr=currscr;
        currscr=0x80;
        
        if(DMaps[currdmap].flags&dmfCAVES)                                         // cave
        {
            music_stop();
            kill_sfx();
            
            if(tmpscr->room==rWARP)
            {
                currscr=0x81;
                specialcave = STAIRCAVE;
            }
            else specialcave = GUYCAVE;
            
            //lighting(2,dir);
            lighting(false, true);
            loadlvlpal(10);
            bool b2 = COOLSCROLL&&
                      ((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2)||
                       (combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2B)||
                       (combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2C)||
                       (combobuf[MAPCOMBO(x,y-16)].type==cCAVED)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2D));
            blackscr(30,b2?false:true);
            loadscr(0,wdmap,currscr,up,false);
            loadscr(1,wdmap,homescr,up,false);
            //preloaded freeform combos
            ffscript_engine(true);
            putscr(scrollbuf,0,0,tmpscr);
            putscrdoors(scrollbuf,0,0,tmpscr);
            dir=up;
            x=112;
            y=160;
            
            if(didpit)
            {
                didpit=false;
                x=pitx;
                y=pity;
            }
            
            reset_hookshot();
            stepforward(diagonalMovement?5:6, false);
        }
        else                                                  // item room
        {
            specialcave = ITEMCELLAR;
            map_bkgsfx(false);
            kill_enemy_sfx();
            draw_screen(tmpscr,false);
            
            //unless the room is already dark, fade to black
            if(!darkroom)
            {
                darkroom = true;
                fade(DMaps[currdmap].color,true,false);
            }
            
            blackscr(30,true);
            loadscr(0,wdmap,currscr,down,false);
            loadscr(1,wdmap,homescr,-1,false);
            if ( dontdraw < 2 ) {  dontdraw=1; }
            draw_screen(tmpscr);
            fade(11,true,true);
            darkroom = false;
            dir=down;
            x=48;
            y=0;
            
            // is this didpit check necessary?
            if(didpit)
            {
                didpit=false;
                x=pitx;
                y=pity;
            }
            
            reset_hookshot();
            lighting(false, true);
            if ( dontdraw < 2 ) { dontdraw=0; }
            stepforward(diagonalMovement?16:18, false);
        }
        
        break;
    }
    
    case wtPASS:                                            // passageway
    {
        map_bkgsfx(false);
        kill_enemy_sfx();
        ALLOFF();
        homescr=currscr;
        currscr=0x81;
        specialcave = PASSAGEWAY;
        byte warpscr2 = wscr + DMaps[wdmap].xoff;
        draw_screen(tmpscr,false);
        
        if(!darkroom)
            fade(DMaps[currdmap].color,true,false);
            
        darkroom=true;
        blackscr(30,true);
        loadscr(0,wdmap,currscr,down,false);
        loadscr(1,wdmap,homescr,-1,false);
        //preloaded freeform combos
        ffscript_engine(true);
        if ( dontdraw < 2 ) { dontdraw=1; }
        draw_screen(tmpscr);
        lighting(false, true);
        dir=down;
        x=48;
        
        if((homescr&15) > (warpscr2&15))
        {
            x=192;
        }
        
        if((homescr&15) == (warpscr2&15))
        {
            if((currscr>>4) > (warpscr2>>4))
            {
                x=192;
            }
        }
        
        // is this didpit check necessary?
        if(didpit)
        {
            didpit=false;
            x=pitx;
            y=pity;
        }
        
        setEntryPoints(x,y=0);
        reset_hookshot();
        if ( dontdraw < 2 ) { dontdraw=0; }
        stepforward(diagonalMovement?16:18, false);
        newscr_clk=frame;
        activated_timed_warp=false;
        stepoutindex=index;
        stepoutscr = warpscr2;
        stepoutdmap = wdmap;
        stepoutwr=wrindex;
    }
    break;
    
    case wtEXIT: // entrance/exit
    {
        ALLOFF();
        music_stop();
        kill_sfx();
        blackscr(30,false);
        currdmap = wdmap;
        dlevel=DMaps[currdmap].level;
        currmap=DMaps[currdmap].map;
        init_dmap();
        update_subscreens(wdmap);
        loadfullpal();
        ringcolor(false);
        loadlvlpal(DMaps[currdmap].color);
        //lastentrance_dmap = currdmap;
        homescr = currscr = wscr + DMaps[currdmap].xoff;
        loadscr(0,currdmap,currscr,-1,overlay);
        
        if(tmpscr->flags&fDARK)
        {
            if(get_bit(quest_rules,qr_FADE))
            {
                interpolatedfade();
            }
            else
            {
                loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
            }
            
            darkroom=naturaldark=true;
        }
        else
        {
            darkroom=naturaldark=false;
        }
        
        int wrx,wry;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
        {
            wrx=tmpscr->warpreturnx[0];
            wry=tmpscr->warpreturny[0];
        }
        else
        {
            wrx=tmpscr->warparrivalx;
            wry=tmpscr->warparrivaly;
        }
        
        if(((wrx>0||wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
        {
            if(dlevel)
            {
                lastentrance = currscr;
            }
            else
            {
                lastentrance = DMaps[currdmap].cont + DMaps[currdmap].xoff;
            }
            
            lastentrance_dmap = wdmap;
        }
        
        if(dlevel)
        {
            if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            {
                x=tmpscr->warpreturnx[wrindex];
                y=tmpscr->warpreturny[wrindex];
            }
            else
            {
                x=tmpscr->warparrivalx;
                y=tmpscr->warparrivaly;
            }
        }
        else
        {
            x=tmpscr->warpreturnx[wrindex];
            y=tmpscr->warpreturny[wrindex];
        }
        
        if(didpit)
        {
            didpit=false;
            x=pitx;
            y=pity;
        }
        
        dir=down;
        
        if(x==0)   dir=right;
        
        if(x==240) dir=left;
        
        if(y==0)   dir=down;
        
        if(y==160) dir=up;
        
        if(dlevel)
        {
            // reset enemy kill counts
            for(int i=0; i<128; i++)
            {
                game->guys[(currmap*MAPSCRSNORMAL)+i] = 0;
                game->maps[(currmap*MAPSCRSNORMAL)+i] &= ~mTMPNORET;
            }
        }
        
        markBmap(dir^1);
        //preloaded freeform combos
        ffscript_engine(true);
        reset_hookshot();
        
        if(isdungeon())
        {
            openscreen();
            if(get_bit(extra_rules, er_SHORTDGNWALK)==0)
                stepforward(diagonalMovement?11:12, false);
            else
                // Didn't walk as far pre-1.93, and some quests depend on that
                stepforward(8, false);
        }
        else
        {
            if(!COOLSCROLL)
                openscreen();
                
            int type1 = combobuf[MAPCOMBO(x,y-16)].type; // Old-style blue square placement
            int type2 = combobuf[MAPCOMBO(x,y)].type;
            int type3 = combobuf[MAPCOMBO(x,y+16)].type; // More old-style blue square placement
            
            if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
            {
                reset_pal_cycling();
                putscr(scrollbuf,0,0,tmpscr);
                putscrdoors(scrollbuf,0,0,tmpscr);
                walkup(COOLSCROLL);
            }
            else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
            {
                reset_pal_cycling();
                putscr(scrollbuf,0,0,tmpscr);
                putscrdoors(scrollbuf,0,0,tmpscr);
                walkdown2(COOLSCROLL);
            }
            else if(COOLSCROLL)
            {
                openscreen();
            }
        }
        
        show_subscreen_life=true;
        show_subscreen_numbers=true;
        playLevelMusic();
        currcset=DMaps[currdmap].color;
        dointro();
        setEntryPoints(x,y);
        
        for(int i=0; i<6; i++)
            visited[i]=-1;
            
        break;
    }
    
    case wtSCROLL:                                          // scrolling warp
    {
        int c = DMaps[currdmap].color;
        currmap = DMaps[wdmap].map;
	update_subscreens(wdmap);
	
	dlevel = DMaps[wdmap].level;
	    //check if Link has the map for the new location before updating the subscreen. ? -Z
	    //This works only in one direction, if Link had a map, to not having one.
	    //If Link does not have a map, and warps somewhere where he does, then the map still briefly shows. 
	update_subscreens(wdmap);
	    
	if ( has_item(itype_map, dlevel) ) 
	{
		//Blank the map during an intra-dmap scrolling warp. 
		dlevel = -1; //a hack for the minimap. This works!! -Z
	}
	    
        // fix the scrolling direction, if it was a tile or instant warp
        if(type==0 || type>=3)
        {
            sdir = dir;
        }
        
        scrollscr(sdir, wscr+DMaps[wdmap].xoff, wdmap);
	dlevel = DMaps[wdmap].level; //Fix dlevel and draw the map (end hack). -Z
	
        reset_hookshot();
        
        if(!intradmap)
        {
            currdmap = wdmap;
            dlevel = DMaps[currdmap].level;
            homescr = currscr = wscr + DMaps[wdmap].xoff;
            init_dmap();
            
            int wrx,wry;
            
            if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            {
                wrx=tmpscr->warpreturnx[0];
                wry=tmpscr->warpreturny[0];
            }
            else
            {
                wrx=tmpscr->warparrivalx;
                wry=tmpscr->warparrivaly;
            }
            
            if(((wrx>0||wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!get_bit(quest_rules,qr_NOSCROLLCONTINUE))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
            {
                if(dlevel)
                {
                    lastentrance = currscr;
                }
                else
                {
                    lastentrance = DMaps[currdmap].cont + DMaps[currdmap].xoff;
                }
                
                lastentrance_dmap = wdmap;
            }
        }
        
        if(DMaps[currdmap].color != c)
        {
            lighting(false, true);
        }
        
        playLevelMusic();
        currcset=DMaps[currdmap].color;
        dointro();
    }
    break;
    
    case wtWHISTLE:                                         // whistle warp
    {
        currmap = DMaps[wdmap].map;
        scrollscr(index, wscr+DMaps[wdmap].xoff, wdmap);
        reset_hookshot();
        currdmap=wdmap;
        dlevel=DMaps[currdmap].level;
        lighting(false, true);
        init_dmap();
        
        playLevelMusic();
        currcset=DMaps[currdmap].color;
        dointro();
        action=inwind; FFCore.setLinkAction(inwind);
        int wry;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wry=tmpscr->warpreturny[0];
        else wry=tmpscr->warparrivaly;
        
        int wrx;
        
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            wrx=tmpscr->warpreturnx[0];
        else wrx=tmpscr->warparrivalx;
        
        Lwpns.add(new weapon((fix)(index==left?240:index==right?0:wrx),(fix)(index==down?0:index==up?160:wry),
                             (fix)0,wWind,1,0,index,whistleitem,getUID()));
        whirlwind=255;
        whistleitem=-1;
    }
    break;
    
    case wtIWARP:
    case wtIWARPBLK:
    case wtIWARPOPEN:
    case wtIWARPZAP:
    case wtIWARPWAVE:                                       // insta-warps
    {
        //for determining whether to exit cave
        int type1 = combobuf[MAPCOMBO(x,y-16)].type;
        int type2 = combobuf[MAPCOMBO(x,y)].type;
        int type3 = combobuf[MAPCOMBO(x,y+16)].type;
        
        bool cavewarp = ((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED)
                         ||(type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D));
                         
        if(!(tmpscr->flags3&fIWARPFULLSCREEN))
        {
            //ALLOFF kills the action, but we want to preserve Link's action if he's swimming or diving -DD
            bool wasswimming = (action == swimming);
            byte olddiveclk = diveclk;
            ALLOFF();
            
            if(wasswimming)
            {
                action=swimming; FFCore.setLinkAction(swimming);
                diveclk = olddiveclk;
            }
            
            kill_sfx();
        }
        
        if(wtype==wtIWARPZAP)
        {
            zapout();
        }
        else if(wtype==wtIWARPWAVE)
        {
            //only draw Link if he's not in a cave -DD
            wavyout(!cavewarp);
        }
        else if(wtype!=wtIWARP)
        {
            bool b2 = COOLSCROLL&&cavewarp;
            blackscr(30,b2?false:true);
        }
        
        int c = DMaps[currdmap].color;
        currdmap = wdmap;
        dlevel = DMaps[currdmap].level;
        currmap = DMaps[currdmap].map;
        init_dmap();
        update_subscreens(wdmap);
        
        ringcolor(false);
        
        if(DMaps[currdmap].color != c)
            loadlvlpal(DMaps[currdmap].color);
            
        homescr = currscr = wscr + DMaps[currdmap].xoff;
        
        lightingInstant(); // Also sets naturaldark
        
        loadscr(0,currdmap,currscr,-1,overlay);
        
        x = tmpscr->warpreturnx[wrindex];
        y = tmpscr->warpreturny[wrindex];
        
        if(didpit)
        {
            didpit=false;
            x=pitx;
            y=pity;
        }
        
        type1 = combobuf[MAPCOMBO(x,y-16)].type;
        type2 = combobuf[MAPCOMBO(x,y)].type;
        type3 = combobuf[MAPCOMBO(x,y+16)].type;
        
        if(x==0)   dir=right;
        
        if(x==240) dir=left;
        
        if(y==0)   dir=down;
        
        if(y==160) dir=up;
        
        markBmap(dir^1);
        
        if(iswater(MAPCOMBO(x,y+8)) && _walkflag(x,y+8,0) && current_item(itype_flippers))
        {
            hopclk=0xFF;
            attackclk = charging = spins = 0;
            action=swimming; FFCore.setLinkAction(swimming);
        }
        else
	{
            action=none; FFCore.setLinkAction(none);
	}
            
        //preloaded freeform combos
        ffscript_engine(true);
        
        putscr(scrollbuf,0,0,tmpscr);
        putscrdoors(scrollbuf,0,0,tmpscr);
        
        if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
        {
            reset_pal_cycling();
            putscr(scrollbuf,0,0,tmpscr);
            putscrdoors(scrollbuf,0,0,tmpscr);
            walkup(COOLSCROLL);
        }
        else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
        {
            reset_pal_cycling();
            putscr(scrollbuf,0,0,tmpscr);
            putscrdoors(scrollbuf,0,0,tmpscr);
            walkdown2(COOLSCROLL);
        }
        else if(wtype==wtIWARPZAP)
        {
            zapin();
        }
        else if(wtype==wtIWARPWAVE)
        {
            wavyin();
        }
        else if(wtype==wtIWARPOPEN)
        {
            openscreen();
        }
        
        show_subscreen_life=true;
        show_subscreen_numbers=true;
        playLevelMusic();
        currcset=DMaps[currdmap].color;
        dointro();
        setEntryPoints(x,y);
    }
    break;
    
    
    case wtNOWARP:
    default:
        didpit=false;
        update_subscreens();
        return false;
    }
    
    // Stop Link from drowning!
    if(action==drowning)
    {
        drownclk=0;
        action=none; FFCore.setLinkAction(none);
    }
    
    // But keep him swimming if he ought to be!
    if(action!=rafting && iswater(MAPCOMBO(x,y+8)) && (_walkflag(x,y+8,0) || get_bit(quest_rules,qr_DROWN))
            && (current_item(itype_flippers)) && (action!=inwind))
    {
        hopclk=0xFF;
        action=swimming; FFCore.setLinkAction(swimming);
    }
    
    newscr_clk=frame;
    activated_timed_warp=false;
    eat_buttons();
    
    if(wtype!=wtIWARP)
        attackclk=0;
        
    didstuff=0;
    map_bkgsfx(true);
    loadside=dir^1;
    whistleclk=-1;
    
    if(z>0 && isSideview())
    {
        y-=z;
        z=0;
    }
    else if(!isSideview())
    {
        fall=0;
    }
    
    // If warping between top-down and sideview screens,
    // fix enemies that are carried over by Full Screen Warp
    const bool tmpscr_is_sideview = isSideview();
    
    if(!wasSideview && tmpscr_is_sideview)
    {
        for(int i=0; i<guys.Count(); i++)
        {
            if(guys.spr(i)->z > 0)
            {
                guys.spr(i)->y -= guys.spr(i)->z;
                guys.spr(i)->z = 0;
            }
            
            if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
                guys.spr(i)->yofs += 2;
        }
    }
    else if(wasSideview && !tmpscr_is_sideview)
    {
        for(int i=0; i<guys.Count(); i++)
        {
            if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
                guys.spr(i)->yofs -= 2;
        }
    }
    
    if((DMaps[currdmap].type&dmfCONTINUE) || (currdmap==0))
    {
        if(dlevel)
        {
            int wrx,wry;
            
            if(get_bit(quest_rules,qr_NOARRIVALPOINT))
            {
                wrx=tmpscr->warpreturnx[0];
                wry=tmpscr->warpreturny[0];
            }
            else
            {
                wrx=tmpscr->warparrivalx;
                wry=tmpscr->warparrivaly;
            }
            
            if((wtype == wtEXIT)
                    || (((wtype == wtSCROLL) && !intradmap) && ((wrx>0 || wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))))
            {
                if(!(wtype==wtSCROLL)||!(get_bit(quest_rules,qr_NOSCROLLCONTINUE)))
                {
                    game->set_continue_scrn(homescr);
                    //Z_message("continue_scrn = %02X e/e\n",game->get_continue_scrn());
                }
                else if(currdmap != game->get_continue_dmap())
                {
                    game->set_continue_scrn(DMaps[currdmap].cont + DMaps[currdmap].xoff);
                }
            }
            else
            {
                if(currdmap != game->get_continue_dmap())
                {
                    game->set_continue_scrn(DMaps[currdmap].cont + DMaps[currdmap].xoff);
                    //Z_message("continue_scrn = %02X dlevel\n",game->get_continue_scrn());
                }
            }
        }
        else
        {
            game->set_continue_scrn(DMaps[currdmap].cont + DMaps[currdmap].xoff);
            //Z_message("continue_scrn = %02X\n !dlevel\n",game->get_continue_scrn());
        }
        
        game->set_continue_dmap(currdmap);
        lastentrance_dmap = currdmap;
        lastentrance = game->get_continue_scrn();
        //Z_message("continue_map = %d\n",game->get_continue_dmap());
    }
    
    if(tmpscr->flags4&fAUTOSAVE)
    {
        save_game(true,0);
    }
    
    if(tmpscr->flags6&fCONTINUEHERE)
    {
        lastentrance_dmap = currdmap;
        lastentrance = homescr;
    }
    
    update_subscreens();
    verifyBothWeapons();
    
    if(wtype==wtCAVE)
    {
        if(DMaps[currdmap].flags&dmfGUYCAVES)
            Z_eventlog("Entered %s containing %s.\n",DMaps[currdmap].flags&dmfCAVES ? "Cave" : "Item Cellar",
                       roomtype_string[tmpscr[1].room]);
        else
            Z_eventlog("Entered %s.",DMaps[currdmap].flags&dmfCAVES ? "Cave" : "Item Cellar");
    }
    else Z_eventlog("Warped to DMap %d: %s, screen %d, via %s.\n", currdmap, DMaps[currdmap].name,currscr,
                        wtype==wtPASS ? "Passageway" :
                        wtype==wtEXIT ? "Entrance/Exit" :
                        wtype==wtSCROLL ? "Scrolling Warp" :
                        wtype==wtWHISTLE ? "Whistle Warp" :
                        "Insta-Warp");
                        
    eventlog_mapflags();
    return true;
}

void LinkClass::exitcave()
{
    stop_sfx(WAV_ER);
    currscr=homescr;
    loadscr(0,currdmap,currscr,255,false);                                   // bogus direction
    x = tmpscr->warpreturnx[0];
    y = tmpscr->warpreturny[0];
    
    if(didpit)
    {
        didpit=false;
        x=pitx;
        y=pity;
    }
    
    if(x+y == 0)
        x = y = 80;
        
    int type1 = combobuf[MAPCOMBO(x,y-16)].type;
    int type2 = combobuf[MAPCOMBO(x,y)].type;
    int type3 = combobuf[MAPCOMBO(x,y+16)].type;
    bool b = COOLSCROLL &&
             ((type1==cCAVE) || (type1>=cCAVEB && type1<=cCAVED) ||
              (type2==cCAVE) || (type2>=cCAVEB && type2<=cCAVED) ||
              (type3==cCAVE2) || (type3>=cCAVE2B && type3<=cCAVE2D) ||
              (type2==cCAVE2) || (type2>=cCAVE2B && type2<=cCAVE2D));
    ALLOFF();
    blackscr(30,b?false:true);
    ringcolor(false);
    loadlvlpal(DMaps[currdmap].color);
    lighting(false, true);
    music_stop();
    kill_sfx();
    putscr(scrollbuf,0,0,tmpscr);
    putscrdoors(scrollbuf,0,0,tmpscr);
    
    if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
    {
        walkup(COOLSCROLL);
    }
    else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
    {
        walkdown2(COOLSCROLL);
    }
    
    show_subscreen_life=true;
    show_subscreen_numbers=true;
    playLevelMusic();
    currcset=DMaps[currdmap].color;
    dointro();
    newscr_clk=frame;
    activated_timed_warp=false;
    dir=down;
    setEntryPoints(x,y);
    eat_buttons();
    didstuff=0;
    map_bkgsfx(true);
    loadside=dir^1;
}


void LinkClass::stepforward(int steps, bool adjust)
{
    int tx=x;           //temp x
    int ty=y;           //temp y
    int tstep=0;        //temp single step distance
    int s=0;            //calculated step distance for all steps
    z3step=2;
    int sh=shiftdir;
    shiftdir=-1;
    
    for(int i=steps; i>0; --i)
    {
        if(diagonalMovement)
        {
            tstep=z3step;
            z3step=(z3step%2)+1;
        }
        else
        {
            tstep=lsteps[int((dir<left)?ty:tx)&7];
            
            switch(dir)
            {
            case up:
                ty-=tstep;
                break;
                
            case down:
                ty+=tstep;
                break;
                
            case left:
                tx-=tstep;
                break;
                
            case right:
                tx+=tstep;
                break;
            }
        }
        
        s+=tstep;
    }
    
    z3step=2;
    
    while(s>=0)
    {
        if(diagonalMovement)
        {
            if((dir<left?int(x)&7:int(y)&7)&&adjust==true)
            {
                walkable=false;
                shiftdir=dir<left?(int(x)&8?left:right):(int(y)&8?down:up);
            }
            else
            {
                s-=z3step;
                walkable=true;
            }
            
            move(dir);
            shiftdir=-1;
        }
        else
        {
            if(dir<left)
            {
                s-=lsteps[int(y)&7];
            }
            else
            {
                s-=lsteps[int(x)&7];
            }
            
            move(dir);
        }
        
        if(s<0)
        {
            // Not quite sure how this is actually supposed to work.
            // There have to be two cases for each direction or Link
            // either walks too far onto the screen or may get stuck
            // going through walk-through walls.
            switch(dir)
            {
            case up:
                if(y<8) // Leaving the screen
                    y+=s;
                else // Entering the screen
                    y-=s;
                    
                break;
                
            case down:
                if(y>152)
                    y-=s;
                else
                    y+=s;
                    
                break;
                
            case left:
                if(x<8)
                    x+=s;
                else
                    x-=s;
                    
                break;
                
            case right:
                if(x>=232)
                    x-=s;
                else
                    x+=s;
                    
                break;
            }
        }
        
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit)
            return;
    }
    
    setEntryPoints(x,y);
    draw_screen(tmpscr);
    eat_buttons();
    shiftdir=sh;
}

void LinkClass::walkdown(bool opening) //entering cave
{
    if(opening)
    {
        close_black_opening(x+8, y+8+playing_field_offset, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(int(x)));
    clk=0;
    //  int cmby=(int(y)&0xF0)+16;
    // Fix Link's position to the grid
    y=int(y)&0xF0;
    action=climbcoverbottom; FFCore.setLinkAction(climbcoverbottom);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=int(x)&0xF0;
    climb_cover_y=(int(y)&0xF0)+16;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
    for(int i=0; i<64; i++)
    {
        linkstep();
        
        if(zinit.linkanimationstyle==las_zelda3 || zinit.linkanimationstyle==las_zelda3slow)
            link_count=(link_count+1)%16;
            
        if((i&3)==3)
            ++y;
            
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit)
            break;
    }
    
    action=none; FFCore.setLinkAction(none);
}

void LinkClass::walkdown2(bool opening) //exiting cave 2
{
    int type = combobuf[MAPCOMBO(x,y)].type;
    
    if((type==cCAVE2)||(type>=cCAVE2B && type<=cCAVE2D))
        y-=16;
        
    dir=down;
    // Fix Link's position to the grid
    y=int(y)&0xF0;
    z=fall=0;
    
    if(opening)
    {
        open_black_opening(x+8, y+8+playing_field_offset+16, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(int(x)));
    clk=0;
    //  int cmby=int(y)&0xF0;
    action=climbcovertop; FFCore.setLinkAction(climbcovertop);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=int(x)&0xF0;
    climb_cover_y=int(y)&0xF0;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
    for(int i=0; i<64; i++)
    {
        linkstep();
        
        if(zinit.linkanimationstyle==las_zelda3 || zinit.linkanimationstyle==las_zelda3slow)
            link_count=(link_count+1)%16;
            
        if((i&3)==3)
            ++y;
            
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit)
            break;
    }
    
    action=none; FFCore.setLinkAction(none);
}

void LinkClass::walkup(bool opening) //exiting cave
{
    int type = combobuf[MAPCOMBO(x,y)].type;
    
    if((type==cCAVE)||(type>=cCAVEB && type<=cCAVED))
        y+=16;
        
    // Fix Link's position to the grid
    y=int(y)&0xF0;
    z=fall=0;
    
    if(opening)
    {
        open_black_opening(x+8, y+8+playing_field_offset-16, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(int(x)));
    dir=down;
    clk=0;
    //  int cmby=int(y)&0xF0;
    action=climbcoverbottom; FFCore.setLinkAction(climbcoverbottom);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=int(x)&0xF0;
    climb_cover_y=int(y)&0xF0;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
    for(int i=0; i<64; i++)
    {
        linkstep();
        
        if(zinit.linkanimationstyle==las_zelda3 || zinit.linkanimationstyle==las_zelda3slow)
            link_count=(link_count+1)%16;
            
        if((i&3)==0)
            --y;
            
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit)
            break;
    }
    
    map_bkgsfx(true);
    loadside=dir^1;
    action=none; FFCore.setLinkAction(none);
}

void LinkClass::walkup2(bool opening) //entering cave2
{
    if(opening)
    {
        close_black_opening(x+8, y+8+playing_field_offset, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(int(x)));
    dir=up;
    clk=0;
    //  int cmby=int(y)&0xF0;
    action=climbcovertop; FFCore.setLinkAction(climbcovertop);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=int(x)&0xF0;
    climb_cover_y=(int(y)&0xF0)-16;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
    for(int i=0; i<64; i++)
    {
        linkstep();
        
        if(zinit.linkanimationstyle==las_zelda3 || zinit.linkanimationstyle==las_zelda3slow)
            link_count=(link_count+1)%16;
            
        if((i&3)==0)
            --y;
            
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit)
            break;
    }
    
    map_bkgsfx(true);
    loadside=dir^1;
    action=none; FFCore.setLinkAction(none);
}

void LinkClass::stepout() // Step out of item cellars and passageways
{
    int sc = specialcave; // This gets erased by ALLOFF()
    ALLOFF();
    stop_sfx(WAV_ER);
    map_bkgsfx(false);
    kill_enemy_sfx();
    draw_screen(tmpscr,false);
    fade(sc>=GUYCAVE?10:11,true,false);
    blackscr(30,true);
    ringcolor(false);
    
    if(sc==PASSAGEWAY && abs(x-warpx)>16) // How did Link leave the passageway?
    {
        currdmap=stepoutdmap;
        currmap=DMaps[currdmap].map;
        dlevel=DMaps[currdmap].level;
        
        //we might have just left a passage, so be sure to update the CSet record -DD
        currcset=DMaps[currdmap].color;
        
        init_dmap();
        homescr=stepoutscr;
    }
    
    currscr=homescr;
    loadscr(0,currdmap,currscr,255,false);                                   // bogus direction
    draw_screen(tmpscr,false);
    
    if((tmpscr->flags&fDARK) == 0)
    {
        darkroom = naturaldark = false;
        fade(DMaps[currdmap].color,true,true);
    }
    else
    {
        darkroom = naturaldark = true;
        
        if(get_bit(quest_rules,qr_FADE))
        {
            interpolatedfade();
        }
        else
        {
            loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
        }
    }
    
    x = tmpscr->warpreturnx[stepoutwr];
    y = tmpscr->warpreturny[stepoutwr];
    
    if(didpit)
    {
        didpit=false;
        x=pitx;
        y=pity;
    }
    
    if(x+y == 0)
        x = y = 80;
        
    dir=down;
    
    setEntryPoints(x,y);
    
    // Let's use the 'exit cave' animation if we entered this cellar via a cave combo.
    int type = combobuf[MAPCOMBO(tmpscr->warpreturnx[stepoutwr],tmpscr->warpreturny[stepoutwr])].type;
    
    if((type==cCAVE)||(type>=cCAVEB && type<=cCAVED))
    {
        walkup(false);
    }
    else if((type==cCAVE2)||(type>=cCAVE2B && type<=cCAVE2D))
    {
        walkdown2(false);
    }
    
    newscr_clk=frame;
    activated_timed_warp=false;
    didstuff=0;
    eat_buttons();
    markBmap(-1);
    map_bkgsfx(true);
    
    if(get_bit(quest_rules, qr_CAVEEXITNOSTOPMUSIC) == 0)
    {
        music_stop();
        playLevelMusic();
    }
    
    loadside=dir^1;
}

bool LinkClass::nextcombo_wf(int d2)
{
    if(toogam || action!=swimming || hopclk==0)
        return false;
        
    // assumes Link is about to scroll screens
    
    int ns = nextscr(d2);
    
    if(ns==0xFFFF)
        return false;
        
    // want actual screen index, not game->maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;
    
    int cx = x;
    int cy = y;
    
    switch(d2)
    {
    case up:
        cy=160;
        break;
        
    case down:
        cy=0;
        break;
        
    case left:
        cx=240;
        break;
        
    case right:
        cx=0;
        break;
    }
    
    // check lower half of combo
    cy += 8;
    
    // from MAPCOMBO()
    int cmb = (cy&0xF0)+(cx>>4);
    
    if(cmb>175)
        return true;
        
    newcombo c = combobuf[TheMaps[ns].data[cmb]];
    bool dried = iswater_type(c.type) && DRIEDLAKE;
    bool swim = iswater_type(c.type) && (current_item(itype_flippers)) && !dried;
    int b=1;
    
    if(cx&8) b<<=2;
    
    if(cy&8) b<<=1;
    
    if((c.walk&b) && !dried && !swim)
        return true;
        
    // next block (i.e. cnt==2)
    if(!(cx&8))
    {
        b<<=2;
    }
    else
    {
        c = combobuf[TheMaps[ns].data[++cmb]];
        dried = iswater_type(c.type) && DRIEDLAKE;
        swim = iswater_type(c.type) && (current_item(itype_flippers)) && !dried;
        b=1;
        
        if(cy&8)
        {
            b<<=1;
        }
    }
    
    return (c.walk&b) ? !dried && !swim : false;
}

bool LinkClass::nextcombo_solid(int d2)
{
    if(toogam || currscr>=128)
        return false;
        
    // assumes Link is about to scroll screens
    
    int ns = nextscr(d2);
    
    if(ns==0xFFFF)
        return false;
        
    // want actual screen index, not game->maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;
    
    int cx = x;
    int cy = y;
    
    switch(d2)
    {
    case up:
        cy=160;
        break;
        
    case down:
        cy=0;
        break;
        
    case left:
        cx=240;
        break;
        
    case right:
        cx=0;
        break;
    }
    
    if(d2==up) cy += 8;
    
    if(d2==left||d2==right) cy+=bigHitbox?0:8;
    
    // from MAPCOMBO()
    
    for(int i=0; i<=((bigHitbox&&!(d2==up||d2==down))?((cy&7)?2:1):((cy&7)?1:0)); cy+=8,i++)
    {
        int cmb = (cy&0xF0)+(cx>>4);
        
        if(cmb>175)
        {
            return true;
        }
        
        newcombo c = combobuf[TheMaps[ns].data[cmb]];
        bool dried = iswater_type(c.type) && DRIEDLAKE;
        bool swim = iswater_type(c.type) && (current_item(itype_flippers) || action==rafting) && !dried;
        int b=1;
        
        if(cx&8) b<<=2;
        
        if(cy&8) b<<=1;
        
        if((c.walk&b) && !dried && !swim)
        {
            return true;
        }
        
#if 0
        
        // next block (i.e. cnt==2)
        if(!(cx&8))
        {
            b<<=2;
        }
        else
        {
            c = combobuf[TheMaps[ns].data[++cmb]];
            dried = iswater_type(c.type) && DRIEDLAKE;
            swim = iswater_type(c.type) && (current_item(itype_flippers));
            b=1;
            
            if(cy&8)
            {
                b<<=1;
            }
        }
        
        if((c.walk&b) && !dried && !swim)
        {
            return true;
        }
        
        cx+=8;
        
        if(cx&7)
        {
            if(!(cx&8))
            {
                b<<=2;
            }
            else
            {
                c = combobuf[TheMaps[ns].data[++cmb]];
                dried = iswater_type(c.type) && DRIEDLAKE;
                swim = iswater_type(c.type) && (current_item(itype_flippers) || action==rafting);
                b=1;
                
                if(cy&8)
                {
                    b<<=1;
                }
            }
            
            if((c.walk&b) && !dried && !swim)
                return true;
        }
        
#endif
    }
    
    return false;
}

void LinkClass::checkscroll()
{
    //DO NOT scroll if Link is vibrating due to Farore's Wind effect -DD
    if(action == casting)
        return;
        
    if(toogam)
    {
        if(x<0 && (currscr&15)==0) x=0;
        
        if(y<0 && currscr<16) y=0;
        
        if(x>240 && (currscr&15)==15) x=240;
        
        if(y>160 && currscr>=112) y=160;
    }
    
    if(y<0)
    {
        bool doit=true;
        y=0;
        
        if(nextcombo_wf(up))
            doit=false;
            
        if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE))&&action!=inwind &&action!=scrolling && !(tmpscr->flags2&wfUP))
        {
            if(nextcombo_solid(up))
                doit=false;
        }
        
        if(doit || action==inwind)
        {
            if(currscr>=128)
            {
                if(specialcave >= GUYCAVE)
                    exitcave();
                else stepout();
            }
            else if(action==inwind)
            {
                if(DMaps[currdmap].flags&dmfWHIRLWINDRET)
                {
                    action=none; FFCore.setLinkAction(none);
                    restart_level();
                }
                else
                {
                    dowarp(2,up);
                }
            }
            else if(tmpscr->flags2&wfUP && (!(tmpscr->flags8&fMAZEvSIDEWARP) || checkmaze(tmpscr,false)))
            {
                sdir=up;
                dowarp(1,(tmpscr->sidewarpindex)&3);
            }
            else if(!edge_of_dmap(up))
            {
                scrollscr(up);
                
                if(tmpscr->flags4&fAUTOSAVE)
                {
                    save_game(true,0);
                }
                
                if(tmpscr->flags6&fCONTINUEHERE)
                {
                    lastentrance_dmap = currdmap;
                    lastentrance = homescr;
                }
            }
        }
    }
    
    if(y>160)
    {
        bool doit=true;
        y=160;
        
        if(nextcombo_wf(down))
            doit=false;
            
        if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE))&&action!=inwind &&action!=scrolling &&!(tmpscr->flags2&wfDOWN))
        {
            if(nextcombo_solid(down))
                doit=false;
        }
        
        if(doit || action==inwind)
        {
            if(currscr>=128)
            {
                if(specialcave >= GUYCAVE)
                    exitcave();
                else stepout();
            }
            else if(action==inwind)
            {
                if(DMaps[currdmap].flags&dmfWHIRLWINDRET)
                {
                    action=none; FFCore.setLinkAction(none);
                    restart_level();
                }
                else
                {
                    dowarp(2,down);
                }
            }
            else if(tmpscr->flags2&wfDOWN && (!(tmpscr->flags8&fMAZEvSIDEWARP) || checkmaze(tmpscr,false)))
            {
                sdir=down;
                dowarp(1,(tmpscr->sidewarpindex>>2)&3);
            }
            else if(!edge_of_dmap(down))
            {
                scrollscr(down);
                
                if(tmpscr->flags4&fAUTOSAVE)
                {
                    save_game(true,0);
                }
                
                if(tmpscr->flags6&fCONTINUEHERE)
                {
                    lastentrance_dmap = currdmap;
                    lastentrance = homescr;
                }
            }
        }
    }
    
    if(x<0)
    {
        bool doit=true;
        x=0;
        
        if(nextcombo_wf(left))
            doit=false;
            
        if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE))&&action!=inwind &&action!=scrolling &&!(tmpscr->flags2&wfLEFT))
        {
            if(nextcombo_solid(left))
                doit=false;
        }
        
        if(doit || action==inwind)
        {
            if(currscr>=128)
            {
                if(specialcave >= GUYCAVE)
                    exitcave();
                else stepout();
            }
            
            if(action==inwind)
            {
                if(DMaps[currdmap].flags&dmfWHIRLWINDRET)
                {
                    action=none; FFCore.setLinkAction(none);
                    restart_level();
                }
                else
                {
                    dowarp(2,left);
                }
            }
            else if(tmpscr->flags2&wfLEFT && (!(tmpscr->flags8&fMAZEvSIDEWARP) || checkmaze(tmpscr,false)))
            {
                sdir=left;
                dowarp(1,(tmpscr->sidewarpindex>>4)&3);
            }
            else if(!edge_of_dmap(left))
            {
                scrollscr(left);
                
                if(tmpscr->flags4&fAUTOSAVE)
                {
                    save_game(true,0);
                }
                
                if(tmpscr->flags6&fCONTINUEHERE)
                {
                    lastentrance_dmap = currdmap;
                    lastentrance = homescr;
                }
            }
        }
    }
    
    if(x>240)
    {
        bool doit=true;
        x=240;
        
        if(nextcombo_wf(right))
            doit=false;
            
        if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE))&&action!=inwind &&action!=scrolling &&!(tmpscr->flags2&wfRIGHT))
        {
            if(nextcombo_solid(right))
                doit=false;
        }
        
        if(doit || action==inwind)
        {
            if(currscr>=128)
            {
                if(specialcave >= GUYCAVE)
                    exitcave();
                else stepout();
            }
            
            if(action==inwind)
            {
                if(DMaps[currdmap].flags&dmfWHIRLWINDRET)
                {
                    action=none; FFCore.setLinkAction(none);
                    restart_level();
                }
                else
                {
                    dowarp(2,right);
                }
            }
            else if(tmpscr->flags2&wfRIGHT && (!(tmpscr->flags8&fMAZEvSIDEWARP) || checkmaze(tmpscr,false)))
            {
                sdir=right;
                dowarp(1,(tmpscr->sidewarpindex>>6)&3);
            }
            else if(!edge_of_dmap(right))
            {
                scrollscr(right);
                
                if(tmpscr->flags4&fAUTOSAVE)
                {
                    save_game(true,0);
                }
                
                if(tmpscr->flags6&fCONTINUEHERE)
                {
                    lastentrance_dmap = currdmap;
                    lastentrance = homescr;
                }
            }
        }
    }
}

// assumes current direction is in lastdir[3]
// compares directions with scr->path and scr->exitdir
bool LinkClass::checkmaze(mapscr *scr, bool sound)
{
    if(!(scr->flags&fMAZE))
        return true;
        
    if(lastdir[3]==scr->exitdir)
        return true;
        
    for(int i=0; i<4; i++)
        if(lastdir[i]!=scr->path[i])
            return false;
            
    if(sound)
        sfx(scr->secretsfx);
        
    return true;
}

bool LinkClass::edge_of_dmap(int side)
{
    if(checkmaze(tmpscr,false)==false)
        return false;
        
    // needs fixin'
    // should check dmap style
    switch(side)
    {
    case up:
        return currscr<16;
        
    case down:
        return currscr>=112;
        
    case left:
        if((currscr&15)==0)
            return true;
            
        if((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((currscr&15)-DMaps[currdmap].xoff)<=0);
            
        break;
        
    case right:
        if((currscr&15)==15)
            return true;
            
        if((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((currscr&15)-DMaps[currdmap].xoff)>=7);
            
        break;
    }
    
    return false;
}

int LinkClass::lookahead(int d2)                       // Helper for scrollscr that gets next combo on next screen.
{
    // Can use destscr for scrolling warps,
    // but assumes currmap is correct.
    
	int cx = vbound(x,0,240); //var = vbound(val, n1, n2), not bound(var, n1, n2) -Z
	int cy = vbound(y + 8,0,160);
	//bound(cx, 0, 240); //Fix crash during screen scroll when Link is moving too quickly through a corner - DarkDragon
	//bound(cy, 0, 168); //Fix crash during screen scroll when Link is moving too quickly through a corner - DarkDragon
	//y+8 could be 168 //Attempt to fix a frash where scrolling through the lower-left corner could crassh ZC as reported by Lut. -Z
    switch(d2)
    {
    case up:
        cy=160;
        break;
        
    case down:
        cy=0;
        break;
        
    case left:
        cx=240;
        break;
        
    case right:
        cx=0;
        break;
    }
    
    int combo = (cy&0xF0)+(cx>>4);
    
    if(combo>175)
        return 0;
        
    return tmpscr[0].data[combo];            // entire combo code
}

int LinkClass::lookaheadflag(int d2)
{
    // Helper for scrollscr that gets next combo on next screen.
    // Can use destscr for scrolling warps,
    // but assumes currmap is correct.
    
    int cx = vbound(x,0,240);
    int cy = vbound(y + 8,0,160);
	
	//bound(cx, 0, 240); //Fix crash during screen scroll when Link is moving too quickly through a corner - DarkDragon
	//bound(cy, 0, 168); //Fix crash during screen scroll when Link is moving too quickly through a corner - DarkDragon
	//y+8 could be 168  //Attempt to fix a frash where scrolling through the lower-left corner could crassh ZC as reported by Lut. -Z
	//Applying this here, too. -Z
    
    switch(d2)
    {
    case up:
        cy=160;
        break;
        
    case down:
        cy=0;
        break;
        
    case left:
        cx=240;
        break;
        
    case right:
        cx=0;
        break;
    }
    
    int combo = (cy&0xF0)+(cx>>4);
    
    if(combo>175)
        return 0;
        
    if(!tmpscr[0].sflag[combo])
    {
        return combobuf[tmpscr[0].data[combo]].flag;           // flag
    }
    
    return tmpscr[0].sflag[combo];           // flag
}

//Bit of a messy kludge to give the correct Link->X/Link->Y in the script
void LinkClass::run_scrolling_script(int scrolldir, int cx, int sx, int sy, bool end_frames)
{
    fix storex = x, storey = y;
    
    switch(scrolldir)
    {
    case up:
        if(y < 160) y = 176;
        else if(cx > 0 && !end_frames) y = sy + 156;
        else y = 160;
        
        break;
        
    case down:
        if(y > 0) y = -16;
        else if(cx > 0 && !end_frames) y = sy - 172;
        else y = 0;
        
        break;
        
    case left:
        if(x < 240) x = 256;
        else if(cx > 0) x = sx + 236;
        else x = 240;
        
        break;
        
    case right:
        if(x > 0) x = -16;
        else if(cx > 0)	x = sx - 252;
        else x = 0;
        
        break;
    }
    
    if(g_doscript)
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_GAME);
        
    x = storex, y = storey;
}

//Has solving the maze enabled a side warp?
//Only used just before scrolling screens
// Note: since scrollscr() calls this, and dowarp() calls scrollscr(),
// return true to abort the topmost scrollscr() call. -L
bool LinkClass::maze_enabled_sizewarp(int scrolldir)
{
    for(int i = 0; i < 3; i++) lastdir[i] = lastdir[i+1];
    
    lastdir[3] = tmpscr->flags&fMAZE ? scrolldir : 0xFF;
    
    if(tmpscr->flags8&fMAZEvSIDEWARP && tmpscr->flags&fMAZE && scrolldir != tmpscr->exitdir)
    {
        switch(scrolldir)
        {
        case up:
            if(tmpscr->flags2&wfUP && checkmaze(tmpscr,true))
            {
                lastdir[3] = 0xFF;
                sdir=up;
                dowarp(1,(tmpscr->sidewarpindex)&3);
                return true;
            }
            
            break;
            
        case down:
            if(tmpscr->flags2&wfDOWN && checkmaze(tmpscr,true))
            {
                lastdir[3] = 0xFF;
                sdir=down;
                dowarp(1,(tmpscr->sidewarpindex>>2)&3);
                return true;
            }
            
            break;
            
        case left:
            if(tmpscr->flags2&wfLEFT && checkmaze(tmpscr,true))
            {
                lastdir[3] = 0xFF;
                sdir=left;
                dowarp(1,(tmpscr->sidewarpindex>>4)&3);
                return true;
            }
            
            break;
            
        case right:
            if(tmpscr->flags2&wfRIGHT && checkmaze(tmpscr,true))
            {
                lastdir[3] = 0xFF;
                sdir=right;
                dowarp(1,(tmpscr->sidewarpindex)&3);
                return true;
            }
            
            break;
        }
    }
    
    return false;
}

int LinkClass::get_scroll_step(int scrolldir)
{
	// For side-scrollers, where the relative speed of 'fast' scrolling is a bit slow.
	if(get_bit(quest_rules, qr_VERYFASTSCROLLING))
		return 16;

    if(get_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING) != 0)
    {
        return (isdungeon() && !get_bit(quest_rules,qr_FASTDNGN)) ? 2 : 4;
    }
    else
    {
        if(scrolldir == up || scrolldir == down)
        {
            return 8;
        }
        else
        {
            return (isdungeon() && !get_bit(quest_rules,qr_FASTDNGN)) ? 2 : 4;
        }
    }
}

int LinkClass::get_scroll_delay(int scrolldir)
{
	if(get_bit(quest_rules, qr_NOSCROLL))
		return 0;
        
	if( (get_bit(quest_rules, qr_VERYFASTSCROLLING) != 0) ||
	    (get_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING) != 0) )
	{
		return 1;
	}
	else
	{
		if(scrolldir == up || scrolldir == down)
		{
			return (isdungeon() && !get_bit(quest_rules,qr_FASTDNGN)) ? 4 : 2;
		}
		else
		{
			return 1;
		}
	}
}

void LinkClass::scrollscr(int scrolldir, int destscr, int destdmap)
{
    if(action==freeze)
    {
        return;
    }
    
    bool overlay = false;
    
    if(scrolldir >= 0 && scrolldir <= 3)
    {
        overlay = get_bit(&tmpscr[(currscr < 128) ? 0 : 1].sidewarpoverlayflags, scrolldir) ? true : false;
    }
    
    if(destdmap == -1)
    {
        if(ZCMaps[currmap].tileWidth  != ZCMaps[DMaps[currdmap].map].tileWidth
                || ZCMaps[currmap].tileHeight != ZCMaps[DMaps[currdmap].map].tileHeight)
            return;
    }
    else
    {
        if(ZCMaps[currmap].tileWidth  != ZCMaps[DMaps[destdmap].map].tileWidth
                || ZCMaps[currmap].tileHeight != ZCMaps[DMaps[destdmap].map].tileHeight)
            return;
    }
    
    if(maze_enabled_sizewarp(scrolldir))  // dowarp() was called
        return;
        
    kill_enemy_sfx();
    stop_sfx(WAV_ER);
    screenscrolling = true;
    
    tmpscr[1] = tmpscr[0];
    
    const int _mapsSize = ZCMaps[currmap].tileWidth * ZCMaps[currmap].tileHeight;
    tmpscr[1].data.resize(_mapsSize, 0);
    tmpscr[1].sflag.resize(_mapsSize, 0);
    tmpscr[1].cset.resize(_mapsSize, 0);
    
    for(int i = 0; i < 6; i++)
    {
        tmpscr3[i] = tmpscr2[i];
        tmpscr3[1].data.resize(_mapsSize, 0);
        tmpscr3[1].sflag.resize(_mapsSize, 0);
        tmpscr3[1].cset.resize(_mapsSize, 0);
    }
    
    conveyclk = 2;
    
    mapscr *newscr = &tmpscr[0];
    mapscr *oldscr = &tmpscr[1];
    
    //scroll x, scroll y, old screen x, old screen y, new screen x, new screen y
    int sx = 0, sy = 0, tx = 0, ty = 0, tx2 = 0, ty2 = 0;
    int cx = 0;
    int step = get_scroll_step(scrolldir);
    int delay = get_scroll_delay(scrolldir);
    
    int scx = get_bit(quest_rules,qr_FASTDNGN) ? 30 : 0;
    if(get_bit(quest_rules, qr_VERYFASTSCROLLING)) //just a minor adjustment.

	  scx = 32; //for sideview very fast screolling. 
  
    actiontype lastaction = action;
    ALLOFF(false, false);
    // for now, restore Link's previous action
    action=lastaction; FFCore.setLinkAction(lastaction);
    
    lstep = (lstep + 6) % 12;
    cx = scx;
    
    if(global_wait)
    {
        ZScriptVersion::RunScript(SCRIPT_GLOBAL, GLOBAL_SCRIPT_GAME);
        global_wait=false;
    }
    
    do
    {
        draw_screen(tmpscr);
        
        if(cx == scx)
            rehydratelake(false);
            
        advanceframe(true);
        
        if(Quit)
        {
            screenscrolling = false;
            return;
        }
        
        ++cx;
    }
    while(cx < 32);
    
    script_drawing_commands.Clear();
    
    
    //clear Link's last hits 
    //for ( int q = 0; q < 4; q++ ) sethitLinkUID(q, 0);
    
    if((DMaps[currdmap].type&dmfTYPE)==dmCAVE)
        markBmap(scrolldir);
        
        
    if(fixed_door)
    {
        unsetmapflag(mSECRET);
        fixed_door = false;
    }
    
    switch(scrolldir)
    {
    case up:
    {
        if(destscr != -1)
            currscr = destscr;
        else if(checkmaze(oldscr,true) && !edge_of_dmap(scrolldir))
            currscr -= 16;
            
        loadscr(0,destdmap,currscr,scrolldir,overlay);
        blit(scrollbuf,scrollbuf,0,0,0,176,256,176);
        putscr(scrollbuf,0,0,newscr);
        putscrdoors(scrollbuf,0,0,newscr);
        sy=176;
        
        if(get_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING) == 0)
            sy+=3;
            
        cx=176/step;
    }
    break;
    
    case down:
    {
        if(destscr != -1)
            currscr = destscr;
        else if(checkmaze(oldscr,true) && !edge_of_dmap(scrolldir))
            currscr += 16;
            
        loadscr(0,destdmap,currscr,scrolldir,overlay);
        putscr(scrollbuf,0,176,newscr);
        putscrdoors(scrollbuf,0,176,newscr);
        sy = 0;
        
        if(get_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING) == 0)
            sy+=3;
            
        cx = 176 / step;
    }
    break;
    
    case left:
    {
        if(destscr!=-1)
            currscr = destscr;
        else if(checkmaze(oldscr,true) && !edge_of_dmap(scrolldir))
            --currscr;
            
        loadscr(0,destdmap,currscr,scrolldir,overlay);
        blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
        putscr(scrollbuf,0,0,newscr);
        putscrdoors(scrollbuf,0,0,newscr);
        sx = 256;
        cx = 256 / step;
    }
    break;
    
    case right:
    {
        if(destscr != -1)
            currscr = destscr;
        else if(checkmaze(oldscr,true) && !edge_of_dmap(scrolldir))
            ++currscr;
            
        loadscr(0,destdmap,currscr,scrolldir,overlay);
        putscr(scrollbuf,256,0,newscr);
        putscrdoors(scrollbuf,256,0,tmpscr);
        sx = 0;
        cx = 256 / step;
    }
    break;
    }

    // change Link's state if entering water
    int ahead = lookahead(scrolldir);
    int aheadflag = lookaheadflag(scrolldir);

    bool nowinwater = false;

    if(lastaction != inwind)
    {
        if(lastaction == rafting && isRaftFlag(aheadflag))
        {
            action=rafting; FFCore.setLinkAction(rafting);
        }
        else if(iswater(ahead) && (current_item(itype_flippers)))
        {
            if(lastaction==swimming)
            {
                action=swimming; FFCore.setLinkAction(swimming);
                hopclk = 0xFF;
                nowinwater = true;
            }
            else
            {
                action=hopping; FFCore.setLinkAction(hopping);
                hopclk = 2;
                nowinwater = true;
            }
        }
        else
        {
            action=none; FFCore.setLinkAction(none);
        }
    }
    
    // The naturaldark state can be read/set by an FFC script before
    // fade() or lighting() is called.
    naturaldark = ((TheMaps[currmap*MAPSCRS+currscr].flags & fDARK) != 0);
    
    if(newscr->oceansfx != oldscr->oceansfx)	adjust_sfx(oldscr->oceansfx, 128, false);
    
    if(newscr->bosssfx != oldscr->bosssfx)	adjust_sfx(oldscr->bosssfx, 128, false);
    
    //Preloaded ffc scripts
    if(destdmap >= 0)
    {
        long dmap = currdmap; // Kludge
        currdmap = destdmap;
        ffscript_engine(true);
        currdmap = dmap;
    }
    else
        ffscript_engine(true);
        
    // There are two occasions when scrolling must be darkened:
    // 1) When scrolling into a dark room.
    // 2) When scrolling between DMaps of different colours.
    if(destdmap != -1 && DMaps[destdmap].color != currcset)
    {
fade((specialcave > 0) ? (specialcave >= GUYCAVE) ? 10 : 11 : currcset, true, false);
        darkroom = true;
    }
    else if(!darkroom)
        lighting(false, false); // NES behaviour: fade to dark before scrolling
        
    if(action != rafting)  // Is this supposed to be here?!
    
        cx++; //This was the easiest way to re-arrange the loop so drawing is in the middle
        
    cx *= delay; //so we can have drawing re-done every frame,
    //previously it was for(0 to delay) advanceframes at end of loop
    int no_move = 0;
    bool end_frames = false;
    
    for(word i = 0; cx >= 0 && delay != 0; i++, cx--) //Go!
    {
        if(Quit)
        {
            screenscrolling = false;
            return;
        }
        
        script_drawing_commands.Clear();
        
        // For rafting (and possibly other esoteric things)
        // Link's action should remain unchanged while scrolling,
        // but for the sake of scripts, here's an eye-watering kludge.
        lastaction = action;
        action=scrolling; FFCore.setLinkAction(scrolling);
        ZScriptVersion::RunScrollingScript(scrolldir, cx, sx, sy, end_frames);
        action=lastaction; FFCore.setLinkAction(lastaction);
        
        if(no_move > 0)
            no_move--;
            
        //Don't want to move things on the first or last iteration, or between delays
        if(i == 0 || cx == 0 || cx % delay != 0)
            no_move++;
            
        if(scrolldir == up || scrolldir == down)
        {
            if(!get_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING))
            {
                //Add a few extra frames if on the second loop and cool scrolling is not set
                if(i == 1)
                {
                    cx += (scrolldir == down) ? 3 : 2;
                    no_move += (scrolldir == down) ? 3 : 2;
                }
            }
            else
            {
                //4 frames after we've finished scrolling of being still
                if(cx == 0 && !end_frames)
                {
                    cx += 4;
                    no_move += 4;
                    end_frames = true;
                }
            }
        }
        
        //Move Link and the scroll position
        if(!no_move)
        {
            switch(scrolldir)
            {
            case up:
                sy -= step;
                y += step;
                break;
                
            case down:
                sy += step;
                y -= step;
                break;
                
            case left:
                sx -= step;
                x += step;
                break;
                
            case right:
                sx += step;
                x -= step;
                break;
            }
            
            //bound Link when me move him off the screen in the last couple of frames of scrolling
            if(y > 160) y = 160;
            
            if(y < 0)   y = 0;
            
            if(x > 240) x = 240;
            
            if(x < 0)   x = 0;
            
            if(ladderx > 0 || laddery > 0)
            {
                // If the ladder moves on both axes, the player can
                // gradually shift it by going back and forth
                if(scrolldir==up || scrolldir==down)
                    laddery = int(y);
                else
                    ladderx = int(x);
            }
        }
        //FFScript.OnWaitdraw()
        if(global_wait)
        {
            // And now to injure your other eye
            lastaction=action;
            action=scrolling; FFCore.setLinkAction(scrolling);
            ZScriptVersion::RunScrollingScript(scrolldir, cx, sx, sy, end_frames);
            action=lastaction; FFCore.setLinkAction(lastaction);
            global_wait=false;
        }
        
        //Drawing
        tx = sx;
        ty = sy;
        tx2 = sx;
        ty2 = sy;
        
        switch(scrolldir)
        {
        case right:
            tx -= 256;
            break;
            
        case down:
            ty -= 176;
            break;
            
        case left:
            tx2 -= 256;
            break;
            
        case up:
            ty2 -= 176;
            break;
        }
        
        clear_bitmap(scrollbuf);
        clear_bitmap(framebuf);
        
        switch(scrolldir)
        {
        case up:
            if(newscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, newscr, 0, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, oldscr, 0, -176+playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, newscr, 0, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, oldscr, 0, -176+playing_field_offset, 3);
            
            // Draw both screens' background layer primitives together, after both layers' combos.
            // Not ideal, but probably good enough for all realistic purposes.
            if(newscr->flags7&fLAYER2BG || oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_primitives(scrollbuf, 2, newscr, sx, sy);
            
            if(newscr->flags7&fLAYER3BG || oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_primitives(scrollbuf, 3, newscr, sx, sy);
            
            putscr(scrollbuf, 0, 0, newscr);
            putscr(scrollbuf, 0, 176, oldscr);
            break;
            
        case down:
            if(newscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, newscr, 0, -176+playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, oldscr, 0, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, newscr, 0, -176+playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, oldscr, 0, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER2BG || oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_primitives(scrollbuf, 2, newscr, sx, sy);
            
            if(newscr->flags7&fLAYER3BG || oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_primitives(scrollbuf, 3, newscr, sx, sy);
            
            putscr(scrollbuf, 0, 0, oldscr);
            putscr(scrollbuf, 0, 176, newscr);
            break;
            
        case left:
            if(newscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, newscr, 0, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, oldscr, -256, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, newscr, 0, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, oldscr, -256, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER2BG || oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_primitives(scrollbuf, 2, newscr, sx, sy);
            
            if(newscr->flags7&fLAYER3BG || oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_primitives(scrollbuf, 3, newscr, sx, sy);
            
            putscr(scrollbuf, 0, 0, newscr);
            putscr(scrollbuf, 256, 0, oldscr);
            break;
            
        case right:
            if(newscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, newscr, -256, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, oldscr, 0, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, newscr, -256, playing_field_offset, 2);
            
            if(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, oldscr, 0, playing_field_offset, 3);
            
            if(newscr->flags7&fLAYER2BG || oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_primitives(scrollbuf, 2, newscr, sx, sy);
            
            if(newscr->flags7&fLAYER3BG || oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_primitives(scrollbuf, 3, newscr, sx, sy);
            
            putscr(scrollbuf, 0, 0, oldscr);
            putscr(scrollbuf, 256, 0, newscr);
            break;
        }
        
        blit(scrollbuf, framebuf, sx, sy, 0, playing_field_offset, 256, 168);
        do_primitives(framebuf, 0, newscr, 0, playing_field_offset);
        
        do_layer(framebuf, 0, oldscr, tx2, ty2, 3);
        
        if(!(oldscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG) ) do_layer(framebuf,1, oldscr, tx2, ty2, 3);
        
        do_layer(framebuf, 0, newscr, tx, ty, 2, false, true);
        
        if(!(newscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(framebuf,1, newscr, tx, ty, 2, false, !(oldscr->flags7&fLAYER2BG));
        
        do_layer(framebuf, -2, oldscr, tx2, ty2, 3); //push blocks
        do_layer(framebuf, -2, newscr, tx, ty, 2);
        
        do_walkflags(framebuf, oldscr, tx2, ty2,3); //show walkflags if the cheat is on
        do_walkflags(framebuf, newscr, tx, ty,2);
        
        if(get_bit(quest_rules, qr_FFCSCROLL))
        {
            do_layer(framebuf, -3, oldscr, tx2, ty2, 3, true); //ffcs
            do_layer(framebuf, -3, newscr, tx, ty, 2, true);
        }
        
        putscrdoors(framebuf, 0-tx2, 0-ty2+playing_field_offset, oldscr);
        putscrdoors(framebuf, 0-tx,  0-ty+playing_field_offset, newscr);
        linkstep();
        
        if(z > 0 && (!get_bit(quest_rules,qr_SHADOWSFLICKER) || frame&1))
        {
            drawshadow(framebuf, get_bit(quest_rules, qr_TRANSSHADOWS) != 0);
        }
        
        if(!isdungeon() || get_bit(quest_rules,qr_FREEFORM))
        {
            draw_under(framebuf); //draw the ladder or raft
            decorations.draw2(framebuf, true);
            draw(framebuf); //Link
            decorations.draw(framebuf,  true);
        }
        
        if(!(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG )) do_layer(framebuf,2, oldscr, tx2, ty2, 3);
        
        do_layer(framebuf, 3, oldscr, tx2, ty2, 3); //layer 3
        do_layer(framebuf,-1, oldscr, tx2, ty2, 3); //overhead combos
        do_layer(framebuf, 4, oldscr, tx2, ty2, 3); //layer 4
        do_layer(framebuf,-4, oldscr, tx2, ty2, 3, true); //overhead FFCs
        do_layer(framebuf, 5, oldscr, tx2, ty2, 3); //layer 5
        
        if(!(newscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG )) do_layer(framebuf,2, newscr, tx, ty, 2, false, !(oldscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ));
        
        do_layer(framebuf, 3, newscr, tx, ty, 2, false, true); //layer 3
        do_layer(framebuf,-1, newscr, tx, ty, 2); //overhead combos
        do_layer(framebuf, 4, newscr, tx, ty, 2, false, true); //layer 4
        do_layer(framebuf,-4, newscr, tx, ty, 2, true); //overhead FFCs
        do_layer(framebuf, 5, newscr, tx, ty, 2, false, true); //layer 5
        
        if(msgdisplaybuf->clip == 0)
            masked_blit(msgdisplaybuf, framebuf, tx2, ty2, 0, playing_field_offset, 256, 168);
            
        put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
        
        if(get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
            do_primitives(framebuf, 7, newscr, 0, playing_field_offset);
            
        //end drawing
        advanceframe(true);
    }//end main scrolling loop (2 spaces tab width makes me sad =( )
    
    
    
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    
    //Move link to the other side of the screen if scrolling's not turned on
    if(get_bit(quest_rules, qr_NOSCROLL))
    {
        switch(scrolldir)
        {
        case up:
            y = 160;
            break;
            
        case down:
            y = 0;
            break;
            
        case left:
            x = 240;
            break;
            
        case right:
            x = 0;
            break;
        }
    }
    
    if(z > 0 && isSideview())
    {
        y -= z;
        z = 0;
    }
    
    entry_x = x;
    entry_y = y;
    warpx   = -1;
    warpy   = -1;
    
    screenscrolling = false;
    
    if(destdmap != -1)
        currdmap = destdmap;
        
    //if Link is going from non-water to water, and we set his animation to "hopping" above, we must now
    //change it to swimming - since we have manually moved Link onto the first tile, the hopping code
    //will get confused and try to hop Link onto the next (possibly nonexistant) water tile in his current
    //direction. -DD
    
    if(nowinwater)
    {
        action=swimming; FFCore.setLinkAction(swimming);
        hopclk = 0xFF;
    }
    
    // NES behaviour: Fade to light after scrolling
    lighting(false, false); // No, we don't need to set naturaldark...
    
    homescr=currscr;
    init_dmap();
    putscr(scrollbuf,0,0,newscr);
    putscrdoors(scrollbuf,0,0,newscr);
    
    // Check for raft flags
    if(action!=rafting && hopclk==0 && !toogam)
    {
        if(MAPFLAG(x,y)==mfRAFT||MAPCOMBOFLAG(x,y)==mfRAFT)
        {
            sfx(tmpscr->secretsfx);
            action=rafting; FFCore.setLinkAction(rafting);
        }
        
        // Half a tile off?
        else if((dir==left || dir==right) && (MAPFLAG(x,y+8)==mfRAFT||MAPCOMBOFLAG(x,y+8)==mfRAFT))
        {
            sfx(tmpscr->secretsfx);
            action=rafting; FFCore.setLinkAction(rafting);
        }
    }
    
    opendoors=0;
    markBmap(-1);
    
    if(isdungeon())
    {
        switch(tmpscr->door[scrolldir^1])
        {
        case dOPEN:
        case dUNLOCKED:
        case dOPENBOSS:
            dir = scrolldir;
            
            if(action!=rafting)
                stepforward(diagonalMovement?11:12, false);
                
            break;
            
        case dSHUTTER:
        case d1WAYSHUTTER:
            dir = scrolldir;
            
            if(action!=rafting)
                stepforward(diagonalMovement?21:24, false);
                
            putdoor(scrollbuf,0,scrolldir^1,tmpscr->door[scrolldir^1]);
            opendoors=-4;
            sfx(WAV_DOOR);
            break;
            
        default:
            dir = scrolldir;
            
            if(action!=rafting)
                stepforward(diagonalMovement?21:24, false);
        }
    }
    
    if(action == scrolling)
    {
        action=none; FFCore.setLinkAction(none);
    }
        
    if(action != attacking)
    {
        charging = 0;
        tapping = false;
    }
    
    map_bkgsfx(true);
    
    if(newscr->flags2&fSECRET)
    {
        sfx(newscr->secretsfx);
    }
    
    playLevelMusic();
    
    newscr_clk = frame;
    activated_timed_warp=false;
    loadside = scrolldir^1;
    eventlog_mapflags();
}

// How much to reduce Link's damage, taking into account various rings.
int LinkClass::ringpower(int dmg)
{
    int result = 1;
    int itemid = current_item_id(itype_ring);
    bool usering = false;
    
    if(itemid>-1)  // current_item_id checks magic cost for rings
    {
        usering = true;
        paymagiccost(itemid);
        result *= itemsbuf[itemid].power;
    }
    
    /* Now for the Peril Ring */
    itemid = current_item_id(itype_perilring);
    
    if(itemid>-1 && game->get_life()<=itemsbuf[itemid].misc1*HP_PER_HEART && checkmagiccost(itemid))
    {
        usering = true;
        paymagiccost(itemid);
        result *= itemsbuf[itemid].power;
    }
    
    // Ring divisor of 0 = no damage. -L
    if(usering && result==0) //Change dto allow negative power rings. -Z
        return 0;
        
    if ( result < 0 ) return dmg * (result*-1);
    return dmg/( result != 0 ? result : 1 ); //zc_max(result, 1); // well, better safe...
}

// Should swinging the hammer make the 'pound' sound?
// Or is Link just hitting air?
bool LinkClass::sideviewhammerpound()
{
    int wx=0,wy=0;
    
    switch(dir)
    {
    case up:
        wx=-1;
        wy=-15;
        
        if(isSideview())  wy+=8;
        
        break;
        
    case down:
        wx=8;
        wy=28;
        
        if(isSideview())  wy-=8;
        
        break;
        
    case left:
        wx=-8;
        wy=14;
        
        if(isSideview()) wy+=8;
        
        break;
        
    case right:
        wx=21;
        wy=14;
        
        if(isSideview()) wy+=8;
        
        break;
    }
    
    if(!isSideview())
    {
        return (COMBOTYPE(x+wx,y+wy)!=cSHALLOWWATER && !iswater(MAPCOMBO(x+wx,y+wy)));
    }
    
    if(_walkflag(x+wx,y+wy,0)) return true;
    
    if(dir==left || dir==right)
    {
        wx+=16;
        
        if(_walkflag(x+wx,y+wy,0)) return true;
    }
    
    return false;
}

/************************************/
/********  More Items Code  *********/
/************************************/

// The following are only used for Link damage. Damage is in quarter hearts.
int enemy_dp(int index)
{
    return (((enemy*)guys.spr(index))->dp)*(HP_PER_HEART/4);
}

int ewpn_dp(int index)
{
    return (((weapon*)Ewpns.spr(index))->power)*(HP_PER_HEART/4);
}

int lwpn_dp(int index)
{
    return (((weapon*)Lwpns.spr(index))->power)*(HP_PER_HEART/4);
}

bool checkmagiccost(int itemid)
{
    if(itemid < 0)
    {
        return false;
    }
    /*
    else if(itemsbuf[itemid].cost_counter == 1)
    {
        return (game->get_rupies()+game->get_drupy()>=itemsbuf[itemid].magic);
    }
    
    else if(get_bit(quest_rules,qr_ENABLEMAGIC))
    {
        return (((current_item_power(itype_magicring) > 0)
                 ? game->get_maxmagic()
                 : game->get_magic()+game->get_dmagic())>=itemsbuf[itemid].magic*game->get_magicdrainrate());
    }
    
    */
    //! New handling for custom cost counters int he item editor, 25th Dec 2017. -Z
    switch (itemsbuf[itemid].cost_counter )
    {
	case 1: //rupees
		return (game->get_rupies()+game->get_drupy()>=itemsbuf[itemid].magic);
		break;
	case 4: //magic
	{
		if (get_bit(quest_rules,qr_ENABLEMAGIC))
		{
			return (((current_item_power(itype_magicring) > 0)
				 ? game->get_maxmagic()
				 : game->get_magic()+game->get_dmagic())>=itemsbuf[itemid].magic*game->get_magicdrainrate());
		}
		else 
		{
			return (game->get_rupies()+game->get_drupy()>=itemsbuf[itemid].magic);
		}
		break;
	}
	
	default:
	{ 
		//all other counters.
		//no need for the QR here, as old quests could only use specific counters.
		return (game->get_counter(itemsbuf[itemid].cost_counter)+game->get_dcounter(itemsbuf[itemid].cost_counter)>=itemsbuf[itemid].magic);
		break;
	}
	    
    }
    
    
    return 1;
}

void paymagiccost(int itemid)
{
    if(itemid < 0)
    {
        return;
    }
    else if(itemsbuf[itemid].magic <= 0)
    {
	    return;
    }    
    else if(current_item_power(itype_magicring) > 0 && ( itemsbuf[itemid].cost_counter == 4 || itemsbuf[itemid].cost_counter == 1 ))
    {
        return;
    }
        
    if(itemsbuf[itemid].cost_counter == 1) //rupees
    {
	if ( itemsbuf[itemid].magiccosttimer > 0 ) 
	{
		//get_counter
		if ( frame % itemsbuf[itemid].magiccosttimer == 0 )  game->change_drupy(-itemsbuf[itemid].magic);
		return;
	}
	else 
	{
		game->change_drupy(-itemsbuf[itemid].magic);
		return;
	}
    }
    else if (itemsbuf[itemid].cost_counter == 4) //magic
    {
	if ( itemsbuf[itemid].magiccosttimer > 0 ) 
	{
		if ( frame % itemsbuf[itemid].magiccosttimer == 0 )  game->change_magic(-(itemsbuf[itemid].magic*game->get_magicdrainrate()));
	}
	else 
	{	game->change_magic(-(itemsbuf[itemid].magic*game->get_magicdrainrate()));
		return;
	}
    }
    else //other counters
    {
	    
	if ( itemsbuf[itemid].magiccosttimer > 0 ) 
	{
		//game->set_counter
		if ( frame % itemsbuf[itemid].magiccosttimer == 0 ) game->set_counter(-(itemsbuf[itemid].magic), itemsbuf[itemid].cost_counter);
	}
	else 
	{	game->set_counter(-(itemsbuf[itemid].magic), itemsbuf[itemid].cost_counter);
		return;
	}
    }
}

int Bweapon(int pos)
{
    if(pos < 0 || current_subscreen_active == NULL)
    {
        return 0;
    }
    
    int p=-1;
    
    for(int i=0; current_subscreen_active->objects[i].type!=ssoNULL; ++i)
    {
        if(current_subscreen_active->objects[i].type==ssoCURRENTITEM && current_subscreen_active->objects[i].d3==pos)
        {
            p=i;
            break;
        }
    }
    
    if(p==-1)
    {
        return 0;
    }
    
    int actualItem = current_subscreen_active->objects[p].d8;
    //int familyCheck = actualItem ? itemsbuf[actualItem].family : current_subscreen_active->objects[p].d1
    int family = -1;
    bool bow = false;
    
    if(actualItem)
    {
        bool select = false;
        
        switch(itemsbuf[actualItem-1].family)
        {
        case itype_bomb:
            if((game->get_bombs() ||
                    // Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
                    (actualItem-1>-1 && itemsbuf[actualItem-1].misc1==0 && Lwpns.idCount(wLitBomb)>0)) ||
                    current_item_power(itype_bombbag))
            {
                select=true;
            }
            
            break;
            
        case itype_bowandarrow:
        case itype_arrow:
            if(actualItem-1>-1 && current_item_id(itype_bow)>-1)
            {
                //bow=(current_subscreen_active->objects[p].d1==itype_bowandarrow);
                select=true;
            }
            
            break;
            
        case itype_letterpotion:
            /*if(current_item_id(itype_potion)>-1)
            {
              select=true;
            }
            else if(current_item_id(itype_letter)>-1)
            {
              select=true;
            }*/
            break;
            
        case itype_sbomb:
        {
            int bombbagid = current_item_id(itype_bombbag);
            
            if((game->get_sbombs() ||
                    // Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
                    (actualItem-1>-1 && itemsbuf[actualItem-1].misc1==0 && Lwpns.idCount(wLitSBomb)>0)) ||
                    (current_item_power(itype_bombbag) && bombbagid>-1 && (itemsbuf[bombbagid].flags & ITEM_FLAG1)))
            {
                select=true;
            }
            
            break;
        }
        
        case itype_sword:
        {
            if(!get_bit(quest_rules,qr_SELECTAWPN))
                break;
                
            select=true;
        }
        break;
        
        default:
            select=true;
        }
        
        if(!item_disabled(actualItem-1) && game->get_item(actualItem-1) && select)
        {
            directItem = actualItem-1;
            
            if(directItem>-1 && itemsbuf[directItem].family == itype_arrow) bow=true;
            
            return actualItem-1+(bow?0xF000:0);
        }
        else return 0;
    }
    
    directItem = -1;
    
    switch(current_subscreen_active->objects[p].d1)
    {
    case itype_bomb:
    {
        int bombid = current_item_id(itype_bomb);
        
        if((game->get_bombs() ||
                // Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
                (bombid>-1 && itemsbuf[bombid].misc1==0 && Lwpns.idCount(wLitBomb)>0)) ||
                current_item_power(itype_bombbag))
        {
            family=itype_bomb;
        }
        
        break;
    }
    
    case itype_bowandarrow:
    case itype_arrow:
        if(current_item_id(itype_bow)>-1 && current_item_id(itype_arrow)>-1)
        {
            bow=(current_subscreen_active->objects[p].d1==itype_bowandarrow);
            family=itype_arrow;
        }
        
        break;
        
    case itype_letterpotion:
        if(current_item_id(itype_potion)>-1)
        {
            family=itype_potion;
        }
        else if(current_item_id(itype_letter)>-1)
        {
            family=itype_letter;
        }
        
        break;
        
    case itype_sbomb:
    {
        int bombbagid = current_item_id(itype_bombbag);
        int sbombid = current_item_id(itype_sbomb);
        
        if((game->get_sbombs() ||
                // Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
                (sbombid>-1 && itemsbuf[sbombid].misc1==0 && Lwpns.idCount(wLitSBomb)>0)) ||
                (current_item_power(itype_bombbag) && bombbagid>-1 && (itemsbuf[bombbagid].flags & ITEM_FLAG1)))
        {
            family=itype_sbomb;
        }
        
        break;
    }
    
    case itype_sword:
    {
        if(!get_bit(quest_rules,qr_SELECTAWPN))
            break;
            
        family=itype_sword;
    }
    break;
    
    default:
        family=current_subscreen_active->objects[p].d1;
    }
    
    if(family==-1)
        return 0;
        
    for(int j=0; j<MAXITEMS; j++)
    {
        // Find the item that matches this subscreen object.
        if(itemsbuf[j].family==family && j == current_item_id(family,false) && !item_disabled(j))
        {
            return j+(bow?0xF000:0);
        }
    }
    
    return 0;
}

void stopCaneOfByrna()
{
    for(int i=0; i<Lwpns.Count(); i++)
    {
        weapon *w = ((weapon*)Lwpns.spr(i));
        
        if(w->id==wCByrna)
            w->dead=1;
    }
}

// Used to find out if an item family is attached to one of the buttons currently pressed.
bool isWpnPressed(int itype)
{
    if((itype==getItemFamily(itemsbuf,Bwpn&0xFFF)) && DrunkcBbtn()) return true;
    
    if((itype==getItemFamily(itemsbuf,Awpn&0xFFF)) && DrunkcAbtn()) return true;
    
    return false;
}

int getWpnPressed(int itype)
{
    if((itype==getItemFamily(itemsbuf,Bwpn)) && DrunkcBbtn()) return Bwpn;

    if((itype==getItemFamily(itemsbuf,Awpn)) && DrunkcAbtn()) return Awpn;
    
    return -1;
}

void selectNextAWpn(int type)
{
    if(!get_bit(quest_rules,qr_SELECTAWPN))
        return;
        
    int ret = selectWpn_new(type, game->awpn, game->bwpn);
    Awpn = Bweapon(ret);
    directItemA = directItem;
    game->awpn = ret;
}

void selectNextBWpn(int type)
{
    int ret = selectWpn_new(type, game->bwpn, game->awpn);
    Bwpn = Bweapon(ret);
    directItemB = directItem;
    game->bwpn = ret;
}

void verifyAWpn()
{
    if(!get_bit(quest_rules,qr_SELECTAWPN))
    {
        Awpn = selectSword();
        game->awpn = 0xFF;
    }
    else
    {
        game->awpn = selectWpn_new(SEL_VERIFY_RIGHT, game->awpn, game->bwpn);
        Awpn = Bweapon(game->awpn);
        directItemA = directItem;
    }
}

void verifyBWpn()
{
    game->bwpn = selectWpn_new(SEL_VERIFY_RIGHT, game->bwpn, game->awpn);
    Bwpn = Bweapon(game->bwpn);
    directItemB = directItem;
}

void verifyBothWeapons()
{
    verifyAWpn();
    verifyBWpn();
}

int selectWpn_new(int type, int startpos, int forbiddenpos)
{
    //what will be returned when all else fails.
    //don't return the forbiddenpos... no matter what -DD
    
    int failpos(0);
    
    if(startpos == forbiddenpos)
        failpos = 0xFF;
    else failpos = startpos;
    
    // verify startpos
    if(startpos < 0 || startpos >= 0xFF)
        startpos = 0;
        
    if(current_subscreen_active == NULL)
        return failpos;
        
    if(type==SEL_VERIFY_RIGHT || type==SEL_VERIFY_LEFT)
    {
        int wpn = Bweapon(startpos);
        
        if(wpn != 0 && startpos != forbiddenpos)
        {
            return startpos;
        }
    }
    
    int p=-1;
    int curpos = startpos;
    int firstValidPos=-1;
    
    for(int i=0; current_subscreen_active->objects[i].type!=ssoNULL; ++i)
    {
        if(current_subscreen_active->objects[i].type==ssoCURRENTITEM)
        {
            if(firstValidPos==-1 && current_subscreen_active->objects[i].d3>=0)
            {
                firstValidPos=i;
            }
            
            if(current_subscreen_active->objects[i].d3==curpos)
            {
                p=i;
                break;
            }
        }
    }
    
    if(p == -1)
    {
        //can't find the current position
        // Switch to a valid weapon if there is one; otherwise,
        // the selector can simply disappear
        if(firstValidPos>=0)
        {
            return current_subscreen_active->objects[firstValidPos].d3;
        }
        //FAILURE
        else
        {
            return failpos;
        }
    }
    
    //remember we've been here
    set<int> oldPositions;
    oldPositions.insert(curpos);
    
    //1. Perform any shifts required by the above
    //2. If that's not possible, go to position 1 and reset the b weapon.
    //2a.  -if we arrive at a position we've already visited, give up and stay there
    //3. Get the weapon at the new slot
    //4. If it's not possible, go to step 1.
    
    for(;;)
    {
        //shift
        switch(type)
        {
        case SEL_LEFT:
        case SEL_VERIFY_LEFT:
            curpos = current_subscreen_active->objects[p].d6;
            break;
            
        case SEL_RIGHT:
        case SEL_VERIFY_RIGHT:
            curpos = current_subscreen_active->objects[p].d7;
            break;
            
        case SEL_DOWN:
            curpos = current_subscreen_active->objects[p].d5;
            break;
            
        case SEL_UP:
            curpos = current_subscreen_active->objects[p].d4;
            break;
        }
        
        //find our new position
        p = -1;
        
        for(int i=0; current_subscreen_active->objects[i].type!=ssoNULL; ++i)
        {
            if(current_subscreen_active->objects[i].type==ssoCURRENTITEM)
            {
                if(current_subscreen_active->objects[i].d3==curpos)
                {
                    p=i;
                    break;
                }
            }
        }
        
        if(p == -1)
        {
            //can't find the current position
            //FAILURE
            return failpos;
        }
        
        //if we've already been here, give up
        if(oldPositions.find(curpos) != oldPositions.end())
        {
            return failpos;
        }
        
        //else, remember we've been here
        oldPositions.insert(curpos);
        
        //see if this weapon is acceptable
        if(Bweapon(curpos) != 0 && curpos != forbiddenpos)
            return curpos;
            
        //keep going otherwise
    }
}

// Select the sword for the A button if the 'select A button weapon' quest rule isn't set.
int selectSword()
{
    int ret = current_item_id(itype_sword);
    
    if(ret == -1)
        ret = 0;
        
    return ret;
}

// Used for the 'Pickup Hearts' item pickup condition.
bool canget(int id)
{
    return id>=0 && (game->get_maxlife()>=(itemsbuf[id].pickup_hearts*HP_PER_HEART));
}

void dospecialmoney(int index)
{
    int tmp=currscr>=128?1:0;
    int priceindex = ((item*)items.spr(index))->PriceIndex;
    
    switch(tmpscr[tmp].room)
    {
    case rINFO:                                             // pay for info
        if(prices[priceindex]!=100000 ) // 100000 is a placeholder price for free items
        {
            if (game->get_spendable_rupies() < abs(prices[priceindex]) && !current_item_power(itype_wallet))
                return;
                
            if(!current_item_power(itype_wallet))
	    {
                game->change_drupy(-abs(prices[priceindex]));
	    }
	    if ( current_item_power(itype_wallet)>0 )
	    {
		 game->change_drupy(0);   
	    }
        }
        rectfill(msgdisplaybuf, 0, 0, msgdisplaybuf->w, 80, 0);
        donewmsg(QMisc.info[tmpscr[tmp].catchall].str[priceindex]);
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        items.del(0);
        
        for(int i=0; i<items.Count(); i++)
            ((item*)items.spr(i))->pickup=ipDUMMY;
            
        // Prevent the prices from being displayed anymore
        for(int i=0; i<3; i++)
        {
            prices[i] = 0;
        }
        
        break;
        
    case rMONEY:                                            // secret money
    {
        ((item*)items.spr(0))->pickup = ipDUMMY;

        prices[0] = tmpscr[tmp].catchall;
        if (!current_item_power(itype_wallet))
            game->change_drupy(prices[0]);

        putprices(false);
        setmapflag();
        break;
    }
        
    case rGAMBLE:                                           // gamble
    {
        if(game->get_spendable_rupies()<10 && !current_item_power(itype_wallet)) return;
        
        unsigned si=(rand()%24)*3;
        
        for(int i=0; i<3; i++)
            prices[i]=gambledat[si++];
            
        game->change_drupy(prices[priceindex]);
        putprices(true);
        
        for(int i=1; i<4; i++)
            ((item*)items.spr(i))->pickup=ipDUMMY;
    }
    break;
    
    case rBOMBS:
        if(game->get_spendable_rupies()<abs(tmpscr[tmp].catchall) && !current_item_power(itype_wallet))
            return;
            
        game->change_drupy(-abs(tmpscr[tmp].catchall));
        setmapflag();
        game->change_maxbombs(4);
        game->set_bombs(game->get_maxbombs());
        {
            int div = zinit.bomb_ratio;
            
            if(div > 0)
                game->change_maxcounter(4/div, 6);
        }
        
        //also give Link an actual Bomb item
        for(int i=0; i<MAXITEMS; i++)
        {
            if(itemsbuf[i].family == itype_bomb && itemsbuf[i].fam_type == 1)
                getitem(i, true);
        }
        
        ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        //    putscr(scrollbuf,0,0,tmpscr);
        verifyBothWeapons();
        break;
        
    case rARROWS:
        if(game->get_spendable_rupies()<abs(tmpscr[tmp].catchall) && !current_item_power(itype_wallet))
            return;
            
        game->change_drupy(-abs(tmpscr[tmp].catchall));
        setmapflag();
        game->change_maxarrows(10);
        game->set_arrows(game->get_maxarrows());
        ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        //    putscr(scrollbuf,0,0,tmpscr);
        verifyBothWeapons();
        break;
        
    case rSWINDLE:
        if(items.spr(index)->id==iRupy)
        {
            if(game->get_spendable_rupies()<abs(tmpscr[tmp].catchall) && !current_item_power(itype_wallet))
                return;
                
            game->change_drupy(-abs(tmpscr[tmp].catchall));
        }
        else
        {
            if(game->get_maxlife()<=HP_PER_HEART)
                return;
                
            game->set_life(zc_max(game->get_life()-HP_PER_HEART,0));
            game->set_maxlife(zc_max(game->get_maxlife()-HP_PER_HEART,(HP_PER_HEART)));
        }
        
        setmapflag();
        ((item*)items.spr(0))->pickup=ipDUMMY+ipFADE;
        ((item*)items.spr(1))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        //    putscr(scrollbuf,0,0,tmpscr);
        break;
    }
}

void getitem(int id, bool nosound)
{
    if(id<0)
    {
        return;
    }
    
    if(itemsbuf[id].family!=0xFF)
    {
        if(itemsbuf[id].flags & ITEM_GAMEDATA && itemsbuf[id].family != itype_triforcepiece)
        {
            // Fix boomerang sounds.
            int itemid = current_item_id(itemsbuf[id].family);
            
            if(itemid>=0 && (itemsbuf[id].family == itype_brang || itemsbuf[id].family == itype_nayruslove
                             || itemsbuf[id].family == itype_hookshot || itemsbuf[id].family == itype_cbyrna)
                    && sfx_allocated(itemsbuf[itemid].usesound)
                    && itemsbuf[id].usesound != itemsbuf[itemid].usesound)
            {
                stop_sfx(itemsbuf[itemid].usesound);
                cont_sfx(itemsbuf[id].usesound);
            }
            
            game->set_item(id,true);
            
            if(!(itemsbuf[id].flags & ITEM_KEEPOLD))
            {
                if(current_item(itemsbuf[id].family)<itemsbuf[id].fam_type)
                {
                    removeLowerLevelItemsOfFamily(game,itemsbuf,itemsbuf[id].family, itemsbuf[id].fam_type);
                }
            }
            
            // NES consistency: replace all flying boomerangs with the current boomerang.
            if(itemsbuf[id].family==itype_brang)
                for(int i=0; i<Lwpns.Count(); i++)
                {
                    weapon *w = ((weapon*)Lwpns.spr(i));
                    
                    if(w->id==wBrang)
                    {
                        w->LOADGFX(itemsbuf[id].wpn);
                    }
                }
        }
        
        if(itemsbuf[id].count!=-1)
        {
            if(itemsbuf[id].setmax)
            {
                // Bomb bags are a special case; they may be set not to increase super bombs
                if(itemsbuf[id].family==itype_bombbag && itemsbuf[id].count==2 && (itemsbuf[id].flags&16)==0)
                {
                    int max = game->get_maxbombs();
                    
                    if(max<itemsbuf[id].max) max=itemsbuf[id].max;
                    
                    game->set_maxbombs(zc_min(game->get_maxbombs()+itemsbuf[id].setmax,max), false);
                }
                else
                {
                    int max = game->get_maxcounter(itemsbuf[id].count);
                    
                    if(max<itemsbuf[id].max) max=itemsbuf[id].max;
                    
                    game->set_maxcounter(zc_min(game->get_maxcounter(itemsbuf[id].count)+itemsbuf[id].setmax,max), itemsbuf[id].count);
                }
            }
            
            // Amount is an unsigned short, but the range is -9999 to 16383
            // -1 is actually 16385 ... -9999 is 26383, and 0x8000 means use the drain counter
            if(itemsbuf[id].amount&0x3FFF)
            {
                if(itemsbuf[id].amount&0x8000)
                    game->set_dcounter(
                        game->get_dcounter(itemsbuf[id].count)+((itemsbuf[id].amount&0x4000)?-(itemsbuf[id].amount&0x3FFF):itemsbuf[id].amount&0x3FFF), itemsbuf[id].count);
                else
                {
                    if(itemsbuf[id].amount>=16385 && game->get_counter(0)<=itemsbuf[id].amount-16384)
                        game->set_counter(0, itemsbuf[id].count);
                    else
                        // This is too confusing to try and change...
                        game->set_counter(zc_min(game->get_counter(itemsbuf[id].count)+((itemsbuf[id].amount&0x4000)?-(itemsbuf[id].amount&0x3FFF):itemsbuf[id].amount&0x3FFF),game->get_maxcounter(itemsbuf[id].count)), itemsbuf[id].count);
                }
            }
        }
    }
    
    if(itemsbuf[id].playsound&&!nosound)
    {
        sfx(itemsbuf[id].playsound);
    }
    
    //add lower-level items
    if(itemsbuf[id].flags&ITEM_GAINOLD)
    {
        for(int i=itemsbuf[id].fam_type-1; i>0; i--)
        {
            int potid = getItemID(itemsbuf, itemsbuf[id].family, i);
            
            if(potid != -1)
            {
                game->set_item(potid, true);
            }
        }
    }
    
    switch(itemsbuf[id&0xFF].family)
    {
    case itype_clock:
    {
        setClock(watch=true);
        
        for(int i=0; i<eMAXGUYS; i++)
            clock_zoras[i]=0;
            
        clockclk=itemsbuf[id&0xFF].misc1;
    }
    break;
    
    case itype_lkey:
        if(game->lvlkeys[dlevel]<255) game->lvlkeys[dlevel]++;
        
        break;
        
    case itype_ring:
    case itype_magicring:
        if((get_bit(quest_rules,qr_OVERWORLDTUNIC) != 0) || (currscr<128 || dlevel))
        {
            ringcolor(false);
        }
        
        break;
        
    case itype_whispring:
    {
        if(itemsbuf[id].flags & ITEM_FLAG1)
        {
            if(LinkSwordClk()==-1) setSwordClk(150);  // Let's not bother applying the divisor.
            
            if(LinkItemClk()==-1) setItemClk(150);  // Let's not bother applying the divisor.
        }
        
        if(itemsbuf[id].power==0)
        {
            setSwordClk(0);
            setItemClk(0);
        }
        
        break;
    }
    
    
    case itype_map:
        game->lvlitems[dlevel]|=liMAP;
        break;
        
    case itype_compass:
        game->lvlitems[dlevel]|=liCOMPASS;
        break;
        
    case itype_bosskey:
        game->lvlitems[dlevel]|=liBOSSKEY;
        break;
        
    case itype_fairy:
    
        game->set_life(zc_min(game->get_life()+(itemsbuf[id].flags&ITEM_FLAG1 ?(int)(game->get_maxlife()*(itemsbuf[id].misc1/100.0)):((itemsbuf[id].misc1*HP_PER_HEART))),game->get_maxlife()));
        game->set_magic(zc_min(game->get_magic()+(itemsbuf[id].flags&ITEM_FLAG2 ?(int)(game->get_maxmagic()*(itemsbuf[id].misc2/100.0)):((itemsbuf[id].misc2*MAGICPERBLOCK))),game->get_maxmagic()));
        break;
        
    case itype_heartpiece:
        game->change_HCpieces(1);
        
        if(game->get_HCpieces()<game->get_hcp_per_hc())
            break;
            
        game->set_HCpieces(0);
        
        for(int i=0; i<MAXITEMS; i++)
        {
            if(itemsbuf[i].family == itype_heartcontainer)
            {
                getitem(i);
                break;
            }
        }
        
        break;
        
    case itype_killem:
        kill_em_all();
        break;
    }
    
    update_subscreens();
    load_Sitems(&QMisc);
    verifyBothWeapons();
}

void takeitem(int id)
{
    game->set_item(id, false);
    
    /* Lower the counters! */
    if(itemsbuf[id].count!=-1)
    {
        if(itemsbuf[id].setmax)
        {
            game->set_maxcounter(game->get_maxcounter(itemsbuf[id].count)-itemsbuf[id].setmax, itemsbuf[id].count);
        }
        
        if(itemsbuf[id].amount&0x3FFF)
        {
            if(itemsbuf[id].amount&0x8000)
                game->set_dcounter(game->get_dcounter(itemsbuf[id].count)-((itemsbuf[id].amount&0x4000)?-(itemsbuf[id].amount&0x3FFF):itemsbuf[id].amount&0x3FFF), itemsbuf[id].count);
            else game->set_counter(game->get_counter(itemsbuf[id].count)-((itemsbuf[id].amount&0x4000)?-(itemsbuf[id].amount&0x3FFF):itemsbuf[id].amount&0x3FFF), itemsbuf[id].count);
        }
    }
    
    switch(itemsbuf[id&0xFF].family)
    {
        // NES consistency: replace all flying boomerangs with the current boomerang.
    case itype_brang:
        if(current_item(itype_brang)) for(int i=0; i<Lwpns.Count(); i++)
            {
                weapon *w = ((weapon*)Lwpns.spr(i));
                
                if(w->id==wBrang)
                {
                    w->LOADGFX(itemsbuf[current_item_id(itype_brang)].wpn);
                }
            }
            
        break;
        
    case itype_heartpiece:
        if(game->get_maxlife()>HP_PER_HEART)
        {
            if(game->get_HCpieces()==0)
            {
                game->set_HCpieces(game->get_hcp_per_hc());
                takeitem(iHeartC);
            }
            
            game->change_HCpieces(-1);
        }
        
        break;
        
    case itype_map:
        game->lvlitems[dlevel]&=~liMAP;
        break;
        
    case itype_compass:
        game->lvlitems[dlevel]&=~liCOMPASS;
        break;
        
    case itype_bosskey:
        game->lvlitems[dlevel]&=~liBOSSKEY;
        break;
        
    case itype_lkey:
        if(game->lvlkeys[dlevel]) game->lvlkeys[dlevel]--;
        
        break;
    }
}

// Attempt to pick up an item. (-1 = check items touching Link.)
void LinkClass::checkitems(int index)
{
    int tmp=currscr>=128?1:0;
    
    if(index==-1)
    {
        if(diagonalMovement)
        {
            index=items.hit(x,y+(bigHitbox?0:8),z,6,6,1);
        }
        else index=items.hit(x,y+(bigHitbox?0:8),z,1,1,1);
    }
    
    if(index==-1)
        return;
        
    // if (tmpscr[tmp].room==rSHOP && boughtsomething==true)
    //   return;
    
    int pickup = ((item*)items.spr(index))->pickup;
    int PriceIndex = ((item*)items.spr(index))->PriceIndex;
    int id2 = ((item*)items.spr(index))->id;
    int pstr = ((item*)items.spr(index))->pstring;
    int pstr_flags = ((item*)items.spr(index))->pickup_string_flags;
    int tempnextmsg;
    
    if((pickup&ipTIMER) && (((item*)items.spr(index))->clk2 < 32))
        if((items.spr(index)->id!=iFairyMoving)&&(items.spr(index)->id!=iFairyMoving))
            // wait for it to stop flashing, doesn't check for other items yet
            return;
            
    if(pickup&ipENEMY)                                        // item was being carried by enemy
        if(more_carried_items()<=1)  // 1 includes this own item.
            hasitem &= ~2;
            
    if(pickup&ipDUMMY)                                        // dummy item (usually a rupee)
    {
        if(pickup&ipMONEY)
            dospecialmoney(index);
            
        return;
    }
    
    if(get_bit(quest_rules,qr_HEARTSREQUIREDFIX) && !canget(id2))
        return;
        
    if((itemsbuf[id2].flags & ITEM_COMBINE) && current_item(itemsbuf[id2].family)==itemsbuf[id2].fam_type)
        // Item upgrade routine.
    {
        int nextitem = -1;
        
        for(int i=0; i<MAXITEMS; i++)
        {
            // Find the item which is as close to this item's fam_type as possible.
            if(itemsbuf[i].family==itemsbuf[id2].family && itemsbuf[i].fam_type>itemsbuf[id2].fam_type
                    && (nextitem>-1 ? itemsbuf[i].fam_type<=itemsbuf[nextitem].fam_type : true))
            {
                nextitem = i;
            }
        }
        
        if(nextitem>-1)
            id2 = nextitem;
    }
    
    if(pickup&ipCHECK)                                        // check restrictions
        switch(tmpscr[tmp].room)
        {
        case rSP_ITEM:                                        // special item
            if(!canget(id2)) // These ones always need the Hearts Required check
                return;
                
            break;
            
        case rP_SHOP:                                         // potion shop
            if(msg_active)
                return;
                
        case rSHOP:                                           // shop
            if(prices[PriceIndex]!=100000) // 100000 is a placeholder price for free items
            {
                if(game->get_spendable_rupies()<abs(prices[PriceIndex]) && !current_item_power(itype_wallet))
                    return;
                
                if(!current_item_power(itype_wallet))
		{
                    game->change_drupy(-abs(prices[PriceIndex]));
		}
		 if(current_item_power(itype_wallet))
		{
                    game->change_drupy(0);
		}
            }
            boughtsomething=true;
            //make the other shop items untouchable after
            //you buy something
            int count = 0;
	    
	    
	    //Show a message string, if set.
	    if ( QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] > 0 && QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] < msg_count )
	    {
		    donewmsg(QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex]);
	    }
	    
            
            for(int i=0; i<3; i++)
            {
                if(QMisc.shop[tmpscr[tmp].catchall].hasitem[i] != 0)
                {
                    ++count;
                }
            }
            
            for(int i=0; i<items.Count(); i++)
            {
                if(((item*)items.spr(i))->PriceIndex >-1 && i!=index)
                    ((item*)items.spr(i))->pickup=ipDUMMY+ipFADE;
            }
            
            break;
        }
        
    if(pickup&ipONETIME)    // set mITEM for one-time-only items
    {
		setmapflag(mITEM);

		// WARNING - Item pickups are very volatile due to crazy compatability hacks, eg., supporting
		// broken behavior from early ZC versions. If you change things here please comment on it's purpose.

		// some old quests need picking up a screen item to also disable the BELOW flag (for hunger rooms, etc)
		// What is etc?! We need to check for every valid state here. ~Gleeok
		if(get_bit(quest_rules, qr_ITEMPICKUPSETSBELOW))
		{
			// Most older quests need one-time-pickups to not remove special items, etc.
			if(tmpscr->room==rGRUMBLE)
			{
				setmapflag(mBELOW);
			}
		}
    }
    else if(pickup&ipONETIME2)                                // set mBELOW flag for other one-time-only items
        setmapflag();
        
    if(itemsbuf[id2].collect_script)
    {
        ZScriptVersion::RunScript(SCRIPT_ITEM, itemsbuf[id2].collect_script, id2 & 0xFFF);
    }
    
    getitem(id2);
    
    if(pickup&ipHOLDUP)
    {
	    
	
	    
        attackclk=0;
        reset_swordcharge();
        
        if(action!=swimming)
            reset_hookshot();
            
        if(msg_onscreen)
        {
            dismissmsg();
        }
        
        clear_bitmap(pricesdisplaybuf);
        
        if(get_bit(quest_rules, qr_OLDPICKUP) || ((tmpscr[tmp].room==rSP_ITEM || tmpscr[tmp].room==rRP_HC || tmpscr[tmp].room==rTAKEONE) && (pickup&ipONETIME2)))
        {
            fadeclk=66;
        }
        
        if(id2!=iBombs || action==swimming || get_bit(quest_rules,qr_BOMBHOLDFIX))
        {
            // don't hold up bombs unless swimming or the bomb hold fix quest rule is on
            if(action==swimming)
            {
                action=waterhold1; FFCore.setLinkAction(waterhold1);
            }
            else
            {
                action=landhold1; FFCore.setLinkAction(landhold1);
            }
            
            if(((item*)items.spr(index))->twohand)
            {
                if(action==waterhold1)
                {
                    action=waterhold2; FFCore.setLinkAction(waterhold2);
                }
                else
                {
                    action=landhold2; FFCore.setLinkAction(landhold2);
                }
            }
            
            holdclk=130;
            
            //restart music
            if(get_bit(quest_rules, qr_HOLDNOSTOPMUSIC) == 0)
                music_stop();
                
            holditem=((item*)items.spr(index))->id; // NES consistency: when combining blue potions, hold up the blue potion.
            freeze_guys=true;
	    //show the info string
	     
	    
	    //if (pstr > 0 ) //&& itemsbuf[index].pstring < msg_count && ( ( itemsbuf[index].pickup_string_flags&itemdataPSTRING_ALWAYS || itemsbuf[index].pickup_string_flags&itemdataPSTRING_IP_HOLDUP ) ) )
	     if ( pstr > 0 && pstr < msg_count )
	    {
		switch(tmpscr[tmp].room)
		{
			case rSHOP: 
			{
				if ( PriceIndex >= 0 )
				{
					//if it is a shop item, and not another item type.
					int shop_pstr = ( QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] > 0 ) ? QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] : pstr;
					tempnextmsg = donew_shop_msg(pstr, shop_pstr);
				}
				else 
				{
					if ( ( ( pstr_flags&itemdataPSTRING_ALWAYS || pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_IP_HOLDUP || (!(FFCore.GetItemMessagePlayed(id2)))  ) ) )
					{	
						donewmsg(pstr); //In case of placed items and drop items, that spawn in shop rooms.
						if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
					}
				}
				break;
			}
			default: 
			{
				if ( ( ( pstr_flags&itemdataPSTRING_ALWAYS || pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_IP_HOLDUP || (!(FFCore.GetItemMessagePlayed(id2)))  ) ) )
				{	
					donewmsg(pstr); //In case of placed items and drop items, that spawn in shop rooms.
					if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
				}
				break;
			}
		}																		
		//donewmsg(pstr);
	    }
	    
        }
        
        if(itemsbuf[id2].family!=itype_triforcepiece || !(itemsbuf[id2].flags & ITEM_GAMEDATA))
        {
            sfx(tmpscr[0].holdupsfx);
        }
        
        items.del(index);
        
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = (weapon*)Lwpns.spr(i);
            
            if(w->dragging==index)
            {
                w->dragging=-1;
            }
            else if(w->dragging>index)
            {
                w->dragging-=1;
            }
        }
        
        // clear up shop stuff
        if((isdungeon()==0)&&(index!=0))
        {
            if(boughtsomething)
            {
                fadeclk=66;
                
                if(((item*)items.spr(0))->id == iRupy && ((item*)items.spr(0))->pickup & ipDUMMY)
                    items.del(0);
                    
                for(int i=0; i<Lwpns.Count(); i++)
                {
                    weapon *w = (weapon*)Lwpns.spr(i);
                    
                    if(w->dragging==0)
                    {
                        w->dragging=-1;
                    }
                    else if(w->dragging>0)
                    {
                        w->dragging-=1;
                    }
                }
            }
            
            if(msg_onscreen)
            {
                dismissmsg();
            }
            
            clear_bitmap(pricesdisplaybuf);
            set_clip_state(pricesdisplaybuf, 1);
        }
        
        //   items.del(index);
    }
    else
    {
        items.del(index);
        
        for(int i=0; i<Lwpns.Count(); i++)
        {
            weapon *w = (weapon*)Lwpns.spr(i);
            
            if(w->dragging==index)
            {
                w->dragging=-1;
            }
            else if(w->dragging>index)
            {
                w->dragging-=1;
            }
        }
        
        if(msg_onscreen)
        {
            dismissmsg();
        }
	
	//general item pickup message
        //show the info string
	//non-held
	//if ( pstr > 0 ) //&& itemsbuf[index].pstring < msg_count && ( ( itemsbuf[index].pickup_string_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(index))) ) ) )
	if ( pstr > 0 && pstr < msg_count )
	{
		switch(tmpscr[tmp].room)
		{
			case rSHOP: 
			{
				if ( PriceIndex >= 0 )
				{
					//if it is a shop item, and not another item type.
					//if there is no shop string, use the item string
					int shop_pstr = ( QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] > 0 ) ? QMisc.shop[tmpscr[tmp].catchall].str[PriceIndex] : pstr;
					tempnextmsg = donew_shop_msg(pstr, shop_pstr);
				}
				else 
				{
					if ( (!(pstr_flags&itemdataPSTRING_IP_HOLDUP)) && ( pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(id2))) ) )
					{
						donewmsg(pstr); //In case of placed items and drop items, that spawn in shop rooms.
						if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
					}
				}
				break;
			}
			default: 
			{
				if ( (!(pstr_flags&itemdataPSTRING_IP_HOLDUP)) && ( pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(id2))) ) )
				{
					donewmsg(pstr); //In case of placed items and drop items, that spawn in shop rooms.
					if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
				}
				break;
			}
		}		
		//donewmsg(pstr);
	}
	
	
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
    }
    
    if(itemsbuf[id2].family==itype_triforcepiece)
    {
        if(itemsbuf[id2].misc2==1)
            getTriforce(id2);
        else
            getBigTri(id2);
    }
}

void LinkClass::StartRefill(int refillWhat)
{
    if(!refilling)
    {
        refillclk=21;
        stop_sfx(WAV_ER);
        sfx(WAV_REFILL,128,true);
        refilling=refillWhat;
        
        if(refill_why>=0) // Item index
        {
            if((itemsbuf[refill_why].family==itype_potion)&&(!get_bit(quest_rules,qr_NONBUBBLEMEDICINE)))
            {
                swordclk=0;
                
                if(get_bit(quest_rules,qr_ITEMBUBBLE)) itemclk=0;
            }
            
            if((itemsbuf[refill_why].family==itype_triforcepiece)&&(!get_bit(quest_rules,qr_NONBUBBLETRIFORCE)))
            {
                swordclk=0;
                
                if(get_bit(quest_rules,qr_ITEMBUBBLE)) itemclk=0;
            }
        }
        else if((refill_why==REFILL_FAIRY)&&(!get_bit(quest_rules,qr_NONBUBBLEFAIRIES)))
        {
            swordclk=0;
            
            if(get_bit(quest_rules,qr_ITEMBUBBLE)) itemclk=0;
        }
    }
}

bool LinkClass::refill()
{
    if(refilling==REFILL_NONE || refilling==REFILL_FAIRYDONE)
        return false;
        
    ++refillclk;
    int speed = get_bit(quest_rules,qr_FASTFILL) ? 6 : 22;
    int refill_heart_stop=game->get_maxlife();
    int refill_magic_stop=game->get_maxmagic();
    
    if(refill_why>=0 && itemsbuf[refill_why].family==itype_potion)
    {
        refill_heart_stop=zc_min(potion_life+(itemsbuf[refill_why].flags & ITEM_FLAG1 ?int(game->get_maxlife()*(itemsbuf[refill_why].misc1 /100.0)):((itemsbuf[refill_why].misc1 *HP_PER_HEART))),game->get_maxlife());
        refill_magic_stop=zc_min(potion_magic+(itemsbuf[refill_why].flags & ITEM_FLAG2 ?int(game->get_maxmagic()*(itemsbuf[refill_why].misc2 /100.0)):((itemsbuf[refill_why].misc2 *MAGICPERBLOCK))),game->get_maxmagic());
    }
    
    if(refillclk%speed == 0)
    {
        //   game->life&=0xFFC;
        switch(refill_what)
        {
        case REFILL_LIFE:
            game->set_life(zc_min(refill_heart_stop, (game->get_life()+HP_PER_HEART/2)));
            
            if(game->get_life()>=refill_heart_stop)
            {
                game->set_life(refill_heart_stop);
                kill_sfx();
                sfx(WAV_MSG);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
            
        case REFILL_MAGIC:
            game->set_magic(zc_min(refill_magic_stop, (game->get_magic()+MAGICPERBLOCK/4)));
            
            if(game->get_magic()>=refill_magic_stop)
            {
                game->set_magic(refill_magic_stop);
                kill_sfx();
                sfx(WAV_MSG);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
            
        case REFILL_ALL:
            game->set_life(zc_min(refill_heart_stop, (game->get_life()+HP_PER_HEART/2)));
            game->set_magic(zc_min(refill_magic_stop, (game->get_magic()+MAGICPERBLOCK/4)));
            
            if((game->get_life()>=refill_heart_stop)&&(game->get_magic()>=refill_magic_stop))
            {
                game->set_life(refill_heart_stop);
                game->set_magic(refill_magic_stop);
                kill_sfx();
                sfx(WAV_MSG);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
        }
    }
    
    return true;
}

void LinkClass::getTriforce(int id2)
{
    PALETTE flash_pal;
    
    for(int i=0; i<256; i++)
    {
        flash_pal[i] = get_bit(quest_rules,qr_FADE) ? _RGB(63,63,0) : _RGB(63,63,63);
    }
    
    //get rid off all sprites but Link
    guys.clear();
    items.clear();
    Ewpns.clear();
    Lwpns.clear();
    Sitems.clear();
    chainlinks.clear();
    
    //decorations.clear();
    if(!COOLSCROLL)
    {
        show_subscreen_items=false;
    }
    
    sfx(itemsbuf[id2].playsound);
    music_stop();
    
    if(itemsbuf[id2].misc1)
        jukebox(itemsbuf[id2].misc1+ZC_MIDI_COUNT-1);
    else
        try_zcmusic((char*)"zelda.nsf",5, ZC_MIDI_TRIFORCE);
        
    if(itemsbuf[id2].flags & ITEM_GAMEDATA)
    {
        game->lvlitems[dlevel]|=liTRIFORCE;
    }
    
    int f=0;
    int x2=0;
    int curtain_x=0;
    int c=0;
    
    do
    {
        if(f==40)
        {
            actiontype oldaction = action;
            ALLOFF(true, false);
            action=oldaction;                                    // have to reset this flag
		FFCore.setLinkAction(oldaction);
        }
        
        if(f>=40 && f<88)
        {
            if(get_bit(quest_rules,qr_FADE))
            {
                if((f&3)==0)
                {
                    fade_interpolate(RAMpal,flash_pal,RAMpal,42,0,CSET(6)-1);
                    refreshpal=true;
                }
                
                if((f&3)==2)
                {
                    loadpalset(0,0);
                    loadpalset(1,1);
                    loadpalset(5,5);
                    
                    if(currscr<128) loadlvlpal(DMaps[currdmap].color);
                    else loadlvlpal(0xB); // TODO: Cave/Item Cellar distinction?
                }
            }
            else
            {
                if((f&7)==0)
                {
                    for(int cs2=2; cs2<5; cs2++)
                    {
                        for(int i=1; i<16; i++)
                        {
                            RAMpal[CSET(cs2)+i]=flash_pal[CSET(cs2)+i];
                        }
                    }
                    
                    refreshpal=true;
                }
                
                if((f&7)==4)
                {
                    if(currscr<128) loadlvlpal(DMaps[currdmap].color);
                    else loadlvlpal(0xB);
                    
                    loadpalset(5,5);
                }
            }
        }
        
        if(itemsbuf[id2].flags & ITEM_GAMEDATA)
        {
            if(f==88)
            {
                refill_what=REFILL_ALL;
                refill_why=id2;
                StartRefill(REFILL_ALL);
                refill();
            }
            
            if(f==89)
            {
                if(refill())
                {
                    --f;
                }
            }
        }
        
        if(itemsbuf[id2].flags & ITEM_FLAG1) // Warp out flag
        {
            if(f>=208 && f<288)
            {
                ++x2;
                
                switch(++c)
                {
                case 5:
                    c=0;
                    
                case 0:
                case 2:
                case 3:
                    ++x2;
                    break;
                }
            }
            
            do_dcounters();
            
            if(f<288)
            {
                curtain_x=x2&0xF8;
                draw_screen_clip_rect_x1=curtain_x;
                draw_screen_clip_rect_x2=255-curtain_x;
                draw_screen_clip_rect_y1=0;
                draw_screen_clip_rect_y2=223;
                //draw_screen_clip_rect_show_link=true;
                //draw_screen(tmpscr);
            }
        }
        
        draw_screen(tmpscr);
        //this causes bugs
        //the subscreen appearing over the curtain effect should now be fixed in draw_screen
        //so this is not necessary -DD
        //put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,false);
        
        advanceframe(true);
        ++f;
    }
    while(f<408 || midi_pos > 0 || (zcmusic!=NULL && zcmusic->position<800));   // 800 may not be just right, but it works
    
    action=none; FFCore.setLinkAction(none);
    holdclk=0;
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    draw_screen_clip_rect_y1=0;
    draw_screen_clip_rect_y2=223;
    //draw_screen_clip_rect_show_link=true;
    show_subscreen_items=true;
    
    if(itemsbuf[id2].flags & ITEM_FLAG1 && currscr < 128)
    {
        sdir=dir;
        dowarp(1,0); //side warp
    }
    else
        playLevelMusic();
}

void red_shift()
{
    int tnum=176;
    
    // set up the new palette
    for(int i=CSET(2); i < CSET(4); i++)
    {
        int r = (i-CSET(2)) << 1;
        RAMpal[i+tnum].r = r;
        RAMpal[i+tnum].g = r >> 3;
        RAMpal[i+tnum].b = r >> 4;
    }
    
    // color scale the game screen
    for(int y=0; y<168; y++)
    {
        for(int x=0; x<256; x++)
        {
            int c = framebuf->line[y+playing_field_offset][x];
            int r = zc_min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
            framebuf->line[y+playing_field_offset][x] = (c ? (r+tnum+CSET(2)) : 0);
        }
    }
    
    refreshpal = true;
}



void setup_red_screen_old()
{
    clear_bitmap(framebuf);
    rectfill(scrollbuf, 0, 0, 255, 167, 0);
    
    if(tmpscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG ) do_layer(scrollbuf,1, tmpscr, 0, playing_field_offset, 2);
    
    if(tmpscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ) do_layer(scrollbuf,2, tmpscr, 0, playing_field_offset, 2);
    
    putscr(scrollbuf, 0, 0, tmpscr);
    putscrdoors(scrollbuf,0,0,tmpscr);
    blit(scrollbuf, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
    do_layer(framebuf,0, tmpscr, 0, 0, 2);
    
    if(!(tmpscr->flags7&fLAYER2BG || DMaps[currdmap].flags&dmfLAYER2BG )) do_layer(framebuf,1, tmpscr, 0, 0, 2);
    
    do_layer(framebuf,-2, tmpscr, 0, 0, 2);
    
    if(!(msgdisplaybuf->clip))
    {
        masked_blit(msgdisplaybuf, framebuf,0,0,0,playing_field_offset, 256,168);
    }
    
    if(!(pricesdisplaybuf->clip))
    {
        masked_blit(pricesdisplaybuf, framebuf,0,0,0,playing_field_offset, 256,168);
    }
    
    //red shift
    // color scale the game screen
    for(int y=0; y<168; y++)
    {
        for(int x=0; x<256; x++)
        {
            int c = framebuf->line[y+playing_field_offset][x];
            int r = zc_min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
            framebuf->line[y+playing_field_offset][x] = (c ? (r+CSET(2)) : 0);
        }
    }
    
    //  Link->draw(framebuf);
    blit(framebuf,scrollbuf, 0, playing_field_offset, 256, playing_field_offset, 256, 168);
    
    clear_bitmap(framebuf);
    
    if(!((tmpscr->layermap[2]==0||(tmpscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG ))
            && tmpscr->layermap[3]==0
            && tmpscr->layermap[4]==0
            && tmpscr->layermap[5]==0
            && !overheadcombos(tmpscr)))
    {
        if(!(tmpscr->flags7&fLAYER3BG || DMaps[currdmap].flags&dmfLAYER3BG )) do_layer(framebuf,2, tmpscr, 0, 0, 2);
        
        do_layer(framebuf,3, tmpscr, 0, 0, 2);
        do_layer(framebuf,-1, tmpscr, 0, 0, 2);
        do_layer(framebuf,4, tmpscr, 0, 0, 2);
        do_layer(framebuf,5, tmpscr, 0, 0, 2);
        
        //do an AND masked blit for messages on top of layers
        if(!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip))
        {
            for(int y=0; y<168; y++)
            {
                for(int x=0; x<256; x++)
                {
                    int c1 = framebuf->line[y+playing_field_offset][x];
                    int c2 = msgdisplaybuf->line[y][x];
                    int c3 = pricesdisplaybuf->line[y][x];
                    
                    if(c1 && c3)
                    {
                        framebuf->line[y+playing_field_offset][x] = c3;
                    }
                    else if(c1 && c2)
                    {
                        framebuf->line[y+playing_field_offset][x] = c2;
                    }
                }
            }
        }
        
        //red shift
        // color scale the game screen
        for(int y=0; y<168; y++)
        {
            for(int x=0; x<256; x++)
            {
                int c = framebuf->line[y+playing_field_offset][x];
                int r = zc_min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
                framebuf->line[y+playing_field_offset][x] = r+CSET(2);
            }
        }
    }
    
    blit(framebuf,scrollbuf, 0, playing_field_offset, 0, playing_field_offset, 256, 168);
    
    // set up the new palette
    for(int i=CSET(2); i < CSET(4); i++)
    {
        int r = (i-CSET(2)) << 1;
        RAMpal[i].r = r;
        RAMpal[i].g = r >> 3;
        RAMpal[i].b = r >> 4;
    }
    
    refreshpal = true;
}



void slide_in_color(int color)
{
    for(int i=1; i<16; i+=3)
    {
        RAMpal[CSET(2)+i+2] = RAMpal[CSET(2)+i+1];
        RAMpal[CSET(2)+i+1] = RAMpal[CSET(2)+i];
        RAMpal[CSET(2)+i]   = NESpal(color);
    }
    
    refreshpal=true;
}

void LinkClass::gameover()
{
	int f=0;
    
	action=none; FFCore.setLinkAction(dying); //mayhaps a new action of 'gameover'? -Z
	Playing=false;
    
	if(!debug_enabled)
	{
		Paused=false;
	}
    
	game->set_deaths(zc_min(game->get_deaths()+1,999));
	dir=down;
	music_stop();
	kill_sfx();
	attackclk=hclk=superman=0;
	scriptcoldet = 1;
    
	for(int i=0; i<32; i++) miscellaneous[i] = 0;
    
	//get rid off all sprites but Link
	guys.clear();
	items.clear();
	Ewpns.clear();
	Lwpns.clear();
	Sitems.clear();
	chainlinks.clear();
	decorations.clear();
    
	playing_field_offset=56; // otherwise, red_shift() may go past the bottom of the screen
	quakeclk=wavy=0;
    
	//in original Z1, Link marker vanishes at death.
	//code in subscr.cpp, put_passive_subscr checks the following value.
	//color 255 is a GUI color, so quest makers shouldn't be using this value.
	//Also, subscreen is static after death in Z1.
	int tmp_link_dot = QMisc.colors.link_dot;
	QMisc.colors.link_dot = 255;
	//doesn't work
	//scrollbuf is tampered with by draw_screen()
	//put_passive_subscr(scrollbuf, &QMisc, 256, passive_subscreen_offset, false, false);//save this and reuse it.
	BITMAP *subscrbmp = create_bitmap_ex(8, framebuf->w, framebuf->h);
	clear_bitmap(subscrbmp);
	put_passive_subscr(subscrbmp, &QMisc, 0, passive_subscreen_offset, false, sspUP);
	QMisc.colors.link_dot = tmp_link_dot;
    
	do
	{
		if(f<254)
		{
			if(f<=32)
			{
				hclk=(32-f);
			}
            
			if(f>=62 && f<138)
			{
				switch((f-62)%20)
				{
				case 0:
					dir=right;
					break;
                    
				case 5:
					dir=up;
					break;
                    
				case 10:
					dir=left;
					break;
                    
				case 15:
					dir=down;
					break;
				}
                
				linkstep();
			}
            
			if(f>=194 && f<208)
			{
				if(f==194)
				{
					action=dying;
					FFCore.setLinkAction(dying);
				}
                    
				extend = 0;
				cs = wpnsbuf[iwDeath].csets&15;
				tile = wpnsbuf[iwDeath].newtile;
                
				if(BSZ)
				{
					tile += (f-194)/3;
				}
				else if(f>=204)
				{
					++tile;
				}
			}
            
			if(f==208)
			{
				if ( dontdraw < 2 ) { dontdraw = 1; }
			}
			if(get_bit(quest_rules,qr_FADE))
			{
				if(f < 170)
				{
					if(f<60)
					{
						draw_screen(tmpscr);
						//reuse our static subscreen
						set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
						blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
					}
                    
					if(f==60)
					{
						red_shift();
						create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
						create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
						memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                        
						for(int q=0; q<PAL_SIZE; q++)
						{
							trans_table2.data[0][q] = q;
							trans_table2.data[q][q] = q;
						}
					}
                    
					if(f>=60 && f<=169)
					{
						draw_screen(tmpscr);
						//reuse our static subscreen
						blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
						red_shift();
                        
					}
                    
					if(f>=139 && f<=169)//fade from red to black
					{
						fade_interpolate(RAMpal,black_palette,RAMpal, (f-138)<<1, 224, 255);
						create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
						create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
						memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                        
						for(int q=0; q<PAL_SIZE; q++)
						{
							trans_table2.data[0][q] = q;
							trans_table2.data[q][q] = q;
						}
                        
						refreshpal=true;
					}
				}
				else //f>=170
				{
					if(f==170)//make Link grayish
					{
						fade_interpolate(RAMpal,black_palette,RAMpal,64, 224, 255);
                        
						for(int i=CSET(6); i < CSET(7); i++)
						{
							int g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
							RAMpal[i] = _RGB(g,g,g);
						}
                        
						refreshpal = true;
					}
                    
					//draw only link. otherwise black layers might cover him.
					rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
					draw(framebuf);
					blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
				}
			}
			else //!qr_FADE
			{
				if(f==58)
				{
					for(int i = 0; i < 96; i++)
						tmpscr->cset[i] = 3;
                        
					for(int j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int i=0; i<96; i++)
								tmpscr2[j].cset[i] = 3;
				}
                
				if(f==59)
				{
					for(int i = 96; i < 176; i++)
						tmpscr->cset[i] = 3;
                        
					for(int j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int i=96; i<176; i++)
								tmpscr2[j].cset[i] = 3;
				}
                
				if(f==60)
				{
					for(int i=0; i<176; i++)
					{
						tmpscr->cset[i] = 2;
					}
                    
					for(int j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int i=0; i<176; i++)
								tmpscr2[j].cset[i] = 2;
                                
					for(int i=1; i<16; i+=3)
					{
						RAMpal[CSET(2)+i]   = NESpal(0x17);
						RAMpal[CSET(2)+i+1] = NESpal(0x16);
						RAMpal[CSET(2)+i+2] = NESpal(0x26);
					}
                    
					refreshpal=true;
				}
                
				if(f==139)
					slide_in_color(0x06);
                    
				if(f==149)
					slide_in_color(0x07);
                    
				if(f==159)
					slide_in_color(0x0F);
                    
				if(f==169)
				{
					slide_in_color(0x0F);
					slide_in_color(0x0F);
				}
                
				if(f==170)
				{
					for(int i=1; i<16; i+=3)
					{
						RAMpal[CSET(6)+i]   = NESpal(0x10);
						RAMpal[CSET(6)+i+1] = NESpal(0x30);
						RAMpal[CSET(6)+i+2] = NESpal(0x00);
						refreshpal = true;
					}
				}
                
				if(f < 169)
				{
					draw_screen(tmpscr);
					//reuse our static subscreen
					blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
				}
				else
				{
					//draw only link. otherwise black layers might cover him.
					rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
					draw(framebuf);
					blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
				}
			}
		}
		else if(f<350)//draw 'GAME OVER' text
		{
			clear_bitmap(framebuf);
			blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
			textout_ex(framebuf,zfont,"GAME OVER",96,playing_field_offset+80,1,-1);
		}
		else
		{
			clear_bitmap(framebuf);
		}
        
		//SFX... put them all here
		switch(f)
		{
		case   0:
			sfx(getHurtSFX(),pan(int(x)));
			break;
			//Death sound.
		case  60:
			sfx(WAV_SPIRAL);
			break;
			//Message sound. 
		case 194:
			sfx(WAV_MSG);
			break;
		}
        
		advanceframe(true);
		++f;
	}
	while(f<353 && !Quit);
    
	destroy_bitmap(subscrbmp);
	action=none; FFCore.setLinkAction(none);
	if ( dontdraw < 2 ) { dontdraw=0; }
}


void LinkClass::ganon_intro()
{
    /*
    ************************
    * GANON INTRO SEQUENCE *
    ************************
    -25 DOT updates
    -24 LINK in
    0 TRIFORCE overhead - code begins at this point (f == 0)
    47 GANON in
    58 LIGHT step
    68 LIGHT step
    78 LIGHT step
    255 TRIFORCE out
    256 TRIFORCE in
    270 TRIFORCE out
    271 GANON out, LINK face up
    */
    loaded_guys=true;
    loaditem();
    
    if(game->lvlitems[dlevel]&liBOSS)
    {
        return;
    }
    
    dir=down;
    if ( !isSideview() )
    {
	fall = 0; //Fix midair glitch on holding triforce. -Z
	z = 0;
    }
    action=landhold2; FFCore.setLinkAction(landhold2);
    holditem=getItemID(itemsbuf,itype_triforcepiece, 1);
    //not good, as this only returns the highest level that Link possesses. -DD
    //getHighestLevelOfFamily(game, itemsbuf, itype_triforcepiece, false));
    
    for(int f=0; f<271 && !Quit; f++)
    {
        if(f==47)
        {
            music_stop();
            stop_sfx(WAV_ROAR);
            sfx(WAV_GASP);
            sfx(WAV_GANON);
            int Id=0;
            
            for(int i=0; i<eMAXGUYS; i++)
            {
                if(guysbuf[i].flags2&eneflag_ganon)
                {
                    Id=i;
                    break;
                }
            }
            
            if(current_item(itype_ring))
            {
                addenemy(160,96,Id,0);
            }
            else
            {
                addenemy(80,32,Id,0);
            }
        }
        
        if(f==48)
        {
            lighting(true,true); // Hmm. -L
            f += 30;
        }
        
        //NES Z1, the triforce vanishes for one frame in two cases
        //while still showing Link's two-handed overhead sprite.
	//This should be a Quest Rule for NES Accuracy. -Z
        if(f==255 || f==270)
        {
            holditem=-1;
        }
        
        if(f==256)
        {
            holditem=getItemID(itemsbuf,itype_triforcepiece,1);
        }
        
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(rSbtn())
        {
            conveyclk=3;
            int tmp_subscr_clk = frame;
            dosubscr(&QMisc);
            newscr_clk += frame - tmp_subscr_clk;
        }
        
    }
    
    action=none; FFCore.setLinkAction(none);
    dir=up;
    
    if(!getmapflag() && (tunes[MAXMIDIS-1].data))
        jukebox(MAXMIDIS-1);
    else
        playLevelMusic();
        
    currcset=DMaps[currdmap].color;
    if ( get_bit(quest_rules, qr_NOGANONINTRO) == 0) 
    {
	    dointro();
    }
    //dointro(); //This is likely what causes Ganon Rooms to repeat the DMap intro.  
    //I suppose it is to allow the user to make Gaanon rooms have their own dialogue, if they are
    //on a different DMap. 
    //~ Otherwise, why is it here?! -Z
    
    
    //if ( !(DMaps[currdmap].flags&dmfALWAYSMSG) ) { dointro(); } //This is likely what causes Ganon Rooms to repeat the DMap intro.  
    //If we try it this way: The dmap flag /always display intro string/ is probably why James had this issue. 
    
    //The only fix that I can think of, off the top of me head, is either a QR or a Screen Flag to disable the intro text.
    //Users who use that dmap rule should put ganons room on its own DMap! -Z 
    cont_sfx(WAV_ROAR);	
}

void LinkClass::saved_Zelda()
{
    Playing=Paused=false;
    action=won; FFCore.setLinkAction(won);
    Quit=qWON;
    hclk=0;
    x = 136;
    y = (isdungeon() && currscr<128) ? 75 : 73;
    z = fall = spins = 0;
    dir=left;
}

void LinkClass::reset_swordcharge()
{
    charging=spins=tapping=0;
}

void LinkClass::reset_hookshot()
{
    if(action!=rafting && action!=landhold1 && action!=landhold2)
    {
        action=none; FFCore.setLinkAction(none);
    }
    
    hookshot_frozen=false;
    hookshot_used=false;
    pull_link=false;
    add_chainlink=false;
    del_chainlink=false;
    hs_fix=false;
    Lwpns.del(Lwpns.idFirst(wHSHandle));
    Lwpns.del(Lwpns.idFirst(wHookshot));
    chainlinks.clear();
    int index=directItem>-1 ? directItem : current_item_id(itype_hookshot);
    
    if(index>=0)
    {
        stop_sfx(itemsbuf[index].usesound);
    }
    
    hs_xdist=0;
    hs_ydist=0;
}


bool LinkClass::can_deploy_ladder()
{
    bool ladderallowed = ((!get_bit(quest_rules,qr_LADDERANYWHERE) && tmpscr->flags&fLADDER) || isdungeon()
                          || (get_bit(quest_rules,qr_LADDERANYWHERE) && !(tmpscr->flags&fLADDER)));
    return (current_item_id(itype_ladder)>-1 && ladderallowed && !ilswim && z==0 &&
            (!isSideview() || ON_SIDEPLATFORM));
}

void LinkClass::reset_ladder()
{
    ladderx=laddery=0;
}

void LinkClass::check_conveyor()
{
    if(action==casting||action==drowning||inlikelike||pull_link||(z>0 && !(tmpscr->flags2&fAIRCOMBOS)))
    {
        return;
    }
    
    WalkflagInfo info;
    int xoff,yoff;
    int deltax=0, deltay=0;
    
    if(conveyclk<=0)
    {
        is_on_conveyor=false;
        int ctype;
        ctype=(combobuf[MAPCOMBO(x+7,y+(bigHitbox?8:12))].type);
        deltax=combo_class_buf[ctype].conveyor_x_speed;
        deltay=combo_class_buf[ctype].conveyor_y_speed;
        
        if((deltax==0&&deltay==0)&&(isSideview() && ON_SIDEPLATFORM))
        {
            ctype=(combobuf[MAPCOMBO(x+8,y+16)].type);
            deltax=combo_class_buf[ctype].conveyor_x_speed;
            deltay=combo_class_buf[ctype].conveyor_y_speed;
        }
        
        if(deltax!=0||deltay!=0)
        {
            is_on_conveyor=true;
        }
        
        if(deltay<0)
        {
            info = walkflag(x,y+8-(bigHitbox*8)-2,2,up);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                int step=0;
                
                if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!diagonalMovement)
                {
                    while(step<(abs(deltay)*(isSideview()?2:1)))
                    {
                        yoff=((int)y-step)&7;
                        
                        if(!yoff) break;
                        
                        step++;
                    }
                }
                else
                {
                    step=abs(deltay);
                }
                
                y=y-step;
                hs_starty-=step;
                
                for(int j=0; j<chainlinks.Count(); j++)
                {
                    chainlinks.spr(j)->y-=step;
                }
                
                if(Lwpns.idFirst(wHookshot)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHookshot))->y-=step;
                }
                
                if(Lwpns.idFirst(wHSHandle)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHSHandle))->y-=step;
                }
            }
            else checkdamagecombos(x,y+8-(bigHitbox ? 8 : 0)-2);
        }
        else if(deltay>0)
        {
            info = walkflag(x,y+15+2,2,down);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                int step=0;
                
                if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!diagonalMovement)
                {
                    while(step<abs(deltay))
                    {
                        yoff=((int)y+step)&7;
                        
                        if(!yoff) break;
                        
                        step++;
                    }
                }
                else
                {
                    step=abs(deltay);
                }
                
                y=y+step;
                hs_starty+=step;
                
                for(int j=0; j<chainlinks.Count(); j++)
                {
                    chainlinks.spr(j)->y+=step;
                }
                
                if(Lwpns.idFirst(wHookshot)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=step;
                }
                
                if(Lwpns.idFirst(wHSHandle)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=step;
                }
            }
            else checkdamagecombos(x,y+15);
        }
        
        if(deltax<0)
        {
            info = walkflag(x-int(lsteps[int(x)&7]),y+8-(bigHitbox ? 8 : 0),1,left);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                int step=0;
                
                if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!diagonalMovement)
                {
                    while(step<abs(deltax))
                    {
                        xoff=((int)x-step)&7;
                        
                        if(!xoff) break;
                        
                        step++;
                    }
                }
                else
                {
                    step=abs(deltax);
                }
                
                x=x-step;
                hs_startx-=step;
                
                for(int j=0; j<chainlinks.Count(); j++)
                {
                    chainlinks.spr(j)->x-=step;
                }
                
                if(Lwpns.idFirst(wHookshot)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHookshot))->x-=step;
                }
                
                if(Lwpns.idFirst(wHSHandle)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHSHandle))->x-=step;
                }
            }
            else checkdamagecombos(x-int(lsteps[int(x)&7]),y+8-(bigHitbox ? 8 : 0));
        }
        else if(deltax>0)
        {
            info = walkflag(x+15+2,y+8-(bigHitbox ? 8 : 0),1,right);
            execute(info);
            
            if(!info.isUnwalkable())
            {
                int step=0;
                
                if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!diagonalMovement)
                {
                    while(step<abs(deltax))
                    {
                        xoff=((int)x+step)&7;
                        
                        if(!xoff) break;
                        
                        step++;
                    }
                }
                else
                {
                    step=abs(deltax);
                }
                
                x=x+step;
                hs_startx+=step;
                
                for(int j=0; j<chainlinks.Count(); j++)
                {
                    chainlinks.spr(j)->x+=step;
                }
                
                if(Lwpns.idFirst(wHookshot)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=step;
                }
                
                if(Lwpns.idFirst(wHSHandle)>-1)
                {
                    Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=step;
                }
            }
            else checkdamagecombos(x+15+2,y+8-(bigHitbox ? 8 : 0));
        }
    }
}

void LinkClass::setNayrusLoveShieldClk(int newclk)
{
    NayrusLoveShieldClk=newclk;
    
    if(decorations.idCount(dNAYRUSLOVESHIELD)==0)
    {
        decoration *dec;
        decorations.add(new dNayrusLoveShield(LinkX(), LinkY(), dNAYRUSLOVESHIELD, 0));
        decorations.spr(decorations.Count()-1)->misc=0;
        decorations.add(new dNayrusLoveShield(LinkX(), LinkY(), dNAYRUSLOVESHIELD, 0));
        dec=(decoration *)decorations.spr(decorations.Count()-1);
        decorations.spr(decorations.Count()-1)->misc=1;
    }
}

int LinkClass::getNayrusLoveShieldClk()
{
    return NayrusLoveShieldClk;
}

int LinkClass::getHoverClk()
{
    return hoverclk;
}

int LinkClass::getHoldClk()
{
    return holdclk;
}

int LinkClass::getLastLensID(){
	return last_lens_id;
}

void LinkClass::setLastLensID(int p_item){
	last_lens_id = p_item;
}

void LinkClass::execute(LinkClass::WalkflagInfo info)
{
    int flags = info.getFlags();
    
    if(flags & WalkflagInfo::CLEARILSWIM)
        ilswim =false;
    else if(flags & WalkflagInfo::SETILSWIM)
        ilswim = true;
        
    if(flags & WalkflagInfo::CLEARCHARGEATTACK)
    {
        charging = 0;
        attackclk = 0;
    }
    
    if(flags & WalkflagInfo::SETDIR)
    {
        dir = info.getDir();
    }
    
    if(flags & WalkflagInfo::SETHOPCLK)
    {
        hopclk = info.getHopClk();
    }
    
    if(flags & WalkflagInfo::SETHOPDIR)
    {
        hopdir = info.getHopDir();
    }
    
}

LinkClass::WalkflagInfo LinkClass::WalkflagInfo::operator ||(LinkClass::WalkflagInfo other)
{
    LinkClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
    
    int flags1 = (flags & ~UNWALKABLE) & (other.flags & ~UNWALKABLE);
    int flags2 = (flags & UNWALKABLE) | (other.flags & UNWALKABLE);
    ret.flags = flags1 | flags2;
    return ret;
}

LinkClass::WalkflagInfo LinkClass::WalkflagInfo::operator &&(LinkClass::WalkflagInfo other)
{
    LinkClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
    
    ret.flags = flags & other.flags;
    return ret;
}

LinkClass::WalkflagInfo LinkClass::WalkflagInfo::operator !()
{
    LinkClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = newhopdir;
    
    ret.flags = flags ^ UNWALKABLE;
    return ret;
}

/*** end of link.cpp ***/
